//===-- UMP Processor ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMP_PROCESSOR_HPP
#define MIDI2_UMP_PROCESSOR_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iterator>
#include <span>
#include <type_traits>

#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

struct ump_common {
  constexpr bool operator==(ump_common const&) const = default;

  uint8_t group = 255;
  ump_message_type messageType = ump_message_type::utility;
  uint8_t status = 0;
};
struct ump_data {
  ump_common common;
  uint8_t streamId = 0;
  uint8_t form = 0;
  std::span<std::uint8_t> data;
};

// See M2-104-UM (UMP Format & MIDI 2.0 Protocol v.1.1.2 2023-10-27)
//    Table 4 Message Type (MT) Allocation
template <midi2::ump_message_type> struct message_size {};
template <> struct message_size<midi2::ump_message_type::utility> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<midi2::ump_message_type::system> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<midi2::ump_message_type::m1cvm> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<midi2::ump_message_type::data64> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<midi2::ump_message_type::m2cvm> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<midi2::ump_message_type::data128> : std::integral_constant<unsigned, 4> {};
template <> struct message_size<midi2::ump_message_type::reserved32_06> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<midi2::ump_message_type::reserved32_07> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<midi2::ump_message_type::reserved64_08> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<midi2::ump_message_type::reserved64_09> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<midi2::ump_message_type::reserved64_0A> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<midi2::ump_message_type::reserved96_0B> : std::integral_constant<unsigned, 3> {};
template <> struct message_size<midi2::ump_message_type::reserved96_0C> : std::integral_constant<unsigned, 3> {};
template <> struct message_size<midi2::ump_message_type::flex_data> : std::integral_constant<unsigned, 4> {};
template <> struct message_size<midi2::ump_message_type::reserved128_0E> : std::integral_constant<unsigned, 4> {};
template <> struct message_size<midi2::ump_message_type::ump_stream> : std::integral_constant<unsigned, 4> {};

constexpr unsigned ump_message_size(ump_message_type const mt) {
  using enum ump_message_type;
#define X(a, b) \
  case a: return message_size<a>();
  switch (mt) {
    UMP_MESSAGE_TYPES
  default: return 0U;
  }
#undef X
}

