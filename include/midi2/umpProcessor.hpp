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
struct ump_cvm {
  constexpr bool operator==(ump_cvm const&) const = default;

  ump_common common;
  uint8_t channel = 0xFF;
  uint8_t note = 0xFF;
  uint32_t value = 0;
  uint16_t index = 0;
  uint8_t bank = 0;
  bool flag1 = false;
  bool flag2 = false;
};

struct ump_generic {
  constexpr bool operator==(ump_generic const&) const = default;

  ump_common common;
  std::uint16_t value = 0;
};

struct ump_data {
  ump_common common;
  uint8_t streamId = 0;
  uint8_t form = 0;
  std::span<std::uint8_t> data;
};

struct chord {
  bool operator==(chord const&) const = default;

  enum class sharps_flats : std::int8_t {
    double_sharp = 2,
    sharp = 1,
    natural = 0,
    flat = -1,
    double_flat = -2,
    /// Indicates that the bass note is the same as the chord tonic note; the
    /// bass note field is set to note::unknown. Valid only for the bass
    /// sharps/flats field.
    chord_tonic = -8,
  };

  enum class note : std::uint8_t {
    unknown = 0x0,
    A = 0x1,
    B = 0x2,
    C = 0x3,
    D = 0x4,
    E = 0x5,
    F = 0x6,
    G = 0x7,
  };

  enum class chord_type : std::uint8_t {
    no_chord = 0x00,
    major = 0x01,
    major_6th = 0x02,
    major_7th = 0x03,
    major_9th = 0x04,
    major_11th = 0x05,
    major_13th = 0x06,
    minor = 0x07,
    minor_6th = 0x08,
    minor_7th = 0x09,
    minor_9th = 0x0A,
    minor_11th = 0x0B,
    minor_13th = 0x0C,
    dominant = 0x0D,
    dominant_ninth = 0x0E,
    dominant_11th = 0x0F,
    dominant_13th = 0x10,
    augmented = 0x11,
    augmented_seventh = 0x12,
    diminished = 0x13,
    diminished_seventh = 0x14,
    half_diminished = 0x15,
    major_minor = 0x16,
    pedal = 0x17,
    power = 0x18,
    suspended_2nd = 0x19,
    suspended_4th = 0x1A,
    seven_suspended_4th = 0x1B,
  };

  struct alteration {
    bool operator==(alteration const&) const = default;
    uint8_t type : 4;
    uint8_t degree : 4;
  };

  sharps_flats chShrpFlt;
  note chTonic;
  chord_type chType;
  alteration chAlt1;
  alteration chAlt2;
  alteration chAlt3;
  alteration chAlt4;
  sharps_flats baShrpFlt;
  note baTonic;
  chord_type baType;
  alteration baAlt1;
  alteration baAlt2;
};

struct function_block_info {
  bool operator==(function_block_info const&) const = default;

  enum class fbdirection : std::uint8_t {
    reserved = 0b00,
    input = 0b01,
    output = 0b10,
    bidirectional = 0b11,
  };

  std::uint8_t fbIdx;
  bool active;
  fbdirection direction;
  std::uint8_t firstGroup;
  std::uint8_t groupLength;
  std::uint8_t midiCIVersion;
  std::uint8_t isMIDI1;
  std::uint8_t maxS8Streams;
};

