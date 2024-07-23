// DUT
#include "bytestreamToUMP.h"

// Standard library
#include <algorithm>
#include <array>
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
  ArrayLike const& container;
};
template <typename ArrayLike>
HexContainer(ArrayLike const&) -> HexContainer<ArrayLike>;

template <typename ArrayLike>
std::ostream& operator<<(std::ostream& os, HexContainer<ArrayLike> const& hc) {
  auto const* separator = "";
  for (auto v : hc.container) {
    os << separator << "0x" << std::hex << std::uppercase
       << static_cast<unsigned>(v);
    separator = ", ";
  }
  return os;
}

template <std::size_t Size>
auto convert(bytestreamToUMP&& bs2ump,
             std::array<std::uint8_t, Size> const& input) {
  std::vector<std::uint32_t> output;
  for (std::uint8_t const b : input) {
    bs2ump.bytestreamParse(b);
    while (bs2ump.availableUMP()) {
      output.push_back(bs2ump.readUMP());
    }
  }
  return output;
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, NoteOnWithRunningStatus) {
  std::array const input{std::uint8_t{0x81}, std::uint8_t{0x60},
                         std::uint8_t{0x50}, std::uint8_t{0x70},
                         std::uint8_t{0x70}};
  auto const actual = convert(bytestreamToUMP{}, input);
  std::array const expected{std::uint32_t{0x20816050},
                            std::uint32_t{0x20817070}};
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << "Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SeqStartMidNoteOn) {
  constexpr auto channel = std::uint8_t{1};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint8_t{127};

  constexpr auto ump_note_on = std::uint32_t{0b1001};
  constexpr auto group = std::uint32_t{0};

  std::array const input{static_cast<std::uint8_t>(status::note_on | channel),
                         std::uint8_t{status::seqstart}, note_number, velocity};
  std::array const expected{
      std::uint32_t{(1U << 28) | (group << 24) | (status::seqstart << 16)},
      std::uint32_t{(2U << 28) | (group << 24) | (channel << 16) |
                    (ump_note_on << 20) | (std::uint32_t{note_number} << 8) |
                    std::uint32_t{velocity}}};

  auto const actual = convert(bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << "Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SystemMessageOneByte) {
  std::array const input{std::uint8_t{0xF8}};
  EXPECT_THAT(convert(bytestreamToUMP{}, input),
              ElementsAre(UINT32_C(0x10f80000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, PCTwoBytes) {
  std::array<uint8_t, 2> const input{0xC6, 0x40};
  EXPECT_THAT(convert(bytestreamToUMP{}, input),
              ElementsAre(UINT32_C(0x20c64000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, SysEx) {
  std::array const input{
      std::uint8_t{0xF0}, std::uint8_t{0x7E}, std::uint8_t{0x7F},
      std::uint8_t{0x0D}, std::uint8_t{0x70}, std::uint8_t{0x02},
      std::uint8_t{0x4B}, std::uint8_t{0x60}, std::uint8_t{0x7A},
      std::uint8_t{0x73}, std::uint8_t{0x7F}, std::uint8_t{0x7F},
      std::uint8_t{0x7F}, std::uint8_t{0x7F}, std::uint8_t{0x7D},
      std::uint8_t{0x00}, std::uint8_t{0x00}, std::uint8_t{0x00},
      std::uint8_t{0x00}, std::uint8_t{0x01}, std::uint8_t{0x00},
      std::uint8_t{0x00}, std::uint8_t{0x00}, std::uint8_t{0x03},
      std::uint8_t{0x00}, std::uint8_t{0x00}, std::uint8_t{0x00},
      std::uint8_t{0x10}, std::uint8_t{0x00}, std::uint8_t{0x00},
      std::uint8_t{0x00}, std::uint8_t{0xF7}};
  EXPECT_THAT(convert(bytestreamToUMP{}, input),
              ElementsAre(UINT32_C(0x30167e7f), UINT32_C(0x0d70024b),
                          UINT32_C(0x3026607a), UINT32_C(0x737f7f7f),
                          UINT32_C(0x30267f7d), UINT32_C(0x00000000),
                          UINT32_C(0x30260100), UINT32_C(0x00000300),
                          UINT32_C(0x30360000), UINT32_C(0x10000000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, MT4NoteOnWithRunningStatus) {
  std::array const input{std::uint8_t{0x81}, std::uint8_t{0x60},
                         std::uint8_t{0x50}, std::uint8_t{0x70},
                         std::uint8_t{0x70}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40816000), UINT32_C(0xA0820000),
                          UINT32_C(0x40817000), UINT32_C(0xe1860000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, MT4PCTwoBytes) {
  std::array const input{std::uint8_t{0xC6}, std::uint8_t{0x40}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40c60000), UINT32_C(0x40000000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, MT4PC2BytesWithBankMsbLsb) {
  std::array const input{std::uint8_t{0xB6}, std::uint8_t{0x00},
                         std::uint8_t{0x01}, std::uint8_t{0x20},
                         std::uint8_t{0x0A}, std::uint8_t{0xC6},
                         std::uint8_t{0x41}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40c60001), UINT32_C(0x4100010A)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, MT4RPN) {
  std::array const input{std::uint8_t{0xB6}, std::uint8_t{0x65},
                         std::uint8_t{0x00}, std::uint8_t{0x64},
                         std::uint8_t{0x06}, std::uint8_t{0x06},
                         std::uint8_t{0x08}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40260006), UINT32_C(0x10000000)));
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi1TwoNoteOffs) {
  std::array const input{std::uint8_t{0x80}, std::uint8_t{0x80}};
  EXPECT_THAT(convert(bytestreamToUMP{}, input), IsEmpty());
}

// NOLINTNEXTLINE
TEST(BytestreamToUMP, Midi2TwoNoteOffs) {
  std::array const input{std::uint8_t{0x80}, std::uint8_t{0x80}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input), IsEmpty());
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

  std::array<std::uint8_t, 7> input() const {
    return {this->GetParam(),    // one of the reserved status codes
            std::uint8_t{0x01},  // two bytes to be ignored
            std::uint8_t{0x02}, std::uint8_t{0x03},
            // a normal note-on message
            static_cast<std::uint8_t>(status::note_on | channel_), note_number_,
            velocity_};
  }
};

TEST_P(BytestreamToUMPReserved, Midi1StatusCodeThenNoteOn) {
  constexpr auto ump_note_on = std::uint32_t{0b1001};
  constexpr auto group = std::uint32_t{0};
  constexpr auto message_type = std::uint32_t{2};

  std::array const expected{static_cast<std::uint32_t>(
      (message_type << 28) | (group << 24) | (channel_ << 16) |
      (ump_note_on << 20) | (static_cast<std::uint32_t>(note_number_) << 8) |
      (static_cast<std::uint32_t>(velocity_)))};

  auto const input = this->input();
  auto const actual = convert(bytestreamToUMP{}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << "Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

TEST_P(BytestreamToUMPReserved, Midi2StatusCodeThenNoteOn) {
  constexpr auto message_type = std::uint32_t{4};
  constexpr auto group = std::uint32_t{0};
  constexpr auto ump_note_on = std::uint32_t{0b1001};
  constexpr auto attribute_type = std::uint32_t{0};  // 0 = no attribute data
  constexpr auto attribute = std::uint32_t{0};

  std::array const expected{
      std::uint32_t{(message_type << 28) | (group << 24) | (channel_ << 16) |
                    (ump_note_on << 20) |
                    (static_cast<std::uint32_t>(note_number_) << 8) |
                    attribute_type},
      std::uint32_t{(M2Utils::scaleUp(velocity_, 7, 16) << 16) | attribute}};
  auto const input = this->input();
  auto const actual = convert(bytestreamToUMP{true}, input);
  EXPECT_THAT(actual, ElementsAreArray(expected))
      << "Input: " << HexContainer(input)
      << "\n Actual: " << HexContainer(actual)
      << "\n Expected: " << HexContainer(expected);
}

INSTANTIATE_TEST_SUITE_P(ReservedStatusCodes, BytestreamToUMPReserved,
                         testing::Values(status::reserved1, status::reserved2,
                                         status::reserved3, status::reserved4));

void NeverCrashes(std::vector<std::uint8_t> const& bytes) {
  // This test simply gets bytestreamToUMP to consume a random buffer.
  bytestreamToUMP bs2ump;
  for (auto const b : bytes) {
    bs2ump.bytestreamParse(b);
    while (bs2ump.availableUMP()) {
      bs2ump.readUMP();
    }
  }
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(BytestreamToUMPFuzz, NeverCrashes);
#endif

}  // end anonymous namespace
