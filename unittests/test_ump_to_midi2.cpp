// DUT
#include "midi2/ump/ump_to_midi2.hpp"
#include "midi2/ump/ump_types.hpp"

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
  midi2::ump::ump_to_midi2 ump2m2{0};
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
  constexpr auto note = 64U;
  constexpr auto in = midi2::ump::m1cvm::note_off{}.group(0).channel(0).note(note).velocity(0x60);
  constexpr auto expected =
      midi2::ump::m2cvm::note_off{}.group(0).channel(0).note(note).attribute_type(0).velocity(0xC104).attribute(0);

  std::array const input{get<0>(in).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(expected).word(), get<1>(expected).word()));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, NoteOn) {
  constexpr auto note = 64U;

  constexpr auto in = midi2::ump::m1cvm::note_on{}.group(0).channel(0).note(note).velocity(0x60);

  constexpr auto expected =
      midi2::ump::m2cvm::note_on{}.group(0).channel(0).note(note).attribute_type(0).velocity(0xC104).attribute(0);

  std::array const input{get<0>(in).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(expected).word(), get<1>(expected).word()));
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
    constexpr auto in_non_1 =
        midi2::ump::m1cvm::note_on{}.group(group).channel(channel).note(note_number).velocity(velocity);
    input.push_back(get<0>(in_non_1).word());
  }
  {
    constexpr auto in_non_2 = midi2::ump::m1cvm::note_on{}.group(group).channel(channel).note(note_number).velocity(0);
    input.push_back(get<0>(in_non_2).word());
  }

  std::vector<std::uint32_t> expected;
  {
    constexpr auto expected_non = midi2::ump::m2cvm::note_on{}
                                      .group(group)
                                      .channel(channel)
                                      .note(note_number)
                                      .velocity(midi2::ump::mcm_scale<7, 16>(velocity));
    expected.push_back(get<0>(expected_non).word());
    expected.push_back(get<1>(expected_non).word());
  }
  {
    constexpr auto expected_noff =
        midi2::ump::m2cvm::note_on{}.group(group).channel(channel).note(note_number).velocity(0);
    expected.push_back(get<0>(expected_noff).word());
    expected.push_back(get<1>(expected_noff).word());
  }

  auto const actual = convert(input);
  EXPECT_THAT(actual, ElementsAreArray(expected));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, PolyPressure) {
  constexpr auto note = 64;

  midi2::ump::m1cvm::poly_pressure in;
  in.group(0);
  in.channel(0);
  in.note(note);
  in.pressure(0x60);

  midi2::ump::m2cvm::poly_pressure expected;
  expected.group(0);
  expected.channel(0);
  expected.note(note);
  expected.pressure(midi2::ump::mcm_scale<7, 32>(in.pressure()));

  std::array const input{get<0>(in).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(expected).word(), get<1>(expected).word()));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, PitchBend) {
  constexpr auto pb14 = 0b0010101010101010U;  // A 14-bit value for the pitch bend

  midi2::ump::m1cvm::pitch_bend m1;
  m1.group(0);
  m1.channel(0);
  m1.lsb_data(pb14 & ((1 << 7) - 1));
  m1.msb_data(pb14 >> 7);

  midi2::ump::m2cvm::pitch_bend m2;
  m2.group(0);
  m2.channel(0);
  m2.value(midi2::ump::mcm_scale<14, 32>(pb14));

  std::array const input{get<0>(m1).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(m2).word(), get<1>(m2).word()));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, ChannelPressure) {
  constexpr auto pressure = 0b0101010;
  constexpr auto group = std::uint8_t{3};
  constexpr auto channel = std::uint8_t{7};

  midi2::ump::m1cvm::channel_pressure m1;
  m1.group(group);
  m1.channel(channel);
  m1.data(pressure);

  midi2::ump::m2cvm::channel_pressure m2;
  m2.group(group);
  m2.channel(channel);
  m2.value(midi2::ump::mcm_scale<7, 32>(pressure));

  std::array const input{get<0>(m1).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(m2).word(), get<1>(m2).word()));
}
// NOLINTNEXTLINE
TEST(UMPToMidi2, SimpleContinuousController) {
  constexpr auto controller = 0b01100110;
  constexpr auto value = 0b01010101;
  constexpr auto group = std::uint8_t{3};
  constexpr auto channel = std::uint8_t{7};

  midi2::ump::m1cvm::control_change m1;
  m1.group(group);
  m1.channel(channel);
  m1.controller(controller);
  m1.value(value);

  midi2::ump::m2cvm::control_change m2;
  m2.group(group);
  m2.channel(channel);
  m2.controller(controller);
  m2.value(midi2::ump::mcm_scale<7, 32>(value));

  std::array const input{get<0>(m1).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(m2).word(), get<1>(m2).word()));
}