// See M2-104-UM (UMP Format & MIDI 2.0 Protocol v.1.1.2 2023-10-27)
//    Table 4 Message Type (MT) Allocation
template <midi2::ump_message_type> struct message_size {};
template <> struct message_size<midi2::ump_message_type::utility> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<midi2::ump_message_type::system> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<midi2::ump_message_type::m1cvm> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<midi2::ump_message_type::sysex7> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<midi2::ump_message_type::m2cvm> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<midi2::ump_message_type::data> : std::integral_constant<unsigned, 4> {};
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
  { v.send_out_sysex(ump_data{}) } -> std::same_as<void>;

  { v.functionBlock(std::uint8_t{}, std::uint8_t{}) } -> std::same_as<void>;
  { v.functionBlockInfo(function_block_info{}) } -> std::same_as<void>;
  { v.functionBlockName(ump_data{}, std::uint8_t{}) } -> std::same_as<void>;

  { v.startOfSeq() } -> std::same_as<void>;
  { v.endOfFile() } -> std::same_as<void>;

  { v.unknownUMPMessage(std::span<std::uint32_t>{}) } -> std::same_as<void>;
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
concept utility_backend = requires(T v, Context context) {
  { v.noop (context) } -> std::same_as<void>;
  { v.jr_clock (context, types::jr_clock{}) } -> std::same_as<void>;
  { v.jr_timestamp (context, types::jr_clock{}) } -> std::same_as<void>;
  { v.delta_clockstamp_tpqn(context, types::jr_clock{}) } -> std::same_as<void>;
  { v.delta_clockstamp(context, types::delta_clockstamp{}) } -> std::same_as<void>;
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


  { v.midiEndpointProdId(ump_data{}) } -> std::same_as<void>;
  { v.midiEndpointJRProtocolReq(std::uint8_t{}, bool{}, bool{}) } -> std::same_as<void>;
  { v.midiEndpointJRProtocolNotify(std::uint8_t{}, bool{}, bool{}) } -> std::same_as<void>;
  };
template <typename T, typename Context>
concept flex_backend = requires(T v, Context context) {
  { v.tempo(context, uint8_t{}, uint32_t{}) } -> std::same_as<void>;
  { v.time_sig(context, uint8_t{}, uint8_t{}, uint8_t{}, uint8_t{}) } -> std::same_as<void>;
  { v.metronome(context, uint8_t{}, uint8_t{}, uint8_t{}, uint8_t{}, uint8_t{}, uint8_t{}, uint8_t{}) } -> std::same_as<void>;
  { v.key_sig(context, uint8_t{}, uint8_t{}, uint8_t{}, uint8_t{}, uint8_t{}) } -> std::same_as<void>;
  { v.chord(context, uint8_t{}, uint8_t{}, uint8_t{}, chord{}) } -> std::same_as<void>;
  { v.performance(context, ump_data{}, uint8_t{}, uint8_t{}) } -> std::same_as<void>;
  { v.lyric(context, ump_data{}, uint8_t{}, uint8_t{}) } -> std::same_as<void>;
};

template <typename T>
concept ump_processor_config = requires (T v) {
  { v.context };
  { v.callbacks } -> backend;
  { v.m1cvm } -> m1cvm_backend<decltype(v.context)>;
  { v.m2cvm } -> m2cvm_backend<decltype(v.context)>;
  { v.utility } -> utility_backend<decltype(v.context)>;
  { v.flex } -> flex_backend<decltype(v.context)>;
  { v.ump_stream } -> ump_stream_backend<decltype(v.context)>;
};
// clang-format on

class callbacks_base {
public:
  callbacks_base() = default;
  callbacks_base(callbacks_base const&) = default;
  virtual ~callbacks_base() = default;

  callbacks_base& operator=(callbacks_base const&) = default;

  virtual void system(types::system_general) { /* nop */ }
  virtual void send_out_sysex(ump_data const& /*mess*/) { /* nop */ }

  //---------- UMP Stream

  virtual void functionBlock(uint8_t /*fbIdx*/, uint8_t /*filter*/) { /* nop */ }
  virtual void functionBlockInfo(function_block_info const& fbi) { (void)fbi; }
  virtual void functionBlockName(ump_data const& /*mess*/, uint8_t /*fbIdx*/) { /* nop */ }

  virtual void startOfSeq() { /* nop */ }
  virtual void endOfFile() { /* nop */ }

