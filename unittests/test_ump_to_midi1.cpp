//===-- UMP to MIDI1 protocol -------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ump/ump_to_midi1.hpp"
#include "midi2/ump/ump_utils.hpp"

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
  midi2::ump::ump_to_midi1 ump2m1;
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
  constexpr std::array input{std::uint32_t{0x20816050}, std::uint32_t{0x20817070}};
  EXPECT_THAT(convert(input), ElementsAreArray(input));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, Sysex) {
  constexpr std::array input{std::uint32_t{0x30167E7F}, std::uint32_t{0x0D70024B}, std::uint32_t{0x3026607A},
                             std::uint32_t{0x737F7F7F}, std::uint32_t{0x30267F7D}, std::uint32_t{0x00000000},
                             std::uint32_t{0x30260100}, std::uint32_t{0x00000300}, std::uint32_t{0x30360000},
                             std::uint32_t{0x10000000}};
  EXPECT_THAT(convert(input), ElementsAreArray(input));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, SystemMessageOneByte) {
  constexpr std::array input{std::uint32_t{0x10F80000}};
  EXPECT_THAT(convert(input), ElementsAreArray(input));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2NoteOn) {
  constexpr auto note = 64;
  auto const [w0, w1] =
      midi2::ump::m2cvm::note_on{}.group(0).channel(0).note(note).attribute_type(0).velocity(0xC104).attribute(0);
  auto const [expected0] = midi2::ump::m1cvm::note_on{}.group(0).channel(0).note(note).velocity(0x60);
  std::array const input{w0.word(), w1.word()};
  EXPECT_THAT(convert(input), ElementsAre(expected0.word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2NoteOff) {
  constexpr auto note = 64;
  auto const [w0, w1] =
      midi2::ump::m2cvm::note_off{}.group(0).channel(0).note(note).attribute_type(0).velocity(0xC104).attribute(0);
  auto const [expected] = midi2::ump::m1cvm::note_off{}.group(0).channel(0).note(note).velocity(0x60);
  std::array const input{w0.word(), w1.word()};
  EXPECT_THAT(convert(input), ElementsAre(expected.word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2PolyPressure) {
  constexpr auto note = std::uint8_t{60};
  midi2::ump::m2cvm::poly_pressure ump;
  ump.group(0);
  ump.channel(0);
  ump.note(note);
  ump.pressure(0xF000F000);

  midi2::ump::m1cvm::poly_pressure expected;
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
  midi2::ump::m2cvm::program_change ump;
  ump.group(0);
  ump.channel(0);
  ump.option_flags(0);
  ump.bank_valid(false);
  ump.program(program);

  midi2::ump::m1cvm::program_change expected;
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

  midi2::ump::m2cvm::program_change ump;
  ump.group(group);
  ump.channel(channel);
  ump.option_flags(0);
  ump.bank_valid(true);
  ump.program(program);
  ump.bank_msb(bank_msb);
  ump.bank_lsb(bank_lsb);

  constexpr auto expected0 = midi2::ump::m1cvm::control_change{}
                                 .group(group)
                                 .channel(channel)
                                 .controller(midi2::ump::control::bank_select)
                                 .value(bank_msb);
  constexpr auto expected1 = midi2::ump::m1cvm::control_change{}
                                 .group(group)
                                 .channel(channel)
                                 .controller(midi2::ump::control::bank_select_lsb)
                                 .value(bank_lsb);
  constexpr auto expected2 = midi2::ump::m1cvm::program_change{}.group(group).channel(channel).program(program);

  std::array const input{get<0>(ump).word(), get<1>(ump).word()};
  EXPECT_THAT(convert(input),
              ElementsAre(get<0>(expected0).word(), get<0>(expected1).word(), get<0>(expected2).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2ChannelPressure) {
  constexpr auto ump = midi2::ump::m2cvm::channel_pressure{}.group(0).channel(0).value(0xF000F000);

  constexpr auto expected = midi2::ump::m1cvm::channel_pressure{}.group(0).channel(0).data(0x78);

  std::array const input{get<0>(ump).word(), get<1>(ump).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(expected).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2PerNotePitchBend) {
  midi2::ump::m2cvm::per_note_pitch_bend ump;
  ump.group(0);
  ump.channel(0);
  ump.note(60);
  ump.value(0x80000000);

  std::array const input{get<0>(ump).word(), get<1>(ump).word()};
  EXPECT_THAT(convert(input), ElementsAre());
}

// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2RPNController) {
  using midi2::hi7, midi2::lo7;
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto bank = std::uint8_t{60};
  constexpr auto index = std::uint8_t{21};
  constexpr auto value = std::uint32_t{0x12345678};

  constexpr auto src =
      midi2::ump::m2cvm::rpn_controller{}.group(group).channel(channel).bank(bank).index(index).value(value);

  constexpr auto val14 = midi2::ump::mcm_scale<32, 14>(value);
  std::array<midi2::ump::m1cvm::control_change, 4> cc{};
  auto& out0 = cc.at(0).group(group).channel(channel).controller(midi2::ump::control::rpn_msb).value(bank);
  auto& out1 = cc.at(1).group(group).channel(channel).controller(midi2::ump::control::rpn_lsb).value(index);
  auto& out2 = cc.at(2).group(group).channel(channel).controller(midi2::ump::control::data_entry_msb).value(hi7(val14));
  auto& out3 = cc.at(3).group(group).channel(channel).controller(midi2::ump::control::data_entry_lsb).value(lo7(val14));

  std::array const input{get<0>(src).word(), get<1>(src).word()};
  EXPECT_THAT(convert(input),
              ElementsAre(get<0>(out0).word(), get<0>(out1).word(), get<0>(out2).word(), get<0>(out3).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2RPNControllerTwoChanges) {
  using midi2::hi7, midi2::lo7;
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto bank = std::uint8_t{60};
  constexpr auto index = std::uint8_t{21};
  constexpr auto value0 = std::uint32_t{0x12345678};
  constexpr auto value1 = std::uint32_t{0x87654321};

  std::vector<std::uint32_t> input;
  input.reserve(4);
  {
    // This test modifies the same RPN controller twice in succession. We
    // expect that the MIDI-1 messages to set the RPN value are sent just
    // once.
    auto const input_append = [&input](std::uint32_t const v) {
      input.push_back(v);
      return false;
    };
    constexpr auto rpnc = []() constexpr {
      return midi2::ump::m2cvm::rpn_controller{}.group(group).channel(channel).bank(bank).index(index);
    };
    midi2::ump::apply(rpnc().value(value0), input_append);
    midi2::ump::apply(rpnc().value(value1), input_append);
  }
  std::vector<std::uint32_t> expected;
  expected.reserve(6);
  {
    auto const expected_append = [&expected](std::uint32_t const v) {
      expected.push_back(v);
      return false;
    };
    constexpr auto value0_14 = midi2::ump::mcm_scale<32, 14>(value0);
    constexpr auto value1_14 = midi2::ump::mcm_scale<32, 14>(value1);
    using enum midi2::ump::control;
    constexpr auto cc = []() constexpr { return midi2::ump::m1cvm::control_change{}.group(group).channel(channel); };
    midi2::ump::apply(cc().controller(rpn_msb).value(bank), expected_append);
    midi2::ump::apply(cc().controller(rpn_lsb).value(index), expected_append);
    midi2::ump::apply(cc().controller(data_entry_msb).value(hi7(value0_14)), expected_append);
    midi2::ump::apply(cc().controller(data_entry_lsb).value(lo7(value0_14)), expected_append);
    midi2::ump::apply(cc().controller(data_entry_msb).value(hi7(value1_14)), expected_append);
    midi2::ump::apply(cc().controller(data_entry_lsb).value(lo7(value1_14)), expected_append);
  }
  EXPECT_THAT(convert(input), ElementsAreArray(expected));
}

// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2RPNTwoDifferentControllers) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto bank = std::uint8_t{60};
  // The controller values we'll be sending.
  constexpr std::array const values{std::uint32_t{0x12345678}, std::uint32_t{0x87654321}, std::uint32_t{0xCAFEBABE}};

  std::vector<std::uint32_t> input;
  input.reserve(10);
  {
    // This test mixes up the modification of two different RPN controllers, sending different values for then each
    // time. This verifies that controller MBS/LSB values are cached correctly.
    auto const input_append = [&input](std::uint32_t const v) {
      input.push_back(v);
      return false;
    };
    constexpr auto rpnc = []() constexpr { return midi2::ump::m2cvm::rpn_controller{}.group(group).channel(channel); };
    midi2::ump::apply(rpnc().bank(bank).index(17).value(values[0]), input_append);
    midi2::ump::apply(rpnc().bank(bank).index(17).value(values[1]), input_append);
    midi2::ump::apply(rpnc().bank(bank).index(18).value(values[1]), input_append);
    midi2::ump::apply(rpnc().bank(bank).index(18).value(values[0]), input_append);
    midi2::ump::apply(rpnc().bank(bank).index(17).value(values[2]), input_append);
  }

  std::vector<std::uint32_t> expected;
  input.reserve(16);
  {
    auto const expected_append = [&expected](std::uint32_t const v) {
      expected.push_back(v);
      return false;
    };
    // values14[] has the 32 bit controller values[] rescaled to 14 bits.
    std::array<std::uint16_t, values.size()> values14{};
    std::ranges::transform(values, std::begin(values14),
                           [](std::uint32_t v) { return midi2::ump::mcm_scale<32, 14>(v); });

    using enum midi2::ump::control;
    using midi2::hi7, midi2::lo7;
    constexpr auto cc = []() constexpr { return midi2::ump::m1cvm::control_change{}.group(group).channel(channel); };
    midi2::ump::apply(cc().controller(rpn_msb).value(bank), expected_append);
    midi2::ump::apply(cc().controller(rpn_lsb).value(17), expected_append);
    midi2::ump::apply(cc().controller(data_entry_msb).value(hi7(values14[0])), expected_append);
    midi2::ump::apply(cc().controller(data_entry_lsb).value(lo7(values14[0])), expected_append);
    midi2::ump::apply(cc().controller(data_entry_msb).value(hi7(values14[1])), expected_append);
    midi2::ump::apply(cc().controller(data_entry_lsb).value(lo7(values14[1])), expected_append);

    midi2::ump::apply(cc().controller(rpn_msb).value(bank), expected_append);
    midi2::ump::apply(cc().controller(rpn_lsb).value(18), expected_append);
    midi2::ump::apply(cc().controller(data_entry_msb).value(hi7(values14[1])), expected_append);
    midi2::ump::apply(cc().controller(data_entry_lsb).value(lo7(values14[1])), expected_append);
    midi2::ump::apply(cc().controller(data_entry_msb).value(hi7(values14[0])), expected_append);
    midi2::ump::apply(cc().controller(data_entry_lsb).value(lo7(values14[0])), expected_append);

    midi2::ump::apply(cc().controller(rpn_msb).value(bank), expected_append);
    midi2::ump::apply(cc().controller(rpn_lsb).value(17), expected_append);
    midi2::ump::apply(cc().controller(data_entry_msb).value(hi7(values14[2])), expected_append);
    midi2::ump::apply(cc().controller(data_entry_lsb).value(lo7(values14[2])), expected_append);
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

  std::vector<std::uint32_t> input;
  midi2::ump::apply(
      midi2::ump::m2cvm::nrpn_controller{}.group(group).channel(channel).bank(bank).index(index).value(value),
      [&input](std::uint32_t const v) {
        input.push_back(v);
        return false;
      });

  constexpr auto val14 = midi2::ump::mcm_scale<32, 14>(value);
  using midi2::hi7, midi2::lo7;
  using midi2::ump::m1cvm::control_change;

  std::vector<std::uint32_t> expected;
  {
    auto const expected_append = [&expected](std::uint32_t const v) {
      expected.push_back(v);
      return false;
    };
    constexpr auto cc = []() constexpr { return control_change{}.group(group).channel(channel); };
    using enum midi2::ump::control;
    midi2::ump::apply(cc().controller(nrpn_msb).value(bank), expected_append);
    midi2::ump::apply(cc().controller(nrpn_lsb).value(index), expected_append);
    midi2::ump::apply(cc().controller(data_entry_msb).value(hi7(val14)), expected_append);
    midi2::ump::apply(cc().controller(data_entry_lsb).value(lo7(val14)), expected_append);
  }
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAreArray(expected));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, PitchBend) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto value = std::uint32_t{0xFFFF0000};

  constexpr auto pb = midi2::ump::m2cvm::pitch_bend{}.group(group).channel(channel).value(value);

  constexpr auto expected = midi2::ump::m1cvm::pitch_bend{}
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
  midi2::ump::m1cvm::note_off off;
  auto const ump = get<0>(off).word();
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1NoteOn) {
  midi2::ump::m1cvm::note_on non;
  auto const ump = get<0>(non).word();
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1PolyPressure) {
  midi2::ump::m1cvm::poly_pressure poly_pressure;
  auto const ump = get<0>(poly_pressure).word();
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1ControlChange) {
  midi2::ump::m1cvm::control_change control_change;
  auto const ump = get<0>(control_change).word();
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1ProgramChange) {
  midi2::ump::m1cvm::program_change program_change;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(program_change));
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1ChannelPressure) {
  midi2::ump::m1cvm::channel_pressure channel_pressure;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(channel_pressure));
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M1PitchBend) {
  midi2::ump::m1cvm::pitch_bend pitch_bend;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(pitch_bend));
  std::array const input{ump};
  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAre(ump));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, SystemMessagePassThrough) {
  midi2::ump::ump_to_midi1 ump2m1;
  std::vector<std::uint32_t> output;
  std::vector<std::uint32_t> input;

  auto add = [&input]<typename T>(T const& ump) {
    static_assert(std::tuple_size_v<T> == 1);
    input.emplace_back(get<0>(ump).word());
  };

  add(midi2::ump::system::midi_time_code{});
  add(midi2::ump::system::song_position_pointer{});
  add(midi2::ump::system::song_select{});
  add(midi2::ump::system::tune_request{});
  add(midi2::ump::system::timing_clock{});
  add(midi2::ump::system::sequence_start{});
  add(midi2::ump::system::sequence_continue{});
  add(midi2::ump::system::sequence_stop{});
  add(midi2::ump::system::active_sensing{});
  add(midi2::ump::system::reset{});

  for (auto const message : input) {
    ump2m1.push(message);
    while (!ump2m1.empty()) {
      output.push_back(ump2m1.pop());
    }
  }
  EXPECT_THAT(input, ContainerEq(output));
}

}  // end anonymous namespace
