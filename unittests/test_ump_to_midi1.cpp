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
#include <array>
#include <vector>

// Google Test/Mock
#include <gmock/gmock.h>

namespace {

template <typename InputIterator>
auto convert(InputIterator first, InputIterator last) {
  std::vector<std::uint32_t> output;
  midi2::ump_to_midi1 ump2m1;
  std::for_each(first, last, [&output, &ump2m1](std::uint32_t const ump) {
    ump2m1.UMPStreamParse(ump);
    while (ump2m1.available()) {
      output.push_back(ump2m1.read());
    }
  });
  return output;
}

using testing::ElementsAre;
using testing::ElementsAreArray;

// NOLINTNEXTLINE
TEST(UMPToMIDI1, Foo) {
  std::array const input{std::uint32_t{0x20816050}, std::uint32_t{0x20817070}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAreArray(input));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, Sysex) {
  std::array const input{std::uint32_t{0x30167E7F}, std::uint32_t{0x0D70024B},
                         std::uint32_t{0x3026607A}, std::uint32_t{0x737F7F7F},
                         std::uint32_t{0x30267F7D}, std::uint32_t{0x00000000},
                         std::uint32_t{0x30260100}, std::uint32_t{0x00000300},
                         std::uint32_t{0x30360000}, std::uint32_t{0x10000000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAreArray(input));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, SystemMessageOneByte) {
  std::array const input{std::uint32_t{0x10F80000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAreArray(input));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2NoteOn) {
  midi2::types::m2cvm::note_on ump;
  auto& in0 = get<0>(ump.w);
  auto& in1 = get<1>(ump.w);
  in0.group = 0;
  in0.channel = 0;
  in0.note = 64;
  in0.attribute = 0;
  in1.velocity = 0xC104;
  in1.attribute = 0;

  midi2::types::m1cvm::note_on expected;
  auto& expected0 = get<0>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.note = 64;
  expected0.velocity = 0x60;

  std::array const input{std::bit_cast<std::uint32_t>(in0), std::bit_cast<std::uint32_t>(in1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAre(std::bit_cast<std::uint32_t>(expected0)));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2NoteOff) {
  midi2::types::m2cvm::note_off in;
  auto& in0 = get<0>(in.w);
  auto& in1 = get<1>(in.w);
  in0.group = 0;
  in0.channel = 0;
  in0.note = 64;
  in0.attribute = 0;
  in1.velocity = 0xC104;
  in1.attribute = 0;

  midi2::types::m1cvm::note_off expected;
  auto& expected0 = get<0>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.note = 64;
  expected0.velocity = 0x60;

  std::array const input{std::bit_cast<std::uint32_t>(in0), std::bit_cast<std::uint32_t>(in1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAre(std::bit_cast<std::uint32_t>(expected0)));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2PolyPressure) {
  constexpr auto note = std::uint8_t{60};
  midi2::types::m2cvm::poly_pressure ump;
  auto& in0 = get<0>(ump.w);
  auto& in1 = get<1>(ump.w);
  in0.group = 0;
  in0.channel = 0;
  in0.note = note;
  in1 = 0xF000F000;

  midi2::types::m1cvm::poly_pressure expected;
  auto& expected0 = get<0>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.note = note;
  expected0.pressure = 0x78;

  std::array const input{std::bit_cast<std::uint32_t>(in0), std::bit_cast<std::uint32_t>(in1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAre(std::bit_cast<std::uint32_t>(expected0)));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2ProgramChangeNoBank) {
  constexpr auto program = std::uint8_t{60};
  midi2::types::m2cvm::program_change ump;
  auto& in0 = get<0>(ump.w);
  auto& in1 = get<1>(ump.w);
  in0.group = 0;
  in0.channel = 0;
  in0.option_flags = 0;
  in0.bank_valid = 0;
  in1.program = program;

  midi2::types::m1cvm::program_change expected;
  auto& expected0 = get<0>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.program = program;

  std::array const input{std::bit_cast<std::uint32_t>(in0), std::bit_cast<std::uint32_t>(in1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAre(std::bit_cast<std::uint32_t>(expected0)));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2ProgramChangeWithBank) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto channel = std::uint8_t{0x02};
  constexpr auto program = std::uint8_t{60};
  constexpr auto bank_msb = std::uint8_t{0b01010101};
  constexpr auto bank_lsb = std::uint8_t{0b00001111};
  midi2::types::m2cvm::program_change ump;
  auto& in0 = get<0>(ump.w);
  auto& in1 = get<1>(ump.w);
  in0.group = group;
  in0.channel = channel;
  in0.option_flags = 0;
  in0.bank_valid = 1;
  in1.program = program;
  in1.bank_msb = bank_msb;
  in1.bank_lsb = bank_lsb;

  midi2::types::m1cvm::control_change expected0;
  auto& x0 = get<0>(expected0.w);
  x0.group = group;
  x0.channel = channel;
  x0.controller = midi2::control::bank_select;
  x0.value = bank_msb;
  midi2::types::m1cvm::control_change expected1;
  auto& x1 = get<0>(expected1.w);
  x1.group = group;
  x1.channel = channel;
  x1.controller = midi2::control::bank_select_lsb;
  x1.value = bank_lsb;
  midi2::types::m1cvm::program_change expected2;
  auto& x2 = get<0>(expected2.w);
  x2.group = group;
  x2.channel = channel;
  x2.program = program;

  std::array const input{std::bit_cast<std::uint32_t>(in0), std::bit_cast<std::uint32_t>(in1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAre(std::bit_cast<std::uint32_t>(x0), std::bit_cast<std::uint32_t>(x1),
                          std::bit_cast<std::uint32_t>(x2)));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2ChannelPressure) {
  midi2::types::m2cvm::channel_pressure ump;
  get<0>(ump.w).group = 0;
  get<0>(ump.w).channel = 0;
  get<1>(ump.w) = 0xF000F000;

  midi2::types::m1cvm::channel_pressure expected;
  get<0>(expected.w).group = 0;
  get<0>(expected.w).channel = 0;
  get<0>(expected.w).data = 0x78;

  std::array const input{std::bit_cast<std::uint32_t>(get<0>(ump.w)), std::bit_cast<std::uint32_t>(get<1>(ump.w))};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAre(std::bit_cast<std::uint32_t>(get<0>(expected.w))));
}
// NOLINTNEXTLINE
TEST(UMPToMIDI1, M2PerNotePitchBend) {
  midi2::types::m2cvm::per_note_pitch_bend ump;
  auto& w0 = get<0>(ump.w);
  auto& w1 = get<1>(ump.w);
  w0.group = 0;
  w0.channel = 0;
  w0.note = 60;
  w1 = 0x80000000;

  std::array const input{std::bit_cast<std::uint32_t>(w0), std::bit_cast<std::uint32_t>(w1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAre());
}

TEST(UMPToMIDI1, M2RPNController) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto bank = std::uint8_t{60};
  constexpr auto index = std::uint8_t{21};
  constexpr auto value = std::uint32_t{0x12345678};
  midi2::types::m2cvm::rpn_controller src;
  auto& src0 = get<0>(src.w);
  auto& src1 = get<1>(src.w);
  src0.group = group;
  src0.channel = channel;
  src0.bank = bank;
  src0.index = index;
  src1 = value;

  midi2::types::m1cvm::control_change cc[4];
  auto& out0 = get<0>(cc[0].w);
  out0.group = group;
  out0.channel = channel;
  out0.controller = midi2::control::rpn_msb;
  out0.value = bank;

  auto& out1 = get<0>(cc[1].w);
  out1.group = group;
  out1.channel = channel;
  out1.controller = midi2::control::rpn_lsb;
  out1.value = index;

  constexpr auto val14 = static_cast<std::uint16_t>(midi2::mcm_scale<32, 14>(value));

  auto& out2 = get<0>(cc[2].w);
  out2.group = group;
  out2.channel = channel;
  out2.controller = midi2::control::data_entry_msb;
  out2.value = (val14 >> 7) & 0x7F;

  auto& out3 = get<0>(cc[3].w);
  out3.group = group;
  out3.channel = channel;
  out3.controller = midi2::control::data_entry_lsb;
  out3.value = val14 & 0x7F;

  std::array const input{std::bit_cast<std::uint32_t>(src0), std::bit_cast<std::uint32_t>(src1)};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAre(std::bit_cast<std::uint32_t>(out0), std::bit_cast<std::uint32_t>(out1),
                          std::bit_cast<std::uint32_t>(out2), std::bit_cast<std::uint32_t>(out3)));
}

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
    midi2::types::m2cvm::rpn_controller src0;
    auto& src00 = get<0>(src0.w);
    src00.group = group;
    src00.channel = channel;
    src00.bank = bank;
    src00.index = index;
    get<1>(src0.w) = value0;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(src0.w)));
    input.push_back(std::bit_cast<std::uint32_t>(get<1>(src0.w)));
  }
  {
    midi2::types::m2cvm::rpn_controller src1;
    auto& src10 = get<0>(src1.w);
    src10.group = group;
    src10.channel = channel;
    src10.bank = bank;
    src10.index = index;
    get<1>(src1.w) = value1;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(src1.w)));
    input.push_back(std::bit_cast<std::uint32_t>(get<1>(src1.w)));
  }

  std::vector<std::uint32_t> expected;
  constexpr auto value0_14 = static_cast<std::uint16_t>(midi2::mcm_scale<32, 14>(value0));
  constexpr auto value1_14 = static_cast<std::uint16_t>(midi2::mcm_scale<32, 14>(value1));
  {
    midi2::types::m1cvm::control_change cc0;
    auto& out0 = get<0>(cc0.w);
    out0.group = group;
    out0.channel = channel;
    out0.controller = midi2::control::rpn_msb;
    out0.value = bank;
    expected.push_back(std::bit_cast<std::uint32_t>(out0));
  }
  {
    midi2::types::m1cvm::control_change cc1;
    auto& out1 = get<0>(cc1.w);
    out1.group = group;
    out1.channel = channel;
    out1.controller = midi2::control::rpn_lsb;
    out1.value = index;
    expected.push_back(std::bit_cast<std::uint32_t>(out1));
  }
  {
    midi2::types::m1cvm::control_change cc2;
    auto& out2 = get<0>(cc2.w);
    out2.group = group;
    out2.channel = channel;
    out2.controller = midi2::control::data_entry_msb;
    out2.value = (value0_14 >> 7) & 0x7F;
    expected.push_back(std::bit_cast<std::uint32_t>(out2));
  }
  {
    midi2::types::m1cvm::control_change cc3;
    auto& out3 = get<0>(cc3.w);
    out3.group = group;
    out3.channel = channel;
    out3.controller = midi2::control::data_entry_lsb;
    out3.value = value0_14 & 0x7F;
    expected.push_back(std::bit_cast<std::uint32_t>(out3));
  }
  {
    midi2::types::m1cvm::control_change cc4;
    auto& out4 = get<0>(cc4.w);
    out4.group = group;
    out4.channel = channel;
    out4.controller = midi2::control::data_entry_msb;
    out4.value = (value1_14 >> 7) & 0x7F;
    expected.push_back(std::bit_cast<std::uint32_t>(out4));
  }
  {
    midi2::types::m1cvm::control_change cc5;
    auto& out5 = get<0>(cc5.w);
    out5.group = group;
    out5.channel = channel;
    out5.controller = midi2::control::data_entry_lsb;
    out5.value = value1_14 & 0x7F;
    expected.push_back(std::bit_cast<std::uint32_t>(out5));
  }

  EXPECT_THAT(convert(std::begin(input), std::end(input)), ElementsAreArray(expected));
}