// NOLINTNEXTLINE
TEST(UMPToMidi2, SimpleProgramChange) {
  constexpr auto program = std::uint8_t{0b01010101};
  constexpr auto group = std::uint8_t{0x1};
  constexpr auto channel = std::uint8_t{0xF};

  midi2::ump::m1cvm::program_change m1;
  m1.group(group);
  m1.channel(channel);
  m1.program(program);

  midi2::ump::m2cvm::program_change m2;
  m2.group(group);
  m2.channel(channel);
  m2.option_flags(0);
  m2.bank_valid(false);
  m2.program(program);
  m2.bank_msb(0);
  m2.bank_lsb(0);

  std::array const input{get<0>(m1).word()};
  EXPECT_THAT(convert(input), ElementsAre(get<0>(m2).word(), get<1>(m2).word()));
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
    constexpr auto m1cc_bank_msb = midi2::ump::m1cvm::control_change{}
                                       .group(group)
                                       .channel(channel)
                                       .controller(midi2::ump::control::bank_select)
                                       .value(bank_msb);
    input.push_back(get<0>(m1cc_bank_msb).word());
  }
  {
    constexpr auto mc11_bank_lsb = midi2::ump::m1cvm::control_change{}
                                       .group(group)
                                       .channel(channel)
                                       .controller(midi2::ump::control::bank_select_lsb)
                                       .value(bank_lsb);
    input.push_back(get<0>(mc11_bank_lsb).word());
  }
  {
    constexpr auto m1 = midi2::ump::m1cvm::program_change{}.group(group).channel(channel).program(program);
    input.push_back(get<0>(m1).word());
  }

  midi2::ump::m2cvm::program_change m2;
  m2.group(group);
  m2.channel(channel);
  m2.option_flags(0);
  m2.bank_valid(true);
  m2.program(program);
  m2.bank_msb(bank_msb);
  m2.bank_lsb(bank_lsb);
  EXPECT_THAT(convert(input), ElementsAre(get<0>(m2).word(), get<1>(m2).word()));
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
    constexpr auto pn_msb = midi2::ump::m1cvm::control_change{}
                                .group(group)
                                .channel(channel)
                                .controller(midi2::ump::control::rpn_msb)
                                .value(control_msb);
    input.push_back(get<0>(pn_msb).word());
  }
  {
    constexpr auto pn_lsb = midi2::ump::m1cvm::control_change{}
                                .group(group)
                                .channel(channel)
                                .controller(midi2::ump::control::rpn_lsb)
                                .value(control_lsb);
    input.push_back(get<0>(pn_lsb).word());
  }
  {
    constexpr auto param_value_msb = midi2::ump::m1cvm::control_change{}
                                         .group(group)
                                         .channel(channel)
                                         .controller(midi2::ump::control::data_entry_msb)
                                         .value(value_msb);
    input.push_back(get<0>(param_value_msb).word());
  }
  {
    constexpr auto param_value_lsb = midi2::ump::m1cvm::control_change{}
                                         .group(group)
                                         .channel(channel)
                                         .controller(midi2::ump::control::data_entry_lsb)
                                         .value(value_lsb);
    input.push_back(get<0>(param_value_lsb).word());
  }
  {
    constexpr auto null_msb = midi2::ump::m1cvm::control_change{}
                                  .group(group)
                                  .channel(channel)
                                  .controller(midi2::ump::control::rpn_msb)
                                  .value(0x7F);
    input.push_back(get<0>(null_msb).word());
  }
  {
    constexpr auto null_lsb = midi2::ump::m1cvm::control_change{}
                                  .group(group)
                                  .channel(channel)
                                  .controller(midi2::ump::control::rpn_lsb)
                                  .value(0x7F);
    input.push_back(get<0>(null_lsb).word());
  }

  constexpr auto m2 =
      midi2::ump::m2cvm::rpn_controller{}
          .group(group)
          .channel(channel)
          .bank(control_msb)
          .index(control_lsb)
          .value(midi2::ump::mcm_scale<14, 32>((std::uint32_t{value_msb} << 7) | std::uint32_t{value_lsb}));
  EXPECT_THAT(convert(input), ElementsAre(get<0>(m2).word(), get<1>(m2).word()));
}

