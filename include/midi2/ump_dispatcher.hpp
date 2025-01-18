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

#include "midi2/ump_dispatcher_backend.hpp"
#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2::ump {

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

template <typename T>
concept ump_dispatcher_config = requires(T v) {
  { v.context };
  { v.utility } -> ump::dispatcher_backend::utility<decltype(v.context)>;
  { v.system } -> dispatcher_backend::system<decltype(v.context)>;
  { v.m1cvm } -> dispatcher_backend::m1cvm<decltype(v.context)>;
  { v.data64 } -> dispatcher_backend::data64<decltype(v.context)>;
  { v.m2cvm } -> dispatcher_backend::m2cvm<decltype(v.context)>;
  { v.data128 } -> dispatcher_backend::data128<decltype(v.context)>;
  { v.stream } -> dispatcher_backend::stream<decltype(v.context)>;
  { v.flex } -> dispatcher_backend::flex_data<decltype(v.context)>;
};

struct default_config {
  struct empty {};
  [[no_unique_address]] empty context{};
  [[no_unique_address]] dispatcher_backend::utility_null<decltype(context)> utility;
  [[no_unique_address]] dispatcher_backend::system_null<decltype(context)> system;
  [[no_unique_address]] dispatcher_backend::m1cvm_null<decltype(context)> m1cvm;
  [[no_unique_address]] dispatcher_backend::data64_null<decltype(context)> data64;
  [[no_unique_address]] dispatcher_backend::m2cvm_null<decltype(context)> m2cvm;
  [[no_unique_address]] dispatcher_backend::data128_null<decltype(context)> data128;
  [[no_unique_address]] dispatcher_backend::stream_null<decltype(context)> stream;
  [[no_unique_address]] dispatcher_backend::flex_data_null<decltype(context)> flex;
};

template <typename Context> struct function_config {
  explicit function_config(Context c) : context{c} {}

  [[no_unique_address]] Context context;
  dispatcher_backend::utility_function<Context> utility;
  dispatcher_backend::system_function<Context> system;
  dispatcher_backend::m1cvm_function<Context> m1cvm;
  dispatcher_backend::data64_function<Context> data64;
  dispatcher_backend::m2cvm_function<Context> m2cvm;
  dispatcher_backend::data128_function<Context> data128;
  dispatcher_backend::stream_function<Context> stream;
  dispatcher_backend::flex_data_function<Context> flex;
};

template <typename T>
concept word_memfun = requires(T a) {
  { a.word() } -> std::convertible_to<std::uint32_t>;
};