TEST(UMPToMIDI1, M2NRPNController) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto bank = std::uint8_t{60};
  constexpr auto index = std::uint8_t{21};
  constexpr auto value = std::uint32_t{0x87654321};
  midi2::types::m2cvm::nrpn_controller src;
  auto& src0 = get<0>(src.w);
  auto& src1 = get<1>(src.w);
  src0.group = group;
  src0.channel = channel;
  src0.bank = bank;
  src0.index = index;
  src1 = value;

  midi2::types::m1cvm::control_change cc[4];
  auto& out0 = get<0>(cc[0].w);
  out0.group = group;
  out0.channel = channel;
  out0.controller = midi2::control::nrpn_msb;
  out0.value = bank;

  auto& out1 = get<0>(cc[1].w);
  out1.group = group;
  out1.channel = channel;
  out1.controller = midi2::control::nrpn_lsb;
  out1.value = index;

  constexpr auto val14 = static_cast<std::uint16_t>(midi2::mcm_scale<32, 14>(value));

  auto& out2 = get<0>(cc[2].w);
  out2.group = group;
  out2.channel = channel;
  out2.controller = midi2::control::data_entry_msb;
  out2.value = (val14 >> 7) & 0x7F;

  auto& out3 = get<0>(cc[3].w);
  out3.group = group;
  out3.channel = channel;
  out3.controller = midi2::control::data_entry_lsb;
  out3.value = val14 & 0x7F;

  std::array const input{std::bit_cast<std::uint32_t>(src0), std::bit_cast<std::uint32_t>(src1)};
  auto const actual = convert(std::begin(input), std::end(input));
  EXPECT_THAT(actual, ElementsAre(std::bit_cast<std::uint32_t>(out0), std::bit_cast<std::uint32_t>(out1),
                                  std::bit_cast<std::uint32_t>(out2), std::bit_cast<std::uint32_t>(out3)));
}
TEST(UMPToMIDI1, PitchBend) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto channel = std::uint8_t{3};
  constexpr auto value = std::uint32_t{0xFFFF0000};

  midi2::types::m2cvm::pitch_bend pb;
  auto& pb0 = get<0>(pb.w);
  auto& pb1 = get<1>(pb.w);
  pb0.group = group;
  pb0.channel = channel;
  pb1 = value;

  midi2::types::m1cvm::pitch_bend expected;
  auto& expected0 = get<0>(expected.w);
  expected0.group = group;
  expected0.channel = channel;
  expected0.lsb_data = (value >> (32 - 14)) & 0x7F;
  expected0.msb_data = ((value >> (32 - 14)) >> 7) & 0x7F;

  std::array const input{std::bit_cast<std::uint32_t>(pb0), std::bit_cast<std::uint32_t>(pb1)};
  auto const actual = convert(std::begin(input), std::end(input));
  EXPECT_THAT(actual, ElementsAre(std::bit_cast<std::uint32_t>(expected0)));
}

