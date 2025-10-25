//===-- UMP Dispatcher --------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ump_dispatcher.hpp
/// \brief Defines the UMP dispatcher

#ifndef MIDI2_UMP_DISPATCHER_HPP
#define MIDI2_UMP_DISPATCHER_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <functional>
#include <span>

#include "midi2/dispatcher.hpp"
#include "midi2/ump/ump_dispatcher_backend.hpp"
#include "midi2/ump/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2::ump {

// See M2-104-UM (UMP Format & MIDI 2.0 Protocol v.1.1.2 2023-10-27)
//    Table 4 Message Type (MT) Allocation

[[nodiscard]] constexpr unsigned ump_message_size(message_type const mt) {
  using enum message_type;
#define MIDI2_X(a, b) \
  case a: return message_size<a>();
  switch (mt) {
    MIDI2_UMP_MESSAGE_TYPES
  default: return 0U;
  }
#undef MIDI2_X
}

template <typename T>
concept ump_dispatcher_config = requires(T v) {
  { v.context };
  { v.utility } -> dispatcher_backend::utility<decltype(v.context)>;
  { v.system } -> dispatcher_backend::system<decltype(v.context)>;
  { v.m1cvm } -> dispatcher_backend::m1cvm<decltype(v.context)>;
  { v.data64 } -> dispatcher_backend::data64<decltype(v.context)>;
  { v.m2cvm } -> dispatcher_backend::m2cvm<decltype(v.context)>;
  { v.data128 } -> dispatcher_backend::data128<decltype(v.context)>;
  { v.stream } -> dispatcher_backend::stream<decltype(v.context)>;
  { v.flex } -> dispatcher_backend::flex_data<decltype(v.context)>;
};

