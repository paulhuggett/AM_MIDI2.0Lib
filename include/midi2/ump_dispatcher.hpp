//===-- UMP Dispatcher --------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMP_DISPATCHER_HPP
#define MIDI2_UMP_DISPATCHER_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <span>

#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

// See M2-104-UM (UMP Format & MIDI 2.0 Protocol v.1.1.2 2023-10-27)
//    Table 4 Message Type (MT) Allocation

constexpr unsigned ump_message_size(ump::message_type const mt) {
  using enum ump::message_type;
#define X(a, b) \
  case a: return message_size<a>();
  switch (mt) {
    UMP_MESSAGE_TYPES
  default: return 0U;
  }
#undef X
}

// clang-format off
template <typename T, typename Context>
concept utility_backend = requires(T v, Context context) {
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
concept system_backend = requires(T v, Context context) {
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
concept m1cvm_backend = requires(T v, Context context) {
  { v.note_off(context, ump::m1cvm::note_off{}) } -> std::same_as<void>;
  { v.note_on(context, ump::m1cvm::note_on{}) } -> std::same_as<void>;
  { v.poly_pressure(context, ump::m1cvm::poly_pressure{}) } -> std::same_as<void>;
  { v.control_change(context, ump::m1cvm::control_change{}) } -> std::same_as<void>;
  { v.program_change(context, ump::m1cvm::program_change{}) } -> std::same_as<void>;
  { v.channel_pressure(context, ump::m1cvm::channel_pressure{}) } -> std::same_as<void>;
  { v.pitch_bend(context, ump::m1cvm::pitch_bend{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept data64_backend = requires(T v, Context context) {
  { v.sysex7_in_1(context, ump::data64::sysex7_in_1{}) } -> std::same_as<void>;
  { v.sysex7_start(context, ump::data64::sysex7_start{}) } -> std::same_as<void>;
  { v.sysex7_continue(context, ump::data64::sysex7_continue{}) } -> std::same_as<void>;
  { v.sysex7_end(context, ump::data64::sysex7_end{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept m2cvm_backend = requires(T v, Context context) {
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
concept data128_backend = requires(T v, Context context) {
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
concept ump_stream_backend = requires(T v, Context context) {
  { v.endpoint_discovery(context, ump::ump_stream::endpoint_discovery{}) } -> std::same_as<void>;
  { v.endpoint_info_notification(context, ump::ump_stream::endpoint_info_notification{}) } -> std::same_as<void>;
  { v.device_identity_notification(context, ump::ump_stream::device_identity_notification{}) } -> std::same_as<void>;
  { v.endpoint_name_notification(context, ump::ump_stream::endpoint_name_notification{}) } -> std::same_as<void>;
  { v.product_instance_id_notification(context, ump::ump_stream::product_instance_id_notification{}) } -> std::same_as<void>;
  { v.jr_configuration_request(context, ump::ump_stream::jr_configuration_request{}) } -> std::same_as<void>;
  { v.jr_configuration_notification(context, ump::ump_stream::jr_configuration_notification{}) } -> std::same_as<void>;
  { v.function_block_discovery(context, ump::ump_stream::function_block_discovery{}) } -> std::same_as<void>;
  { v.function_block_info_notification(context, ump::ump_stream::function_block_info_notification{}) } -> std::same_as<void>;
  { v.function_block_name_notification(context, ump::ump_stream::function_block_name_notification{}) } -> std::same_as<void>;
  { v.start_of_clip(context, ump::ump_stream::start_of_clip{}) } -> std::same_as<void>;
  { v.end_of_clip(context, ump::ump_stream::end_of_clip{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept flex_data_backend = requires(T v, Context context) {
  { v.set_tempo(context, ump::flex_data::set_tempo{}) } -> std::same_as<void>;
  { v.set_time_signature(context, ump::flex_data::set_time_signature{}) } -> std::same_as<void>;
  { v.set_metronome(context, ump::flex_data::set_metronome{}) } -> std::same_as<void>;
  { v.set_key_signature(context, ump::flex_data::set_key_signature{}) } -> std::same_as<void>;
  { v.set_chord_name(context, ump::flex_data::set_chord_name{}) } -> std::same_as<void>;
  { v.text(context, ump::flex_data::text_common{}) } -> std::same_as<void>;
};

template <typename T>
concept ump_dispatcher_config = requires (T v) {
  { v.context };
  { v.utility } -> utility_backend<decltype(v.context)>;
  { v.system } -> system_backend<decltype(v.context)>;
  { v.m1cvm } -> m1cvm_backend<decltype(v.context)>;
  { v.data64 } -> data64_backend<decltype(v.context)>;
  { v.m2cvm } -> m2cvm_backend<decltype(v.context)>;
  { v.data128 } -> data128_backend<decltype(v.context)>;
  { v.ump_stream } -> ump_stream_backend<decltype(v.context)>;
  { v.flex } -> flex_data_backend<decltype(v.context)>;
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
template <typename Context> struct m1cvm_null {
  constexpr static void note_off(Context, ump::m1cvm::note_off const &) { /* do nothing */ }
  constexpr static void note_on(Context, ump::m1cvm::note_on const &) { /* do nothing */ }
  constexpr static void poly_pressure(Context, ump::m1cvm::poly_pressure const &) { /* do nothing */ }
  constexpr static void control_change(Context, ump::m1cvm::control_change const &) { /* do nothing */ }
  constexpr static void program_change(Context, ump::m1cvm::program_change const &) { /* do nothing */ }
  constexpr static void channel_pressure(Context, ump::m1cvm::channel_pressure const &) { /* do nothing */ }
  constexpr static void pitch_bend(Context, ump::m1cvm::pitch_bend const &) { /* do nothing */ }
};
template <typename Context> struct data64_null {
  constexpr static void sysex7_in_1(Context, ump::data64::sysex7_in_1 const &) { /* do nothing */ }
  constexpr static void sysex7_start(Context, ump::data64::sysex7_start const &) { /* do nothing */ }
  constexpr static void sysex7_continue(Context, ump::data64::sysex7_continue const &) { /* do nothing */ }
  constexpr static void sysex7_end(Context, ump::data64::sysex7_end const &) { /* do nothing */ }
};
template <typename Context> struct m2cvm_null {
  constexpr static void note_off(Context, ump::m2cvm::note_off const &) { /* do nothing */ }
  constexpr static void note_on(Context, ump::m2cvm::note_on const &) { /* do nothing */ }
  constexpr static void poly_pressure(Context, ump::m2cvm::poly_pressure const &) { /* do nothing */ }
  constexpr static void program_change(Context, ump::m2cvm::program_change const &) { /* do nothing */ }
  constexpr static void channel_pressure(Context, ump::m2cvm::channel_pressure const &) { /* do nothing */ }

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  constexpr static void rpn_per_note_controller(Context, ump::m2cvm::rpn_per_note_controller const &) { /* do nothing */ }
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x1)
  constexpr static void nrpn_per_note_controller(Context, ump::m2cvm::nrpn_per_note_controller const &) { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  constexpr static void rpn_controller(Context, ump::m2cvm::rpn_controller const &) { /* do nothing */ }
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  constexpr static void nrpn_controller(Context, ump::m2cvm::nrpn_controller const &) { /* do nothing */ }
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  constexpr static void rpn_relative_controller(Context, ump::m2cvm::rpn_relative_controller const &) { /* do nothing */ }
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  constexpr static void nrpn_relative_controller(Context, ump::m2cvm::nrpn_relative_controller const &) { /* do nothing */ }

  constexpr static void per_note_management(Context, ump::m2cvm::per_note_management const &) { /* do nothing */ }
  constexpr static void control_change(Context, ump::m2cvm::control_change const &) { /* do nothing */ }
  constexpr static void pitch_bend(Context, ump::m2cvm::pitch_bend const &) { /* do nothing */ }
  constexpr static void per_note_pitch_bend(Context, ump::m2cvm::per_note_pitch_bend const &) { /* do nothing */ }
};
template <typename Context> struct data128_null {
  constexpr static void sysex8_in_1(Context, ump::data128::sysex8_in_1 const &) { /* do nothing */ }
  constexpr static void sysex8_start(Context, ump::data128::sysex8_start const &) { /* do nothing */ }
  constexpr static void sysex8_continue(Context, ump::data128::sysex8_continue const &) { /* do nothing */ }
  constexpr static void sysex8_end(Context, ump::data128::sysex8_end const &) { /* do nothing */ }
  constexpr static void mds_header(Context, ump::data128::mds_header const &) { /* do nothing */ }
  constexpr static void mds_payload(Context, ump::data128::mds_payload const &) { /* do nothing */ }
};
template <typename Context> struct ump_stream_null {
  constexpr static void endpoint_discovery(Context, ump::ump_stream::endpoint_discovery const &) { /* do nothing */ }
  constexpr static void endpoint_info_notification(Context, ump::ump_stream::endpoint_info_notification const &) { /* do nothing */ }
  constexpr static void device_identity_notification(Context, ump::ump_stream::device_identity_notification const &) { /* do nothing */ }
  constexpr static void endpoint_name_notification(Context, ump::ump_stream::endpoint_name_notification const &) { /* do nothing */ }
  constexpr static void product_instance_id_notification(Context, ump::ump_stream::product_instance_id_notification const &) { /* do nothing */ }
  constexpr static void jr_configuration_request(Context, ump::ump_stream::jr_configuration_request const &) { /* do nothing */ }
  constexpr static void jr_configuration_notification(Context, ump::ump_stream::jr_configuration_notification const &) { /* do nothing */ }

  constexpr static void function_block_discovery(Context, ump::ump_stream::function_block_discovery const &) { /* do nothing */ }
  constexpr static void function_block_info_notification(Context, ump::ump_stream::function_block_info_notification const &) { /* do nothing */ }
  constexpr static void function_block_name_notification(Context, ump::ump_stream::function_block_name_notification const &) { /* do nothing */ }

  constexpr static void start_of_clip(Context, ump::ump_stream::start_of_clip const &) { /* do nothing */ }
  constexpr static void end_of_clip(Context, ump::ump_stream::end_of_clip const &) { /* do nothing */ }
};
template <typename Context> struct flex_data_null {
  constexpr static void set_tempo(Context, ump::flex_data::set_tempo const &) { /* do nothing */ }
  constexpr static void set_time_signature(Context, ump::flex_data::set_time_signature const &) { /* do nothing */ }
  constexpr static void set_metronome(Context, ump::flex_data::set_metronome const &) { /* do nothing */ }
  constexpr static void set_key_signature(Context, ump::flex_data::set_key_signature const &) { /* do nothing */ }
  constexpr static void set_chord_name(Context, ump::flex_data::set_chord_name const &) { /* do nothing */ }
  constexpr static void text(Context, ump::flex_data::text_common const &) { /* do nothing */ }
};
struct default_config {
  struct empty {};
  [[no_unique_address]] empty context{};
  [[no_unique_address]] utility_null<decltype(context)> utility;
  [[no_unique_address]] system_null<decltype(context)> system;
  [[no_unique_address]] m1cvm_null<decltype(context)> m1cvm;
  [[no_unique_address]] data64_null<decltype(context)> data64;
  [[no_unique_address]] m2cvm_null<decltype(context)> m2cvm;
  [[no_unique_address]] data128_null<decltype(context)> data128;
  [[no_unique_address]] ump_stream_null<decltype(context)> ump_stream;
  [[no_unique_address]] flex_data_null<decltype(context)> flex;
};

template <typename T> concept word_memfun = requires(T a) {
  { a.word() } -> std::convertible_to<std::uint32_t>;
};

template <ump_dispatcher_config Config = default_config> class ump_dispatcher {
public:
  explicit constexpr ump_dispatcher(Config const &config = default_config{}) : config_{config} {}

  void clearUMP() {
    // Note that this member function has to be defined in the class declaration to avoid a spurious GCC
    // warning that the function is defined but not used. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79001>
    pos_ = 0;
    std::ranges::fill(message_, std::uint8_t{0});
  }
  void processUMP(std::uint32_t const ump) {
    // Note that this member function has to be defined in the class declaration to avoid a spurious GCC
    // warning that the function is defined but not used. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79001>
    assert(pos_ < message_.size());
    message_[pos_++] = ump;
    if (auto const mt = static_cast<ump::message_type>((message_[0] >> 28) & 0xF); pos_ >= ump_message_size(mt)) {
      switch (mt) {
      case ump::message_type::utility: this->utility_message(); break;
      case ump::message_type::system: this->system_message(); break;
      case ump::message_type::m1cvm: this->m1cvm_message(); break;
      case ump::message_type::m2cvm: this->m2cvm_message(); break;
      case ump::message_type::flex_data: this->flex_data_message(); break;
      case ump::message_type::ump_stream: this->ump_stream_message(); break;
      case ump::message_type::data64: this->data64_message(); break;
      case ump::message_type::data128: this->data128_message(); break;

      case ump::message_type::reserved32_06:
      case ump::message_type::reserved32_07:
      case ump::message_type::reserved64_08:
      case ump::message_type::reserved64_09:
      case ump::message_type::reserved64_0A:
      case ump::message_type::reserved96_0B:
      case ump::message_type::reserved96_0C:
      case ump::message_type::reserved128_0E:
        config_.utility.unknown(config_.context, std::span{message_.data(), pos_});
        break;
      default:
        assert(false);
        unreachable();
        break;
      }
      pos_ = 0;
    }
  }

private:
  void utility_message();
  void system_message();
  void m1cvm_message();
  void data64_message();
  void m2cvm_message();
  void ump_stream_message();
  void data128_message();
  void flex_data_message();

  std::array<std::uint32_t, 4> message_{};
  std::uint8_t pos_ = 0;

  [[no_unique_address]] Config config_;
};

template <typename T> ump_dispatcher(T) -> ump_dispatcher<T>;

// utility message
// ~~~~~~~~~~~~~~~
// 32 bit utility messages
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::utility_message() {
  constexpr auto size = ump_message_size(midi2::ump::message_type::utility);
  assert(pos_ == size);

  auto const span = std::span<std::uint32_t, size>{message_.data(), size};
  using enum ump::mt::ump_utility;
  switch (static_cast<ump::mt::ump_utility>((message_[0] >> 20) & 0x0F)) {
  // 7.2.1 NOOP
  case noop: config_.utility.noop(config_.context); break;
  // 7.2.2.1 JR Clock
  case jr_clock: config_.utility.jr_clock(config_.context, ump::utility::jr_clock{span}); break;
  // 7.2.2.2 JR Timestamp
  case jr_ts: config_.utility.jr_timestamp(config_.context, ump::utility::jr_timestamp{span}); break;
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
  case delta_clock_tick:
    config_.utility.delta_clockstamp_tpqn(config_.context, ump::utility::delta_clockstamp_tpqn{message_[0]});
    break;
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  case delta_clock_since:
    config_.utility.delta_clockstamp(config_.context, ump::utility::delta_clockstamp{message_[0]});
    break;
  default: config_.utility.unknown(config_.context, std::span{message_.data(), 1}); break;
  }
}

// system message
// ~~~~~~~~~~~~~~
// 32 bit System Common and Real Time
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::system_message() {
  static_assert(message_size<midi2::ump::message_type::system>() == 1);
  using enum ump::mt::system_crt;
  switch (static_cast<ump::mt::system_crt>((message_[0] >> 16) & 0xFF)) {
  case timing_code: config_.system.midi_time_code(config_.context, ump::system::midi_time_code{message_[0]}); break;
  case spp:
    config_.system.song_position_pointer(config_.context, ump::system::song_position_pointer{message_[0]});
    break;
  case song_select: config_.system.song_select(config_.context, ump::system::song_select{message_[0]}); break;
  case tune_request: config_.system.tune_request(config_.context, ump::system::tune_request{message_[0]}); break;
  case timing_clock: config_.system.timing_clock(config_.context, ump::system::timing_clock{message_[0]}); break;
  case sequence_start: config_.system.seq_start(config_.context, ump::system::sequence_start{message_[0]}); break;
  case sequence_continue:
    config_.system.seq_continue(config_.context, ump::system::sequence_continue{message_[0]});
    break;
  case sequence_stop: config_.system.seq_stop(config_.context, ump::system::sequence_stop{message_[0]}); break;
  case active_sensing: config_.system.active_sensing(config_.context, ump::system::active_sensing{message_[0]}); break;
  case system_reset: config_.system.reset(config_.context, ump::system::reset{message_[0]}); break;
  default:
    config_.utility.unknown(config_.context,
                            std::span{message_.data(), message_size<midi2::ump::message_type::system>()});
    break;
  }
}

// m1cvm message
// ~~~~~~~~~~~~~
// 32 Bit MIDI 1.0 Channel Voice Messages
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::m1cvm_message() {
  static_assert(ump_message_size(midi2::ump::message_type::m1cvm) == 1);
  assert(pos_ >= ump_message_size(midi2::ump::message_type::m1cvm));

  using enum ump::mt::m1cvm;
  switch (static_cast<ump::mt::m1cvm>((message_[0] >> 20) & 0xF)) {
  // 7.3.1 MIDI 1.0 Note Off Message
  case note_off: config_.m1cvm.note_off(config_.context, ump::m1cvm::note_off{message_[0]}); break;
  // 7.3.2 MIDI 1.0 Note On Message
  case note_on: config_.m1cvm.note_on(config_.context, ump::m1cvm::note_on{message_[0]}); break;
  // 7.3.3 MIDI 1.0 Poly Pressure Message
  case poly_pressure: config_.m1cvm.poly_pressure(config_.context, ump::m1cvm::poly_pressure{message_[0]}); break;
  // 7.3.4 MIDI 1.0 Control Change Message
  case cc: config_.m1cvm.control_change(config_.context, ump::m1cvm::control_change{message_[0]}); break;
  // 7.3.5 MIDI 1.0 Program Change Message
  case program_change: config_.m1cvm.program_change(config_.context, ump::m1cvm::program_change{message_[0]}); break;
  // 7.3.6 MIDI 1.0 Channel Pressure Message
  case channel_pressure:
    config_.m1cvm.channel_pressure(config_.context, ump::m1cvm::channel_pressure{message_[0]});
    break;
  // 7.3.7 MIDI 1.0 Pitch Bend Message
  case pitch_bend: config_.m1cvm.pitch_bend(config_.context, ump::m1cvm::pitch_bend{message_[0]}); break;
  default: config_.utility.unknown(config_.context, std::span{message_.data(), 1}); break;
  }
}

// data64 message
// ~~~~~~~~~~~~~~
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::data64_message() {
  constexpr auto size = ump_message_size(midi2::ump::message_type::data64);
  assert(pos_ == size);

  using enum ump::mt::data64;
  auto const span = std::span<std::uint32_t, size>{message_.data(), size};
  switch (static_cast<ump::mt::data64>((message_[0] >> 20) & 0x0F)) {
  case sysex7_in_1: config_.data64.sysex7_in_1(config_.context, ump::data64::sysex7_in_1{span}); break;
  case sysex7_start: config_.data64.sysex7_start(config_.context, ump::data64::sysex7_start{span}); break;
  case sysex7_continue: config_.data64.sysex7_continue(config_.context, ump::data64::sysex7_continue{span}); break;
  case sysex7_end: config_.data64.sysex7_end(config_.context, ump::data64::sysex7_end{span}); break;
  default: config_.utility.unknown(config_.context, span); break;
  }
}

// m2cvm message
// ~~~~~~~~~~~~~
// 64 bit MIDI 2.0 Channel Voice Messages
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::m2cvm_message() {
  static_assert(message_size<midi2::ump::message_type::m2cvm>() == 2);
  auto const span = std::span<std::uint32_t, 2>{message_.data(), 2};
  using enum ump::mt::m2cvm;
  switch (static_cast<ump::mt::m2cvm>((message_[0] >> 20) & 0xF)) {
  // 7.4.1 MIDI 2.0 Note Off Message
  case note_off: config_.m2cvm.note_off(config_.context, ump::m2cvm::note_off{span}); break;
  // 7.4.2 MIDI 2.0 Note On Message
  case note_on: config_.m2cvm.note_on(config_.context, ump::m2cvm::note_on{span}); break;
  // 7.4.3 MIDI 2.0 Poly Pressure Message
  case poly_pressure: config_.m2cvm.poly_pressure(config_.context, ump::m2cvm::poly_pressure{span}); break;
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message
  case rpn_pernote:
    config_.m2cvm.rpn_per_note_controller(config_.context, ump::m2cvm::rpn_per_note_controller{span});
    break;
  // 7.4.4 MIDI 2.0 Assignable Per-Note Controller Message
  case nrpn_pernote:
    config_.m2cvm.nrpn_per_note_controller(config_.context, ump::m2cvm::nrpn_per_note_controller{span});
    break;
  // 7.4.5 MIDI 2.0 Per-Note Management Message
  case pernote_manage: config_.m2cvm.per_note_management(config_.context, ump::m2cvm::per_note_management{span}); break;
  // 7.4.6 MIDI 2.0 Control Change Message
  case cc: config_.m2cvm.control_change(config_.context, ump::m2cvm::control_change{span}); break;
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) and Assignable Controller (NRPN) Message
  case rpn: config_.m2cvm.rpn_controller(config_.context, ump::m2cvm::rpn_controller{span}); break;
  case nrpn: config_.m2cvm.nrpn_controller(config_.context, ump::m2cvm::nrpn_controller{span}); break;
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) and Assignable Controller (NRPN) Message
  case rpn_relative:
    config_.m2cvm.rpn_relative_controller(config_.context, ump::m2cvm::rpn_relative_controller{span});
    break;
  case nrpn_relative:
    config_.m2cvm.nrpn_relative_controller(config_.context, ump::m2cvm::nrpn_relative_controller{span});
    break;
  // 7.4.9 MIDI 2.0 Program Change Message
  case program_change: config_.m2cvm.program_change(config_.context, ump::m2cvm::program_change{span}); break;
  // 7.4.10 MIDI 2.0 Channel Pressure Message
  case channel_pressure: config_.m2cvm.channel_pressure(config_.context, ump::m2cvm::channel_pressure{span}); break;
  // 7.4.11 MIDI 2.0 Pitch Bend Message
  case pitch_bend: config_.m2cvm.pitch_bend(config_.context, ump::m2cvm::pitch_bend{span}); break;
  // 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
  case pitch_bend_pernote:
    config_.m2cvm.per_note_pitch_bend(config_.context, ump::m2cvm::per_note_pitch_bend{span});
    break;
  default: config_.utility.unknown(config_.context, std::span{message_.data(), 2}); break;
  }
}

// ump stream message
// ~~~~~~~~~~~~~~~~~~
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::ump_stream_message() {
  using ump::ump_stream::device_identity_notification;
  using ump::ump_stream::end_of_clip;
  using ump::ump_stream::endpoint_discovery;
  using ump::ump_stream::endpoint_info_notification;
  using ump::ump_stream::endpoint_name_notification;
  using ump::ump_stream::function_block_discovery;
  using ump::ump_stream::function_block_info_notification;
  using ump::ump_stream::function_block_name_notification;
  using ump::ump_stream::jr_configuration_notification;
  using ump::ump_stream::jr_configuration_request;
  using ump::ump_stream::product_instance_id_notification;
  using ump::ump_stream::start_of_clip;

  static_assert(ump_message_size(midi2::ump::message_type::ump_stream) == 4);
  assert(pos_ >= ump_message_size(midi2::ump::message_type::ump_stream));
  auto const span = std::span<std::uint32_t, 4>{message_.data(), 4};
  switch (static_cast<ump::mt::ump_stream>((message_[0] >> 16) & ((std::uint32_t{1} << 10) - 1U))) {
  // 7.1.1 Endpoint Discovery Message
  case ump::mt::ump_stream::endpoint_discovery:
    config_.ump_stream.endpoint_discovery(config_.context, endpoint_discovery{span});
    break;
  // 7.1.2 Endpoint Info Notification Message
  case ump::mt::ump_stream::endpoint_info_notification:
    config_.ump_stream.endpoint_info_notification(config_.context, endpoint_info_notification{span});
    break;
  // 7.1.3 Device Identity Notification Message
  case ump::mt::ump_stream::device_identity_notification:
    config_.ump_stream.device_identity_notification(config_.context, device_identity_notification{span});
    break;
  // 7.1.4 Endpoint Name Notification
  case ump::mt::ump_stream::endpoint_name_notification:
    config_.ump_stream.endpoint_name_notification(config_.context, endpoint_name_notification{span});
    break;
  // 7.1.5 Product Instance Id Notification Message
  case ump::mt::ump_stream::product_instance_id_notification:
    config_.ump_stream.product_instance_id_notification(config_.context, product_instance_id_notification{span});
    break;
  // 7.1.6.2 Stream Configuration Request
  case ump::mt::ump_stream::jr_configuration_request:
    config_.ump_stream.jr_configuration_request(config_.context, jr_configuration_request{span});
    break;
  // 7.1.6.3 Stream Configuration Notification Message
  case ump::mt::ump_stream::jr_configuration_notification:
    config_.ump_stream.jr_configuration_notification(config_.context, jr_configuration_notification{span});
    break;
  // 7.1.7 Function Block Discovery Message
  case ump::mt::ump_stream::function_block_discovery:
    config_.ump_stream.function_block_discovery(config_.context, function_block_discovery{span});
    break;
  // 7.1.8 Function Block Info Notification
  case ump::mt::ump_stream::function_block_info_notification:
    config_.ump_stream.function_block_info_notification(config_.context, function_block_info_notification{span});
    break;
  // 7.1.9 Function Block Name Notification
  case ump::mt::ump_stream::function_block_name_notification:
    config_.ump_stream.function_block_name_notification(config_.context, function_block_name_notification{span});
    break;
  // 7.1.10 Start of Clip Message
  case ump::mt::ump_stream::start_of_clip:
    config_.ump_stream.start_of_clip(config_.context, start_of_clip{span});
    break;
  // 7.1.11 End of Clip Message
  case ump::mt::ump_stream::end_of_clip: config_.ump_stream.end_of_clip(config_.context, end_of_clip{span}); break;
  default: config_.utility.unknown(config_.context, std::span{message_.data(), 4}); break;
  }
}

// data128 message
// ~~~~~~~~~~~~~~~
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::data128_message() {
  static_assert(ump_message_size(midi2::ump::message_type::ump_stream) == 4);
  assert(pos_ >= ump_message_size(midi2::ump::message_type::ump_stream));

  auto const span = std::span<std::uint32_t, 4>{message_.data(), 4};
  using enum ump::mt::data128;
  switch (static_cast<ump::mt::data128>((message_[0] >> 20) & 0x0F)) {
  case sysex8_in_1: config_.data128.sysex8_in_1(config_.context, ump::data128::sysex8_in_1{span}); break;
  case sysex8_start: config_.data128.sysex8_start(config_.context, ump::data128::sysex8_start{span}); break;
  case sysex8_continue: config_.data128.sysex8_continue(config_.context, ump::data128::sysex8_continue{span}); break;
  case sysex8_end: config_.data128.sysex8_end(config_.context, ump::data128::sysex8_end{span}); break;
  case mixed_data_set_header: config_.data128.mds_header(config_.context, ump::data128::mds_header{span}); break;
  case mixed_data_set_payload: config_.data128.mds_payload(config_.context, ump::data128::mds_payload{span}); break;
  default: config_.utility.unknown(config_.context, span); break;
  }
}

// flex data message
// ~~~~~~~~~~~~~~~~~
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::flex_data_message() {
  static_assert(ump_message_size(midi2::ump::message_type::ump_stream) == 4);
  assert(pos_ >= ump_message_size(midi2::ump::message_type::ump_stream));

  auto const span = std::span<std::uint32_t, 4>{message_.data(), 4};
  auto const status_bank = (message_[0] >> 8) & 0xFF;
  if (status_bank == 0) {
    using enum ump::mt::flex_data;
    auto const status = static_cast<ump::mt::flex_data>(message_[0] & 0xFF);
    switch (status) {
    // 7.5.3 Set Tempo Message
    case set_tempo: config_.flex.set_tempo(config_.context, ump::flex_data::set_tempo{span}); break;
    // 7.5.4 Set Time Signature Message
    case set_time_signature:
      config_.flex.set_time_signature(config_.context, ump::flex_data::set_time_signature{span});
      break;
    // 7.5.5 Set Metronome Message
    case set_metronome: config_.flex.set_metronome(config_.context, ump::flex_data::set_metronome{span}); break;
    // 7.5.7 Set Key Signature Message
    case set_key_signature:
      config_.flex.set_key_signature(config_.context, ump::flex_data::set_key_signature{span});
      break;
    // 7.5.8 Set Chord Name Message
    case set_chord_name: config_.flex.set_chord_name(config_.context, ump::flex_data::set_chord_name{span}); break;
    default: config_.utility.unknown(config_.context, span); break;
    }
  } else {
    config_.flex.text(config_.context, ump::flex_data::text_common{span});
  }
}

}  // end namespace midi2

#endif  // MIDI2_UMP_DISPATCHER_HPP
