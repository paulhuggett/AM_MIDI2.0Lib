//===-- UMP -------------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// Force the UMP text-common message to include the str() method.
#define MIDI2_TEXT_COMMON_STRING (1)

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

TEST(UMPTypesSysex7, AssignIndividually) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4);
  message2[0] = 0x7E;
  message2[1] = 0x7F;
  message2[2] = 0x07;
  message2[3] = 0x0D;
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, InitializerList) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D});
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, Range) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr std::array values{0x7E, 0x7F, 0x07, 0x0D};
  auto const message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data(values);
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, IteratorAssignData) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr std::array values{0x7E, 0x7F, 0x07, 0x0D};
  auto const message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data(values.begin(), values.end());
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, IteratorRead) {
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
TEST(UMPTypesSysex7, ConstIteratorRead) {
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
TEST(UMPTypesSysex7, IteratorWrite) {
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
TEST(UMPTypesSysex7, IteratorWriteUsingAlgorithm) {
  using testing::ElementsAreArray;

  auto message = midi2::ump::data64::sysex7_in_1{}.group(0);
  constexpr std::array src{0x1, 0x3, 0x5, 0x7};
  std::ranges::copy(src, message.begin());
  using count_type = midi2::ump::data64::sysex7_in_1::word0::number_of_bytes::uinteger;
  message.number_of_bytes(static_cast<count_type>(src.size()));
  EXPECT_THAT(message, ElementsAreArray(src));
}
TEST(UMPTypesSysex7, IteratorSubtract) {
  using testing::ElementsAreArray;

  auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x1, 0x3, 0x5, 0x7});
  auto first = std::begin(message);
  auto last = std::end(message);
  EXPECT_EQ(last - first, 4U);
}
TEST(UMPTypesSysex7, ReadArray) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x00);
  EXPECT_EQ(message[5], 0x00);
}

TEST(UMPTypesSysex8, AssignIndividually) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message = sysex8_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  auto message2 = sysex8_in_1{}.group(0).number_of_bytes(4);
  message2[0] = 0x7E;
  message2[1] = 0x7F;
  message2[2] = 0x07;
  message2[3] = 0x0D;
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex8, AllElements) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message = sysex8_in_1{}
                               .group(0)
                               .number_of_bytes(13)
                               .data0(0x7E)
                               .data1(0x7F)
                               .data2(0x07)
                               .data3(0x0D)
                               .data4(0x01)
                               .data5(0x02)
                               .data6(0x03)
                               .data7(0x04)
                               .data8(0x05)
                               .data9(0x06)
                               .data10(0x07)
                               .data11(0x08)
                               .data12(0x09);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x01);
  EXPECT_EQ(message[5], 0x02);
  EXPECT_EQ(message[6], 0x03);
  EXPECT_EQ(message[7], 0x04);
  EXPECT_EQ(message[8], 0x05);
  EXPECT_EQ(message[9], 0x06);
  EXPECT_EQ(message[10], 0x07);
  EXPECT_EQ(message[11], 0x08);
  EXPECT_EQ(message[12], 0x09);
}
TEST(UMPTypesSysex8, InitializerList) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message = sysex8_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D});
  ASSERT_EQ(message.number_of_bytes(), 4U);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
}
TEST(UMPTypesSysex8, InitializerListMaxLength) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message =
      sysex8_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09});
  ASSERT_EQ(message.number_of_bytes(), 13U);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x01);
  EXPECT_EQ(message[5], 0x02);
  EXPECT_EQ(message[6], 0x03);
  EXPECT_EQ(message[7], 0x04);
  EXPECT_EQ(message[8], 0x05);
  EXPECT_EQ(message[9], 0x06);
  EXPECT_EQ(message[10], 0x07);
  EXPECT_EQ(message[11], 0x08);
  EXPECT_EQ(message[12], 0x09);
}
TEST(UMPTypesSysex8, IteratorWriteUsingAlgorithm) {
  using testing::ElementsAreArray;
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  auto message = sysex8_in_1{}.group(0);
  constexpr std::array src{0x1, 0x3, 0x5, 0x7};
  std::ranges::copy(src, message.begin());
  using count_type = sysex8_in_1::word0::number_of_bytes::uinteger;
  message.number_of_bytes(static_cast<count_type>(src.size()));
  EXPECT_THAT(message, ElementsAreArray(src));
}
TEST(UMPTypesSysex8, ReadArray) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message = sysex8_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x00);
  EXPECT_EQ(message[5], 0x00);
  EXPECT_EQ(message[6], 0x00);
  EXPECT_EQ(message[7], 0x00);
  EXPECT_EQ(message[8], 0x00);
  EXPECT_EQ(message[9], 0x00);
  EXPECT_EQ(message[10], 0x00);
  EXPECT_EQ(message[11], 0x00);
  EXPECT_EQ(message[12], 0x00);
}

