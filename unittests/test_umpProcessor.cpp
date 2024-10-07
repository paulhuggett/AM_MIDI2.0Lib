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

namespace {

using midi2::pack;
using context_type = int;

struct utility_base {
  utility_base() = default;
  utility_base(utility_base const&) = default;
  virtual ~utility_base() noexcept = default;

  virtual void noop(context_type) = 0;
  virtual void jr_clock(context_type, midi2::types::jr_clock) = 0;
  virtual void jr_timestamp(context_type, midi2::types::jr_clock) = 0;
  virtual void delta_clockstamp_tpqn(context_type, midi2::types::jr_clock) = 0;
  virtual void delta_clockstamp(context_type, midi2::types::delta_clockstamp) = 0;

  virtual void unknown(std::span<std::uint32_t>) = 0;
};
class UtilityMocks : public utility_base {
public:
  MOCK_METHOD(void, noop, (context_type), (override));
  MOCK_METHOD(void, jr_clock, (context_type, midi2::types::jr_clock), (override));
  MOCK_METHOD(void, jr_timestamp, (context_type, midi2::types::jr_clock), (override));
  MOCK_METHOD(void, delta_clockstamp_tpqn, (context_type, midi2::types::jr_clock), (override));
  MOCK_METHOD(void, delta_clockstamp, (context_type, midi2::types::delta_clockstamp), (override));

  MOCK_METHOD(void, unknown, (std::span<std::uint32_t>), (override));
};
struct system_base {
  system_base() = default;
  system_base(system_base const &) = default;
  virtual ~system_base() noexcept = default;

  virtual void midi_time_code(context_type, midi2::types::system::midi_time_code) = 0;
  virtual void song_position_pointer(context_type, midi2::types::system::song_position_pointer) = 0;
  virtual void song_select(context_type, midi2::types::system::song_select) = 0;
  virtual void tune_request(context_type, midi2::types::system::tune_request) = 0;
  virtual void timing_clock(context_type, midi2::types::system::timing_clock) = 0;
  virtual void seq_start(context_type, midi2::types::system::seq_start) = 0;
  virtual void seq_continue(context_type, midi2::types::system::seq_continue) = 0;
  virtual void seq_stop(context_type, midi2::types::system::seq_stop) = 0;
  virtual void active_sensing(context_type, midi2::types::system::active_sensing) = 0;
  virtual void reset(context_type, midi2::types::system::reset) = 0;
};
class SystemMocks : public system_base {
public:
  MOCK_METHOD(void, midi_time_code, (context_type, midi2::types::system::midi_time_code), (override));
  MOCK_METHOD(void, song_position_pointer, (context_type, midi2::types::system::song_position_pointer), (override));
  MOCK_METHOD(void, song_select, (context_type, midi2::types::system::song_select), (override));
  MOCK_METHOD(void, tune_request, (context_type, midi2::types::system::tune_request), (override));
  MOCK_METHOD(void, timing_clock, (context_type, midi2::types::system::timing_clock), (override));
  MOCK_METHOD(void, seq_start, (context_type, midi2::types::system::seq_start), (override));
  MOCK_METHOD(void, seq_continue, (context_type, midi2::types::system::seq_continue), (override));
  MOCK_METHOD(void, seq_stop, (context_type, midi2::types::system::seq_stop), (override));
  MOCK_METHOD(void, active_sensing, (context_type, midi2::types::system::active_sensing), (override));
  MOCK_METHOD(void, reset, (context_type, midi2::types::system::reset), (override));
};
struct m1cvm_base {
  m1cvm_base() = default;
  m1cvm_base(m1cvm_base const&) = default;
  virtual ~m1cvm_base() noexcept = default;

  virtual void note_off(context_type, midi2::types::m1cvm_w0) = 0;
  virtual void note_on(context_type, midi2::types::m1cvm_w0) = 0;
  virtual void poly_pressure(context_type, midi2::types::m1cvm_w0) = 0;
  virtual void control_change(context_type, midi2::types::m1cvm_w0) = 0;
  virtual void program_change(context_type, midi2::types::m1cvm_w0) = 0;
  virtual void channel_pressure(context_type, midi2::types::m1cvm_w0) = 0;
  virtual void pitch_bend(context_type, midi2::types::m1cvm_w0) = 0;
};
class M1CVMMocks : public m1cvm_base {
public:
  MOCK_METHOD(void, note_off, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, note_on, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, poly_pressure, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, control_change, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, program_change, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, channel_pressure, (context_type, midi2::types::m1cvm_w0), (override));
  MOCK_METHOD(void, pitch_bend, (context_type, midi2::types::m1cvm_w0), (override));
};
struct data64_base {
  data64_base() = default;
  data64_base(data64_base const&) = default;
  virtual ~data64_base() noexcept = default;

