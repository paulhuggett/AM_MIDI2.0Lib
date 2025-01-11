//===-- UMP Dispatcher Backends -----------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//
#include "midi2/dispatcher_backend.hpp"

// Standard library
#include <array>
#include <span>

// Google Test/Mock
#include <gmock/gmock.h>

namespace {

struct context_type {
  constexpr bool operator==(context_type const &) const noexcept = default;
  int value = 23;
};

using testing::ElementsAreArray;
using testing::InSequence;
using testing::MockFunction;
using testing::StrictMock;

[[nodiscard]] consteval std::uint32_t operator""_u32(unsigned long long v) {
  assert(v <= std::numeric_limits<std::uint32_t>::max());
  return static_cast<std::uint32_t>(v);
}

class DispatcherBackendUtility : public testing::Test {
protected:
  context_type context_;
  midi2::ump::dispatcher_backend::utility_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(DispatcherBackendUtility, Noop) {
  StrictMock<MockFunction<decltype(be_)::noop_fn>> fn;
  // The first call should do nothing since no handler has been installed.
  be_.noop(context_);
  // Install a handler for the noop message.
  be_.on_noop(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  EXPECT_CALL(fn, Call(context_)).Times(1);
  be_.noop(context_);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendUtility, JrClock) {
  StrictMock<MockFunction<decltype(be_)::jr_clock_fn>> fn;
  midi2::ump::utility::jr_clock clock;
  be_.jr_clock(context_, clock);
  be_.on_jr_clock(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, clock)).Times(1);
  be_.jr_clock(context_, clock);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendUtility, JrTimestamp) {
  StrictMock<MockFunction<decltype(be_)::jr_timestamp_fn>> fn;
  midi2::ump::utility::jr_timestamp time_stamp;
  be_.jr_timestamp(context_, time_stamp);
  be_.on_jr_timestamp(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, time_stamp)).Times(1);
  be_.jr_timestamp(context_, time_stamp);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendUtility, DeltaClockstampTPQN) {
  StrictMock<MockFunction<decltype(be_)::delta_clockstamp_tpqn_fn>> fn;
  midi2::ump::utility::delta_clockstamp_tpqn delta_clockstamp;
  be_.delta_clockstamp_tpqn(context_, delta_clockstamp);
  be_.on_delta_clockstamp_tpqn(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, delta_clockstamp)).Times(1);
  be_.delta_clockstamp_tpqn(context_, delta_clockstamp);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendUtility, DeltaClockstamp) {
  StrictMock<MockFunction<decltype(be_)::delta_clockstamp_fn>> fn;
  midi2::ump::utility::delta_clockstamp delta_clockstamp;
  be_.delta_clockstamp(context_, delta_clockstamp);
  be_.on_delta_clockstamp(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, delta_clockstamp)).Times(1);
  be_.delta_clockstamp(context_, delta_clockstamp);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendUtility, Unknown) {
  std::array<std::uint32_t, 5> message{0xFFFFFFFF_u32, 0xFFFFFFFE_u32, 0xFFFFFFFD_u32, 0xFFFFFFFC_u32, 0xFFFFFFFB_u32};
  be_.unknown(context_, std::span<std::uint32_t>{message});

  StrictMock<MockFunction<decltype(be_)::unknown_fn>> fn;
  EXPECT_CALL(fn, Call(context_, ElementsAreArray(message))).Times(1);
  be_.on_unknown(fn.AsStdFunction());
  be_.unknown(context_, std::span{message});
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendUtility, Chained) {
  StrictMock<MockFunction<decltype(be_)::noop_fn>> noop;
  StrictMock<MockFunction<decltype(be_)::jr_clock_fn>> jrc;
  InSequence _;
  midi2::ump::utility::jr_clock clock;
  EXPECT_CALL(noop, Call(context_)).Times(1);
  EXPECT_CALL(jrc, Call(context_, clock)).Times(1);
  // Chained calls to the functions setting the message handlers.
  be_.on_noop(noop.AsStdFunction()).on_jr_clock(jrc.AsStdFunction());

  be_.noop(context_);
  be_.jr_clock(context_, clock);
}