  virtual void unknownUMPMessage(std::span<std::uint32_t>) { /* nop */ }
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
template <typename Context> struct flex_base {
  flex_base() = default;
  flex_base(flex_base const&) = default;
  virtual ~flex_base() noexcept = default;

  virtual void tempo(Context, uint8_t /*group*/, uint32_t /*num10nsPQN*/) { /* do nothing */ }
  virtual void time_sig(Context, uint8_t /*group*/, uint8_t /*numerator*/, uint8_t /*denominator*/,
                        uint8_t /*num32Notes*/) { /* do nothing */ }
  virtual void metronome(Context, uint8_t /*group*/, uint8_t /*numClkpPriCli*/, uint8_t /*bAccP1*/, uint8_t /*bAccP2*/,
                         uint8_t /*bAccP3*/, uint8_t /*numSubDivCli1*/, uint8_t /*numSubDivCli2*/) { /* do nothing */ }
  virtual void key_sig(Context, uint8_t /*group*/, uint8_t /*addrs*/, uint8_t /*channel*/, uint8_t /*sharpFlats*/,
                       uint8_t /*tonic*/) { /* do nothing */ }
  virtual void chord(Context, uint8_t /*group*/, uint8_t /*addrs*/, uint8_t /*channel*/,
                     chord const& /*chord*/) { /* do nothing */ }
  virtual void performance(Context, ump_data const& /*mess*/, uint8_t /*addrs*/, uint8_t /*channel*/) { /* do nothing */
  }
  virtual void lyric(Context, ump_data const& /*mess*/, uint8_t /*addrs*/, uint8_t /*channel*/) { /* do nothing */ }
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

  virtual void midiEndpointProdId(ump_data const& /*mess*/) { /* do nothing */ }
  virtual void midiEndpointJRProtocolReq(uint8_t /*protocol*/, bool /*jrrx*/, bool /*jrtx*/) { /* do nothing */ }
  virtual void midiEndpointJRProtocolNotify(uint8_t /*protocol*/, bool /*jrrx*/, bool /*jrtx*/) { /* do nothing */ }
};

struct default_config {
  struct empty {};
  [[no_unique_address]] empty context{};
  callbacks_base callbacks;
  m1cvm_base<decltype(context)> m1cvm;
  m2cvm_base<decltype(context)> m2cvm;
  utility_base<decltype(context)> utility;
  flex_base<decltype(context)> flex;
  ump_stream_base<decltype(context)> ump_stream;
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

  void processUMP(std::uint32_t ump) {
    // Note that this member function has to be defined in the class declaration to avoid a spurious GCC
    // warning that the function is defined but not used. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79001>
    assert(pos_ < message_.size());
    message_[pos_++] = ump;

    auto const mt = static_cast<ump_message_type>((message_[0] >> 28) & 0xF);
    if (pos_ < ump_message_size(mt)) {
      return;
    }
    pos_ = 0;

    auto const group = static_cast<std::uint8_t>((message_[0] >> 24) & 0xF);
    switch (mt) {
    case ump_message_type::utility: this->utility_message(); break;
    case ump_message_type::system: this->system_message(); break;
    case ump_message_type::m1cvm: this->m1cvm_message(); break;
    case ump_message_type::sysex7: this->sysex7_message(mt, group); break;
    case ump_message_type::m2cvm: this->m2cvm_message(); break;
    case ump_message_type::data: this->data_message(); break;
    case ump_message_type::flex_data: this->flexdata_message(mt, group); break;
    case ump_message_type::ump_stream: this->midi_endpoint_message(mt); break;

    case ump_message_type::reserved32_06:
    case ump_message_type::reserved32_07:
    case ump_message_type::reserved64_08:
    case ump_message_type::reserved64_09:
    case ump_message_type::reserved64_0A:
    case ump_message_type::reserved96_0B:
    case ump_message_type::reserved96_0C:
    case ump_message_type::reserved128_0E: config_.callbacks.unknownUMPMessage(std::span{message_.data(), pos_}); break;
    default:
      assert(false);
      unreachable();
      break;
    }
  }

private:
  void utility_message();
  void system_message();
  void m1cvm_message();
  void sysex7_message(ump_message_type mt, std::uint8_t group);
  void m2cvm_message();
  void midi_endpoint_message(ump_message_type mt);
  void data_message();
  void flexdata_message(ump_message_type mt, std::uint8_t group);