  virtual void sysex7_in_1(context_type, midi2::types::data64::sysex7) = 0;
  virtual void sysex7_start(context_type, midi2::types::data64::sysex7) = 0;
  virtual void sysex7_continue(context_type, midi2::types::data64::sysex7) = 0;
  virtual void sysex7_end(context_type, midi2::types::data64::sysex7) = 0;
};
class Data64Mocks : public data64_base {
public:
  MOCK_METHOD(void, sysex7_in_1, (context_type, midi2::types::data64::sysex7), (override));
  MOCK_METHOD(void, sysex7_start, (context_type, midi2::types::data64::sysex7), (override));
  MOCK_METHOD(void, sysex7_continue, (context_type, midi2::types::data64::sysex7), (override));
  MOCK_METHOD(void, sysex7_end, (context_type, midi2::types::data64::sysex7), (override));
};
struct m2cvm_base {
  m2cvm_base() = default;
  m2cvm_base(m2cvm_base const&) = default;
  virtual ~m2cvm_base() noexcept = default;

  virtual void note_off(context_type, midi2::types::m2cvm::note) = 0;
  virtual void note_on(context_type, midi2::types::m2cvm::note) = 0;
  virtual void poly_pressure(context_type, midi2::types::m2cvm::poly_pressure) = 0;
  virtual void program_change(context_type, midi2::types::m2cvm::program_change) = 0;
  virtual void channel_pressure(context_type, midi2::types::m2cvm::channel_pressure) = 0;
  virtual void rpn_controller(context_type, midi2::types::m2cvm::per_note_controller) = 0;
  virtual void nrpn_controller(context_type, midi2::types::m2cvm::per_note_controller) = 0;
  virtual void per_note_management(context_type, midi2::types::m2cvm::per_note_management_w0, std::uint32_t) = 0;
  virtual void control_change(context_type, midi2::types::m2cvm::control_change_w0, std::uint32_t) = 0;
  virtual void controller_message(context_type, midi2::types::m2cvm::controller_message) = 0;
  virtual void pitch_bend(context_type, midi2::types::m2cvm::pitch_bend_w0, std::uint32_t) = 0;
  virtual void per_note_pitch_bend(context_type, midi2::types::m2cvm::per_note_pitch_bend_w0, std::uint32_t) = 0;
};
class M2CVMMocks : public m2cvm_base {
public:
  MOCK_METHOD(void, note_off, (context_type, midi2::types::m2cvm::note), (override));
  MOCK_METHOD(void, note_on, (context_type, midi2::types::m2cvm::note), (override));
  MOCK_METHOD(void, poly_pressure, (context_type, midi2::types::m2cvm::poly_pressure), (override));
  MOCK_METHOD(void, program_change, (context_type, midi2::types::m2cvm::program_change), (override));
  MOCK_METHOD(void, channel_pressure, (context_type, midi2::types::m2cvm::channel_pressure), (override));
  MOCK_METHOD(void, rpn_controller, (context_type, midi2::types::m2cvm::per_note_controller), (override));
  MOCK_METHOD(void, nrpn_controller, (context_type, midi2::types::m2cvm::per_note_controller), (override));
  MOCK_METHOD(void, per_note_management, (context_type, midi2::types::m2cvm::per_note_management_w0, std::uint32_t),
              (override));
  MOCK_METHOD(void, control_change, (context_type, midi2::types::m2cvm::control_change_w0, std::uint32_t), (override));
  MOCK_METHOD(void, controller_message, (context_type, midi2::types::m2cvm::controller_message), (override));
  MOCK_METHOD(void, pitch_bend, (context_type, midi2::types::m2cvm::pitch_bend_w0, std::uint32_t), (override));
  MOCK_METHOD(void, per_note_pitch_bend, (context_type, midi2::types::m2cvm::per_note_pitch_bend_w0, std::uint32_t),
              (override));
};
struct data128_base {
  data128_base() = default;
  data128_base(data128_base const&) = default;
  virtual ~data128_base() noexcept = default;

  virtual void sysex8_in_1(context_type, midi2::types::data128::sysex8 const &) = 0;
  virtual void sysex8_start(context_type, midi2::types::data128::sysex8 const &) = 0;
  virtual void sysex8_continue(context_type, midi2::types::data128::sysex8 const &) = 0;
  virtual void sysex8_end(context_type, midi2::types::data128::sysex8 const &) = 0;
  virtual void mds_header(context_type, midi2::types::data128::mds_header const &) = 0;
  virtual void mds_payload(context_type, midi2::types::data128::mds_payload const &) = 0;
};
class Data128Mocks : public data128_base {
public:
  MOCK_METHOD(void, sysex8_in_1, (context_type, midi2::types::data128::sysex8 const &), (override));
  MOCK_METHOD(void, sysex8_start, (context_type, midi2::types::data128::sysex8 const &), (override));
  MOCK_METHOD(void, sysex8_continue, (context_type, midi2::types::data128::sysex8 const &), (override));
  MOCK_METHOD(void, sysex8_end, (context_type, midi2::types::data128::sysex8 const &), (override));
  MOCK_METHOD(void, mds_header, (context_type, midi2::types::data128::mds_header const &), (override));
  MOCK_METHOD(void, mds_payload, (context_type, midi2::types::data128::mds_payload const &), (override));
};
struct ump_stream_base {
  ump_stream_base() = default;
  ump_stream_base(ump_stream_base const&) = default;
  virtual ~ump_stream_base() noexcept = default;