template <ump_dispatcher_config Config = default_config> class ump_dispatcher {
public:
  explicit constexpr ump_dispatcher(Config const &config = default_config{}) : config_{config} {}

  void clear() {
    // Note that this member function has to be defined in the class declaration to avoid a spurious GCC
    // warning that the function is defined but not used. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79001>
    pos_ = 0;
    std::ranges::fill(message_, std::uint8_t{0});
  }
  void process_ump(std::uint32_t const ump) {
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
      case ump::message_type::stream: this->stream_message(); break;
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

  [[nodiscard]] constexpr Config &config() noexcept { return config_; }
  [[nodiscard]] constexpr Config const &config() const noexcept { return config_; }

private:
  void utility_message();
  void system_message();
  void m1cvm_message();
  void data64_message();
  void m2cvm_message();
  void stream_message();
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
  case noop:
    config_.utility.noop(config_.context);
    break;
    // 7.2.2.1 JR Clock
  case jr_clock:
    config_.utility.jr_clock(config_.context, ump::utility::jr_clock{span});
    break;
    // 7.2.2.2 JR Timestamp
  case jr_ts:
    config_.utility.jr_timestamp(config_.context, ump::utility::jr_timestamp{span});
    break;
    // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
  case delta_clock_tick:
    config_.utility.delta_clockstamp_tpqn(config_.context, ump::utility::delta_clockstamp_tpqn{message_[0]});
    break;
    // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  case delta_clock_since:
    config_.utility.delta_clockstamp(config_.context, ump::utility::delta_clockstamp{message_[0]});
    break;
  default: config_.utility.unknown(config_.context, span); break;
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
  case note_off:
    config_.m1cvm.note_off(config_.context, ump::m1cvm::note_off{message_[0]});
    break;
    // 7.3.2 MIDI 1.0 Note On Message
  case note_on:
    config_.m1cvm.note_on(config_.context, ump::m1cvm::note_on{message_[0]});
    break;
    // 7.3.3 MIDI 1.0 Poly Pressure Message
  case poly_pressure:
    config_.m1cvm.poly_pressure(config_.context, ump::m1cvm::poly_pressure{message_[0]});
    break;
    // 7.3.4 MIDI 1.0 Control Change Message
  case cc:
    config_.m1cvm.control_change(config_.context, ump::m1cvm::control_change{message_[0]});
    break;
    // 7.3.5 MIDI 1.0 Program Change Message
  case program_change:
    config_.m1cvm.program_change(config_.context, ump::m1cvm::program_change{message_[0]});
    break;
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
  case note_off:
    config_.m2cvm.note_off(config_.context, ump::m2cvm::note_off{span});
    break;
    // 7.4.2 MIDI 2.0 Note On Message
  case note_on:
    config_.m2cvm.note_on(config_.context, ump::m2cvm::note_on{span});
    break;
    // 7.4.3 MIDI 2.0 Poly Pressure Message
  case poly_pressure:
    config_.m2cvm.poly_pressure(config_.context, ump::m2cvm::poly_pressure{span});
    break;
    // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message
  case rpn_pernote:
    config_.m2cvm.rpn_per_note_controller(config_.context, ump::m2cvm::rpn_per_note_controller{span});
    break;
    // 7.4.4 MIDI 2.0 Assignable Per-Note Controller Message
  case nrpn_pernote:
    config_.m2cvm.nrpn_per_note_controller(config_.context, ump::m2cvm::nrpn_per_note_controller{span});
    break;
    // 7.4.5 MIDI 2.0 Per-Note Management Message
  case pernote_manage:
    config_.m2cvm.per_note_management(config_.context, ump::m2cvm::per_note_management{span});
    break;
    // 7.4.6 MIDI 2.0 Control Change Message
  case cc:
    config_.m2cvm.control_change(config_.context, ump::m2cvm::control_change{span});
    break;
    // 7.4.7 MIDI 2.0 Registered Controller (RPN) and Assignable Controller (NRPN) Message
  case rpn: config_.m2cvm.rpn_controller(config_.context, ump::m2cvm::rpn_controller{span}); break;
  case nrpn:
    config_.m2cvm.nrpn_controller(config_.context, ump::m2cvm::nrpn_controller{span});
    break;
    // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) and Assignable Controller (NRPN) Message
  case rpn_relative:
    config_.m2cvm.rpn_relative_controller(config_.context, ump::m2cvm::rpn_relative_controller{span});
    break;
  case nrpn_relative:
    config_.m2cvm.nrpn_relative_controller(config_.context, ump::m2cvm::nrpn_relative_controller{span});
    break;
    // 7.4.9 MIDI 2.0 Program Change Message
  case program_change:
    config_.m2cvm.program_change(config_.context, ump::m2cvm::program_change{span});
    break;
    // 7.4.10 MIDI 2.0 Channel Pressure Message
  case channel_pressure:
    config_.m2cvm.channel_pressure(config_.context, ump::m2cvm::channel_pressure{span});
    break;
    // 7.4.11 MIDI 2.0 Pitch Bend Message
  case pitch_bend:
    config_.m2cvm.pitch_bend(config_.context, ump::m2cvm::pitch_bend{span});
    break;
    // 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
  case pitch_bend_pernote:
    config_.m2cvm.per_note_pitch_bend(config_.context, ump::m2cvm::per_note_pitch_bend{span});
    break;
  default: config_.utility.unknown(config_.context, std::span{message_.data(), 2}); break;
  }
}