TEST(UMPToMIDI1, M1NoteOff) {
  midi2::types::m1cvm::note_off noff;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(noff.w));
  std::array const input{ump};
  auto const actual = convert(std::begin(input), std::end(input));
  EXPECT_THAT(actual, ElementsAre(ump));
}
TEST(UMPToMIDI1, M1NoteOn) {
  midi2::types::m1cvm::note_on non;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(non.w));
  std::array const input{ump};
  auto const actual = convert(std::begin(input), std::end(input));
  EXPECT_THAT(actual, ElementsAre(ump));
}
TEST(UMPToMIDI1, M1PolyPressure) {
  midi2::types::m1cvm::poly_pressure poly_pressure;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(poly_pressure.w));
  std::array const input{ump};
  auto const actual = convert(std::begin(input), std::end(input));
  EXPECT_THAT(actual, ElementsAre(ump));
}
TEST(UMPToMIDI1, M1ControlChange) {
  midi2::types::m1cvm::control_change control_change;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(control_change.w));
  std::array const input{ump};
  auto const actual = convert(std::begin(input), std::end(input));
  EXPECT_THAT(actual, ElementsAre(ump));
}
TEST(UMPToMIDI1, M1ProgramChange) {
  midi2::types::m1cvm::program_change program_change;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(program_change.w));
  std::array const input{ump};
  auto const actual = convert(std::begin(input), std::end(input));
  EXPECT_THAT(actual, ElementsAre(ump));
}
TEST(UMPToMIDI1, M1ChannelPressure) {
  midi2::types::m1cvm::channel_pressure channel_pressure;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(channel_pressure.w));
  std::array const input{ump};
  auto const actual = convert(std::begin(input), std::end(input));
  EXPECT_THAT(actual, ElementsAre(ump));
}
TEST(UMPToMIDI1, M1PitchBend) {
  midi2::types::m1cvm::pitch_bend pitch_bend;
  auto const ump = std::bit_cast<std::uint32_t>(get<0>(pitch_bend.w));
  std::array const input{ump};
  auto const actual = convert(std::begin(input), std::end(input));
  EXPECT_THAT(actual, ElementsAre(ump));
}

}  // end anonymous namespace
