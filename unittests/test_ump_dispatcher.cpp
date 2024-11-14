//===-- UMP Dispatcher --------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ump_dispatcher.hpp"

// MIDI2 library
#include "midi2/adt/bitfield.hpp"
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
  [[maybe_unused]] utility_base(utility_base const &) = default;
  [[maybe_unused]] utility_base(utility_base &&) noexcept = default;
  virtual ~utility_base() noexcept = default;

  [[maybe_unused]] utility_base &operator=(utility_base const &) = default;
  [[maybe_unused]] utility_base &operator=(utility_base &&) noexcept = default;

  virtual void noop(context_type) = 0;
  virtual void jr_clock(context_type, midi2::types::utility::jr_clock) = 0;
  virtual void jr_timestamp(context_type, midi2::types::utility::jr_timestamp) = 0;
  virtual void delta_clockstamp_tpqn(context_type, midi2::types::utility::delta_clockstamp_tpqn) = 0;
  virtual void delta_clockstamp(context_type, midi2::types::utility::delta_clockstamp) = 0;

  virtual void unknown(context_type, std::span<std::uint32_t>) = 0;
};
class UtilityMocks : public utility_base {
public:
  MOCK_METHOD(void, noop, (context_type), (override));
  MOCK_METHOD(void, jr_clock, (context_type, midi2::types::utility::jr_clock), (override));
  MOCK_METHOD(void, jr_timestamp, (context_type, midi2::types::utility::jr_timestamp), (override));
  MOCK_METHOD(void, delta_clockstamp_tpqn, (context_type, midi2::types::utility::delta_clockstamp_tpqn), (override));
  MOCK_METHOD(void, delta_clockstamp, (context_type, midi2::types::utility::delta_clockstamp), (override));

  MOCK_METHOD(void, unknown, (context_type, std::span<std::uint32_t>), (override));
};
struct system_base {
  system_base() = default;
  [[maybe_unused]] system_base(system_base const &) = default;
  [[maybe_unused]] system_base(system_base &&) noexcept = default;
  virtual ~system_base() noexcept = default;

  [[maybe_unused]] system_base &operator=(system_base const &) = default;
  [[maybe_unused]] system_base &operator=(system_base &&) noexcept = default;

  virtual void midi_time_code(context_type, midi2::types::system::midi_time_code) = 0;
  virtual void song_position_pointer(context_type, midi2::types::system::song_position_pointer) = 0;
  virtual void song_select(context_type, midi2::types::system::song_select) = 0;
  virtual void tune_request(context_type, midi2::types::system::tune_request) = 0;
  virtual void timing_clock(context_type, midi2::types::system::timing_clock) = 0;
  virtual void seq_start(context_type, midi2::types::system::sequence_start) = 0;
  virtual void seq_continue(context_type, midi2::types::system::sequence_continue) = 0;
  virtual void seq_stop(context_type, midi2::types::system::sequence_stop) = 0;
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
  MOCK_METHOD(void, seq_start, (context_type, midi2::types::system::sequence_start), (override));
  MOCK_METHOD(void, seq_continue, (context_type, midi2::types::system::sequence_continue), (override));
  MOCK_METHOD(void, seq_stop, (context_type, midi2::types::system::sequence_stop), (override));
  MOCK_METHOD(void, active_sensing, (context_type, midi2::types::system::active_sensing), (override));
  MOCK_METHOD(void, reset, (context_type, midi2::types::system::reset), (override));
};
struct m1cvm_base {
  m1cvm_base() = default;
  [[maybe_unused]] m1cvm_base(m1cvm_base const &) = default;
  [[maybe_unused]] m1cvm_base(m1cvm_base &&) noexcept = default;
  virtual ~m1cvm_base() noexcept = default;

  [[maybe_unused]] m1cvm_base &operator=(m1cvm_base const &) = default;
  [[maybe_unused]] m1cvm_base &operator=(m1cvm_base &&) noexcept = default;

  virtual void note_off(context_type, midi2::types::m1cvm::note_off) = 0;
  virtual void note_on(context_type, midi2::types::m1cvm::note_on) = 0;
  virtual void poly_pressure(context_type, midi2::types::m1cvm::poly_pressure) = 0;
  virtual void control_change(context_type, midi2::types::m1cvm::control_change) = 0;
  virtual void program_change(context_type, midi2::types::m1cvm::program_change) = 0;
  virtual void channel_pressure(context_type, midi2::types::m1cvm::channel_pressure) = 0;
  virtual void pitch_bend(context_type, midi2::types::m1cvm::pitch_bend) = 0;
};
class M1CVMMocks : public m1cvm_base {
public:
  MOCK_METHOD(void, note_off, (context_type, midi2::types::m1cvm::note_off), (override));
  MOCK_METHOD(void, note_on, (context_type, midi2::types::m1cvm::note_on), (override));
  MOCK_METHOD(void, poly_pressure, (context_type, midi2::types::m1cvm::poly_pressure), (override));
  MOCK_METHOD(void, control_change, (context_type, midi2::types::m1cvm::control_change), (override));
  MOCK_METHOD(void, program_change, (context_type, midi2::types::m1cvm::program_change), (override));
  MOCK_METHOD(void, channel_pressure, (context_type, midi2::types::m1cvm::channel_pressure), (override));
  MOCK_METHOD(void, pitch_bend, (context_type, midi2::types::m1cvm::pitch_bend), (override));
};
struct data64_base {
  data64_base() = default;
  [[maybe_unused]] data64_base(data64_base const &) = default;
  [[maybe_unused]] data64_base(data64_base &&) noexcept = default;
  virtual ~data64_base() noexcept = default;

  [[maybe_unused]] data64_base &operator=(data64_base const &) = default;
  [[maybe_unused]] data64_base &operator=(data64_base &&) noexcept = default;

