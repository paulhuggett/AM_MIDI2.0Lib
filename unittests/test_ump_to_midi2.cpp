// DUT
#include "midi2/ump_to_midi2.hpp"

// Standard Library
#include <array>
#include <ranges>
#include <vector>

// Google Test/Mock/Fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

using testing::ContainerEq;
using testing::ElementsAre;
using testing::ElementsAreArray;
using testing::IsEmpty;

namespace {

template <std::ranges::input_range Range> auto convert(Range const& input) {
  std::vector<std::uint32_t> output;
  midi2::ump_to_midi2 ump2m2{0};
  for (auto const message : input) {
    ump2m2.push(message);
    while (!ump2m2.empty()) {
      output.push_back(ump2m2.pop());
    }
  }
  return output;
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, NoteOff) {
  constexpr auto note = 64;

  midi2::types::m1cvm::note_off in;
  auto& in0 = get<0>(in.w);
  in0.group = 0;
  in0.channel = 0;
  in0.note = note;
  in0.velocity = 0x60;

  midi2::types::m2cvm::note_off expected;
  auto& expected0 = get<0>(expected.w);
  auto& expected1 = get<1>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.note = note;
  expected0.attribute = 0;
  expected1.velocity = 0xC104;
  expected1.attribute = 0;

  std::array const input{std::bit_cast<std::uint32_t>(in0)};
  EXPECT_THAT(convert(input),
              ElementsAre(std::bit_cast<std::uint32_t>(expected0), std::bit_cast<std::uint32_t>(expected1)));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, NoteOn) {
  constexpr auto note = 64;

  midi2::types::m1cvm::note_on in;
  auto& in0 = get<0>(in.w);
  in0.group = 0;
  in0.channel = 0;
  in0.note = note;
  in0.velocity = 0x60;

  midi2::types::m2cvm::note_on expected;
  auto& expected0 = get<0>(expected.w);
  auto& expected1 = get<1>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.note = note;
  expected0.attribute = 0;
  expected1.velocity = 0xC104;
  expected1.attribute = 0;

  std::array const input{std::bit_cast<std::uint32_t>(in0)};
  EXPECT_THAT(convert(input),
              ElementsAre(std::bit_cast<std::uint32_t>(expected0), std::bit_cast<std::uint32_t>(expected1)));
}

// UMPToMidi2
TEST(UMPToMidi2, NoteOnImplicitNoteOff) {
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint8_t{0x60};
  constexpr auto group = std::uint8_t{3};
  constexpr auto channel = std::uint8_t{5};

  // A note on message followed by a note-on with velocity 0. The second of
  // these should become a note-off.
  std::vector<std::uint32_t> input;
  {
    midi2::types::m1cvm::note_on in_non_1;
    get<0>(in_non_1.w).group = group;
    get<0>(in_non_1.w).channel = channel;
    get<0>(in_non_1.w).note = note_number;
    get<0>(in_non_1.w).velocity = velocity;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(in_non_1.w)));
  }
  {
    midi2::types::m1cvm::note_on in_non_2;
    get<0>(in_non_2.w).group = group;
    get<0>(in_non_2.w).channel = channel;
    get<0>(in_non_2.w).note = note_number;
    get<0>(in_non_2.w).velocity = 0;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(in_non_2.w)));
  }

  std::vector<std::uint32_t> expected;
  {
    midi2::types::m2cvm::note_on expected_non;
    get<0>(expected_non.w).group = group;
    get<0>(expected_non.w).channel = channel;
    get<0>(expected_non.w).note = note_number;
    get<1>(expected_non.w).velocity = static_cast<std::uint16_t>(midi2::mcm_scale<7, 16>(velocity));
    expected.push_back(std::bit_cast<std::uint32_t>(get<0>(expected_non.w)));
    expected.push_back(std::bit_cast<std::uint32_t>(get<1>(expected_non.w)));
  }
  {
    midi2::types::m2cvm::note_on expected_noff;
    get<0>(expected_noff.w).group = group;
    get<0>(expected_noff.w).channel = channel;
    get<0>(expected_noff.w).note = note_number;
    get<1>(expected_noff.w).velocity = 0;
    expected.push_back(std::bit_cast<std::uint32_t>(get<0>(expected_noff.w)));
    expected.push_back(std::bit_cast<std::uint32_t>(get<1>(expected_noff.w)));
  }

  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAreArray(expected));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, PolyPressure) {
  constexpr auto note = 64;

  midi2::types::m1cvm::poly_pressure in;
  auto& in0 = get<0>(in.w);
  in0.group = 0;
  in0.channel = 0;
  in0.note = note;
  in0.pressure = 0x60;

  midi2::types::m2cvm::poly_pressure expected;
  auto& expected0 = get<0>(expected.w);
  auto& expected1 = get<1>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.note = note;
  expected1 = mcm_scale<7, 32>(in0.pressure);

  std::array const input{std::bit_cast<std::uint32_t>(in0)};
  EXPECT_THAT(convert(input),
              ElementsAre(std::bit_cast<std::uint32_t>(expected0), std::bit_cast<std::uint32_t>(expected1)));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, PitchBend) {
  constexpr auto pb14 = 0b0010101010101010U;  // A 14-bit value for the pitch bend

  midi2::types::m1cvm::pitch_bend m1;
  auto& m10 = get<0>(m1.w);
  m10.group = 0;
  m10.channel = 0;
  m10.lsb_data = pb14 & ((1 << 7) - 1);
  m10.msb_data = pb14 >> 7;

  midi2::types::m2cvm::pitch_bend m2;
  auto& m20 = get<0>(m2.w);
  auto& m21 = get<1>(m2.w);
  m20.group = 0;
  m20.channel = 0;
  m21 = midi2::mcm_scale<14, 32>(pb14);

  std::array const input{std::bit_cast<std::uint32_t>(m10)};
  EXPECT_THAT(convert(input), ElementsAre(std::bit_cast<std::uint32_t>(m20), std::bit_cast<std::uint32_t>(m21)));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, ChannelPressure) {
  constexpr auto pressure = 0b0101010;
  constexpr auto group = std::uint8_t{3};
  constexpr auto channel = std::uint8_t{7};

  midi2::types::m1cvm::channel_pressure m1;
  auto& m10 = get<0>(m1.w);
  m10.group = group;
  m10.channel = channel;
  m10.data = pressure;

  midi2::types::m2cvm::channel_pressure m2;
  auto& m20 = get<0>(m2.w);
  auto& m21 = get<1>(m2.w);
  m20.group = group;
  m20.channel = channel;
  m21 = midi2::mcm_scale<7, 32>(pressure);

  std::array const input{std::bit_cast<std::uint32_t>(m10)};
  EXPECT_THAT(convert(input), ElementsAre(std::bit_cast<std::uint32_t>(m20), std::bit_cast<std::uint32_t>(m21)));
}
// NOLINTNEXTLINE
TEST(UMPToMidi2, SimpleContinuousController) {
  constexpr auto controller = 0b01100110;
  constexpr auto value = 0b01010101;
  constexpr auto group = std::uint8_t{3};
  constexpr auto channel = std::uint8_t{7};

  midi2::types::m1cvm::control_change m1;
  auto& m10 = get<0>(m1.w);
  m10.group = group;
  m10.channel = channel;
  m10.controller = controller;
  m10.value = value;

  midi2::types::m2cvm::control_change m2;
  auto& m20 = get<0>(m2.w);
  auto& m21 = get<1>(m2.w);
  m20.group = group;
  m20.channel = channel;
  m20.controller = controller;
  m21 = midi2::mcm_scale<7, 32>(value);

  std::array const input{std::bit_cast<std::uint32_t>(m10)};
  EXPECT_THAT(convert(input), ElementsAre(std::bit_cast<std::uint32_t>(m20), std::bit_cast<std::uint32_t>(m21)));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, SimpleProgramChange) {
  constexpr auto program = std::uint8_t{0b01010101};
  constexpr auto group = std::uint8_t{0x1};
  constexpr auto channel = std::uint8_t{0xF};

  midi2::types::m1cvm::program_change m1;
  auto& m10 = get<0>(m1.w);
  m10.group = group;
  m10.channel = channel;
  m10.program = program;

  midi2::types::m2cvm::program_change m2;
  auto& m20 = get<0>(m2.w);
  auto& m21 = get<1>(m2.w);
  m20.group = group;
  m20.channel = channel;
  m20.option_flags = 0;
  m20.bank_valid = 0;
  m21.program = program;
  m21.bank_msb = 0;
  m21.bank_lsb = 0;

  std::array const input{std::bit_cast<std::uint32_t>(m10)};
  EXPECT_THAT(convert(input), ElementsAre(std::bit_cast<std::uint32_t>(m20), std::bit_cast<std::uint32_t>(m21)));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, ProgramChangeWithBank) {
  constexpr auto group = std::uint8_t{0x1};
  constexpr auto channel = std::uint8_t{0xF};
  constexpr auto program = std::uint8_t{0b01010101};
  constexpr auto bank_msb = std::uint8_t{0b01110001};
  constexpr auto bank_lsb = std::uint8_t{0b01001110};

  std::vector<std::uint32_t> input;

  {
    midi2::types::m1cvm::control_change m1cc_bank_msb;
    get<0>(m1cc_bank_msb.w).group = group;
    get<0>(m1cc_bank_msb.w).channel = channel;
    get<0>(m1cc_bank_msb.w).controller = midi2::control::bank_select;
    get<0>(m1cc_bank_msb.w).value = bank_msb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(m1cc_bank_msb.w)));
  }
  {
    midi2::types::m1cvm::control_change m1cc_bank_lsb;
    get<0>(m1cc_bank_lsb.w).group = group;
    get<0>(m1cc_bank_lsb.w).channel = channel;
    get<0>(m1cc_bank_lsb.w).controller = midi2::control::bank_select_lsb;
    get<0>(m1cc_bank_lsb.w).value = bank_lsb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(m1cc_bank_lsb.w)));
  }
  {
    midi2::types::m1cvm::program_change m1;
    get<0>(m1.w).group = group;
    get<0>(m1.w).channel = channel;
    get<0>(m1.w).program = program;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(m1.w)));
  }

  midi2::types::m2cvm::program_change m2;
  auto& m20 = get<0>(m2.w);
  auto& m21 = get<1>(m2.w);
  m20.group = group;
  m20.channel = channel;
  m20.option_flags = 0;
  m20.bank_valid = 1;
  m21.program = program;
  m21.bank_msb = bank_msb;
  m21.bank_lsb = bank_lsb;
  EXPECT_THAT(convert(input), ElementsAre(std::bit_cast<std::uint32_t>(m20), std::bit_cast<std::uint32_t>(m21)));
}

