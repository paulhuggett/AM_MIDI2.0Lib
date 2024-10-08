//===-- UMP to MIDI1 protocol -------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/umpToMIDI1Protocol.hpp"

// Standard Library
#include <algorithm>
#include <vector>

// Google Test/Mock
#include <gmock/gmock.h>

namespace {

template <typename InputIterator>
auto convert(InputIterator first, InputIterator last) {
  std::vector<std::uint32_t> output;
  midi2::umpToMIDI1Protocol UMP2M1;
  std::for_each(first, last, [&output, &UMP2M1](std::uint32_t const ump) {
    UMP2M1.UMPStreamParse(ump);
    while (UMP2M1.availableUMP()) {
      output.push_back(UMP2M1.readUMP());
    }
  });
  return output;
}

using testing::ElementsAre;
using testing::ElementsAreArray;

TEST(UMPToMIDI1, Foo) {
  std::array const input{std::uint32_t{0x20816050}, std::uint32_t{0x20817070}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAreArray(input));
}
TEST(UMPToMIDI1, Sysex) {
  std::array const input{std::uint32_t{0x30167E7F}, std::uint32_t{0x0D70024B},
                         std::uint32_t{0x3026607A}, std::uint32_t{0x737F7F7F},
                         std::uint32_t{0x30267F7D}, std::uint32_t{0x00000000},
                         std::uint32_t{0x30260100}, std::uint32_t{0x00000300},
                         std::uint32_t{0x30360000}, std::uint32_t{0x10000000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAreArray(input));
}
TEST(UMPToMIDI1, SystemMessageOneByte) {
  std::array const input{std::uint32_t{0x10F80000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAreArray(input));
}
TEST(UMPToMIDI1, NoteOn) {
  midi2::types::m2cvm::note_on ump;
  ump.w0.group = 0;
  ump.w0.channel = 0;
  ump.w0.note = 64;
  ump.w0.attribute = 0;
  ump.w1.velocity = 0xC104;
  ump.w1.attribute = 0;

  midi2::types::m1cvm::note_on expected;
  expected.w0.group = 0;
  expected.w0.channel = 0;
  expected.w0.note = 64;
  expected.w0.velocity = 0x60;

  std::array const input{std::bit_cast<std::uint32_t>(ump.w0), std::bit_cast<std::uint32_t>(ump.w1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAre(std::bit_cast<std::uint32_t>(expected)));
}
TEST(UMPToMIDI1, NoteOff) {
  midi2::types::m2cvm::note_off ump;
  ump.w0.group = 0;
  ump.w0.channel = 0;
  ump.w0.note = 64;
  ump.w0.attribute = 0;
  ump.w1.velocity = 0xC104;
  ump.w1.attribute = 0;

  midi2::types::m1cvm::note_off expected;
  expected.w0.group = 0;
  expected.w0.channel = 0;
  expected.w0.note = 64;
  expected.w0.velocity = 0x60;

  std::array const input{std::bit_cast<std::uint32_t>(ump.w0), std::bit_cast<std::uint32_t>(ump.w1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAre(std::bit_cast<std::uint32_t>(expected)));
}
TEST(UMPToMIDI1, PolyPressure) {
  constexpr auto note = std::uint8_t{60};
  midi2::types::m2cvm::poly_pressure ump;
  ump.w0.group = 0;
  ump.w0.channel = 0;
  ump.w0.note = note;
  ump.w1 = 0xF000F000;

  midi2::types::m1cvm::poly_pressure expected;
  expected.w0.group = 0;
  expected.w0.channel = 0;
  expected.w0.note = note;
  expected.w0.pressure = 0x78;

  std::array const input{std::bit_cast<std::uint32_t>(ump.w0), std::bit_cast<std::uint32_t>(ump.w1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAre(std::bit_cast<std::uint32_t>(expected)));
}
TEST(UMPToMIDI1, ProgramChangeNoBank) {
  constexpr auto program = std::uint8_t{60};
  midi2::types::m2cvm::program_change ump;
  ump.w0.group = 0;
  ump.w0.channel = 0;
  ump.w0.option_flags = 0;
  ump.w0.bank_valid = 0;
  ump.w1.program = program;

  midi2::types::m1cvm::program_change expected;
  expected.w0.group = 0;
  expected.w0.channel = 0;
  expected.w0.program = program;

  std::array const input{std::bit_cast<std::uint32_t>(ump.w0), std::bit_cast<std::uint32_t>(ump.w1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAre(std::bit_cast<std::uint32_t>(expected)));
}
TEST(UMPToMIDI1, ProgramChangeWithBank) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto channel = std::uint8_t{0x02};
  constexpr auto program = std::uint8_t{60};
  constexpr auto bank_msb = std::uint8_t{0b01010101};
  constexpr auto bank_lsb = std::uint8_t{0b00001111};
  midi2::types::m2cvm::program_change ump;
  ump.w0.group = group;
  ump.w0.channel = channel;
  ump.w0.option_flags = 0;
  ump.w0.bank_valid = 1;
  ump.w1.program = program;
  ump.w1.bank_msb = bank_msb;
  ump.w1.bank_lsb = bank_lsb;

  midi2::types::m1cvm::control_change expected0;
  expected0.w0.group = group;
  expected0.w0.channel = channel;
  expected0.w0.index = midi2::control::bank_select;
  expected0.w0.data = bank_msb;
  midi2::types::m1cvm::control_change expected1;
  expected1.w0.group = group;
  expected1.w0.channel = channel;
  expected1.w0.index = midi2::control::bank_select_lsb;
  expected1.w0.data = bank_lsb;
  midi2::types::m1cvm::program_change expected2;
  expected2.w0.group = group;
  expected2.w0.channel = channel;
  expected2.w0.program = program;

  std::array const input{std::bit_cast<std::uint32_t>(ump.w0), std::bit_cast<std::uint32_t>(ump.w1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAre(std::bit_cast<std::uint32_t>(expected0), std::bit_cast<std::uint32_t>(expected1),
                          std::bit_cast<std::uint32_t>(expected2)));
}

}  // end anonymous namespace