  virtual void sysex7_in_1(context_type, midi2::types::data64::sysex7_in_1) = 0;
  virtual void sysex7_start(context_type, midi2::types::data64::sysex7_start) = 0;
  virtual void sysex7_continue(context_type, midi2::types::data64::sysex7_continue) = 0;
  virtual void sysex7_end(context_type, midi2::types::data64::sysex7_end) = 0;
};
class Data64Mocks : public data64_base {
public:
  MOCK_METHOD(void, sysex7_in_1, (context_type, midi2::types::data64::sysex7_in_1), (override));
  MOCK_METHOD(void, sysex7_start, (context_type, midi2::types::data64::sysex7_start), (override));
  MOCK_METHOD(void, sysex7_continue, (context_type, midi2::types::data64::sysex7_continue), (override));
  MOCK_METHOD(void, sysex7_end, (context_type, midi2::types::data64::sysex7_end), (override));
};
struct m2cvm_base {
  m2cvm_base() = default;
  [[maybe_unused]] m2cvm_base(m2cvm_base const &) = default;
  [[maybe_unused]] m2cvm_base(m2cvm_base &&) noexcept = default;
  virtual ~m2cvm_base() noexcept = default;

  [[maybe_unused]] m2cvm_base &operator=(m2cvm_base const &) = default;
  [[maybe_unused]] m2cvm_base &operator=(m2cvm_base &&) noexcept = default;

  virtual void note_off(context_type, midi2::types::m2cvm::note_off) = 0;
  virtual void note_on(context_type, midi2::types::m2cvm::note_on) = 0;
  virtual void poly_pressure(context_type, midi2::types::m2cvm::poly_pressure) = 0;
  virtual void program_change(context_type, midi2::types::m2cvm::program_change) = 0;
  virtual void channel_pressure(context_type, midi2::types::m2cvm::channel_pressure) = 0;

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  virtual void rpn_per_note_controller(context_type, midi2::types::m2cvm::rpn_per_note_controller) = 0;
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  virtual void nrpn_per_note_controller(context_type, midi2::types::m2cvm::nrpn_per_note_controller) = 0;
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  virtual void rpn_controller(context_type, midi2::types::m2cvm::rpn_controller) = 0;
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  virtual void nrpn_controller(context_type, midi2::types::m2cvm::nrpn_controller) = 0;
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  virtual void rpn_relative_controller(context_type, midi2::types::m2cvm::rpn_relative_controller) = 0;
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  virtual void nrpn_relative_controller(context_type, midi2::types::m2cvm::nrpn_relative_controller) = 0;

  virtual void per_note_management(context_type, midi2::types::m2cvm::per_note_management) = 0;
  virtual void control_change(context_type, midi2::types::m2cvm::control_change) = 0;
  virtual void pitch_bend(context_type, midi2::types::m2cvm::pitch_bend) = 0;
  virtual void per_note_pitch_bend(context_type, midi2::types::m2cvm::per_note_pitch_bend) = 0;
};
class M2CVMMocks : public m2cvm_base {
public:
  MOCK_METHOD(void, note_off, (context_type, midi2::types::m2cvm::note_off), (override));
  MOCK_METHOD(void, note_on, (context_type, midi2::types::m2cvm::note_on), (override));
  MOCK_METHOD(void, poly_pressure, (context_type, midi2::types::m2cvm::poly_pressure), (override));
  MOCK_METHOD(void, program_change, (context_type, midi2::types::m2cvm::program_change), (override));
  MOCK_METHOD(void, channel_pressure, (context_type, midi2::types::m2cvm::channel_pressure), (override));

  MOCK_METHOD(void, rpn_per_note_controller, (context_type, midi2::types::m2cvm::rpn_per_note_controller), (override));
  MOCK_METHOD(void, nrpn_per_note_controller, (context_type, midi2::types::m2cvm::nrpn_per_note_controller),
              (override));
  MOCK_METHOD(void, rpn_controller, (context_type, midi2::types::m2cvm::rpn_controller), (override));
  MOCK_METHOD(void, nrpn_controller, (context_type, midi2::types::m2cvm::nrpn_controller), (override));
  MOCK_METHOD(void, rpn_relative_controller, (context_type, midi2::types::m2cvm::rpn_relative_controller), (override));
  MOCK_METHOD(void, nrpn_relative_controller, (context_type, midi2::types::m2cvm::nrpn_relative_controller),
              (override));

  MOCK_METHOD(void, per_note_management, (context_type, midi2::types::m2cvm::per_note_management), (override));
  MOCK_METHOD(void, control_change, (context_type, midi2::types::m2cvm::control_change), (override));
  MOCK_METHOD(void, pitch_bend, (context_type, midi2::types::m2cvm::pitch_bend), (override));
  MOCK_METHOD(void, per_note_pitch_bend, (context_type, midi2::types::m2cvm::per_note_pitch_bend), (override));
};
struct data128_base {
  data128_base() = default;
  [[maybe_unused]] data128_base(data128_base const &) = default;
  [[maybe_unused]] data128_base(data128_base &&) noexcept = default;
  virtual ~data128_base() noexcept = default;

  [[maybe_unused]] data128_base &operator=(data128_base const &) = default;
  [[maybe_unused]] data128_base &operator=(data128_base &&) noexcept = default;

  virtual void sysex8_in_1(context_type, midi2::types::data128::sysex8_in_1 const &) = 0;
  virtual void sysex8_start(context_type, midi2::types::data128::sysex8_start const &) = 0;
  virtual void sysex8_continue(context_type, midi2::types::data128::sysex8_continue const &) = 0;
  virtual void sysex8_end(context_type, midi2::types::data128::sysex8_end const &) = 0;
  virtual void mds_header(context_type, midi2::types::data128::mds_header const &) = 0;
  virtual void mds_payload(context_type, midi2::types::data128::mds_payload const &) = 0;
};
class Data128Mocks : public data128_base {
public:
  MOCK_METHOD(void, sysex8_in_1, (context_type, midi2::types::data128::sysex8_in_1 const &), (override));
  MOCK_METHOD(void, sysex8_start, (context_type, midi2::types::data128::sysex8_start const &), (override));
  MOCK_METHOD(void, sysex8_continue, (context_type, midi2::types::data128::sysex8_continue const &), (override));
  MOCK_METHOD(void, sysex8_end, (context_type, midi2::types::data128::sysex8_end const &), (override));
  MOCK_METHOD(void, mds_header, (context_type, midi2::types::data128::mds_header const &), (override));
  MOCK_METHOD(void, mds_payload, (context_type, midi2::types::data128::mds_payload const &), (override));
};
struct ump_stream_base {
  ump_stream_base() = default;
  [[maybe_unused]] ump_stream_base(ump_stream_base const &) = default;
  [[maybe_unused]] ump_stream_base(ump_stream_base &&) noexcept = default;
  virtual ~ump_stream_base() noexcept = default;