/// A configuration type for the ump_dispatcher which uses std::function<> for all the available callbacks.
/// \note This is probably the simplest possible configuration type to use, but may not always be the most time and
///   space efficient. Use judiciously!
///
/// \tparam Context  The type of the context object. This is passed to the callbacks to enable sharing
///   of context.
template <typename Context> struct function_config {
  explicit function_config(Context c) : context{std::move(c)} {}

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

template <typename Config>
  requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
class ump_dispatcher {
public:
  /// Type of input messages
  using input_type = std::uint32_t;
  using config_type = std::remove_reference_t<std::unwrap_reference_t<Config>>;

  template <typename OtherConfig>
    requires std::convertible_to<OtherConfig, Config>
  constexpr explicit ump_dispatcher(OtherConfig&& config) : config_{std::forward<OtherConfig>(config)} {
    static_assert(midi2::dispatcher<Config, std::uint32_t, decltype(*this)>);
  }

  void reset() {
    // Note that this member function has to be defined in the class declaration to avoid a spurious GCC
    // warning that the function is defined but not used. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79001>
    pos_ = 0;
    std::ranges::fill(message_, std::uint8_t{0});
  }
  void dispatch(std::uint32_t const ump) {
    // Note that this member function has to be defined in the class declaration to avoid a spurious GCC
    // warning that the function is defined but not used. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79001>
    assert(pos_ < message_.size());
    message_[pos_] = ump;
    ++pos_;
    if (auto const mt = static_cast<message_type>((message_[0] >> 28) & 0xF); pos_ >= ump_message_size(mt)) {
      using enum message_type;
      switch (mt) {
      case utility: this->utility_message(); break;
      case system: this->system_message(); break;
      case m1cvm: this->m1cvm_message(); break;
      case m2cvm: this->m2cvm_message(); break;
      case flex_data: this->flex_data_message(); break;
      case stream: this->stream_message(); break;
      case data64: this->data64_message(); break;
      case data128: this->data128_message(); break;

      case reserved32_06:
      case reserved32_07:
      case reserved64_08:
      case reserved64_09:
      case reserved64_0a:
      case reserved96_0b:
      case reserved96_0c:
      case reserved128_0e: this->unknown(); break;
      default:
        assert(false);
        unreachable();
        break;
      }
      pos_ = 0;
    }
  }

  [[nodiscard]] constexpr config_type& config() noexcept { return config_; }
  [[nodiscard]] constexpr config_type const& config() const noexcept { return config_; }

private:
  void utility_message();
  void system_message();
  void m1cvm_message();
  void data64_message();
  void m2cvm_message();
  void stream_message();
  void data128_message();
  void flex_data_message();
  void unknown() {
    auto& c = this->config();
    c.utility.unknown(c.context, std::span{message_.data(), pos_});
  }
  // TODO: replace message_/pos_ with inplace_vector<> eventually.
  std::array<std::uint32_t, 4> message_{};
  std::uint8_t pos_ = 0;

  [[no_unique_address]] Config config_;
};

template <typename T> ump_dispatcher(T) -> ump_dispatcher<T>;

// utility message
// ~~~~~~~~~~~~~~~
// 32 bit utility messages
template <typename Config>
  requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
void ump_dispatcher<Config>::utility_message() {
  constexpr auto size = ump_message_size(message_type::utility);
  auto const span = std::span<std::uint32_t, size>{message_.data(), size};
  assert(pos_ == size);

  auto& c = this->config();
  using enum mt::utility;
  switch (static_cast<mt::utility>((message_[0] >> 20) & 0x0F)) {
    // 7.2.1 NOOP
  case noop:
    c.utility.noop(c.context);
    break;
    // 7.2.2.1 JR Clock
  case jr_clock:
    c.utility.jr_clock(c.context, utility::jr_clock{span});
    break;
    // 7.2.2.2 JR Timestamp
  case jr_ts:
    c.utility.jr_timestamp(c.context, utility::jr_timestamp{span});
    break;
    // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
  case delta_clock_tick:
    c.utility.delta_clockstamp_tpqn(c.context, utility::delta_clockstamp_tpqn{span});
    break;
    // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  case delta_clock_since: c.utility.delta_clockstamp(c.context, utility::delta_clockstamp{span}); break;
  default: c.utility.unknown(c.context, span); break;
  }
}

// system message
// ~~~~~~~~~~~~~~
// 32-bit System Common and Real Time
template <typename Config>
  requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
void ump_dispatcher<Config>::system_message() {
  constexpr auto size = ump_message_size(message_type::system);
  auto const span = std::span<std::uint32_t, size>{message_.data(), size};
  assert(pos_ == span.size());

  using enum mt::system_crt;
  auto& c = this->config();
  switch (static_cast<mt::system_crt>((message_[0] >> 16) & 0xFF)) {
  case timing_code: c.system.midi_time_code(c.context, system::midi_time_code{span}); break;
  case spp: c.system.song_position_pointer(c.context, system::song_position_pointer{span}); break;
  case song_select: c.system.song_select(c.context, system::song_select{span}); break;
  case tune_request: c.system.tune_request(c.context, system::tune_request{span}); break;
  case timing_clock: c.system.timing_clock(c.context, system::timing_clock{span}); break;
  case sequence_start: c.system.seq_start(c.context, system::sequence_start{span}); break;
  case sequence_continue: c.system.seq_continue(c.context, system::sequence_continue{span}); break;
  case sequence_stop: c.system.seq_stop(c.context, system::sequence_stop{span}); break;
  case active_sensing: c.system.active_sensing(c.context, system::active_sensing{span}); break;
  case system_reset: c.system.reset(c.context, system::reset{span}); break;
  default: c.utility.unknown(c.context, span); break;
  }
}

// m1cvm message
// ~~~~~~~~~~~~~
// 32 Bit MIDI 1.0 Channel Voice Messages
template <typename Config>
  requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
void ump_dispatcher<Config>::m1cvm_message() {
  constexpr auto size = ump_message_size(message_type::system);
  auto const span = std::span<std::uint32_t, size>{message_.data(), size};
  assert(pos_ == span.size());

  using enum mt::m1cvm;
  auto& c = this->config();
  switch (static_cast<mt::m1cvm>((message_[0] >> 20) & 0xF)) {
  case note_off: c.m1cvm.note_off(c.context, m1cvm::note_off{span}); break;
  case note_on: c.m1cvm.note_on(c.context, m1cvm::note_on{span}); break;
  case poly_pressure: c.m1cvm.poly_pressure(c.context, m1cvm::poly_pressure{span}); break;
  case cc: c.m1cvm.control_change(c.context, m1cvm::control_change{span}); break;
  case program_change: c.m1cvm.program_change(c.context, m1cvm::program_change{span}); break;
  case channel_pressure: c.m1cvm.channel_pressure(c.context, m1cvm::channel_pressure{span}); break;
  case pitch_bend: c.m1cvm.pitch_bend(c.context, m1cvm::pitch_bend{span}); break;
  default: c.utility.unknown(c.context, span); break;
  }
}

// data64 message
// ~~~~~~~~~~~~~~
template <typename Config>
  requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
void ump_dispatcher<Config>::data64_message() {
  constexpr auto size = ump_message_size(message_type::data64);
  assert(pos_ == size);

  using enum mt::data64;
  auto& c = this->config();
  auto const span = std::span<std::uint32_t, size>{message_.data(), size};
  switch (static_cast<mt::data64>((message_[0] >> 20) & 0x0F)) {
  case sysex7_in_1: c.data64.sysex7_in_1(c.context, data64::sysex7_in_1{span}); break;
  case sysex7_start: c.data64.sysex7_start(c.context, data64::sysex7_start{span}); break;
  case sysex7_continue: c.data64.sysex7_continue(c.context, data64::sysex7_continue{span}); break;
  case sysex7_end: c.data64.sysex7_end(c.context, data64::sysex7_end{span}); break;
  default: c.utility.unknown(c.context, span); break;
  }
}

// m2cvm message
// ~~~~~~~~~~~~~
// 64 bit MIDI 2.0 Channel Voice Messages
template <typename Config>
  requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
void ump_dispatcher<Config>::m2cvm_message() {
  static_assert(message_size<message_type::m2cvm>() == 2);
  auto const span = std::span<std::uint32_t, 2>{message_.data(), 2};
  using enum mt::m2cvm;
  auto& c = this->config();
  switch (static_cast<mt::m2cvm>((message_[0] >> 20) & 0xF)) {
    // 7.4.1 MIDI 2.0 Note Off Message
  case note_off:
    c.m2cvm.note_off(c.context, m2cvm::note_off{span});
    break;
    // 7.4.2 MIDI 2.0 Note On Message
  case note_on:
    c.m2cvm.note_on(c.context, m2cvm::note_on{span});
    break;
    // 7.4.3 MIDI 2.0 Poly Pressure Message
  case poly_pressure:
    c.m2cvm.poly_pressure(c.context, m2cvm::poly_pressure{span});
    break;
    // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message
  case rpn_per_note:
    c.m2cvm.rpn_per_note_controller(c.context, m2cvm::rpn_per_note_controller{span});
    break;
    // 7.4.4 MIDI 2.0 Assignable Per-Note Controller Message
  case nrpn_per_note:
    c.m2cvm.nrpn_per_note_controller(c.context, m2cvm::nrpn_per_note_controller{span});
    break;
    // 7.4.5 MIDI 2.0 Per-Note Management Message
  case per_note_manage:
    c.m2cvm.per_note_management(c.context, m2cvm::per_note_management{span});
    break;
    // 7.4.6 MIDI 2.0 Control Change Message
  case cc:
    c.m2cvm.control_change(c.context, m2cvm::control_change{span});
    break;
    // 7.4.7 MIDI 2.0 Registered Controller (RPN) and Assignable Controller (NRPN) Message
  case rpn: c.m2cvm.rpn_controller(c.context, m2cvm::rpn_controller{span}); break;
  case nrpn:
    c.m2cvm.nrpn_controller(c.context, m2cvm::nrpn_controller{span});
    break;
    // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) and Assignable Controller (NRPN) Message
  case rpn_relative: c.m2cvm.rpn_relative_controller(c.context, m2cvm::rpn_relative_controller{span}); break;
  case nrpn_relative:
    c.m2cvm.nrpn_relative_controller(c.context, m2cvm::nrpn_relative_controller{span});
    break;
    // 7.4.9 MIDI 2.0 Program Change Message
  case program_change:
    c.m2cvm.program_change(c.context, m2cvm::program_change{span});
    break;
    // 7.4.10 MIDI 2.0 Channel Pressure Message
  case channel_pressure:
    c.m2cvm.channel_pressure(c.context, m2cvm::channel_pressure{span});
    break;
    // 7.4.11 MIDI 2.0 Pitch Bend Message
  case pitch_bend:
    c.m2cvm.pitch_bend(c.context, m2cvm::pitch_bend{span});
    break;
    // 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
  case pitch_bend_per_note: c.m2cvm.per_note_pitch_bend(c.context, m2cvm::per_note_pitch_bend{span}); break;
  default: c.utility.unknown(c.context, std::span{message_.data(), 2}); break;
  }
}

// ump stream message
// ~~~~~~~~~~~~~~~~~~
template <typename Config>
  requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
void ump_dispatcher<Config>::stream_message() {
  using stream::device_identity_notification;
  using stream::end_of_clip;
  using stream::endpoint_discovery;
  using stream::endpoint_info_notification;
  using stream::endpoint_name_notification;
  using stream::function_block_discovery;
  using stream::function_block_info_notification;
  using stream::function_block_name_notification;
  using stream::jr_configuration_notification;
  using stream::jr_configuration_request;
  using stream::product_instance_id_notification;
  using stream::start_of_clip;

  static_assert(ump_message_size(message_type::stream) == 4);
  assert(pos_ >= ump_message_size(message_type::stream));
  auto& c = this->config();
  auto const span = std::span<std::uint32_t, 4>{message_.data(), 4};
  switch (static_cast<mt::stream>((message_[0] >> 16) & ((std::uint32_t{1} << 10) - 1U))) {
    // 7.1.1 Endpoint Discovery Message
  case mt::stream::endpoint_discovery:
    c.stream.endpoint_discovery(c.context, endpoint_discovery{span});
    break;
    // 7.1.2 Endpoint Info Notification Message
  case mt::stream::endpoint_info_notification:
    c.stream.endpoint_info_notification(c.context, endpoint_info_notification{span});
    break;
    // 7.1.3 Device Identity Notification Message
  case mt::stream::device_identity_notification:
    c.stream.device_identity_notification(c.context, device_identity_notification{span});
    break;
    // 7.1.4 Endpoint Name Notification
  case mt::stream::endpoint_name_notification:
    c.stream.endpoint_name_notification(c.context, endpoint_name_notification{span});
    break;
    // 7.1.5 Product Instance Id Notification Message
  case mt::stream::product_instance_id_notification:
    c.stream.product_instance_id_notification(c.context, product_instance_id_notification{span});
    break;
    // 7.1.6.2 Stream Configuration Request
  case mt::stream::jr_configuration_request:
    c.stream.jr_configuration_request(c.context, jr_configuration_request{span});
    break;
    // 7.1.6.3 Stream Configuration Notification Message
  case mt::stream::jr_configuration_notification:
    c.stream.jr_configuration_notification(c.context, jr_configuration_notification{span});
    break;
    // 7.1.7 Function Block Discovery Message
  case mt::stream::function_block_discovery:
    c.stream.function_block_discovery(c.context, function_block_discovery{span});
    break;
    // 7.1.8 Function Block Info Notification
  case mt::stream::function_block_info_notification:
    c.stream.function_block_info_notification(c.context, function_block_info_notification{span});
    break;
    // 7.1.9 Function Block Name Notification
  case mt::stream::function_block_name_notification:
    c.stream.function_block_name_notification(c.context, function_block_name_notification{span});
    break;
    // 7.1.10 Start of Clip Message
  case mt::stream::start_of_clip:
    c.stream.start_of_clip(c.context, start_of_clip{span});
    break;
    // 7.1.11 End of Clip Message
  case mt::stream::end_of_clip: c.stream.end_of_clip(c.context, end_of_clip{span}); break;
  default: c.utility.unknown(c.context, std::span{message_.data(), 4}); break;
  }
}

// data128 message
// ~~~~~~~~~~~~~~~
template <typename Config>
  requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
void ump_dispatcher<Config>::data128_message() {
  static_assert(ump_message_size(message_type::stream) == 4);
  assert(pos_ >= ump_message_size(message_type::stream));

  auto const span = std::span<std::uint32_t, 4>{message_.data(), 4};
  using enum mt::data128;
  auto& c = this->config();
  switch (static_cast<mt::data128>((message_[0] >> 20) & 0x0F)) {
  case sysex8_in_1: c.data128.sysex8_in_1(c.context, data128::sysex8_in_1{span}); break;
  case sysex8_start: c.data128.sysex8_start(c.context, data128::sysex8_start{span}); break;
  case sysex8_continue: c.data128.sysex8_continue(c.context, data128::sysex8_continue{span}); break;
  case sysex8_end: c.data128.sysex8_end(c.context, data128::sysex8_end{span}); break;
  case mixed_data_set_header: c.data128.mds_header(c.context, data128::mds_header{span}); break;
  case mixed_data_set_payload: c.data128.mds_payload(c.context, data128::mds_payload{span}); break;
  default: c.utility.unknown(c.context, span); break;
  }
}

// flex data message
// ~~~~~~~~~~~~~~~~~
template <typename Config>
  requires ump_dispatcher_config<std::unwrap_reference_t<Config>>
void ump_dispatcher<Config>::flex_data_message() {
  static_assert(ump_message_size(message_type::stream) == 4);
  assert(pos_ >= ump_message_size(message_type::stream));

  auto const span = std::span<std::uint32_t, 4>{message_.data(), 4};
  auto const status_bank = (message_[0] >> 8) & 0xFF;
  auto& c = this->config();
  if (status_bank == 0) {
    using enum mt::flex_data;
    switch (auto const status = static_cast<mt::flex_data>(message_[0] & 0xFF); status) {
      // 7.5.3 Set Tempo Message
    case set_tempo:
      c.flex.set_tempo(c.context, flex_data::set_tempo{span});
      break;
      // 7.5.4 Set Time Signature Message
    case set_time_signature:
      c.flex.set_time_signature(c.context, flex_data::set_time_signature{span});
      break;
      // 7.5.5 Set Metronome Message
    case set_metronome:
      c.flex.set_metronome(c.context, flex_data::set_metronome{span});
      break;
      // 7.5.7 Set Key Signature Message
    case set_key_signature:
      c.flex.set_key_signature(c.context, flex_data::set_key_signature{span});
      break;
      // 7.5.8 Set Chord Name Message
    case set_chord_name: c.flex.set_chord_name(c.context, flex_data::set_chord_name{span}); break;
    default: c.utility.unknown(c.context, span); break;
    }
  } else {
    c.flex.text(c.context, flex_data::text_common{span});
  }
}

template <typename Context>
ump_dispatcher<function_config<Context>> make_ump_function_dispatcher(Context&& context = Context{}) {
  return ump_dispatcher{function_config{std::forward<Context>(context)}};
}

}  // end namespace midi2::ump

#endif  // MIDI2_UMP_DISPATCHER_HPP