  virtual void endpoint_discovery(context_type, midi2::types::ump_stream::endpoint_discovery) = 0;
  virtual void endpoint_info_notification(context_type, midi2::types::ump_stream::endpoint_info_notification) = 0;
  virtual void device_identity_notification(context_type, midi2::types::ump_stream::device_identity_notification) = 0;
  virtual void endpoint_name_notification(context_type, midi2::types::ump_stream::endpoint_name_notification) = 0;
  virtual void product_instance_id_notification(context_type,
                                                midi2::types::ump_stream::product_instance_id_notification) = 0;
  virtual void jr_configuration_request(context_type, midi2::types::ump_stream::jr_configuration_request) = 0;
  virtual void jr_configuration_notification(context_type, midi2::types::ump_stream::jr_configuration_notification) = 0;

  virtual void function_block_discovery(context_type, midi2::types::ump_stream::function_block_discovery) = 0;
  virtual void function_block_info_notification(context_type,
                                                midi2::types::ump_stream::function_block_info_notification) = 0;
  virtual void function_block_name_notification(context_type,
                                                midi2::types::ump_stream::function_block_name_notification) = 0;

  virtual void start_of_clip(context_type, midi2::types::ump_stream::start_of_clip) = 0;
  virtual void end_of_clip(context_type, midi2::types::ump_stream::end_of_clip) = 0;
};
class UMPStreamMocks : public ump_stream_base {
public:
  MOCK_METHOD(void, endpoint_discovery, (context_type, midi2::types::ump_stream::endpoint_discovery), (override));
  MOCK_METHOD(void, endpoint_info_notification, (context_type, midi2::types::ump_stream::endpoint_info_notification),
              (override));
  MOCK_METHOD(void, device_identity_notification,
              (context_type, midi2::types::ump_stream::device_identity_notification), (override));
  MOCK_METHOD(void, endpoint_name_notification, (context_type, midi2::types::ump_stream::endpoint_name_notification),
              (override));
  MOCK_METHOD(void, product_instance_id_notification,
              (context_type, midi2::types::ump_stream::product_instance_id_notification), (override));

  MOCK_METHOD(void, jr_configuration_request, (context_type, midi2::types::ump_stream::jr_configuration_request),
              (override));
  MOCK_METHOD(void, jr_configuration_notification,
              (context_type, midi2::types::ump_stream::jr_configuration_notification), (override));

  MOCK_METHOD(void, function_block_discovery, (context_type, midi2::types::ump_stream::function_block_discovery),
              (override));
  MOCK_METHOD(void, function_block_info_notification,
              (context_type, midi2::types::ump_stream::function_block_info_notification), (override));
  MOCK_METHOD(void, function_block_name_notification,
              (context_type, midi2::types::ump_stream::function_block_name_notification), (override));

  MOCK_METHOD(void, start_of_clip, (context_type, midi2::types::ump_stream::start_of_clip), (override));
  MOCK_METHOD(void, end_of_clip, (context_type, midi2::types::ump_stream::end_of_clip), (override));
};
struct flex_data_base {
  flex_data_base() = default;
  flex_data_base(flex_data_base const&) = default;
  virtual ~flex_data_base() noexcept = default;