  enum data_message_status : std::uint8_t {
    sysex8_in_1_ump = 0b0000,
    sysex8_start = 0b0001,
    sysex8_continue = 0b0010,
    sysex8_end = 0b0011,
    mixed_data_set_header = 0b1000,
    mixed_data_set_payload = 0b1001,
  };

  template <std::output_iterator<std::uint8_t> OutputIterator>
  static constexpr OutputIterator payload(std::array<std::uint32_t, 4> const& message, std::size_t index,
                                          std::size_t limit, OutputIterator out);

  void midiendpoint_name_or_prodid(ump_message_type mt);
  void functionblock_name();
  void functionblock_info();
  void set_chord_name();
  void flexdata_performance_or_lyric(ump_message_type mt, std::uint8_t group);

  std::array<std::uint32_t, 4> message_{};
  std::uint8_t pos_ = 0;

  [[no_unique_address]] Config config_;
};

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
  default: config_.callbacks.unknownUMPMessage(std::span{message_.data(), 1}); break;
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
    config_.callbacks.unknownUMPMessage(std::span{message_.data(), 1});
  }
}

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
  default: config_.callbacks.unknownUMPMessage(std::span{message_.data(), 1}); break;
  }
}

// 64 bit System Exclusive Data Message
template <ump_processor_config Config>
void umpProcessor<Config>::sysex7_message(ump_message_type const mt, std::uint8_t const group) {
  std::array<std::uint8_t, 7> sysex{};
  auto const data_length = (message_[0] >> 16) & 0x7;
  if (data_length > 0) {
    sysex[0] = (message_[0] >> 8) & 0x7F;
  }
  if (data_length > 1) {
    sysex[1] = message_[0] & 0x7F;
  }
  if (data_length > 2) {
    sysex[2] = (message_[1] >> 24) & 0x7F;
  }
  if (data_length > 3) {
    sysex[3] = (message_[1] >> 16) & 0x7F;
  }
  if (data_length > 4) {
    sysex[4] = (message_[1] >> 8) & 0x7F;
  }
  if (data_length > 5) {
    sysex[5] = message_[1] & 0x7F;
  }
  ump_data mess;
  mess.common.group = group;
  mess.common.messageType = mt;
  mess.form = (message_[0] >> 20) & 0xF;
  mess.data = std::span{sysex.data(), data_length};
  assert(mess.data.size() <= sysex.size());
  config_.callbacks.send_out_sysex(mess);
}

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
  default: config_.callbacks.unknownUMPMessage(std::span{message_.data(), 2}); break;
  }
}

template <ump_processor_config Config>
void umpProcessor<Config>::midiendpoint_name_or_prodid(ump_message_type const mt) {
  std::uint16_t status = (message_[0] >> 16) & 0x3FF;
  assert(status == to_underlying(ump_stream::MIDIENDPOINT_PRODID_NOTIFICATION));

  std::array<std::uint8_t, 14> text{};
  auto text_length = 0U;
  if ((message_[0] >> 8) & 0xFF) {
    text[text_length++] = (message_[0] >> 8) & 0xFF;
  }
  if (message_[0] & 0xFF) {
    text[text_length++] = message_[0] & 0xFF;
  }
  for (auto i = 1U; i <= 3U; ++i) {
    for (auto j = 24; j >= 0; j -= 8) {
      if (std::uint8_t c = (message_[i] >> j) & 0xFF) {
        text[text_length++] = c;
      }
    }
  }
  assert(text_length <= text.size());
  ump_data mess;
  mess.common.messageType = mt;
  mess.common.status = static_cast<std::uint8_t>(status);
  mess.form = message_[0] >> 24 & 0x3;
  mess.data = std::span{text.data(), text_length};
  config_.ump_stream.midiEndpointProdId(mess);
}

template <ump_processor_config Config>
template <std::output_iterator<std::uint8_t> OutputIterator>
constexpr OutputIterator umpProcessor<Config>::payload(std::array<std::uint32_t, 4> const& message, std::size_t index,
                                                       std::size_t limit, OutputIterator out) {
  assert(limit < message.size() * sizeof(std::uint32_t) && index <= limit);
  if (index >= limit) {
    return out;
  }
  // There are 4 bytes per packet and we start at packet #1.
  auto const packet_num = (index >> 2) + 1U;
  auto const rem4 = index & 0b11U;  // rem4 = index % 4
  auto const shift = 24U - 8U * rem4;
  *(out++) = (message[packet_num] >> shift) & 0xFF;
  return umpProcessor::payload(message, index + 1U, limit, out);
}