TEST(UMPToMidi2, ControlChangeRPN) {
  constexpr auto group = std::uint8_t{0x1};
  constexpr auto channel = std::uint8_t{0xF};
  constexpr auto control_msb = std::uint8_t{0b01010101};
  constexpr auto control_lsb = std::uint8_t{0b01101010};
  constexpr auto value_msb = std::uint8_t{0b00011001};
  constexpr auto value_lsb = std::uint8_t{0b01100110};

  std::vector<std::uint32_t> input;

  {
    midi2::types::m1cvm::control_change pn_msb;
    get<0>(pn_msb.w).group = group;
    get<0>(pn_msb.w).channel = channel;
    get<0>(pn_msb.w).controller = midi2::control::rpn_msb;
    get<0>(pn_msb.w).value = control_msb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(pn_msb.w)));
  }
  {
    midi2::types::m1cvm::control_change pn_lsb;
    get<0>(pn_lsb.w).group = group;
    get<0>(pn_lsb.w).channel = channel;
    get<0>(pn_lsb.w).controller = midi2::control::rpn_lsb;
    get<0>(pn_lsb.w).value = control_lsb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(pn_lsb.w)));
  }
  {
    midi2::types::m1cvm::control_change param_value_msb;
    get<0>(param_value_msb.w).group = group;
    get<0>(param_value_msb.w).channel = channel;
    get<0>(param_value_msb.w).controller = midi2::control::data_entry_msb;
    get<0>(param_value_msb.w).value = value_msb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(param_value_msb.w)));
  }
  {
    midi2::types::m1cvm::control_change param_value_lsb;
    get<0>(param_value_lsb.w).group = group;
    get<0>(param_value_lsb.w).channel = channel;
    get<0>(param_value_lsb.w).controller = midi2::control::data_entry_lsb;
    get<0>(param_value_lsb.w).value = value_lsb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(param_value_lsb.w)));
  }
  {
    midi2::types::m1cvm::control_change null_msb;
    get<0>(null_msb.w).group = group;
    get<0>(null_msb.w).channel = channel;
    get<0>(null_msb.w).controller = midi2::control::rpn_msb;
    get<0>(null_msb.w).value = 0x7F;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(null_msb.w)));
  }
  {
    midi2::types::m1cvm::control_change null_lsb;
    get<0>(null_lsb.w).group = group;
    get<0>(null_lsb.w).channel = channel;
    get<0>(null_lsb.w).controller = midi2::control::rpn_lsb;
    get<0>(null_lsb.w).value = 0x7F;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(null_lsb.w)));
  }

  midi2::types::m2cvm::rpn_controller m2;
  auto& m20 = get<0>(m2.w);
  auto& m21 = get<1>(m2.w);
  m20.group = group;
  m20.channel = channel;
  m20.bank = control_msb;
  m20.index = control_lsb;
  m21 = midi2::mcm_scale<14, 32>((std::uint32_t{value_msb} << 7) | std::uint32_t{value_lsb});
  EXPECT_THAT(convert(input), ElementsAre(std::bit_cast<std::uint32_t>(m20), std::bit_cast<std::uint32_t>(m21)));
}

