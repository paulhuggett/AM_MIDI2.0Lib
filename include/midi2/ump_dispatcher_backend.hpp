//===-- UMP Dispatcher Backends -----------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//
#ifndef MIDI2_UMP_DISPATCHER_BACKEND_HPP
#define MIDI2_UMP_DISPATCHER_BACKEND_HPP

#include <functional>

#include "midi2/ump_types.hpp"

namespace midi2::ump::dispatcher_backend {

// clang-format off
template <typename T, typename Context>
concept utility = requires(T v, Context context) {
  // 7.2.1 NOOP
  { v.noop(context) } -> std::same_as<void>;
  // 7.2.2.1 JR Clock Message
  { v.jr_clock(context, ump::utility::jr_clock{}) } -> std::same_as<void>;
  // 7.2.2.2 JR Timestamp Message
  { v.jr_timestamp(context, ump::utility::jr_timestamp{}) } -> std::same_as<void>;
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
  { v.delta_clockstamp_tpqn(context, ump::utility::delta_clockstamp_tpqn{}) } -> std::same_as<void>;
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  { v.delta_clockstamp(context, ump::utility::delta_clockstamp{}) } -> std::same_as<void>;

  { v.unknown(context, std::span<std::uint32_t>{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept system = requires(T v, Context context) {
  // 7.6 System Common and System Real Time Messages
  { v.midi_time_code(context, ump::system::midi_time_code{}) } -> std::same_as<void>;
  { v.song_position_pointer(context, ump::system::song_position_pointer{}) } -> std::same_as<void>;
  { v.song_select(context,  ump::system::song_select{}) } -> std::same_as<void>;
  { v.tune_request(context,  ump::system::tune_request{}) } -> std::same_as<void>;
  { v.timing_clock(context,  ump::system::timing_clock{}) } -> std::same_as<void>;
  { v.seq_start(context,  ump::system::sequence_start{}) } -> std::same_as<void>;
  { v.seq_continue(context,  ump::system::sequence_continue{}) } -> std::same_as<void>;
  { v.seq_stop(context,  ump::system::sequence_stop{}) } -> std::same_as<void>;
  { v.active_sensing(context,  ump::system::active_sensing{}) } -> std::same_as<void>;
  { v.reset(context,  ump::system::reset{}) } -> std::same_as<void>;
};
template<typename T, typename Context>
concept m1cvm = requires(T v, Context context) {
  { v.note_off(context, ump::m1cvm::note_off{}) } -> std::same_as<void>;
  { v.note_on(context, ump::m1cvm::note_on{}) } -> std::same_as<void>;
  { v.poly_pressure(context, ump::m1cvm::poly_pressure{}) } -> std::same_as<void>;
  { v.control_change(context, ump::m1cvm::control_change{}) } -> std::same_as<void>;
  { v.program_change(context, ump::m1cvm::program_change{}) } -> std::same_as<void>;
  { v.channel_pressure(context, ump::m1cvm::channel_pressure{}) } -> std::same_as<void>;
  { v.pitch_bend(context, ump::m1cvm::pitch_bend{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept data64 = requires(T v, Context context) {
  { v.sysex7_in_1(context, ump::data64::sysex7_in_1{}) } -> std::same_as<void>;
  { v.sysex7_start(context, ump::data64::sysex7_start{}) } -> std::same_as<void>;
  { v.sysex7_continue(context, ump::data64::sysex7_continue{}) } -> std::same_as<void>;
  { v.sysex7_end(context, ump::data64::sysex7_end{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept m2cvm = requires(T v, Context context) {
  // 7.4.1 MIDI 2.0 Note Off Message (status=0x8)
  { v.note_off(context, ump::m2cvm::note_off{}) } -> std::same_as<void>;
  // 7.4.2 MIDI 2.0 Note On Message (status=0x9)
  { v.note_on(context, ump::m2cvm::note_on{}) } -> std::same_as<void>;
  // 7.4.3 MIDI 2.0 Poly Pressure Message (status=0xA)
  { v.poly_pressure(context, ump::m2cvm::poly_pressure{}) } -> std::same_as<void>;

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  { v.rpn_per_note_controller(context, midi2::ump::m2cvm::rpn_per_note_controller{}) } -> std::same_as<void>;
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  { v.nrpn_per_note_controller(context, midi2::ump::m2cvm::nrpn_per_note_controller{}) } -> std::same_as<void>;
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  { v.rpn_controller(context, midi2::ump::m2cvm::rpn_controller{}) } -> std::same_as<void>;
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  { v.nrpn_controller(context, midi2::ump::m2cvm::nrpn_controller{}) } -> std::same_as<void>;
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  { v.rpn_relative_controller(context, midi2::ump::m2cvm::rpn_relative_controller{}) } -> std::same_as<void>;
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  { v.nrpn_relative_controller(context, midi2::ump::m2cvm::nrpn_relative_controller{}) } -> std::same_as<void>;

  // 7.4.9 MIDI 2.0 Program Change Message (status=0xC)
  { v.program_change(context, ump::m2cvm::program_change{}) } -> std::same_as<void>;
  // 7.4.10 MIDI 2.0 Channel Pressure Message (status=0xD)
  { v.channel_pressure(context, ump::m2cvm::channel_pressure{}) } -> std::same_as<void>;

  { v.per_note_management(context, ump::m2cvm::per_note_management{}) } -> std::same_as<void>;
  { v.control_change(context, ump::m2cvm::control_change{}) } -> std::same_as<void>;

  { v.pitch_bend(context, ump::m2cvm::pitch_bend{}) } -> std::same_as<void>;
  { v.per_note_pitch_bend(context, ump::m2cvm::per_note_pitch_bend{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept data128 = requires(T v, Context context) {
  // 7.8 System Exclusive 8 (8-Bit) Messages
  { v.sysex8_in_1(context, ump::data128::sysex8_in_1{}) } -> std::same_as<void>;
  { v.sysex8_start(context, ump::data128::sysex8_start{}) } -> std::same_as<void>;
  { v.sysex8_continue(context, ump::data128::sysex8_continue{}) } -> std::same_as<void>;
  { v.sysex8_end(context, ump::data128::sysex8_end{}) } -> std::same_as<void>;
  // 7.9 Mixed Data Set Message
  { v.mds_header(context, ump::data128::mds_header{}) } -> std::same_as<void>;
  { v.mds_payload(context, ump::data128::mds_payload{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept stream = requires(T v, Context context) {
  { v.endpoint_discovery(context, ump::stream::endpoint_discovery{}) } -> std::same_as<void>;
  { v.endpoint_info_notification(context, ump::stream::endpoint_info_notification{}) } -> std::same_as<void>;
  { v.device_identity_notification(context, ump::stream::device_identity_notification{}) } -> std::same_as<void>;
  { v.endpoint_name_notification(context, ump::stream::endpoint_name_notification{}) } -> std::same_as<void>;
  { v.product_instance_id_notification(context, ump::stream::product_instance_id_notification{}) } -> std::same_as<void>;
  { v.jr_configuration_request(context, ump::stream::jr_configuration_request{}) } -> std::same_as<void>;
  { v.jr_configuration_notification(context, ump::stream::jr_configuration_notification{}) } -> std::same_as<void>;
  { v.function_block_discovery(context, ump::stream::function_block_discovery{}) } -> std::same_as<void>;
  { v.function_block_info_notification(context, ump::stream::function_block_info_notification{}) } -> std::same_as<void>;
  { v.function_block_name_notification(context, ump::stream::function_block_name_notification{}) } -> std::same_as<void>;
  { v.start_of_clip(context, ump::stream::start_of_clip{}) } -> std::same_as<void>;
  { v.end_of_clip(context, ump::stream::end_of_clip{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept flex_data = requires(T v, Context context) {
  { v.set_tempo(context, ump::flex_data::set_tempo{}) } -> std::same_as<void>;
  { v.set_time_signature(context, ump::flex_data::set_time_signature{}) } -> std::same_as<void>;
  { v.set_metronome(context, ump::flex_data::set_metronome{}) } -> std::same_as<void>;
  { v.set_key_signature(context, ump::flex_data::set_key_signature{}) } -> std::same_as<void>;
  { v.set_chord_name(context, ump::flex_data::set_chord_name{}) } -> std::same_as<void>;
  { v.text(context, ump::flex_data::text_common{}) } -> std::same_as<void>;
};
// clang-format on

template <typename Context> struct utility_null {
  // 7.2.1 NOOP
  constexpr static void noop(Context) { /* do nothing */ }
  // 7.2.2.1 JR Clock Message
  constexpr static void jr_clock(Context, ump::utility::jr_clock const &) { /* do nothing */ }
  // 7.2.2.2 JR Timestamp Message
  constexpr static void jr_timestamp(Context, ump::utility::jr_timestamp const &) { /* do nothing */ }
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
  constexpr static void delta_clockstamp_tpqn(Context, ump::utility::delta_clockstamp_tpqn const &) { /* do nothing */ }
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  constexpr static void delta_clockstamp(Context, ump::utility::delta_clockstamp const &) { /* do nothing */ }

  constexpr static void unknown(Context, std::span<std::uint32_t>) { /* do nothing */ }
};

static_assert(utility<utility_null<int>, int>, "utility_null must implement the utility concept");

template <typename Context> struct system_null {
  // 7.6 System Common and System Real Time Messages
  constexpr static void midi_time_code(Context, ump::system::midi_time_code const &) { /* do nothing */ }
  constexpr static void song_position_pointer(Context, ump::system::song_position_pointer const &) { /* do nothing */ }
  constexpr static void song_select(Context, ump::system::song_select const &) { /* do nothing */ }
  constexpr static void tune_request(Context, ump::system::tune_request const &) { /* do nothing */ }
  constexpr static void timing_clock(Context, ump::system::timing_clock const &) { /* do nothing */ }
  constexpr static void seq_start(Context, ump::system::sequence_start const &) { /* do nothing */ }
  constexpr static void seq_continue(Context, ump::system::sequence_continue const &) { /* do nothing */ }
  constexpr static void seq_stop(Context, ump::system::sequence_stop const &) { /* do nothing */ }
  constexpr static void active_sensing(Context, ump::system::active_sensing const &) { /* do nothing */ }
  constexpr static void reset(Context, ump::system::reset const &) { /* do nothing */ }
};

static_assert(system<system_null<int>, int>, "system_null must implement the system concept");

template <typename Context> struct m1cvm_null {
  constexpr static void note_off(Context, ump::m1cvm::note_off const &) { /* do nothing */ }
  constexpr static void note_on(Context, ump::m1cvm::note_on const &) { /* do nothing */ }
  constexpr static void poly_pressure(Context, ump::m1cvm::poly_pressure const &) { /* do nothing */ }
  constexpr static void control_change(Context, ump::m1cvm::control_change const &) { /* do nothing */ }
  constexpr static void program_change(Context, ump::m1cvm::program_change const &) { /* do nothing */ }
  constexpr static void channel_pressure(Context, ump::m1cvm::channel_pressure const &) { /* do nothing */ }
  constexpr static void pitch_bend(Context, ump::m1cvm::pitch_bend const &) { /* do nothing */ }
};

static_assert(m1cvm<m1cvm_null<int>, int>, "m1cvm_null must implement the m1cvm concept");

template <typename Context> struct data64_null {
  constexpr static void sysex7_in_1(Context, ump::data64::sysex7_in_1 const &) { /* do nothing */ }
  constexpr static void sysex7_start(Context, ump::data64::sysex7_start const &) { /* do nothing */ }
  constexpr static void sysex7_continue(Context, ump::data64::sysex7_continue const &) { /* do nothing */ }
  constexpr static void sysex7_end(Context, ump::data64::sysex7_end const &) { /* do nothing */ }
};

static_assert(data64<data64_null<int>, int>, "data64_null must implement the data64 concept");

template <typename Context> struct m2cvm_null {
  constexpr static void note_off(Context, ump::m2cvm::note_off const &) { /* do nothing */ }
  constexpr static void note_on(Context, ump::m2cvm::note_on const &) { /* do nothing */ }
  constexpr static void poly_pressure(Context, ump::m2cvm::poly_pressure const &) { /* do nothing */ }
  constexpr static void program_change(Context, ump::m2cvm::program_change const &) { /* do nothing */ }
  constexpr static void channel_pressure(Context, ump::m2cvm::channel_pressure const &) { /* do nothing */ }

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  constexpr static void rpn_per_note_controller(Context, ump::m2cvm::rpn_per_note_controller const &) { /* do nothing */
  }
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  constexpr static void nrpn_per_note_controller(Context,
                                                 ump::m2cvm::nrpn_per_note_controller const &) { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  constexpr static void rpn_controller(Context, ump::m2cvm::rpn_controller const &) { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  constexpr static void nrpn_controller(Context, ump::m2cvm::nrpn_controller const &) { /* do nothing */ }
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  constexpr static void rpn_relative_controller(Context, ump::m2cvm::rpn_relative_controller const &) { /* do nothing */
  }
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  constexpr static void nrpn_relative_controller(Context,
                                                 ump::m2cvm::nrpn_relative_controller const &) { /* do nothing */ }

  constexpr static void per_note_management(Context, ump::m2cvm::per_note_management const &) { /* do nothing */ }
  constexpr static void control_change(Context, ump::m2cvm::control_change const &) { /* do nothing */ }
  constexpr static void pitch_bend(Context, ump::m2cvm::pitch_bend const &) { /* do nothing */ }
  constexpr static void per_note_pitch_bend(Context, ump::m2cvm::per_note_pitch_bend const &) { /* do nothing */ }
};

static_assert(m2cvm<m2cvm_null<int>, int>, "m2cvm_null must implement the m2cvm concept");

template <typename Context> struct data128_null {
  constexpr static void sysex8_in_1(Context, ump::data128::sysex8_in_1 const &) { /* do nothing */ }
  constexpr static void sysex8_start(Context, ump::data128::sysex8_start const &) { /* do nothing */ }
  constexpr static void sysex8_continue(Context, ump::data128::sysex8_continue const &) { /* do nothing */ }
  constexpr static void sysex8_end(Context, ump::data128::sysex8_end const &) { /* do nothing */ }
  constexpr static void mds_header(Context, ump::data128::mds_header const &) { /* do nothing */ }
  constexpr static void mds_payload(Context, ump::data128::mds_payload const &) { /* do nothing */ }
};

static_assert(data128<data128_null<int>, int>, "data128_null must implement the data128 concept");

template <typename Context> struct stream_null {
  constexpr static void endpoint_discovery(Context, ump::stream::endpoint_discovery const &) { /* do nothing */ }
  constexpr static void endpoint_info_notification(Context,
                                                   ump::stream::endpoint_info_notification const &) { /* do nothing */ }
  constexpr static void device_identity_notification(
      Context, ump::stream::device_identity_notification const &) { /* do nothing */ }
  constexpr static void endpoint_name_notification(Context,
                                                   ump::stream::endpoint_name_notification const &) { /* do nothing */ }
  constexpr static void product_instance_id_notification(
      Context, ump::stream::product_instance_id_notification const &) { /* do nothing */ }
  constexpr static void jr_configuration_request(Context,
                                                 ump::stream::jr_configuration_request const &) { /* do nothing */ }
  constexpr static void jr_configuration_notification(
      Context, ump::stream::jr_configuration_notification const &) { /* do nothing */ }

  constexpr static void function_block_discovery(Context,
                                                 ump::stream::function_block_discovery const &) { /* do nothing */ }
  constexpr static void function_block_info_notification(
      Context, ump::stream::function_block_info_notification const &) { /* do nothing */ }
  constexpr static void function_block_name_notification(
      Context, ump::stream::function_block_name_notification const &) { /* do nothing */ }

  constexpr static void start_of_clip(Context, ump::stream::start_of_clip const &) { /* do nothing */ }
  constexpr static void end_of_clip(Context, ump::stream::end_of_clip const &) { /* do nothing */ }
};

static_assert(stream<stream_null<int>, int>, "stream_null must implement the stream concept");

template <typename Context> struct flex_data_null {
  constexpr static void set_tempo(Context, ump::flex_data::set_tempo const &) { /* do nothing */ }
  constexpr static void set_time_signature(Context, ump::flex_data::set_time_signature const &) { /* do nothing */ }
  constexpr static void set_metronome(Context, ump::flex_data::set_metronome const &) { /* do nothing */ }
  constexpr static void set_key_signature(Context, ump::flex_data::set_key_signature const &) { /* do nothing */ }
  constexpr static void set_chord_name(Context, ump::flex_data::set_chord_name const &) { /* do nothing */ }
  constexpr static void text(Context, ump::flex_data::text_common const &) { /* do nothing */ }
};

static_assert(flex_data<flex_data_null<int>, int>, "flex_data_null must implement the flex_data concept");

template <typename Context> struct utility_pure {
  virtual ~utility_pure() noexcept = default;

  // 7.2.1 NOOP
  virtual void noop(Context) = 0;
  // 7.2.2.1 JR Clock Message
  virtual void jr_clock(Context, ump::utility::jr_clock const &) = 0;
  // 7.2.2.2 JR Timestamp Message
  virtual void jr_timestamp(Context, ump::utility::jr_timestamp const &) = 0;
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
  virtual void delta_clockstamp_tpqn(Context, ump::utility::delta_clockstamp_tpqn const &) = 0;
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  virtual void delta_clockstamp(Context, ump::utility::delta_clockstamp const &) = 0;

  virtual void unknown(Context, std::span<std::uint32_t>) = 0;
};

static_assert(utility<utility_pure<int>, int>, "utility_pure must implement the utility concept");

template <typename Context> struct system_pure {
  virtual ~system_pure() = default;

  // 7.6 System Common and System Real Time Messages
  virtual void midi_time_code(Context, ump::system::midi_time_code const &) = 0;
  virtual void song_position_pointer(Context, ump::system::song_position_pointer const &) = 0;
  virtual void song_select(Context, ump::system::song_select const &) = 0;
  virtual void tune_request(Context, ump::system::tune_request const &) = 0;
  virtual void timing_clock(Context, ump::system::timing_clock const &) = 0;
  virtual void seq_start(Context, ump::system::sequence_start const &) = 0;
  virtual void seq_continue(Context, ump::system::sequence_continue const &) = 0;
  virtual void seq_stop(Context, ump::system::sequence_stop const &) = 0;
  virtual void active_sensing(Context, ump::system::active_sensing const &) = 0;
  virtual void reset(Context, ump::system::reset const &) = 0;
};

static_assert(system<system_pure<int>, int>, "system_pure must implement the system concept");

template <typename Context> struct m1cvm_pure {
  virtual ~m1cvm_pure() = default;

  virtual void note_off(Context, ump::m1cvm::note_off const &) = 0;
  virtual void note_on(Context, ump::m1cvm::note_on const &) = 0;
  virtual void poly_pressure(Context, ump::m1cvm::poly_pressure const &) = 0;
  virtual void control_change(Context, ump::m1cvm::control_change const &) = 0;
  virtual void program_change(Context, ump::m1cvm::program_change const &) = 0;
  virtual void channel_pressure(Context, ump::m1cvm::channel_pressure const &) = 0;
  virtual void pitch_bend(Context, ump::m1cvm::pitch_bend const &) = 0;
};

static_assert(m1cvm<m1cvm_pure<int>, int>, "m1cvm_pure must implement the m1cvm concept");

template <typename Context> struct data64_pure {
  virtual ~data64_pure() = default;

  virtual void sysex7_in_1(Context, ump::data64::sysex7_in_1 const &) = 0;
  virtual void sysex7_start(Context, ump::data64::sysex7_start const &) = 0;
  virtual void sysex7_continue(Context, ump::data64::sysex7_continue const &) = 0;
  virtual void sysex7_end(Context, ump::data64::sysex7_end const &) = 0;
};

static_assert(data64<data64_pure<int>, int>, "data64_pure must implement the data64 concept");

template <typename Context> struct m2cvm_pure {
  virtual ~m2cvm_pure() = default;

  virtual void note_off(Context, ump::m2cvm::note_off const &) = 0;
  virtual void note_on(Context, ump::m2cvm::note_on const &) = 0;
  virtual void poly_pressure(Context, ump::m2cvm::poly_pressure const &) = 0;
  virtual void program_change(Context, ump::m2cvm::program_change const &) = 0;
  virtual void channel_pressure(Context, ump::m2cvm::channel_pressure const &) = 0;

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  virtual void rpn_per_note_controller(Context, ump::m2cvm::rpn_per_note_controller const &) = 0;
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  virtual void nrpn_per_note_controller(Context, ump::m2cvm::nrpn_per_note_controller const &) = 0;
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  virtual void rpn_controller(Context, ump::m2cvm::rpn_controller const &) = 0;
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  virtual void nrpn_controller(Context, ump::m2cvm::nrpn_controller const &) = 0;
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  virtual void rpn_relative_controller(Context, ump::m2cvm::rpn_relative_controller const &) = 0;
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  virtual void nrpn_relative_controller(Context, ump::m2cvm::nrpn_relative_controller const &) = 0;

  virtual void per_note_management(Context, ump::m2cvm::per_note_management const &) = 0;
  virtual void control_change(Context, ump::m2cvm::control_change const &) = 0;
  virtual void pitch_bend(Context, ump::m2cvm::pitch_bend const &) = 0;
  virtual void per_note_pitch_bend(Context, ump::m2cvm::per_note_pitch_bend const &) = 0;
};

static_assert(m2cvm<m2cvm_pure<int>, int>, "m2cvm_pure must implement the m2cvm concept");

template <typename Context> struct data128_pure {
  virtual ~data128_pure() = default;

  virtual void sysex8_in_1(Context, ump::data128::sysex8_in_1 const &) = 0;
  virtual void sysex8_start(Context, ump::data128::sysex8_start const &) = 0;
  virtual void sysex8_continue(Context, ump::data128::sysex8_continue const &) = 0;
  virtual void sysex8_end(Context, ump::data128::sysex8_end const &) = 0;
  virtual void mds_header(Context, ump::data128::mds_header const &) = 0;
  virtual void mds_payload(Context, ump::data128::mds_payload const &) = 0;
};

static_assert(data128<data128_pure<int>, int>, "data128_pure must implement the data128 concept");

template <typename Context> struct stream_pure {
  virtual ~stream_pure() = default;

  virtual void endpoint_discovery(Context, ump::stream::endpoint_discovery const &) = 0;
  virtual void endpoint_info_notification(Context, ump::stream::endpoint_info_notification const &) = 0;
  virtual void device_identity_notification(Context, ump::stream::device_identity_notification const &) = 0;
  virtual void endpoint_name_notification(Context, ump::stream::endpoint_name_notification const &) = 0;
  virtual void product_instance_id_notification(Context, ump::stream::product_instance_id_notification const &) = 0;
  virtual void jr_configuration_request(Context, ump::stream::jr_configuration_request const &) = 0;
  virtual void jr_configuration_notification(Context, ump::stream::jr_configuration_notification const &) = 0;

  virtual void function_block_discovery(Context, ump::stream::function_block_discovery const &) = 0;
  virtual void function_block_info_notification(Context, ump::stream::function_block_info_notification const &) = 0;
  virtual void function_block_name_notification(Context, ump::stream::function_block_name_notification const &) = 0;

  virtual void start_of_clip(Context, ump::stream::start_of_clip const &) = 0;
  virtual void end_of_clip(Context, ump::stream::end_of_clip const &) = 0;
};

static_assert(stream<stream_pure<int>, int>, "stream_pure must implement the stream concept");

template <typename Context> struct flex_data_pure {
  virtual ~flex_data_pure() = default;

  virtual void set_tempo(Context, ump::flex_data::set_tempo const &) = 0;
  virtual void set_time_signature(Context, ump::flex_data::set_time_signature const &) = 0;
  virtual void set_metronome(Context, ump::flex_data::set_metronome const &) = 0;
  virtual void set_key_signature(Context, ump::flex_data::set_key_signature const &) = 0;
  virtual void set_chord_name(Context, ump::flex_data::set_chord_name const &) = 0;
  virtual void text(Context, ump::flex_data::text_common const &) = 0;
};

static_assert(flex_data<flex_data_pure<int>, int>, "flex_data_pure must implement the flex_data concept");

template <typename Context> struct utility_base : public utility_pure<Context> {
  // 7.2.1 NOOP
  void noop(Context) override { /* do nothing */ }
  // 7.2.2.1 JR Clock Message
  void jr_clock(Context, ump::utility::jr_clock const &) override { /* do nothing */ }
  // 7.2.2.2 JR Timestamp Message
  void jr_timestamp(Context, ump::utility::jr_timestamp const &) override { /* do nothing */ }
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
  void delta_clockstamp_tpqn(Context, ump::utility::delta_clockstamp_tpqn const &) override { /* do nothing */ }
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  void delta_clockstamp(Context, ump::utility::delta_clockstamp const &) override { /* do nothing */ }

  void unknown(Context, std::span<std::uint32_t>) override { /* do nothing */ }
};
template <typename Context> struct system_base : public system_pure<Context> {
  // 7.6 System Common and System Real Time Messages
  void midi_time_code(Context, ump::system::midi_time_code const &) override { /* do nothing */ }
  void song_position_pointer(Context, ump::system::song_position_pointer const &) override { /* do nothing */ }
  void song_select(Context, ump::system::song_select const &) override { /* do nothing */ }
  void tune_request(Context, ump::system::tune_request const &) override { /* do nothing */ }
  void timing_clock(Context, ump::system::timing_clock const &) override { /* do nothing */ }
  void seq_start(Context, ump::system::sequence_start const &) override { /* do nothing */ }
  void seq_continue(Context, ump::system::sequence_continue const &) override { /* do nothing */ }
  void seq_stop(Context, ump::system::sequence_stop const &) override { /* do nothing */ }
  void active_sensing(Context, ump::system::active_sensing const &) override { /* do nothing */ }
  void reset(Context, ump::system::reset const &) override { /* do nothing */ }
};
template <typename Context> struct m1cvm_base : public m1cvm_pure<Context> {
  void note_off(Context, ump::m1cvm::note_off const &) override { /* do nothing */ }
  void note_on(Context, ump::m1cvm::note_on const &) override { /* do nothing */ }
  void poly_pressure(Context, ump::m1cvm::poly_pressure const &) override { /* do nothing */ }
  void control_change(Context, ump::m1cvm::control_change const &) override { /* do nothing */ }
  void program_change(Context, ump::m1cvm::program_change const &) override { /* do nothing */ }
  void channel_pressure(Context, ump::m1cvm::channel_pressure const &) override { /* do nothing */ }
  void pitch_bend(Context, ump::m1cvm::pitch_bend const &) override { /* do nothing */ }
};
template <typename Context> struct data64_base : public data64_pure<Context> {
  void sysex7_in_1(Context, ump::data64::sysex7_in_1 const &) override { /* do nothing */ }
  void sysex7_start(Context, ump::data64::sysex7_start const &) override { /* do nothing */ }
  void sysex7_continue(Context, ump::data64::sysex7_continue const &) override { /* do nothing */ }
  void sysex7_end(Context, ump::data64::sysex7_end const &) override { /* do nothing */ }
};
template <typename Context> struct m2cvm_base : public m2cvm_pure<Context> {
  void note_off(Context, ump::m2cvm::note_off const &) override { /* do nothing */ }
  void note_on(Context, ump::m2cvm::note_on const &) override { /* do nothing */ }
  void poly_pressure(Context, ump::m2cvm::poly_pressure const &) override { /* do nothing */ }
  void program_change(Context, ump::m2cvm::program_change const &) override { /* do nothing */ }
  void channel_pressure(Context, ump::m2cvm::channel_pressure const &) override { /* do nothing */ }

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  void rpn_per_note_controller(Context, ump::m2cvm::rpn_per_note_controller const &) override { /* do nothing */ }
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  void nrpn_per_note_controller(Context, ump::m2cvm::nrpn_per_note_controller const &) override { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  void rpn_controller(Context, ump::m2cvm::rpn_controller const &) override { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  void nrpn_controller(Context, ump::m2cvm::nrpn_controller const &) override { /* do nothing */ }
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  void rpn_relative_controller(Context, ump::m2cvm::rpn_relative_controller const &) override { /* do nothing */ }
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  void nrpn_relative_controller(Context, ump::m2cvm::nrpn_relative_controller const &) override { /* do nothing */ }

  void per_note_management(Context, ump::m2cvm::per_note_management const &) override { /* do nothing */ }
  void control_change(Context, ump::m2cvm::control_change const &) override { /* do nothing */ }
  void pitch_bend(Context, ump::m2cvm::pitch_bend const &) override { /* do nothing */ }
  void per_note_pitch_bend(Context, ump::m2cvm::per_note_pitch_bend const &) override { /* do nothing */ }
};
template <typename Context> struct data128_base : public data128_pure<Context> {
  void sysex8_in_1(Context, ump::data128::sysex8_in_1 const &) override { /* do nothing */ }
  void sysex8_start(Context, ump::data128::sysex8_start const &) override { /* do nothing */ }
  void sysex8_continue(Context, ump::data128::sysex8_continue const &) override { /* do nothing */ }
  void sysex8_end(Context, ump::data128::sysex8_end const &) override { /* do nothing */ }
  void mds_header(Context, ump::data128::mds_header const &) override { /* do nothing */ }
  void mds_payload(Context, ump::data128::mds_payload const &) override { /* do nothing */ }
};
template <typename Context> struct stream_base : public stream_pure<Context> {
  void endpoint_discovery(Context, ump::stream::endpoint_discovery const &) override { /* do nothing */ }
  void endpoint_info_notification(Context, ump::stream::endpoint_info_notification const &) override { /* do nothing */ }
  void device_identity_notification(Context,
                                    ump::stream::device_identity_notification const &) override { /* do nothing */ }
  void endpoint_name_notification(Context, ump::stream::endpoint_name_notification const &) override { /* do nothing */ }
  void product_instance_id_notification(Context, ump::stream::product_instance_id_notification const &) override { /* do nothing */ }
  void jr_configuration_request(Context, ump::stream::jr_configuration_request const &) override { /* do nothing */ }
  void jr_configuration_notification(Context, ump::stream::jr_configuration_notification const &) override { /* do nothing */ }

  void function_block_discovery(Context, ump::stream::function_block_discovery const &) override { /* do nothing */ }
  void function_block_info_notification(Context, ump::stream::function_block_info_notification const &) override { /* do nothing */ }
  void function_block_name_notification(Context, ump::stream::function_block_name_notification const &) override { /* do nothing */ }

  void start_of_clip(Context, ump::stream::start_of_clip const &) override { /* do nothing */ }
  void end_of_clip(Context, ump::stream::end_of_clip const &) override { /* do nothing */ }
};
template <typename Context> struct flex_data_base : public flex_data_pure<Context> {
  void set_tempo(Context, ump::flex_data::set_tempo const &) override { /* do nothing */ }
  void set_time_signature(Context, ump::flex_data::set_time_signature const &) override { /* do nothing */ }
  void set_metronome(Context, ump::flex_data::set_metronome const &) override { /* do nothing */ }
  void set_key_signature(Context, ump::flex_data::set_key_signature const &) override { /* do nothing */ }
  void set_chord_name(Context, ump::flex_data::set_chord_name const &) override { /* do nothing */ }
  void text(Context, ump::flex_data::text_common const &) override { /* do nothing */ }
};

template <typename Context> class utility_function {
public:
  using noop_fn = std::function<void(Context)>;
  using jr_clock_fn = std::function<void(Context, ump::utility::jr_clock const &)>;
  using jr_timestamp_fn = std::function<void(Context, ump::utility::jr_timestamp const &)>;
  using delta_clockstamp_tpqn_fn = std::function<void(Context, ump::utility::delta_clockstamp_tpqn const &)>;
  using delta_clockstamp_fn = std::function<void(Context, ump::utility::delta_clockstamp const &)>;
  using unknown_fn = std::function<void(Context, std::span<std::uint32_t>)>;

  constexpr utility_function &on_noop(noop_fn noop) noexcept {
    noop_ = std::move(noop);
    return *this;
  }
  constexpr utility_function &on_jr_clock(jr_clock_fn jr_clock) noexcept {
    jr_clock_ = std::move(jr_clock);
    return *this;
  }
  constexpr utility_function &on_jr_timestamp(jr_timestamp_fn jr_timestamp) noexcept {
    jr_timestamp_ = std::move(jr_timestamp);
    return *this;
  }
  constexpr utility_function &on_delta_clockstamp_tpqn(delta_clockstamp_tpqn_fn delta_clockstamp) noexcept {
    delta_clockstamp_tpqn_ = std::move(delta_clockstamp);
    return *this;
  }
  constexpr utility_function &on_delta_clockstamp(delta_clockstamp_fn delta_clockstamp) noexcept {
    delta_clockstamp_ = std::move(delta_clockstamp);
    return *this;
  }
  constexpr utility_function &on_unknown(unknown_fn unknown) noexcept {
    unknown_ = std::move(unknown);
    return *this;
  }

  void noop(Context context) const { call(noop_, context); }
  void jr_clock(Context context, ump::utility::jr_clock const &clock) const { call(jr_clock_, context, clock); }
  void jr_timestamp(Context context, ump::utility::jr_timestamp const &ts) const { call(jr_timestamp_, context, ts); }
  void delta_clockstamp_tpqn(Context context, ump::utility::delta_clockstamp_tpqn const &time) const {
    call(delta_clockstamp_tpqn_, context, time);
  }
  void delta_clockstamp(Context context, ump::utility::delta_clockstamp const &time) const {
    call(delta_clockstamp_, context, time);
  }
  void unknown(Context context, std::span<std::uint32_t> data) const { call(unknown_, context, data); }

private:
  noop_fn noop_;
  jr_clock_fn jr_clock_;
  jr_timestamp_fn jr_timestamp_;
  delta_clockstamp_tpqn_fn delta_clockstamp_tpqn_;
  delta_clockstamp_fn delta_clockstamp_;
  unknown_fn unknown_;
};

static_assert(utility<utility_function<int>, int>, "utility_function must implement the utility concept");

template <typename Context> class system_function {
public:
  using midi_time_code_fn = std::function<void(Context, ump::system::midi_time_code const &)>;
  using song_position_pointer_fn = std::function<void(Context, ump::system::song_position_pointer const &)>;
  using song_select_fn = std::function<void(Context, ump::system::song_select const &)>;
  using tune_request_fn = std::function<void(Context, ump::system::tune_request const &)>;
  using timing_clock_fn = std::function<void(Context, ump::system::timing_clock const &)>;
  using seq_start_fn = std::function<void(Context, ump::system::sequence_start const &)>;
  using seq_continue_fn = std::function<void(Context, ump::system::sequence_continue const &)>;
  using seq_stop_fn = std::function<void(Context, ump::system::sequence_stop const &)>;
  using active_sensing_fn = std::function<void(Context, ump::system::active_sensing const &)>;
  using reset_fn = std::function<void(Context, ump::system::reset const &)>;

  constexpr system_function &on_midi_time_code(midi_time_code_fn midi_time_code) {
    midi_time_code_ = std::move(midi_time_code);
    return *this;
  }
  constexpr system_function &on_song_position_pointer(song_position_pointer_fn song_position_pointer) {
    song_position_pointer_ = std::move(song_position_pointer);
    return *this;
  }
  constexpr system_function &on_song_select(song_select_fn song_select) {
    song_select_ = std::move(song_select);
    return *this;
  }
  constexpr system_function &on_tune_request(tune_request_fn tune_request) {
    tune_request_ = std::move(tune_request);
    return *this;
  }
  constexpr system_function &on_timing_clock(timing_clock_fn timing_clock) {
    timing_clock_ = std::move(timing_clock);
    return *this;
  }
  constexpr system_function &on_seq_start(seq_start_fn seq_start) {
    seq_start_ = std::move(seq_start);
    return *this;
  }
  constexpr system_function &on_seq_continue(seq_continue_fn seq_continue) {
    seq_continue_ = std::move(seq_continue);
    return *this;
  }
  constexpr system_function &on_seq_stop(seq_stop_fn seq_stop) {
    seq_stop_ = std::move(seq_stop);
    return *this;
  }
  constexpr system_function &on_active_sensing(active_sensing_fn active_sensing) {
    active_sensing_ = std::move(active_sensing);
    return *this;
  }
  constexpr system_function &on_reset(reset_fn reset) {
    reset_ = std::move(reset);
    return *this;
  }

  // 7.6 System Common and System Real Time Messages
  void midi_time_code(Context context, ump::system::midi_time_code const &mtc) const {
    call(midi_time_code_, context, mtc);
  }
  void song_position_pointer(Context context, ump::system::song_position_pointer const &spp) const {
    call(song_position_pointer_, context, spp);
  }
  void song_select(Context context, ump::system::song_select const &song) const { call(song_select_, context, song); }
  void tune_request(Context context, ump::system::tune_request const &request) const {
    call(tune_request_, context, request);
  }
  void timing_clock(Context context, ump::system::timing_clock const &clock) const {
    call(timing_clock_, context, clock);
  }
  void seq_start(Context context, ump::system::sequence_start const &ss) const { call(seq_start_, context, ss); }
  void seq_continue(Context context, ump::system::sequence_continue const &sc) const {
    call(seq_continue_, context, sc);
  }
  void seq_stop(Context context, ump::system::sequence_stop const &ss) const { call(seq_stop_, context, ss); }
  void active_sensing(Context context, ump::system::active_sensing const &as) const {
    call(active_sensing_, context, as);
  }
  void reset(Context context, ump::system::reset const &r) const { call(reset_, context, r); }

private:
  midi_time_code_fn midi_time_code_;
  song_position_pointer_fn song_position_pointer_;
  song_select_fn song_select_;
  tune_request_fn tune_request_;
  timing_clock_fn timing_clock_;
  seq_start_fn seq_start_;
  seq_continue_fn seq_continue_;
  seq_stop_fn seq_stop_;
  active_sensing_fn active_sensing_;
  reset_fn reset_;
};

static_assert(system<system_function<int>, int>, "system_function must implement the system concept");

template <typename Context> class m1cvm_function {
public:
  using note_off_fn = std::function<void(Context, ump::m1cvm::note_off const &)>;
  using note_on_fn = std::function<void(Context, ump::m1cvm::note_on const &)>;
  using poly_pressure_fn = std::function<void(Context, ump::m1cvm::poly_pressure const &)>;
  using control_change_fn = std::function<void(Context, ump::m1cvm::control_change const &)>;
  using program_change_fn = std::function<void(Context, ump::m1cvm::program_change const &)>;
  using channel_pressure_fn = std::function<void(Context, ump::m1cvm::channel_pressure const &)>;
  using pitch_bend_fn = std::function<void(Context, ump::m1cvm::pitch_bend const &)>;

  constexpr m1cvm_function &on_note_off(note_off_fn note_off) noexcept {
    note_off_ = std::move(note_off);
    return *this;
  }
  constexpr m1cvm_function &on_note_on(note_on_fn note_on) noexcept {
    note_on_ = std::move(note_on);
    return *this;
  }
  constexpr m1cvm_function &on_poly_pressure(poly_pressure_fn poly_pressure) noexcept {
    poly_pressure_ = std::move(poly_pressure);
    return *this;
  }
  constexpr m1cvm_function &on_control_change(control_change_fn control_change) noexcept {
    control_change_ = std::move(control_change);
    return *this;
  }
  constexpr m1cvm_function &on_program_change(program_change_fn program_change) noexcept {
    program_change_ = std::move(program_change);
    return *this;
  }
  constexpr m1cvm_function &on_channel_pressure(channel_pressure_fn channel_pressure) noexcept {
    channel_pressure_ = std::move(channel_pressure);
    return *this;
  }
  constexpr m1cvm_function &on_pitch_bend(pitch_bend_fn pitch_bend) noexcept {
    pitch_bend_ = std::move(pitch_bend);
    return *this;
  }

  void note_off(Context context, ump::m1cvm::note_off const &noff) const { call(note_off_, context, noff); }
  void note_on(Context context, ump::m1cvm::note_on const &non) const { call(note_on_, context, non); }
  void poly_pressure(Context context, ump::m1cvm::poly_pressure const &pressure) const {
    call(poly_pressure_, context, pressure);
  }
  void control_change(Context context, ump::m1cvm::control_change const &cc) const {
    call(control_change_, context, cc);
  }
  void program_change(Context context, ump::m1cvm::program_change const &program) const {
    call(program_change_, context, program);
  }
  void channel_pressure(Context context, ump::m1cvm::channel_pressure const &pressure) const {
    call(channel_pressure_, context, pressure);
  }
  void pitch_bend(Context context, ump::m1cvm::pitch_bend const &bend) const { call(pitch_bend_, context, bend); }

private:
  note_off_fn note_off_;
  note_on_fn note_on_;
  poly_pressure_fn poly_pressure_;
  control_change_fn control_change_;
  program_change_fn program_change_;
  channel_pressure_fn channel_pressure_;
  pitch_bend_fn pitch_bend_;
};

static_assert(m1cvm<m1cvm_function<int>, int>, "m1cvm_function must implement the m1cvm concept");

template <typename Context> class data64_function {
public:
  using sysex7_in_1_fn = std::function<void(Context, ump::data64::sysex7_in_1 const &)>;
  using sysex7_start_fn = std::function<void(Context, ump::data64::sysex7_start const &)>;
  using sysex7_continue_fn = std::function<void(Context, ump::data64::sysex7_continue const &)>;
  using sysex7_end_fn = std::function<void(Context, ump::data64::sysex7_end const &)>;

  constexpr data64_function &on_sysex7_in_1(sysex7_in_1_fn sysex7_in_1) noexcept {
    sysex7_in_1_ = std::move(sysex7_in_1);
    return *this;
  }
  constexpr data64_function &on_sysex7_start(sysex7_start_fn sysex7_start) noexcept {
    sysex7_start_ = std::move(sysex7_start);
    return *this;
  }
  constexpr data64_function &on_sysex7_continue(sysex7_continue_fn sysex7_continue) noexcept {
    sysex7_continue_ = std::move(sysex7_continue);
    return *this;
  }
  constexpr data64_function &on_sysex7_end(sysex7_end_fn sysex7_end) noexcept {
    sysex7_end_ = std::move(sysex7_end);
    return *this;
  }

  void sysex7_in_1(Context context, ump::data64::sysex7_in_1 const &sx) const { call(sysex7_in_1_, context, sx); }
  void sysex7_start(Context context, ump::data64::sysex7_start const &sx) const { call(sysex7_start_, context, sx); }
  void sysex7_continue(Context context, ump::data64::sysex7_continue const &sx) const {
    call(sysex7_continue_, context, sx);
  }
  void sysex7_end(Context context, ump::data64::sysex7_end const &sx) const { call(sysex7_end_, context, sx); }

private:
  sysex7_in_1_fn sysex7_in_1_;
  sysex7_start_fn sysex7_start_;
  sysex7_continue_fn sysex7_continue_;
  sysex7_end_fn sysex7_end_;
};

static_assert(data64<data64_function<int>, int>, "data64_function must implement the data64 concept");

template <typename Context> class m2cvm_function {
public:
  using note_off_fn = std::function<void(Context, ump::m2cvm::note_off const &)>;
  using note_on_fn = std::function<void(Context, ump::m2cvm::note_on const &)>;
  using poly_pressure_fn = std::function<void(Context, ump::m2cvm::poly_pressure const &)>;
  using program_change_fn = std::function<void(Context, ump::m2cvm::program_change const &)>;
  using channel_pressure_fn = std::function<void(Context, ump::m2cvm::channel_pressure const &)>;
  using rpn_per_note_controller_fn = std::function<void(Context, ump::m2cvm::rpn_per_note_controller const &)>;
  using nrpn_per_note_controller_fn = std::function<void(Context, ump::m2cvm::nrpn_per_note_controller const &)>;
  using rpn_controller_fn = std::function<void(Context, ump::m2cvm::rpn_controller const &)>;
  using nrpn_controller_fn = std::function<void(Context, ump::m2cvm::nrpn_controller const &)>;
  using rpn_relative_controller_fn = std::function<void(Context, ump::m2cvm::rpn_relative_controller const &)>;
  using nrpn_relative_controller_fn = std::function<void(Context, ump::m2cvm::nrpn_relative_controller const &)>;
  using per_note_management_fn = std::function<void(Context, ump::m2cvm::per_note_management const &)>;
  using control_change_fn = std::function<void(Context, ump::m2cvm::control_change const &)>;
  using pitch_bend_fn = std::function<void(Context, ump::m2cvm::pitch_bend const &)>;
  using per_note_pitch_bend_fn = std::function<void(Context, ump::m2cvm::per_note_pitch_bend const &)>;

  constexpr m2cvm_function &on_note_off(note_off_fn noff) noexcept {
    note_off_ = std::move(noff);
    return *this;
  }
  constexpr m2cvm_function &on_note_on(note_on_fn non) noexcept {
    note_on_ = std::move(non);
    return *this;
  }
  constexpr m2cvm_function &on_poly_pressure(poly_pressure_fn pressure) noexcept {
    poly_pressure_ = std::move(pressure);
    return *this;
  }
  constexpr m2cvm_function &on_program_change(program_change_fn program_change) noexcept {
    program_change_ = std::move(program_change);
    return *this;
  }
  constexpr m2cvm_function &on_channel_pressure(channel_pressure_fn pressure) noexcept {
    channel_pressure_ = std::move(pressure);
    return *this;
  }
  constexpr m2cvm_function &on_rpn_per_note_controller(rpn_per_note_controller_fn controller) noexcept {
    rpn_per_note_controller_ = std::move(controller);
    return *this;
  }
  constexpr m2cvm_function &on_nrpn_per_note_controller(nrpn_per_note_controller_fn controller) noexcept {
    nrpn_per_note_controller_ = std::move(controller);
    return *this;
  }
  constexpr m2cvm_function &on_rpn_controller(rpn_controller_fn controller) noexcept {
    rpn_controller_ = std::move(controller);
    return *this;
  }
  constexpr m2cvm_function &on_nrpn_controller(nrpn_controller_fn controller) noexcept {
    nrpn_controller_ = std::move(controller);
    return *this;
  }
  constexpr m2cvm_function &on_rpn_relative_controller(rpn_relative_controller_fn controller) noexcept {
    rpn_relative_controller_ = std::move(controller);
    return *this;
  }
  constexpr m2cvm_function &on_nrpn_relative_controller(nrpn_relative_controller_fn controller) noexcept {
    nrpn_relative_controller_ = std::move(controller);
    return *this;
  }
  constexpr m2cvm_function &on_per_note_management(per_note_management_fn per_note_management) noexcept {
    per_note_management_ = std::move(per_note_management);
    return *this;
  }
  constexpr m2cvm_function &on_control_change(control_change_fn cc) noexcept {
    control_change_ = std::move(cc);
    return *this;
  }
  constexpr m2cvm_function &on_pitch_bend(pitch_bend_fn pitch_bend) noexcept {
    pitch_bend_ = std::move(pitch_bend);
    return *this;
  }
  constexpr m2cvm_function &on_per_note_pitch_bend(per_note_pitch_bend_fn per_note_pitch_bend) noexcept {
    per_note_pitch_bend_ = std::move(per_note_pitch_bend);
    return *this;
  }

  void note_off(Context context, ump::m2cvm::note_off const &noff) const { call(note_off_, context, noff); }
  void note_on(Context context, ump::m2cvm::note_on const &non) const { call(note_on_, context, non); }
  void poly_pressure(Context context, ump::m2cvm::poly_pressure const &pressure) const {
    call(poly_pressure_, context, pressure);
  }
  void program_change(Context context, ump::m2cvm::program_change const &program) const {
    call(program_change_, context, program);
  }
  void channel_pressure(Context context, ump::m2cvm::channel_pressure const &pressure) const {
    call(channel_pressure_, context, pressure);
  }
  void rpn_per_note_controller(Context context, ump::m2cvm::rpn_per_note_controller const &rpn) const {
    call(rpn_per_note_controller_, context, rpn);
  }
  void nrpn_per_note_controller(Context context, ump::m2cvm::nrpn_per_note_controller const &nrpn) const {
    call(nrpn_per_note_controller_, context, nrpn);
  }
  void rpn_controller(Context context, ump::m2cvm::rpn_controller const &rpn) const {
    call(rpn_controller_, context, rpn);
  }
  void nrpn_controller(Context context, ump::m2cvm::nrpn_controller const &nrpn) const {
    call(nrpn_controller_, context, nrpn);
  }
  void rpn_relative_controller(Context context, ump::m2cvm::rpn_relative_controller const &rpn) const {
    call(rpn_relative_controller_, context, rpn);
  }
  void nrpn_relative_controller(Context context, ump::m2cvm::nrpn_relative_controller const &nrpn) const {
    call(nrpn_relative_controller_, context, nrpn);
  }
  void per_note_management(Context context, ump::m2cvm::per_note_management const &pnm) const {
    call(per_note_management_, context, pnm);
  }
  void control_change(Context context, ump::m2cvm::control_change const &cc) const {
    call(control_change_, context, cc);
  }
  void pitch_bend(Context context, ump::m2cvm::pitch_bend const &pb) const { call(pitch_bend_, context, pb); }
  void per_note_pitch_bend(Context context, ump::m2cvm::per_note_pitch_bend const &pb) const {
    call(per_note_pitch_bend_, context, pb);
  }

private:
  note_off_fn note_off_;
  note_on_fn note_on_;
  poly_pressure_fn poly_pressure_;
  program_change_fn program_change_;
  channel_pressure_fn channel_pressure_;
  rpn_per_note_controller_fn rpn_per_note_controller_;
  nrpn_per_note_controller_fn nrpn_per_note_controller_;
  rpn_controller_fn rpn_controller_;
  nrpn_controller_fn nrpn_controller_;
  rpn_relative_controller_fn rpn_relative_controller_;
  nrpn_relative_controller_fn nrpn_relative_controller_;
  per_note_management_fn per_note_management_;
  control_change_fn control_change_;
  pitch_bend_fn pitch_bend_;
  per_note_pitch_bend_fn per_note_pitch_bend_;
};

static_assert(m2cvm<m2cvm_function<int>, int>, "m2cvm_function must implement the m2cvm concept");

template <typename Context> class data128_function {
public:
  using sysex8_in_1_fn = std::function<void(Context, ump::data128::sysex8_in_1 const &)>;
  using sysex8_start_fn = std::function<void(Context, ump::data128::sysex8_start const &)>;
  using sysex8_continue_fn = std::function<void(Context, ump::data128::sysex8_continue const &)>;
  using sysex8_end_fn = std::function<void(Context, ump::data128::sysex8_end const &)>;
  using mds_header_fn = std::function<void(Context, ump::data128::mds_header const &)>;
  using mds_payload_fn = std::function<void(Context, ump::data128::mds_payload const &)>;

  constexpr data128_function &on_sysex8_in_1(sysex8_in_1_fn sysex8_in_1) noexcept {
    sysex8_in_1_ = std::move(sysex8_in_1);
    return *this;
  }
  constexpr data128_function &on_sysex8_start(sysex8_start_fn sysex8_start) noexcept {
    sysex8_start_ = std::move(sysex8_start);
    return *this;
  }
  constexpr data128_function &on_sysex8_continue(sysex8_continue_fn sysex8_continue) noexcept {
    sysex8_continue_ = std::move(sysex8_continue);
    return *this;
  }
  constexpr data128_function &on_sysex8_end(sysex8_end_fn sysex8_end) noexcept {
    sysex8_end_ = std::move(sysex8_end);
    return *this;
  }
  constexpr data128_function &on_mds_header(mds_header_fn mds_header) noexcept {
    mds_header_ = std::move(mds_header);
    return *this;
  }
  constexpr data128_function &on_mds_payload(mds_payload_fn mds_payload) noexcept {
    mds_payload_ = std::move(mds_payload);
    return *this;
  }

  void sysex8_in_1(Context context, ump::data128::sysex8_in_1 const &sysex) const {
    call(sysex8_in_1_, context, sysex);
  }
  void sysex8_start(Context context, ump::data128::sysex8_start const &sysex) const {
    call(sysex8_start_, context, sysex);
  }
  void sysex8_continue(Context context, ump::data128::sysex8_continue const &sysex) const {
    call(sysex8_continue_, context, sysex);
  }
  void sysex8_end(Context context, ump::data128::sysex8_end const &sysex) const { call(sysex8_end_, context, sysex); }
  void mds_header(Context context, ump::data128::mds_header const &mds) const { call(mds_header_, context, mds); }
  void mds_payload(Context context, ump::data128::mds_payload const &mds) const { call(mds_payload_, context, mds); }

private:
  sysex8_in_1_fn sysex8_in_1_;
  sysex8_start_fn sysex8_start_;
  sysex8_continue_fn sysex8_continue_;
  sysex8_end_fn sysex8_end_;
  mds_header_fn mds_header_;
  mds_payload_fn mds_payload_;
};

static_assert(data128<data128_function<int>, int>, "data128_function must implement the data128 concept");

template <typename Context> class stream_function {
public:
  using endpoint_discovery_fn = std::function<void(Context, ump::stream::endpoint_discovery const &)>;
  using endpoint_info_notification_fn = std::function<void(Context, ump::stream::endpoint_info_notification const &)>;
  using device_identity_notification_fn =
      std::function<void(Context, ump::stream::device_identity_notification const &)>;
  using endpoint_name_notification_fn = std::function<void(Context, ump::stream::endpoint_name_notification const &)>;
  using product_instance_id_notification_fn =
      std::function<void(Context, ump::stream::product_instance_id_notification const &)>;
  using jr_configuration_request_fn = std::function<void(Context, ump::stream::jr_configuration_request const &)>;
  using jr_configuration_notification_fn =
      std::function<void(Context, ump::stream::jr_configuration_notification const &)>;
  using function_block_discovery_fn = std::function<void(Context, ump::stream::function_block_discovery const &)>;
  using function_block_info_notification_fn =
      std::function<void(Context, ump::stream::function_block_info_notification const &)>;
  using function_block_name_notification_fn =
      std::function<void(Context, ump::stream::function_block_name_notification const &)>;
  using start_of_clip_fn = std::function<void(Context, ump::stream::start_of_clip const &)>;
  using end_of_clip_fn = std::function<void(Context, ump::stream::end_of_clip const &)>;

  constexpr stream_function &on_endpoint_discovery(endpoint_discovery_fn discovery) noexcept {
    endpoint_discovery_ = std::move(discovery);
    return *this;
  }
  constexpr stream_function &on_endpoint_info_notification(endpoint_info_notification_fn notification) noexcept {
    endpoint_info_notification_ = std::move(notification);
    return *this;
  }
  constexpr stream_function &on_device_identity_notification(device_identity_notification_fn notification) noexcept {
    device_identity_notification_ = std::move(notification);
    return *this;
  }
  constexpr stream_function &on_endpoint_name_notification(endpoint_name_notification_fn notification) noexcept {
    endpoint_name_notification_ = std::move(notification);
    return *this;
  }
  constexpr stream_function &on_product_instance_id_notification(
      product_instance_id_notification_fn notification) noexcept {
    product_instance_id_notification_ = std::move(notification);
    return *this;
  }
  constexpr stream_function &on_jr_configuration_request(jr_configuration_request_fn request) noexcept {
    jr_configuration_request_ = std::move(request);
    return *this;
  }
  constexpr stream_function &on_jr_configuration_notification(jr_configuration_notification_fn notification) noexcept {
    jr_configuration_notification_ = std::move(notification);
    return *this;
  }
  constexpr stream_function &on_function_block_discovery(function_block_discovery_fn discovery) noexcept {
    function_block_discovery_ = std::move(discovery);
    return *this;
  }
  constexpr stream_function &on_function_block_info_notification(
      function_block_info_notification_fn notification) noexcept {
    function_block_info_notification_ = std::move(notification);
    return *this;
  }
  constexpr stream_function &on_function_block_name_notification(
      function_block_name_notification_fn notification) noexcept {
    function_block_name_notification_ = std::move(notification);
    return *this;
  }
  constexpr stream_function &on_start_of_clip(start_of_clip_fn clip) noexcept {
    start_of_clip_ = std::move(clip);
    return *this;
  }
  constexpr stream_function &on_end_of_clip(end_of_clip_fn clip) noexcept {
    end_of_clip_ = std::move(clip);
    return *this;
  }

  void endpoint_discovery(Context context, ump::stream::endpoint_discovery const &discovery) const {
    call(endpoint_discovery_, context, discovery);
  }
  void endpoint_info_notification(Context context, ump::stream::endpoint_info_notification const &notification) const {
    call(endpoint_info_notification_, context, notification);
  }
  void device_identity_notification(Context context, ump::stream::device_identity_notification const &notification) const {
    call(device_identity_notification_, context, notification);
  }
  void endpoint_name_notification(Context context, ump::stream::endpoint_name_notification const &notification) const {
    call(endpoint_name_notification_, context, notification);
  }
  void product_instance_id_notification(Context context,
                                        ump::stream::product_instance_id_notification const &notification) const {
    call(product_instance_id_notification_, context, notification);
  }
  void jr_configuration_request(Context context, ump::stream::jr_configuration_request const &request) const {
    call(jr_configuration_request_, context, request);
  }
  void jr_configuration_notification(Context context, ump::stream::jr_configuration_notification const &notification) const {
    call(jr_configuration_notification_, context, notification);
  }
  void function_block_discovery(Context context, ump::stream::function_block_discovery const &discovery) const {
    call(function_block_discovery_, context, discovery);
  }
  void function_block_info_notification(Context context,
                                        ump::stream::function_block_info_notification const &notification) const {
    call(function_block_info_notification_, context, notification);
  }
  void function_block_name_notification(Context context,
                                        ump::stream::function_block_name_notification const &notification) const {
    call(function_block_name_notification_, context, notification);
  }
  void start_of_clip(Context context, ump::stream::start_of_clip const &clip) const {
    call(start_of_clip_, context, clip);
  }
  void end_of_clip(Context context, ump::stream::end_of_clip const &clip) const { call(end_of_clip_, context, clip); }

private:
  endpoint_discovery_fn endpoint_discovery_;
  endpoint_info_notification_fn endpoint_info_notification_;
  device_identity_notification_fn device_identity_notification_;
  endpoint_name_notification_fn endpoint_name_notification_;
  product_instance_id_notification_fn product_instance_id_notification_;
  jr_configuration_request_fn jr_configuration_request_;
  jr_configuration_notification_fn jr_configuration_notification_;

  function_block_discovery_fn function_block_discovery_;
  function_block_info_notification_fn function_block_info_notification_;
  function_block_name_notification_fn function_block_name_notification_;

  start_of_clip_fn start_of_clip_;
  end_of_clip_fn end_of_clip_;
};

static_assert(stream<stream_function<int>, int>, "stream_function must implement the stream concept");

template <typename Context> class flex_data_function {
public:
  using set_tempo_fn = std::function<void(Context, ump::flex_data::set_tempo const &)>;
  using set_time_signature_fn = std::function<void(Context, ump::flex_data::set_time_signature const &)>;
  using set_metronome_fn = std::function<void(Context, ump::flex_data::set_metronome const &)>;
  using set_key_signature_fn = std::function<void(Context, ump::flex_data::set_key_signature const &)>;
  using set_chord_name_fn = std::function<void(Context, ump::flex_data::set_chord_name const &)>;
  using text_fn = std::function<void(Context, ump::flex_data::text_common const &)>;

  constexpr flex_data_function &on_set_tempo(set_tempo_fn set_tempo) noexcept {
    set_tempo_ = std::move(set_tempo);
    return *this;
  }
  constexpr flex_data_function &on_set_time_signature(set_time_signature_fn set_time_signature) noexcept {
    set_time_signature_ = std::move(set_time_signature);
    return *this;
  }
  constexpr flex_data_function &on_set_metronome(set_metronome_fn set_metronome) noexcept {
    set_metronome_ = std::move(set_metronome);
    return *this;
  }
  constexpr flex_data_function &on_set_key_signature(set_key_signature_fn set_key_signature) noexcept {
    set_key_signature_ = std::move(set_key_signature);
    return *this;
  }
  constexpr flex_data_function &on_set_chord_name(set_chord_name_fn set_chord_name) noexcept {
    set_chord_name_ = std::move(set_chord_name);
    return *this;
  }
  constexpr flex_data_function &on_text(text_fn text) noexcept {
    text_ = std::move(text);
    return *this;
  }

  void set_tempo(Context context, ump::flex_data::set_tempo const &tempo) const { call(set_tempo_, context, tempo); }
  void set_time_signature(Context context, ump::flex_data::set_time_signature const &ts) const {
    call(set_time_signature_, context, ts);
  }
  void set_metronome(Context context, ump::flex_data::set_metronome const &metronome) const {
    call(set_metronome_, context, metronome);
  }
  void set_key_signature(Context context, ump::flex_data::set_key_signature const &ks) const {
    call(set_key_signature_, context, ks);
  }
  void set_chord_name(Context context, ump::flex_data::set_chord_name const &chord) const {
    call(set_chord_name_, context, chord);
  }
  void text(Context context, ump::flex_data::text_common const &t) const { call(text_, context, t); }

private:
  set_tempo_fn set_tempo_;
  set_time_signature_fn set_time_signature_;
  set_metronome_fn set_metronome_;
  set_key_signature_fn set_key_signature_;
  set_chord_name_fn set_chord_name_;
  text_fn text_;
};

static_assert(flex_data<flex_data_function<int>, int>, "flex_data_function must implement the flex_data concept");

}  // end namespace midi2::ump::dispatcher_backend

#endif  // MIDI2_UMP_DISPATCHER_BACKEND_HPP