TEST(UMPTypesTextCommon, InitializerList) {
  using text_common = midi2::ump::flex_data::text_common;
  constexpr auto message = text_common{}.group(0).data({'a', 'b', 'c', 'd'});
  ASSERT_EQ(message.number_of_bytes(), 4U);
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), u8"abcd");
#endif
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], '\0');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
TEST(UMPTypesTextCommon, InitializerListMaxLength) {
  using text_common = midi2::ump::flex_data::text_common;
  constexpr auto message = text_common{}.group(0).data({'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'});
  EXPECT_EQ(message.number_of_bytes(), 12U);
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), u8"abcdefghijkl");
#endif
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], 'h');
  EXPECT_EQ(message[8], 'i');
  EXPECT_EQ(message[9], 'j');
  EXPECT_EQ(message[10], 'k');
  EXPECT_EQ(message[11], 'l');
}
TEST(UMPTypesTextCommon, DataSetByIteratorSentinel) {
  using namespace std::literals::string_view_literals;
  using text_common = midi2::ump::flex_data::text_common;
  constexpr auto str = u8"hello"sv;
  constexpr auto message = text_common{}.group(0).data(std::begin(str), std::end(str));
  ASSERT_EQ(message.number_of_bytes(), 5U);
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), str);
#endif
  EXPECT_EQ(message[0], 'h');
  EXPECT_EQ(message[1], 'e');
  EXPECT_EQ(message[2], 'l');
  EXPECT_EQ(message[3], 'l');
  EXPECT_EQ(message[4], 'o');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
TEST(UMPTypesTextCommon, IteratorWriteUsingAlgorithm) {
  using testing::ElementsAreArray;
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0);
  constexpr std::array src{'a', 'b', 'c', 'd'};
  std::ranges::copy(src, message.begin());
  EXPECT_THAT(message, ElementsAreArray(src));
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), u8"abcd");
#endif
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], '\0');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
TEST(UMPTypesTextCommon, IteratorWriteUsingAlgorithmMaxLength) {
  using testing::ElementsAreArray;
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0);
  constexpr std::array src{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'};
  std::ranges::copy(src, message.begin());
  EXPECT_THAT(message, ElementsAreArray(src));
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), u8"abcdefghijkl");
#endif
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], 'h');
  EXPECT_EQ(message[8], 'i');
  EXPECT_EQ(message[9], 'j');
  EXPECT_EQ(message[10], 'k');
  EXPECT_EQ(message[11], 'l');
}
TEST(UMPTypesTextCommon, MaxSize) {
  EXPECT_EQ(midi2::ump::flex_data::text_common{}.max_size(), 12U);
}
TEST(UMPTypesTextCommon, ReadArray) {
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0).data({'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'});
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], 'h');
  EXPECT_EQ(message[8], 'i');
  EXPECT_EQ(message[9], 'j');
  EXPECT_EQ(message[10], 'k');
  EXPECT_EQ(message[11], 'l');
  message.data({'a', 'b'});
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], '\0');
  EXPECT_EQ(message[3], '\0');
  EXPECT_EQ(message[4], '\0');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
TEST(UMPTypesTextCommon, String) {
  using text_common = midi2::ump::flex_data::text_common;
  constexpr auto message = text_common{}.group(0).data({'a', 'b', 'c', 'd', 'e', 'f', 'g'});
  EXPECT_EQ(message.str(), u8"abcdefg");
}
#endif  // MIDI2_TEXT_COMMON_STRING
TEST(UMPTypesTextCommon, FromStringView) {
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0).data("abcdefg");
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
  message.data("ab");
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], '\0');
  EXPECT_EQ(message[3], '\0');
  EXPECT_EQ(message[4], '\0');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
TEST(UMPTypesTextCommon, FromStringTooLong) {
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0).data("abcdefghijklmnopqrstuvwxyz");
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], 'h');
  EXPECT_EQ(message[8], 'i');
  EXPECT_EQ(message[9], 'j');
  EXPECT_EQ(message[10], 'k');
  EXPECT_EQ(message[11], 'l');
}

}  // end anonymous namespace