TEST(UMPToMidi2, ControlChangeNRPN) {
  constexpr auto group = std::uint8_t{0x1};
  constexpr auto channel = std::uint8_t{0xF};
  constexpr auto control_msb = std::uint8_t{0b01010101};
  constexpr auto control_lsb = std::uint8_t{0b01101010};
  constexpr auto value_msb = std::uint8_t{0b00011001};
  constexpr auto value_lsb = std::uint8_t{0b01100110};

  std::vector<std::uint32_t> input;

  {
    midi2::types::m1cvm::control_change pn_msb;
    get<0>(pn_msb.w).group = group;
    get<0>(pn_msb.w).channel = channel;
    get<0>(pn_msb.w).controller = midi2::control::nrpn_msb;
    get<0>(pn_msb.w).value = control_msb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(pn_msb.w)));
  }
  {
    midi2::types::m1cvm::control_change pn_lsb;
    get<0>(pn_lsb.w).group = group;
    get<0>(pn_lsb.w).channel = channel;
    get<0>(pn_lsb.w).controller = midi2::control::nrpn_lsb;
    get<0>(pn_lsb.w).value = control_lsb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(pn_lsb.w)));
  }
  {
    midi2::types::m1cvm::control_change param_value_msb;
    get<0>(param_value_msb.w).group = group;
    get<0>(param_value_msb.w).channel = channel;
    get<0>(param_value_msb.w).controller = midi2::control::data_entry_msb;
    get<0>(param_value_msb.w).value = value_msb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(param_value_msb.w)));
  }
  {
    midi2::types::m1cvm::control_change param_value_lsb;
    get<0>(param_value_lsb.w).group = group;
    get<0>(param_value_lsb.w).channel = channel;
    get<0>(param_value_lsb.w).controller = midi2::control::data_entry_lsb;
    get<0>(param_value_lsb.w).value = value_lsb;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(param_value_lsb.w)));
  }
  {
    midi2::types::m1cvm::control_change null_msb;
    get<0>(null_msb.w).group = group;
    get<0>(null_msb.w).channel = channel;
    get<0>(null_msb.w).controller = midi2::control::nrpn_msb;
    get<0>(null_msb.w).value = 0x7F;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(null_msb.w)));
  }
  {
    midi2::types::m1cvm::control_change null_lsb;
    get<0>(null_lsb.w).group = group;
    get<0>(null_lsb.w).channel = channel;
    get<0>(null_lsb.w).controller = midi2::control::nrpn_lsb;
    get<0>(null_lsb.w).value = 0x7F;
    input.push_back(std::bit_cast<std::uint32_t>(get<0>(null_lsb.w)));
  }

  midi2::types::m2cvm::nrpn_controller m2;
  auto& m20 = get<0>(m2.w);
  auto& m21 = get<1>(m2.w);
  m20.group = group;
  m20.channel = channel;
  m20.bank = control_msb;
  m20.index = control_lsb;
  m21 = midi2::mcm_scale<14, 32>((std::uint32_t{value_msb} << 7) | std::uint32_t{value_lsb});
  EXPECT_THAT(convert(input), ElementsAre(std::bit_cast<std::uint32_t>(m20), std::bit_cast<std::uint32_t>(m21)));
}

