//===-- UMP -------------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/ump/ump_types.hpp"

// Google test
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

template <typename T> class UMPCheckFixture : public testing::Test {
public:
  T value_;
};
using ump_types = ::testing::Types<
    midi2::ump::utility::noop, midi2::ump::utility::jr_clock, midi2::ump::utility::jr_timestamp,
    midi2::ump::utility::delta_clockstamp_tpqn, midi2::ump::utility::delta_clockstamp,

    midi2::ump::system::midi_time_code, midi2::ump::system::song_position_pointer, midi2::ump::system::song_select,
    midi2::ump::system::tune_request, midi2::ump::system::timing_clock, midi2::ump::system::sequence_start,
    midi2::ump::system::sequence_continue, midi2::ump::system::sequence_stop, midi2::ump::system::active_sensing,
    midi2::ump::system::reset,

    midi2::ump::m1cvm::note_on, midi2::ump::m1cvm::note_off, midi2::ump::m1cvm::poly_pressure,
    midi2::ump::m1cvm::control_change, midi2::ump::m1cvm::program_change, midi2::ump::m1cvm::channel_pressure,
    midi2::ump::m1cvm::pitch_bend,

    midi2::ump::m2cvm::note_off, midi2::ump::m2cvm::note_on, midi2::ump::m2cvm::poly_pressure,
    midi2::ump::m2cvm::rpn_per_note_controller, midi2::ump::m2cvm::nrpn_per_note_controller,
    midi2::ump::m2cvm::rpn_controller, midi2::ump::m2cvm::nrpn_controller, midi2::ump::m2cvm::rpn_relative_controller,
    midi2::ump::m2cvm::nrpn_relative_controller, midi2::ump::m2cvm::per_note_management,
    midi2::ump::m2cvm::control_change, midi2::ump::m2cvm::program_change, midi2::ump::m2cvm::channel_pressure,
    midi2::ump::m2cvm::pitch_bend, midi2::ump::m2cvm::per_note_pitch_bend,

    midi2::ump::stream::endpoint_discovery, midi2::ump::stream::endpoint_info_notification,
    midi2::ump::stream::device_identity_notification, midi2::ump::stream::endpoint_name_notification,
    midi2::ump::stream::product_instance_id_notification, midi2::ump::stream::jr_configuration_request,
    midi2::ump::stream::jr_configuration_notification, midi2::ump::stream::function_block_discovery,
    midi2::ump::stream::function_block_info_notification, midi2::ump::stream::function_block_name_notification,
    midi2::ump::stream::start_of_clip, midi2::ump::stream::end_of_clip,

    midi2::ump::flex_data::set_tempo, midi2::ump::flex_data::set_time_signature, midi2::ump::flex_data::set_metronome,
    midi2::ump::flex_data::set_key_signature, midi2::ump::flex_data::set_chord_name, midi2::ump::flex_data::text_common,

    midi2::ump::data128::sysex8_in_1, midi2::ump::data128::sysex8_start, midi2::ump::data128::sysex8_continue,
    midi2::ump::data128::sysex8_end, midi2::ump::data128::mds_header, midi2::ump::data128::mds_payload>;
TYPED_TEST_SUITE(UMPCheckFixture, ump_types);
TYPED_TEST(UMPCheckFixture, Check) {
  ASSERT_TRUE(midi2::ump::check(this->value_));
}

TEST(UMPTypes, Sysex7InOne) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4);
  message2[0] = 0x7E;
  message2[1] = 0x7F;
  message2[2] = 0x07;
  message2[3] = 0x0D;
  EXPECT_EQ(message, message2);
}
TEST(UMPTypes, Sysex7InOneInitializerList) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D});
  EXPECT_EQ(message, message2);
}
TEST(UMPTypes, Sysex7InOneRange) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr std::array values{0x7E, 0x7F, 0x07, 0x0D};
  auto const message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data(values);
  EXPECT_EQ(message, message2);
}
TEST(UMPTypes, Sysex7InOneIteratorAssignData) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr std::array values{0x7E, 0x7F, 0x07, 0x0D};
  auto const message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data(values.begin(), values.end());
  EXPECT_EQ(message, message2);
}
TEST(UMPTypes, Sysex7InOneIteratorRead) {
  auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D});
  auto first = message.begin();
  auto last = message.end();
  EXPECT_EQ(std::distance(first, last), 4U);
  EXPECT_EQ(*first, 0x7E);
  ++first;
  EXPECT_EQ(*first, 0x7F);
  ++first;
  EXPECT_EQ(*first, 0x07);
  ++first;
  EXPECT_EQ(*first, 0x0D);
  ++first;
  EXPECT_EQ(first, last);
}
TEST(UMPTypes, Sysex7InOneConstIteratorRead) {
  auto const message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D});
  auto first = message.begin();
  auto last = message.end();
  EXPECT_EQ(std::distance(first, last), 4U);
  EXPECT_EQ(*first, 0x7E);
  ++first;
  EXPECT_EQ(*first, 0x7F);
  ++first;
  EXPECT_EQ(*first, 0x07);
  ++first;
  EXPECT_EQ(*first, 0x0D);
  ++first;
  EXPECT_EQ(first, last);
}
TEST(UMPTypes, Sysex7InOneIteratorWrite) {
  auto message = midi2::ump::data64::sysex7_in_1{}.group(0);
  EXPECT_TRUE(message.empty());
  EXPECT_EQ(message.size(), 0U);
  message.data({1, 2, 3});
  EXPECT_FALSE(message.empty());
  EXPECT_EQ(message.size(), 3U);
  auto first = message.begin();
  auto last = message.end();
  *first = 4;
  EXPECT_EQ(*first, 4);
  ++first;
  EXPECT_EQ(*first, 2);
  ++first;
  EXPECT_EQ(*first, 3);
  ++first;
  EXPECT_EQ(first, last);
}
TEST(UMPTypes, Sysex7InOneIteratorWriteUsingAlgorithm) {
  using testing::ElementsAreArray;

  auto message = midi2::ump::data64::sysex7_in_1{}.group(0);
  constexpr std::array src{0x1, 0x3, 0x5, 0x7};
  std::ranges::copy(src, message.begin());
  message.number_of_bytes(src.size());

  EXPECT_THAT(message, ElementsAreArray(src));
}
TEST(UMPTypes, Sysex7InOneReadArray) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x00);
  EXPECT_EQ(message[5], 0x00);
}

TEST(UMPTypes, Sysex8InOne) {
  constexpr auto message =
      midi2::ump::data128::sysex8_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  auto message2 = midi2::ump::data128::sysex8_in_1{}.group(0).number_of_bytes(4);
  message2[0] = 0x7E;
  message2[1] = 0x7F;
  message2[2] = 0x07;
  message2[3] = 0x0D;
  EXPECT_EQ(message, message2);
}
TEST(UMPTypes, Sysex8InOneReadArray) {
  constexpr auto message =
      midi2::ump::data128::sysex8_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x00);
  EXPECT_EQ(message[5], 0x00);
}

}  // end anonymous namespace
