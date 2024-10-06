//===-- UMP Processor ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/umpProcessor.hpp"
#include "midi2/bitfield.hpp"
#include "midi2/ump_types.hpp"

// Standard library
#include <algorithm>
#include <bit>
#include <functional>
#include <numeric>

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace midi2 {

std::ostream& operator<<(std::ostream& os, ump_common const& common);
std::ostream& operator<<(std::ostream& os, ump_common const& common) {
  return os << "{ group=" << static_cast<unsigned>(common.group)
            << ", messageType=" << static_cast<unsigned>(common.messageType)
            << ", status=" << static_cast<unsigned>(common.status) << " }";
};

std::ostream& operator<<(std::ostream& os, ump_data const& data);
std::ostream& operator<<(std::ostream& os, ump_data const& data) {
  os << "{ common:" << data.common
     << ", streamId=" << static_cast<unsigned>(data.streamId)
     << ", form=" << static_cast<unsigned>(data.form) << ", data=[";
  std::copy(std::begin(data.data), std::end(data.data),
            std::ostream_iterator<unsigned>(os, ","));
  os << "] }";
  return os;
}

}  // end namespace midi2

namespace {

using midi2::pack;

using context_type = int;

class MockCallbacks : public midi2::callbacks_base {
public:
  MOCK_METHOD(void, system, (midi2::types::system_general), (override));
  MOCK_METHOD(void, unknown, (std::span<std::uint32_t>), (override));
};
class UtilityMocks : public midi2::utility_base<context_type> {
public:
  MOCK_METHOD(void, noop, (context_type), (override));
  MOCK_METHOD(void, jr_clock, (context_type, midi2::types::jr_clock), (override));
  MOCK_METHOD(void, jr_timestamp, (context_type, midi2::types::jr_clock), (override));
  MOCK_METHOD(void, delta_clockstamp_tpqn, (context_type, midi2::types::jr_clock), (override));
  MOCK_METHOD(void, delta_clockstamp, (context_type, midi2::types::delta_clockstamp), (override));
};
class M1CVMMocks : public midi2::m1cvm_base<context_type> {
public:
  MOCK_METHOD(void, note_off, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, note_on, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, poly_pressure, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, control_change, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, program_change, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, channel_pressure, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, pitch_bend, (context_type, midi2::types::m1cvm_w0), (override));
};
class Data64Mocks : public midi2::data64_base<context_type> {
public:
  MOCK_METHOD(void, sysex7_in_1, (context_type, midi2::types::data64::sysex7_w0, midi2::types::data64::sysex7_w1),
              (override));
  MOCK_METHOD(void, sysex7_start, (context_type, midi2::types::data64::sysex7_w0, midi2::types::data64::sysex7_w1),
              (override));
  MOCK_METHOD(void, sysex7_continue, (context_type, midi2::types::data64::sysex7_w0, midi2::types::data64::sysex7_w1),
              (override));
  MOCK_METHOD(void, sysex7_end, (context_type, midi2::types::data64::sysex7_w0, midi2::types::data64::sysex7_w1),
              (override));
};
class M2CVMMocks : public midi2::m2cvm_base<context_type> {
public:
  MOCK_METHOD(void, note_off, (context_type, midi2::types::m2cvm::note_w0, midi2::types::m2cvm::note_w1), (override));
  MOCK_METHOD(void, note_on, (context_type, midi2::types::m2cvm::note_w0, midi2::types::m2cvm::note_w1), (override));
  MOCK_METHOD(void, poly_pressure, (context_type, midi2::types::m2cvm::poly_pressure_w0, std::uint32_t), (override));
  MOCK_METHOD(void, program_change,
              (context_type, midi2::types::m2cvm::program_change_w0, midi2::types::m2cvm::program_change_w1),
              (override));
  MOCK_METHOD(void, channel_pressure, (context_type, midi2::types::m2cvm::channel_pressure_w0, std::uint32_t),
              (override));
  MOCK_METHOD(void, rpn_controller, (context_type, midi2::types::m2cvm::controller_w0, std::uint32_t), (override));
  MOCK_METHOD(void, nrpn_controller, (context_type, midi2::types::m2cvm::controller_w0, std::uint32_t), (override));
  MOCK_METHOD(void, per_note_management, (context_type, midi2::types::m2cvm::per_note_management_w0, std::uint32_t),
              (override));
  MOCK_METHOD(void, control_change, (context_type, midi2::types::m2cvm::control_change_w0, std::uint32_t), (override));
  MOCK_METHOD(void, controller_message, (context_type, midi2::types::m2cvm::controller_message_w0, std::uint32_t),
              (override));
  MOCK_METHOD(void, pitch_bend, (context_type, midi2::types::m2cvm::pitch_bend_w0, std::uint32_t), (override));
  MOCK_METHOD(void, per_note_pitch_bend, (context_type, midi2::types::m2cvm::per_note_pitch_bend_w0, std::uint32_t),
              (override));
};
class Data128Mocks : public midi2::data128_base<context_type> {
public:
  MOCK_METHOD(void, sysex8_in_1,
              (context_type, midi2::types::data128::sysex8_w0, midi2::types::data128::sysex8_w1,
               midi2::types::data128::sysex8_w2, midi2::types::data128::sysex8_w3),
              (override));
  MOCK_METHOD(void, sysex8_start,
              (context_type, midi2::types::data128::sysex8_w0, midi2::types::data128::sysex8_w1,
               midi2::types::data128::sysex8_w2, midi2::types::data128::sysex8_w3),
              (override));
  MOCK_METHOD(void, sysex8_continue,
              (context_type, midi2::types::data128::sysex8_w0, midi2::types::data128::sysex8_w1,
               midi2::types::data128::sysex8_w2, midi2::types::data128::sysex8_w3),
              (override));
  MOCK_METHOD(void, sysex8_end,
              (context_type, midi2::types::data128::sysex8_w0, midi2::types::data128::sysex8_w1,
               midi2::types::data128::sysex8_w2, midi2::types::data128::sysex8_w3),
              (override));
  MOCK_METHOD(void, mds_header,
              (context_type, midi2::types::data128::mds_header_w0, midi2::types::data128::mds_header_w1,
               midi2::types::data128::mds_header_w2, midi2::types::data128::mds_header_w3),
              (override));
  MOCK_METHOD(void, mds_payload,
              (context_type, midi2::types::data128::mds_payload_w0, midi2::types::data128::mds_payload_w1,
               midi2::types::data128::mds_payload_w2, midi2::types::data128::mds_payload_w3),
              (override));
};
class UMPStreamMocks : public midi2::ump_stream_base<context_type> {
public:
  MOCK_METHOD(void, endpoint_discovery,
              (context_type, midi2::types::ump_stream::endpoint_discovery_w0,
               midi2::types::ump_stream::endpoint_discovery_w1, midi2::types::ump_stream::endpoint_discovery_w2,
               midi2::types::ump_stream::endpoint_discovery_w3),
              (override));
  MOCK_METHOD(void, endpoint_info_notification,
              (context_type, midi2::types::ump_stream::endpoint_info_notification_w0,
               midi2::types::ump_stream::endpoint_info_notification_w1,
               midi2::types::ump_stream::endpoint_info_notification_w2,
               midi2::types::ump_stream::endpoint_info_notification_w3),
              (override));
  MOCK_METHOD(void, device_identity_notification,
              (context_type, midi2::types::ump_stream::device_identity_notification_w0,
               midi2::types::ump_stream::device_identity_notification_w1,
               midi2::types::ump_stream::device_identity_notification_w2,
               midi2::types::ump_stream::device_identity_notification_w3),
              (override));
  MOCK_METHOD(void, endpoint_name_notification,
              (context_type, midi2::types::ump_stream::endpoint_name_notification_w0,
               midi2::types::ump_stream::endpoint_name_notification_w1,
               midi2::types::ump_stream::endpoint_name_notification_w2,
               midi2::types::ump_stream::endpoint_name_notification_w3),
              (override));
  MOCK_METHOD(void, product_instance_id_notification,
              (context_type, midi2::types::ump_stream::product_instance_id_notification_w0,
               midi2::types::ump_stream::product_instance_id_notification_w1,
               midi2::types::ump_stream::product_instance_id_notification_w2,
               midi2::types::ump_stream::product_instance_id_notification_w3),
              (override));