template <typename T> class UMPToMidi2PassThrough : public testing::Test {
public:
  template <typename T2>
    requires(std::tuple_size_v<decltype(T2::w)> == 1)
  static std::vector<std::uint32_t> add(T2 const& ump) {
    return {std::bit_cast<std::uint32_t>(get<0>(ump.w))};
  }

  template <typename T2>
    requires(std::tuple_size_v<decltype(T2::w)> == 2)
  static std::vector<std::uint32_t> add(T2 const& ump) {
    return {std::bit_cast<std::uint32_t>(get<0>(ump.w)), std::bit_cast<std::uint32_t>(get<1>(ump.w))};
  }

  template <typename T2>
    requires(std::tuple_size_v<decltype(T2::w)> == 4)
  static std::vector<std::uint32_t> add(T2 const& ump) {
    return {std::bit_cast<std::uint32_t>(get<0>(ump.w)), std::bit_cast<std::uint32_t>(get<1>(ump.w)),
            std::bit_cast<std::uint32_t>(get<2>(ump.w)), std::bit_cast<std::uint32_t>(get<3>(ump.w))};
  }
};

// clang-format off
using PassThroughTypes = ::testing::Types<
  midi2::types::utility::jr_clock,
  midi2::types::utility::jr_timestamp,
  midi2::types::utility::delta_clockstamp_tpqn,
  midi2::types::utility::delta_clockstamp,

  midi2::types::system::midi_time_code,
  midi2::types::system::song_position_pointer,
  midi2::types::system::song_select,
  midi2::types::system::tune_request,
  midi2::types::system::timing_clock,
  midi2::types::system::sequence_start,
  midi2::types::system::sequence_continue,
  midi2::types::system::sequence_stop,
  midi2::types::system::active_sensing,
  midi2::types::system::reset,

  midi2::types::data64::sysex7_in_1,
  midi2::types::data64::sysex7_start,
  midi2::types::data64::sysex7_continue,
  midi2::types::data64::sysex7_end,

  midi2::types::m2cvm::note_off,
  midi2::types::m2cvm::note_on,
  midi2::types::m2cvm::poly_pressure,
  midi2::types::m2cvm::program_change,
  midi2::types::m2cvm::channel_pressure,
  midi2::types::m2cvm::rpn_controller,
  midi2::types::m2cvm::nrpn_controller,
  midi2::types::m2cvm::rpn_per_note_controller,
  midi2::types::m2cvm::nrpn_per_note_controller,
  midi2::types::m2cvm::rpn_relative_controller,
  midi2::types::m2cvm::nrpn_relative_controller,
  midi2::types::m2cvm::per_note_management,
  midi2::types::m2cvm::control_change,
  midi2::types::m2cvm::pitch_bend,
  midi2::types::m2cvm::per_note_pitch_bend,

  midi2::types::data128::sysex8_in_1,
  midi2::types::data128::sysex8_start,
  midi2::types::data128::sysex8_continue,
  midi2::types::data128::sysex8_end,
  midi2::types::data128::mds_header,
  midi2::types::data128::mds_payload,

  midi2::types::ump_stream::endpoint_discovery,
  midi2::types::ump_stream::endpoint_info_notification,
  midi2::types::ump_stream::device_identity_notification,
  midi2::types::ump_stream::endpoint_name_notification,
  midi2::types::ump_stream::product_instance_id_notification,
  midi2::types::ump_stream::jr_configuration_request,
  midi2::types::ump_stream::jr_configuration_notification,
  midi2::types::ump_stream::function_block_discovery,
  midi2::types::ump_stream::function_block_info_notification,
  midi2::types::ump_stream::function_block_name_notification,
  midi2::types::ump_stream::start_of_clip,
  midi2::types::ump_stream::end_of_clip,

  midi2::types::flex_data::set_tempo,
  midi2::types::flex_data::set_time_signature,
  midi2::types::flex_data::set_metronome,
  midi2::types::flex_data::set_key_signature,
  midi2::types::flex_data::set_chord_name
