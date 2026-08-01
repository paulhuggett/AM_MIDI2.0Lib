//===-- Bytestream To UMP -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/bytestream/bytestream_to_ump.hpp"
#include "midi2/bytestream/bytestream_types.hpp"
#include "midi2/ump/ump_types.hpp"
#include "midi2/utils.hpp"

// Standard library
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <format>
#include <ostream>
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

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace midi2::literals;

template <typename ArrayLike> struct HexContainer {
  constexpr explicit HexContainer(ArrayLike const& container_) : container{&container_} {}

  friend std::ostream& operator<<(std::ostream& os, HexContainer const& hc) {
    auto const* separator = "";
    for (auto v : *hc.container) {
      os << std::format("{}0x{:x}", separator, static_cast<unsigned>(v));
      separator = ", ";
    }
    return os;
  }

  ArrayLike const* container;
};
template <typename ArrayLike> HexContainer(ArrayLike const&) -> HexContainer<ArrayLike>;

template <std::size_t Size>
std::vector<std::uint32_t> convert(midi2::bytestream::to_ump bs2ump, std::array<std::byte, Size> const& input) {
  std::vector<std::uint32_t> output;
  for (std::byte const b : input) {
    bs2ump.push(b);
    while (!bs2ump.empty()) {
      output.push_back(bs2ump.pop());
    }
  }
  return output;
}

constexpr std::uint32_t ump_cvm(midi2::bytestream::status s) {
  static_assert(std::is_same_v<std::underlying_type_t<midi2::bytestream::status>, std::uint8_t>,
                "status type must be a std::uint8_t");
  assert((std::to_underlying(s) & 0x0F) == 0 && "Bottom 4 bits of a channel voice message status enum  must be 0");
  return std::uint32_t{std::to_underlying(s)} >> 4;
}

constexpr auto ump_note_on = ump_cvm(midi2::bytestream::status::note_on);
constexpr auto ump_note_off = ump_cvm(midi2::bytestream::status::note_off);
constexpr auto ump_pitch_bend = ump_cvm(midi2::bytestream::status::pitch_bend);
constexpr auto ump_control_change = ump_cvm(midi2::bytestream::status::cc);
constexpr auto ump_program_change = ump_cvm(midi2::bytestream::status::program_change);