  MOCK_METHOD(void, jr_configuration_request,
              (context_type, midi2::types::ump_stream::jr_configuration_request_w0,
               midi2::types::ump_stream::jr_configuration_request_w1,
               midi2::types::ump_stream::jr_configuration_request_w2,
               midi2::types::ump_stream::jr_configuration_request_w3),
              (override));
  MOCK_METHOD(void, jr_configuration_notification,
              (context_type, midi2::types::ump_stream::jr_configuration_notification_w0,
               midi2::types::ump_stream::jr_configuration_notification_w1,
               midi2::types::ump_stream::jr_configuration_notification_w2,
               midi2::types::ump_stream::jr_configuration_notification_w3),
              (override));

  MOCK_METHOD(void, function_block_discovery,
              (context_type, midi2::types::ump_stream::function_block_discovery_w0,
               midi2::types::ump_stream::function_block_discovery_w1,
               midi2::types::ump_stream::function_block_discovery_w2,
               midi2::types::ump_stream::function_block_discovery_w3),
              (override));
  MOCK_METHOD(void, function_block_info_notification,
              (context_type, midi2::types::ump_stream::function_block_info_notification_w0,
               midi2::types::ump_stream::function_block_info_notification_w1,
               midi2::types::ump_stream::function_block_info_notification_w2,
               midi2::types::ump_stream::function_block_info_notification_w3),
              (override));
  MOCK_METHOD(void, function_block_name_notification,
              (context_type, midi2::types::ump_stream::function_block_name_notification_w0,
               midi2::types::ump_stream::function_block_name_notification_w1,
               midi2::types::ump_stream::function_block_name_notification_w2,
               midi2::types::ump_stream::function_block_name_notification_w3),
              (override));

  MOCK_METHOD(void, start_of_clip,
              (context_type, midi2::types::ump_stream::start_of_clip_w0, midi2::types::ump_stream::start_of_clip_w1,
               midi2::types::ump_stream::start_of_clip_w2, midi2::types::ump_stream::start_of_clip_w3),
              (override));
  MOCK_METHOD(void, end_of_clip,
              (context_type, midi2::types::ump_stream::end_of_clip_w0, midi2::types::ump_stream::end_of_clip_w1,
               midi2::types::ump_stream::end_of_clip_w2, midi2::types::ump_stream::end_of_clip_w3),
              (override));
};

class FlexDataMocks : public midi2::flex_data_base<context_type> {
public:
  MOCK_METHOD(void, set_tempo,
              (context_type, midi2::types::flex_data::set_tempo_w0, midi2::types::flex_data::set_tempo_w1,
               midi2::types::flex_data::set_tempo_w2, midi2::types::flex_data::set_tempo_w3),
              (override));
  MOCK_METHOD(void, set_time_signature,
              (context_type, midi2::types::flex_data::set_time_signature_w0,
               midi2::types::flex_data::set_time_signature_w1, midi2::types::flex_data::set_time_signature_w2,
               midi2::types::flex_data::set_time_signature_w3),
              (override));
  MOCK_METHOD(void, set_metronome,
              (context_type, midi2::types::flex_data::set_metronome_w0, midi2::types::flex_data::set_metronome_w1,
               midi2::types::flex_data::set_metronome_w2, midi2::types::flex_data::set_metronome_w3),
              (override));
  MOCK_METHOD(void, set_key_signature,
              (context_type, midi2::types::flex_data::set_key_signature_w0,
               midi2::types::flex_data::set_key_signature_w1, midi2::types::flex_data::set_key_signature_w2,
               midi2::types::flex_data::set_key_signature_w3),
              (override));
  MOCK_METHOD(void, set_chord_name,
              (context_type, midi2::types::flex_data::set_chord_name_w0, midi2::types::flex_data::set_chord_name_w1,
               midi2::types::flex_data::set_chord_name_w2, midi2::types::flex_data::set_chord_name_w3),
              (override));
  MOCK_METHOD(void, text,
              (context_type, midi2::types::flex_data::text_common_w0, midi2::types::flex_data::text_common_w1,
               midi2::types::flex_data::text_common_w2, midi2::types::flex_data::text_common_w3),
              (override));
};

}  // end anonymous namespace

