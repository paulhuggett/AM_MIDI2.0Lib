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
#include <system_error>

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace {

using testing::ElementsAre;
using testing::InSequence;
using testing::StrictMock;

TEST(UMPApply, AlwaysTrue) {
  std::vector<std::uint32_t> values;
  midi2::types::apply(std::tuple{1, 2}, [&values](int const v) {
    values.push_back(v);
    return false;
  });
  EXPECT_THAT(values, ElementsAre(1, 2));
}

TEST(UMPApply, ErrorCodeAlwaysSuccess) {
  std::vector<std::uint32_t> values;
  midi2::types::apply(std::tuple{1, 2}, [&values](int const v) -> std::error_code {
    values.push_back(v);
    return std::error_code{};
  });
  EXPECT_THAT(values, ElementsAre(1, 2));
}

TEST(UMPApply, ErrorCodeFails) {
  std::vector<std::uint32_t> values;
  auto const result = midi2::types::apply(std::tuple{1, 2}, [&values](auto const v) {
    values.push_back(v);
    return std::make_error_code(std::errc::io_error);
  });
  EXPECT_EQ(result, std::make_error_code(std::errc::io_error));
  EXPECT_THAT(values, ElementsAre(1))
      << "Expected a single element because the lambda returned an error to stop processing";
}

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

class UMPDispatcher : public testing::Test {
public:
  UMPDispatcher() : dispatcher_{std::ref(config_)} {}

  void apply(auto const &message) {
    midi2::types::apply(message, [this](auto const v) {
      dispatcher_.processUMP(std::uint32_t{v});
      return false;
    });
  }

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
  this->apply(midi2::types::utility::noop{});
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, JRClock) {
  constexpr auto message = midi2::types::utility::jr_clock{}.sender_clock_time(0b1010101010101010);
  EXPECT_CALL(config_.utility, jr_clock(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, JRTimestamp) {
  midi2::types::utility::jr_timestamp message{};
  message.timestamp((1U << 16) - 1U);
  EXPECT_CALL(config_.utility, jr_timestamp(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, DeltaClockstampTqpn) {
  midi2::types::utility::delta_clockstamp_tpqn message;
  message.ticks_pqn(0b1010101010101010);
  EXPECT_CALL(config_.utility, delta_clockstamp_tpqn(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, DeltaClockstamp) {
  midi2::types::utility::delta_clockstamp message{};
  message.ticks_per_quarter_note((1U << decltype(message)::word0::ticks_per_quarter_note::bits()) - 1U);
  EXPECT_CALL(config_.utility, delta_clockstamp(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherUtility, BadMessage) {
  constexpr std::uint32_t message =
      (to_underlying(midi2::ump_message_type::utility) << 28) | (std::uint32_t{0xF} << 20);
  EXPECT_CALL(config_.utility, unknown(config_.context, ElementsAre(message)));
  dispatcher_.processUMP(message);
}

//*  ___         _              *
//* / __|_  _ __| |_ ___ _ __   *
//* \__ \ || (_-<  _/ -_) '  \  *
//* |___/\_, /__/\__\___|_|_|_| *
//*      |__/                   *
class UMPDispatcherSystem : public UMPDispatcher {};
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, MIDITimeCode) {
  constexpr auto message = midi2::types::system::midi_time_code{}.group(0).time_code(0b1010101);
  EXPECT_CALL(config_.system, midi_time_code(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, SongPositionPointer) {
  constexpr auto message =
      midi2::types::system::song_position_pointer{}.group(0).position_lsb(0b1010101).position_msb(0b1111111);
  EXPECT_CALL(config_.system, song_position_pointer(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, SongSelect) {
  constexpr auto message = midi2::types::system::song_select{}.group(3).song(0b1010101);
  EXPECT_CALL(config_.system, song_select(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, TuneRequest) {
  constexpr auto message = midi2::types::system::tune_request{}.group(1);
  EXPECT_CALL(config_.system, tune_request(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, TimingClock) {
  midi2::types::system::timing_clock message;
  message.group(0);
  EXPECT_CALL(config_.system, timing_clock(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, Start) {
  midi2::types::system::sequence_start message;
  message.group(0);
  EXPECT_CALL(config_.system, seq_start(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, Continue) {
  midi2::types::system::sequence_continue message;
  message.group(0);
  EXPECT_CALL(config_.system, seq_continue(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, Stop) {
  midi2::types::system::sequence_stop message;
  message.group(0);
  EXPECT_CALL(config_.system, seq_stop(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, ActiveSensing) {
  midi2::types::system::active_sensing message;
  message.group(0);
  EXPECT_CALL(config_.system, active_sensing(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, Reset) {
  midi2::types::system::reset message;
  message.group(0);
  EXPECT_CALL(config_.system, reset(config_.context, message));
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherSystem, BadStatus) {
  constexpr std::uint32_t message = (to_underlying(midi2::ump_message_type::system) << 28) | (std::uint32_t{0xF} << 20);
  EXPECT_CALL(config_.utility, unknown(config_.context, ElementsAre(message)));
  dispatcher_.processUMP(message);
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
  constexpr auto message = midi2::types::m1cvm::note_on{}.group(0).channel(3).note(60).velocity(0x43);
  EXPECT_CALL(config_.m1cvm, note_on(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI1, NoteOff) {
  constexpr auto message = midi2::types::m1cvm::note_off{}.group(0).channel(3).note(60).velocity(0x43);
  EXPECT_CALL(config_.m1cvm, note_off(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI1, PolyPressure) {
  constexpr auto message = midi2::types::m1cvm::poly_pressure{}.group(0).channel(3).note(60).pressure(0x43);
  EXPECT_CALL(config_.m1cvm, poly_pressure(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI1, ControlChange) {
  constexpr auto message = midi2::types::m1cvm::control_change{}.group(0).channel(3).controller(60).value(127);
  EXPECT_CALL(config_.m1cvm, control_change(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI1, ChannelPressure) {
  constexpr auto message = midi2::types::m1cvm::channel_pressure{}.group(0).channel(3).data(0b01010101);
  EXPECT_CALL(config_.m1cvm, channel_pressure(config_.context, message)).Times(1);
  this->apply(message);
}

//*     _      _           __ _ _   *
//*  __| |__ _| |_ __ _   / /| | |  *
//* / _` / _` |  _/ _` | / _ \_  _| *
//* \__,_\__,_|\__\__,_| \___/ |_|  *
//*                                 *
// NOLINTNEXTLINE
TEST_F(UMPDispatcher, Data64SysExIn1) {
  constexpr auto message =
      midi2::types::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(2).data1(3).data2(5).data3(7);
  EXPECT_CALL(config_.data64, sysex7_in_1(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcher, Data64Sysex8StartAndEnd) {
  constexpr auto group = std::uint8_t{1};
  constexpr auto m0 = midi2::types::data64::sysex7_start{}
                          .group(group)
                          .number_of_bytes(6)
                          .data0(2)
                          .data1(3)
                          .data2(5)
                          .data3(7)
                          .data4(11)
                          .data5(13);
  constexpr auto m1 = midi2::types::data64::sysex7_continue{}
                          .group(group)
                          .number_of_bytes(6)
                          .data0(17)
                          .data1(19)
                          .data2(23)
                          .data3(29)
                          .data4(31)
                          .data5(37);
  constexpr auto m2 =
      midi2::types::data64::sysex7_end{}.group(group).number_of_bytes(4).data0(41).data1(43).data2(47).data3(53);
  {
    InSequence _;
    EXPECT_CALL(config_.data64, sysex7_start(config_.context, m0)).Times(1);
    EXPECT_CALL(config_.data64, sysex7_continue(config_.context, m1)).Times(1);
    EXPECT_CALL(config_.data64, sysex7_end(config_.context, m2)).Times(1);
  }
  this->apply(m0);
  this->apply(m1);
  this->apply(m2);
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
  message.group(0).channel(3).note(60).attribute(0).velocity(0x432).attribute(0);
  EXPECT_CALL(config_.m2cvm, note_on(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, NoteOff) {
  midi2::types::m2cvm::note_off message;
  message.group(0).channel(3).note(60).attribute_type(0).velocity(0x432).attribute(0);
  EXPECT_CALL(config_.m2cvm, note_off(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, ProgramChange) {
  constexpr auto message = midi2::types::m2cvm::program_change{}
                               .group(0)
                               .channel(3)
                               .option_flags(0)
                               .bank_valid(true)
                               .program(0b10101010)
                               .bank_msb(0b01010101)
                               .bank_lsb(0b00101010);
  EXPECT_CALL(config_.m2cvm, program_change(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, ControlChange) {
  constexpr auto message = midi2::types::m2cvm::control_change{}.group(0).channel(3).controller(2).value(0xF0F0E1E1);
  EXPECT_CALL(config_.m2cvm, control_change(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, ChannelPressure) {
  constexpr auto message = midi2::types::m2cvm::channel_pressure{}.group(0).channel(3).value(0xF0F0E1E1);
  EXPECT_CALL(config_.m2cvm, channel_pressure(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, RPNPerNoteController) {
  constexpr auto message =
      midi2::types::m2cvm::rpn_per_note_controller{}.group(0).channel(3).note(60).index(1).value(0xF0F0E1E1);
  EXPECT_CALL(config_.m2cvm, rpn_per_note_controller(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, NRPNPerNoteController) {
  constexpr auto message =
      midi2::types::m2cvm::nrpn_per_note_controller{}.group(0).channel(3).note(60).index(1).value(0xF0F0E1E1);
  EXPECT_CALL(config_.m2cvm, nrpn_per_note_controller(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, RPNController) {
  constexpr auto message =
      midi2::types::m2cvm::rpn_controller{}.group(0).channel(3).bank(23).index(31).value(0xF0F0E1E1);
  EXPECT_CALL(config_.m2cvm, rpn_controller(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherMIDI2CVM, NRPNController) {
  constexpr auto message =
      midi2::types::m2cvm::nrpn_controller{}.group(0).channel(3).bank(23).index(31).value(0xF0F0E1E1);
  EXPECT_CALL(config_.m2cvm, nrpn_controller(config_.context, message)).Times(1);
  this->apply(message);
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

  constexpr auto message = midi2::types::data128::sysex8_in_1{}
                               .group(group)
                               .number_of_bytes(10)
                               .stream_id(stream_id)
                               .data0(2)
                               .data1(3)
                               .data2(5)
                               .data3(7)
                               .data4(11)
                               .data5(13)
                               .data6(17)
                               .data7(19)
                               .data8(23)
                               .data9(29);
  EXPECT_CALL(config_.data128, sysex8_in_1(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherData128, Sysex8StartAndEnd) {
  constexpr auto group = std::uint8_t{0};
  constexpr auto stream_id = std::uint8_t{0};

  constexpr auto part0 = midi2::types::data128::sysex8_start{}
                             .group(group)
                             .number_of_bytes(13)
                             .stream_id(stream_id)
                             .data0(2)
                             .data1(3)
                             .data2(5)
                             .data3(7)
                             .data4(11)
                             .data5(13)
                             .data6(17)
                             .data7(19)
                             .data8(23)
                             .data9(29)
                             .data10(31)
                             .data11(37)
                             .data12(41);
  constexpr auto part1 = midi2::types::data128::sysex8_continue{}
                             .group(group)
                             .number_of_bytes(13)
                             .stream_id(stream_id)
                             .data0(43)
                             .data1(47)
                             .data2(53)
                             .data3(59)
                             .data4(61)
                             .data5(67)
                             .data6(71)
                             .data7(73)
                             .data8(79)
                             .data9(83)
                             .data10(89)
                             .data11(97)
                             .data12(101);
  constexpr auto part2 = midi2::types::data128::sysex8_end{}
                             .group(group)
                             .number_of_bytes(4)
                             .stream_id(stream_id)
                             .data0(103)
                             .data1(107)
                             .data2(109)
                             .data2(113);

  {
    InSequence _;
    EXPECT_CALL(config_.data128, sysex8_start(config_.context, part0)).Times(1);
    EXPECT_CALL(config_.data128, sysex8_continue(config_.context, part1)).Times(1);
    EXPECT_CALL(config_.data128, sysex8_end(config_.context, part2)).Times(1);
  }
  this->apply(part0);
  this->apply(part1);
  this->apply(part2);
}

// NOLINTNEXTLINE
TEST_F(UMPDispatcherData128, MixedDatSet) {
  constexpr auto group = std::uint8_t{0};
  constexpr auto mds_id = std::uint8_t{0b1010};

  constexpr auto header = midi2::types::data128::mds_header{}
                              .group(group)
                              .mds_id(mds_id)
                              .bytes_in_chunk(2)
                              .chunks_in_mds(1)
                              .chunk_num(1)
                              .manufacturer_id(43)
                              .device_id(61)
                              .sub_id_1(19)
                              .sub_id_2(23);

  constexpr auto payload = midi2::types::data128::mds_payload{}
                               .group(group)
                               .mds_id(mds_id)
                               .value0(std::uint16_t{0xFFFF})
                               .value1(0xFFFFFFFF)
                               .value2(0xFFFFFFFF)
                               .value3(0xFFFFFFFF);
  {
    InSequence _;
    EXPECT_CALL(config_.data128, mds_header(config_.context, header)).Times(1);
    EXPECT_CALL(config_.data128, mds_payload(config_.context, payload)).Times(1);
  }
  this->apply(header);
  this->apply(payload);
}

// NOLINTNEXTLINE
constexpr auto ump_note_on = ump_cvm(midi2::status::note_on);
TEST_F(UMPDispatcher, PartialMessageThenClear) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint16_t{0x43};  // 7 bits
  constexpr auto group = std::uint8_t{0};

  midi2::types::m1cvm::note_on message;
  message.group(group).channel(channel).note(note_number).velocity(velocity);

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
  constexpr auto message =
      midi2::types::ump_stream::endpoint_discovery{}.format(0x03).version_major(0x01).version_minor(0x01).filter(
          0b00011111);
  EXPECT_CALL(config_.ump_stream, endpoint_discovery(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, EndpointInfoNotification) {
  constexpr auto message = midi2::types::ump_stream::endpoint_info_notification{}
                               .format(0x00)
                               .version_major(0x01)
                               .version_minor(0x01)
                               .static_function_blocks(1)
                               .number_function_blocks(0b0101010)
                               .midi2_protocol_capability(1)
                               .midi1_protocol_capability(0)
                               .receive_jr_timestamp_capability(1)
                               .transmit_jr_timestamp_capability(0);
  EXPECT_CALL(config_.ump_stream, endpoint_info_notification(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, DeviceIdentityNotification) {
  constexpr auto message = midi2::types::ump_stream::device_identity_notification{}
                               .format(0x00)
                               .dev_manuf_sysex_id_1(1)
                               .dev_manuf_sysex_id_2(1)
                               .dev_manuf_sysex_id_3(0)
                               .device_family_lsb(0x79)
                               .device_family_msb(0x7B)
                               .device_family_model_lsb(0x7D)
                               .device_family_model_msb(0x7F)
                               .sw_revision_1(0x7F)
                               .sw_revision_2(0x7D)
                               .sw_revision_3(0x7B)
                               .sw_revision_4(0x79);
  EXPECT_CALL(config_.ump_stream, device_identity_notification(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, EndpointNameNotification) {
  constexpr auto message = midi2::types::ump_stream::endpoint_name_notification{}
                               .format(0x00)
                               .name1(std::uint8_t{'a'})
                               .name2(std::uint8_t{'b'})
                               .name3(std::uint8_t{'c'})
                               .name4(std::uint8_t{'d'})
                               .name5(std::uint8_t{'e'})
                               .name6(std::uint8_t{'f'})
                               .name7(std::uint8_t{'g'})
                               .name8(std::uint8_t{'h'})
                               .name9(std::uint8_t{'i'})
                               .name10(std::uint8_t{'j'})
                               .name11(std::uint8_t{'k'})
                               .name12(std::uint8_t{'l'})
                               .name13(std::uint8_t{'m'})
                               .name14(std::uint8_t{'m'});
  EXPECT_CALL(config_.ump_stream, endpoint_name_notification(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, ProductInstanceIdNotification) {
  constexpr auto message = midi2::types::ump_stream::product_instance_id_notification{}
                               .format(0x00)
                               .pid1(0x22)
                               .pid2(0x33)
                               .pid3(0x44)
                               .pid4(0x55)
                               .pid5(0x66)
                               .pid6(0x77)
                               .pid7(0x88)
                               .pid8(0x99)
                               .pid9(0xAA)
                               .pid10(0xBB)
                               .pid11(0xCC)
                               .pid12(0xDD)
                               .pid13(0xEE)
                               .pid14(0xFF);
  EXPECT_CALL(config_.ump_stream, product_instance_id_notification(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, JRConfigurationRequest) {
  constexpr auto message =
      midi2::types::ump_stream::jr_configuration_request{}.format(0x00).protocol(0x02).rxjr(1).txjr(0);
  EXPECT_CALL(config_.ump_stream, jr_configuration_request(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, JRConfigurationNotification) {
  constexpr auto message =
      midi2::types::ump_stream::jr_configuration_notification{}.format(0x00).protocol(0x02).rxjr(1).txjr(0);
  EXPECT_CALL(config_.ump_stream, jr_configuration_notification(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, FunctionBlockDiscovery) {
  constexpr auto message =
      midi2::types::ump_stream::function_block_discovery{}.format(0x00).block_num(0xFF).filter(0x03);
  EXPECT_CALL(config_.ump_stream, function_block_discovery(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, FunctionBlockInfoNotification) {
  constexpr auto message = midi2::types::ump_stream::function_block_info_notification{}
                               .format(0x00)
                               .block_active(1)
                               .block_num(0x1F)
                               .ui_hint(0b10)
                               .midi1(0)
                               .direction(0b10)
                               .first_group(0b10101010)
                               .num_spanned(0x10)
                               .ci_message_version(0x1)
                               .max_sys8_streams(2);
  EXPECT_CALL(config_.ump_stream, function_block_info_notification(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, FunctionBlockNameNotification) {
  constexpr auto message = midi2::types::ump_stream::function_block_name_notification{}
                               .format(0x00)
                               .block_num(0x1F)
                               .name0('a')
                               .name1('b')
                               .name2('c')
                               .name3('d')
                               .name4('e')
                               .name5('f')
                               .name6('g')
                               .name7('h')
                               .name8('i')
                               .name9('k')
                               .name10('l')
                               .name11('m')
                               .name12('n');
  EXPECT_CALL(config_.ump_stream, function_block_name_notification(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, StartOfClip) {
  constexpr auto message = midi2::types::ump_stream::start_of_clip{}.format(0x00);
  EXPECT_CALL(config_.ump_stream, start_of_clip(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherStream, EndOfClip) {
  constexpr auto message = midi2::types::ump_stream::end_of_clip{}.format(0x00);
  EXPECT_CALL(config_.ump_stream, end_of_clip(config_.context, message)).Times(1);
  this->apply(message);
}

//*  ___ _           ___       _         *
//* | __| |_____ __ |   \ __ _| |_ __ _  *
//* | _|| / -_) \ / | |) / _` |  _/ _` | *
//* |_| |_\___/_\_\ |___/\__,_|\__\__,_| *
//*                                      *
class UMPDispatcherFlexData : public UMPDispatcher {};

// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetTempo) {
  constexpr auto message =
      midi2::types::flex_data::set_tempo{}.group(0).form(0).addrs(1).channel(0).status_bank(0).value1(0xF0F0F0F0);
  EXPECT_CALL(config_.flex, set_tempo(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetTimeSignature) {
  constexpr auto message = midi2::types::flex_data::set_time_signature{}
                               .group(0)
                               .form(0)
                               .addrs(1)
                               .channel(3)
                               .status_bank(0)
                               .numerator(1)
                               .denominator(2)
                               .number_of_32_notes(16);
  EXPECT_CALL(config_.flex, set_time_signature(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetMetronome) {
  constexpr auto message = midi2::types::flex_data::set_metronome{}
                               .group(0)
                               .form(0)
                               .addrs(1)
                               .channel(3)
                               .status_bank(0)
                               .num_clocks_per_primary_click(24)
                               .bar_accent_part_1(4)
                               .bar_accent_part_2(0)
                               .bar_accent_part_3(0)
                               .num_subdivision_clicks_1(0)
                               .num_subdivision_clicks_2(0);
  EXPECT_CALL(config_.flex, set_metronome(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetKeySignature) {
  constexpr auto message = midi2::types::flex_data::set_key_signature{}
                               .group(0)
                               .form(0)
                               .addrs(1)
                               .channel(3)
                               .status_bank(0)
                               .sharps_flats(0b100)  // (-8)
                               .tonic_note(midi2::to_underlying(midi2::types::flex_data::note::E));
  EXPECT_CALL(config_.flex, set_key_signature(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, SetChordName) {
  constexpr auto message = midi2::types::flex_data::set_chord_name{}
                               .group(0x0F)
                               .form(0x0)
                               .addrs(3)
                               .channel(3)
                               .status_bank(0x00)
                               .tonic_sharps_flats(0x1)
                               .chord_tonic(midi2::to_underlying(midi2::types::flex_data::note::E))
                               .chord_type(midi2::to_underlying(midi2::types::flex_data::chord_type::augmented))
                               .alter_1_type(1)
                               .alter_1_degree(5)
                               .alter_2_type(2)
                               .alter_2_degree(6)
                               .alter_3_type(3)
                               .alter_3_degree(7)
                               .alter_4_type(4)
                               .alter_4_degree(8)
                               .bass_sharps_flats(0xE)
                               .bass_note(midi2::to_underlying(midi2::types::flex_data::note::unknown))
                               .bass_chord_type(midi2::to_underlying(midi2::types::flex_data::chord_type::diminished))
                               .bass_alter_1_type(1)
                               .bass_alter_1_degree(3)
                               .bass_alter_2_type(2)
                               .bass_alter_2_degree(4);
  EXPECT_CALL(config_.flex, set_chord_name(config_.context, message)).Times(1);
  this->apply(message);
}
// NOLINTNEXTLINE
TEST_F(UMPDispatcherFlexData, Text) {
  constexpr auto message = midi2::types::flex_data::text_common{}
                               .group(0)
                               .form(0)
                               .addrs(1)
                               .channel(3)
                               .status_bank(1)
                               .status(4)
                               .value1((std::uint32_t{0xC2} << 24) | (std::uint32_t{0xA9} << 16) |
                                       (std::uint32_t{'2'} << 8) | (std::uint32_t{'0'} << 0))
                               .value2((std::uint32_t{'2'} << 24) | (std::uint32_t{'4'} << 16) |
                                       (std::uint32_t{' '} << 8) | (std::uint32_t{'P'} << 0))
                               .value3((std::uint32_t{'B'} << 24) | (std::uint32_t{'H'} << 16) |
                                       (std::uint32_t{'\0'} << 8) | (std::uint32_t{'\0'} << 0));
  EXPECT_CALL(config_.flex, text(config_.context, message)).Times(1);
  this->apply(message);
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
