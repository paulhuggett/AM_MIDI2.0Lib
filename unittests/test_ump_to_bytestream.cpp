//===-- UMP to bytestream -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ump_to_bytestream.hpp"

// Standard library
#include <array>
#include <cstdint>
#include <ranges>
#include <vector>

// Google test
#include <gmock/gmock.h>

namespace {

template <std::ranges::input_range Range>
std::vector<std::byte> convert(Range && range) {
  midi2::ump_to_bytestream ump2bs;
  std::vector<std::byte> output;
  for (auto const ump : range) {
    ump2bs.UMPStreamParse(ump);
    while (ump2bs.available()) {
      output.push_back(ump2bs.read());
    }
  }
  return output;
}

using testing::ElementsAre;
using testing::ElementsAreArray;

// NOLINTNEXTLINE
TEST(UMPToBytestream, NoteOff) {
  std::array const input{std::uint32_t{0x20816050}, std::uint32_t{0x20817070}};
  EXPECT_THAT(convert(input), ElementsAre(std::byte{0x81}, std::byte{0x60}, std::byte{0x50}, std::byte{0x81},
                                          std::byte{0x70}, std::byte{0x70}));
}
// NOLINTNEXTLINE
TEST(UMPToBytestream, NoteOn) {
  constexpr auto channel = 1;
  constexpr auto note0 = 62;
  constexpr auto velocity0 = 0x7F;
  constexpr auto note1 = 74;
  constexpr auto velocity1 = 0;

  std::array<midi2::types::m1cvm::note_on, 2> message;
  auto& w0 = get<0>(message[0].w);
  w0.channel = channel;
  w0.note = note0;
  w0.velocity = velocity0;
  auto& w1 = get<0>(message[1].w);
  w1.channel = channel;
  w1.note = note1;
  w1.velocity = velocity1;
  std::array const input{std::bit_cast<std::uint32_t>(w0), std::bit_cast<std::uint32_t>(w1)};

  std::array const expected{
      std::byte{to_underlying(midi2::status::note_on)} | std::byte{channel}, std::byte{note0}, std::byte{velocity0},
      std::byte{to_underlying(midi2::status::note_on)} | std::byte{channel}, std::byte{note1}, std::byte{velocity1},
  };
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAreArray(expected));
}

// NOLINTNEXTLINE
TEST(UMPTOBytestream, SystemTimeCode) {
  midi2::types::system::midi_time_code message;
  auto const tc = 0b1010101;
  get<0>(message.w).time_code = tc;
  std::array const input{std::bit_cast<std::uint32_t>(std::get<0>(message.w))};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(std::byte{to_underlying(midi2::status::timing_code)}, std::byte{tc}));
}
// NOLINTNEXTLINE
TEST(UMPToByteStream, SystemSongPositionPointer) {
  auto const lsb = 0b01111000;
  auto const msb = 0b00001111;
  midi2::types::system::song_position_pointer message;
  auto& w0 = get<0>(message.w);
  w0.position_lsb = lsb;
  w0.position_msb = msb;

  std::array const input{std::bit_cast<std::uint32_t>(w0)};
  std::array const expected{std::byte{to_underlying(midi2::status::spp)}, std::byte{lsb}, std::byte{msb}};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAreArray(expected));
}
// NOLINTNEXTLINE
TEST(UMPToBytestream, SystemTuneRequest) {
  midi2::types::system::tune_request message;
  std::array const input{std::bit_cast<std::uint32_t>(std::get<0>(message.w))};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(std::byte{to_underlying(midi2::status::tune_request)}));
}
// NOLINTNEXTLINE
TEST(UMPToBytestream, SystemTimingClock) {
  midi2::types::system::timing_clock message;
  std::array const input{std::bit_cast<std::uint32_t>(std::get<0>(message.w))};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(std::byte{to_underlying(midi2::status::timing_clock)}));
}
// NOLINTNEXTLINE
TEST(UMPToBytestream, ProgramChangeTwoBytes) {
  std::array const input{std::uint32_t{0x20C64000}};
  EXPECT_THAT(convert(input), ElementsAre(std::byte{0xC6}, std::byte{0x40}));
}
// NOLINTNEXTLINE
TEST(UMPToBytestream, Sysex) {
  std::array const input{std::uint32_t{0x30167E7F}, std::uint32_t{0x0D70024B}, std::uint32_t{0x3026607A},
                         std::uint32_t{0x737F7F7F}, std::uint32_t{0x30267F7D}, std::uint32_t{0x00000000},
                         std::uint32_t{0x30260100}, std::uint32_t{0x00000300}, std::uint32_t{0x30360000},
                         std::uint32_t{0x10000000}};
  EXPECT_THAT(
      convert(input),
      ElementsAre(std::byte{0xF0}, std::byte{0x7E}, std::byte{0x7F}, std::byte{0x0D}, std::byte{0x70}, std::byte{0x02},
                  std::byte{0x4B}, std::byte{0x60}, std::byte{0x7A}, std::byte{0x73}, std::byte{0x7F}, std::byte{0x7F},
                  std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7D}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
                  std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x03},
                  std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x10}, std::byte{0x00}, std::byte{0x00},
                  std::byte{0x00}, std::byte{0xF7}));
}

}  // end anonymous namespace