TEST(UMPToMidi2, ControlChangeNRPN) {
  constexpr auto group = std::uint8_t{0x1};
  constexpr auto channel = std::uint8_t{0xF};
  constexpr auto control_msb = std::uint8_t{0b01010101};
  constexpr auto control_lsb = std::uint8_t{0b01101010};
  constexpr auto value_msb = std::uint8_t{0b00011001};
  constexpr auto value_lsb = std::uint8_t{0b01100110};

  std::vector<std::uint32_t> input;

  using enum midi2::ump::control;
  {
    auto const pn_msb =
        midi2::ump::m1cvm::control_change{}.group(group).channel(channel).controller(nrpn_msb).value(control_msb);
    input.push_back(get<0>(pn_msb).word());
  }
  {
    auto const pn_lsb =
        midi2::ump::m1cvm::control_change{}.group(group).channel(channel).controller(nrpn_lsb).value(control_lsb);
    input.push_back(get<0>(pn_lsb).word());
  }
  {
    auto const param_value_msb =
        midi2::ump::m1cvm::control_change{}.group(group).channel(channel).controller(data_entry_msb).value(value_msb);
    input.push_back(get<0>(param_value_msb).word());
  }
  {
    auto const param_value_lsb =
        midi2::ump::m1cvm::control_change{}.group(group).channel(channel).controller(data_entry_lsb).value(value_lsb);
    input.push_back(get<0>(param_value_lsb).word());
  }
  {
    auto const null_msb =
        midi2::ump::m1cvm::control_change{}.group(group).channel(channel).controller(nrpn_msb).value(0x7F);
    input.push_back(get<0>(null_msb).word());
  }
  {
    auto const null_lsb =
        midi2::ump::m1cvm::control_change{}.group(group).channel(channel).controller(nrpn_lsb).value(0x7F);
    input.push_back(get<0>(null_lsb).word());
  }

  auto const m2 = midi2::ump::m2cvm::nrpn_controller{}
                      .group(group)
                      .channel(channel)
                      .bank(control_msb)
                      .index(control_lsb)
                      .value(midi2::ump::mcm_scale<14, 32>((std::uint32_t{value_msb} << 7) | std::uint32_t{value_lsb}));
  EXPECT_THAT(convert(input), ElementsAre(get<0>(m2).word(), get<1>(m2).word()));
}

template <typename T> class UMPToMidi2PassThrough : public testing::Test {
public:
  template <typename T2> static std::vector<std::uint32_t> add(T2 const& ump) {
    std::vector<std::uint32_t> result;
    result.reserve(std::tuple_size_v<T2>);
    midi2::ump::apply(ump, [&result](auto const v) {
      result.push_back(std::uint32_t{v});
      return false;
    });
    return result;
  }
};

