//===-- USB MIDI1.0 to Bytestream ---------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/bytestream/usbm1_to_bytestream.hpp"

// Standard library
#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <vector>

// Google Test/Mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::ElementsAre;
using testing::IsEmpty;

namespace {

template <std::ranges::input_range Range> constexpr auto convert(std::uint8_t const cable, Range const& range) {
  std::vector<std::byte> output;
  midi2::bytestream::usbm1_to_bytestream m1_to_bs{cable};
  for (std::uint32_t const m1 : range) {
    m1_to_bs.push(m1);
    while (!m1_to_bs.empty()) {
      output.push_back(m1_to_bs.pop());
    }
  }
  return output;
}

//| Description                                           | MIDI_ver. 1.0  | Event packet |
//| ----------------------------------------------------- | -------------- | ------------ |
//| Note-on message on virtual cable 1 (CN=0x1; CIN=0x9)  | 9n kk vv       | 19 9n kk vv  |
// NOLINTNEXTLINE
TEST(USBM1ToByteStream, NoteOnCable1) {
  constexpr auto cable = 0x1U;
  constexpr auto kk = 0x46U;
  constexpr auto vv = 0x3FU;

  constexpr auto events = std::array{std::uint32_t{(cable << 28) | (9U << 24) | (0x90 << 16) | (kk << 8) | vv}};
  auto const actual = convert(cable, events);
  EXPECT_THAT(actual, ElementsAre(std::byte{0x90}, std::byte{kk}, std::byte{vv}));
  EXPECT_THAT(convert(0, events), IsEmpty());
}

//| Description                                            | MIDI_ver. 1.0  | Event packet |
//| ------------------------------------------------------ | -------------- | ------------ |
//| Control change message on cable 10 (CN=0xA; CIN=0xB)   | Bn pp vv       | AB Bn pp vv  |
// NOLINTNEXTLINE
TEST(USBM1ToByteStream, ControlChangeCable10) {
  constexpr auto cable = 0xAU;
  constexpr auto pp = 0x46U;
  constexpr auto vv = 0x3FU;

  constexpr auto events = std::array{std::uint32_t{(cable << 28) | (0xBU << 24) | (0xB0 << 16) | (pp << 8) | vv}};
  auto const actual = convert(cable, events);
  EXPECT_THAT(actual, ElementsAre(std::byte{0xB0}, std::byte{pp}, std::byte{vv}));
}

//| Description                                        | MIDI_ver. 1.0  | Event packet |
//| -------------------------------------------------- | -------------- | ------------ |
//| Real-time message F8 on cable 3 (CN=0x3; CIN=0xF)  | F8 xx xx       | 3F F8 xx xx  |
// NOLINTNEXTLINE
TEST(USBM1ToByteStream, TimingClockCable3) {
  constexpr auto cable = 0x3U;

  constexpr auto events = std::array{std::uint32_t{(cable << 28) | (0xFU << 24) | (0xF8 << 16)}};
  auto const actual = convert(cable, events);
  EXPECT_THAT(actual, ElementsAre(std::byte{0xF8}));
}

//| Description                                      | MIDI_ver. 1.0  | Event packet |
//| ------------------------------------------------ | -------------- | ------------ |
//|  SysEx message on cable p (CN=0xp).              | F0 00 01 F7    | p4 F0 00 01  |
//|  Start of SysEx: CIN=0x4. End of SysEx: CIN=0x5  |                | p5 F7 00 00  |
// NOLINTNEXTLINE
TEST(USBM1ToByteStream, SysExFourBytes) {
  constexpr auto cable = 0x3U;
  constexpr auto events = std::array{std::uint32_t{(cable << 28) | (0x4U << 24) | (0xF0 << 16) | (0x00 << 8) | 0x01},
                                     std::uint32_t{(cable << 28) | (0x5U << 24) | (0xF7 << 16)}};
  auto const actual = convert(cable, events);
  EXPECT_THAT(actual, ElementsAre(std::byte{0xF0}, std::byte{0x00}, std::byte{0x01}, std::byte{0xF7}));
}

//| Description                                     | MIDI_ver. 1.0   | Event packet |
//| ----------------------------------------------- | --------------- | ------------ |
//| SysEx message on cable p (CN=0xp).              | F0 00 01 02 F7  | p4 F0 00 01  |
//| Start of SysEx: CIN=0x4. End of SysEx: CIN=0x6  |                 | p6 02 F7 00  |
// NOLINTNEXTLINE
TEST(USBM1ToByteStream, SysExFiveBytes) {
  constexpr auto cable = 0x2U;
  constexpr auto events = std::array{std::uint32_t{(cable << 28) | (0x4U << 24) | (0xF0 << 16) | (0x00 << 8) | 0x01},
                                     std::uint32_t{(cable << 28) | (0x6U << 24) | (0x02 << 16) | (0xF7 << 8)}};
  auto const actual = convert(cable, events);
  EXPECT_THAT(actual, ElementsAre(std::byte{0xF0}, std::byte{0x00}, std::byte{0x01}, std::byte{0x02}, std::byte{0xF7}));
}

//| Description                                    | MIDI_ver. 1.0      | Event packet |
//| -----------------------------------------------| ------------------ | ------------ |
//| SysEx message on cable p (CN=0xp).             | F0 00 01 02 03 F7  | p4 F0 00 01  |
//| Start of SysEx: CIN=0x4. End of SysEx: CIN=0x7 |                    | p7 02 03 F7  |
// NOLINTNEXTLINE
TEST(USBM1ToByteStream, SysExSixBytes) {
  constexpr auto cable = 0x9U;
  constexpr auto events = std::array{std::uint32_t{(cable << 28) | (0x4U << 24) | (0xF0 << 16) | (0x00 << 8) | 0x01},
                                     std::uint32_t{(cable << 28) | (0x7U << 24) | (0x02 << 16) | (0x03 << 8) | 0xF7}};
  auto const actual = convert(cable, events);
  EXPECT_THAT(actual, ElementsAre(std::byte{0xF0}, std::byte{0x00}, std::byte{0x01}, std::byte{0x02}, std::byte{0x03},
                                  std::byte{0xF7}));
}

//| Description                                  | MIDI_ver. 1.0 | Event packet |
//| -------------------------------------------- | --------------| ------------ |
//| Two-byte SysEx on cable p (CN=0xp; CIN=0x6)  | F0 F7         | p6 F0 F7 00  |
// NOLINTNEXTLINE
TEST(USBM1ToByteStream, SysExTwoBytes) {
  constexpr auto cable = 0x9U;
  constexpr auto events = std::array{std::uint32_t{(cable << 28) | (0x6U << 24) | (0xF0 << 16) | (0xF7 << 8)}};
  auto const actual = convert(cable, events);
  EXPECT_THAT(actual, ElementsAre(std::byte{0xF0}, std::byte{0xF7}));
}

//| Description                                    | MIDI_ver. 1.0 | Event packet |
//| -----------------------------------------------| --------------| ------------ |
//| Three-byte SysEx on cable p (CN=0xp; CIN=0x7)  | F0 mm F7      | p7 F0 mm F7  |
// NOLINTNEXTLINE
TEST(USBM1ToByteStream, SysExThreeBytes) {
  constexpr auto cable = 0x9U;
  constexpr auto events = std::array{std::uint32_t{(cable << 28) | (0x7U << 24) | (0xF0 << 16) | (0x7F << 8) | 0xF7}};
  auto const actual = convert(cable, events);
  EXPECT_THAT(actual, ElementsAre(std::byte{0xF0}, std::byte{0x7F}, std::byte{0xF7}));
}

}  // end anonymous namespace