namespace {

using testing::ElementsAre;
using testing::InSequence;
using testing::StrictMock;

class UMPProcessor : public testing::Test {
public:
  UMPProcessor() : processor_{std::ref(config_)} {}

  struct mocked_config {
    context_type context = 42;
    StrictMock<MockCallbacks> callbacks;
    StrictMock<M1CVMMocks> m1cvm;
    StrictMock<Data64Mocks> data64;
    StrictMock<M2CVMMocks> m2cvm;
    StrictMock<UtilityMocks> utility;
    StrictMock<Data128Mocks> data128;
    StrictMock<UMPStreamMocks> ump_stream;
    StrictMock<FlexDataMocks> flex;
  };
  mocked_config config_;
  midi2::umpProcessor<mocked_config&> processor_;
};

constexpr std::uint8_t ump_mt(midi2::ump_message_type mt) {
  auto const result = static_cast<std::uint8_t>(mt);
  assert(to_underlying(mt) == result);
  return result;
}

// NOLINTNEXTLINE
TEST_F(UMPProcessor, Noop) {
  EXPECT_CALL(config_.utility, noop(config_.context)).Times(1);

  midi2::types::noop w0{};
  w0.mt = ump_mt(midi2::ump_message_type::utility);
  w0.reserved = 0;
  w0.status = 0b0000;
  w0.data = 0;
  processor_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, JRClock) {
  midi2::types::jr_clock message{};
  message.mt = ump_mt(midi2::ump_message_type::utility);
  message.status = static_cast<std::uint8_t>(midi2::ump_utility::jr_clock);
  message.sender_clock_time = 0b1010101010101010;
  EXPECT_CALL(config_.utility, jr_clock(config_.context, message)).Times(1);
  processor_.processUMP(message);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, JRTimestamp) {
  midi2::types::jr_clock message{};
  message.mt = ump_mt(midi2::ump_message_type::utility);
  message.status = static_cast<std::uint8_t>(midi2::ump_utility::jr_ts);
  message.sender_clock_time = (1U << 16) - 1U;
  EXPECT_CALL(config_.utility, jr_timestamp(config_.context, message)).Times(1);
  processor_.processUMP(message);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, DeltaClockstampTqpn) {
  midi2::types::jr_clock message{};
  message.mt = ump_mt(midi2::ump_message_type::utility);
  message.status = static_cast<std::uint8_t>(midi2::ump_utility::delta_clock_tick);
  message.sender_clock_time = 0b1010101010101010;
  EXPECT_CALL(config_.utility, delta_clockstamp_tpqn(config_.context, message)).Times(1);
  processor_.processUMP(message);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, DeltaClockstamp) {
  midi2::types::delta_clockstamp message{};
  message.mt = ump_mt(midi2::ump_message_type::utility);
  message.status = static_cast<std::uint8_t>(midi2::ump_utility::delta_clock_since);
  message.ticks_per_quarter_note = (1U << 20) - 1U;
  EXPECT_CALL(config_.utility, delta_clockstamp(config_.context, message)).Times(1);
  processor_.processUMP(message);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, BadUtility) {
  midi2::types::delta_clockstamp message;
  message.mt = to_underlying(midi2::ump_message_type::utility);
  message.status = std::uint8_t{0b1111};
  EXPECT_CALL(config_.callbacks, unknown(ElementsAre(std::bit_cast<std::uint32_t>(message))));
  processor_.processUMP(message);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, System) {
  midi2::types::system_general sg;
  sg.mt = ump_mt(midi2::ump_message_type::system);
  sg.group = 1;
  sg.status = midi2::status::spp;
  sg.byte2 = 0x7F;
  sg.byte3 = 0x7F;
  EXPECT_CALL(config_.callbacks, system(sg)).Times(1);
  processor_.processUMP(sg);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, SystemBadStatus) {
  midi2::types::system_general sg{};
  sg.mt = ump_mt(midi2::ump_message_type::system);
  sg.group = 1;
  sg.status = 0x00;
  sg.byte2 = 0x7F;
  sg.byte3 = 0x7F;
  EXPECT_CALL(config_.callbacks, unknown(ElementsAre(std::bit_cast<std::uint32_t>(sg))));
  processor_.processUMP(sg);
}
constexpr std::uint8_t ump_cvm(midi2::status s) {
  static_assert(
      std::is_same_v<std::underlying_type_t<midi2::status>, std::uint8_t>,
      "status type must be a std::uint8_t");
  assert((s & 0x0F) == 0 &&
         "Bottom 4 bits of a channel voice message status enum must be 0");
  return std::uint8_t{s} >> 4;
}

constexpr auto ump_note_on = ump_cvm(midi2::status::note_on);

// NOLINTNEXTLINE
TEST_F(UMPProcessor, Midi1NoteOn) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint16_t{0x43};
  constexpr auto group = std::uint8_t{0};