template <ump_processor_config Config> void umpProcessor<Config>::functionblock_name() {
  auto w0 = std::bit_cast<types::function_block_name_w0>(message_[0]);

  std::uint8_t const fbIdx = w0.block_number;
  std::array<std::uint8_t, 13> text{};
  text[0] = w0.name;
  umpProcessor::payload(message_, 0, text.size() - 1, std::begin(text) + 1);
  auto const text_length = std::distance(
      find_if_not(std::rbegin(text), std::rend(text), [](std::uint8_t v) { return v == 0; }), std::rend(text));
  assert(text_length >= 0 && static_cast<std::size_t>(text_length) <= text.size());

  ump_data mess;
  mess.common.messageType = static_cast<ump_message_type>(w0.mt.value());
  mess.common.status = static_cast<std::uint8_t>(w0.status);
  mess.form = w0.format;
  mess.data = std::span{text.data(), static_cast<std::size_t>(text_length)};
  config_.callbacks.functionBlockName(mess, fbIdx);
}

template <ump_processor_config Config> void umpProcessor<Config>::functionblock_info() {
  function_block_info info;
  auto const w0 = std::bit_cast<types::function_block_info_w0>(message_[0]);
  auto const w1 = std::bit_cast<types::function_block_info_w1>(message_[1]);

  info.fbIdx = w0.block_number;
  info.active = w0.a;
  info.direction = static_cast<function_block_info::fbdirection>(w0.dir.value());
  info.firstGroup = w1.first_group;
  info.groupLength = w1.groups_spanned;
  info.midiCIVersion = w1.message_version;
  info.isMIDI1 = w0.m1;
  info.maxS8Streams = w1.num_sysex8_streams;
  config_.callbacks.functionBlockInfo(info);
}

template <ump_processor_config Config> void umpProcessor<Config>::midi_endpoint_message(ump_message_type const mt) {
  // 128 bits UMP Stream Messages
  auto const status = static_cast<ump_stream>((message_[0] >> 16) & ((std::uint32_t{1} << 10) - 1U));
  switch (status) {
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

  case ump_stream::MIDIENDPOINT_PRODID_NOTIFICATION: this->midiendpoint_name_or_prodid(mt); break;
  case ump_stream::MIDIENDPOINT_PROTOCOL_REQUEST:  // JR Protocol Req
    config_.ump_stream.midiEndpointJRProtocolReq(static_cast<std::uint8_t>(message_[0] >> 8), (message_[0] >> 1) & 1,
                                                 message_[0] & 1);
    break;
  case ump_stream::MIDIENDPOINT_PROTOCOL_NOTIFICATION:  // JR Protocol Req
    config_.ump_stream.midiEndpointJRProtocolNotify(static_cast<std::uint8_t>(message_[0] >> 8), (message_[0] >> 1) & 1,
                                                    message_[0] & 1);
    break;

  case ump_stream::FUNCTIONBLOCK:
    config_.callbacks.functionBlock((message_[0] >> 8) & 0xFF,  // fbIdx
                                    message_[0] & 0xFF          // filter
    );
    break;

  case ump_stream::FUNCTIONBLOCK_INFO_NOTFICATION: this->functionblock_info(); break;
  case ump_stream::FUNCTIONBLOCK_NAME_NOTIFICATION: this->functionblock_name(); break;
  case ump_stream::STARTOFSEQ: config_.callbacks.startOfSeq(); break;
  case ump_stream::ENDOFFILE: config_.callbacks.endOfFile(); break;
  default: config_.callbacks.unknownUMPMessage(std::span{message_.data(), 4}); break;
  }
}