// ump stream message
// ~~~~~~~~~~~~~~~~~~
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::stream_message() {
  using ump::stream::device_identity_notification;
  using ump::stream::end_of_clip;
  using ump::stream::endpoint_discovery;
  using ump::stream::endpoint_info_notification;
  using ump::stream::endpoint_name_notification;
  using ump::stream::function_block_discovery;
  using ump::stream::function_block_info_notification;
  using ump::stream::function_block_name_notification;
  using ump::stream::jr_configuration_notification;
  using ump::stream::jr_configuration_request;
  using ump::stream::product_instance_id_notification;
  using ump::stream::start_of_clip;

  static_assert(ump_message_size(midi2::ump::message_type::stream) == 4);
  assert(pos_ >= ump_message_size(midi2::ump::message_type::stream));
  auto const span = std::span<std::uint32_t, 4>{message_.data(), 4};
  switch (static_cast<ump::mt::stream>((message_[0] >> 16) & ((std::uint32_t{1} << 10) - 1U))) {
    // 7.1.1 Endpoint Discovery Message
  case ump::mt::stream::endpoint_discovery:
    config_.stream.endpoint_discovery(config_.context, endpoint_discovery{span});
    break;
    // 7.1.2 Endpoint Info Notification Message
  case ump::mt::stream::endpoint_info_notification:
    config_.stream.endpoint_info_notification(config_.context, endpoint_info_notification{span});
    break;
    // 7.1.3 Device Identity Notification Message
  case ump::mt::stream::device_identity_notification:
    config_.stream.device_identity_notification(config_.context, device_identity_notification{span});
    break;
    // 7.1.4 Endpoint Name Notification
  case ump::mt::stream::endpoint_name_notification:
    config_.stream.endpoint_name_notification(config_.context, endpoint_name_notification{span});
    break;
    // 7.1.5 Product Instance Id Notification Message
  case ump::mt::stream::product_instance_id_notification:
    config_.stream.product_instance_id_notification(config_.context, product_instance_id_notification{span});
    break;
    // 7.1.6.2 Stream Configuration Request
  case ump::mt::stream::jr_configuration_request:
    config_.stream.jr_configuration_request(config_.context, jr_configuration_request{span});
    break;
    // 7.1.6.3 Stream Configuration Notification Message
  case ump::mt::stream::jr_configuration_notification:
    config_.stream.jr_configuration_notification(config_.context, jr_configuration_notification{span});
    break;
    // 7.1.7 Function Block Discovery Message
  case ump::mt::stream::function_block_discovery:
    config_.stream.function_block_discovery(config_.context, function_block_discovery{span});
    break;
    // 7.1.8 Function Block Info Notification
  case ump::mt::stream::function_block_info_notification:
    config_.stream.function_block_info_notification(config_.context, function_block_info_notification{span});
    break;
    // 7.1.9 Function Block Name Notification
  case ump::mt::stream::function_block_name_notification:
    config_.stream.function_block_name_notification(config_.context, function_block_name_notification{span});
    break;
    // 7.1.10 Start of Clip Message
  case ump::mt::stream::start_of_clip:
    config_.stream.start_of_clip(config_.context, start_of_clip{span});
    break;
    // 7.1.11 End of Clip Message
  case ump::mt::stream::end_of_clip: config_.stream.end_of_clip(config_.context, end_of_clip{span}); break;
  default: config_.utility.unknown(config_.context, std::span{message_.data(), 4}); break;
  }
}

// data128 message
// ~~~~~~~~~~~~~~~
template <ump_dispatcher_config Config> void ump_dispatcher<Config>::data128_message() {
  static_assert(ump_message_size(midi2::ump::message_type::stream) == 4);
  assert(pos_ >= ump_message_size(midi2::ump::message_type::stream));

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
  static_assert(ump_message_size(midi2::ump::message_type::stream) == 4);
  assert(pos_ >= ump_message_size(midi2::ump::message_type::stream));

  auto const span = std::span<std::uint32_t, 4>{message_.data(), 4};
  auto const status_bank = (message_[0] >> 8) & 0xFF;
  if (status_bank == 0) {
    using enum ump::mt::flex_data;
    auto const status = static_cast<ump::mt::flex_data>(message_[0] & 0xFF);
    switch (status) {
      // 7.5.3 Set Tempo Message
    case set_tempo:
      config_.flex.set_tempo(config_.context, ump::flex_data::set_tempo{span});
      break;
      // 7.5.4 Set Time Signature Message
    case set_time_signature:
      config_.flex.set_time_signature(config_.context, ump::flex_data::set_time_signature{span});
      break;
      // 7.5.5 Set Metronome Message
    case set_metronome:
      config_.flex.set_metronome(config_.context, ump::flex_data::set_metronome{span});
      break;
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

template <typename Context>
ump_dispatcher<function_config<Context>> make_ump_function_dispatcher(Context context = Context{}) {
  return ump_dispatcher{function_config{context}};
}

}  // end namespace midi2::ump

#endif  // MIDI2_UMP_DISPATCHER_HPP