// clang-format off
using PassThroughTypes = ::testing::Types<
  midi2::ump::utility::jr_clock,
  midi2::ump::utility::jr_timestamp,
  midi2::ump::utility::delta_clockstamp_tpqn,
  midi2::ump::utility::delta_clockstamp,

  midi2::ump::system::midi_time_code,
  midi2::ump::system::song_position_pointer,
  midi2::ump::system::song_select,
  midi2::ump::system::tune_request,
  midi2::ump::system::timing_clock,
  midi2::ump::system::sequence_start,
  midi2::ump::system::sequence_continue,
  midi2::ump::system::sequence_stop,
  midi2::ump::system::active_sensing,
  midi2::ump::system::reset,

  midi2::ump::data64::sysex7_in_1,
  midi2::ump::data64::sysex7_start,
  midi2::ump::data64::sysex7_continue,
  midi2::ump::data64::sysex7_end,

  midi2::ump::m2cvm::note_off,
  midi2::ump::m2cvm::note_on,
  midi2::ump::m2cvm::poly_pressure,
  midi2::ump::m2cvm::program_change,
  midi2::ump::m2cvm::channel_pressure,
  midi2::ump::m2cvm::rpn_controller,
  midi2::ump::m2cvm::nrpn_controller,
  midi2::ump::m2cvm::rpn_per_note_controller,
  midi2::ump::m2cvm::nrpn_per_note_controller,
  midi2::ump::m2cvm::rpn_relative_controller,
  midi2::ump::m2cvm::nrpn_relative_controller,
  midi2::ump::m2cvm::per_note_management,
  midi2::ump::m2cvm::control_change,
  midi2::ump::m2cvm::pitch_bend,
  midi2::ump::m2cvm::per_note_pitch_bend,

  midi2::ump::data128::sysex8_in_1,
  midi2::ump::data128::sysex8_start,
  midi2::ump::data128::sysex8_continue,
  midi2::ump::data128::sysex8_end,
  midi2::ump::data128::mds_header,
  midi2::ump::data128::mds_payload,

  midi2::ump::stream::endpoint_discovery,
  midi2::ump::stream::endpoint_info_notification,
  midi2::ump::stream::device_identity_notification,
  midi2::ump::stream::endpoint_name_notification,
  midi2::ump::stream::product_instance_id_notification,
  midi2::ump::stream::jr_configuration_request,
  midi2::ump::stream::jr_configuration_notification,
  midi2::ump::stream::function_block_discovery,
  midi2::ump::stream::function_block_info_notification,
  midi2::ump::stream::function_block_name_notification,
  midi2::ump::stream::start_of_clip,
  midi2::ump::stream::end_of_clip,

  midi2::ump::flex_data::set_tempo,
  midi2::ump::flex_data::set_time_signature,
  midi2::ump::flex_data::set_metronome,
  midi2::ump::flex_data::set_key_signature,
  midi2::ump::flex_data::set_chord_name
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
  auto const message = midi2::ump::flex_data::text_common{}
                           .group(0)
                           .form(0)
                           .addrs(1)
                           .channel(3)
                           .status_bank(1)
                           .status(4)
                           .value1((u32{0xC2} << 24) | (u32{0xA9} << 16) | (u32{'2'} << 8) | (u32{'0'} << 0))
                           .value2((u32{'2'} << 24) | (u32{'4'} << 16) | (u32{' '} << 8) | (u32{'P'} << 0))
                           .value3((u32{'B'} << 24) | (u32{'H'} << 16) | (u32{'\0'} << 8) | (u32{'\0'} << 0));

  auto input =
      std::array{get<0>(message).word(), get<1>(message).word(), get<2>(message).word(), get<3>(message).word()};
  auto const output = convert(input);
  EXPECT_THAT(output, ElementsAreArray(input));
}

void NeverCrashes(std::uint8_t group, std::vector<std::uint32_t> const& packets) {
  if (group > 0xF) {
    return;
  }
  // This test simply gets ump_to_midi2 to consume a random buffer.
  midi2::ump::ump_to_midi2 ump2m2{group};
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