// 128 bit Data Messages (including System Exclusive 8)
template <ump_processor_config Config> void umpProcessor<Config>::data_message() {
  uint8_t const status = (message_[0] >> 20) & 0xF;
  switch (status) {
  case data_message_status::sysex8_in_1_ump:
  case data_message_status::sysex8_start:
  case data_message_status::sysex8_continue:
  case data_message_status::sysex8_end: {
    std::array<std::uint8_t, 13> sysex{};
    auto const w0 = std::bit_cast<types::sysex8_w0>(message_[0]);
    auto const data_length = std::min(std::size_t{w0.number_of_bytes}, sysex.size());
    if (data_length >= 1) {
      sysex[0] = w0.data;
      umpProcessor::payload(message_, 0, data_length - 1, std::begin(sysex) + 1);
    }
    ump_data mess;
    mess.common.group = w0.group;
    mess.common.messageType = static_cast<ump_message_type>(w0.mt.value());
    mess.streamId = w0.stream_id;
    mess.form = w0.status;
    assert(data_length <= sysex.size());
    mess.data = std::span{sysex.data(), data_length};
    config_.callbacks.send_out_sysex(mess);
  } break;
  case data_message_status::mixed_data_set_header:
  case data_message_status::mixed_data_set_payload: {
    // Beginning of Mixed Data Set
    // uint8_t mdsId  = (umpMess[0] >> 16) & 0xF;

    if (status == 8) {
      /*uint16_t numValidBytes  = umpMess[0] & 0xFFFF;
      uint16_t numChunk  = (umpMess[1] >> 16) & 0xFFFF;
      uint16_t numOfChunk  = umpMess[1] & 0xFFFF;
      uint16_t manuId  = (umpMess[2] >> 16) & 0xFFFF;
      uint16_t deviceId  = umpMess[2] & 0xFFFF;
      uint16_t subId1  = (umpMess[3] >> 16) & 0xFFFF;
      uint16_t subId2  = umpMess[3] & 0xFFFF;*/
    } else {
      // MDS bytes?
    }
    config_.callbacks.unknownUMPMessage(std::span{message_.data(), 4});
  } break;
  default: config_.callbacks.unknownUMPMessage(std::span{message_.data(), 4}); break;
  }
}

template <ump_processor_config Config> void umpProcessor<Config>::set_chord_name() {
  auto const w0 = std::bit_cast<types::set_chord_name_w0>(message_[0]);
  auto const w1 = std::bit_cast<types::set_chord_name_w1>(message_[1]);
  auto const w2 = std::bit_cast<types::set_chord_name_w2>(message_[2]);
  auto const w3 = std::bit_cast<types::set_chord_name_w3>(message_[3]);

  auto const valid_note = [](std::uint8_t n) {
    return n <= static_cast<std::uint8_t>(chord::note::G) ? static_cast<chord::note>(n) : chord::note::unknown;
  };

  auto const valid_chord_type = [](std::uint8_t ct) {
    return ct <= static_cast<std::uint8_t>(chord::chord_type::seven_suspended_4th) ? static_cast<chord::chord_type>(ct)
                                                                                   : chord::chord_type::no_chord;
  };

  chord c;
  // TODO(pbh): validate the ShrpFlt fields.
  c.chShrpFlt = static_cast<chord::sharps_flats>(w1.tonic_sharps_flats.signed_value());
  c.chTonic = valid_note(w1.chord_tonic);
  c.chType = valid_chord_type(w1.chord_type);
  c.chAlt1.type = w1.alter_1_type;
  c.chAlt1.degree = w1.alter_1_degree;
  c.chAlt2.type = w1.alter_2_type;
  c.chAlt2.degree = w1.alter_2_degree;
  c.chAlt3.type = w2.alter_3_type;
  c.chAlt3.degree = w2.alter_3_degree;
  c.chAlt4.type = w2.alter_4_type;
  c.chAlt4.degree = w2.alter_4_degree;
  c.baShrpFlt = static_cast<chord::sharps_flats>(w3.bass_sharps_flats.signed_value());
  c.baTonic = valid_note(w3.bass_note);
  c.baType = valid_chord_type(w3.bass_chord_type);
  c.baAlt1.type = w3.alter_1_type;
  c.baAlt1.degree = w3.alter_1_degree;
  c.baAlt2.type = w3.alter_2_type;
  c.baAlt2.degree = w3.alter_2_degree;
  config_.flex.chord(config_.context, w0.group, w0.addrs, w0.channel, c);
}