class DispatcherBackendSystem : public testing::Test {
protected:
  context_type context_;
  midi2::ump::dispatcher_backend::system_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, MidiTimeCode) {
  midi2::ump::system::midi_time_code mtc;
  // The first call should do nothing since no handler has been installed.
  be_.midi_time_code(context_, mtc);

  // Expect that our handler is called with the correct arguments.
  StrictMock<MockFunction<decltype(be_)::midi_time_code_fn>> fn;
  EXPECT_CALL(fn, Call(context_, mtc)).Times(1);
  // Install a handler for the noop message.
  be_.on_midi_time_code(fn.AsStdFunction());
  be_.midi_time_code(context_, mtc);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, SongPositionPointer) {
  midi2::ump::system::song_position_pointer spp;
  be_.song_position_pointer(context_, spp);

  StrictMock<MockFunction<decltype(be_)::song_position_pointer_fn>> fn;
  EXPECT_CALL(fn, Call(context_, spp)).Times(1);
  be_.on_song_position_pointer(fn.AsStdFunction());
  be_.song_position_pointer(context_, spp);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, SongSelect) {
  midi2::ump::system::song_select ss;
  be_.song_select(context_, ss);

  StrictMock<MockFunction<decltype(be_)::song_select_fn>> fn;
  EXPECT_CALL(fn, Call(context_, ss)).Times(1);
  be_.on_song_select(fn.AsStdFunction());
  be_.song_select(context_, ss);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, TuneRequest) {
  midi2::ump::system::tune_request tr;
  be_.tune_request(context_, tr);

  StrictMock<MockFunction<decltype(be_)::tune_request_fn>> fn;
  EXPECT_CALL(fn, Call(context_, tr)).Times(1);
  be_.on_tune_request(fn.AsStdFunction());
  be_.tune_request(context_, tr);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, TimingClock) {
  midi2::ump::system::timing_clock clock;
  be_.timing_clock(context_, clock);

  StrictMock<MockFunction<decltype(be_)::timing_clock_fn>> fn;
  EXPECT_CALL(fn, Call(context_, clock)).Times(1);
  be_.on_timing_clock(fn.AsStdFunction());
  be_.timing_clock(context_, clock);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, SequenceStart) {
  midi2::ump::system::sequence_start ss;
  be_.seq_start(context_, ss);

  StrictMock<MockFunction<decltype(be_)::seq_start_fn>> fn;
  EXPECT_CALL(fn, Call(context_, ss)).Times(1);
  be_.on_seq_start(fn.AsStdFunction());
  be_.seq_start(context_, ss);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, SequenceContinue) {
  midi2::ump::system::sequence_continue sc;
  be_.seq_continue(context_, sc);

  StrictMock<MockFunction<decltype(be_)::seq_continue_fn>> fn;
  EXPECT_CALL(fn, Call(context_, sc)).Times(1);
  be_.on_seq_continue(fn.AsStdFunction());
  be_.seq_continue(context_, sc);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, SequenceStop) {
  StrictMock<MockFunction<decltype(be_)::seq_stop_fn>> fn;
  midi2::ump::system::sequence_stop ss;
  be_.seq_stop(context_, ss);
  be_.on_seq_stop(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, ss)).Times(1);
  be_.seq_stop(context_, ss);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, ActiveSensing) {
  StrictMock<MockFunction<decltype(be_)::active_sensing_fn>> fn;
  midi2::ump::system::active_sensing as;
  be_.active_sensing(context_, as);
  be_.on_active_sensing(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, as)).Times(1);
  be_.active_sensing(context_, as);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendSystem, Reset) {
  StrictMock<MockFunction<decltype(be_)::reset_fn>> fn;
  midi2::ump::system::reset r;
  be_.reset(context_, r);
  be_.on_reset(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, r)).Times(1);
  be_.reset(context_, r);
}