  midi2::types::m1cvm_w0 w0{};
  w0.mt = ump_mt(midi2::ump_message_type::m1cvm);
  w0.group = group;
  w0.status = ump_cvm(midi2::status::note_on);
  w0.channel = channel;
  w0.data_a = note_number;
  w0.data_b = velocity;
  EXPECT_CALL(config_.m1cvm, note_on(config_.context, w0)).Times(1);
  processor_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, Midi1NoteOff) {
  midi2::types::m1cvm_w0 w0{};
  w0.mt = ump_mt(midi2::ump_message_type::m1cvm);
  w0.group = std::uint8_t{0};
  w0.status = ump_cvm(midi2::status::note_off);
  w0.channel = std::uint8_t{3};
  w0.data_a = std::uint8_t{60};
  w0.data_b = std::uint16_t{0x43};
  EXPECT_CALL(config_.m1cvm, note_off(config_.context, w0)).Times(1);
  processor_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, Midi1PolyPressure) {
  midi2::types::m1cvm_w0 w0{};
  w0.mt = ump_mt(midi2::ump_message_type::m1cvm);
  w0.group = std::uint8_t{0};
  w0.status = ump_cvm(midi2::status::key_pressure);
  w0.channel = std::uint8_t{3};
  w0.data_a = std::uint8_t{60};
  w0.data_b = std::uint8_t{0x43};
  EXPECT_CALL(config_.m1cvm, poly_pressure(config_.context, w0)).Times(1);
  processor_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, Midi1ControlChange) {
  midi2::types::m1cvm_w0 w0{};
  w0.mt = ump_mt(midi2::ump_message_type::m1cvm);
  w0.group = 0;
  w0.status = ump_cvm(midi2::status::cc);
  w0.channel = 3;
  w0.data_a = 60;
  w0.data_b = 127;
  EXPECT_CALL(config_.m1cvm, control_change(config_.context, w0)).Times(1);
  processor_.processUMP(w0);
}

// NOLINTNEXTLINE
TEST_F(UMPProcessor, Data64SysExIn1) {
  midi2::types::data64::sysex7_w0 w0;
  w0.mt = to_underlying(midi2::ump_message_type::data64);
  w0.group = 0;
  w0.status = to_underlying(midi2::data64::sysex7_in_1);
  w0.number_of_bytes = 4;
  w0.data0 = 2;
  w0.data1 = 3;
  midi2::types::data64::sysex7_w1 w1;
  w1.data2 = 5;
  w1.data3 = 7;
  EXPECT_CALL(config_.data64, sysex7_in_1(config_.context, w0, w1)).Times(1);
  processor_.processUMP(w0, w1);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, Data64Sysex8StartAndEnd) {
  constexpr auto group = std::uint8_t{0};

  midi2::types::data64::sysex7_w0 w0;
  w0.mt = to_underlying(midi2::ump_message_type::data64);
  w0.group = group;
  w0.status = to_underlying(midi2::data64::sysex7_start);
  w0.number_of_bytes = 6;
  w0.data0 = 2;
  w0.data1 = 3;
  midi2::types::data64::sysex7_w1 w1;
  w1.data2 = 5;
  w1.data3 = 7;
  w1.data4 = 11;
  w1.data5 = 13;

  midi2::types::data64::sysex7_w0 w2;
  w2.mt = to_underlying(midi2::ump_message_type::data64);
  w2.group = group;
  w2.status = to_underlying(midi2::data64::sysex7_continue);
  w2.number_of_bytes = 6;
  w2.data0 = 17;
  w2.data1 = 19;
  midi2::types::data64::sysex7_w1 w3;
  w3.data2 = 23;
  w3.data3 = 29;
  w3.data4 = 31;
  w3.data5 = 37;

  midi2::types::data64::sysex7_w0 w4;
  w4.mt = to_underlying(midi2::ump_message_type::data64);
  w4.group = group;
  w4.status = to_underlying(midi2::data64::sysex7_end);
  w4.number_of_bytes = 4;
  w4.data0 = 41;
  w4.data1 = 43;
  midi2::types::data64::sysex7_w1 w5;
  w5.data2 = 47;
  w5.data3 = 53;
  {
    InSequence _;
    EXPECT_CALL(config_.data64, sysex7_start(config_.context, w0, w1)).Times(1);
    EXPECT_CALL(config_.data64, sysex7_continue(config_.context, w2, w3)).Times(1);
    EXPECT_CALL(config_.data64, sysex7_end(config_.context, w4, w5)).Times(1);
  }
  processor_.processUMP(w0, w1);
  processor_.processUMP(w2, w3);
  processor_.processUMP(w4, w5);
}

// NOLINTNEXTLINE
TEST_F(UMPProcessor, Midi2NoteOn) {
  midi2::types::m2cvm::note_w0 w0;
  w0.mt = ump_mt(midi2::ump_message_type::m2cvm);
  w0.group = std::uint8_t{0};
  w0.status = 0x9;
  w0.channel = std::uint8_t{3};
  w0.note = std::uint8_t{60};
  w0.attribute = 0;

  midi2::types::m2cvm::note_w1 w1;
  w1.velocity = std::uint16_t{0x432};
  w1.attribute = 0;

  EXPECT_CALL(config_.m2cvm, note_on(config_.context, w0, w1)).Times(1);

  processor_.processUMP(w0, w1);
}

// NOLINTNEXTLINE
TEST_F(UMPProcessor, Midi2ProgramChange) {
  midi2::types::m2cvm::program_change_w0 w0;
  w0.mt = ump_mt(midi2::ump_message_type::m2cvm);
  w0.group = std::uint8_t{0};
  w0.status = ump_cvm(midi2::status::program_change);
  w0.channel = std::uint8_t{3};
  w0.reserved = 0;
  w0.option_flags = 0;
  w0.bank_valid = true;

  midi2::types::m2cvm::program_change_w1 w1;
  w1.program = 0b10101010;
  w1.reserved = 0;
  w1.r0 = 0;
  w1.bank_msb = 0b01010101;
  w1.r1 = 0;
  w1.bank_lsb = 0b00101010;

  EXPECT_CALL(config_.m2cvm, program_change(config_.context, w0, w1)).Times(1);

  processor_.processUMP(w0, w1);
}