  virtual void set_tempo(context_type, midi2::types::flex_data::set_tempo) = 0;
  virtual void set_time_signature(context_type, midi2::types::flex_data::set_time_signature) = 0;
  virtual void set_metronome(context_type, midi2::types::flex_data::set_metronome) = 0;
  virtual void set_key_signature(context_type, midi2::types::flex_data::set_key_signature) = 0;
  virtual void set_chord_name(context_type, midi2::types::flex_data::set_chord_name) = 0;
  virtual void text(context_type, midi2::types::flex_data::text_common) = 0;
};
class FlexDataMocks : public flex_data_base {
public:
  MOCK_METHOD(void, set_tempo, (context_type, midi2::types::flex_data::set_tempo), (override));
  MOCK_METHOD(void, set_time_signature, (context_type, midi2::types::flex_data::set_time_signature), (override));
  MOCK_METHOD(void, set_metronome, (context_type, midi2::types::flex_data::set_metronome), (override));
  MOCK_METHOD(void, set_key_signature, (context_type, midi2::types::flex_data::set_key_signature), (override));
  MOCK_METHOD(void, set_chord_name, (context_type, midi2::types::flex_data::set_chord_name), (override));
  MOCK_METHOD(void, text, (context_type, midi2::types::flex_data::text_common), (override));
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
    StrictMock<UtilityMocks> utility;
    StrictMock<SystemMocks> system;
    StrictMock<M1CVMMocks> m1cvm;
    StrictMock<Data64Mocks> data64;
    StrictMock<M2CVMMocks> m2cvm;
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
  w0.status = to_underlying(midi2::ump_utility::noop);
  w0.data = 0;
  processor_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, JRClock) {
  midi2::types::jr_clock message{};
  message.mt = ump_mt(midi2::ump_message_type::utility);
  message.status = to_underlying(midi2::ump_utility::jr_clock);
  message.sender_clock_time = 0b1010101010101010;
  EXPECT_CALL(config_.utility, jr_clock(config_.context, message)).Times(1);
  processor_.processUMP(message);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, JRTimestamp) {
  midi2::types::jr_clock message{};
  message.mt = ump_mt(midi2::ump_message_type::utility);
  message.status = to_underlying(midi2::ump_utility::jr_ts);
  message.sender_clock_time = (1U << 16) - 1U;
  EXPECT_CALL(config_.utility, jr_timestamp(config_.context, message)).Times(1);
  processor_.processUMP(message);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, DeltaClockstampTqpn) {
  midi2::types::jr_clock message{};
  message.mt = ump_mt(midi2::ump_message_type::utility);
  message.status = to_underlying(midi2::ump_utility::delta_clock_tick);
  message.sender_clock_time = 0b1010101010101010;
  EXPECT_CALL(config_.utility, delta_clockstamp_tpqn(config_.context, message)).Times(1);
  processor_.processUMP(message);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, DeltaClockstamp) {
  midi2::types::delta_clockstamp message{};
  message.mt = ump_mt(midi2::ump_message_type::utility);
  message.status = to_underlying(midi2::ump_utility::delta_clock_since);
  message.ticks_per_quarter_note = (1U << 20) - 1U;
  EXPECT_CALL(config_.utility, delta_clockstamp(config_.context, message)).Times(1);
  processor_.processUMP(message);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, BadUtility) {
  midi2::types::delta_clockstamp message;
  message.mt = to_underlying(midi2::ump_message_type::utility);
  message.status = std::uint8_t{0b1111};
  EXPECT_CALL(config_.utility, unknown(ElementsAre(std::bit_cast<std::uint32_t>(message))));
  processor_.processUMP(message);
}

//*  ___         _              *
//* / __|_  _ __| |_ ___ _ __   *
//* \__ \ || (_-<  _/ -_) '  \  *
//* |___/\_, /__/\__\___|_|_|_| *
//*      |__/                   *
class UMPProcessorSystem : public UMPProcessor {};
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, MIDITimeCode) {
  midi2::types::system::midi_time_code message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::timing_code;
  message.w0.time_code = 0b1010101;
  EXPECT_CALL(config_.system, midi_time_code(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, SongPositionPointer) {
  midi2::types::system::song_position_pointer message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::spp;
  message.w0.position_lsb = 0b1010101;
  message.w0.position_msb = 0b1111111;
  EXPECT_CALL(config_.system, song_position_pointer(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, SongSelect) {
  midi2::types::system::song_select message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::song_select;
  message.w0.song = 0b1010101;
  EXPECT_CALL(config_.system, song_select(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, TuneRequest) {
  midi2::types::system::tune_request message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::tunerequest;
  EXPECT_CALL(config_.system, tune_request(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, TimingClock) {
  midi2::types::system::tune_request message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::tunerequest;
  EXPECT_CALL(config_.system, tune_request(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, Start) {
  midi2::types::system::seq_start message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::seqstart;
  EXPECT_CALL(config_.system, seq_start(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, Continue) {
  midi2::types::system::seq_continue message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::seqcont;
  EXPECT_CALL(config_.system, seq_continue(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, Stop) {
  midi2::types::system::seq_stop message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::seqstop;
  EXPECT_CALL(config_.system, seq_stop(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, ActiveSensing) {
  midi2::types::system::active_sensing message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::activesense;
  EXPECT_CALL(config_.system, active_sensing(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, Reset) {
  midi2::types::system::reset message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = midi2::status::systemreset;
  EXPECT_CALL(config_.system, reset(config_.context, message));
  processor_.processUMP(message.w0);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorSystem, BadStatus) {
  midi2::types::system::reset message;
  message.w0.mt = to_underlying(midi2::ump_message_type::system);
  message.w0.group = 0;
  message.w0.status = 0x00;
  EXPECT_CALL(config_.utility, unknown(ElementsAre(message.w0.word())));
  processor_.processUMP(message.w0);
}

//*        _    _ _   _   __   *
//*  _ __ (_)__| (_) / | /  \  *
//* | '  \| / _` | | | || () | *
//* |_|_|_|_\__,_|_| |_(_)__/  *
//*                            *
constexpr std::uint8_t ump_cvm(midi2::status s) {
  static_assert(
      std::is_same_v<std::underlying_type_t<midi2::status>, std::uint8_t>,
      "status type must be a std::uint8_t");
  assert((s & 0x0F) == 0 &&
         "Bottom 4 bits of a channel voice message status enum must be 0");
  return std::uint8_t{s} >> 4;
}

constexpr auto ump_note_on = ump_cvm(midi2::status::note_on);
class UMPProcessorMIDI1 : public UMPProcessor {};
// NOLINTNEXTLINE
TEST_F(UMPProcessorMIDI1, Midi1NoteOn) {
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
TEST_F(UMPProcessorMIDI1, Midi1NoteOff) {
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
TEST_F(UMPProcessorMIDI1, Midi1PolyPressure) {
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
TEST_F(UMPProcessorMIDI1, Midi1ControlChange) {
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

//*     _      _           __ _ _   *
//*  __| |__ _| |_ __ _   / /| | |  *
//* / _` / _` |  _/ _` | / _ \_  _| *
//* \__,_\__,_|\__\__,_| \___/ |_|  *
//*                                 *
// NOLINTNEXTLINE
TEST_F(UMPProcessor, Data64SysExIn1) {
  midi2::types::data64::sysex7 m0;
  m0.w0.mt = to_underlying(midi2::ump_message_type::data64);
  m0.w0.group = 0;
  m0.w0.status = to_underlying(midi2::data64::sysex7_in_1);
  m0.w0.number_of_bytes = 4;
  m0.w0.data0 = 2;
  m0.w0.data1 = 3;
  m0.w1.data2 = 5;
  m0.w1.data3 = 7;
  EXPECT_CALL(config_.data64, sysex7_in_1(config_.context, m0)).Times(1);
  processor_.processUMP(m0.w0, m0.w1);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessor, Data64Sysex8StartAndEnd) {
  constexpr auto group = std::uint8_t{0};
  using midi2::types::data64::sysex7;

  sysex7 m0;
  m0.w0.mt = to_underlying(midi2::ump_message_type::data64);
  m0.w0.group = group;
  m0.w0.status = to_underlying(midi2::data64::sysex7_start);
  m0.w0.number_of_bytes = 6;
  m0.w0.data0 = 2;
  m0.w0.data1 = 3;
  m0.w1.data2 = 5;
  m0.w1.data3 = 7;
  m0.w1.data4 = 11;
  m0.w1.data5 = 13;

  sysex7 m1;
  m1.w0.mt = to_underlying(midi2::ump_message_type::data64);
  m1.w0.group = group;
  m1.w0.status = to_underlying(midi2::data64::sysex7_continue);
  m1.w0.number_of_bytes = 6;
  m1.w0.data0 = 17;
  m1.w0.data1 = 19;
  m1.w1.data2 = 23;
  m1.w1.data3 = 29;
  m1.w1.data4 = 31;
  m1.w1.data5 = 37;

  sysex7 m2;
  m2.w0.mt = to_underlying(midi2::ump_message_type::data64);
  m2.w0.group = group;
  m2.w0.status = to_underlying(midi2::data64::sysex7_end);
  m2.w0.number_of_bytes = 4;
  m2.w0.data0 = 41;
  m2.w0.data1 = 43;
  m2.w1.data2 = 47;
  m2.w1.data3 = 53;
  {
    InSequence _;
    EXPECT_CALL(config_.data64, sysex7_start(config_.context, m0)).Times(1);
    EXPECT_CALL(config_.data64, sysex7_continue(config_.context, m1)).Times(1);
    EXPECT_CALL(config_.data64, sysex7_end(config_.context, m2)).Times(1);
  }
  processor_.processUMP(m0.w0, m0.w1);
  processor_.processUMP(m1.w0, m1.w1);
  processor_.processUMP(m2.w0, m2.w1);
}

//*        _    _ _   ___                 *
//*  _ __ (_)__| (_) |_  )  ____ ___ __   *
//* | '  \| / _` | |  / /  / _\ V / '  \  *
//* |_|_|_|_\__,_|_| /___| \__|\_/|_|_|_| *
//*                                       *
class UMPProcessorMIDI2CVM : public UMPProcessor {};

// NOLINTNEXTLINE
TEST_F(UMPProcessorMIDI2CVM, NoteOn) {
  midi2::types::m2cvm::note message;
  message.w0.mt = ump_mt(midi2::ump_message_type::m2cvm);
  message.w0.group = std::uint8_t{0};
  message.w0.status = ump_cvm(midi2::status::note_on);
  message.w0.channel = std::uint8_t{3};
  message.w0.note = std::uint8_t{60};
  message.w0.attribute = 0;
  message.w1.velocity = std::uint16_t{0x432};
  message.w1.attribute = 0;
  EXPECT_CALL(config_.m2cvm, note_on(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorMIDI2CVM, NoteOff) {
  midi2::types::m2cvm::note n;
  n.w0.mt = ump_mt(midi2::ump_message_type::m2cvm);
  n.w0.group = std::uint8_t{0};
  n.w0.status = ump_cvm(midi2::status::note_off);
  n.w0.channel = std::uint8_t{3};
  n.w0.note = std::uint8_t{60};
  n.w0.attribute = 0;
  n.w1.velocity = std::uint16_t{0x432};
  n.w1.attribute = 0;
  EXPECT_CALL(config_.m2cvm, note_off(config_.context, n)).Times(1);
  processor_.processUMP(n.w0, n.w1);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorMIDI2CVM, ProgramChange) {
  midi2::types::m2cvm::program_change message;
  message.w0.mt = to_underlying(midi2::ump_message_type::m2cvm);
  message.w0.group = std::uint8_t{0};
  message.w0.status = ump_cvm(midi2::status::program_change);
  message.w0.channel = std::uint8_t{3};
  message.w0.reserved = 0;
  message.w0.option_flags = 0;
  message.w0.bank_valid = true;
  message.w1.program = 0b10101010;
  message.w1.bank_msb = 0b01010101;
  message.w1.bank_lsb = 0b00101010;
  EXPECT_CALL(config_.m2cvm, program_change(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorMIDI2CVM, ControllerMessage) {
  midi2::types::m2cvm::controller_message message;
  message.w0.mt = to_underlying(midi2::ump_message_type::m2cvm);
  message.w0.group = std::uint8_t{0};
  message.w0.status = to_underlying(midi2::midi2status::rpn) >> 4;
  message.w0.channel = std::uint8_t{3};
  message.w0.bank = 1;
  message.w0.index = 2;
  message.w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, controller_message(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorMIDI2CVM, ChannelPressure) {
  midi2::types::m2cvm::channel_pressure message;
  message.w0.mt = to_underlying(midi2::ump_message_type::m2cvm);
  message.w0.group = std::uint8_t{0};
  message.w0.status = to_underlying(midi2::status::channel_pressure) >> 4;
  message.w0.channel = std::uint8_t{3};
  message.w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, channel_pressure(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorMIDI2CVM, RPNPerNote) {
  midi2::types::m2cvm::per_note_controller message;
  message.w0.mt = to_underlying(midi2::ump_message_type::m2cvm);
  message.w0.group = std::uint8_t{0};
  message.w0.status = to_underlying(midi2::midi2status::rpn_pernote) >> 4;
  message.w0.channel = std::uint8_t{3};
  message.w0.note = 60;
  message.w0.index = 1;
  message.w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, rpn_controller(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorMIDI2CVM, NRPNPerNote) {
  midi2::types::m2cvm::per_note_controller message;
  message.w0.mt = to_underlying(midi2::ump_message_type::m2cvm);
  message.w0.group = std::uint8_t{0};
  message.w0.status = to_underlying(midi2::midi2status::nrpn_pernote) >> 4;
  message.w0.channel = std::uint8_t{3};
  message.w0.note = 60;
  message.w0.index = 1;
  message.w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, nrpn_controller(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1);
}

//*     _      _          _ ___ ___  *
//*  __| |__ _| |_ __ _  / |_  | _ ) *
//* / _` / _` |  _/ _` | | |/ // _ \ *
//* \__,_\__,_|\__\__,_| |_/___\___/ *
//*                                  *
class UMPProcessorData128 : public UMPProcessor {};

// NOLINTNEXTLINE
TEST_F(UMPProcessorData128, Sysex8In1) {
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
  EXPECT_CALL(config_.data128, sysex8_in_1(config_.context, part0)).Times(1);
  processor_.processUMP(part0.w0, part0.w1, part0.w2, part0.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorData128, Sysex8StartAndEnd) {
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
    EXPECT_CALL(config_.data128, sysex8_start(config_.context, part0)).Times(1);
    EXPECT_CALL(config_.data128, sysex8_continue(config_.context, part1)).Times(1);
    EXPECT_CALL(config_.data128, sysex8_end(config_.context, part2)).Times(1);
  }

  processor_.processUMP(part0.w0, part0.w1, part0.w2, part0.w3);
  processor_.processUMP(part1.w0, part1.w1, part1.w2, part1.w3);
  processor_.processUMP(part2.w0, part2.w1, part2.w2, part2.w3);
}

// NOLINTNEXTLINE
TEST_F(UMPProcessorData128, MixedDatSet) {
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
    EXPECT_CALL(config_.data128, mds_header(config_.context, header)).Times(1);
    EXPECT_CALL(config_.data128, mds_payload(config_.context, payload)).Times(1);
  }

  processor_.processUMP(header.w0, header.w1, header.w2, header.w3);
  processor_.processUMP(payload.w0, payload.w1, payload.w2, payload.w3);
}

// NOLINTNEXTLINE
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
class UMPProcessorStream : public UMPProcessor {};

// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, EndpointDiscovery) {
  midi2::types::ump_stream::endpoint_discovery message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x03;
  message.w0.status = to_underlying(midi2::ump_stream::endpoint_discovery);
  message.w0.version_major = 0x01;
  message.w0.version_minor = 0x01;
  message.w1.filter = 0b00011111;
  EXPECT_CALL(config_.ump_stream, endpoint_discovery(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, EndpointInfoNotification) {
  midi2::types::ump_stream::endpoint_info_notification message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::endpoint_info_notification);
  message.w0.version_major = 0x01;
  message.w0.version_minor = 0x01;
  message.w1.static_function_blocks = 1;
  message.w1.number_function_blocks = 0b0101010;
  message.w1.midi2_protocol_capability = 1;
  message.w1.midi1_protocol_capability = 0;
  message.w1.receive_jr_timestamp_capability = 1;
  message.w1.transmit_jr_timestamp_capability = 0;
  EXPECT_CALL(config_.ump_stream, endpoint_info_notification(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, DeviceIdentityNotification) {
  midi2::types::ump_stream::device_identity_notification message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::device_identity_notification);
  message.w1.dev_manuf_sysex_id_1 = 1;
  message.w1.dev_manuf_sysex_id_2 = 1;
  message.w1.dev_manuf_sysex_id_3 = 0;
  message.w2.device_family_lsb = 0x79;
  message.w2.device_family_msb = 0x7B;
  message.w2.device_family_model_lsb = 0x7D;
  message.w2.device_family_model_msb = 0x7F;
  message.w3.sw_revision_1 = 0x7F;
  message.w3.sw_revision_2 = 0x7D;
  message.w3.sw_revision_3 = 0x7B;
  message.w3.sw_revision_4 = 0x79;
  EXPECT_CALL(config_.ump_stream, device_identity_notification(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, EndpointNameNotification) {
  midi2::types::ump_stream::endpoint_name_notification message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::endpoint_name_notification);
  message.w0.name1 = std::uint8_t{'a'};
  message.w0.name2 = std::uint8_t{'b'};
  message.w1.name3 = std::uint8_t{'c'};
  message.w1.name4 = std::uint8_t{'d'};
  message.w1.name5 = std::uint8_t{'e'};
  message.w1.name6 = std::uint8_t{'f'};
  message.w2.name7 = std::uint8_t{'g'};
  message.w2.name8 = std::uint8_t{'h'};
  message.w2.name9 = std::uint8_t{'i'};
  message.w2.name10 = std::uint8_t{'j'};
  message.w3.name11 = std::uint8_t{'k'};
  message.w3.name12 = std::uint8_t{'l'};
  message.w3.name13 = std::uint8_t{'m'};
  message.w3.name14 = std::uint8_t{'m'};
  EXPECT_CALL(config_.ump_stream, endpoint_name_notification(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, ProductInstanceIdNotification) {
  midi2::types::ump_stream::product_instance_id_notification message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::product_instance_id_notification);
  message.w0.pid1 = 0x22;
  message.w0.pid2 = 0x33;
  message.w1.pid3 = 0x44;
  message.w1.pid4 = 0x55;
  message.w1.pid5 = 0x66;
  message.w1.pid6 = 0x77;
  message.w2.pid7 = 0x88;
  message.w2.pid8 = 0x99;
  message.w2.pid9 = 0xAA;
  message.w2.pid10 = 0xBB;
  message.w3.pid11 = 0xCC;
  message.w3.pid12 = 0xDD;
  message.w3.pid13 = 0xEE;
  message.w3.pid14 = 0xFF;
  EXPECT_CALL(config_.ump_stream, product_instance_id_notification(config_.context, message)).Times(1);

  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, JRConfigurationRequest) {
  midi2::types::ump_stream::jr_configuration_request message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::jr_configuration_request);
  message.w0.protocol = 0x02;
  message.w0.rxjr = 1;
  message.w0.txjr = 0;
  EXPECT_CALL(config_.ump_stream, jr_configuration_request(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, JRConfigurationNotification) {
  midi2::types::ump_stream::jr_configuration_notification message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::jr_configuration_notification);
  message.w0.protocol = 0x02;
  message.w0.rxjr = 1;
  message.w0.txjr = 0;
  EXPECT_CALL(config_.ump_stream, jr_configuration_notification(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, FunctionBlockDiscovery) {
  midi2::types::ump_stream::function_block_discovery message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::function_block_discovery);
  message.w0.block_num = 0xFF;
  message.w0.filter = 0x03;
  EXPECT_CALL(config_.ump_stream, function_block_discovery(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, FunctionBlockInfoNotification) {
  midi2::types::ump_stream::function_block_info_notification message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::function_block_info_notification);
  message.w0.block_active = 1;
  message.w0.block_num = 0x1F;
  message.w0.ui_hint = 0b10;
  message.w0.midi1 = 0;
  message.w0.direction = 0b10;
  message.w1.first_group = 0b10101010;
  message.w1.num_spanned = 0x10;
  message.w1.ci_message_version = 0x1;
  message.w1.max_sys8_streams = 2;
  EXPECT_CALL(config_.ump_stream, function_block_info_notification(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, FunctionBlockNameNotification) {
  midi2::types::ump_stream::function_block_name_notification message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::function_block_name_notification);
  message.w0.block_num = 0x1F;
  message.w0.name0 = 'a';
  message.w1.name1 = 'b';
  message.w1.name2 = 'c';
  message.w1.name3 = 'd';
  message.w1.name4 = 'e';
  message.w2.name5 = 'f';
  message.w2.name6 = 'g';
  message.w2.name7 = 'h';
  message.w2.name8 = 'i';
  message.w3.name9 = 'k';
  message.w3.name10 = 'l';
  message.w3.name11 = 'm';
  message.w3.name12 = 'n';
  EXPECT_CALL(config_.ump_stream, function_block_name_notification(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, StartOfClip) {
  midi2::types::ump_stream::start_of_clip message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::start_of_clip);
  EXPECT_CALL(config_.ump_stream, start_of_clip(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorStream, EndOfClip) {
  midi2::types::ump_stream::end_of_clip message{};
  message.w0.mt = to_underlying(midi2::ump_message_type::ump_stream);
  message.w0.format = 0x00;
  message.w0.status = to_underlying(midi2::ump_stream::end_of_clip);
  EXPECT_CALL(config_.ump_stream, end_of_clip(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}

//*  ___ _           ___       _         *
//* | __| |_____ __ |   \ __ _| |_ __ _  *
//* | _|| / -_) \ / | |) / _` |  _/ _` | *
//* |_| |_\___/_\_\ |___/\__,_|\__\__,_| *
//*                                      *
class UMPProcessorFlexData : public UMPProcessor {};

// NOLINTNEXTLINE
TEST_F(UMPProcessorFlexData, SetTempo) {
  midi2::types::flex_data::set_tempo message;
  message.w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  message.w0.group = 0;
  message.w0.form = 0;
  message.w0.addrs = 1;
  message.w0.channel = 0;
  message.w0.status_bank = 0;
  message.w0.status = to_underlying(midi2::flex_data::set_tempo);
  message.w1 = std::uint32_t{0xF0F0F0F0};
  EXPECT_CALL(config_.flex, set_tempo(config_.context, message)).Times(1);
  processor_.processUMP(message.w0);
  processor_.processUMP(message.w1);
  processor_.processUMP(message.w2);
  processor_.processUMP(message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorFlexData, SetTimeSignature) {
  midi2::types::flex_data::set_time_signature message;
  message.w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  message.w0.group = 0;
  message.w0.form = 0;
  message.w0.addrs = 1;
  message.w0.channel = 3;
  message.w0.status_bank = 0;
  message.w0.status = to_underlying(midi2::flex_data::set_time_signature);
  message.w1.numerator = 1;
  message.w1.denominator = 2;
  message.w1.number_of_32_notes = 16;
  EXPECT_CALL(config_.flex, set_time_signature(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorFlexData, SetMetronome) {
  midi2::types::flex_data::set_metronome message;
  message.w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  message.w0.group = 0;
  message.w0.form = 0;
  message.w0.addrs = 1;
  message.w0.channel = 3;
  message.w0.status_bank = 0;
  message.w0.status = to_underlying(midi2::flex_data::set_metronome);
  message.w1.num_clocks_per_primary_click = 24;
  message.w1.bar_accent_part_1 = 4;
  message.w1.bar_accent_part_2 = 0;
  message.w1.bar_accent_part_3 = 0;
  message.w2.num_subdivision_clicks_1 = 0;
  message.w2.num_subdivision_clicks_2 = 0;
  EXPECT_CALL(config_.flex, set_metronome(config_.context, message)).Times(1);

  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorFlexData, SetKeySignature) {
  midi2::types::flex_data::set_key_signature message;
  message.w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  message.w0.group = 0;
  message.w0.form = 0;
  message.w0.addrs = 1;
  message.w0.channel = 3;
  message.w0.status_bank = 0;
  message.w0.status = to_underlying(midi2::flex_data::set_key_signature);
  message.w1.sharps_flats = 0b100;  // (-8)
  message.w1.tonic_note = static_cast<std::uint8_t>(midi2::types::flex_data::note::E);
  EXPECT_CALL(config_.flex, set_key_signature(config_.context, message)).Times(1);
  processor_.processUMP(message.w0, message.w1, message.w2, message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorFlexData, SetChordName) {
  midi2::types::flex_data::set_chord_name message;
  message.w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  message.w0.group = 0x0F;
  message.w0.form = 0x0;
  message.w0.addrs = 3;
  message.w0.channel = 3;
  message.w0.status_bank = 0x00;
  message.w0.status = to_underlying(midi2::flex_data::set_chord_name);
  message.w1.tonic_sharps_flats = 0x1;
  message.w1.chord_tonic = midi2::to_underlying(midi2::types::flex_data::note::E);
  message.w1.chord_type = midi2::to_underlying(midi2::types::flex_data::chord_type::augmented);
  message.w1.alter_1_type = 1;
  message.w1.alter_1_degree = 5;
  message.w1.alter_2_type = 2;
  message.w1.alter_2_degree = 6;
  message.w2.alter_3_type = 3;
  message.w2.alter_3_degree = 7;
  message.w2.alter_4_type = 4;
  message.w2.alter_4_degree = 8;
  message.w2.reserved = 0x0000;
  message.w3.bass_sharps_flats = 0xE;
  message.w3.bass_note = midi2::to_underlying(midi2::types::flex_data::note::unknown);
  message.w3.bass_chord_type = midi2::to_underlying(midi2::types::flex_data::chord_type::diminished);
  message.w3.alter_1_type = 1;
  message.w3.alter_1_degree = 3;
  message.w3.alter_2_type = 2;
  message.w3.alter_2_degree = 4;
  EXPECT_CALL(config_.flex, set_chord_name(config_.context, message)).Times(1);
  processor_.processUMP(message.w0);
  processor_.processUMP(message.w1);
  processor_.processUMP(message.w2);
  processor_.processUMP(message.w3);
}
// NOLINTNEXTLINE
TEST_F(UMPProcessorFlexData, Text) {
  midi2::types::flex_data::text_common message;
  message.w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  message.w0.group = 0;
  message.w0.form = 0;
  message.w0.addrs = 1;
  message.w0.channel = 3;
  message.w0.status_bank = 1;
  message.w0.status = 4;
  message.w1 =
      (std::uint32_t{0xC2} << 24) | (std::uint32_t{0xA9} << 16) | (std::uint32_t{'2'} << 8) | (std::uint32_t{'0'} << 0);
  message.w2 =
      (std::uint32_t{'2'} << 24) | (std::uint32_t{'4'} << 16) | (std::uint32_t{' '} << 8) | (std::uint32_t{'P'} << 0);
  message.w3 =
      (std::uint32_t{'B'} << 24) | (std::uint32_t{'H'} << 16) | (std::uint32_t{'\0'} << 8) | (std::uint32_t{'\0'} << 0);
  EXPECT_CALL(config_.flex, text(config_.context, message)).Times(1);

  processor_.processUMP(message.w0);
  processor_.processUMP(message.w1);
  processor_.processUMP(message.w2);
  processor_.processUMP(message.w3);
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