template <ump_processor_config Config>
void umpProcessor<Config>::flexdata_performance_or_lyric(ump_message_type const mt, std::uint8_t const group) {
  std::uint8_t const status_bank = (message_[0] >> 8) & 0xFF;
  std::uint8_t const status = message_[0] & 0xFF;
  std::uint8_t const channel = (message_[0] >> 16) & 0xF;
  std::uint8_t const addrs = (message_[0] >> 18) & 3;
  std::uint8_t const form = (message_[0] >> 20) & 3;

  std::array<std::uint8_t, 12> text{};
  auto text_length = 0U;
  for (uint8_t i = 1; i <= 3; i++) {
    for (int j = 24; j >= 0; j -= 8) {
      if (uint8_t const c = (message_[i] >> j) & 0xFF) {
        text[text_length++] = c;
      }
    }
  }
  assert(text_length <= text.size());

  ump_data mess;
  mess.common.group = group;
  mess.common.messageType = mt;
  mess.common.status = status;
  mess.form = form;
  mess.data = std::span{text.data(), text_length};
  if (status_bank == FLEXDATA_LYRIC) {
    config_.flex.lyric(config_.context, mess, addrs, channel);
  } else {
    assert(status_bank == FLEXDATA_PERFORMANCE);
    config_.flex.performance(config_.context, mess, addrs, channel);
  }
}

// 128 bit Data Messages (including System Exclusive 8)
template <ump_processor_config Config>
void umpProcessor<Config>::flexdata_message(ump_message_type const mt, std::uint8_t const group) {
  uint8_t const status_bank = (message_[0] >> 8) & 0xFF;
  uint8_t const status = message_[0] & 0xFF;
  uint8_t const channel = (message_[0] >> 16) & 0xF;
  uint8_t const addrs = (message_[0] >> 18) & 3;
  // SysEx 8
  switch (status_bank) {
  case FLEXDATA_COMMON: {  // Common/Configuration for MIDI File, Project, and Track
    switch (status) {
    case FLEXDATA_COMMON_TEMPO: {  // Set Tempo Message
      config_.flex.tempo(config_.context, group, message_[1]);
      break;
    }
    case FLEXDATA_COMMON_TIMESIG: {  // Set Time Signature Message
      config_.flex.time_sig(config_.context, group, (message_[1] >> 24) & 0xFF, (message_[1] >> 16) & 0xFF,
                            (message_[1] >> 8) & 0xFF);
      break;
    }
    case FLEXDATA_COMMON_METRONOME: {  // Set Metronome Message
      config_.flex.metronome(config_.context, group, (message_[1] >> 24) & 0xFF, (message_[1] >> 16) & 0xFF,
                             (message_[1] >> 8) & 0xFF, message_[1] & 0xFF, (message_[2] >> 24) & 0xFF,
                             (message_[2] >> 16) & 0xFF);
      break;
    }
    case FLEXDATA_COMMON_KEYSIG: {  // Set Key Signature Message
      config_.flex.key_sig(config_.context, group, addrs, channel, (message_[1] >> 24) & 0xFF,
                           (message_[1] >> 16) & 0xFF);
      break;
    }
    case FLEXDATA_COMMON_CHORD: this->set_chord_name(); break;
    default: config_.callbacks.unknownUMPMessage(std::span{message_.data(), 4}); break;
    }
    break;
  }
  case FLEXDATA_PERFORMANCE:
  case FLEXDATA_LYRIC: this->flexdata_performance_or_lyric(mt, group); break;

  default: config_.callbacks.unknownUMPMessage(std::span{message_.data(), 4}); break;
  }
}

}  // end namespace midi2

#endif  // MIDI2_UMP_PROCESSOR_HPP