>;
// clang-format on
TYPED_TEST_SUITE(UMPToMidi2PassThrough, PassThroughTypes);

// NOLINTNEXTLINE
TYPED_TEST(UMPToMidi2PassThrough, PassThrough) {
  auto const input = this->add(TypeParam{});
  auto const output = convert(input);
  EXPECT_THAT(output, ContainerEq(input));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2PassThroughExtras, Noop) {
  constexpr auto input = std::array{std::uint32_t{0}};
  auto const output = convert(input);
  EXPECT_THAT(output, IsEmpty()) << "NOOP messages should be removed";
}

// NOLINTNEXTLINE
TEST(UMPToMidi2PassThroughExtras, Unknown) {
  constexpr auto input = std::array{std::uint32_t{0xFFFFFFFF}};
  auto const output = convert(input);
  EXPECT_THAT(output, IsEmpty()) << "Unknown messages should be removed";
}

// NOLINTNEXTLINE
TEST(UMPToMidi2PassThroughExtras, Text) {
  using u32 = std::uint32_t;
  midi2::types::flex_data::text_common message;
  auto& w0 = get<0>(message.w);
  auto& w1 = get<1>(message.w);
  auto& w2 = get<2>(message.w);
  auto& w3 = get<3>(message.w);
  w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 3;
  w0.status_bank = 1;
  w0.status = 4;
  w1 = (u32{0xC2} << 24) | (u32{0xA9} << 16) | (u32{'2'} << 8) | (u32{'0'} << 0);
  w2 = (u32{'2'} << 24) | (u32{'4'} << 16) | (u32{' '} << 8) | (u32{'P'} << 0);
  w3 = (u32{'B'} << 24) | (u32{'H'} << 16) | (u32{'\0'} << 8) | (u32{'\0'} << 0);

  auto input =
      std::array{std::bit_cast<u32>(w0), std::bit_cast<u32>(w1), std::bit_cast<u32>(w2), std::bit_cast<u32>(w3)};
  auto const output = convert(input);
  EXPECT_THAT(output, ElementsAreArray(input));
}

void NeverCrashes(std::uint8_t group, std::vector<std::uint32_t> const& packets) {
  if (group > 0xF) {
    return;
  }
  // This test simply gets ump_to_midi2 to consume a random buffer.
  midi2::ump_to_midi2 ump2m2{group};
  for (auto const b : packets) {
    ump2m2.push(b);
    while (!ump2m2.empty()) {
      (void)ump2m2.pop();
    }
  }
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPToMidi2Fuzz, NeverCrashes);
#endif
TEST(UMPToMidi2Fuzz, Empty) {
  NeverCrashes(0, {});
}

}  // end anonymous namespace