class DispatcherBackendM1CVM : public testing::Test {
protected:
  context_type context_;
  midi2::ump::dispatcher_backend::m1cvm_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM1CVM, NoteOff) {
  StrictMock<MockFunction<decltype(be_)::note_off_fn>> fn;
  midi2::ump::m1cvm::note_off noff;
  // The first call should do nothing since no handler has been installed.
  be_.note_off(context_, noff);
  // Install a handler for the noop message.
  be_.on_note_off(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  EXPECT_CALL(fn, Call(context_, noff)).Times(1);
  be_.note_off(context_, noff);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM1CVM, NoteOn) {
  StrictMock<MockFunction<decltype(be_)::note_on_fn>> fn;
  midi2::ump::m1cvm::note_on non;
  be_.note_on(context_, non);
  be_.on_note_on(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, non)).Times(1);
  be_.note_on(context_, non);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM1CVM, PolyPressure) {
  StrictMock<MockFunction<decltype(be_)::poly_pressure_fn>> fn;
  midi2::ump::m1cvm::poly_pressure pressure;
  be_.poly_pressure(context_, pressure);
  be_.on_poly_pressure(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, pressure)).Times(1);
  be_.poly_pressure(context_, pressure);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM1CVM, ControlChange) {
  StrictMock<MockFunction<decltype(be_)::control_change_fn>> fn;
  midi2::ump::m1cvm::control_change cc;
  be_.control_change(context_, cc);
  be_.on_control_change(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, cc)).Times(1);
  be_.control_change(context_, cc);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM1CVM, ProgramChange) {
  StrictMock<MockFunction<decltype(be_)::program_change_fn>> fn;
  midi2::ump::m1cvm::program_change pc;
  be_.program_change(context_, pc);
  be_.on_program_change(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, pc)).Times(1);
  be_.program_change(context_, pc);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM1CVM, ChannelPressure) {
  StrictMock<MockFunction<decltype(be_)::channel_pressure_fn>> fn;
  midi2::ump::m1cvm::channel_pressure pressure;
  be_.channel_pressure(context_, pressure);
  be_.on_channel_pressure(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, pressure)).Times(1);
  be_.channel_pressure(context_, pressure);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM1CVM, PitchBend) {
  StrictMock<MockFunction<decltype(be_)::pitch_bend_fn>> fn;
  midi2::ump::m1cvm::pitch_bend pb;
  be_.pitch_bend(context_, pb);
  be_.on_pitch_bend(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, pb)).Times(1);
  be_.pitch_bend(context_, pb);
}

class DispatcherBackendData64 : public testing::Test {
protected:
  context_type context_;
  midi2::ump::dispatcher_backend::data64_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData64, Sysex7In1) {
  StrictMock<MockFunction<decltype(be_)::sysex7_in_1_fn>> fn;
  midi2::ump::data64::sysex7_in_1 sx;
  // The first call should do nothing since no handler has been installed.
  be_.sysex7_in_1(context_, sx);
  // Install a handler for the noop message.
  be_.on_sysex7_in_1(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  EXPECT_CALL(fn, Call(context_, sx)).Times(1);
  be_.sysex7_in_1(context_, sx);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData64, Sysex7Start) {
  StrictMock<MockFunction<decltype(be_)::sysex7_start_fn>> fn;
  midi2::ump::data64::sysex7_start sx;
  be_.sysex7_start(context_, sx);
  be_.on_sysex7_start(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, sx)).Times(1);
  be_.sysex7_start(context_, sx);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData64, Sysex7Continue) {
  StrictMock<MockFunction<decltype(be_)::sysex7_continue_fn>> fn;
  midi2::ump::data64::sysex7_continue sx;
  be_.sysex7_continue(context_, sx);
  be_.on_sysex7_continue(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, sx)).Times(1);
  be_.sysex7_continue(context_, sx);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData64, Sysex7End) {
  StrictMock<MockFunction<decltype(be_)::sysex7_end_fn>> fn;
  midi2::ump::data64::sysex7_end sx;
  be_.sysex7_end(context_, sx);
  be_.on_sysex7_end(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, sx)).Times(1);
  be_.sysex7_end(context_, sx);
}

