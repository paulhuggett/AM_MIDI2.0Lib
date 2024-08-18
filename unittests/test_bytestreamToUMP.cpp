// DUT
#include "midi2/bytestreamToUMP.hpp"
#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

// Standard library
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace {

using testing::ElementsAre;
using testing::ElementsAreArray;
using testing::IsEmpty;
using testing::TestWithParam;

template <typename ArrayLike> struct HexContainer {
  constexpr explicit HexContainer(ArrayLike const& container_)
      : container{container_} {}

  friend std::ostream& operator<<(std::ostream& os, HexContainer<ArrayLike> const& hc) {
    auto const* separator = "";
    for (auto v : hc.container) {
      os << separator << "0x" << std::hex << std::uppercase << static_cast<unsigned>(v);
      separator = ", ";
    }
    return os;
  }

  ArrayLike const& container;
};
template <typename ArrayLike>
HexContainer(ArrayLike const&) -> HexContainer<ArrayLike>;

template <std::size_t Size> auto convert(midi2::bytestreamToUMP bs2ump, std::array<std::uint8_t, Size> const& input) {
  std::vector<std::uint32_t> output;
  for (std::uint8_t const b : input) {
    bs2ump.bytestreamParse(b);
    while (bs2ump.availableUMP()) {
      output.push_back(bs2ump.readUMP());
    }
  }
  return output;
}

constexpr std::uint32_t ump_cvm(midi2::status s) {
  static_assert(
      std::is_same_v<std::underlying_type_t<midi2::status>, std::uint8_t>,
      "status type must be a std::uint8_t");
  assert((s & 0x0F) == 0 &&
         "Bottom 4 bits of a channel voice message status enum  must be 0");
  return std::uint32_t{s} >> 4;
}

constexpr auto ump_note_on = ump_cvm(midi2::status::note_on);
constexpr auto ump_note_off = ump_cvm(midi2::status::note_off);
constexpr auto ump_pitch_bend = ump_cvm(midi2::status::pitch_bend);
constexpr auto ump_control_change = ump_cvm(midi2::status::cc);
constexpr auto ump_program_change = ump_cvm(midi2::status::program_change);
constexpr auto ump_channel_pressure = ump_cvm(midi2::status::channel_pressure);

// NOLINTNEXTLINE
TEST(BytestreamToUMP, NoteOnWithRunningStatus) {
  std::array const input{std::uint8_t{0x81}, std::uint8_t{0x60},
                         std::uint8_t{0x50}, std::uint8_t{0x70},
                         std::uint8_t{0x70}};
  auto const actual = convert(midi2::bytestreamToUMP{}, input);
  std::array const expected{std::uint32_t{0x20816050},
                            std::uint32_t{0x20817070}};
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, NoteOnImplicitNoteOffWithRunningStatus) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint8_t{127};

  constexpr auto group = std::uint32_t{0};

  // A note on message followed by a note-on with velocity 0. The second of
  // these should be treated as a note-off. Running status is used for the two
  // input messages.
  std::array const input{std::uint8_t{midi2::status::note_on | channel},
                         note_number, velocity, note_number, std::uint8_t{0}};

  auto const m0 = std::uint32_t{
      (2U << 28) | (group << 24) | (ump_note_on << 20) | (channel << 16) |
      (std::uint32_t{note_number} << 8) | std::uint32_t{velocity}};
  auto const m1 = std::uint32_t{
      (2U << 28) | (group << 24) | (ump_note_on << 20) | (channel << 16) |
      (std::uint32_t{note_number} << 8) | std::uint32_t{0x00}};
  std::array const expected{m0, m1};
  auto const actual = convert(midi2::bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2NoteOnImplicitNoteOffWithRunningStatus) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint8_t{127};

  constexpr auto message_type =
      static_cast<std::uint32_t>(midi2::ump_message_type::m2cvm);
  constexpr auto group = std::uint32_t{0};

  // A note on message followed by a note-on with velocity 0. The second of
  // these should become a note-off. Running status is used for the two input
  // messages.
  std::array const input{std::uint8_t{midi2::status::note_on | channel},
                         note_number, velocity, note_number, std::uint8_t{0}};

  auto const m0 =
      std::uint32_t{(message_type << 28) | (group << 24) | (channel << 16) |
                    (ump_note_on << 20) | (std::uint32_t{note_number} << 8)};
  auto const m1 = std::uint32_t{(midi2::scaleUp(velocity, 7, 16) << 16)};
  auto const m2 =
      std::uint32_t{(message_type << 28) | (group << 24) | (channel << 16) |
                    (ump_note_off << 20) | (std::uint32_t{note_number} << 8)};
  auto const m3 = std::uint32_t{(midi2::scaleUp(0x40, 7, 16) << 16)};
  std::array const expected{m0, m1, m2, m3};
  auto const actual = convert(midi2::bytestreamToUMP{true}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi1ChannelPressure) {
  constexpr auto channel = std::uint8_t{5};    // 4 bits
  constexpr auto pressure = std::uint8_t{57};  // 7 bits
  std::array const input{std::uint8_t{midi2::channel_pressure | channel},
                         pressure};

  constexpr auto message_type =
      static_cast<std::uint32_t>(midi2::ump_message_type::m1cvm);
  constexpr auto group = std::uint32_t{0};
  std::array const expected{std::uint32_t{
      (message_type << 28) | (group << 24) | (ump_channel_pressure << 20) |
      (std::uint32_t{channel} << 16) | (std::uint32_t{pressure} << 8)}};

  auto const actual = convert(midi2::bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2ChannelPressure) {
  constexpr auto channel = std::uint8_t{5};    // 4 bits
  constexpr auto pressure = std::uint8_t{57};  // 7 bits
  std::array const input{std::uint8_t{midi2::channel_pressure | channel},
                         pressure};

  constexpr auto message_type =
      static_cast<std::uint32_t>(midi2::ump_message_type::m2cvm);
  constexpr auto group = std::uint32_t{0};
  std::array const expected{std::uint32_t{(message_type << 28) | (group << 24) |
                                          (ump_channel_pressure << 20) |
                                          (std::uint32_t{channel} << 16)},
                            midi2::scaleUp(pressure, 7, 32)};

  auto const actual = convert(midi2::bytestreamToUMP{true}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, PitchBend) {
  constexpr auto bend_lsb = std::uint8_t{0x00};
  constexpr auto bend_msb = std::uint8_t{0x40};
  constexpr auto channel = std::uint8_t{3};
  std::array const input{std::uint8_t{midi2::status::pitch_bend | channel},
                         bend_lsb, bend_msb};

  constexpr auto message_type =
      static_cast<std::uint32_t>(midi2::ump_message_type::m1cvm);
  constexpr auto group = std::uint32_t{0};

  std::array const expected{
      std::uint32_t{(message_type << 28) | (group << 24) |
                    (ump_pitch_bend << 20) | (channel << 16) |
                    (std::uint32_t{bend_lsb} << 8) | std::uint32_t{bend_msb}}};

  auto const actual = convert(midi2::bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2PitchBend) {
  constexpr auto bend_lsb = std::uint8_t{0x00};
  constexpr auto bend_msb = std::uint8_t{0x40};
  constexpr auto channel = std::uint8_t{3};
  std::array const input{std::uint8_t{midi2::status::pitch_bend | channel},
                         bend_lsb, bend_msb};

  constexpr auto message_type =
      static_cast<std::uint32_t>(midi2::ump_message_type::m2cvm);
  constexpr auto group = std::uint32_t{0};
  std::array const expected{
      std::uint32_t{(message_type << 28) | (group << 24) |
                    (ump_pitch_bend << 20) | (channel << 16)},
      std::uint32_t{0x80000000}};

  auto const actual = convert(midi2::bytestreamToUMP{true}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SeqStartMidNoteOn) {
  constexpr auto channel = std::uint8_t{1};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint8_t{127};

  // A real-time message can appear anywhere, even in the middle of another
  // multi-byte message.
  std::array const input{std::uint8_t{midi2::status::note_on | channel},
                         std::uint8_t{midi2::status::seqstart}, note_number,
                         velocity};

  constexpr auto group = std::uint32_t{0};
  std::array const expected{
      std::uint32_t{(1U << 28) | (group << 24) |
                    (midi2::status::seqstart << 16)},
      std::uint32_t{(2U << 28) | (group << 24) | (channel << 16) |
                    (ump_note_on << 20) | (std::uint32_t{note_number} << 8) |
                    std::uint32_t{velocity}}};

  auto const actual = convert(midi2::bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SystemMessageOneByte) {
  std::array const input{std::uint8_t{0xF8}};
  EXPECT_THAT(convert(midi2::bytestreamToUMP{}, input),
              ElementsAre(UINT32_C(0x10f80000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, BankAndProgramChange) {
  constexpr auto channel = std::uint8_t{0x0F};  // 4 bits
  constexpr auto program = std::uint8_t{0x42};  // 8 bits
  constexpr auto bank_msb = std::uint8_t{0x51};  // 8 bits
  constexpr auto bank_lsb = std::uint8_t{0x01};  // 8 bits

  constexpr auto controller_set_msb = std::uint8_t{0x00};
  constexpr auto controller_set_lsb = std::uint8_t{0x20};

  std::array const input{
      // MSB (Coarse) Bank Select
      std::uint8_t{midi2::status::cc | channel}, controller_set_msb, bank_msb,
      // LSB (Fine) Bank Select
      std::uint8_t{midi2::status::cc | channel}, controller_set_lsb, bank_lsb,
      // Program Change
      std::uint8_t{midi2::status::program_change | channel}, program};

  constexpr auto message_type =
      static_cast<std::uint32_t>(midi2::ump_message_type::m1cvm);  // 4 bits
  constexpr auto group = std::uint32_t{0x00};         // 4 bits

  std::array const expected{
      // MSB (Coarse) Bank Select
      std::uint32_t{(message_type << 28) | (group << 24) |
                    (ump_control_change << 20) |
                    (std::uint32_t{channel} << 16) |
                    (std::uint32_t{controller_set_msb} << 8) | bank_msb},
      // LSB (Fine) Bank Select
      std::uint32_t{(message_type << 28) | (group << 24) |
                    (ump_control_change << 20) |
                    (std::uint32_t{channel} << 16) |
                    (std::uint32_t{controller_set_lsb} << 8) | bank_lsb},
      // Program Change
      std::uint32_t{
          (message_type << 28) | (group << 24) | (ump_program_change << 20) |
          (std::uint32_t{channel} << 16) | (std::uint32_t{program} << 8)}};

  auto const actual = convert(midi2::bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2BankAndProgramChange) {
  constexpr auto channel = std::uint8_t{0x0F};   // 4 bits
  constexpr auto program = std::uint8_t{0x42};   // 8 bits
  constexpr auto bank_msb = std::uint8_t{0x51};  // 8 bits
  constexpr auto bank_lsb = std::uint8_t{0x01};  // 8 bits

  std::array const input{
      // MSB (Coarse) Bank Select
      std::uint8_t{midi2::status::cc | channel}, std::uint8_t{0x00}, bank_msb,
      // LSB (Fine) Bank Select
      std::uint8_t{midi2::status::cc | channel}, std::uint8_t{0x20}, bank_lsb,
      // Program Change
      std::uint8_t{midi2::status::program_change | channel}, program};

  constexpr auto message_type =
      static_cast<std::uint32_t>(midi2::ump_message_type::m2cvm);  // 4 bits
  constexpr auto group = std::uint32_t{0x00};         // 4 bits
  constexpr auto option_flags = std::uint32_t{0x00};  // 7 bits
  constexpr auto bank_valid = std::uint32_t{0x01};    // 1 bit

  std::array const expected{
      std::uint32_t{(message_type << 28) | (group << 24) | (channel << 16) |
                    (ump_program_change << 20) | (option_flags << 1) |
                    bank_valid},
      std::uint32_t{(std::uint32_t{program} << 24) |
                    (std::uint32_t{bank_msb} << 8) | std::uint32_t{bank_lsb}}};

  auto const actual = convert(midi2::bytestreamToUMP{true}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, PCTwoBytes) {
  std::array<uint8_t, 2> const input{0xC6, 0x40};
  EXPECT_THAT(convert(midi2::bytestreamToUMP{}, input),
              ElementsAre(UINT32_C(0x20C64000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SysEx) {
  using u8 = std::uint8_t;
  constexpr auto start = u8{midi2::status::sysex_start};
  constexpr auto stop = u8{midi2::status::sysex_stop};
  std::array const input{
      start,    u8{0x7E}, u8{0x7F}, u8{0x0D}, u8{0x70}, u8{0x02}, u8{0x4B},
      u8{0x60}, u8{0x7A}, u8{0x73}, u8{0x7F}, u8{0x7F}, u8{0x7F}, u8{0x7F},
      u8{0x7D}, u8{0x00}, u8{0x00}, u8{0x00}, u8{0x00}, u8{0x01}, u8{0x00},
      u8{0x00}, u8{0x00}, u8{0x03}, u8{0x00}, u8{0x00}, u8{0x00}, u8{0x10},
      u8{0x00}, u8{0x00}, u8{0x00}, stop};
  std::array const expected{
      std::uint32_t{0x30167E7F}, std::uint32_t{0x0D70024B},
      std::uint32_t{0x3026607A}, std::uint32_t{0x737F7F7F},
      std::uint32_t{0x30267F7D}, std::uint32_t{0x00000000},
      std::uint32_t{0x30260100}, std::uint32_t{0x00000300},
      std::uint32_t{0x30360000}, std::uint32_t{0x10000000}};
  auto const actual = convert(midi2::bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

TEST(BytestreamToUMP, MultipleSysExMessages) {
  using u8 = std::uint8_t;
  constexpr auto start = u8{midi2::status::sysex_start};
  constexpr auto stop = u8{midi2::status::sysex_stop};
  std::array const input{
      start,  // start sysex
      u8{0x0A}, u8{0x0B}, u8{0x0C}, u8{0x0D}, u8{0x0E}, u8{0x0F},
      u8{0x1A}, u8{0x1B}, u8{0x1C}, u8{0x1D}, u8{0x1E}, u8{0x1F},
      stop,   // end sysex
      start,  // start sysex
      u8{0x2A}, u8{0x2B}, u8{0x2C}, u8{0x2D}, u8{0x2E}, u8{0x2F},
      u8{0x3A}, u8{0x3B}, u8{0x3C}, u8{0x3D}, u8{0x3E},
      stop,   // end sysex
      start,  // start sysex
      u8{0x4A}, u8{0x4B}, u8{0x4C}, u8{0x4D}, u8{0x4E},
      stop,   // end sysex
      start,  // start sysex
      u8{0x5A}, u8{0x5B}, u8{0x5C}, u8{0x5D},
      stop,   // end sysex
      start,  // start sysex
      u8{0x6A}, u8{0x6B}, u8{0x6C},
      stop,   // end sysex
      start,  // start sysex
      u8{0x7A}, u8{0x7B},
      stop,  // end sysex
  };

  enum class sysex7_status : std::uint8_t {
    single_ump = 0x0,
    start = 0x1,
    cont = 0x02,
    end = 0x03,
  };

  constexpr auto group = std::uint32_t{0};
  auto in_one_message = [](u8 number_of_bytes, u8 data0, u8 data1) {
    midi2::types::sysex7_w1 w1{};
    w1.mt = static_cast<std::uint8_t>(midi2::ump_message_type::sysex7);
    w1.group = group;
    w1.status = static_cast<std::uint8_t>(sysex7_status::single_ump);
    w1.number_of_bytes = number_of_bytes;
    w1.data0 = data0;
    w1.data1 = data1;
    return std::bit_cast<std::uint32_t>(w1);
  };
  auto start_message = [](u8 data0, u8 data1) {
    midi2::types::sysex7_w1 w1{};
    w1.mt = static_cast<std::uint8_t>(midi2::ump_message_type::sysex7);
    w1.group = group;
    w1.status = static_cast<std::uint8_t>(sysex7_status::start);
    w1.number_of_bytes = std::uint8_t{6};
    w1.data0 = data0;
    w1.data1 = data1;
    return std::bit_cast<std::uint32_t>(w1);
  };
  auto end_message = [](u8 number_of_bytes, u8 data0, u8 data1) {
    assert(number_of_bytes <= 6);
    midi2::types::sysex7_w1 w1{};
    w1.mt = static_cast<std::uint8_t>(midi2::ump_message_type::sysex7);
    w1.group = group;
    w1.status = static_cast<std::uint8_t>(sysex7_status::end);
    w1.number_of_bytes = number_of_bytes;
    w1.data0 = data0;
    w1.data1 = data1;
    return std::bit_cast<std::uint32_t>(w1);
  };

  std::array const expected{
      start_message(0x0A, 0x0B),     midi2::pack(0x0C, 0x0D, 0x0E, 0x0F),
      end_message(6, 0x1A, 0x1B),    midi2::pack(0x1C, 0x1D, 0x1E, 0x1F),
      start_message(0x2A, 0x2B),     midi2::pack(0x2C, 0x2D, 0x2E, 0x2F),
      end_message(5, 0x3A, 0x3B),    midi2::pack(0x3C, 0x3D, 0x3E, 0),
      in_one_message(5, 0x4A, 0x4B), midi2::pack(0x4C, 0x4D, 0x4E, 0),
      in_one_message(4, 0x5A, 0x5B), midi2::pack(0x5C, 0x5D, 0, 0),
      in_one_message(3, 0x6A, 0x6B), midi2::pack(0x6C, 0, 0, 0),
      in_one_message(2, 0x7A, 0x7B), midi2::pack(0, 0, 0, 0),
  };

  auto const actual = convert(midi2::bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, MT4NoteOnWithRunningStatus) {
  std::array const input{std::uint8_t{0x81}, std::uint8_t{0x60},
                         std::uint8_t{0x50}, std::uint8_t{0x70},
                         std::uint8_t{0x70}};
  EXPECT_THAT(convert(midi2::bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40816000), UINT32_C(0xA0820000),
                          UINT32_C(0x40817000), UINT32_C(0xe1860000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, MT4ProgramChangeTwoBytes) {
  std::array const input{std::uint8_t{0xC6}, std::uint8_t{0x40}};
  EXPECT_THAT(convert(midi2::bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40C60000), UINT32_C(0x40000000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, MT4PC2BytesWithBankMsbLsb) {
  std::array const input{std::uint8_t{0xB6}, std::uint8_t{0x00},
                         std::uint8_t{0x01}, std::uint8_t{0x20},
                         std::uint8_t{0x0A}, std::uint8_t{0xC6},
                         std::uint8_t{0x41}};
  EXPECT_THAT(convert(midi2::bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40C60001), UINT32_C(0x4100010A)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2RPN) {
  constexpr auto channel = std::uint8_t{0x0F};  // 4 bits
  constexpr auto msb = std::uint8_t{0x00};      // 7 bits
  constexpr auto lsb = std::uint8_t{0x06};      // 7 bits
  constexpr auto de = std::uint8_t{0x7F};       // Data entry: 7 bits

  // "The basic procedure for altering a parameter value is to first send the
  // Registered or Non-Registered Parameter Number corresponding to the
  // parameter to be modified, followed by the Data Entry, Data Increment, or
  // Data Decrement value to be applied to the parameter."
  //
  // B6 65 (msb)  set RPN MSB
  // B6 64 (lsb)  set RPN LSB
  // B6 06 (de)   data entry.

  std::array const input{// Set RPN MSB
                         std::uint8_t{midi2::status::cc | channel},
                         std::uint8_t{midi2::control::rpn_msb}, msb,
                         // (running status) Set RPN LSB
                         std::uint8_t{midi2::control::rpn_lsb}, lsb,
                         // (running status) Set data
                         std::uint8_t{midi2::control::data_entry_msb}, de};

  constexpr auto message_type =
      static_cast<std::uint32_t>(midi2::ump_message_type::m2cvm);  // 4 bits
  constexpr auto group = std::uint32_t{0};                  // 4 bits
  constexpr auto bank = std::uint32_t{msb};                 // 7 bits
  constexpr auto index = std::uint32_t{lsb};                // 7 bits
  constexpr auto data = std::uint32_t{(midi2::scaleUp(de << 7, 14, 32))};

  std::array const expected{
      std::uint32_t{(message_type << 28) | (group << 24) |
                    ((midi2::midi2status::rpn | channel) << 16) | (bank << 8) |
                    index},
      data,
  };

  auto const actual = convert(midi2::bytestreamToUMP{true}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2RPNWithLSB) {
  std::array const input{
    std::uint8_t{0xB0}, std::uint8_t{0x64}, std::uint8_t{0x00}, // RPN (LSB)
    std::uint8_t{0xB0}, std::uint8_t{0x65}, std::uint8_t{0x00}, // RPN (MSB)

    std::uint8_t{0xB0}, std::uint8_t{0x06}, std::uint8_t{0x02}, // Data entry (MSB)
    std::uint8_t{0xB0}, std::uint8_t{0x26}, std::uint8_t{0x03}, // Data entry (LSB)

    // End of the controller sequence
    std::uint8_t{0xB0}, std::uint8_t{0x64}, std::uint8_t{0x7F},
    std::uint8_t{0xB0}, std::uint8_t{0x65}, std::uint8_t{0x7F},
  };

  std::array const expected{
      std::uint32_t{0x40200000},
      std::uint32_t{0x04000000},
      std::uint32_t{0x40200000},
      std::uint32_t{0xFE0FF07F},
  };

  auto const actual = convert(midi2::bytestreamToUMP{true}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2NonRegisteredParameterSetMSBAndLSB) {
  constexpr auto channel = std::uint8_t{0x0F};   // 4 bits
  constexpr auto msb = std::uint8_t{0x00};       // 7 bits
  constexpr auto lsb = std::uint8_t{0x06};       // 7 bits
  constexpr auto data_msb = std::uint8_t{0x12};  // Data entry: 7 bits
  constexpr auto data_lsb = std::uint8_t{0x34};  // Data entry: 7 bits

  std::array const input{
      // Set NRPN MSB
      std::uint8_t{midi2::status::cc | channel},
      std::uint8_t{midi2::control::nrpn_msb},
      msb,
      // (running status) Set NRPN LSB
      std::uint8_t{midi2::control::nrpn_lsb},
      lsb,
      // Set data MSB
      std::uint8_t{midi2::control::data_entry_msb},
      data_msb,
      // Set data LSB
      std::uint8_t{midi2::control::data_entry_lsb},
      data_lsb,
  };

  constexpr auto message_type =
      static_cast<std::uint32_t>(midi2::ump_message_type::m2cvm);  // 4 bits
  constexpr auto group = std::uint32_t{0};                  // 4 bits
  constexpr auto bank = std::uint32_t{msb};                 // 7 bits
  constexpr auto index = std::uint32_t{lsb};                // 7 bits

  std::array const expected{
      std::uint32_t{(message_type << 28) | (group << 24) |
                    ((midi2::midi2status::nrpn | channel) << 16) | (bank << 8) |
                    index},
      midi2::scaleUp((std::uint32_t{data_msb} << 7) | data_lsb, 14, 32),
  };

  auto const actual = convert(midi2::bytestreamToUMP{true}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi1BadDataTwoNoteOffs) {
  std::array const input{std::uint8_t{0x80}, std::uint8_t{0x80}};
  EXPECT_THAT(convert(midi2::bytestreamToUMP{}, input), IsEmpty());
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2BadDataTwoNoteOffs) {
  std::array const input{std::uint8_t{0x80}, std::uint8_t{0x80}};
  EXPECT_THAT(convert(midi2::bytestreamToUMP{true}, input), IsEmpty());
}

// This group of tests uses a bytestream which starts with one of the
// reserved status byte values and followed by two 0 bytes to act as
// dummy payload/parameters. We then have a standard note-on message.
//
// The tests expect the unknown status codes and two following bytes
// to be ignored. The output should be a single note-on message.
class BytestreamToUMPReserved : public TestWithParam<std::uint8_t> {
protected:
  static constexpr auto note_number_ = std::uint8_t{60};
  static constexpr auto velocity_ = std::uint8_t{127};
  static constexpr auto channel_ = std::uint8_t{1};

  [[nodiscard]] static auto input() {
    return std::array{// a normal note-on message
                      static_cast<std::uint8_t>(midi2::status::note_on | channel_), note_number_, velocity_,
                      //
                      BytestreamToUMPReserved::GetParam(),  // one of the reserved status codes
                      std::uint8_t{0x01},                   // two bytes to be ignored
                      std::uint8_t{0x02}, std::uint8_t{0x03},
                      // a normal note-off message
                      static_cast<std::uint8_t>(midi2::status::note_off | channel_), note_number_, velocity_};
  }
};

TEST_P(BytestreamToUMPReserved, Midi1ReservedStatusCodeThenNoteOn) {
  constexpr auto group = std::uint32_t{0};
  constexpr auto message_type = std::uint32_t{2};

  std::array const expected{
      std::uint32_t{(message_type << 28) | (group << 24) | (channel_ << 16) |
                    (ump_note_on << 20) | (std::uint32_t{note_number_} << 8) |
                    (static_cast<std::uint32_t>(velocity_))},

      std::uint32_t{(message_type << 28) | (group << 24) | (channel_ << 16) |
                    (ump_note_off << 20) | (std::uint32_t{note_number_} << 8) |
                    (static_cast<std::uint32_t>(velocity_))}};

  auto const input = BytestreamToUMPReserved::input();
  auto const actual = convert(midi2::bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << "Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

TEST_P(BytestreamToUMPReserved, Midi2ReservedStatusCodeThenNoteOn) {
  constexpr auto message_type = std::uint32_t{4};
  constexpr auto group = std::uint32_t{0};
  constexpr auto attribute_type = std::uint32_t{0};  // 0 = no attribute data
  constexpr auto attribute = std::uint32_t{0};

  std::array const expected{
      std::uint32_t{(message_type << 28) | (group << 24) | (channel_ << 16) |
                    (ump_note_on << 20) | (std::uint32_t{note_number_} << 8) |
                    attribute_type},
      std::uint32_t{(midi2::scaleUp(velocity_, 7, 16) << 16) | attribute},

      std::uint32_t{(message_type << 28) | (group << 24) | (channel_ << 16) |
                    (ump_note_off << 20) | (std::uint32_t{note_number_} << 8) |
                    attribute_type},
      std::uint32_t{(midi2::scaleUp(velocity_, 7, 16) << 16) | attribute}};
  auto const input = BytestreamToUMPReserved::input();
  auto const actual = convert(midi2::bytestreamToUMP{true}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << "Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

INSTANTIATE_TEST_SUITE_P(ReservedStatusCodes, BytestreamToUMPReserved,
                         testing::Values(midi2::status::reserved1,
                                         midi2::status::reserved2,
                                         midi2::status::reserved3,
                                         midi2::status::reserved4));

void NeverCrashes(std::vector<std::uint8_t> const& bytes) {
  // This test simply gets bytestreamToUMP to consume a random buffer.
  midi2::bytestreamToUMP bs2ump;
  for (auto const b : bytes) {
    bs2ump.bytestreamParse(b);
    while (bs2ump.availableUMP()) {
      (void)bs2ump.readUMP();
    }
  }
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(BytestreamToUMPFuzz, NeverCrashes);
#endif
TEST(BytestreamToUMPFuzz, Empty) {
  NeverCrashes({});
}

}  // end anonymous namespace