// clang-format off
template <typename T> concept backend = requires(T && v) {
  { v.system(midi2::types::system_general{}) } -> std::same_as<void>;

  { v.unknown(std::span<std::uint32_t>{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept utility_backend = requires(T v, Context context) {
  { v.noop(context) } -> std::same_as<void>;
  { v.jr_clock(context, types::jr_clock{}) } -> std::same_as<void>;
  { v.jr_timestamp(context, types::jr_clock{}) } -> std::same_as<void>;
  { v.delta_clockstamp_tpqn(context, types::jr_clock{}) } -> std::same_as<void>;
  { v.delta_clockstamp(context, types::delta_clockstamp{}) } -> std::same_as<void>;
};
template<typename T, typename Context>
concept m1cvm_backend = requires(T v, Context context) {
  { v.note_off(context, types::m1cvm_w0{}) } -> std::same_as<void>;
  { v.note_on(context, types::m1cvm_w0{}) } -> std::same_as<void>;
  { v.poly_pressure(context, types::m1cvm_w0{}) } -> std::same_as<void>;
  { v.control_change(context, types::m1cvm_w0{}) } -> std::same_as<void>;
  { v.program_change(context, types::m1cvm_w0{}) } -> std::same_as<void>;
  { v.channel_pressure (context, types::m1cvm_w0{}) } -> std::same_as<void>;
  { v.pitch_bend (context, types::m1cvm_w0{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept data64_backend = requires(T v, Context context) {
  { v.sysex7_in_1(context, types::data64::sysex7_w0{}, types::data64::sysex7_w1{}) } -> std::same_as<void>;
  { v.sysex7_start(context, types::data64::sysex7_w0{}, types::data64::sysex7_w1{}) } -> std::same_as<void>;
  { v.sysex7_continue(context, types::data64::sysex7_w0{}, types::data64::sysex7_w1{}) } -> std::same_as<void>;
  { v.sysex7_end(context, types::data64::sysex7_w0{}, types::data64::sysex7_w1{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept m2cvm_backend = requires(T v, Context context) {
  { v.note_off(context, types::m2cvm::note_w0{}, types::m2cvm::note_w1{}) } -> std::same_as<void>;
  { v.note_on(context, types::m2cvm::note_w0{}, types::m2cvm::note_w1{}) } -> std::same_as<void>;
  { v.poly_pressure(context, types::m2cvm::poly_pressure_w0{}, std::uint32_t{}) } -> std::same_as<void>;
  { v.program_change(context, types::m2cvm::program_change_w0{}, types::m2cvm::program_change_w1{}) } -> std::same_as<void>;
  { v.channel_pressure(context, types::m2cvm::channel_pressure_w0{}, std::uint32_t{}) } -> std::same_as<void>;
  { v.rpn_controller(context, types::m2cvm::controller_w0{}, std::uint32_t{}) } -> std::same_as<void>;
  { v.nrpn_controller(context, types::m2cvm::controller_w0{}, std::uint32_t{}) } -> std::same_as<void>;
  { v.per_note_management(context, types::m2cvm::per_note_management_w0{}, std::uint32_t{}) } -> std::same_as<void>;
  { v.control_change(context, types::m2cvm::control_change_w0{}, std::uint32_t{}) } -> std::same_as<void>;
  { v.controller_message(context, types::m2cvm::controller_message_w0{}, std::uint32_t{}) } -> std::same_as<void>;
  { v.pitch_bend(context, types::m2cvm::pitch_bend_w0{}, std::uint32_t{}) } -> std::same_as<void>;
  { v.per_note_pitch_bend(context, types::m2cvm::per_note_pitch_bend_w0{}, std::uint32_t{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept data128_backend = requires(T v, Context context) {
  { v.sysex8_in_1(context,
      types::data128::sysex8_w0{},
      types::data128::sysex8_w1{},
      types::data128::sysex8_w2{},
      types::data128::sysex8_w3{}) } -> std::same_as<void>;
  { v.sysex8_start(context,
      types::data128::sysex8_w0{},
      types::data128::sysex8_w1{},
      types::data128::sysex8_w2{},
      types::data128::sysex8_w3{}) } -> std::same_as<void>;
  { v.sysex8_continue(context,
      types::data128::sysex8_w0{},
      types::data128::sysex8_w1{},
      types::data128::sysex8_w2{},
      types::data128::sysex8_w3{}) } -> std::same_as<void>;
  { v.sysex8_end(context,
      types::data128::sysex8_w0{},
      types::data128::sysex8_w1{},
      types::data128::sysex8_w2{},
      types::data128::sysex8_w3{}) } -> std::same_as<void>;
  // TODO: mixed data set header
  // TODO: mixed data set payload
};
template <typename T, typename Context>
concept ump_stream_backend = requires(T v, Context context) {
  { v.endpoint_discovery(context,
      types::ump_stream::endpoint_discovery_w0{},
      types::ump_stream::endpoint_discovery_w1{},
      types::ump_stream::endpoint_discovery_w2{},
      types::ump_stream::endpoint_discovery_w3{}) } -> std::same_as<void>;
  { v.endpoint_info_notification(context,
      types::ump_stream::endpoint_info_notification_w0{},
      types::ump_stream::endpoint_info_notification_w1{},
      types::ump_stream::endpoint_info_notification_w2{},
      types::ump_stream::endpoint_info_notification_w3{}) } -> std::same_as<void>;
  { v.device_identity_notification(context,
      types::ump_stream::device_identity_notification_w0{},
      types::ump_stream::device_identity_notification_w1{},
      types::ump_stream::device_identity_notification_w2{},
      types::ump_stream::device_identity_notification_w3{}) } -> std::same_as<void>;
  { v.endpoint_name_notification(context,
      types::ump_stream::endpoint_name_notification_w0{},
      types::ump_stream::endpoint_name_notification_w1{},
      types::ump_stream::endpoint_name_notification_w2{},
      types::ump_stream::endpoint_name_notification_w3{}) } -> std::same_as<void>;
  { v.product_instance_id_notification(context,
      types::ump_stream::product_instance_id_notification_w0{},
      types::ump_stream::product_instance_id_notification_w1{},
      types::ump_stream::product_instance_id_notification_w2{},
      types::ump_stream::product_instance_id_notification_w3{}) } -> std::same_as<void>;
  { v.jr_configuration_request(context,
      types::ump_stream::jr_configuration_request_w0{},
      types::ump_stream::jr_configuration_request_w1{},
      types::ump_stream::jr_configuration_request_w2{},
      types::ump_stream::jr_configuration_request_w3{}) } -> std::same_as<void>;
  { v.jr_configuration_notification(context,
      types::ump_stream::jr_configuration_notification_w0{},
      types::ump_stream::jr_configuration_notification_w1{},
      types::ump_stream::jr_configuration_notification_w2{},
      types::ump_stream::jr_configuration_notification_w3{}) } -> std::same_as<void>;
  { v.function_block_discovery(context,
      types::ump_stream::function_block_discovery_w0{},
      types::ump_stream::function_block_discovery_w1{},
      types::ump_stream::function_block_discovery_w2{},
      types::ump_stream::function_block_discovery_w3{}) } -> std::same_as<void>;
  { v.function_block_info_notification(context,
      types::ump_stream::function_block_info_notification_w0{},
      types::ump_stream::function_block_info_notification_w1{},
      types::ump_stream::function_block_info_notification_w2{},
      types::ump_stream::function_block_info_notification_w3{}) } -> std::same_as<void>;
  { v.function_block_name_notification(context,
      types::ump_stream::function_block_name_notification_w0{},
      types::ump_stream::function_block_name_notification_w1{},
      types::ump_stream::function_block_name_notification_w2{},
      types::ump_stream::function_block_name_notification_w3{}) } -> std::same_as<void>;
  { v.start_of_clip(context,
      types::ump_stream::start_of_clip_w0{},
      types::ump_stream::start_of_clip_w1{},
      types::ump_stream::start_of_clip_w2{},
      types::ump_stream::start_of_clip_w3{}) } -> std::same_as<void>;
  { v.end_of_clip(context,
      types::ump_stream::end_of_clip_w0{},
      types::ump_stream::end_of_clip_w1{},
      types::ump_stream::end_of_clip_w2{},
      types::ump_stream::end_of_clip_w3{}) } -> std::same_as<void>;
};
template <typename T, typename Context>
concept flex_data_backend = requires(T v, Context context) {
  { v.set_tempo(context,
      types::flex_data::set_tempo_w0{},
      types::flex_data::set_tempo_w1{},
      types::flex_data::set_tempo_w2{},
      types::flex_data::set_tempo_w3{}) } -> std::same_as<void>;
  { v.set_time_signature(context,
      types::flex_data::set_time_signature_w0{},
      types::flex_data::set_time_signature_w1{},
      types::flex_data::set_time_signature_w2{},
      types::flex_data::set_time_signature_w3{}) } -> std::same_as<void>;
  { v.set_metronome(context,
      types::flex_data::set_metronome_w0{},
      types::flex_data::set_metronome_w1{},
      types::flex_data::set_metronome_w2{},
      types::flex_data::set_metronome_w3{}) } -> std::same_as<void>;
  { v.set_key_signature(context,
      types::flex_data::set_key_signature_w0{},
      types::flex_data::set_key_signature_w1{},
      types::flex_data::set_key_signature_w2{},
      types::flex_data::set_key_signature_w3{}) } -> std::same_as<void>;
  { v.set_chord_name(context,
      types::flex_data::set_chord_name_w0{},
      types::flex_data::set_chord_name_w1{},
      types::flex_data::set_chord_name_w2{},
      types::flex_data::set_chord_name_w3{}) } -> std::same_as<void>;
  { v.text (context,
      types::flex_data::text_common_w0{},
      types::flex_data::text_common_w1{},
      types::flex_data::text_common_w2{},
      types::flex_data::text_common_w3{}) } -> std::same_as<void>;
};

template <typename T>
concept ump_processor_config = requires (T v) {
  { v.context };
  { v.callbacks } -> backend;
  { v.utility } -> utility_backend<decltype(v.context)>;
  { v.m1cvm } -> m1cvm_backend<decltype(v.context)>;
  { v.data64 } -> data64_backend<decltype(v.context)>;
  { v.m2cvm } -> m2cvm_backend<decltype(v.context)>;
  { v.data128 } -> data128_backend<decltype(v.context)>;
  { v.ump_stream } -> ump_stream_backend<decltype(v.context)>;
  { v.flex } -> flex_data_backend<decltype(v.context)>;
};
// clang-format on

class callbacks_base {
public:
  callbacks_base() = default;
  callbacks_base(callbacks_base const&) = default;
  virtual ~callbacks_base() = default;

  callbacks_base& operator=(callbacks_base const&) = default;

  virtual void system(types::system_general) { /* nop */ }

  virtual void unknown(std::span<std::uint32_t>) { /* nop */ }
};
template <typename Context> struct utility_base {
  utility_base() = default;
  utility_base(utility_base const&) = default;
  virtual ~utility_base() noexcept = default;

  virtual void noop(Context) { /* do nothing */ }
  virtual void jr_clock(Context, types::jr_clock) { /* do nothing */ }
  virtual void jr_timestamp(Context, types::jr_clock) { /* do nothing */ }
  virtual void delta_clockstamp_tpqn(Context, types::jr_clock) { /* do nothing */ }
  virtual void delta_clockstamp(Context, types::delta_clockstamp) { /* do nothing */ }
};

template <typename Context> struct m1cvm_base {
  m1cvm_base() = default;
  m1cvm_base(m1cvm_base const&) = default;
  virtual ~m1cvm_base() noexcept = default;

  virtual void note_off(Context, types::m1cvm_w0) { /* do nothing */ }
  virtual void note_on(Context, types::m1cvm_w0) { /* do nothing */ }
  virtual void poly_pressure(Context, types::m1cvm_w0) { /* do nothing */ }
  virtual void control_change(Context, types::m1cvm_w0) { /* do nothing */ }
  virtual void program_change(Context, types::m1cvm_w0) { /* do nothing */ }
  virtual void channel_pressure(Context, types::m1cvm_w0) { /* do nothing */ }
  virtual void pitch_bend(Context, types::m1cvm_w0) { /* do nothing */ }
};
template <typename Context> struct data64_base {
  data64_base() = default;
  data64_base(data64_base const&) = default;
  virtual ~data64_base() noexcept = default;

  virtual void sysex7_in_1(Context, types::data64::sysex7_w0, types::data64::sysex7_w1) { /* do nothing */ }
  virtual void sysex7_start(Context, types::data64::sysex7_w0, types::data64::sysex7_w1) { /* do nothing */ }
  virtual void sysex7_continue(Context, types::data64::sysex7_w0, types::data64::sysex7_w1) { /* do nothing */ }
  virtual void sysex7_end(Context, types::data64::sysex7_w0, types::data64::sysex7_w1) { /* do nothing */ }
};
template <typename Context> struct m2cvm_base {
  m2cvm_base() = default;
  m2cvm_base(m2cvm_base const&) = default;
  virtual ~m2cvm_base() noexcept = default;

  virtual void note_off(Context, types::m2cvm::note_w0, types::m2cvm::note_w1) { /* do nothing */ }
  virtual void note_on(Context, types::m2cvm::note_w0, types::m2cvm::note_w1) { /* do nothing */ }
  virtual void poly_pressure(Context, types::m2cvm::poly_pressure_w0, std::uint32_t) { /* do nothing */ }
  virtual void program_change(Context, types::m2cvm::program_change_w0,
                              types::m2cvm::program_change_w1) { /* do nothing */ }
  virtual void channel_pressure(Context, types::m2cvm::channel_pressure_w0, std::uint32_t) { /* do nothing */ }
  virtual void rpn_controller(Context, types::m2cvm::controller_w0, std::uint32_t) { /* do nothing */ }
  virtual void nrpn_controller(Context, types::m2cvm::controller_w0, std::uint32_t) { /* do nothing */ }
  virtual void per_note_management(Context, types::m2cvm::per_note_management_w0, std::uint32_t) { /* do nothing */ }
  virtual void control_change(Context, types::m2cvm::control_change_w0, std::uint32_t) { /* do nothing */ }
  virtual void controller_message(Context, types::m2cvm::controller_message_w0, std::uint32_t) { /* do nothing */ }
  virtual void pitch_bend(Context, types::m2cvm::pitch_bend_w0, std::uint32_t) { /* do nothing */ }
  virtual void per_note_pitch_bend(Context, types::m2cvm::per_note_pitch_bend_w0, std::uint32_t) { /* do nothing */ }
};
template <typename Context> struct data128_base {
  data128_base() = default;
  data128_base(data128_base const&) = default;
  virtual ~data128_base() noexcept = default;

  virtual void sysex8_in_1(Context, types::data128::sysex8_w0, types::data128::sysex8_w1, types::data128::sysex8_w2,
                           types::data128::sysex8_w3) { /* do nothing */ }
  virtual void sysex8_start(Context, types::data128::sysex8_w0, types::data128::sysex8_w1, types::data128::sysex8_w2,
                            types::data128::sysex8_w3) { /* do nothing */ }
  virtual void sysex8_continue(Context, types::data128::sysex8_w0, types::data128::sysex8_w1, types::data128::sysex8_w2,
                               types::data128::sysex8_w3) { /* do nothing */ }
  virtual void sysex8_end(Context, types::data128::sysex8_w0, types::data128::sysex8_w1, types::data128::sysex8_w2,
                          types::data128::sysex8_w3) { /* do nothing */ }
  // TODO: mixed data set header
  // TODO: mixed data set payload
};

template <typename Context> struct flex_data_base {
  flex_data_base() = default;
  flex_data_base(flex_data_base const&) = default;
  virtual ~flex_data_base() noexcept = default;

  virtual void set_tempo(Context, types::flex_data::set_tempo_w0, types::flex_data::set_tempo_w1,
                         types::flex_data::set_tempo_w2, types::flex_data::set_tempo_w3) { /* do nothing */ }
  virtual void set_time_signature(Context, types::flex_data::set_time_signature_w0,
                                  types::flex_data::set_time_signature_w1, types::flex_data::set_time_signature_w2,
                                  types::flex_data::set_time_signature_w3) { /* do nothing */ }
  virtual void set_metronome(Context, types::flex_data::set_metronome_w0, types::flex_data::set_metronome_w1,
                             types::flex_data::set_metronome_w2, types::flex_data::set_metronome_w3) { /* do nothing */
  }
  virtual void set_key_signature(Context, types::flex_data::set_key_signature_w0,
                                 types::flex_data::set_key_signature_w1, types::flex_data::set_key_signature_w2,
                                 types::flex_data::set_key_signature_w3) { /* do nothing */ }
  virtual void set_chord_name(Context, types::flex_data::set_chord_name_w0, types::flex_data::set_chord_name_w1,
                              types::flex_data::set_chord_name_w2,
                              types::flex_data::set_chord_name_w3) { /* do nothing */ }
  virtual void text(Context, types::flex_data::text_common_w0, types::flex_data::text_common_w1,
                    types::flex_data::text_common_w2, types::flex_data::text_common_w3) { /* do nothing */ }
};
template <typename Context> struct ump_stream_base {
  ump_stream_base() = default;
  ump_stream_base(ump_stream_base const&) = default;
  virtual ~ump_stream_base() noexcept = default;

  virtual void endpoint_discovery(Context, types::ump_stream::endpoint_discovery_w0,
                                  types::ump_stream::endpoint_discovery_w1, types::ump_stream::endpoint_discovery_w2,
                                  types::ump_stream::endpoint_discovery_w3) { /* do nothing */ }
  virtual void endpoint_info_notification(Context, types::ump_stream::endpoint_info_notification_w0,
                                          types::ump_stream::endpoint_info_notification_w1,
                                          types::ump_stream::endpoint_info_notification_w2,
                                          types::ump_stream::endpoint_info_notification_w3) { /* do nothing */ }
  virtual void device_identity_notification(Context, types::ump_stream::device_identity_notification_w0,
                                            types::ump_stream::device_identity_notification_w1,
                                            types::ump_stream::device_identity_notification_w2,
                                            types::ump_stream::device_identity_notification_w3) { /* do nothing */ }
  virtual void endpoint_name_notification(Context, types::ump_stream::endpoint_name_notification_w0,
                                          types::ump_stream::endpoint_name_notification_w1,
                                          types::ump_stream::endpoint_name_notification_w2,
                                          types::ump_stream::endpoint_name_notification_w3) { /* do nothing */ }
  virtual void product_instance_id_notification(
      Context, types::ump_stream::product_instance_id_notification_w0,
      types::ump_stream::product_instance_id_notification_w1, types::ump_stream::product_instance_id_notification_w2,
      types::ump_stream::product_instance_id_notification_w3) { /* do nothing */ }
  virtual void jr_configuration_request(Context, types::ump_stream::jr_configuration_request_w0,
                                        types::ump_stream::jr_configuration_request_w1,
                                        types::ump_stream::jr_configuration_request_w2,
                                        types::ump_stream::jr_configuration_request_w3) { /* do nothing */ }
  virtual void jr_configuration_notification(Context, types::ump_stream::jr_configuration_notification_w0,
                                             types::ump_stream::jr_configuration_notification_w1,
                                             types::ump_stream::jr_configuration_notification_w2,
                                             types::ump_stream::jr_configuration_notification_w3) { /* do nothing */ }

  virtual void function_block_discovery(Context, types::ump_stream::function_block_discovery_w0,
                                        types::ump_stream::function_block_discovery_w1,
                                        types::ump_stream::function_block_discovery_w2,
                                        types::ump_stream::function_block_discovery_w3) { /* do nothing */ }
  virtual void function_block_info_notification(
      Context, types::ump_stream::function_block_info_notification_w0,
      types::ump_stream::function_block_info_notification_w1, types::ump_stream::function_block_info_notification_w2,
      types::ump_stream::function_block_info_notification_w3) { /* do nothing */ }
  virtual void function_block_name_notification(
      Context, types::ump_stream::function_block_name_notification_w0,
      types::ump_stream::function_block_name_notification_w1, types::ump_stream::function_block_name_notification_w2,
      types::ump_stream::function_block_name_notification_w3) { /* do nothing */ }

  virtual void start_of_clip(Context, types::ump_stream::start_of_clip_w0, types::ump_stream::start_of_clip_w1,
                             types::ump_stream::start_of_clip_w2,
                             types::ump_stream::start_of_clip_w3) { /* do nothing */ }

  virtual void end_of_clip(Context, types::ump_stream::end_of_clip_w0, types::ump_stream::end_of_clip_w1,
                           types::ump_stream::end_of_clip_w2, types::ump_stream::end_of_clip_w3) { /* do nothing */ }
};

struct default_config {
  struct empty {};
  [[no_unique_address]] empty context{};
  callbacks_base callbacks;
  utility_base<decltype(context)> utility;
  m1cvm_base<decltype(context)> m1cvm;
  data64_base<decltype(context)> data64;
  m2cvm_base<decltype(context)> m2cvm;
  data128_base<decltype(context)> data128;
  ump_stream_base<decltype(context)> ump_stream;
  flex_data_base<decltype(context)> flex;
};

template <ump_processor_config Config = default_config> class umpProcessor {
public:
  explicit constexpr umpProcessor(Config const& config = default_config{}) : config_{config} {}

  void clearUMP() {
    // Note that this member function has to be defined in the class declaration to avoid a spurious GCC
    // warning that the function is defined but not used. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79001>
    pos_ = 0;
    std::ranges::fill(message_, std::uint8_t{0});
  }
  void processUMP() { /* nothing to do */ }
  template <typename First, typename... Rest>
    requires (sizeof(First) == sizeof(std::uint32_t))
  void processUMP(First ump, Rest&&... rest) {
    this->processUMP(ump.word(), std::forward<Rest>(rest)...);
  }
  template <typename... Rest> void processUMP(std::uint32_t ump, Rest&&... rest) {
    // Note that this member function has to be defined in the class declaration to avoid a spurious GCC
    // warning that the function is defined but not used. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79001>
    assert(pos_ < message_.size());
    message_[pos_++] = ump;

    auto const mt = static_cast<ump_message_type>((message_[0] >> 28) & 0xF);
    if (pos_ >= ump_message_size(mt)) {
      pos_ = 0;

      switch (mt) {
      case ump_message_type::utility: this->utility_message(); break;
      case ump_message_type::system: this->system_message(); break;
      case ump_message_type::m1cvm: this->m1cvm_message(); break;
      case ump_message_type::m2cvm: this->m2cvm_message(); break;
      case ump_message_type::flex_data: this->flex_data_message(); break;
      case ump_message_type::ump_stream: this->ump_stream_message(); break;
      case ump_message_type::data64: this->data64_message(); break;
      case ump_message_type::data128: this->data128_message(); break;

      case ump_message_type::reserved32_06:
      case ump_message_type::reserved32_07:
      case ump_message_type::reserved64_08:
      case ump_message_type::reserved64_09:
      case ump_message_type::reserved64_0A:
      case ump_message_type::reserved96_0B:
      case ump_message_type::reserved96_0C:
      case ump_message_type::reserved128_0E: config_.callbacks.unknown(std::span{message_.data(), pos_}); break;
      default:
        assert(false);
        unreachable();
        break;
      }
    }
    this->processUMP(std::forward<Rest>(rest)...);
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

// utility message
// ~~~~~~~~~~~~~~~
// 32 bit utility messages
template <ump_processor_config Config> void umpProcessor<Config>::utility_message() {
  static_assert(message_size<midi2::ump_message_type::utility>() == 1);
  switch (static_cast<ump_utility>((message_[0] >> 20) & 0x0F)) {
  // 7.2.1 NOOP
  case ump_utility::noop: config_.utility.noop(config_.context); break;
  // 7.2.2.1 JR Clock
  case ump_utility::jr_clock:
    config_.utility.jr_clock(config_.context, std::bit_cast<types::jr_clock>(message_[0]));
    break;
  // 7.2.2.2 JR Timestamp
  case ump_utility::jr_ts:
    config_.utility.jr_timestamp(config_.context, std::bit_cast<types::jr_clock>(message_[0]));
    break;
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
  case ump_utility::delta_clock_tick:
    config_.utility.delta_clockstamp_tpqn(config_.context, std::bit_cast<types::jr_clock>(message_[0]));
    break;
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  case ump_utility::delta_clock_since:
    config_.utility.delta_clockstamp(config_.context, std::bit_cast<types::delta_clockstamp>(message_[0]));
    break;
  default: config_.callbacks.unknown(std::span{message_.data(), 1}); break;
  }
}

// 32 bit System Real Time and System Common Messages (except System Exclusive)
template <ump_processor_config Config> void umpProcessor<Config>::system_message() {
  static_assert(message_size<midi2::ump_message_type::system>() == 1);
  // 7.6 System Common and System Real Time Messages
  auto const sg = std::bit_cast<types::system_general>(message_[0]);
  if (sg.status >= 0xF0) {
    config_.callbacks.system(sg);
  } else {
    config_.callbacks.unknown(std::span{message_.data(), 1});
  }
}

// m1cvm message
// ~~~~~~~~~~~~~
// 32 Bit MIDI 1.0 Channel Voice Messages
template <ump_processor_config Config> void umpProcessor<Config>::m1cvm_message() {
  static_assert(message_size<midi2::ump_message_type::m1cvm>() == 1);
  auto const w0 = std::bit_cast<types::m1cvm_w0>(message_[0]);
  switch ((message_[0] >> 16) & 0xF0) {
  // 7.3.1 MIDI 1.0 Note Off Message
  case status::note_off: config_.m1cvm.note_off(config_.context, w0); break;
  // 7.3.2 MIDI 1.0 Note On Message
  case status::note_on: config_.m1cvm.note_on(config_.context, w0); break;
  // 7.3.3 MIDI 1.0 Poly Pressure Message
  case status::key_pressure: config_.m1cvm.poly_pressure(config_.context, w0); break;
  // 7.3.4 MIDI 1.0 Control Change Message
  case status::cc: config_.m1cvm.control_change(config_.context, w0); break;
  // 7.3.5 MIDI 1.0 Program Change Message
  case status::program_change: config_.m1cvm.program_change(config_.context, w0); break;
  // 7.3.6 MIDI 1.0 Channel Pressure Message
  case status::channel_pressure: config_.m1cvm.channel_pressure(config_.context, w0); break;
  // 7.3.7 MIDI 1.0 Pitch Bend Message
  case status::pitch_bend: config_.m1cvm.pitch_bend(config_.context, w0); break;
  default: config_.callbacks.unknown(std::span{message_.data(), 1}); break;
  }
}

// data64 message
// ~~~~~~~~~~~~~~
template <ump_processor_config Config> void umpProcessor<Config>::data64_message() {
  auto const w0 = std::bit_cast<types::data64::sysex7_w0>(message_[0]);
  auto const w1 = std::bit_cast<types::data64::sysex7_w1>(message_[1]);
  switch (static_cast<data64>(w0.status.value())) {
  case data64::sysex7_in_1: config_.data64.sysex7_in_1(config_.context, w0, w1); break;
  case data64::sysex7_start: config_.data64.sysex7_start(config_.context, w0, w1); break;
  case data64::sysex7_continue: config_.data64.sysex7_continue(config_.context, w0, w1); break;
  case data64::sysex7_end: config_.data64.sysex7_end(config_.context, w0, w1); break;
  default: config_.callbacks.unknown(std::span{message_.data(), 2}); break;
  }
}

// m2cvm message
// ~~~~~~~~~~~~~
// 64 bit MIDI 2.0 Channel Voice Messages
template <ump_processor_config Config> void umpProcessor<Config>::m2cvm_message() {
  static_assert(message_size<midi2::ump_message_type::m2cvm>() == 2);
  switch ((message_[0] >> 16) & 0xF0) {
  // 7.4.1 MIDI 2.0 Note Off Message
  case status::note_off:
    config_.m2cvm.note_off(config_.context, std::bit_cast<types::m2cvm::note_w0>(message_[0]),
                           std::bit_cast<types::m2cvm::note_w1>(message_[1]));
    break;
  // 7.4.2 MIDI 2.0 Note On Message
  case status::note_on:
    config_.m2cvm.note_on(config_.context, std::bit_cast<types::m2cvm::note_w0>(message_[0]),
                          std::bit_cast<types::m2cvm::note_w1>(message_[1]));
    break;
  // 7.4.3 MIDI 2.0 Poly Pressure Message
  case status::key_pressure:  // Polyphonic pressure
    config_.m2cvm.poly_pressure(config_.context, std::bit_cast<types::m2cvm::poly_pressure_w0>(message_[0]),
                                message_[1]);
    break;
  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message
  case midi2status::rpn_pernote:
    config_.m2cvm.rpn_controller(config_.context, std::bit_cast<types::m2cvm::controller_w0>(message_[0]), message_[1]);
    break;
  // 7.4.4 MIDI 2.0 Assignable Per-Note Controller Message
  case midi2status::nrpn_pernote:
    config_.m2cvm.nrpn_controller(config_.context, std::bit_cast<types::m2cvm::controller_w0>(message_[0]),
                                  message_[1]);
    break;
  // 7.4.5 MIDI 2.0 Per-Note Management Message
  case midi2status::pernote_manage:
    config_.m2cvm.per_note_management(config_.context, std::bit_cast<types::m2cvm::per_note_management_w0>(message_[0]),
                                      message_[1]);
    break;
  // 7.4.6 MIDI 2.0 Control Change Message
  case status::cc:
    config_.m2cvm.control_change(config_.context, std::bit_cast<types::m2cvm::control_change_w0>(message_[0]),
                                 message_[1]);
    break;
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) and Assignable Controller (NRPN) Message
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) and Assignable Controller (NRPN) Message
  case midi2status::rpn:
  case midi2status::nrpn:
  case midi2status::rpn_relative:
  case midi2status::nrpn_relative:
    config_.m2cvm.controller_message(config_.context, std::bit_cast<types::m2cvm::controller_message_w0>(message_[0]),
                                     message_[1]);
    break;
  // 7.4.9 MIDI 2.0 Program Change Message
  case status::program_change:  // Program Change Message
    config_.m2cvm.program_change(config_.context, std::bit_cast<types::m2cvm::program_change_w0>(message_[0]),
                                 std::bit_cast<types::m2cvm::program_change_w1>(message_[1]));
    break;
  // 7.4.10 MIDI 2.0 Channel Pressure Message
  case status::channel_pressure:
    config_.m2cvm.channel_pressure(config_.context, std::bit_cast<types::m2cvm::channel_pressure_w0>(message_[0]),
                                   message_[1]);
    break;
  // 7.4.11 MIDI 2.0 Pitch Bend Message
  case status::pitch_bend:
    config_.m2cvm.pitch_bend(config_.context, std::bit_cast<types::m2cvm::pitch_bend_w0>(message_[0]), message_[1]);
    break;
  // 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
  case midi2status::pitch_bend_pernote:
    config_.m2cvm.per_note_pitch_bend(config_.context, std::bit_cast<types::m2cvm::per_note_pitch_bend_w0>(message_[0]),
                                      message_[1]);
    break;
  default: config_.callbacks.unknown(std::span{message_.data(), 2}); break;
  }
}

// ump stream message
// ~~~~~~~~~~~~~~~~~~
template <ump_processor_config Config> void umpProcessor<Config>::ump_stream_message() {
  switch (static_cast<ump_stream>((message_[0] >> 16) & ((std::uint32_t{1} << 10) - 1U))) {
  // 7.1.1 Endpoint Discovery Message
  case ump_stream::endpoint_discovery:
    config_.ump_stream.endpoint_discovery(
        config_.context, std::bit_cast<types::ump_stream::endpoint_discovery_w0>(message_[0]),
        std::bit_cast<types::ump_stream::endpoint_discovery_w1>(message_[1]), message_[2], message_[3]);
    break;

  // 7.1.2 Endpoint Info Notification Message
  case ump_stream::endpoint_info_notification:
    config_.ump_stream.endpoint_info_notification(
        config_.context, std::bit_cast<types::ump_stream::endpoint_info_notification_w0>(message_[0]),
        std::bit_cast<types::ump_stream::endpoint_info_notification_w1>(message_[1]), message_[2], message_[3]);
    break;

  // 7.1.3 Device Identity Notification Message
  case ump_stream::device_identity_notification:
    config_.ump_stream.device_identity_notification(
        config_.context, std::bit_cast<types::ump_stream::device_identity_notification_w0>(message_[0]),
        std::bit_cast<types::ump_stream::device_identity_notification_w1>(message_[1]),
        std::bit_cast<types::ump_stream::device_identity_notification_w2>(message_[2]),
        std::bit_cast<types::ump_stream::device_identity_notification_w3>(message_[3]));
    break;

  // 7.1.4 Endpoint Name Notification
  case ump_stream::endpoint_name_notification:
    config_.ump_stream.endpoint_name_notification(
        config_.context, std::bit_cast<types::ump_stream::endpoint_name_notification_w0>(message_[0]),
        std::bit_cast<types::ump_stream::endpoint_name_notification_w1>(message_[1]),
        std::bit_cast<types::ump_stream::endpoint_name_notification_w2>(message_[2]),
        std::bit_cast<types::ump_stream::endpoint_name_notification_w3>(message_[3]));
    break;
  // 7.1.5 Product Instance Id Notification Message
  case ump_stream::product_instance_id_notification:
    config_.ump_stream.product_instance_id_notification(
        config_.context, std::bit_cast<types::ump_stream::product_instance_id_notification_w0>(message_[0]),
        std::bit_cast<types::ump_stream::product_instance_id_notification_w1>(message_[1]),
        std::bit_cast<types::ump_stream::product_instance_id_notification_w2>(message_[2]),
        std::bit_cast<types::ump_stream::product_instance_id_notification_w3>(message_[3]));
    break;
  // 7.1.6.2 Stream Configuration Request
  case ump_stream::jr_configuration_request:
    config_.ump_stream.jr_configuration_request(
        config_.context, std::bit_cast<types::ump_stream::jr_configuration_request_w0>(message_[0]),
        std::bit_cast<types::ump_stream::jr_configuration_request_w1>(message_[1]),
        std::bit_cast<types::ump_stream::jr_configuration_request_w2>(message_[2]),
        std::bit_cast<types::ump_stream::jr_configuration_request_w3>(message_[3]));
    break;
  // 7.1.6.3 Stream Configuration Notification Message
  case ump_stream::jr_configuration_notification:
    config_.ump_stream.jr_configuration_notification(
        config_.context, std::bit_cast<types::ump_stream::jr_configuration_notification_w0>(message_[0]),
        std::bit_cast<types::ump_stream::jr_configuration_notification_w1>(message_[1]),
        std::bit_cast<types::ump_stream::jr_configuration_notification_w2>(message_[2]),
        std::bit_cast<types::ump_stream::jr_configuration_notification_w3>(message_[3]));
    break;
  // 7.1.7 Function Block Discovery Message
  case ump_stream::function_block_discovery:
    config_.ump_stream.function_block_discovery(
        config_.context, std::bit_cast<types::ump_stream::function_block_discovery_w0>(message_[0]),
        std::bit_cast<types::ump_stream::function_block_discovery_w1>(message_[1]),
        std::bit_cast<types::ump_stream::function_block_discovery_w2>(message_[2]),
        std::bit_cast<types::ump_stream::function_block_discovery_w3>(message_[3]));
    break;
  // 7.1.8 Function Block Info Notification
  case ump_stream::function_block_info_notification:
    config_.ump_stream.function_block_info_notification(
        config_.context, std::bit_cast<types::ump_stream::function_block_info_notification_w0>(message_[0]),
        std::bit_cast<types::ump_stream::function_block_info_notification_w1>(message_[1]),
        std::bit_cast<types::ump_stream::function_block_info_notification_w2>(message_[2]),
        std::bit_cast<types::ump_stream::function_block_info_notification_w3>(message_[3]));
    break;
  // 7.1.9 Function Block Name Notification
  case ump_stream::function_block_name_notification:
    config_.ump_stream.function_block_name_notification(
        config_.context, std::bit_cast<types::ump_stream::function_block_name_notification_w0>(message_[0]),
        std::bit_cast<types::ump_stream::function_block_name_notification_w1>(message_[1]),
        std::bit_cast<types::ump_stream::function_block_name_notification_w2>(message_[2]),
        std::bit_cast<types::ump_stream::function_block_name_notification_w3>(message_[3]));
    break;
  case ump_stream::start_of_clip:
    config_.ump_stream.start_of_clip(config_.context, std::bit_cast<types::ump_stream::start_of_clip_w0>(message_[0]),
                                     std::bit_cast<types::ump_stream::start_of_clip_w1>(message_[1]),
                                     std::bit_cast<types::ump_stream::start_of_clip_w2>(message_[2]),
                                     std::bit_cast<types::ump_stream::start_of_clip_w3>(message_[3]));
    break;
  case ump_stream::end_of_clip:
    config_.ump_stream.end_of_clip(config_.context, std::bit_cast<types::ump_stream::end_of_clip_w0>(message_[0]),
                                   std::bit_cast<types::ump_stream::end_of_clip_w1>(message_[1]),
                                   std::bit_cast<types::ump_stream::end_of_clip_w2>(message_[2]),
                                   std::bit_cast<types::ump_stream::end_of_clip_w3>(message_[3]));
    break;
  default: config_.callbacks.unknown(std::span{message_.data(), 4}); break;
  }
}

// 128 bit Data Messages (including System Exclusive 8)
template <ump_processor_config Config> void umpProcessor<Config>::data128_message() {
  switch (static_cast<data128>((message_[0] >> 20) & 0x0F)) {
  case data128::sysex8_in_1:
    config_.data128.sysex8_in_1(config_.context, std::bit_cast<types::data128::sysex8_w0>(message_[0]),
                                std::bit_cast<types::data128::sysex8_w1>(message_[1]),
                                std::bit_cast<types::data128::sysex8_w2>(message_[2]),
                                std::bit_cast<types::data128::sysex8_w3>(message_[3]));
    break;
  case data128::sysex8_start:
    config_.data128.sysex8_start(config_.context, std::bit_cast<types::data128::sysex8_w0>(message_[0]),
                                 std::bit_cast<types::data128::sysex8_w1>(message_[1]),
                                 std::bit_cast<types::data128::sysex8_w2>(message_[2]),
                                 std::bit_cast<types::data128::sysex8_w3>(message_[3]));
    break;
  case data128::sysex8_continue:
    config_.data128.sysex8_continue(config_.context, std::bit_cast<types::data128::sysex8_w0>(message_[0]),
                                    std::bit_cast<types::data128::sysex8_w1>(message_[1]),
                                    std::bit_cast<types::data128::sysex8_w2>(message_[2]),
                                    std::bit_cast<types::data128::sysex8_w3>(message_[3]));
    break;
  case data128::sysex8_end:
    config_.data128.sysex8_end(config_.context, std::bit_cast<types::data128::sysex8_w0>(message_[0]),
                               std::bit_cast<types::data128::sysex8_w1>(message_[1]),
                               std::bit_cast<types::data128::sysex8_w2>(message_[2]),
                               std::bit_cast<types::data128::sysex8_w3>(message_[3]));
    break;
  case data128::mixed_data_set_header: break;
  case data128::mixed_data_set_payload: break;
  default: config_.callbacks.unknown(std::span{message_.data(), 4}); break;
  }
}

// flex data message
// ~~~~~~~~~~~~~~~~~
// 128 bit Data Messages (including System Exclusive 8)
template <ump_processor_config Config> void umpProcessor<Config>::flex_data_message() {
  auto const m0 = std::bit_cast<types::flex_data::flex_data_w0>(message_[0]);
  auto const status = static_cast<flex_data>(m0.status.value());
  if (m0.status_bank == 0) {
    switch (status) {
    // 7.5.3 Set Tempo Message
    case flex_data::set_tempo:
      config_.flex.set_tempo(config_.context, std::bit_cast<types::flex_data::set_tempo_w0>(message_[0]),
                             std::bit_cast<types::flex_data::set_tempo_w1>(message_[1]),
                             std::bit_cast<types::flex_data::set_tempo_w2>(message_[2]),
                             std::bit_cast<types::flex_data::set_tempo_w3>(message_[3]));
      break;
    // 7.5.4 Set Time Signature Message
    case flex_data::set_time_signature:
      config_.flex.set_time_signature(config_.context,
                                      std::bit_cast<types::flex_data::set_time_signature_w0>(message_[0]),
                                      std::bit_cast<types::flex_data::set_time_signature_w1>(message_[1]),
                                      std::bit_cast<types::flex_data::set_time_signature_w2>(message_[2]),
                                      std::bit_cast<types::flex_data::set_time_signature_w3>(message_[3]));
      break;
    // 7.5.5 Set Metronome Message
    case flex_data::set_metronome:
      config_.flex.set_metronome(config_.context, std::bit_cast<types::flex_data::set_metronome_w0>(message_[0]),
                                 std::bit_cast<types::flex_data::set_metronome_w1>(message_[1]),
                                 std::bit_cast<types::flex_data::set_metronome_w2>(message_[2]),
                                 std::bit_cast<types::flex_data::set_metronome_w3>(message_[3]));
      break;
    // 7.5.7 Set Key Signature Message
    case flex_data::set_key_signature:
      config_.flex.set_key_signature(config_.context,
                                     std::bit_cast<types::flex_data::set_key_signature_w0>(message_[0]),
                                     std::bit_cast<types::flex_data::set_key_signature_w1>(message_[1]),
                                     std::bit_cast<types::flex_data::set_key_signature_w2>(message_[2]),
                                     std::bit_cast<types::flex_data::set_key_signature_w3>(message_[3]));
      break;
    // 7.5.8 Set Chord Name Message
    case flex_data::set_chord_name:
      config_.flex.set_chord_name(config_.context, std::bit_cast<types::flex_data::set_chord_name_w0>(message_[0]),
                                  std::bit_cast<types::flex_data::set_chord_name_w1>(message_[1]),
                                  std::bit_cast<types::flex_data::set_chord_name_w2>(message_[2]),
                                  std::bit_cast<types::flex_data::set_chord_name_w3>(message_[3]));
      break;
    default: config_.callbacks.unknown(std::span{message_.data(), 4}); break;
    }
  } else {
    config_.flex.text(config_.context, std::bit_cast<types::flex_data::text_common_w0>(message_[0]),
                      std::bit_cast<types::flex_data::text_common_w1>(message_[1]),
                      std::bit_cast<types::flex_data::text_common_w2>(message_[2]),
                      std::bit_cast<types::flex_data::text_common_w3>(message_[3]));
  }
}

}  // end namespace midi2

#endif  // MIDI2_UMP_PROCESSOR_HPP