class DispatcherBackendM2CVM : public testing::Test {
protected:
  context_type context_;
  midi2::ump::dispatcher_backend::m2cvm_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, NoteOff) {
  StrictMock<MockFunction<decltype(be_)::note_off_fn>> fn;
  midi2::ump::m2cvm::note_off noff;
  // The first call should do nothing since no handler has been installed.
  be_.note_off(context_, noff);
  // Install a handler for the noop message.
  be_.on_note_off(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  EXPECT_CALL(fn, Call(context_, noff)).Times(1);
  be_.note_off(context_, noff);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, NoteOn) {
  StrictMock<MockFunction<decltype(be_)::note_on_fn>> fn;
  midi2::ump::m2cvm::note_on non;
  be_.note_on(context_, non);
  be_.on_note_on(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, non)).Times(1);
  be_.note_on(context_, non);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, PolyPressure) {
  StrictMock<MockFunction<decltype(be_)::poly_pressure_fn>> fn;
  midi2::ump::m2cvm::poly_pressure pressure;
  be_.poly_pressure(context_, pressure);
  be_.on_poly_pressure(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, pressure)).Times(1);
  be_.poly_pressure(context_, pressure);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, ProgramChange) {
  midi2::ump::m2cvm::program_change pc;
  be_.program_change(context_, pc);

  StrictMock<MockFunction<decltype(be_)::program_change_fn>> fn;
  EXPECT_CALL(fn, Call(context_, pc)).Times(1);
  be_.on_program_change(fn.AsStdFunction());
  be_.program_change(context_, pc);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, ChannelPressure) {
  midi2::ump::m2cvm::channel_pressure pressure;
  be_.channel_pressure(context_, pressure);

  StrictMock<MockFunction<decltype(be_)::channel_pressure_fn>> fn;
  EXPECT_CALL(fn, Call(context_, pressure)).Times(1);
  be_.on_channel_pressure(fn.AsStdFunction());
  be_.channel_pressure(context_, pressure);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, RPNPerNoteController) {
  midi2::ump::m2cvm::rpn_per_note_controller rpn;
  be_.rpn_per_note_controller(context_, rpn);

  StrictMock<MockFunction<decltype(be_)::rpn_per_note_controller_fn>> fn;
  EXPECT_CALL(fn, Call(context_, rpn)).Times(1);
  be_.on_rpn_per_note_controller(fn.AsStdFunction());
  be_.rpn_per_note_controller(context_, rpn);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, NRPNPerNoteController) {
  midi2::ump::m2cvm::nrpn_per_note_controller nrpn;
  be_.nrpn_per_note_controller(context_, nrpn);

  StrictMock<MockFunction<decltype(be_)::nrpn_per_note_controller_fn>> fn;
  EXPECT_CALL(fn, Call(context_, nrpn)).Times(1);
  be_.on_nrpn_per_note_controller(fn.AsStdFunction());
  be_.nrpn_per_note_controller(context_, nrpn);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, RPNController) {
  midi2::ump::m2cvm::rpn_controller rpn;
  be_.rpn_controller(context_, rpn);

  StrictMock<MockFunction<decltype(be_)::rpn_controller_fn>> fn;
  EXPECT_CALL(fn, Call(context_, rpn)).Times(1);
  be_.on_rpn_controller(fn.AsStdFunction());
  be_.rpn_controller(context_, rpn);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, NRPNController) {
  midi2::ump::m2cvm::nrpn_controller nrpn;
  be_.nrpn_controller(context_, nrpn);

  StrictMock<MockFunction<decltype(be_)::nrpn_controller_fn>> fn;
  EXPECT_CALL(fn, Call(context_, nrpn)).Times(1);
  be_.on_nrpn_controller(fn.AsStdFunction());
  be_.nrpn_controller(context_, nrpn);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, RPNRelativeController) {
  midi2::ump::m2cvm::rpn_relative_controller message;
  be_.rpn_relative_controller(context_, message);

  StrictMock<MockFunction<decltype(be_)::rpn_relative_controller_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_rpn_relative_controller(fn.AsStdFunction());
  be_.rpn_relative_controller(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, NRPNRelativeController) {
  midi2::ump::m2cvm::nrpn_relative_controller message;
  be_.nrpn_relative_controller(context_, message);

  StrictMock<MockFunction<decltype(be_)::nrpn_relative_controller_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_nrpn_relative_controller(fn.AsStdFunction());
  be_.nrpn_relative_controller(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, PerNoteManagement) {
  midi2::ump::m2cvm::per_note_management message;
  be_.per_note_management(context_, message);

  StrictMock<MockFunction<decltype(be_)::per_note_management_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_per_note_management(fn.AsStdFunction());
  be_.per_note_management(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, ControlChange) {
  midi2::ump::m2cvm::control_change message;
  be_.control_change(context_, message);

  StrictMock<MockFunction<decltype(be_)::control_change_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_control_change(fn.AsStdFunction());
  be_.control_change(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, PitchBend) {
  midi2::ump::m2cvm::pitch_bend message;
  be_.pitch_bend(context_, message);

  StrictMock<MockFunction<decltype(be_)::pitch_bend_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_pitch_bend(fn.AsStdFunction());
  be_.pitch_bend(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendM2CVM, PerNotePitchBend) {
  midi2::ump::m2cvm::per_note_pitch_bend message;
  be_.per_note_pitch_bend(context_, message);

  StrictMock<MockFunction<decltype(be_)::per_note_pitch_bend_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_per_note_pitch_bend(fn.AsStdFunction());
  be_.per_note_pitch_bend(context_, message);
}

class DispatcherBackendData128 : public testing::Test {
protected:
  context_type context_;
  midi2::ump::dispatcher_backend::data128_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData128, SysEx8In1) {
  midi2::ump::data128::sysex8_in_1 sx;
  // The first call should do nothing since no handler has been installed.
  be_.sysex8_in_1(context_, sx);
  // Install a handler for the noop message.
  StrictMock<MockFunction<decltype(be_)::sysex8_in_1_fn>> fn;
  EXPECT_CALL(fn, Call(context_, sx)).Times(1);
  be_.on_sysex8_in_1(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  be_.sysex8_in_1(context_, sx);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData128, SysEx8Start) {
  midi2::ump::data128::sysex8_start sx;
  be_.sysex8_start(context_, sx);
  StrictMock<MockFunction<decltype(be_)::sysex8_start_fn>> fn;
  EXPECT_CALL(fn, Call(context_, sx)).Times(1);
  be_.on_sysex8_start(fn.AsStdFunction());
  be_.sysex8_start(context_, sx);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData128, SysEx8Continue) {
  midi2::ump::data128::sysex8_continue sx;
  be_.sysex8_continue(context_, sx);
  StrictMock<MockFunction<decltype(be_)::sysex8_continue_fn>> fn;
  EXPECT_CALL(fn, Call(context_, sx)).Times(1);
  be_.on_sysex8_continue(fn.AsStdFunction());
  be_.sysex8_continue(context_, sx);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData128, SysEx8End) {
  midi2::ump::data128::sysex8_end sx;
  be_.sysex8_end(context_, sx);
  StrictMock<MockFunction<decltype(be_)::sysex8_end_fn>> fn;
  EXPECT_CALL(fn, Call(context_, sx)).Times(1);
  be_.on_sysex8_end(fn.AsStdFunction());
  be_.sysex8_end(context_, sx);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData128, MDSHeader) {
  midi2::ump::data128::mds_header mds;
  be_.mds_header(context_, mds);
  StrictMock<MockFunction<decltype(be_)::mds_header_fn>> fn;
  EXPECT_CALL(fn, Call(context_, mds)).Times(1);
  be_.on_mds_header(fn.AsStdFunction());
  be_.mds_header(context_, mds);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendData128, MDSPayload) {
  midi2::ump::data128::mds_payload mds;
  be_.mds_payload(context_, mds);
  StrictMock<MockFunction<decltype(be_)::mds_payload_fn>> fn;
  EXPECT_CALL(fn, Call(context_, mds)).Times(1);
  be_.on_mds_payload(fn.AsStdFunction());
  be_.mds_payload(context_, mds);
}

class DispatcherBackendStream : public testing::Test {
protected:
  context_type context_;
  midi2::ump::dispatcher_backend::stream_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, EndpointDiscovery) {
  midi2::ump::stream::endpoint_discovery message;
  // The first call should do nothing since no handler has been installed.
  be_.endpoint_discovery(context_, message);
  // Install a handler for the noop message.
  StrictMock<MockFunction<decltype(be_)::endpoint_discovery_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_endpoint_discovery(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  be_.endpoint_discovery(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, EndpointNotification) {
  midi2::ump::stream::endpoint_info_notification message;
  be_.endpoint_info_notification(context_, message);

  StrictMock<MockFunction<decltype(be_)::endpoint_info_notification_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_endpoint_info_notification(fn.AsStdFunction());
  be_.endpoint_info_notification(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, DeviceIdentityNotification) {
  midi2::ump::stream::device_identity_notification message;
  be_.device_identity_notification(context_, message);

  StrictMock<MockFunction<decltype(be_)::device_identity_notification_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_device_identity_notification(fn.AsStdFunction());
  be_.device_identity_notification(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, EndpointNameNotification) {
  midi2::ump::stream::endpoint_name_notification message;
  be_.endpoint_name_notification(context_, message);

  StrictMock<MockFunction<decltype(be_)::endpoint_name_notification_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_endpoint_name_notification(fn.AsStdFunction());
  be_.endpoint_name_notification(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, ProduceInstanceIDNotification) {
  midi2::ump::stream::product_instance_id_notification message;
  be_.product_instance_id_notification(context_, message);

  StrictMock<MockFunction<decltype(be_)::product_instance_id_notification_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_product_instance_id_notification(fn.AsStdFunction());
  be_.product_instance_id_notification(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, JRConfigurationRequest) {
  midi2::ump::stream::jr_configuration_request message;
  be_.jr_configuration_request(context_, message);

  StrictMock<MockFunction<decltype(be_)::jr_configuration_request_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_jr_configuration_request(fn.AsStdFunction());
  be_.jr_configuration_request(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, JRConfigurationNotification) {
  midi2::ump::stream::jr_configuration_notification message;
  be_.jr_configuration_notification(context_, message);

  StrictMock<MockFunction<decltype(be_)::jr_configuration_notification_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_jr_configuration_notification(fn.AsStdFunction());
  be_.jr_configuration_notification(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, FunctionBlockDiscovery) {
  midi2::ump::stream::function_block_discovery message;
  be_.function_block_discovery(context_, message);

  StrictMock<MockFunction<decltype(be_)::function_block_discovery_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_function_block_discovery(fn.AsStdFunction());
  be_.function_block_discovery(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, FunctionBlockInfoNotification) {
  midi2::ump::stream::function_block_info_notification message;
  be_.function_block_info_notification(context_, message);

  StrictMock<MockFunction<decltype(be_)::function_block_info_notification_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_function_block_info_notification(fn.AsStdFunction());
  be_.function_block_info_notification(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, FunctionBlockNameNotification) {
  midi2::ump::stream::function_block_name_notification message;
  be_.function_block_name_notification(context_, message);

  StrictMock<MockFunction<decltype(be_)::function_block_name_notification_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_function_block_name_notification(fn.AsStdFunction());
  be_.function_block_name_notification(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, StartOfClip) {
  midi2::ump::stream::start_of_clip message;
  be_.start_of_clip(context_, message);

  StrictMock<MockFunction<decltype(be_)::start_of_clip_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_start_of_clip(fn.AsStdFunction());
  be_.start_of_clip(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendStream, EndOfClip) {
  midi2::ump::stream::end_of_clip message;
  be_.end_of_clip(context_, message);

  StrictMock<MockFunction<decltype(be_)::end_of_clip_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_end_of_clip(fn.AsStdFunction());
  be_.end_of_clip(context_, message);
}

class DispatcherBackendFlexData : public testing::Test {
protected:
  context_type context_;
  midi2::ump::dispatcher_backend::flex_data_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(DispatcherBackendFlexData, SetTempo) {
  midi2::ump::flex_data::set_tempo message;
  // The first call should do nothing since no handler has been installed.
  be_.set_tempo(context_, message);
  // Install a handler for the noop message.
  StrictMock<MockFunction<decltype(be_)::set_tempo_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_set_tempo(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  be_.set_tempo(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendFlexData, SetTimeSignature) {
  midi2::ump::flex_data::set_time_signature message;
  be_.set_time_signature(context_, message);

  StrictMock<MockFunction<decltype(be_)::set_time_signature_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_set_time_signature(fn.AsStdFunction());
  be_.set_time_signature(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendFlexData, SetMetronome) {
  midi2::ump::flex_data::set_metronome message;
  be_.set_metronome(context_, message);

  StrictMock<MockFunction<decltype(be_)::set_metronome_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_set_metronome(fn.AsStdFunction());
  be_.set_metronome(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendFlexData, SetKeySignature) {
  midi2::ump::flex_data::set_key_signature message;
  be_.set_key_signature(context_, message);

  StrictMock<MockFunction<decltype(be_)::set_key_signature_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_set_key_signature(fn.AsStdFunction());
  be_.set_key_signature(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendFlexData, SetChordName) {
  midi2::ump::flex_data::set_chord_name message;
  be_.set_chord_name(context_, message);

  StrictMock<MockFunction<decltype(be_)::set_chord_name_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_set_chord_name(fn.AsStdFunction());
  be_.set_chord_name(context_, message);
}
// NOLINTNEXTLINE
TEST_F(DispatcherBackendFlexData, Text) {
  midi2::ump::flex_data::text_common message;
  be_.text(context_, message);

  StrictMock<MockFunction<decltype(be_)::text_fn>> fn;
  EXPECT_CALL(fn, Call(context_, message)).Times(1);
  be_.on_text(fn.AsStdFunction());
  be_.text(context_, message);
}

}  // end anonymous namespace
