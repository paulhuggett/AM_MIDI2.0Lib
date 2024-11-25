//===-- UMP to MIDI1 protocol -------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ump_to_midi1.hpp"

// Standard Library
#include <algorithm>
#include <any>
#include <array>
#include <ranges>
#include <vector>

// Google Test/Mock
#include <gmock/gmock.h>

namespace {

template <std::ranges::input_range Range> auto convert(Range const& range) {
  std::vector<std::uint32_t> output;
  midi2::ump_to_midi1 ump2m1;
  for (auto const ump : range) {
    ump2m1.push(ump);
    while (!ump2m1.empty()) {
      output.push_back(ump2m1.pop());
    }
  }
  return output;
}

using testing::ContainerEq;
using testing::ElementsAre;
using testing::ElementsAreArray;

// NOLINTNEXTLINE
TEST(UMPToMIDI1, Foo) {
  std::array const input{std::uint32_t{0x20816050}, std::uint32_t{0x20817070}};
  EXPECT_THAT(convert(input), ElementsAreArray(input));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, Sysex) {
  std::array const input{std::uint32_t{0x30167E7F}, std::uint32_t{0x0D70024B},
                         std::uint32_t{0x3026607A}, std::uint32_t{0x737F7F7F},
                         std::uint32_t{0x30267F7D}, std::uint32_t{0x00000000},
                         std::uint32_t{0x30260100}, std::uint32_t{0x00000300},
                         std::uint32_t{0x30360000}, std::uint32_t{0x10000000}};
  EXPECT_THAT(convert(input), ElementsAreArray(input));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, SystemMessageOneByte) {
  std::array const input{std::uint32_t{0x10F80000}};
  EXPECT_THAT(convert(input), ElementsAreArray(input));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2NoteOn) {
  constexpr auto note = 64;
  auto const [w0, w1] =
      midi2::types::m2cvm::note_on{}.group(0).channel(0).note(note).attribute_type(0).velocity(0xC104).attribute(0);
  auto const [expected0] = midi2::types::m1cvm::note_on{}.group(0).channel(0).note(note).velocity(0x60);
  std::array const input{w0.word(), w1.word()};
  EXPECT_THAT(convert(input), ElementsAre(expected0.word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2NoteOff) {
  constexpr auto note = 64;
  auto const [w0, w1] = midi2::types::m2cvm::note_off{}.group(0).channel(0).note(note).attribute_type(0).velocity(0xC104).attribute(0);
  auto const [expected] = midi2::types::m1cvm::note_off{}.group(0).channel(0).note(note).velocity(0x60);
  std::array const input{w0.word(), w1.word()};
  EXPECT_THAT(convert(input), ElementsAre(expected.word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2PolyPressure) {
  constexpr auto note = std::uint8_t{60};
  midi2::types::m2cvm::poly_pressure ump;
  ump.group(0);
  ump.channel(0);
  ump.note(note);
  ump.pressure(0xF000F000);

  midi2::types::m1cvm::poly_pressure expected;
  expected.group(0);
  expected.channel(0);
  expected.note(note);
  expected.pressure(0x78);

  std::array const input{get<0>(ump).word(), get<1>(ump).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(expected).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2ProgramChangeNoBank) {
  constexpr auto program = std::uint8_t{60};
  midi2::types::m2cvm::program_change ump;
  ump.group(0);
  ump.channel(0);
  ump.option_flags(0);
  ump.bank_valid(false);
  ump.program(program);

  midi2::types::m1cvm::program_change expected;
  expected.group(0);
  expected.channel(0);
  expected.program(program);

  std::array const input{get<0>(ump).word(), get<1>(ump).word()};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(get<0>(expected).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2ProgramChangeWithBank) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto channel = std::uint8_t{0x02};
  constexpr auto program = std::uint8_t{60};
  constexpr auto bank_msb = std::uint8_t{0b01010101};
  constexpr auto bank_lsb = std::uint8_t{0b00001111};

  midi2::types::m2cvm::program_change ump;
  ump.group(group);
  ump.channel(channel);
  ump.option_flags(0);
  ump.bank_valid(true);
  ump.program(program);
  ump.bank_msb(bank_msb);
  ump.bank_lsb(bank_lsb);

  constexpr auto expected0 = midi2::types::m1cvm::control_change{}
                                 .group(group)
                                 .channel(channel)
                                 .controller(midi2::control::bank_select)
                                 .value(bank_msb);
  constexpr auto expected1 = midi2::types::m1cvm::control_change{}
                                 .group(group)
                                 .channel(channel)
                                 .controller(midi2::control::bank_select_lsb)
                                 .value(bank_lsb);
  constexpr auto expected2 = midi2::types::m1cvm::program_change{}.group(group).channel(channel).program(program);

  std::array const input{get<0>(ump).word(), get<1>(ump).word()};
  EXPECT_THAT(convert(input),
              ElementsAre(get<0>(expected0).word(), get<0>(expected1).word(), get<0>(expected2).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2ChannelPressure) {
  constexpr auto ump = midi2::types::m2cvm::channel_pressure{}.group(0).channel(0).value(0xF000F000);

  constexpr auto expected = midi2::types::m1cvm::channel_pressure{}.group(0).channel(0).data(0x78);

  std::array const input{get<0>(ump).word(), get<1>(ump).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(expected).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2PerNotePitchBend) {
  midi2::types::m2cvm::per_note_pitch_bend ump;
  ump.group(0);
  ump.channel(0);
  ump.note(60);
  ump.value(0x80000000);

  std::array const input{get<0>(ump).word(), get<1>(ump).word()};
  EXPECT_THAT(convert(input), ElementsAre());
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2RPNController) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto bank = std::uint8_t{60};
  constexpr auto index = std::uint8_t{21};
  constexpr auto value = std::uint32_t{0x12345678};

  constexpr auto src =
      midi2::types::m2cvm::rpn_controller{}.group(group).channel(channel).bank(bank).index(index).value(value);

  std::array<midi2::types::m1cvm::control_change, 4> cc;
  auto& out0 = cc.at(0);
  out0.group(group);
  out0.channel(channel);
  out0.controller(midi2::control::rpn_msb);
  out0.value(bank);

  auto& out1 = cc.at(1);
  out1.group(group);
  out1.channel(channel);
  out1.controller(midi2::control::rpn_lsb);
  out1.value(index);

  constexpr auto val14 = midi2::mcm_scale<32, 14>(value);

  auto& out2 = cc.at(2);
  out2.group(group);
  out2.channel(channel);
  out2.controller(midi2::control::data_entry_msb);
  out2.value((val14 >> 7) & 0x7F);

  auto& out3 = cc.at(3);
  out3.group(group);
  out3.channel(channel);
  out3.controller(midi2::control::data_entry_lsb);
  out3.value(val14 & 0x7F);

  std::array const input{get<0>(src).word(), get<1>(src).word()};
  EXPECT_THAT(convert(input),
              ElementsAre(get<0>(out0).word(), get<0>(out1).word(), get<0>(out2).word(), get<0>(out3).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2RPNControllerTwoChanges) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto bank = std::uint8_t{60};
  constexpr auto index = std::uint8_t{21};
  constexpr auto value0 = std::uint32_t{0x12345678};
  constexpr auto value1 = std::uint32_t{0x87654321};

  std::vector<std::uint32_t> input;

  // This test modifies the same RPN controller twice in succession. We
  // expect that the MIDI-1 messages to set the RPN value are sent just
  // once.
  {
    constexpr auto src0 =
        midi2::types::m2cvm::rpn_controller{}.group(group).channel(channel).bank(bank).index(index).value(value0);
    input.push_back(get<0>(src0).word());
    input.push_back(get<1>(src0).word());
  }
  {
    constexpr auto src1 =
        midi2::types::m2cvm::rpn_controller{}.group(group).channel(channel).bank(bank).index(index).value(value1);
    input.push_back(get<0>(src1).word());
    input.push_back(get<1>(src1).word());
  }

  std::vector<std::uint32_t> expected;
  constexpr auto value0_14 = midi2::mcm_scale<32, 14>(value0);
  constexpr auto value1_14 = midi2::mcm_scale<32, 14>(value1);
  {
    constexpr auto cc0 = midi2::types::m1cvm::control_change{}
                             .group(group)
                             .channel(channel)
                             .controller(midi2::control::rpn_msb)
                             .value(bank);
    expected.push_back(get<0>(cc0).word());
  }
  {
    constexpr auto cc1 = midi2::types::m1cvm::control_change{}
                             .group(group)
                             .channel(channel)
                             .controller(midi2::control::rpn_lsb)
                             .value(index);
    expected.push_back(get<0>(cc1).word());
  }
  {
    constexpr auto cc2 = midi2::types::m1cvm::control_change{}
                             .group(group)
                             .channel(channel)
                             .controller(midi2::control::data_entry_msb)
                             .value((value0_14 >> 7) & 0x7F);
    expected.push_back(get<0>(cc2).word());
  }
  {
    constexpr auto cc3 = midi2::types::m1cvm::control_change{}
                             .group(group)
                             .channel(channel)
                             .controller(midi2::control::data_entry_lsb)
                             .value(value0_14 & 0x7F);
    expected.push_back(get<0>(cc3).word());
  }
  {
    constexpr auto cc4 = midi2::types::m1cvm::control_change{}
                             .group(group)
                             .channel(channel)
                             .controller(midi2::control::data_entry_msb)
                             .value((value1_14 >> 7) & 0x7F);
    expected.push_back(get<0>(cc4).word());
  }
  {
    constexpr auto cc5 = midi2::types::m1cvm::control_change{}
                             .group(group)
                             .channel(channel)
                             .controller(midi2::control::data_entry_lsb)
                             .value(value1_14 & 0x7F);
    expected.push_back(get<0>(cc5).word());
  }
  EXPECT_THAT(convert(input), ElementsAreArray(expected));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2NRPNController) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto bank = std::uint8_t{60};
  constexpr auto index = std::uint8_t{21};
  constexpr auto value = std::uint32_t{0x87654321};
  midi2::types::m2cvm::nrpn_controller src;
  src.group(group);
  src.channel(channel);
  src.bank(bank);
  src.index(index);
  src.value(value);

  std::array<midi2::types::m1cvm::control_change, 4> cc;
  auto& out0 = cc.at(0);
  out0.group(group);
  out0.channel(channel);
  out0.controller(midi2::control::nrpn_msb);
  out0.value(bank);

  auto& out1 = cc.at(1);
  out1.group(group);
  out1.channel(channel);
  out1.controller(midi2::control::nrpn_lsb);
  out1.value(index);

  constexpr auto val14 = midi2::mcm_scale<32, 14>(value);

  auto& out2 = cc.at(2);
  out2.group(group);
  out2.channel(channel);
  out2.controller(midi2::control::data_entry_msb);
  out2.value((val14 >> 7) & 0x7F);

  auto& out3 = cc.at(3);
  out3.group(group);
  out3.channel(channel);
  out3.controller(midi2::control::data_entry_lsb);
  out3.value(val14 & 0x7F);

  std::array const input{get<0>(src).word(), get<1>(src).word()};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(get<0>(out0).word(), get<0>(out1).word(), get<0>(out2).word(), get<0>(out3).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, PitchBend) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto value = std::uint32_t{0xFFFF0000};

  constexpr auto pb = midi2::types::m2cvm::pitch_bend{}.group(group).channel(channel).value(value);

  constexpr auto expected = midi2::types::m1cvm::pitch_bend{}
                                .group(group)
                                .channel(channel)
                                .lsb_data((value >> (32 - 14)) & 0x7F)
                                .msb_data(((value >> (32 - 14)) >> 7) & 0x7F);

  std::array const input{get<0>(pb).word(), get<1>(pb).word()};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(get<0>(expected).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1NoteOff) {
  midi2::types::m1cvm::note_off noff;
  auto const ump = get<0>(noff).word();
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1NoteOn) {
  midi2::types::m1cvm::note_on non;
  auto const ump = get<0>(non).word();
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1PolyPressure) {
  midi2::types::m1cvm::poly_pressure poly_pressure;
  auto const ump = get<0>(poly_pressure).word();
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1ControlChange) {
  midi2::types::m1cvm::control_change control_change;
  auto const ump = get<0>(control_change).word();
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1ProgramChange) {
  midi2::types::m1cvm::program_change program_change;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(program_change));
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1ChannelPressure) {
  midi2::types::m1cvm::channel_pressure channel_pressure;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(channel_pressure));
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1PitchBend) {
  midi2::types::m1cvm::pitch_bend pitch_bend;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(pitch_bend));
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, SystemMessagePassThrough) {
  midi2::ump_to_midi1 ump2m1;
  std::vector<std::uint32_t> output;
  std::vector<std::uint32_t> input;

  auto add = [&input]<typename T>(T const& ump) {
    static_assert(std::tuple_size_v<T> == 1);
    input.emplace_back(get<0>(ump).word());
  };

  add(midi2::types::system::midi_time_code{});
  add(midi2::types::system::song_position_pointer{});
  add(midi2::types::system::song_select{});
  add(midi2::types::system::tune_request{});
  add(midi2::types::system::timing_clock{});
  add(midi2::types::system::sequence_start{});
  add(midi2::types::system::sequence_continue{});
  add(midi2::types::system::sequence_stop{});
  add(midi2::types::system::active_sensing{});
  add(midi2::types::system::reset{});

  for (auto const message: input) {
    ump2m1.push(message);
    while (!ump2m1.empty()) {
      output.push_back(ump2m1.pop());
    }
  }
  EXPECT_THAT(input, ContainerEq(output));
}


}  // end anonymous namespace
