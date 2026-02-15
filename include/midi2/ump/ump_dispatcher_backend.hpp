//===-- UMP Dispatcher Backends -----------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ump_dispatcher_backend.hpp
/// \brief Contains concepts and types for the UMP dispatcher

#ifndef MIDI2_UMP_DISPATCHER_BACKEND_HPP
#define MIDI2_UMP_DISPATCHER_BACKEND_HPP

#include <functional>

#include "midi2/ump/ump_types.hpp"
#include "midi2/utils.hpp"

/// \brief Concepts and types for the UMP dispatcher
namespace midi2::ump::dispatcher_backend {

// clang-format off
template <typename T, typename Context>
concept utility = requires(T v, Context context) {
  // 7.2.1 NOOP
  { v.noop(context) } -> std::same_as<void>;
  // 7.2.2.1 JR Clock Message
  { v.jr_clock(context, utility::jr_clock{}) } -> std::same_as<void>;
  // 7.2.2.2 JR Timestamp Message
  { v.jr_timestamp(context, utility::jr_timestamp{}) } -> std::same_as<void>;
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (TPQN)
  { v.delta_clockstamp_tpqn(context, utility::delta_clockstamp_tpqn{}) } -> std::same_as<void>;
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  { v.delta_clockstamp(context, utility::delta_clockstamp{}) } -> std::same_as<void>;

  { v.unknown(context, std::span<std::uint32_t>{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept system = requires(T v, Context context) {
  // 7.6 System Common and System Real Time Messages
  { v.midi_time_code(context, system::midi_time_code{}) } -> std::same_as<void>;
  { v.song_position_pointer(context, system::song_position_pointer{}) } -> std::same_as<void>;
  { v.song_select(context, system::song_select{}) } -> std::same_as<void>;
  { v.tune_request(context, system::tune_request{}) } -> std::same_as<void>;
  { v.timing_clock(context, system::timing_clock{}) } -> std::same_as<void>;
  { v.seq_start(context, system::sequence_start{}) } -> std::same_as<void>;
  { v.seq_continue(context, system::sequence_continue{}) } -> std::same_as<void>;
  { v.seq_stop(context, system::sequence_stop{}) } -> std::same_as<void>;
  { v.active_sensing(context, system::active_sensing{}) } -> std::same_as<void>;
  { v.reset(context, system::reset{}) } -> std::same_as<void>;
};
template<typename T, typename Context>
concept m1cvm = requires(T v, Context context) {
  { v.note_off(context, m1cvm::note_off{}) } -> std::same_as<void>;
  { v.note_on(context, m1cvm::note_on{}) } -> std::same_as<void>;
  { v.poly_pressure(context, m1cvm::poly_pressure{}) } -> std::same_as<void>;
  { v.control_change(context, m1cvm::control_change{}) } -> std::same_as<void>;
  { v.program_change(context, m1cvm::program_change{}) } -> std::same_as<void>;
  { v.channel_pressure(context, m1cvm::channel_pressure{}) } -> std::same_as<void>;
  { v.pitch_bend(context, m1cvm::pitch_bend{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept data64 = requires(T v, Context context) {
  { v.sysex7_in_1(context, data64::sysex7_in_1{}) } -> std::same_as<void>;
  { v.sysex7_start(context, data64::sysex7_start{}) } -> std::same_as<void>;
  { v.sysex7_continue(context, data64::sysex7_continue{}) } -> std::same_as<void>;
  { v.sysex7_end(context, data64::sysex7_end{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept m2cvm = requires(T v, Context context) {
  // 7.4.1 MIDI 2.0 Note Off Message (status=0x8)
  { v.note_off(context, m2cvm::note_off{}) } -> std::same_as<void>;
  // 7.4.2 MIDI 2.0 Note On Message (status=0x9)
  { v.note_on(context, m2cvm::note_on{}) } -> std::same_as<void>;
  // 7.4.3 MIDI 2.0 Poly Pressure Message (status=0xA)
  { v.poly_pressure(context, m2cvm::poly_pressure{}) } -> std::same_as<void>;

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  { v.rpn_per_note_controller(context, m2cvm::rpn_per_note_controller{}) } -> std::same_as<void>;
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  { v.nrpn_per_note_controller(context, m2cvm::nrpn_per_note_controller{}) } -> std::same_as<void>;
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  { v.rpn_controller(context, m2cvm::rpn_controller{}) } -> std::same_as<void>;
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  { v.nrpn_controller(context, m2cvm::nrpn_controller{}) } -> std::same_as<void>;
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  { v.rpn_relative_controller(context, m2cvm::rpn_relative_controller{}) } -> std::same_as<void>;
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  { v.nrpn_relative_controller(context, m2cvm::nrpn_relative_controller{}) } -> std::same_as<void>;

  // 7.4.9 MIDI 2.0 Program Change Message (status=0xC)
  { v.program_change(context, m2cvm::program_change{}) } -> std::same_as<void>;
  // 7.4.10 MIDI 2.0 Channel Pressure Message (status=0xD)
  { v.channel_pressure(context, m2cvm::channel_pressure{}) } -> std::same_as<void>;

  { v.per_note_management(context, m2cvm::per_note_management{}) } -> std::same_as<void>;
  { v.control_change(context, m2cvm::control_change{}) } -> std::same_as<void>;

  { v.pitch_bend(context, m2cvm::pitch_bend{}) } -> std::same_as<void>;
  { v.per_note_pitch_bend(context, m2cvm::per_note_pitch_bend{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept data128 = requires(T v, Context context) {
  // 7.8 System Exclusive 8 (8-Bit) Messages
  { v.sysex8_in_1(context, data128::sysex8_in_1{}) } -> std::same_as<void>;
  { v.sysex8_start(context, data128::sysex8_start{}) } -> std::same_as<void>;
  { v.sysex8_continue(context, data128::sysex8_continue{}) } -> std::same_as<void>;
  { v.sysex8_end(context, data128::sysex8_end{}) } -> std::same_as<void>;
  // 7.9 Mixed Data Set Message
  { v.mds_header(context, data128::mds_header{}) } -> std::same_as<void>;
  { v.mds_payload(context, data128::mds_payload{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept stream = requires(T v, Context context) {
  { v.endpoint_discovery(context, stream::endpoint_discovery{}) } -> std::same_as<void>;
  { v.endpoint_info_notification(context, stream::endpoint_info_notification{}) } -> std::same_as<void>;
  { v.device_identity_notification(context, stream::device_identity_notification{}) } -> std::same_as<void>;
  { v.endpoint_name_notification(context, stream::endpoint_name_notification{}) } -> std::same_as<void>;
  { v.product_instance_id_notification(context, stream::product_instance_id_notification{}) } -> std::same_as<void>;
  { v.jr_configuration_request(context, stream::jr_configuration_request{}) } -> std::same_as<void>;
  { v.jr_configuration_notification(context, stream::jr_configuration_notification{}) } -> std::same_as<void>;
  { v.function_block_discovery(context, stream::function_block_discovery{}) } -> std::same_as<void>;
  { v.function_block_info_notification(context, stream::function_block_info_notification{}) } -> std::same_as<void>;
  { v.function_block_name_notification(context, stream::function_block_name_notification{}) } -> std::same_as<void>;
  { v.start_of_clip(context, stream::start_of_clip{}) } -> std::same_as<void>;
  { v.end_of_clip(context, stream::end_of_clip{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept flex_data = requires(T v, Context context) {
  { v.set_tempo(context, flex_data::set_tempo{}) } -> std::same_as<void>;
  { v.set_time_signature(context, flex_data::set_time_signature{}) } -> std::same_as<void>;
  { v.set_metronome(context, flex_data::set_metronome{}) } -> std::same_as<void>;
  { v.set_key_signature(context, flex_data::set_key_signature{}) } -> std::same_as<void>;
  { v.set_chord_name(context, flex_data::set_chord_name{}) } -> std::same_as<void>;
  { v.text(context, flex_data::text_common{}) } -> std::same_as<void>;
};
// clang-format on

// clang-format off
template <typename Context> struct utility_null {
  // 7.2.1 NOOP
  constexpr static void noop(Context) noexcept { /* do nothing */ }
  // 7.2.2.1 JR Clock Message
  constexpr static void jr_clock(Context, utility::jr_clock const &) noexcept { /* do nothing */ }
  // 7.2.2.2 JR Timestamp Message
  constexpr static void jr_timestamp(Context, utility::jr_timestamp const &) noexcept { /* do nothing */ }
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (TPQN)
  constexpr static void delta_clockstamp_tpqn(Context, utility::delta_clockstamp_tpqn const &) noexcept { /* do nothing */ }
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  constexpr static void delta_clockstamp(Context, utility::delta_clockstamp const &) noexcept { /* do nothing */ }

  constexpr static void unknown(Context, std::span<std::uint32_t>) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(utility<utility_null<int>, int>, "utility_null must implement the utility concept");

// clang-format off
template <typename Context> struct system_null {
  // 7.6 System Common and System Real Time Messages
  constexpr static void midi_time_code(Context, system::midi_time_code const &) noexcept { /* do nothing */ }
  constexpr static void song_position_pointer(Context, system::song_position_pointer const &) noexcept { /* do nothing */ }
  constexpr static void song_select(Context, system::song_select const &) noexcept { /* do nothing */ }
  constexpr static void tune_request(Context, system::tune_request const &) noexcept { /* do nothing */ }
  constexpr static void timing_clock(Context, system::timing_clock const &) noexcept { /* do nothing */ }
  constexpr static void seq_start(Context, system::sequence_start const &) noexcept { /* do nothing */ }
  constexpr static void seq_continue(Context, system::sequence_continue const &) noexcept { /* do nothing */ }
  constexpr static void seq_stop(Context, system::sequence_stop const &) noexcept { /* do nothing */ }
  constexpr static void active_sensing(Context, system::active_sensing const &) noexcept { /* do nothing */ }
  constexpr static void reset(Context, system::reset const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(system<system_null<int>, int>, "system_null must implement the system concept");

// clang-format off
template <typename Context> struct m1cvm_null {
  constexpr static void note_off(Context, m1cvm::note_off const &) noexcept { /* do nothing */ }
  constexpr static void note_on(Context, m1cvm::note_on const &) noexcept { /* do nothing */ }
  constexpr static void poly_pressure(Context, m1cvm::poly_pressure const &) noexcept { /* do nothing */ }
  constexpr static void control_change(Context, m1cvm::control_change const &) noexcept { /* do nothing */ }
  constexpr static void program_change(Context, m1cvm::program_change const &) noexcept { /* do nothing */ }
  constexpr static void channel_pressure(Context, m1cvm::channel_pressure const &) noexcept { /* do nothing */ }
  constexpr static void pitch_bend(Context, m1cvm::pitch_bend const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(m1cvm<m1cvm_null<int>, int>, "m1cvm_null must implement the m1cvm concept");

// clang-format off
template <typename Context> struct data64_null {
  constexpr static void sysex7_in_1(Context, data64::sysex7_in_1 const &) noexcept { /* do nothing */ }
  constexpr static void sysex7_start(Context, data64::sysex7_start const &) noexcept { /* do nothing */ }
  constexpr static void sysex7_continue(Context, data64::sysex7_continue const &) noexcept { /* do nothing */ }
  constexpr static void sysex7_end(Context, data64::sysex7_end const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(data64<data64_null<int>, int>, "data64_null must implement the data64 concept");

// clang-format off
template <typename Context> struct m2cvm_null {
  constexpr static void note_off(Context, m2cvm::note_off const &) noexcept { /* do nothing */ }
  constexpr static void note_on(Context, m2cvm::note_on const &) noexcept { /* do nothing */ }
  constexpr static void poly_pressure(Context, m2cvm::poly_pressure const &) noexcept { /* do nothing */ }
  constexpr static void program_change(Context, m2cvm::program_change const &) noexcept { /* do nothing */ }
  constexpr static void channel_pressure(Context, m2cvm::channel_pressure const &) noexcept { /* do nothing */ }
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  constexpr static void rpn_per_note_controller(Context, m2cvm::rpn_per_note_controller const &) noexcept { /* do nothing */ }
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  constexpr static void nrpn_per_note_controller(Context, m2cvm::nrpn_per_note_controller const &) noexcept { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  constexpr static void rpn_controller(Context, m2cvm::rpn_controller const &) noexcept { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  constexpr static void nrpn_controller(Context, m2cvm::nrpn_controller const &) noexcept { /* do nothing */ }
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  constexpr static void rpn_relative_controller(Context, m2cvm::rpn_relative_controller const &) noexcept { /* do nothing */ }
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  constexpr static void nrpn_relative_controller(Context, m2cvm::nrpn_relative_controller const &) noexcept { /* do nothing */ }
  constexpr static void per_note_management(Context, m2cvm::per_note_management const &) noexcept { /* do nothing */ }
  constexpr static void control_change(Context, m2cvm::control_change const &) noexcept { /* do nothing */ }
  constexpr static void pitch_bend(Context, m2cvm::pitch_bend const &) noexcept { /* do nothing */ }
  constexpr static void per_note_pitch_bend(Context, m2cvm::per_note_pitch_bend const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(m2cvm<m2cvm_null<int>, int>, "m2cvm_null must implement the m2cvm concept");

// clang-format off
template <typename Context> struct data128_null {
  constexpr static void sysex8_in_1(Context, data128::sysex8_in_1 const &) noexcept { /* do nothing */ }
  constexpr static void sysex8_start(Context, data128::sysex8_start const &) noexcept { /* do nothing */ }
  constexpr static void sysex8_continue(Context, data128::sysex8_continue const &) noexcept { /* do nothing */ }
  constexpr static void sysex8_end(Context, data128::sysex8_end const &) noexcept { /* do nothing */ }
  constexpr static void mds_header(Context, data128::mds_header const &) noexcept { /* do nothing */ }
  constexpr static void mds_payload(Context, data128::mds_payload const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(data128<data128_null<int>, int>, "data128_null must implement the data128 concept");

// clang-format off
template <typename Context> struct stream_null {
  constexpr static void endpoint_discovery(Context, stream::endpoint_discovery const &) noexcept { /* do nothing */ }
  constexpr static void endpoint_info_notification(Context, stream::endpoint_info_notification const &) noexcept { /* do nothing */ }
  constexpr static void device_identity_notification(Context, stream::device_identity_notification const &) noexcept { /* do nothing */ }
  constexpr static void endpoint_name_notification(Context, stream::endpoint_name_notification const &) noexcept { /* do nothing */ }
  constexpr static void product_instance_id_notification(Context, stream::product_instance_id_notification const &) noexcept { /* do nothing */ }
  constexpr static void jr_configuration_request(Context, stream::jr_configuration_request const &) noexcept { /* do nothing */ }
  constexpr static void jr_configuration_notification(Context, stream::jr_configuration_notification const &) noexcept { /* do nothing */ }

  constexpr static void function_block_discovery(Context, stream::function_block_discovery const &) noexcept { /* do nothing */ }
  constexpr static void function_block_info_notification(Context, stream::function_block_info_notification const &) noexcept { /* do nothing */ }
  constexpr static void function_block_name_notification(Context, stream::function_block_name_notification const &) noexcept { /* do nothing */ }

  constexpr static void start_of_clip(Context, stream::start_of_clip const &) noexcept { /* do nothing */ }
  constexpr static void end_of_clip(Context, stream::end_of_clip const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(stream<stream_null<int>, int>, "stream_null must implement the stream concept");

// clang-format off
template <typename Context> struct flex_data_null {
  constexpr static void set_tempo(Context, flex_data::set_tempo const &) noexcept { /* do nothing */ }
  constexpr static void set_time_signature(Context, flex_data::set_time_signature const &) noexcept { /* do nothing */ }
  constexpr static void set_metronome(Context, flex_data::set_metronome const &) noexcept { /* do nothing */ }
  constexpr static void set_key_signature(Context, flex_data::set_key_signature const &) noexcept { /* do nothing */ }
  constexpr static void set_chord_name(Context, flex_data::set_chord_name const &) noexcept { /* do nothing */ }
  constexpr static void text(Context, flex_data::text_common const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(flex_data<flex_data_null<int>, int>, "flex_data_null must implement the flex_data concept");

template <typename Context> struct utility_pure {
  constexpr utility_pure() noexcept = default;
  constexpr utility_pure(utility_pure const &) = default;
  constexpr utility_pure(utility_pure &&) noexcept = default;
  virtual ~utility_pure() noexcept = default;

  constexpr utility_pure &operator=(utility_pure const &) = default;
  constexpr utility_pure &operator=(utility_pure &&) noexcept = default;

  // 7.2.1 NOOP
  virtual void noop(Context) = 0;
  // 7.2.2.1 JR Clock Message
  virtual void jr_clock(Context, utility::jr_clock const &) = 0;
  // 7.2.2.2 JR Timestamp Message
  virtual void jr_timestamp(Context, utility::jr_timestamp const &) = 0;
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (TPQN)
  virtual void delta_clockstamp_tpqn(Context, utility::delta_clockstamp_tpqn const &) = 0;
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  virtual void delta_clockstamp(Context, utility::delta_clockstamp const &) = 0;

  virtual void unknown(Context, std::span<std::uint32_t>) = 0;
};

static_assert(utility<utility_pure<int>, int>, "utility_pure must implement the utility concept");

template <typename Context> struct system_pure {
  constexpr system_pure() noexcept = default;
  constexpr system_pure(system_pure const &) = default;
  constexpr system_pure(system_pure &&) noexcept = default;
  virtual ~system_pure() noexcept = default;

  constexpr system_pure &operator=(system_pure const &) = default;
  constexpr system_pure &operator=(system_pure &&) noexcept = default;

  // 7.6 System Common and System Real Time Messages
  virtual void midi_time_code(Context, system::midi_time_code const &) = 0;
  virtual void song_position_pointer(Context, system::song_position_pointer const &) = 0;
  virtual void song_select(Context, system::song_select const &) = 0;
  virtual void tune_request(Context, system::tune_request const &) = 0;
  virtual void timing_clock(Context, system::timing_clock const &) = 0;
  virtual void seq_start(Context, system::sequence_start const &) = 0;
  virtual void seq_continue(Context, system::sequence_continue const &) = 0;
  virtual void seq_stop(Context, system::sequence_stop const &) = 0;
  virtual void active_sensing(Context, system::active_sensing const &) = 0;
  virtual void reset(Context, system::reset const &) = 0;
};

static_assert(system<system_pure<int>, int>, "system_pure must implement the system concept");

template <typename Context> struct m1cvm_pure {
  constexpr m1cvm_pure() noexcept = default;
  constexpr m1cvm_pure(m1cvm_pure const &) = default;
  constexpr m1cvm_pure(m1cvm_pure &&) noexcept = default;
  virtual ~m1cvm_pure() noexcept = default;

  constexpr m1cvm_pure &operator=(m1cvm_pure const &) = default;
  constexpr m1cvm_pure &operator=(m1cvm_pure &&) noexcept = default;

  virtual void note_off(Context, m1cvm::note_off const &) = 0;
  virtual void note_on(Context, m1cvm::note_on const &) = 0;
  virtual void poly_pressure(Context, m1cvm::poly_pressure const &) = 0;
  virtual void control_change(Context, m1cvm::control_change const &) = 0;
  virtual void program_change(Context, m1cvm::program_change const &) = 0;
  virtual void channel_pressure(Context, m1cvm::channel_pressure const &) = 0;
  virtual void pitch_bend(Context, m1cvm::pitch_bend const &) = 0;
};

static_assert(m1cvm<m1cvm_pure<int>, int>, "m1cvm_pure must implement the m1cvm concept");

template <typename Context> struct data64_pure {
  constexpr data64_pure() noexcept = default;
  constexpr data64_pure(data64_pure const &) = default;
  constexpr data64_pure(data64_pure &&) noexcept = default;
  virtual ~data64_pure() noexcept = default;

  constexpr data64_pure &operator=(data64_pure const &) = default;
  constexpr data64_pure &operator=(data64_pure &&) noexcept = default;

  virtual void sysex7_in_1(Context, data64::sysex7_in_1 const &) = 0;
  virtual void sysex7_start(Context, data64::sysex7_start const &) = 0;
  virtual void sysex7_continue(Context, data64::sysex7_continue const &) = 0;
  virtual void sysex7_end(Context, data64::sysex7_end const &) = 0;
};

static_assert(data64<data64_pure<int>, int>, "data64_pure must implement the data64 concept");

template <typename Context> struct m2cvm_pure {
  constexpr m2cvm_pure() noexcept = default;
  constexpr m2cvm_pure(m2cvm_pure const &) = default;
  constexpr m2cvm_pure(m2cvm_pure &&) noexcept = default;
  virtual ~m2cvm_pure() noexcept = default;

  constexpr m2cvm_pure &operator=(m2cvm_pure const &) = default;
  constexpr m2cvm_pure &operator=(m2cvm_pure &&) noexcept = default;

  virtual void note_off(Context, m2cvm::note_off const &) = 0;
  virtual void note_on(Context, m2cvm::note_on const &) = 0;
  virtual void poly_pressure(Context, m2cvm::poly_pressure const &) = 0;
  virtual void program_change(Context, m2cvm::program_change const &) = 0;
  virtual void channel_pressure(Context, m2cvm::channel_pressure const &) = 0;

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  virtual void rpn_per_note_controller(Context, m2cvm::rpn_per_note_controller const &) = 0;
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  virtual void nrpn_per_note_controller(Context, m2cvm::nrpn_per_note_controller const &) = 0;
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  virtual void rpn_controller(Context, m2cvm::rpn_controller const &) = 0;
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  virtual void nrpn_controller(Context, m2cvm::nrpn_controller const &) = 0;
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  virtual void rpn_relative_controller(Context, m2cvm::rpn_relative_controller const &) = 0;
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  virtual void nrpn_relative_controller(Context, m2cvm::nrpn_relative_controller const &) = 0;

  virtual void per_note_management(Context, m2cvm::per_note_management const &) = 0;
  virtual void control_change(Context, m2cvm::control_change const &) = 0;
  virtual void pitch_bend(Context, m2cvm::pitch_bend const &) = 0;
  virtual void per_note_pitch_bend(Context, m2cvm::per_note_pitch_bend const &) = 0;
};

static_assert(m2cvm<m2cvm_pure<int>, int>, "m2cvm_pure must implement the m2cvm concept");

template <typename Context> struct data128_pure {
  constexpr data128_pure() noexcept = default;
  constexpr data128_pure(data128_pure const &) = default;
  constexpr data128_pure(data128_pure &&) noexcept = default;
  virtual ~data128_pure() noexcept = default;

  constexpr data128_pure &operator=(data128_pure const &) = default;
  constexpr data128_pure &operator=(data128_pure &&) noexcept = default;

  virtual void sysex8_in_1(Context, data128::sysex8_in_1 const &) = 0;
  virtual void sysex8_start(Context, data128::sysex8_start const &) = 0;
  virtual void sysex8_continue(Context, data128::sysex8_continue const &) = 0;
  virtual void sysex8_end(Context, data128::sysex8_end const &) = 0;
  virtual void mds_header(Context, data128::mds_header const &) = 0;
  virtual void mds_payload(Context, data128::mds_payload const &) = 0;
};

static_assert(data128<data128_pure<int>, int>, "data128_pure must implement the data128 concept");

template <typename Context> struct stream_pure {
  constexpr stream_pure() noexcept = default;
  constexpr stream_pure(stream_pure const &) = default;
  constexpr stream_pure(stream_pure &&) noexcept = default;
  virtual ~stream_pure() noexcept = default;

  constexpr stream_pure &operator=(stream_pure const &) = default;
  constexpr stream_pure &operator=(stream_pure &&) noexcept = default;

  virtual void endpoint_discovery(Context, stream::endpoint_discovery const &) = 0;
  virtual void endpoint_info_notification(Context, stream::endpoint_info_notification const &) = 0;
  virtual void device_identity_notification(Context, stream::device_identity_notification const &) = 0;
  virtual void endpoint_name_notification(Context, stream::endpoint_name_notification const &) = 0;
  virtual void product_instance_id_notification(Context, stream::product_instance_id_notification const &) = 0;
  virtual void jr_configuration_request(Context, stream::jr_configuration_request const &) = 0;
  virtual void jr_configuration_notification(Context, stream::jr_configuration_notification const &) = 0;

  virtual void function_block_discovery(Context, stream::function_block_discovery const &) = 0;
  virtual void function_block_info_notification(Context, stream::function_block_info_notification const &) = 0;
  virtual void function_block_name_notification(Context, stream::function_block_name_notification const &) = 0;

  virtual void start_of_clip(Context, stream::start_of_clip const &) = 0;
  virtual void end_of_clip(Context, stream::end_of_clip const &) = 0;
};

static_assert(stream<stream_pure<int>, int>, "stream_pure must implement the stream concept");

template <typename Context> struct flex_data_pure {
  constexpr flex_data_pure() noexcept = default;
  constexpr flex_data_pure(flex_data_pure const &) = default;
  constexpr flex_data_pure(flex_data_pure &&) noexcept = default;
  virtual ~flex_data_pure() noexcept = default;

  constexpr flex_data_pure &operator=(flex_data_pure const &) = default;
  constexpr flex_data_pure &operator=(flex_data_pure &&) noexcept = default;

  virtual void set_tempo(Context, flex_data::set_tempo const &) = 0;
  virtual void set_time_signature(Context, flex_data::set_time_signature const &) = 0;
  virtual void set_metronome(Context, flex_data::set_metronome const &) = 0;
  virtual void set_key_signature(Context, flex_data::set_key_signature const &) = 0;
  virtual void set_chord_name(Context, flex_data::set_chord_name const &) = 0;
  virtual void text(Context, flex_data::text_common const &) = 0;
};

static_assert(flex_data<flex_data_pure<int>, int>, "flex_data_pure must implement the flex_data concept");

// clang-format off
template <typename Context> struct utility_base : public utility_pure<Context> {
  // 7.2.1 NOOP
  void noop(Context) override { /* do nothing */ }
  // 7.2.2.1 JR Clock Message
  void jr_clock(Context, utility::jr_clock const &) override { /* do nothing */ }
  // 7.2.2.2 JR Timestamp Message
  void jr_timestamp(Context, utility::jr_timestamp const &) override { /* do nothing */ }
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (TPQN)
  void delta_clockstamp_tpqn(Context, utility::delta_clockstamp_tpqn const &) override { /* do nothing */ }
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  void delta_clockstamp(Context, utility::delta_clockstamp const &) override { /* do nothing */ }

  void unknown(Context, std::span<std::uint32_t>) override { /* do nothing */ }
};
template <typename Context> struct system_base : public system_pure<Context> {
  // 7.6 System Common and System Real Time Messages
  void midi_time_code(Context, system::midi_time_code const &) override { /* do nothing */ }
  void song_position_pointer(Context, system::song_position_pointer const &) override { /* do nothing */ }
  void song_select(Context, system::song_select const &) override { /* do nothing */ }
  void tune_request(Context, system::tune_request const &) override { /* do nothing */ }
  void timing_clock(Context, system::timing_clock const &) override { /* do nothing */ }
  void seq_start(Context, system::sequence_start const &) override { /* do nothing */ }
  void seq_continue(Context, system::sequence_continue const &) override { /* do nothing */ }
  void seq_stop(Context, system::sequence_stop const &) override { /* do nothing */ }
  void active_sensing(Context, system::active_sensing const &) override { /* do nothing */ }
  void reset(Context, system::reset const &) override { /* do nothing */ }
};
template <typename Context> struct m1cvm_base : public m1cvm_pure<Context> {
  void note_off(Context, m1cvm::note_off const &) override { /* do nothing */ }
  void note_on(Context, m1cvm::note_on const &) override { /* do nothing */ }
  void poly_pressure(Context, m1cvm::poly_pressure const &) override { /* do nothing */ }
  void control_change(Context, m1cvm::control_change const &) override { /* do nothing */ }
  void program_change(Context, m1cvm::program_change const &) override { /* do nothing */ }
  void channel_pressure(Context, m1cvm::channel_pressure const &) override { /* do nothing */ }
  void pitch_bend(Context, m1cvm::pitch_bend const &) override { /* do nothing */ }
};
template <typename Context> struct data64_base : public data64_pure<Context> {
  void sysex7_in_1(Context, data64::sysex7_in_1 const &) override { /* do nothing */ }
  void sysex7_start(Context, data64::sysex7_start const &) override { /* do nothing */ }
  void sysex7_continue(Context, data64::sysex7_continue const &) override { /* do nothing */ }
  void sysex7_end(Context, data64::sysex7_end const &) override { /* do nothing */ }
};
template <typename Context> struct m2cvm_base : public m2cvm_pure<Context> {
  void note_off(Context, m2cvm::note_off const &) override { /* do nothing */ }
  void note_on(Context, m2cvm::note_on const &) override { /* do nothing */ }
  void poly_pressure(Context, m2cvm::poly_pressure const &) override { /* do nothing */ }
  void program_change(Context, m2cvm::program_change const &) override { /* do nothing */ }
  void channel_pressure(Context, m2cvm::channel_pressure const &) override { /* do nothing */ }

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  void rpn_per_note_controller(Context, m2cvm::rpn_per_note_controller const &) override { /* do nothing */ }
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  void nrpn_per_note_controller(Context, m2cvm::nrpn_per_note_controller const &) override { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  void rpn_controller(Context, m2cvm::rpn_controller const &) override { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  void nrpn_controller(Context, m2cvm::nrpn_controller const &) override { /* do nothing */ }
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  void rpn_relative_controller(Context, m2cvm::rpn_relative_controller const &) override { /* do nothing */ }
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  void nrpn_relative_controller(Context, m2cvm::nrpn_relative_controller const &) override { /* do nothing */ }

  void per_note_management(Context, m2cvm::per_note_management const &) override { /* do nothing */ }
  void control_change(Context, m2cvm::control_change const &) override { /* do nothing */ }
  void pitch_bend(Context, m2cvm::pitch_bend const &) override { /* do nothing */ }
  void per_note_pitch_bend(Context, m2cvm::per_note_pitch_bend const &) override { /* do nothing */ }
};
template <typename Context> struct data128_base : public data128_pure<Context> {
  void sysex8_in_1(Context, data128::sysex8_in_1 const &) override { /* do nothing */ }
  void sysex8_start(Context, data128::sysex8_start const &) override { /* do nothing */ }
  void sysex8_continue(Context, data128::sysex8_continue const &) override { /* do nothing */ }
  void sysex8_end(Context, data128::sysex8_end const &) override { /* do nothing */ }
  void mds_header(Context, data128::mds_header const &) override { /* do nothing */ }
  void mds_payload(Context, data128::mds_payload const &) override { /* do nothing */ }
};
template <typename Context> struct stream_base : public stream_pure<Context> {
  void endpoint_discovery(Context, stream::endpoint_discovery const &) override { /* do nothing */ }
  void endpoint_info_notification(Context, stream::endpoint_info_notification const &) override { /* do nothing */ }
  void device_identity_notification(Context, stream::device_identity_notification const &) override { /* do nothing */ }
  void endpoint_name_notification(Context, stream::endpoint_name_notification const &) override { /* do nothing */ }
  void product_instance_id_notification(Context, stream::product_instance_id_notification const &) override { /* do nothing */ }
  void jr_configuration_request(Context, stream::jr_configuration_request const &) override { /* do nothing */ }
  void jr_configuration_notification(Context, stream::jr_configuration_notification const &) override { /* do nothing */ }

  void function_block_discovery(Context, stream::function_block_discovery const &) override { /* do nothing */ }
  void function_block_info_notification(Context, stream::function_block_info_notification const &) override { /* do nothing */ }
  void function_block_name_notification(Context, stream::function_block_name_notification const &) override { /* do nothing */ }

  void start_of_clip(Context, stream::start_of_clip const &) override { /* do nothing */ }
  void end_of_clip(Context, stream::end_of_clip const &) override { /* do nothing */ }
};
template <typename Context> struct flex_data_base : public flex_data_pure<Context> {
  void set_tempo(Context, flex_data::set_tempo const &) override { /* do nothing */ }
  void set_time_signature(Context, flex_data::set_time_signature const &) override { /* do nothing */ }
  void set_metronome(Context, flex_data::set_metronome const &) override { /* do nothing */ }
  void set_key_signature(Context, flex_data::set_key_signature const &) override { /* do nothing */ }
  void set_chord_name(Context, flex_data::set_chord_name const &) override { /* do nothing */ }
  void text(Context, flex_data::text_common const &) override { /* do nothing */ }
};
// clang-format on

template <typename Context> class utility_function {
public:
  using noop_fn = std::function<void(Context)>;
  using jr_clock_fn = std::function<void(Context, utility::jr_clock const &)>;
  using jr_timestamp_fn = std::function<void(Context, utility::jr_timestamp const &)>;
  using delta_clockstamp_tpqn_fn = std::function<void(Context, utility::delta_clockstamp_tpqn const &)>;
  using delta_clockstamp_fn = std::function<void(Context, utility::delta_clockstamp const &)>;
  using unknown_fn = std::function<void(Context, std::span<std::uint32_t>)>;

  // clang-format off
  constexpr utility_function &on_noop(noop_fn noop) noexcept { noop_ = std::move(noop); return *this; }
  constexpr utility_function &on_jr_clock(jr_clock_fn jr_clock) noexcept { jr_clock_ = std::move(jr_clock); return *this; }
  constexpr utility_function &on_jr_timestamp(jr_timestamp_fn jr_timestamp) noexcept { jr_timestamp_ = std::move(jr_timestamp); return *this; }
  constexpr utility_function &on_delta_clockstamp_tpqn(delta_clockstamp_tpqn_fn delta_clockstamp) noexcept { delta_clockstamp_tpqn_ = std::move(delta_clockstamp); return *this; }
  constexpr utility_function &on_delta_clockstamp(delta_clockstamp_fn delta_clockstamp) noexcept { delta_clockstamp_ = std::move(delta_clockstamp); return *this; }
  constexpr utility_function &on_unknown(unknown_fn unknown) noexcept { unknown_ = std::move(unknown); return *this; }

  void noop(Context c) const { call(noop_, c); }
  void jr_clock(Context c, utility::jr_clock const &clock) const { call(jr_clock_, c, clock); }
  void jr_timestamp(Context c, utility::jr_timestamp const &ts) const { call(jr_timestamp_, c, ts); }
  void delta_clockstamp_tpqn(Context c, utility::delta_clockstamp_tpqn const &time) const { call(delta_clockstamp_tpqn_, c, time); }
  void delta_clockstamp(Context c, utility::delta_clockstamp const &time) const { call(delta_clockstamp_, c, time); }
  void unknown(Context c, std::span<std::uint32_t> data) const { call(unknown_, c, data); }
  // clang-format on

private:
  noop_fn noop_;
  jr_clock_fn jr_clock_;
  jr_timestamp_fn jr_timestamp_;
  delta_clockstamp_tpqn_fn delta_clockstamp_tpqn_;
  delta_clockstamp_fn delta_clockstamp_;
  unknown_fn unknown_;
};

static_assert(utility<utility_function<int>, int>, "utility_function must implement the utility concept");

// 7.6 System Common and System Real Time Messages
template <typename Context> class system_function {
public:
  using midi_time_code_fn = std::function<void(Context, system::midi_time_code const &)>;
  using song_position_pointer_fn = std::function<void(Context, system::song_position_pointer const &)>;
  using song_select_fn = std::function<void(Context, system::song_select const &)>;
  using tune_request_fn = std::function<void(Context, system::tune_request const &)>;
  using timing_clock_fn = std::function<void(Context, system::timing_clock const &)>;
  using seq_start_fn = std::function<void(Context, system::sequence_start const &)>;
  using seq_continue_fn = std::function<void(Context, system::sequence_continue const &)>;
  using seq_stop_fn = std::function<void(Context, system::sequence_stop const &)>;
  using active_sensing_fn = std::function<void(Context, system::active_sensing const &)>;
  using reset_fn = std::function<void(Context, system::reset const &)>;

  // clang-format off
  constexpr system_function &on_midi_time_code(midi_time_code_fn midi_time_code) { midi_time_code_ = std::move(midi_time_code); return *this; }
  constexpr system_function &on_song_position_pointer(song_position_pointer_fn spp) { song_position_pointer_ = std::move(spp); return *this; }
  constexpr system_function &on_song_select(song_select_fn song_select) { song_select_ = std::move(song_select); return *this; }
  constexpr system_function &on_tune_request(tune_request_fn tune_request) { tune_request_ = std::move(tune_request); return *this; }
  constexpr system_function &on_timing_clock(timing_clock_fn timing_clock) { timing_clock_ = std::move(timing_clock); return *this; }
  constexpr system_function &on_seq_start(seq_start_fn seq_start) { seq_start_ = std::move(seq_start); return *this; }
  constexpr system_function &on_seq_continue(seq_continue_fn seq_continue) { seq_continue_ = std::move(seq_continue); return *this; }
  constexpr system_function &on_seq_stop(seq_stop_fn seq_stop) { seq_stop_ = std::move(seq_stop); return *this; }
  constexpr system_function &on_active_sensing(active_sensing_fn active_sensing) { active_sensing_ = std::move(active_sensing); return *this; }
  constexpr system_function &on_reset(reset_fn reset) { reset_ = std::move(reset); return *this; }

  void midi_time_code(Context c, system::midi_time_code const &mtc) const { call(midi_time_code_, c, mtc); }
  void song_position_pointer(Context c, system::song_position_pointer const &spp) const { call(song_position_pointer_, c, spp); }
  void song_select(Context c, system::song_select const &song) const { call(song_select_, c, song); }
  void tune_request(Context c, system::tune_request const &request) const { call(tune_request_, c, request); }
  void timing_clock(Context c, system::timing_clock const &clock) const { call(timing_clock_, c, clock); }
  void seq_start(Context c, system::sequence_start const &ss) const { call(seq_start_, c, ss); }
  void seq_continue(Context c, system::sequence_continue const &sc) const { call(seq_continue_, c, sc); }
  void seq_stop(Context c, system::sequence_stop const &ss) const { call(seq_stop_, c, ss); }
  void active_sensing(Context c, system::active_sensing const &as) const { call(active_sensing_, c, as); }
  void reset(Context c, system::reset const &r) const { call(reset_, c, r); }
  // clang-format on

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
  using note_off_fn = std::function<void(Context, m1cvm::note_off const &)>;
  using note_on_fn = std::function<void(Context, m1cvm::note_on const &)>;
  using poly_pressure_fn = std::function<void(Context, m1cvm::poly_pressure const &)>;
  using control_change_fn = std::function<void(Context, m1cvm::control_change const &)>;
  using program_change_fn = std::function<void(Context, m1cvm::program_change const &)>;
  using channel_pressure_fn = std::function<void(Context, m1cvm::channel_pressure const &)>;
  using pitch_bend_fn = std::function<void(Context, m1cvm::pitch_bend const &)>;

  // clang-format off
  constexpr m1cvm_function &on_note_off(note_off_fn note_off) noexcept { note_off_ = std::move(note_off); return *this; }
  constexpr m1cvm_function &on_note_on(note_on_fn note_on) noexcept { note_on_ = std::move(note_on); return *this; }
  constexpr m1cvm_function &on_poly_pressure(poly_pressure_fn poly_pressure) noexcept { poly_pressure_ = std::move(poly_pressure); return *this; }
  constexpr m1cvm_function &on_control_change(control_change_fn control_change) noexcept { control_change_ = std::move(control_change); return *this; }
  constexpr m1cvm_function &on_program_change(program_change_fn program_change) noexcept { program_change_ = std::move(program_change); return *this; }
  constexpr m1cvm_function &on_channel_pressure(channel_pressure_fn channel_pressure) noexcept { channel_pressure_ = std::move(channel_pressure); return *this; }
  constexpr m1cvm_function &on_pitch_bend(pitch_bend_fn pitch_bend) noexcept { pitch_bend_ = std::move(pitch_bend); return *this; }

  void note_off(Context c, m1cvm::note_off const &off) const { call(note_off_, c, off); }
  void note_on(Context c, m1cvm::note_on const &on) const { call(note_on_, c, on); }
  void poly_pressure(Context c, m1cvm::poly_pressure const &pressure) const { call(poly_pressure_, c, pressure); }
  void control_change(Context c, m1cvm::control_change const &cc) const { call(control_change_, c, cc); }
  void program_change(Context c, m1cvm::program_change const &program) const { call(program_change_, c, program); }
  void channel_pressure(Context c, m1cvm::channel_pressure const &pressure) const { call(channel_pressure_, c, pressure); }
  void pitch_bend(Context c, m1cvm::pitch_bend const &bend) const { call(pitch_bend_, c, bend); }
  // clang-format on

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
  using sysex7_in_1_fn = std::function<void(Context, data64::sysex7_in_1 const &)>;
  using sysex7_start_fn = std::function<void(Context, data64::sysex7_start const &)>;
  using sysex7_continue_fn = std::function<void(Context, data64::sysex7_continue const &)>;
  using sysex7_end_fn = std::function<void(Context, data64::sysex7_end const &)>;

  // clang-format off
  constexpr data64_function &on_sysex7_in_1(sysex7_in_1_fn sysex7_in_1) noexcept { sysex7_in_1_ = std::move(sysex7_in_1); return *this; }
  constexpr data64_function &on_sysex7_start(sysex7_start_fn sysex7_start) noexcept { sysex7_start_ = std::move(sysex7_start); return *this; }
  constexpr data64_function &on_sysex7_continue(sysex7_continue_fn sysex7_continue) noexcept { sysex7_continue_ = std::move(sysex7_continue); return *this; }
  constexpr data64_function &on_sysex7_end(sysex7_end_fn sysex7_end) noexcept { sysex7_end_ = std::move(sysex7_end); return *this; }

  void sysex7_in_1(Context c, data64::sysex7_in_1 const &sx) const { call(sysex7_in_1_, c, sx); }
  void sysex7_start(Context c, data64::sysex7_start const &sx) const { call(sysex7_start_, c, sx); }
  void sysex7_continue(Context c, data64::sysex7_continue const &sx) const { call(sysex7_continue_, c, sx); }
  void sysex7_end(Context c, data64::sysex7_end const &sx) const { call(sysex7_end_, c, sx); }
  // clang-format on

private:
  sysex7_in_1_fn sysex7_in_1_;
  sysex7_start_fn sysex7_start_;
  sysex7_continue_fn sysex7_continue_;
  sysex7_end_fn sysex7_end_;
};

static_assert(data64<data64_function<int>, int>, "data64_function must implement the data64 concept");

template <typename Context> class m2cvm_function {
public:
  using note_off_fn = std::function<void(Context, m2cvm::note_off const &)>;
  using note_on_fn = std::function<void(Context, m2cvm::note_on const &)>;
  using poly_pressure_fn = std::function<void(Context, m2cvm::poly_pressure const &)>;
  using program_change_fn = std::function<void(Context, m2cvm::program_change const &)>;
  using channel_pressure_fn = std::function<void(Context, m2cvm::channel_pressure const &)>;
  using rpn_per_note_controller_fn = std::function<void(Context, m2cvm::rpn_per_note_controller const &)>;
  using nrpn_per_note_controller_fn = std::function<void(Context, m2cvm::nrpn_per_note_controller const &)>;
  using rpn_controller_fn = std::function<void(Context, m2cvm::rpn_controller const &)>;
  using nrpn_controller_fn = std::function<void(Context, m2cvm::nrpn_controller const &)>;
  using rpn_relative_controller_fn = std::function<void(Context, m2cvm::rpn_relative_controller const &)>;
  using nrpn_relative_controller_fn = std::function<void(Context, m2cvm::nrpn_relative_controller const &)>;
  using per_note_management_fn = std::function<void(Context, m2cvm::per_note_management const &)>;
  using control_change_fn = std::function<void(Context, m2cvm::control_change const &)>;
  using pitch_bend_fn = std::function<void(Context, m2cvm::pitch_bend const &)>;
  using per_note_pitch_bend_fn = std::function<void(Context, m2cvm::per_note_pitch_bend const &)>;

  // clang-format off
  constexpr m2cvm_function &on_note_off(note_off_fn off) noexcept { note_off_ = std::move(off); return *this; }
  constexpr m2cvm_function &on_note_on(note_on_fn on) noexcept { note_on_ = std::move(on); return *this; }
  constexpr m2cvm_function &on_poly_pressure(poly_pressure_fn pressure) noexcept { poly_pressure_ = std::move(pressure); return *this; }
  constexpr m2cvm_function &on_program_change(program_change_fn program_change) noexcept { program_change_ = std::move(program_change); return *this; }
  constexpr m2cvm_function &on_channel_pressure(channel_pressure_fn pressure) noexcept { channel_pressure_ = std::move(pressure); return *this; }
  constexpr m2cvm_function &on_rpn_per_note_controller(rpn_per_note_controller_fn controller) noexcept { rpn_per_note_controller_ = std::move(controller); return *this; }
  constexpr m2cvm_function &on_nrpn_per_note_controller(nrpn_per_note_controller_fn controller) noexcept { nrpn_per_note_controller_ = std::move(controller); return *this; }
  constexpr m2cvm_function &on_rpn_controller(rpn_controller_fn controller) noexcept { rpn_controller_ = std::move(controller); return *this; }
  constexpr m2cvm_function &on_nrpn_controller(nrpn_controller_fn controller) noexcept { nrpn_controller_ = std::move(controller); return *this; }
  constexpr m2cvm_function &on_rpn_relative_controller(rpn_relative_controller_fn controller) noexcept { rpn_relative_controller_ = std::move(controller); return *this; }
  constexpr m2cvm_function &on_nrpn_relative_controller(nrpn_relative_controller_fn controller) noexcept { nrpn_relative_controller_ = std::move(controller); return *this; }
  constexpr m2cvm_function &on_per_note_management(per_note_management_fn per_note_management) noexcept { per_note_management_ = std::move(per_note_management); return *this; }
  constexpr m2cvm_function &on_control_change(control_change_fn cc) noexcept { control_change_ = std::move(cc); return *this; }
  constexpr m2cvm_function &on_pitch_bend(pitch_bend_fn pitch_bend) noexcept { pitch_bend_ = std::move(pitch_bend); return *this; }
  constexpr m2cvm_function &on_per_note_pitch_bend(per_note_pitch_bend_fn per_note_pitch_bend) noexcept { per_note_pitch_bend_ = std::move(per_note_pitch_bend); return *this; }

  void note_off(Context c, m2cvm::note_off const &off) const { call(note_off_, c, off); }
  void note_on(Context c, m2cvm::note_on const &on) const { call(note_on_, c, on); }
  void poly_pressure(Context c, m2cvm::poly_pressure const &pressure) const { call(poly_pressure_, c, pressure); }
  void program_change(Context c, m2cvm::program_change const &program) const { call(program_change_, c, program); }
  void channel_pressure(Context c, m2cvm::channel_pressure const &pressure) const { call(channel_pressure_, c, pressure); }
  void rpn_per_note_controller(Context c, m2cvm::rpn_per_note_controller const &rpn) const { call(rpn_per_note_controller_, c, rpn); }
  void nrpn_per_note_controller(Context c, m2cvm::nrpn_per_note_controller const &nrpn) const { call(nrpn_per_note_controller_, c, nrpn); }
  void rpn_controller(Context c, m2cvm::rpn_controller const &rpn) const { call(rpn_controller_, c, rpn); }
  void nrpn_controller(Context c, m2cvm::nrpn_controller const &nrpn) const { call(nrpn_controller_, c, nrpn); }
  void rpn_relative_controller(Context c, m2cvm::rpn_relative_controller const &rpn) const { call(rpn_relative_controller_, c, rpn); }
  void nrpn_relative_controller(Context c, m2cvm::nrpn_relative_controller const &nrpn) const { call(nrpn_relative_controller_, c, nrpn); }
  void per_note_management(Context c, m2cvm::per_note_management const &pnm) const { call(per_note_management_, c, pnm); }
  void control_change(Context c, m2cvm::control_change const &cc) const { call(control_change_, c, cc); }
  void pitch_bend(Context c, m2cvm::pitch_bend const &pb) const { call(pitch_bend_, c, pb); }
  void per_note_pitch_bend(Context c, m2cvm::per_note_pitch_bend const &pb) const { call(per_note_pitch_bend_, c, pb); }
  // clang-format on

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
  using sysex8_in_1_fn = std::function<void(Context, data128::sysex8_in_1 const &)>;
  using sysex8_start_fn = std::function<void(Context, data128::sysex8_start const &)>;
  using sysex8_continue_fn = std::function<void(Context, data128::sysex8_continue const &)>;
  using sysex8_end_fn = std::function<void(Context, data128::sysex8_end const &)>;
  using mds_header_fn = std::function<void(Context, data128::mds_header const &)>;
  using mds_payload_fn = std::function<void(Context, data128::mds_payload const &)>;

  // clang-format off
  constexpr data128_function &on_sysex8_in_1(sysex8_in_1_fn sysex8_in_1) noexcept { sysex8_in_1_ = std::move(sysex8_in_1); return *this; }
  constexpr data128_function &on_sysex8_start(sysex8_start_fn sysex8_start) noexcept { sysex8_start_ = std::move(sysex8_start); return *this; }
  constexpr data128_function &on_sysex8_continue(sysex8_continue_fn sysex8_continue) noexcept { sysex8_continue_ = std::move(sysex8_continue); return *this; }
  constexpr data128_function &on_sysex8_end(sysex8_end_fn sysex8_end) noexcept { sysex8_end_ = std::move(sysex8_end); return *this; }
  constexpr data128_function &on_mds_header(mds_header_fn mds_header) noexcept { mds_header_ = std::move(mds_header); return *this; }
  constexpr data128_function &on_mds_payload(mds_payload_fn mds_payload) noexcept { mds_payload_ = std::move(mds_payload); return *this; }

  void sysex8_in_1(Context c, data128::sysex8_in_1 const &sysex) const { call(sysex8_in_1_, c, sysex); }
  void sysex8_start(Context c, data128::sysex8_start const &sysex) const { call(sysex8_start_, c, sysex); }
  void sysex8_continue(Context c, data128::sysex8_continue const &sysex) const { call(sysex8_continue_, c, sysex); }
  void sysex8_end(Context c, data128::sysex8_end const &sysex) const { call(sysex8_end_, c, sysex); }
  void mds_header(Context c, data128::mds_header const &mds) const { call(mds_header_, c, mds); }
  void mds_payload(Context c, data128::mds_payload const &mds) const { call(mds_payload_, c, mds); }
  // clang-format on

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
  using endpoint_discovery_fn = std::function<void(Context, stream::endpoint_discovery const &)>;
  using endpoint_info_notification_fn = std::function<void(Context, stream::endpoint_info_notification const &)>;
  using device_identity_notification_fn = std::function<void(Context, stream::device_identity_notification const &)>;
  using endpoint_name_notification_fn = std::function<void(Context, stream::endpoint_name_notification const &)>;
  using product_instance_id_notification_fn =
      std::function<void(Context, stream::product_instance_id_notification const &)>;
  using jr_configuration_request_fn = std::function<void(Context, stream::jr_configuration_request const &)>;
  using jr_configuration_notification_fn = std::function<void(Context, stream::jr_configuration_notification const &)>;
  using function_block_discovery_fn = std::function<void(Context, stream::function_block_discovery const &)>;
  using function_block_info_notification_fn =
      std::function<void(Context, stream::function_block_info_notification const &)>;
  using function_block_name_notification_fn =
      std::function<void(Context, stream::function_block_name_notification const &)>;
  using start_of_clip_fn = std::function<void(Context, stream::start_of_clip const &)>;
  using end_of_clip_fn = std::function<void(Context, stream::end_of_clip const &)>;

  // clang-format off
  constexpr stream_function &on_endpoint_discovery(endpoint_discovery_fn discovery) noexcept { endpoint_discovery_ = std::move(discovery); return *this; }
  constexpr stream_function &on_endpoint_info_notification(endpoint_info_notification_fn notification) noexcept { endpoint_info_notification_ = std::move(notification); return *this; }
  constexpr stream_function &on_device_identity_notification(device_identity_notification_fn notification) noexcept { device_identity_notification_ = std::move(notification); return *this; }
  constexpr stream_function &on_endpoint_name_notification(endpoint_name_notification_fn notification) noexcept { endpoint_name_notification_ = std::move(notification); return *this; }
  constexpr stream_function &on_product_instance_id_notification(product_instance_id_notification_fn notification) noexcept { product_instance_id_notification_ = std::move(notification); return *this; }
  constexpr stream_function &on_jr_configuration_request(jr_configuration_request_fn request) noexcept { jr_configuration_request_ = std::move(request); return *this; }
  constexpr stream_function &on_jr_configuration_notification(jr_configuration_notification_fn notification) noexcept { jr_configuration_notification_ = std::move(notification); return *this; }
  constexpr stream_function &on_function_block_discovery(function_block_discovery_fn discovery) noexcept { function_block_discovery_ = std::move(discovery); return *this; }
  constexpr stream_function &on_function_block_info_notification(function_block_info_notification_fn notification) noexcept { function_block_info_notification_ = std::move(notification); return *this; }
  constexpr stream_function &on_function_block_name_notification(function_block_name_notification_fn notification) noexcept { function_block_name_notification_ = std::move(notification); return *this; }
  constexpr stream_function &on_start_of_clip(start_of_clip_fn clip) noexcept { start_of_clip_ = std::move(clip); return *this; }
  constexpr stream_function &on_end_of_clip(end_of_clip_fn clip) noexcept { end_of_clip_ = std::move(clip); return *this; }

  void endpoint_discovery(Context c, stream::endpoint_discovery const &discovery) const { call(endpoint_discovery_, c, discovery); }
  void endpoint_info_notification(Context c, stream::endpoint_info_notification const &notification) const { call(endpoint_info_notification_, c, notification); }
  void device_identity_notification(Context c, stream::device_identity_notification const &notification) const { call(device_identity_notification_, c, notification); }
  void endpoint_name_notification(Context c, stream::endpoint_name_notification const &notification) const { call(endpoint_name_notification_, c, notification); }
  void product_instance_id_notification(Context c, stream::product_instance_id_notification const &notification) const { call(product_instance_id_notification_, c, notification); }
  void jr_configuration_request(Context c, stream::jr_configuration_request const &request) const { call(jr_configuration_request_, c, request); }
  void jr_configuration_notification(Context c, stream::jr_configuration_notification const &notification) const { call(jr_configuration_notification_, c, notification); }
  void function_block_discovery(Context c, stream::function_block_discovery const &discovery) const { call(function_block_discovery_, c, discovery); }
  void function_block_info_notification(Context c, stream::function_block_info_notification const &notification) const { call(function_block_info_notification_, c, notification); }
  void function_block_name_notification(Context c, stream::function_block_name_notification const &notification) const { call(function_block_name_notification_, c, notification); }
  void start_of_clip(Context c, stream::start_of_clip const &clip) const { call(start_of_clip_, c, clip); }
  void end_of_clip(Context c, stream::end_of_clip const &clip) const { call(end_of_clip_, c, clip); }
  // clang-format on

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
  using set_tempo_fn = std::function<void(Context, flex_data::set_tempo const &)>;
  using set_time_signature_fn = std::function<void(Context, flex_data::set_time_signature const &)>;
  using set_metronome_fn = std::function<void(Context, flex_data::set_metronome const &)>;
  using set_key_signature_fn = std::function<void(Context, flex_data::set_key_signature const &)>;
  using set_chord_name_fn = std::function<void(Context, flex_data::set_chord_name const &)>;
  using text_fn = std::function<void(Context, flex_data::text_common const &)>;

  // clang-format off
  constexpr flex_data_function &on_set_tempo(set_tempo_fn set_tempo) noexcept { set_tempo_ = std::move(set_tempo); return *this; }
  constexpr flex_data_function &on_set_time_signature(set_time_signature_fn set_time_signature) noexcept { set_time_signature_ = std::move(set_time_signature); return *this; }
  constexpr flex_data_function &on_set_metronome(set_metronome_fn set_metronome) noexcept { set_metronome_ = std::move(set_metronome); return *this; }
  constexpr flex_data_function &on_set_key_signature(set_key_signature_fn set_key_signature) noexcept { set_key_signature_ = std::move(set_key_signature); return *this; }
  constexpr flex_data_function &on_set_chord_name(set_chord_name_fn set_chord_name) noexcept { set_chord_name_ = std::move(set_chord_name); return *this; }
  constexpr flex_data_function &on_text(text_fn text) noexcept { text_ = std::move(text); return *this; }

  void set_tempo(Context c, flex_data::set_tempo const &tempo) const { call(set_tempo_, c, tempo); }
  void set_time_signature(Context c, flex_data::set_time_signature const &ts) const { call(set_time_signature_, c, ts); }
  void set_metronome(Context c, flex_data::set_metronome const &metronome) const { call(set_metronome_, c, metronome); }
  void set_key_signature(Context c, flex_data::set_key_signature const &ks) const { call(set_key_signature_, c, ks); }
  void set_chord_name(Context c, flex_data::set_chord_name const &chord) const { call(set_chord_name_, c, chord); }
  void text(Context c, flex_data::text_common const &t) const { call(text_, c, t); }
  // clang-format on

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