// NOLINTNEXTLINE
TEST_F(UMPProcessor, Data128Sysex8In1) {
  constexpr auto group = std::uint8_t{0};
  constexpr auto stream_id = std::uint8_t{0};

  midi2::types::data128::sysex8 part0;
  part0.w0.mt = to_underlying(midi2::ump_message_type::data128);
  part0.w0.group = group;
  part0.w0.status = to_underlying(midi2::data128::sysex8_in_1);
  part0.w0.number_of_bytes = 10;
  part0.w0.stream_id = stream_id;
  part0.w0.data0 = 2;
  part0.w1.data1 = 3;
  part0.w1.data2 = 5;
  part0.w1.data3 = 7;
  part0.w1.data4 = 11;
  part0.w2.data5 = 13;
  part0.w2.data6 = 17;
  part0.w2.data7 = 19;
  part0.w2.data8 = 23;
  part0.w3.data9 = 29;
  EXPECT_CALL(config_.data128, sysex8_in_1(config_.context, part0.w0, part0.w1, part0.w2, part0.w3)).Times(1);

  processor_.processUMP(part0.w0, part0.w1, part0.w2, part0.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, Data128Sysex8StartAndEnd) {
  constexpr auto group = std::uint8_t{0};
  constexpr auto stream_id = std::uint8_t{0};

  midi2::types::data128::sysex8 part0;
  part0.w0.mt = to_underlying(midi2::ump_message_type::data128);
  part0.w0.group = group;
  part0.w0.status = to_underlying(midi2::data128::sysex8_start);
  part0.w0.number_of_bytes = 13;
  part0.w0.stream_id = stream_id;
  part0.w0.data0 = 2;
  part0.w1.data1 = 3;
  part0.w1.data2 = 5;
  part0.w1.data3 = 7;
  part0.w1.data4 = 11;
  part0.w2.data5 = 13;
  part0.w2.data6 = 17;
  part0.w2.data7 = 19;
  part0.w2.data8 = 23;
  part0.w3.data9 = 29;
  part0.w3.data10 = 31;
  part0.w3.data11 = 37;
  part0.w3.data12 = 41;

  midi2::types::data128::sysex8 part1;
  part1.w0.mt = to_underlying(midi2::ump_message_type::data128);
  part1.w0.group = group;
  part1.w0.status = to_underlying(midi2::data128::sysex8_continue);
  part1.w0.number_of_bytes = 13;
  part1.w0.stream_id = stream_id;
  part1.w0.data0 = 43;
  part1.w1.data1 = 47;
  part1.w1.data2 = 53;
  part1.w1.data3 = 59;
  part1.w1.data4 = 61;
  part1.w2.data5 = 67;
  part1.w2.data6 = 71;
  part1.w2.data7 = 73;
  part1.w2.data8 = 79;
  part1.w3.data9 = 83;
  part1.w3.data10 = 89;
  part1.w3.data11 = 97;
  part1.w3.data12 = 101;

  midi2::types::data128::sysex8 part2;
  part2.w0.mt = to_underlying(midi2::ump_message_type::data128);
  part2.w0.group = group;
  part2.w0.status = to_underlying(midi2::data128::sysex8_end);
  part2.w0.number_of_bytes = 4;
  part2.w0.stream_id = stream_id;
  part2.w0.data0 = 103;
  part2.w1.data1 = 107;
  part2.w1.data2 = 109;
  part2.w1.data2 = 113;

  {
    InSequence _;
    EXPECT_CALL(config_.data128, sysex8_start(config_.context, part0.w0, part0.w1, part0.w2, part0.w3)).Times(1);
    EXPECT_CALL(config_.data128, sysex8_continue(config_.context, part1.w0, part1.w1, part1.w2, part1.w3)).Times(1);
    EXPECT_CALL(config_.data128, sysex8_end(config_.context, part2.w0, part2.w1, part2.w2, part2.w3)).Times(1);
  }

  // Send 13 bytes
  processor_.processUMP(part0.w0, part0.w1, part0.w2, part0.w3);
  processor_.processUMP(part1.w0, part1.w1, part1.w2, part1.w3);
  processor_.processUMP(part2.w0, part2.w1, part2.w2, part2.w3);
}

// NOLINTNEXTLINE
TEST_F(UMPProcessor, Data128MixedDatSet) {
  constexpr auto group = std::uint8_t{0};
  constexpr auto mds_id = std::uint8_t{0b1010};

  midi2::types::data128::mds_header header;
  header.w0.mt = to_underlying(midi2::ump_message_type::data128);
  header.w0.group = group;
  header.w0.status = to_underlying(midi2::data128::mixed_data_set_header);
  header.w0.mds_id = mds_id;
  header.w0.bytes_in_chunk = 2;

  header.w1.chunks_in_mds = 1;
  header.w1.chunk_num = 1;
  header.w2.manufacturer_id = 43;
  header.w2.device_id = 61;
  header.w3.sub_id_1 = 19;
  header.w3.sub_id_2 = 23;

  midi2::types::data128::mds_payload payload;
  payload.w0.mt = to_underlying(midi2::ump_message_type::data128);
  payload.w0.group = group;
  payload.w0.status = to_underlying(midi2::data128::mixed_data_set_payload);
  payload.w0.mds_id = mds_id;
  payload.w0.data0 = std::uint16_t{0xFFFF};
  payload.w1 = 0xFFFFFFFF;
  payload.w2 = 0xFFFFFFFF;
  payload.w3 = 0xFFFFFFFF;

  {
    InSequence _;
    EXPECT_CALL(config_.data128, mds_header(config_.context, header.w0, header.w1, header.w2, header.w3)).Times(1);
    EXPECT_CALL(config_.data128, mds_payload(config_.context, payload.w0, payload.w1, payload.w2, payload.w3)).Times(1);
  }

  processor_.processUMP(header.w0, header.w1, header.w2, header.w3);
  processor_.processUMP(payload.w0, payload.w1, payload.w2, payload.w3);
}
TEST_F(UMPProcessor, PartialMessageThenClear) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint16_t{0x43};  // 7 bits
  constexpr auto group = std::uint8_t{0};

  midi2::types::m1cvm_w0 message{};
  message.mt = to_underlying(midi2::ump_message_type::m1cvm);
  message.group = group;
  message.status = midi2::status::note_on >> 4;
  message.channel = channel;
  message.data_a = note_number;
  message.data_b = velocity;

  EXPECT_CALL(config_.m1cvm, note_on(config_.context, message)).Times(1);

  // The first half of a 64-bit MIDI 2 note-on message.
  processor_.processUMP(
      pack((to_underlying(midi2::ump_message_type::m2cvm) << 4) | group, (ump_note_on << 4) | channel, note_number, 0));
  processor_.clearUMP();

  // An entire 32-bit MIDI 1 note-on message.
  processor_.processUMP(pack((to_underlying(midi2::ump_message_type::m1cvm) << 4) | group, (ump_note_on << 4) | channel,
                             note_number, velocity));
}
//*  _   _ __  __ ___   ___ _                       *
//* | | | |  \/  | _ \ / __| |_ _ _ ___ __ _ _ __   *
//* | |_| | |\/| |  _/ \__ \  _| '_/ -_) _` | '  \  *
//*  \___/|_|  |_|_|   |___/\__|_| \___\__,_|_|_|_| *
//*                                                 *
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamEndpointDiscovery) {
  midi2::types::ump_stream::endpoint_discovery_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x03;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::endpoint_discovery));
  w0.version_major = 0x01;
  w0.version_minor = 0x01;
  midi2::types::ump_stream::endpoint_discovery_w1 w1{};
  w1.filter = 0b00011111;
  midi2::types::ump_stream::endpoint_discovery_w2 w2{};
  midi2::types::ump_stream::endpoint_discovery_w3 w3{};

  EXPECT_CALL(config_.ump_stream, endpoint_discovery(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamEndpointInfoNotification) {
  midi2::types::ump_stream::endpoint_info_notification_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::endpoint_info_notification));
  w0.version_major = 0x01;
  w0.version_minor = 0x01;
  midi2::types::ump_stream::endpoint_info_notification_w1 w1{};
  w1.static_function_blocks = 1;
  w1.number_function_blocks = 0b0101010;
  w1.midi2_protocol_capability = 1;
  w1.midi1_protocol_capability = 0;
  w1.receive_jr_timestamp_capability = 1;
  w1.transmit_jr_timestamp_capability = 0;
  midi2::types::ump_stream::endpoint_info_notification_w2 w2{};
  midi2::types::ump_stream::endpoint_info_notification_w3 w3{};

  EXPECT_CALL(config_.ump_stream, endpoint_info_notification(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamDeviceIdentityNotification) {
  midi2::types::ump_stream::device_identity_notification_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::device_identity_notification));
  midi2::types::ump_stream::device_identity_notification_w1 w1{};
  w1.dev_manuf_sysex_id_1 = 1;
  w1.dev_manuf_sysex_id_2 = 1;
  w1.dev_manuf_sysex_id_3 = 0;
  midi2::types::ump_stream::device_identity_notification_w2 w2{};
  w2.device_family_lsb = 0x79;
  w2.device_family_msb = 0x7B;
  w2.device_family_model_lsb = 0x7D;
  w2.device_family_model_msb = 0x7F;
  midi2::types::ump_stream::device_identity_notification_w3 w3{};
  w3.sw_revision_1 = 0x7F;
  w3.sw_revision_2 = 0x7D;
  w3.sw_revision_3 = 0x7B;
  w3.sw_revision_4 = 0x79;
  EXPECT_CALL(config_.ump_stream, device_identity_notification(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamEndpointNameNotification) {
  midi2::types::ump_stream::endpoint_name_notification_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::endpoint_name_notification));
  w0.name1 = std::uint8_t{'a'};
  w0.name2 = std::uint8_t{'b'};
  midi2::types::ump_stream::endpoint_name_notification_w1 w1{};
  w1.name3 = std::uint8_t{'c'};
  w1.name4 = std::uint8_t{'d'};
  w1.name5 = std::uint8_t{'e'};
  w1.name6 = std::uint8_t{'f'};
  midi2::types::ump_stream::endpoint_name_notification_w2 w2{};
  w2.name7 = std::uint8_t{'g'};
  w2.name8 = std::uint8_t{'h'};
  w2.name9 = std::uint8_t{'i'};
  w2.name10 = std::uint8_t{'j'};
  midi2::types::ump_stream::endpoint_name_notification_w3 w3{};
  w3.name11 = std::uint8_t{'k'};
  w3.name12 = std::uint8_t{'l'};
  w3.name13 = std::uint8_t{'m'};
  w3.name14 = std::uint8_t{'m'};
  EXPECT_CALL(config_.ump_stream, endpoint_name_notification(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamProductInstanceIdNotification) {
  midi2::types::ump_stream::product_instance_id_notification_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::product_instance_id_notification));
  w0.pid1 = 0x22;
  w0.pid2 = 0x33;
  midi2::types::ump_stream::product_instance_id_notification_w1 w1{};
  w1.pid3 = 0x44;
  w1.pid4 = 0x55;
  w1.pid5 = 0x66;
  w1.pid6 = 0x77;
  midi2::types::ump_stream::product_instance_id_notification_w2 w2{};
  w2.pid7 = 0x88;
  w2.pid8 = 0x99;
  w2.pid9 = 0xAA;
  w2.pid10 = 0xBB;
  midi2::types::ump_stream::product_instance_id_notification_w3 w3{};
  w3.pid11 = 0xCC;
  w3.pid12 = 0xDD;
  w3.pid13 = 0xEE;
  w3.pid14 = 0xFF;
  EXPECT_CALL(config_.ump_stream, product_instance_id_notification(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamJRConfigurationRequest) {
  midi2::types::ump_stream::jr_configuration_request_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::jr_configuration_request));
  w0.protocol = 0x02;
  w0.rxjr = 1;
  w0.txjr = 0;
  midi2::types::ump_stream::jr_configuration_request_w1 w1{};
  midi2::types::ump_stream::jr_configuration_request_w2 w2{};
  midi2::types::ump_stream::jr_configuration_request_w3 w3{};
  EXPECT_CALL(config_.ump_stream, jr_configuration_request(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamJRConfigurationNotification) {
  midi2::types::ump_stream::jr_configuration_notification_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::jr_configuration_notification));
  w0.protocol = 0x02;
  w0.rxjr = 1;
  w0.txjr = 0;
  midi2::types::ump_stream::jr_configuration_notification_w1 w1{};
  midi2::types::ump_stream::jr_configuration_notification_w2 w2{};
  midi2::types::ump_stream::jr_configuration_notification_w3 w3{};
  EXPECT_CALL(config_.ump_stream, jr_configuration_notification(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamFunctionBlockDiscovery) {
  midi2::types::ump_stream::function_block_discovery_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::function_block_discovery));
  w0.block_num = 0xFF;
  w0.filter = 0x03;
  midi2::types::ump_stream::function_block_discovery_w1 w1{};
  midi2::types::ump_stream::function_block_discovery_w2 w2{};
  midi2::types::ump_stream::function_block_discovery_w3 w3{};
  EXPECT_CALL(config_.ump_stream, function_block_discovery(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamFunctionBlockInfoNotification) {
  midi2::types::ump_stream::function_block_info_notification_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::function_block_info_notification));
  w0.block_active = 1;
  w0.block_num = 0x1F;
  w0.ui_hint = 0b10;
  w0.midi1 = 0;
  w0.direction = 0b10;
  midi2::types::ump_stream::function_block_info_notification_w1 w1{};
  w1.first_group = 0b10101010;
  w1.num_spanned = 0x10;
  w1.ci_message_version = 0x1;
  w1.max_sys8_streams = 2;
  midi2::types::ump_stream::function_block_info_notification_w2 w2{};
  midi2::types::ump_stream::function_block_info_notification_w3 w3{};
  EXPECT_CALL(config_.ump_stream, function_block_info_notification(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamFunctionBlockNameNotification) {
  midi2::types::ump_stream::function_block_name_notification_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::function_block_name_notification));
  w0.block_num = 0x1F;
  w0.name0 = 'a';
  midi2::types::ump_stream::function_block_name_notification_w1 w1{};
  w1.name1 = 'b';
  w1.name2 = 'c';
  w1.name3 = 'd';
  w1.name4 = 'e';
  midi2::types::ump_stream::function_block_name_notification_w2 w2{};
  w2.name5 = 'f';
  w2.name6 = 'g';
  w2.name7 = 'h';
  w2.name8 = 'i';
  midi2::types::ump_stream::function_block_name_notification_w3 w3{};
  w3.name9 = 'k';
  w3.name10 = 'l';
  w3.name11 = 'm';
  w3.name12 = 'n';
  EXPECT_CALL(config_.ump_stream, function_block_name_notification(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamStartOfClip) {
  midi2::types::ump_stream::start_of_clip_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::start_of_clip));
  midi2::types::ump_stream::start_of_clip_w1 w1{};
  midi2::types::ump_stream::start_of_clip_w2 w2{};
  midi2::types::ump_stream::start_of_clip_w3 w3{};
  EXPECT_CALL(config_.ump_stream, start_of_clip(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, StreamEndOfClip) {
  midi2::types::ump_stream::end_of_clip_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  w0.format = 0x00;
  w0.status = static_cast<std::uint16_t>(to_underlying(midi2::ump_stream::end_of_clip));
  midi2::types::ump_stream::end_of_clip_w1 w1{};
  midi2::types::ump_stream::end_of_clip_w2 w2{};
  midi2::types::ump_stream::end_of_clip_w3 w3{};
  EXPECT_CALL(config_.ump_stream, end_of_clip(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}

//*  ___ _           ___       _         *
//* | __| |_____ __ |   \ __ _| |_ __ _  *
//* | _|| / -_) \ / | |) / _` |  _/ _` | *
//* |_| |_\___/_\_\ |___/\__,_|\__\__,_| *
//*                                      *
// NOLINTNEXTLINE
TEST_F(UMPProcessor, FlexDataSetTempo) {
  midi2::types::flex_data::set_tempo_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 0;
  w0.status_bank = 0;
  w0.status = static_cast<std::uint8_t>(to_underlying(midi2::flex_data::set_tempo));
  midi2::types::flex_data::set_tempo_w1 w1 = std::uint32_t{0xF0F0F0F0};
  midi2::types::flex_data::set_tempo_w2 w2{};
  midi2::types::flex_data::set_tempo_w3 w3{};
  EXPECT_CALL(config_.flex, set_tempo(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, FlexDataSetTimeSignature) {
  midi2::types::flex_data::set_time_signature_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 3;
  w0.status_bank = 0;
  w0.status = static_cast<std::uint8_t>(to_underlying(midi2::flex_data::set_time_signature));
  midi2::types::flex_data::set_time_signature_w1 w1{};
  w1.numerator = 1;
  w1.denominator = 2;
  w1.number_of_32_notes = 16;
  midi2::types::flex_data::set_time_signature_w2 w2{};
  midi2::types::flex_data::set_time_signature_w3 w3{};
  EXPECT_CALL(config_.flex, set_time_signature(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, FlexDataSetMetronome) {
  midi2::types::flex_data::set_metronome_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 3;
  w0.status_bank = 0;
  w0.status = static_cast<std::uint8_t>(to_underlying(midi2::flex_data::set_metronome));
  midi2::types::flex_data::set_metronome_w1 w1{};
  w1.num_clocks_per_primary_click = 24;
  w1.bar_accent_part_1 = 4;
  w1.bar_accent_part_2 = 0;
  w1.bar_accent_part_3 = 0;
  midi2::types::flex_data::set_metronome_w2 w2{};
  w2.num_subdivision_clicks_1 = 0;
  w2.num_subdivision_clicks_2 = 0;
  midi2::types::flex_data::set_metronome_w3 w3{};
  EXPECT_CALL(config_.flex, set_metronome(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, FlexDataSetKeySignature) {
  midi2::types::flex_data::set_key_signature_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 3;
  w0.status_bank = 0;
  w0.status = static_cast<std::uint8_t>(to_underlying(midi2::flex_data::set_key_signature));
  midi2::types::flex_data::set_key_signature_w1 w1{};
  w1.sharps_flats = 0b100;  // (-8)
  w1.tonic_note = static_cast<std::uint8_t>(midi2::types::flex_data::note::E);
  midi2::types::flex_data::set_key_signature_w2 w2{};
  midi2::types::flex_data::set_key_signature_w3 w3{};
  EXPECT_CALL(config_.flex, set_key_signature(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, FlexDataSetChordName) {
  constexpr auto group = std::uint8_t{0x0F};
  constexpr auto addrs = std::uint8_t{0x03};
  constexpr auto channel = std::uint8_t{3};

  constexpr auto chord_tonic = midi2::types::flex_data::note::E;
  constexpr auto chord_type = midi2::types::flex_data::chord_type::augmented;
  constexpr auto bass_note = midi2::types::flex_data::note::unknown;
  constexpr auto bass_chord_type = midi2::types::flex_data::chord_type::diminished;

  midi2::types::flex_data::set_chord_name_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  w0.group = group;
  w0.form = 0x0;
  w0.addrs = addrs;
  w0.channel = channel;
  w0.status_bank = 0x00;
  w0.status = static_cast<std::uint8_t>(to_underlying(midi2::flex_data::set_chord_name));

  midi2::types::flex_data::set_chord_name_w1 w1{};
  w1.tonic_sharps_flats = 0x1;
  w1.chord_tonic = static_cast<std::uint8_t>(chord_tonic);
  w1.chord_type = static_cast<std::uint8_t>(chord_type);
  w1.alter_1_type = 1;
  w1.alter_1_degree = 5;
  w1.alter_2_type = 2;
  w1.alter_2_degree = 6;

  midi2::types::flex_data::set_chord_name_w2 w2{};
  w2.alter_3_type = 3;
  w2.alter_3_degree = 7;
  w2.alter_4_type = 4;
  w2.alter_4_degree = 8;
  w2.reserved = 0x0000;

  midi2::types::flex_data::set_chord_name_w3 w3{};
  w3.bass_sharps_flats = 0xE;
  w3.bass_note = static_cast<std::uint8_t>(bass_note);
  w3.bass_chord_type = static_cast<std::uint8_t>(bass_chord_type);
  w3.alter_1_type = 1;
  w3.alter_1_degree = 3;
  w3.alter_2_type = 2;
  w3.alter_2_degree = 4;

  EXPECT_CALL(config_.flex, set_chord_name(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, FlexDataText) {
  midi2::types::flex_data::text_common_w0 w0{};
  w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 3;
  w0.status_bank = 1;
  w0.status = 4;
  midi2::types::flex_data::text_common_w1 const w1 =
      (std::uint32_t{0xC2} << 24) | (std::uint32_t{0xA9} << 16) | (std::uint32_t{'2'} << 8) | (std::uint32_t{'0'} << 0);
  midi2::types::flex_data::text_common_w2 const w2 =
      (std::uint32_t{'2'} << 24) | (std::uint32_t{'4'} << 16) | (std::uint32_t{' '} << 8) | (std::uint32_t{'P'} << 0);
  midi2::types::flex_data::text_common_w3 const w3 =
      (std::uint32_t{'B'} << 24) | (std::uint32_t{'H'} << 16) | (std::uint32_t{'\0'} << 8) | (std::uint32_t{'\0'} << 0);
  EXPECT_CALL(config_.flex, text(config_.context, w0, w1, w2, w3)).Times(1);

  processor_.processUMP(w0);
  processor_.processUMP(w1);
  processor_.processUMP(w2);
  processor_.processUMP(w3);
}

void UMPProcessorNeverCrashes(std::vector<std::uint32_t> const& in) {
  midi2::umpProcessor p;
  std::ranges::for_each(in, [&p](std::uint32_t ump) { p.processUMP(ump); });
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessorFuzz, UMPProcessorNeverCrashes);
#endif
// NOLINTNEXTLINE
TEST(UMPProcessorFuzz, Empty) {
  UMPProcessorNeverCrashes({});
}

template <midi2::ump_message_type MessageType>
void process_message(std::span<std::uint32_t> message) {
  if (message.size() == midi2::message_size<MessageType>::value) {
    message[0] = (message[0] & 0x00FFFFFF) |
                 (static_cast<std::uint32_t>(MessageType) << 24);
    midi2::umpProcessor p;
    for (auto const w : message) {
      p.processUMP(w);
    }
  }
}

void utility(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::utility>(
      {std::begin(message), std::end(message)});
}
void system(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::system>(
      {std::begin(message), std::end(message)});
}
void m1cvm(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::m1cvm>(
      {std::begin(message), std::end(message)});
}
void data64(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::data64>({std::begin(message), std::end(message)});
}
void m2cvm(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::m2cvm>(
      {std::begin(message), std::end(message)});
}
void data128(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::data128>({std::begin(message), std::end(message)});
}
void flex_data(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::flex_data>(
      {std::begin(message), std::end(message)});
}
void stream(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::ump_stream>({std::begin(message), std::end(message)});
}
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessorFuzz, utility);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessorFuzz, system);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessorFuzz, m1cvm);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessorFuzz, data64);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessorFuzz, m2cvm);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessorFuzz, data128);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessorFuzz, flex_data);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessorFuzz, stream);
#endif

// NOLINTNEXTLINE
TEST(UMPProcessorFuzz, UtilityMessage) {
  utility({});
}
// NOLINTNEXTLINE
TEST(UMPProcessorFuzz, SystemMessage) {
  system({});
}
// NOLINTNEXTLINE
TEST(UMPProcessorFuzz, M1CVMMessage) {
  m1cvm({});
}
// NOLINTNEXTLINE
TEST(UMPProcessorFuzz, Data64Message) {
  data64({});
}
// NOLINTNEXTLINE
TEST(UMPProcessorFuzz, M2CVMMessage) {
  m2cvm({});
}
// NOLINTNEXTLINE
TEST(UMPProcessorFuzz, Data128Message) {
  data128({});
}
// NOLINTNEXTLINE
TEST(UMPProcessorFuzz, FlexDataMessage) {
  flex_data({});
}
// NOLINTNEXTLINE
TEST(UMPProcessorFuzz, UMPStreamMessage) {
  stream({});
}

}  // end anonymous namespace
