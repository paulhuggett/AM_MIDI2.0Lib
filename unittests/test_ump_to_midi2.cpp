// DUT
#include "midi2/ump_to_midi2.hpp"

// Standard Library
#include <array>
#include <ranges>
#include <vector>

// Google Test/Mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::ContainerEq;
using testing::ElementsAre;

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

template <typename T> class UMPToMidi2PassThrough : public testing::Test {
public:
  template <typename T2>
    requires(std::tuple_size_v<decltype(T2::w)> == 1)
  std::vector<std::uint32_t> add(T2 const& ump) {
    return {std::bit_cast<std::uint32_t>(get<0>(ump.w))};
  }

  template <typename T2>
    requires(std::tuple_size_v<decltype(T2::w)> == 2)
  std::vector<std::uint32_t> add(T2 const& ump) {
    return {std::bit_cast<std::uint32_t>(get<0>(ump.w)), std::bit_cast<std::uint32_t>(get<1>(ump.w))};
  }

  template <typename T2>
    requires(std::tuple_size_v<decltype(T2::w)> == 4)
  std::vector<std::uint32_t> add(T2 const& ump) {
    return {std::bit_cast<std::uint32_t>(get<0>(ump.w)), std::bit_cast<std::uint32_t>(get<1>(ump.w)),
            std::bit_cast<std::uint32_t>(get<2>(ump.w)), std::bit_cast<std::uint32_t>(get<3>(ump.w))};
  }
};

// clang-format off
using PassThroughTypes = ::testing::Types<
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
  // TODO: text common format
  //  midi2::types::flex_data::text_common
>;
// clang-format on
TYPED_TEST_SUITE(UMPToMidi2PassThrough, PassThroughTypes);

// NOLINTNEXTLINE
TYPED_TEST(UMPToMidi2PassThrough, PassThrough) {
  auto const input = this->add(TypeParam{});
  auto const output = convert(input);
  EXPECT_THAT(input, ContainerEq(output));
}

}  // end anonymous namespace