  [[maybe_unused]] ump_stream_base &operator=(ump_stream_base const &) = default;
  [[maybe_unused]] ump_stream_base &operator=(ump_stream_base &&) noexcept = default;

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
  [[maybe_unused]] flex_data_base(flex_data_base const &) = default;
  [[maybe_unused]] flex_data_base(flex_data_base &&) noexcept = default;
  virtual ~flex_data_base() noexcept = default;

  [[maybe_unused]] flex_data_base &operator=(flex_data_base const &) = default;
  [[maybe_unused]] flex_data_base &operator=(flex_data_base &&) noexcept = default;

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

class UMPDispatcher : public testing::Test {
public:
  UMPDispatcher() : dispatcher_{std::ref(config_)} {}

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
  midi2::ump_dispatcher<mocked_config &> dispatcher_;
};

//*       _   _ _ _ _         *
//*  _  _| |_(_) (_) |_ _  _  *
//* | || |  _| | | |  _| || | *
//*  \_,_|\__|_|_|_|\__|\_, | *
//*                     |__/  *
class UMPDispatcherUtility : public UMPDispatcher {};

// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, Noop) {
  EXPECT_CALL(config_.utility, noop(config_.context)).Times(1);

  midi2::types::utility::noop message{};
  dispatcher_.processUMP(get<0>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, JRClock) {
  midi2::types::utility::jr_clock message;
  using word0 = decltype(message)::word0;
  get<word0>(message.w).set<word0::sender_clock_time>(0b1010101010101010);
  EXPECT_CALL(config_.utility, jr_clock(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<word0>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, JRTimestamp) {
  midi2::types::utility::jr_timestamp message{};
  using word0 = decltype(message)::word0;
  get<word0>(message.w).set<word0::timestamp>((1U << 16) - 1U);
  EXPECT_CALL(config_.utility, jr_timestamp(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, DeltaClockstampTqpn) {
  midi2::types::utility::delta_clockstamp_tpqn message{};
  using word0 = decltype(message)::word0;
  std::get<word0>(message.w).set<word0::ticks_pqn>(0b1010101010101010);
  EXPECT_CALL(config_.utility, delta_clockstamp_tpqn(config_.context, message)).Times(1);
  dispatcher_.processUMP(std::get<0>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, DeltaClockstamp) {
  midi2::types::utility::delta_clockstamp message{};
  using word0 = decltype(message)::word0;
  auto &w0 = get<word0>(message.w);
  w0.set<word0::ticks_per_quarter_note>((1U << word0::ticks_per_quarter_note::bits()) - 1U);
  EXPECT_CALL(config_.utility, delta_clockstamp(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, BadMessage) {
  midi2::types::utility::delta_clockstamp message;
  using word0 = decltype(message)::word0;
  auto &w0 = get<word0>(message.w);
  // Splash the mt and status fields with something that will not be recognized.
  w0.set<word0::mt>(to_underlying(midi2::ump_message_type::utility));
  w0.set<word0::status>(std::uint8_t{0b1111});
  EXPECT_CALL(config_.utility, unknown(config_.context, ElementsAre(std::bit_cast<std::uint32_t>(w0))));
  dispatcher_.processUMP(w0);
}

//*  ___         _              *
//* / __|_  _ __| |_ ___ _ __   *
//* \__ \ || (_-<  _/ -_) '  \  *
//* |___/\_, /__/\__\___|_|_|_| *
//*      |__/                   *
class UMPDispatcherSystem : public UMPDispatcher {};
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, MIDITimeCode) {
  midi2::types::system::midi_time_code message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  w0.set<word0::time_code>(0b1010101);
  EXPECT_CALL(config_.system, midi_time_code(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, SongPositionPointer) {
  midi2::types::system::song_position_pointer message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  w0.set<word0::position_lsb>(0b1010101);
  w0.set<word0::position_msb>(0b1111111);
  EXPECT_CALL(config_.system, song_position_pointer(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, SongSelect) {
  midi2::types::system::song_select message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  w0.set<word0::song>(0b1010101);
  EXPECT_CALL(config_.system, song_select(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, TuneRequest) {
  midi2::types::system::tune_request message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  EXPECT_CALL(config_.system, tune_request(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, TimingClock) {
  midi2::types::system::timing_clock message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  EXPECT_CALL(config_.system, timing_clock(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, Start) {
  midi2::types::system::sequence_start message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  EXPECT_CALL(config_.system, seq_start(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, Continue) {
  midi2::types::system::sequence_continue message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  EXPECT_CALL(config_.system, seq_continue(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, Stop) {
  midi2::types::system::sequence_stop message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  EXPECT_CALL(config_.system, seq_stop(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, ActiveSensing) {
  midi2::types::system::active_sensing message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  EXPECT_CALL(config_.system, active_sensing(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, Reset) {
  midi2::types::system::reset message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  EXPECT_CALL(config_.system, reset(config_.context, message));
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, BadStatus) {
  midi2::types::system::reset message;
  using word0 = decltype(message)::word0;
  auto &w0 = std::get<0>(message.w);
  w0.set<word0::group>(0);
  w0.set<word0::status>(0x00);
  EXPECT_CALL(config_.utility, unknown(config_.context, ElementsAre(w0.word())));
  dispatcher_.processUMP(w0);
}

//*        _    _ _   _   __   *
//*  _ __ (_)__| (_) / | /  \  *
//* | '  \| / _` | | | || () | *
//* |_|_|_|_\__,_|_| |_(_)__/  *
//*                            *
constexpr std::uint8_t ump_cvm(midi2::status s) {
  static_assert(std::is_same_v<std::underlying_type_t<midi2::status>, std::uint8_t>,
                "status type must be a std::uint8_t");
  assert((to_underlying(s) & 0x0F) == 0 && "Bottom 4 bits of a channel voice message status enum must be 0");
  return to_underlying(s) >> 4;
}

class UMPDispatcherMIDI1 : public UMPDispatcher {};
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI1, NoteOn) {
  midi2::types::m1cvm::note_on message;
  auto &w0 = std::get<0>(message.w);
  w0.group = 0;
  w0.channel = 3;
  w0.note = 60;
  w0.velocity = 0x43;
  EXPECT_CALL(config_.m1cvm, note_on(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI1, NoteOff) {
  midi2::types::m1cvm::note_off message;
  auto &w0 = get<0>(message.w);
  w0.group = 0;
  w0.channel = 3;
  w0.note = 60;
  w0.velocity = 0x43;
  EXPECT_CALL(config_.m1cvm, note_off(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI1, PolyPressure) {
  midi2::types::m1cvm::poly_pressure message;
  auto &w0 = get<0>(message.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w0.note = std::uint8_t{60};
  w0.pressure = std::uint8_t{0x43};
  EXPECT_CALL(config_.m1cvm, poly_pressure(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI1, ControlChange) {
  midi2::types::m1cvm::control_change message;
  auto &w0 = get<0>(message.w);
  w0.group = 0;
  w0.channel = 3;
  w0.controller = 60;
  w0.value = 127;
  EXPECT_CALL(config_.m1cvm, control_change(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI1, ChannelPressure) {
  midi2::types::m1cvm::channel_pressure message;
  auto &w0 = get<0>(message.w);
  w0.group = 0;
  w0.channel = 3;
  w0.data = 0b01010101;
  EXPECT_CALL(config_.m1cvm, channel_pressure(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0);
}

//*     _      _           __ _ _   *
//*  __| |__ _| |_ __ _   / /| | |  *
//* / _` / _` |  _/ _` | / _ \_  _| *
//* \__,_\__,_|\__\__,_| \___/ |_|  *
//*                                 *
// NOLINTNEXTLINE
TEST_F(UMPDispatcher, Data64SysExIn1) {
  midi2::types::data64::sysex7_in_1 m0;
  using word0 = decltype(m0)::word0;
  using word1 = decltype(m0)::word1;
  get<word0>(m0.w).template set<word0::group>(0);
  get<word0>(m0.w).template set<word0::number_of_bytes>(4);
  get<word0>(m0.w).template set<word0::data0>(2);
  get<word0>(m0.w).template set<word0::data1>(3);
  get<word1>(m0.w).template set<word1::data2>(5);
  get<word1>(m0.w).template set<word1::data3>(7);
  EXPECT_CALL(config_.data64, sysex7_in_1(config_.context, m0)).Times(1);
  dispatcher_.processUMP(get<word0>(m0.w).word(), get<word1>(m0.w).word());
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcher, Data64Sysex8StartAndEnd) {
  constexpr auto group = std::uint8_t{0};

  midi2::types::data64::sysex7_start m0;
  using m0w0 = decltype(m0)::word0;
  using m0w1 = decltype(m0)::word1;
  get<m0w0>(m0.w).template set<m0w0::group>(group);
  get<m0w0>(m0.w).template set<m0w0::number_of_bytes>(6);
  get<m0w0>(m0.w).template set<m0w0::data0>(2);
  get<m0w0>(m0.w).template set<m0w0::data1>(3);
  get<m0w1>(m0.w).template set<m0w1::data2>(5);
  get<m0w1>(m0.w).template set<m0w1::data3>(7);
  get<m0w1>(m0.w).template set<m0w1::data4>(11);
  get<m0w1>(m0.w).template set<m0w1::data5>(13);

  midi2::types::data64::sysex7_continue m1;
  using m1w0 = decltype(m1)::word0;
  using m1w1 = decltype(m1)::word1;
  get<m1w0>(m1.w).template set<m1w0::group>(group);
  get<m1w0>(m1.w).template set<m1w0::number_of_bytes>(6);
  get<m1w0>(m1.w).template set<m1w0::data0>(17);
  get<m1w0>(m1.w).template set<m1w0::data1>(19);
  get<m1w1>(m1.w).template set<m1w1::data2>(23);
  get<m1w1>(m1.w).template set<m1w1::data3>(29);
  get<m1w1>(m1.w).template set<m1w1::data4>(31);
  get<m1w1>(m1.w).template set<m1w1::data5>(37);

  midi2::types::data64::sysex7_end m2;
  using m2w0 = decltype(m2)::word0;
  using m2w1 = decltype(m2)::word1;
  get<m2w0>(m2.w).template set<m2w0::group>(group);
  get<m2w0>(m2.w).template set<m2w0::number_of_bytes>(4);
  get<m2w0>(m2.w).template set<m2w0::data0>(41);
  get<m2w0>(m2.w).template set<m2w0::data1>(43);
  get<m2w1>(m2.w).template set<m2w1::data2>(47);
  get<m2w1>(m2.w).template set<m2w1::data3>(53);
  {
    InSequence _;
    EXPECT_CALL(config_.data64, sysex7_start(config_.context, m0)).Times(1);
    EXPECT_CALL(config_.data64, sysex7_continue(config_.context, m1)).Times(1);
    EXPECT_CALL(config_.data64, sysex7_end(config_.context, m2)).Times(1);
  }
  dispatcher_.processUMP(get<m0w0>(m0.w).word(), get<m0w1>(m0.w).word());
  dispatcher_.processUMP(get<m1w0>(m1.w).word(), get<m1w1>(m1.w).word());
  dispatcher_.processUMP(get<m2w0>(m2.w).word(), get<m2w1>(m2.w).word());
}

//*        _    _ _   ___                 *
//*  _ __ (_)__| (_) |_  )  ____ ___ __   *
//* | '  \| / _` | |  / /  / _\ V / '  \  *
//* |_|_|_|_\__,_|_| /___| \__|\_/|_|_|_| *
//*                                       *
class UMPDispatcherMIDI2CVM : public UMPDispatcher {};

// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, NoteOn) {
  midi2::types::m2cvm::note_on message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w0.note = std::uint8_t{60};
  w0.attribute = 0;
  w1.velocity = std::uint16_t{0x432};
  w1.attribute = 0;
  EXPECT_CALL(config_.m2cvm, note_on(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, NoteOff) {
  midi2::types::m2cvm::note_off n;
  auto &w0 = get<0>(n.w);
  auto &w1 = get<1>(n.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w0.note = std::uint8_t{60};
  w0.attribute = 0;
  w1.velocity = std::uint16_t{0x432};
  w1.attribute = 0;
  EXPECT_CALL(config_.m2cvm, note_off(config_.context, n)).Times(1);
  dispatcher_.processUMP(w0, w1);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, ProgramChange) {
  midi2::types::m2cvm::program_change message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w0.reserved = 0;
  w0.option_flags = 0;
  w0.bank_valid = true;
  w1.program = 0b10101010;
  w1.bank_msb = 0b01010101;
  w1.bank_lsb = 0b00101010;
  EXPECT_CALL(config_.m2cvm, program_change(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, ControlChange) {
  midi2::types::m2cvm::control_change message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w0.controller = 2;
  w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, control_change(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, ChannelPressure) {
  midi2::types::m2cvm::channel_pressure message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, channel_pressure(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, RPNPerNoteController) {
  midi2::types::m2cvm::rpn_per_note_controller message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w0.note = 60;
  w0.index = 1;
  w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, rpn_per_note_controller(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, NRPNPerNoteController) {
  midi2::types::m2cvm::nrpn_per_note_controller message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w0.note = 60;
  w0.index = 1;
  w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, nrpn_per_note_controller(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, RPNController) {
  midi2::types::m2cvm::rpn_controller message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w0.bank = 23;
  w0.index = 31;
  w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, rpn_controller(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, NRPNController) {
  midi2::types::m2cvm::nrpn_controller message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = std::uint8_t{0};
  w0.channel = std::uint8_t{3};
  w0.bank = 23;
  w0.index = 31;
  w1 = 0xF0F0E1E1;
  EXPECT_CALL(config_.m2cvm, nrpn_controller(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1);
}

//*     _      _          _ ___ ___  *
//*  __| |__ _| |_ __ _  / |_  | _ ) *
//* / _` / _` |  _/ _` | | |/ // _ \ *
//* \__,_\__,_|\__\__,_| |_/___\___/ *
//*                                  *
class UMPDispatcherData128 : public UMPDispatcher {};

// NOLINTNEXTLINE
TEST_F(UMPDispatcherData128, Sysex8In1) {
  constexpr auto group = std::uint8_t{0};
  constexpr auto stream_id = std::uint8_t{0};

  midi2::types::data128::sysex8_in_1 message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  auto &w2 = get<2>(message.w);
  auto &w3 = get<3>(message.w);
  w0.group = group;
  w0.number_of_bytes = 10;
  w0.stream_id = stream_id;
  w0.data0 = 2;
  w1.data1 = 3;
  w1.data2 = 5;
  w1.data3 = 7;
  w1.data4 = 11;
  w2.data5 = 13;
  w2.data6 = 17;
  w2.data7 = 19;
  w2.data8 = 23;
  w3.data9 = 29;
  EXPECT_CALL(config_.data128, sysex8_in_1(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherData128, Sysex8StartAndEnd) {
  constexpr auto group = std::uint8_t{0};
  constexpr auto stream_id = std::uint8_t{0};

  midi2::types::data128::sysex8_start part0;
  get<0>(part0.w).group = group;
  get<0>(part0.w).number_of_bytes = 13;
  get<0>(part0.w).stream_id = stream_id;
  get<0>(part0.w).data0 = 2;
  get<1>(part0.w).data1 = 3;
  get<1>(part0.w).data2 = 5;
  get<1>(part0.w).data3 = 7;
  get<1>(part0.w).data4 = 11;
  get<2>(part0.w).data5 = 13;
  get<2>(part0.w).data6 = 17;
  get<2>(part0.w).data7 = 19;
  get<2>(part0.w).data8 = 23;
  get<3>(part0.w).data9 = 29;
  get<3>(part0.w).data10 = 31;
  get<3>(part0.w).data11 = 37;
  get<3>(part0.w).data12 = 41;
  midi2::types::data128::sysex8_continue part1;
  get<0>(part1.w).group = group;
  get<0>(part1.w).number_of_bytes = 13;
  get<0>(part1.w).stream_id = stream_id;
  get<0>(part1.w).data0 = 43;
  get<1>(part1.w).data1 = 47;
  get<1>(part1.w).data2 = 53;
  get<1>(part1.w).data3 = 59;
  get<1>(part1.w).data4 = 61;
  get<2>(part1.w).data5 = 67;
  get<2>(part1.w).data6 = 71;
  get<2>(part1.w).data7 = 73;
  get<2>(part1.w).data8 = 79;
  get<3>(part1.w).data9 = 83;
  get<3>(part1.w).data10 = 89;
  get<3>(part1.w).data11 = 97;
  get<3>(part1.w).data12 = 101;
  midi2::types::data128::sysex8_end part2;
  get<0>(part2.w).group = group;
  get<0>(part2.w).number_of_bytes = 4;
  get<0>(part2.w).stream_id = stream_id;
  get<0>(part2.w).data0 = 103;
  get<1>(part2.w).data1 = 107;
  get<1>(part2.w).data2 = 109;
  get<1>(part2.w).data2 = 113;

  {
    InSequence _;
    EXPECT_CALL(config_.data128, sysex8_start(config_.context, part0)).Times(1);
    EXPECT_CALL(config_.data128, sysex8_continue(config_.context, part1)).Times(1);
    EXPECT_CALL(config_.data128, sysex8_end(config_.context, part2)).Times(1);
  }

  dispatcher_.processUMP(get<0>(part0.w), get<1>(part0.w), get<2>(part0.w), get<3>(part0.w));
  dispatcher_.processUMP(get<0>(part1.w), get<1>(part1.w), get<2>(part1.w), get<3>(part1.w));
  dispatcher_.processUMP(get<0>(part2.w), get<1>(part2.w), get<2>(part2.w), get<3>(part2.w));
}

// NOLINTNEXTLINE
TEST_F(UMPDispatcherData128, MixedDatSet) {
  constexpr auto group = std::uint8_t{0};
  constexpr auto mds_id = std::uint8_t{0b1010};

  midi2::types::data128::mds_header header;
  get<0>(header.w).group = group;
  get<0>(header.w).mds_id = mds_id;
  get<0>(header.w).bytes_in_chunk = 2;

  get<1>(header.w).chunks_in_mds = 1;
  get<1>(header.w).chunk_num = 1;
  get<2>(header.w).manufacturer_id = 43;
  get<2>(header.w).device_id = 61;
  get<3>(header.w).sub_id_1 = 19;
  get<3>(header.w).sub_id_2 = 23;

  midi2::types::data128::mds_payload payload;
  get<0>(payload.w).group = group;
  get<0>(payload.w).mds_id = mds_id;
  get<0>(payload.w).data0 = std::uint16_t{0xFFFF};
  get<1>(payload.w) = 0xFFFFFFFF;
  get<2>(payload.w) = 0xFFFFFFFF;
  get<3>(payload.w) = 0xFFFFFFFF;
  {
    InSequence _;
    EXPECT_CALL(config_.data128, mds_header(config_.context, header)).Times(1);
    EXPECT_CALL(config_.data128, mds_payload(config_.context, payload)).Times(1);
  }
  dispatcher_.processUMP(get<0>(header.w), get<1>(header.w), get<2>(header.w), get<3>(header.w));
  dispatcher_.processUMP(get<0>(payload.w), get<1>(payload.w), get<2>(payload.w), get<3>(payload.w));
}

// NOLINTNEXTLINE
constexpr auto ump_note_on = ump_cvm(midi2::status::note_on);
TEST_F(UMPDispatcher, PartialMessageThenClear) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint16_t{0x43};  // 7 bits
  constexpr auto group = std::uint8_t{0};

  midi2::types::m1cvm::note_on message;
  auto &w0 = get<0>(message.w);
  w0.group = group;
  w0.channel = channel;
  w0.note = note_number;
  w0.velocity = velocity;

  EXPECT_CALL(config_.m1cvm, note_on(config_.context, message)).Times(1);

  // The first half of a 64-bit MIDI 2 note-on message.
  dispatcher_.processUMP(
      pack((to_underlying(midi2::ump_message_type::m2cvm) << 4) | group, (ump_note_on << 4) | channel, note_number, 0));
  dispatcher_.clearUMP();

  // An entire 32-bit MIDI 1 note-on message.
  dispatcher_.processUMP(pack((to_underlying(midi2::ump_message_type::m1cvm) << 4) | group,
                              (ump_note_on << 4) | channel, note_number, velocity));
}
//*  _   _ __  __ ___   ___ _                       *
//* | | | |  \/  | _ \ / __| |_ _ _ ___ __ _ _ __   *
//* | |_| | |\/| |  _/ \__ \  _| '_/ -_) _` | '  \  *
//*  \___/|_|  |_|_|   |___/\__|_| \___\__,_|_|_|_| *
//*                                                 *
class UMPDispatcherStream : public UMPDispatcher {};

// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, EndpointDiscovery) {
  midi2::types::ump_stream::endpoint_discovery message{};
  auto &w0 = get<0>(message.w);
  w0.format = 0x03;
  w0.version_major = 0x01;
  w0.version_minor = 0x01;
  get<1>(message.w).filter = 0b00011111;
  EXPECT_CALL(config_.ump_stream, endpoint_discovery(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, EndpointInfoNotification) {
  midi2::types::ump_stream::endpoint_info_notification message{};
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.format = 0x00;
  w0.version_major = 0x01;
  w0.version_minor = 0x01;
  w1.static_function_blocks = 1;
  w1.number_function_blocks = 0b0101010;
  w1.midi2_protocol_capability = 1;
  w1.midi1_protocol_capability = 0;
  w1.receive_jr_timestamp_capability = 1;
  w1.transmit_jr_timestamp_capability = 0;
  EXPECT_CALL(config_.ump_stream, endpoint_info_notification(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, DeviceIdentityNotification) {
  midi2::types::ump_stream::device_identity_notification message{};
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  auto &w2 = get<2>(message.w);
  auto &w3 = get<3>(message.w);
  w0.format = 0x00;
  w1.dev_manuf_sysex_id_1 = 1;
  w1.dev_manuf_sysex_id_2 = 1;
  w1.dev_manuf_sysex_id_3 = 0;
  w2.device_family_lsb = 0x79;
  w2.device_family_msb = 0x7B;
  w2.device_family_model_lsb = 0x7D;
  w2.device_family_model_msb = 0x7F;
  w3.sw_revision_1 = 0x7F;
  w3.sw_revision_2 = 0x7D;
  w3.sw_revision_3 = 0x7B;
  w3.sw_revision_4 = 0x79;
  EXPECT_CALL(config_.ump_stream, device_identity_notification(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1, w2, w3);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, EndpointNameNotification) {
  midi2::types::ump_stream::endpoint_name_notification message{};
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  auto &w2 = get<2>(message.w);
  auto &w3 = get<3>(message.w);
  w0.format = 0x00;
  w0.name1 = std::uint8_t{'a'};
  w0.name2 = std::uint8_t{'b'};
  w1.name3 = std::uint8_t{'c'};
  w1.name4 = std::uint8_t{'d'};
  w1.name5 = std::uint8_t{'e'};
  w1.name6 = std::uint8_t{'f'};
  w2.name7 = std::uint8_t{'g'};
  w2.name8 = std::uint8_t{'h'};
  w2.name9 = std::uint8_t{'i'};
  w2.name10 = std::uint8_t{'j'};
  w3.name11 = std::uint8_t{'k'};
  w3.name12 = std::uint8_t{'l'};
  w3.name13 = std::uint8_t{'m'};
  w3.name14 = std::uint8_t{'m'};
  EXPECT_CALL(config_.ump_stream, endpoint_name_notification(config_.context, message)).Times(1);
  dispatcher_.processUMP(w0, w1, w2, w3);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, ProductInstanceIdNotification) {
  midi2::types::ump_stream::product_instance_id_notification message{};
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  auto &w2 = get<2>(message.w);
  auto &w3 = get<3>(message.w);
  w0.format = 0x00;
  w0.pid1 = 0x22;
  w0.pid2 = 0x33;
  w1.pid3 = 0x44;
  w1.pid4 = 0x55;
  w1.pid5 = 0x66;
  w1.pid6 = 0x77;
  w2.pid7 = 0x88;
  w2.pid8 = 0x99;
  w2.pid9 = 0xAA;
  w2.pid10 = 0xBB;
  w3.pid11 = 0xCC;
  w3.pid12 = 0xDD;
  w3.pid13 = 0xEE;
  w3.pid14 = 0xFF;
  EXPECT_CALL(config_.ump_stream, product_instance_id_notification(config_.context, message)).Times(1);

  dispatcher_.processUMP(w0, w1, w2, w3);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, JRConfigurationRequest) {
  midi2::types::ump_stream::jr_configuration_request message{};
  auto &w0 = get<0>(message.w);
  w0.format = 0x00;
  w0.protocol = 0x02;
  w0.rxjr = 1;
  w0.txjr = 0;
  EXPECT_CALL(config_.ump_stream, jr_configuration_request(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, JRConfigurationNotification) {
  midi2::types::ump_stream::jr_configuration_notification message{};
  auto &w0 = get<0>(message.w);
  w0.format = 0x00;
  w0.protocol = 0x02;
  w0.rxjr = 1;
  w0.txjr = 0;
  EXPECT_CALL(config_.ump_stream, jr_configuration_notification(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, FunctionBlockDiscovery) {
  midi2::types::ump_stream::function_block_discovery message{};
  auto &w0 = get<0>(message.w);
  w0.format = 0x00;
  w0.block_num = 0xFF;
  w0.filter = 0x03;
  EXPECT_CALL(config_.ump_stream, function_block_discovery(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, FunctionBlockInfoNotification) {
  midi2::types::ump_stream::function_block_info_notification message{};
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.format = 0x00;
  w0.block_active = 1;
  w0.block_num = 0x1F;
  w0.ui_hint = 0b10;
  w0.midi1 = 0;
  w0.direction = 0b10;
  w1.first_group = 0b10101010;
  w1.num_spanned = 0x10;
  w1.ci_message_version = 0x1;
  w1.max_sys8_streams = 2;
  EXPECT_CALL(config_.ump_stream, function_block_info_notification(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, FunctionBlockNameNotification) {
  midi2::types::ump_stream::function_block_name_notification message{};
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  auto &w2 = get<2>(message.w);
  auto &w3 = get<3>(message.w);
  w0.format = 0x00;
  w0.block_num = 0x1F;
  w0.name0 = 'a';
  w1.name1 = 'b';
  w1.name2 = 'c';
  w1.name3 = 'd';
  w1.name4 = 'e';
  w2.name5 = 'f';
  w2.name6 = 'g';
  w2.name7 = 'h';
  w2.name8 = 'i';
  w3.name9 = 'k';
  w3.name10 = 'l';
  w3.name11 = 'm';
  w3.name12 = 'n';
  EXPECT_CALL(config_.ump_stream, function_block_name_notification(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, StartOfClip) {
  midi2::types::ump_stream::start_of_clip message{};
  auto &w0 = get<0>(message.w);
  w0.format = 0x00;
  EXPECT_CALL(config_.ump_stream, start_of_clip(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, EndOfClip) {
  midi2::types::ump_stream::end_of_clip message{};
  auto &w0 = get<0>(message.w);
  w0.format = 0x00;
  EXPECT_CALL(config_.ump_stream, end_of_clip(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}

//*  ___ _           ___       _         *
//* | __| |_____ __ |   \ __ _| |_ __ _  *
//* | _|| / -_) \ / | |) / _` |  _/ _` | *
//* |_| |_\___/_\_\ |___/\__,_|\__\__,_| *
//*                                      *
class UMPDispatcherFlexData : public UMPDispatcher {};

// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetTempo) {
  midi2::types::flex_data::set_tempo message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 0;
  w0.status_bank = 0;
  w1 = std::uint32_t{0xF0F0F0F0};
  EXPECT_CALL(config_.flex, set_tempo(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w));
  dispatcher_.processUMP(get<1>(message.w));
  dispatcher_.processUMP(get<2>(message.w));
  dispatcher_.processUMP(get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetTimeSignature) {
  midi2::types::flex_data::set_time_signature message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 3;
  w0.status_bank = 0;
  w1.numerator = 1;
  w1.denominator = 2;
  w1.number_of_32_notes = 16;
  EXPECT_CALL(config_.flex, set_time_signature(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetMetronome) {
  midi2::types::flex_data::set_metronome message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  auto &w2 = get<2>(message.w);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 3;
  w0.status_bank = 0;
  w1.num_clocks_per_primary_click = 24;
  w1.bar_accent_part_1 = 4;
  w1.bar_accent_part_2 = 0;
  w1.bar_accent_part_3 = 0;
  w2.num_subdivision_clicks_1 = 0;
  w2.num_subdivision_clicks_2 = 0;
  EXPECT_CALL(config_.flex, set_metronome(config_.context, message)).Times(1);

  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetKeySignature) {
  midi2::types::flex_data::set_key_signature message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 3;
  w0.status_bank = 0;
  w1.sharps_flats = 0b100;  // (-8)
  w1.tonic_note = static_cast<std::uint8_t>(midi2::types::flex_data::note::E);
  EXPECT_CALL(config_.flex, set_key_signature(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w), get<1>(message.w), get<2>(message.w), get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetChordName) {
  midi2::types::flex_data::set_chord_name message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  auto &w2 = get<2>(message.w);
  auto &w3 = get<3>(message.w);
  w0.group = 0x0F;
  w0.form = 0x0;
  w0.addrs = 3;
  w0.channel = 3;
  w0.status_bank = 0x00;
  w1.tonic_sharps_flats = 0x1;
  w1.chord_tonic = midi2::to_underlying(midi2::types::flex_data::note::E);
  w1.chord_type = midi2::to_underlying(midi2::types::flex_data::chord_type::augmented);
  w1.alter_1_type = 1;
  w1.alter_1_degree = 5;
  w1.alter_2_type = 2;
  w1.alter_2_degree = 6;
  w2.alter_3_type = 3;
  w2.alter_3_degree = 7;
  w2.alter_4_type = 4;
  w2.alter_4_degree = 8;
  w2.reserved = 0x0000;
  w3.bass_sharps_flats = 0xE;
  w3.bass_note = midi2::to_underlying(midi2::types::flex_data::note::unknown);
  w3.bass_chord_type = midi2::to_underlying(midi2::types::flex_data::chord_type::diminished);
  w3.alter_1_type = 1;
  w3.alter_1_degree = 3;
  w3.alter_2_type = 2;
  w3.alter_2_degree = 4;
  EXPECT_CALL(config_.flex, set_chord_name(config_.context, message)).Times(1);
  dispatcher_.processUMP(get<0>(message.w));
  dispatcher_.processUMP(get<1>(message.w));
  dispatcher_.processUMP(get<2>(message.w));
  dispatcher_.processUMP(get<3>(message.w));
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, Text) {
  midi2::types::flex_data::text_common message;
  auto &w0 = get<0>(message.w);
  auto &w1 = get<1>(message.w);
  auto &w2 = get<2>(message.w);
  auto &w3 = get<3>(message.w);
  w0.mt = to_underlying(midi2::ump_message_type::flex_data);
  w0.group = 0;
  w0.form = 0;
  w0.addrs = 1;
  w0.channel = 3;
  w0.status_bank = 1;
  w0.status = 4;
  w1 =
      (std::uint32_t{0xC2} << 24) | (std::uint32_t{0xA9} << 16) | (std::uint32_t{'2'} << 8) | (std::uint32_t{'0'} << 0);
  w2 = (std::uint32_t{'2'} << 24) | (std::uint32_t{'4'} << 16) | (std::uint32_t{' '} << 8) | (std::uint32_t{'P'} << 0);
  w3 =
      (std::uint32_t{'B'} << 24) | (std::uint32_t{'H'} << 16) | (std::uint32_t{'\0'} << 8) | (std::uint32_t{'\0'} << 0);
  EXPECT_CALL(config_.flex, text(config_.context, message)).Times(1);

  dispatcher_.processUMP(get<0>(message.w));
  dispatcher_.processUMP(get<1>(message.w));
  dispatcher_.processUMP(get<2>(message.w));
  dispatcher_.processUMP(get<3>(message.w));
}

void UMPDispatcherNeverCrashes(std::vector<std::uint32_t> const &in) {
  midi2::ump_dispatcher p;
  std::ranges::for_each(in, [&p](std::uint32_t ump) { p.processUMP(ump); });
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPDispatcherFuzz, UMPDispatcherNeverCrashes);
#endif
// NOLINTNEXTLINE
TEST(UMPDispatcherFuzz, Empty) {
  UMPDispatcherNeverCrashes({});
}

template <midi2::ump_message_type MessageType> void process_message(std::span<std::uint32_t> message) {
  if (message.size() == midi2::message_size<MessageType>::value) {
    message[0] = (message[0] & 0x00FFFFFF) | (static_cast<std::uint32_t>(MessageType) << 24);
    midi2::ump_dispatcher p;
    for (auto const w : message) {
      p.processUMP(w);
    }
  }
}

void utility(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::utility>({std::begin(message), std::end(message)});
}
void system(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::system>({std::begin(message), std::end(message)});
}
void m1cvm(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::m1cvm>({std::begin(message), std::end(message)});
}
void data64(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::data64>({std::begin(message), std::end(message)});
}
void m2cvm(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::m2cvm>({std::begin(message), std::end(message)});
}
void data128(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::data128>({std::begin(message), std::end(message)});
}
void flex_data(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::flex_data>({std::begin(message), std::end(message)});
}
void stream(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::ump_stream>({std::begin(message), std::end(message)});
}
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPDispatcherFuzz, utility);
// NOLINTNEXTLINE
FUZZ_TEST(UMPDispatcherFuzz, system);
// NOLINTNEXTLINE
FUZZ_TEST(UMPDispatcherFuzz, m1cvm);
// NOLINTNEXTLINE
FUZZ_TEST(UMPDispatcherFuzz, data64);
// NOLINTNEXTLINE
FUZZ_TEST(UMPDispatcherFuzz, m2cvm);
// NOLINTNEXTLINE
FUZZ_TEST(UMPDispatcherFuzz, data128);
// NOLINTNEXTLINE
FUZZ_TEST(UMPDispatcherFuzz, flex_data);
// NOLINTNEXTLINE
FUZZ_TEST(UMPDispatcherFuzz, stream);
#endif

// NOLINTNEXTLINE
TEST(UMPDispatcherFuzz, UtilityMessage) {
  utility({});
}
// NOLINTNEXTLINE
TEST(UMPDispatcherFuzz, SystemMessage) {
  system({});
}
// NOLINTNEXTLINE
TEST(UMPDispatcherFuzz, M1CVMMessage) {
  m1cvm({});
}
// NOLINTNEXTLINE
TEST(UMPDispatcherFuzz, Data64Message) {
  data64({});
}
// NOLINTNEXTLINE
TEST(UMPDispatcherFuzz, M2CVMMessage) {
  m2cvm({});
}
// NOLINTNEXTLINE
TEST(UMPDispatcherFuzz, Data128Message) {
  data128({});
}
// NOLINTNEXTLINE
TEST(UMPDispatcherFuzz, FlexDataMessage) {
  flex_data({});
}
// NOLINTNEXTLINE
TEST(UMPDispatcherFuzz, UMPStreamMessage) {
  stream({});
}

}  // end anonymous namespace