// NOLINTNEXTLINE
TEST(BytestreamToUMP, NoteOnWithRunningStatus) {
  constexpr std::array input{0x81_b, 0x60_b, 0x50_b, 0x70_b, 0x70_b};
  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  constexpr std::array expected{std::uint32_t{0x20816050}, std::uint32_t{0x20817070}};
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, NoteOnImplicitNoteOffWithRunningStatus) {
  constexpr auto channel = std::byte{3};
  constexpr auto note_number = std::byte{60};
  constexpr auto velocity = std::byte{127};

  constexpr auto group = std::uint32_t{0};

  // A note on message followed by a note-on with velocity 0. The second of
  // these should be treated as a note-off. Running status is used for the two
  // input messages.
  constexpr std::array input{std::byte{static_cast<std::byte>(midi2::bytestream::status::note_on) | channel},
                             note_number, velocity, note_number, std::byte{0U}};

  constexpr auto m0 =
      std::uint32_t{(2U << 28) | (group << 24) | (ump_note_on << 20) | (std::to_integer<std::uint32_t>(channel) << 16) |
                    (std::to_integer<std::uint32_t>(note_number) << 8) | std::to_integer<std::uint32_t>(velocity)};
  constexpr auto m1 =
      std::uint32_t{(2U << 28) | (group << 24) | (ump_note_on << 20) | (std::to_integer<std::uint32_t>(channel) << 16) |
                    (std::to_integer<std::uint32_t>(note_number) << 8) | std::uint32_t{0x00}};
  constexpr std::array expected{m0, m1};
  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

TEST(BytestreamToUMP, Reset) {
  midi2::bytestream::to_ump bs2ump;
  bs2ump.set_group(7U);
  bs2ump.push(static_cast<std::byte>(midi2::bytestream::status::sysex_start));
  bs2ump.reset();

  ASSERT_TRUE(bs2ump.empty());

  bs2ump.push(static_cast<std::byte>(midi2::bytestream::status::note_on));
  ASSERT_TRUE(bs2ump.empty());
  bs2ump.push(std::byte{0x60U});
  ASSERT_TRUE(bs2ump.empty());
  bs2ump.push(std::byte{0x50U});

  ASSERT_FALSE(bs2ump.empty());
  EXPECT_EQ(bs2ump.pop(), std::uint32_t{0x20906050});
  EXPECT_TRUE(bs2ump.empty());
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi1ChannelPressure) {
  constexpr auto channel = std::byte{5};    // 4 bits
  constexpr auto pressure = std::byte{57};  // 7 bits

  // A MIDI 1 bytestream channel pressure message.
  constexpr std::array input{midi2::to_byte(midi2::bytestream::status::channel_pressure) | channel, pressure};

  // Build an equivalent UMP message.
  std::vector<std::uint32_t> expected;
  midi2::ump::apply(
      midi2::ump::m1cvm::channel_pressure().channel(std::to_underlying(channel)).data(std::to_underlying(pressure)),
      [&expected](std::uint32_t const v) {
        expected.push_back(v);
        return false;
      });

  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, PitchBend) {
  constexpr auto bend_lsb = 0x00_b;
  constexpr auto bend_msb = 0x40_b;
  constexpr auto channel = std::byte{3};
  constexpr std::array input{static_cast<std::byte>(midi2::bytestream::status::pitch_bend) | channel, bend_lsb,
                             bend_msb};

  constexpr auto message_type = static_cast<std::uint32_t>(std::to_underlying(midi2::ump::message_type::m1cvm));
  constexpr auto group = std::uint32_t{0};

  constexpr std::array expected{std::uint32_t{
      (message_type << 28) | (group << 24) | (ump_pitch_bend << 20) | (std::to_integer<std::uint32_t>(channel) << 16) |
      (std::to_integer<std::uint32_t>(bend_lsb) << 8) | std::to_integer<std::uint32_t>(bend_msb)}};

  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SeqStartMidNoteOn) {
  constexpr auto channel = std::byte{1};
  constexpr auto note_number = std::byte{60};
  constexpr auto velocity = std::byte{127};

  // A real-time message can appear anywhere, even in the middle of another
  // multibyte message.
  constexpr std::array input{static_cast<std::byte>(midi2::bytestream::status::note_on) | channel,
                             static_cast<std::byte>(midi2::bytestream::status::sequence_start), note_number, velocity};

  constexpr auto group = std::uint32_t{0};
  constexpr std::array expected{
      std::uint32_t{(1U << 28) | (group << 24) |
                    (std::uint32_t{std::to_underlying(midi2::bytestream::status::sequence_start)} << 16)},
      std::uint32_t{(2U << 28) | (group << 24) | (std::to_integer<std::uint32_t>(channel) << 16) | (ump_note_on << 20) |
                    (std::to_integer<std::uint32_t>(note_number) << 8) | std::to_integer<std::uint32_t>(velocity)}};

  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SystemMessageOneByte) {
  constexpr std::array input{0xF8_b};
  EXPECT_THAT(convert(midi2::bytestream::to_ump{}, input), ElementsAre(UINT32_C(0x10f80000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, BankAndProgramChange) {
  constexpr auto channel = 0x0F_b;   // 4 bits
  constexpr auto program = 0x42_b;   // 8 bits
  constexpr auto bank_msb = 0x51_b;  // 8 bits
  constexpr auto bank_lsb = 0x01_b;  // 8 bits

  constexpr auto controller_set_msb = 0x00_b;
  constexpr auto controller_set_lsb = 0x20_b;

  constexpr auto cc = midi2::bytestream::status::cc;
  constexpr auto program_change = midi2::bytestream::status::program_change;

  constexpr std::array input{// MSB (Coarse) Bank Select
                             static_cast<std::byte>(cc) | channel, controller_set_msb, bank_msb,
                             // LSB (Fine) Bank Select
                             static_cast<std::byte>(cc) | channel, controller_set_lsb, bank_lsb,
                             // Program Change
                             static_cast<std::byte>(program_change) | channel, program};

  constexpr auto message_type =
      static_cast<std::uint32_t>(std::to_underlying(midi2::ump::message_type::m1cvm));  // 4 bits
  constexpr auto group = std::uint32_t{0x00};                                           // 4 bits

  constexpr std::array expected{
      // MSB (Coarse) Bank Select
      std::uint32_t{(message_type << 28) | (group << 24) | (ump_control_change << 20) |
                    (std::to_integer<uint32_t>(channel) << 16) | (std::to_integer<uint32_t>(controller_set_msb) << 8) |
                    std::to_integer<std::uint32_t>(bank_msb)},
      // LSB (Fine) Bank Select
      std::uint32_t{(message_type << 28) | (group << 24) | (ump_control_change << 20) |
                    (std::to_integer<std::uint32_t>(channel) << 16) |
                    (std::to_integer<std::uint32_t>(controller_set_lsb) << 8) |
                    std::to_integer<std::uint32_t>(bank_lsb)},
      // Program Change
      std::uint32_t{(message_type << 28) | (group << 24) | (ump_program_change << 20) |
                    (std::to_integer<std::uint32_t>(channel) << 16) | (std::to_integer<std::uint32_t>(program) << 8)}};

  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, ProgramChangeTwoBytes) {
  constexpr std::array input{0xC6_b, 0x40_b};
  EXPECT_THAT(convert(midi2::bytestream::to_ump{}, input), ElementsAre(UINT32_C(0x20C64000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SysEx) {
  using b8 = std::byte;
  constexpr auto start = static_cast<b8>(std::to_underlying(midi2::bytestream::status::sysex_start));
  constexpr auto stop = static_cast<b8>(std::to_underlying(midi2::bytestream::status::sysex_stop));
  constexpr std::array input{start,    b8{0x7E}, b8{0x7F}, b8{0x0D}, b8{0x70}, b8{0x02}, b8{0x4B}, b8{0x60},
                             b8{0x7A}, b8{0x73}, b8{0x7F}, b8{0x7F}, b8{0x7F}, b8{0x7F}, b8{0x7D}, b8{0x00},
                             b8{0x00}, b8{0x00}, b8{0x00}, b8{0x01}, b8{0x00}, b8{0x00}, b8{0x00}, b8{0x03},
                             b8{0x00}, b8{0x00}, b8{0x00}, b8{0x10}, b8{0x00}, b8{0x00}, b8{0x00}, stop};
  constexpr std::array expected{std::uint32_t{0x30167E7F}, std::uint32_t{0x0D70024B}, std::uint32_t{0x3026607A},
                                std::uint32_t{0x737F7F7F}, std::uint32_t{0x30267F7D}, std::uint32_t{0x00000000},
                                std::uint32_t{0x30260100}, std::uint32_t{0x00000300}, std::uint32_t{0x30360000},
                                std::uint32_t{0x10000000}};
  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input) << "\n Actual:   " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, LonelySysExEnd) {
  using b8 = std::byte;
  constexpr auto stop = static_cast<b8>(std::to_underlying(midi2::bytestream::status::sysex_stop));
  constexpr std::array input{stop};
  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  EXPECT_THAT(actual, IsEmpty()) << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SysExEndFollowedByDataBytes) {
  using b8 = std::byte;
  constexpr auto stop = static_cast<b8>(std::to_underlying(midi2::bytestream::status::sysex_stop));
  constexpr std::array input{stop, b8{1}, b8{2}, stop};
  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  EXPECT_THAT(actual, IsEmpty()) << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual);
}
// NOLINTNEXTLINE
TEST(BytestreamToUMP, MissingSysExEnd) {
  using b8 = std::byte;
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{1};
  constexpr auto start = static_cast<b8>(std::to_underlying(midi2::bytestream::status::sysex_start));
  constexpr auto note_off = static_cast<b8>(std::to_underlying(midi2::bytestream::status::note_off));
  constexpr auto note_number = std::uint8_t{62};

  constexpr std::array input{start,           b8{1}, b8{2}, b8{3}, b8{4}, b8{5}, b8{6}, b8{7}, note_off | b8{channel},
                             b8{note_number}, b8{0}};

  std::vector<std::uint32_t> expected;
  auto const expect = [&expected](auto const& message) {
    midi2::ump::apply(message, [&](std::uint32_t const v) {
      expected.push_back(v);
      return false;
    });
  };
  expect(midi2::ump::data64::sysex7_start{}.group(group).data({1U, 2U, 3U, 4U, 5U, 6U}));
  expect(midi2::ump::data64::sysex7_end{}.group(group).number_of_bytes(1).data0(7U));
  expect(midi2::ump::m1cvm::note_off{}.group(group).channel(channel).note(note_number).velocity(0));

  auto const actual = convert(midi2::bytestream::to_ump{group}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Actual: " << HexContainer(actual) << "\n Expected: " << HexContainer(expected);
}
// NOLINTNEXTLINE
TEST(BytestreamToUMP, MissingSysExEndBeforeStart) {
  using b8 = std::byte;
  using sysex7_in_1 = midi2::ump::data64::sysex7_in_1;
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{1};
  constexpr auto start = static_cast<b8>(std::to_underlying(midi2::bytestream::status::sysex_start));
  constexpr auto note_off = static_cast<b8>(std::to_underlying(midi2::bytestream::status::note_off));
  constexpr auto note_number = std::uint8_t{62};

  constexpr std::array input{
      start, b8{1}, b8{2}, b8{3}, start, b8{4}, b8{5}, b8{6}, b8{7}, note_off | b8{channel}, b8{note_number}, b8{0}};

  std::vector<std::uint32_t> expected;
  auto const expect = [&expected](auto const& message) {
    midi2::ump::apply(message, [&](std::uint32_t const v) {
      expected.push_back(v);
      return false;
    });
  };
  expect(sysex7_in_1{}.group(group).data({1U, 2U, 3U}));
  expect(sysex7_in_1{}.group(group).data({4U, 5U, 6U, 7U}));
  expect(midi2::ump::m1cvm::note_off{}.group(group).channel(channel).note(note_number));

  auto const actual = convert(midi2::bytestream::to_ump{group}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Actual: " << HexContainer(actual) << "\n Expected: " << HexContainer(expected);
}

[[nodiscard]] constexpr std::uint32_t pack(std::uint8_t const b0, std::uint8_t const b1, std::uint8_t const b2,
                                           std::uint8_t const b3) {
  return (std::uint32_t{b0} << 24) | (std::uint32_t{b1} << 16) | (std::uint32_t{b2} << 8) | std::uint32_t{b3};
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, MultipleSysExMessages) {
  using u8 = std::uint8_t;
  constexpr auto start = std::to_underlying(midi2::bytestream::status::sysex_start);
  constexpr auto stop = std::to_underlying(midi2::bytestream::status::sysex_stop);
  constexpr std::array input{
      static_cast<std::byte>(start),  // start sysex
      0x0A_b,
      0x0B_b,
      0x0C_b,
      0x0D_b,
      0x0E_b,
      0x0F_b,
      0x1A_b,
      0x1B_b,
      0x1C_b,
      0x1D_b,
      0x1E_b,
      0x1F_b,
      static_cast<std::byte>(stop),   // end sysex
      static_cast<std::byte>(start),  // start sysex
      0x2A_b,
      0x2B_b,
      0x2C_b,
      0x2D_b,
      0x2E_b,
      0x2F_b,
      0x3A_b,
      0x3B_b,
      0x3C_b,
      0x3D_b,
      0x3E_b,
      static_cast<std::byte>(stop),   // end sysex
      static_cast<std::byte>(start),  // start sysex
      0x4A_b,
      0x4B_b,
      0x4C_b,
      0x4D_b,
      0x4E_b,
      static_cast<std::byte>(stop),   // end sysex
      static_cast<std::byte>(start),  // start sysex
      0x5A_b,
      0x5B_b,
      0x5C_b,
      0x5D_b,
      static_cast<std::byte>(stop),   // end sysex
      static_cast<std::byte>(start),  // start sysex
      0x6A_b,
      0x6B_b,
      0x6C_b,
      static_cast<std::byte>(stop),   // end sysex
      static_cast<std::byte>(start),  // start sysex
      0x7A_b,
      0x7B_b,
      static_cast<std::byte>(stop),  // end sysex
  };

  constexpr auto group = std::uint8_t{0xF};
  auto const in_one_message = [](u8 number_of_bytes, u8 data0, u8 data1) constexpr {
    midi2::ump::data64::sysex7_in_1::word0 w0{};
    w0.template set<decltype(w0)::group>(group);
    w0.template set<decltype(w0)::number_of_bytes>(number_of_bytes);
    w0.template set<decltype(w0)::data0>(data0);
    w0.template set<decltype(w0)::data1>(data1);
    return std::uint32_t{w0};
  };
  auto const start_message = [](u8 data0, u8 data1) constexpr {
    midi2::ump::data64::sysex7_start::word0 w0{};
    w0.template set<decltype(w0)::group>(group);
    w0.template set<decltype(w0)::number_of_bytes>(6U);
    w0.template set<decltype(w0)::data0>(data0);
    w0.template set<decltype(w0)::data1>(data1);
    return std::uint32_t{w0};
  };
  auto const end_message = [](u8 number_of_bytes, u8 data0, u8 data1) constexpr {
    assert(number_of_bytes <= 6);
    midi2::ump::data64::sysex7_end::word0 w0{};
    w0.template set<decltype(w0)::group>(group);
    w0.template set<decltype(w0)::number_of_bytes>(number_of_bytes);
    w0.template set<decltype(w0)::data0>(data0);
    w0.template set<decltype(w0)::data1>(data1);
    return std::uint32_t{w0};
  };

  constexpr std::array expected{
      start_message(0x0A, 0x0B),     pack(0x0C, 0x0D, 0x0E, 0x0F),
      end_message(6, 0x1A, 0x1B),    pack(0x1C, 0x1D, 0x1E, 0x1F),
      start_message(0x2A, 0x2B),     pack(0x2C, 0x2D, 0x2E, 0x2F),
      end_message(5, 0x3A, 0x3B),    pack(0x3C, 0x3D, 0x3E, 0),
      in_one_message(5, 0x4A, 0x4B), pack(0x4C, 0x4D, 0x4E, 0),
      in_one_message(4, 0x5A, 0x5B), pack(0x5C, 0x5D, 0, 0),
      in_one_message(3, 0x6A, 0x6B), pack(0x6C, 0, 0, 0),
      in_one_message(2, 0x7A, 0x7B), pack(0, 0, 0, 0),
  };

  auto const actual = convert(midi2::bytestream::to_ump{group}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi1BadDataTwoNoteOffs) {
  constexpr std::array input{0x80_b, 0x80_b};
  EXPECT_THAT(convert(midi2::bytestream::to_ump{}, input), IsEmpty());
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2BadDataTwoNoteOffs) {
  constexpr std::array input{0x80_b, 0x80_b};
  EXPECT_THAT(convert(midi2::bytestream::to_ump{0}, input), IsEmpty());
}

TEST(BytestreamToUMP, MIdi1BadDuplicateEndSysex) {
  constexpr std::array input{0xF0_b, 0x21_b, 0x22_b, 0x23_b, 0x24_b, 0x25_b, 0xF7_b, 0xF7_b};
  constexpr auto output = midi2::ump::data64::sysex7_in_1{}.data({0x21U, 0x22U, 0x23U, 0x24U, 0x25U});
  std::vector<std::uint32_t> expected;
  midi2::ump::apply(output, [&expected](std::uint32_t const word) noexcept -> std::error_code {
    expected.push_back(word);
    return {};
  });
  EXPECT_THAT(convert(midi2::bytestream::to_ump{}, input), expected);
}

// This group of tests uses a bytestream which starts with one of the
// reserved status byte values and followed by two 0 bytes to act as
// dummy payload/parameters. We then have a standard note-on message.
//
// The tests expect the unknown status codes and two following bytes
// to be ignored. The output should be a single note-on message.
class BytestreamToUMPReserved : public TestWithParam<std::uint8_t> {
protected:
  static constexpr auto note_number_ = 0x3C_b;
  static constexpr auto velocity_ = 0x7F_b;
  static constexpr auto channel_ = 1_b;

  [[nodiscard]] static auto input() {
    return std::array{// a normal note-on message
                      static_cast<std::byte>(midi2::bytestream::status::note_on) | channel_, note_number_, velocity_,
                      static_cast<std::byte>(BytestreamToUMPReserved::GetParam()),  // one of the reserved status codes
                      0x01_b,                                                       // three bytes to be ignored
                      0x02_b, 0x03_b,
                      // a normal note-off message
                      static_cast<std::byte>(midi2::bytestream::status::note_off) | channel_, note_number_, velocity_};
  }
};

TEST_P(BytestreamToUMPReserved, Midi1ReservedStatusCodeThenNoteOn) {
  constexpr auto group = std::uint32_t{0};
  constexpr auto message_type = std::uint32_t{2};

  constexpr std::array expected{
      std::uint32_t{(message_type << 28) | (group << 24) | (std::to_integer<std::uint32_t>(channel_) << 16) |
                    (ump_note_on << 20) | (std::to_integer<std::uint32_t>(note_number_) << 8) |
                    (std::to_integer<std::uint32_t>(velocity_))},

      std::uint32_t{(message_type << 28) | (group << 24) | (std::to_integer<std::uint32_t>(channel_) << 16) |
                    (ump_note_off << 20) | (std::to_integer<std::uint32_t>(note_number_) << 8) |
                    (std::to_integer<std::uint32_t>(velocity_))}};

  auto const input = BytestreamToUMPReserved::input();
  auto const actual = convert(midi2::bytestream::to_ump{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << "Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

INSTANTIATE_TEST_SUITE_P(ReservedStatusCodes, BytestreamToUMPReserved,
                         testing::Values(midi2::bytestream::status::reserved1, midi2::bytestream::status::reserved2,
                                         midi2::bytestream::status::reserved3, midi2::bytestream::status::reserved4));

class BytestreamToUMPGroups : public TestWithParam<std::uint8_t> {};

TEST_P(BytestreamToUMPGroups, NoteOnWithRunningStatus) {
  constexpr std::array input{std::byte{0x81U}, std::byte{0x60U}, std::byte{0x50U}, std::byte{0x70U}, std::byte{0x70U}};

  midi2::bytestream::to_ump converter;
  auto const group = GetParam();
  converter.set_group(group);

  auto const actual = convert(converter, input);
  ASSERT_LT(group, 8U);
  auto const group32 = static_cast<std::uint32_t>(group);
  std::array const expected{std::uint32_t{0x20816050U} | std::uint32_t{group32 << 24},
                            std::uint32_t{0x20817070U} | std::uint32_t{group32 << 24}};
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << " Input: " << HexContainer(input) << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

INSTANTIATE_TEST_SUITE_P(BytestreamToUMPGroups, BytestreamToUMPGroups,
                         testing::Range(std::uint8_t{0U}, std::uint8_t{8U}));

void NeverCrashes(std::vector<std::byte> const& bytes) {
  // This test simply gets bytestream_to_ump to consume a random buffer.
  midi2::bytestream::to_ump bs2ump;
  for (auto const b : bytes) {
    bs2ump.push(b);
    while (!bs2ump.empty()) {
      (void)bs2ump.pop();
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
TEST(BytestreamToUMPFuzz, OneByte) {
  NeverCrashes({static_cast<std::byte>(std::to_underlying(midi2::bytestream::status::tune_request))});
}

}  // end anonymous namespace
