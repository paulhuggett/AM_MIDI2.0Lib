//===-- UMP Types -------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ump_types.hpp
/// \brief Defines UMP message types

#ifndef MIDI2_UMP_TYPES_HPP
#define MIDI2_UMP_TYPES_HPP

#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include "midi2/adt/bitfield.hpp"
#include "midi2/adt/uinteger.hpp"
#include "midi2/ump/ump_utils.hpp"

namespace midi2::ump {

/// Convert a fixed-size span<> of T to a tuple<> with the same number of elements in the same order. Each
/// member of the tuple must be constructible from T.
///
///     std::array<T1, 2> in = {...};
///     std::tuple<T2, T3> out = span_to_tuple(in);
///
/// (where instances of T2 and T3 can be constructed from T1).
template <typename T, std::size_t N> constexpr auto span_to_tuple(std::span<T, N> s) {
  auto const impl = []<std::size_t... Is>(std::span<T, N> sp, std::index_sequence<Is...>) {
    return std::tuple{sp[Is]...};
  };
  return impl(s, std::make_index_sequence<N>{});
}

#define MIDI2_UMP_MESSAGE_TYPES \
  MIDI2_X(utility, 0x00)        \
  MIDI2_X(system, 0x01)         \
  MIDI2_X(m1cvm, 0x02)          \
  MIDI2_X(data64, 0x03)         \
  MIDI2_X(m2cvm, 0x04)          \
  MIDI2_X(data128, 0x05)        \
  MIDI2_X(reserved32_06, 0x06)  \
  MIDI2_X(reserved32_07, 0x07)  \
  MIDI2_X(reserved64_08, 0x08)  \
  MIDI2_X(reserved64_09, 0x09)  \
  MIDI2_X(reserved64_0a, 0x0A)  \
  MIDI2_X(reserved96_0b, 0x0B)  \
  MIDI2_X(reserved96_0c, 0x0C)  \
  MIDI2_X(flex_data, 0x0D)      \
  MIDI2_X(reserved128_0e, 0x0E) \
  MIDI2_X(stream, 0x0F)

#define MIDI2_X(a, b) a = (b),
enum class message_type : std::uint8_t { MIDI2_UMP_MESSAGE_TYPES };
#undef MIDI2_X

/// Collects the enumerations that define the values for the UMP "mt" (message type) field
namespace mt {

// Here, CRT is short for "common and real-time".
/// The message types for the System Common and System Real-time messages
enum class system_crt : std::uint8_t {
  // System Common messages
  timing_code = 0xF1,   ///< MIDI Time Code
  spp = 0xF2,           ///< Song Position Pointer
  song_select = 0xF3,   ///< Song Select
  tune_request = 0xF6,  ///< Tune Request
  timing_clock = 0xF8,  ///< Timing Clock
  // System Real Time messages
  sequence_start = 0xFA,     ///< Start the current sequence playing
  sequence_continue = 0xFB,  ///< Continue at the point the sequence was stopped
  sequence_stop = 0xFC,      ///< Stop the current sequence
  active_sensing = 0xFE,     ///< Active Sensing
  system_reset = 0xFF,       ///< Reset
};

/// The message types for the MIDI 1 Channel Voice messages
enum class m1cvm : std::uint8_t {
  note_off = 0x8,          ///< Note Off
  note_on = 0x9,           ///< Note On
  poly_pressure = 0xA,     ///< Polyphonic Key Pressure (Aftertouch).
  cc = 0xB,                ///< Continuous Controller
  program_change = 0xC,    /// Program Change
  channel_pressure = 0xD,  ///< Channel Pressure (Aftertouch).
  pitch_bend = 0xE,        ///< Pitch Bend
};

/// Message types for the MIDI 2 Channel Voice messages
enum class m2cvm : std::uint8_t {
  rpn_per_note = 0x0,         ///< Registered per-note controller
  nrpn_per_note = 0x1,        ///< Assignable per-note controller
  rpn = 0x2,                  ///< Registered parameter number
  nrpn = 0x3,                 ///< Assignable controller number
  rpn_relative = 0x4,         ///< Relative registered controller number
  nrpn_relative = 0x5,        ///< Relative non-registered controller Number
  pitch_bend_per_note = 0x6,  ///< Per-note patch bend
  note_off = 0x8,             ///< Note off
  note_on = 0x9,              ///< Note on
  poly_pressure = 0xA,        ///< Polyphonic aftertouch
  cc = 0xB,                   ///< Continuous controller
  program_change = 0xC,       ///< Program change
  channel_pressure = 0xD,     ///< Channel pressure (aftertouch)
  pitch_bend = 0xE,           ///< Pitch bend
  per_note_manage = 0xF,      ///< Per-note management
};

/// Message types for the Data 64 Bit messages
enum class data64 : std::uint8_t {
  /// Complete system exclusive message in one UMP.
  sysex7_in_1 = 0x00,
  /// System exclusive start UMP. Terminate with a System Exclusive End UMP.
  sysex7_start = 0x01,
  /// System exclusive continue UMP.
  sysex7_continue = 0x02,
  /// System exclusive end UMP.
  sysex7_end = 0x03,
};

/// Status codes for UMP messages in the Utility group
enum class utility : std::uint8_t {
  noop = 0b0000,               ///< A "no operation" message
  jr_clock = 0b0001,           ///< Jitter reduction clock
  jr_ts = 0b0010,              ///< Jitter reduction time-stamp
  delta_clock_tick = 0b0011,   ///< Delta Clockstamp: Ticks Per Quarter Note
  delta_clock_since = 0b0100,  ///< Delta Clockstamp: Ticks Since Last Event
};

/// Status codes for UMP messages in the Flex Data group
enum class flex_data : std::uint8_t {
  set_tempo = 0x00,
  set_time_signature = 0x01,
  set_metronome = 0x02,
  set_key_signature = 0x05,
  set_chord_name = 0x06,
};

/// Status codes for UMP messages in the UMP Stream group
enum class stream : std::uint16_t {
  endpoint_discovery = 0x00,
  endpoint_info_notification = 0x01,
  device_identity_notification = 0x02,
  endpoint_name_notification = 0x03,
  product_instance_id_notification = 0x04,
  jr_configuration_request = 0x05,
  jr_configuration_notification = 0x06,
  function_block_discovery = 0x10,
  function_block_info_notification = 0x11,
  function_block_name_notification = 0x12,
  start_of_clip = 0x20,
  end_of_clip = 0x21,
};

/// Status codes for UMP messages in the DATA 128 Bit group
enum class data128 : std::uint8_t {
  /// Status code for the midi2::ump::data128::sysex8_in_1 message.
  sysex8_in_1 = 0x00,
  /// Status code for the midi2::ump::data128::sysex8_start message.
  sysex8_start = 0x01,
  /// Status code for the midi2::ump::data128::sysex8_continue message.
  sysex8_continue = 0x02,
  /// Status code for the midi2::ump::data128::sysex8_end message.
  sysex8_end = 0x03,
  /// Status code for the midi2::ump::data128::mds_header message.
  mixed_data_set_header = 0x08,
  /// Status code for the midi2::ump::data128::mds_header message.
  mixed_data_set_payload = 0x09,
};

}  // end namespace mt

/// \brief Private implementation details of the UMP types and functions
namespace details {

/// \brief Converts one of the status enumerations to the corresponding message_type enumeration value
/// This value is used by the "mt" field in UMP messages.
/// \tparam T One fo the status enumerations defined in midi2::ump::mt
template <typename T>
  requires std::is_enum_v<T>
struct status_to_message_type;
template <> struct status_to_message_type<mt::system_crt> {
  static constexpr auto value = message_type::system;
};
template <> struct status_to_message_type<mt::utility> {
  static constexpr auto value = message_type::utility;
};
template <> struct status_to_message_type<mt::m1cvm> {
  static constexpr auto value = message_type::m1cvm;
};
template <> struct status_to_message_type<mt::data64> {
  static constexpr auto value = message_type::data64;
};
template <> struct status_to_message_type<mt::m2cvm> {
  static constexpr auto value = message_type::m2cvm;
};
template <> struct status_to_message_type<mt::data128> {
  static constexpr auto value = message_type::data128;
};
template <> struct status_to_message_type<mt::flex_data> {
  static constexpr auto value = message_type::flex_data;
};
template <> struct status_to_message_type<mt::stream> {
  static constexpr auto value = message_type::stream;
};

template <typename T>
  requires std::is_enum_v<T>
constexpr message_type status_to_message_type_v = status_to_message_type<T>::value;

class word_base : public adt::bit_field<std::uint32_t> {
public:
  constexpr word_base() noexcept = default;
  constexpr explicit word_base(value_type const v) noexcept : adt::bit_field<std::uint32_t>{v} {}

  friend constexpr bool operator==(word_base const &a, word_base const &b) noexcept = default;

  template <adt::bit_range_type Field, typename Enumeration>
    requires(std::is_enum_v<Enumeration>)
  constexpr auto &set_enum_field(Enumeration const v) noexcept {
    if constexpr (std::is_unsigned_v<std::underlying_type_t<Enumeration>>) {
      this->template set<Field>(std::to_underlying(v));
    } else {
      this->template set_signed<Field>(std::to_underlying(v));
    }
    return *this;
  }
  template <adt::bit_range_type Field, typename Enumeration>
    requires(std::is_enum_v<Enumeration>)
  constexpr auto get_enum_field_raw() const noexcept {
    if constexpr (std::is_unsigned_v<std::underlying_type_t<Enumeration>>) {
      return this->template get<Field>();
    } else {
      return this->template get_signed<Field>();
    }
  }
  template <adt::bit_range_type Field, typename Enumeration>
    requires(std::is_enum_v<Enumeration>)
  constexpr Enumeration get_enum_field() const noexcept {
    return static_cast<Enumeration>(this->get_enum_field_raw<Field, Enumeration>());
  }

protected:
  template <adt::bit_range_type MtField, adt::bit_range_type StatusField, typename StatusType>
    requires std::is_enum_v<StatusType>
  constexpr void init(StatusType const status) noexcept {
    this->set<MtField>(std::to_underlying(status_to_message_type_v<StatusType>));
    this->set<StatusField>(std::to_underlying(status));
  }

  template <typename StatusType>
    requires std::is_enum_v<StatusType>
  static constexpr auto underlying_mt(StatusType const /*s*/) {
    return std::to_underlying(ump::details::status_to_message_type_v<std::remove_const_t<StatusType>>);
  }

  template <adt::bit_range_type MtField, adt::bit_range_type StatusField, typename StatusType>
    requires std::is_enum_v<StatusType>
  [[nodiscard]] constexpr bool check(StatusType const status) const noexcept {
    return this->get<MtField>() == underlying_mt(status) && this->get<StatusField>() == std::to_underlying(status);
  }
};

}  // end namespace details

/// Calls the supplied function for each of the words in a UMP message of type \p T.
///
/// \tparam T  A message type. One of the types defined in the midi2::ump namespace.
/// \tparam Function  A function accepting a std::uint32_t message word. Returns a value which can be cast to
///   bool.
/// \tparam Index  The index at which processing should start.
/// \param message   An instance of one of the UMP message types.
/// \param function  The function to be called for each member of the UMP message type.
/// \returns  The result of the calling \p function which was converted to boolean true, or the
///   last result received.
template <typename T, typename Function, std::size_t Index = 0>
  requires(Index < std::tuple_size_v<T> &&
           std::derived_from<std::remove_cvref_t<decltype(get<0>(T{}))>, details::word_base> &&
           std::is_constructible_v<bool, std::invoke_result_t<Function, std::uint32_t>>)
constexpr auto apply(T const &message, Function function) {
  auto const result = function(static_cast<std::uint32_t>(get<Index>(message)));
  if (bool{result}) {
    return result;
  }
  if constexpr (Index + 1 >= std::tuple_size_v<T>) {
    return result;
  } else {
    return apply<T, Function, Index + 1>(message, std::move(function));
  }
}

/// Calls the check() method of each word of a UMP message of type \p T.
///
/// This can be used to validate the internal consistency of a complete UMP message.
///
/// \tparam T  A message type. One of the types defined in the midi2::ump namespace.
/// \tparam Index  The index at which processing should start.
/// \param message   An instance of one of the UMP message types.
/// \returns  True if each of the check() functions returns true, false otherwise.
template <typename T, std::size_t Index = 0>
  requires(Index < std::tuple_size_v<T> &&
           std::derived_from<std::remove_cvref_t<decltype(get<0>(T{}))>, details::word_base>)
constexpr bool check(T const &message) {
  if (!get<Index>(message).check()) {
    return false;
  }
  if constexpr (Index + 1 >= std::tuple_size_v<T>) {
    return true;
  } else {
    return check<T, Index + 1>(message);
  }
}

}  // end namespace midi2::ump

/// Creates a "getter" member function for classes derived from midi2::ump::details::word_base.
#define MIDI2_UMP_GETTER(word, field)                                   \
  constexpr auto field() const noexcept {                               \
    return std::get<word>(words_).template get<typename word::field>(); \
  }
#define MIDI2_UMP_GETTER_ENUM(word, field, enumeration)                                             \
  constexpr auto field##_raw() const noexcept {                                                     \
    return std::get<word>(words_).template get_enum_field_raw<typename word::field, enumeration>(); \
  }                                                                                                 \
  constexpr auto field() const noexcept {                                                           \
    return std::get<word>(words_).template get_enum_field<typename word::field, enumeration>();     \
  }

/// Creates a "setter" member function for classes derived from midi2::ump::details::word_base.
#define MIDI2_UMP_SETTER(word, field)                                                 \
  constexpr auto &field(adt::uinteger_t<word::field::bits::value> const v) noexcept { \
    std::get<word>(words_).template set<typename word::field>(v);                     \
    return *this;                                                                     \
  }
#define MIDI2_UMP_SETTER_ENUM(word, field, enumeration)                                   \
  constexpr auto &field(enum enumeration const v) noexcept {                              \
    std::get<word>(words_).template set_enum_field<typename word::field, enumeration>(v); \
    return *this;                                                                         \
  }

/// Creates both "getter" and "setter" member functions for classes derived from midi2::ump::details::word_base.
#define MIDI2_UMP_GETTER_SETTER(word, field) \
  MIDI2_UMP_GETTER(word, field)              \
  MIDI2_UMP_SETTER(word, field)
#define MIDI2_UMP_GETTER_SETTER_ENUM(word, field, enumeration) \
  MIDI2_UMP_GETTER_ENUM(word, field, enumeration)              \
  MIDI2_UMP_SETTER_ENUM(word, field, enumeration)

/// This macro is used to generate boilerplate definitions of three items for the class specified by the \p group
/// and \p message parameters:
/// 1. A specialization of std::tuple_size<>
/// 2. A specialization of std::tuple_element<>
/// 3. A static assertion that the tuple size for the class matches that of the group as a whole
#define MIDI2_UMP_TUPLE(group, message)                                                                              \
  template <>                                                                                                        \
  struct std::tuple_size<::midi2::ump::group::message> /* NOLINT(cert-dcl58-cpp] */                                  \
      : std::integral_constant<std::size_t, std::tuple_size_v<decltype(::midi2::ump::group::message::words_)>> {};   \
  template <std::size_t I> struct std::tuple_element<I, ::midi2::ump::group::message> { /* NOLINT(cert-dcl58-cpp] */ \
    using type = std::tuple_element_t<I, decltype(::midi2::ump::group::message::words_)>;                            \
  };                                                                                                                 \
  static_assert(std::tuple_size_v<::midi2::ump::group::message> ==                                                   \
                ::midi2::ump::message_size<::midi2::ump::message_type::group>::value);

/// Defines a constructor for the class specified by the \p group and \p message parameters which takes a span of
/// std::uint32_t with a size equal to the number of words in the message. The constructor initialises the message from
/// the span and checks that the message type and status fields are correct.
#define MIDI2_SPAN_CTOR(group, message)                                            \
  constexpr ::midi2::ump::group::message::message(                                 \
      ::std::span<::std::uint32_t, ::std::tuple_size_v<message>> const m) noexcept \
      : words_{span_to_tuple(m)} {                                                 \
    assert(get<0>(words_).check());                                                \
  }

namespace midi2::ump {

/// \brief An integral constant which holds the number of bytes of a specific UMP message type.
template <message_type> struct message_size;  // not defined

}  // end namespace midi2::ump

//*       _   _ _ _ _         *
//*  _  _| |_(_) (_) |_ _  _  *
//* | || |  _| | | |  _| || | *
//*  \_,_|\__|_|_|_|\__|\_, | *
//*                     |__/  *
/// \brief An integral constant which holds the number of bytes of a UMP utility message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::utility> : std::integral_constant<unsigned, 1> {};

/// Defines the C++ types that represent Utility type messages
namespace midi2::ump::utility {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return std::get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return std::get<I>(t.words_);
}

class noop;
class jr_clock;
class jr_timestamp;
class delta_clockstamp_tpqn;
class delta_clockstamp;

}  // end namespace midi2::ump::utility

// F.1.1 Message Type 0x0: Utility
// Table 26 4-Byte UMP Formats for Message Type 0x0: Utility

/// \brief The NOOP message (section 7.2.1)
class midi2::ump::utility::noop {
public:
  /// The word of a NOOP message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::utility::noop;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept {
      return this->word_base::check<mt, status>(message) && this->get<reserved>() == 0;
    }

    /// Defines the bit position of the mt (message-type) field. Always 0.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the status field for the NOOP message
    using status = adt::bit_range<20, 4>;

    using reserved = adt::bit_range<0, 20>;
  };

  constexpr noop() noexcept = default;
  friend constexpr bool operator==(noop const &, noop const &) noexcept = default;

private:
  friend struct ::std::tuple_size<noop>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(utility, noop)  // Define tuple_size and tuple_element for noop

// 7.2.2.1 JR Clock Message
/// \brief The JR Clock message (section 7.2.2.1)
class midi2::ump::utility::jr_clock {
public:
  /// The first word of a Jitter Reduction Clock message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::utility::jr_clock;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0.
    using mt = adt::bit_range<28, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x1.
    using status = adt::bit_range<20, 4>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<16, 4>;
    /// \brief Sender Clock Time
    /// A 16-bit time value in clock ticks of 1/31250 of one second (32 µsec, clock frequency of 1 MHz / 32).
    using sender_clock_time = adt::bit_range<0, 16>;
  };

  constexpr jr_clock() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit jr_clock(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(jr_clock const &, jr_clock const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, sender_clock_time)

private:
  friend struct ::std::tuple_size<jr_clock>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(utility, jr_clock)  // Define tuple_size and tuple_element for jr_clock
MIDI2_SPAN_CTOR(utility, jr_clock)  // Define the span constructor for jr_clock

// 7.2.2.2 Jitter-Reduction Timestamp Message
/// \brief The Jitter-Reduction Timestamp message (section 7.2.2.2)
class midi2::ump::utility::jr_timestamp {
public:
  /// The first word of a Jitter-Reduction Timestamp message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::utility::jr_ts;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0.
    using mt = adt::bit_range<28, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x2.
    using status = adt::bit_range<20, 4>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<16, 4>;
    /// \brief Sender Clock Timestamp
    /// A 16-bit time value in clock ticks of 1/31250 of one second (32 µsec, clock frequency of 1 MHz / 32).
    using timestamp = adt::bit_range<0, 16>;
  };

  constexpr jr_timestamp() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit jr_timestamp(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(jr_timestamp const &, jr_timestamp const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, timestamp)

private:
  friend struct ::std::tuple_size<jr_timestamp>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(utility, jr_timestamp)  // Define tuple_size and tuple_element for jr_timestamp
MIDI2_SPAN_CTOR(utility, jr_timestamp)  // Define the span constructor for jr_timestamp

// 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (TPQN)
/// \brief The Delta Clockstamp Ticks Per Quarter Note (TPQN) message
class midi2::ump::utility::delta_clockstamp_tpqn {
public:
  /// \brief The first word of a Delta Clockstamp TPQN message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::utility::delta_clock_tick;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0.
    using mt = adt::bit_range<28, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x3.
    using status = adt::bit_range<20, 4>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<16, 4>;
    ///
    using ticks_pqn = adt::bit_range<0, 16>;
  };

  constexpr delta_clockstamp_tpqn() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit delta_clockstamp_tpqn(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(delta_clockstamp_tpqn const &, delta_clockstamp_tpqn const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, ticks_pqn)

private:
  friend struct ::std::tuple_size<delta_clockstamp_tpqn>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(utility, delta_clockstamp_tpqn)  // Define tuple_size and tuple_element for delta_clockstamp_tpqn
MIDI2_SPAN_CTOR(utility, delta_clockstamp_tpqn)  // Define the span constructor for delta_clockstamp_tpqn

/// \brief The Delta Clockstamp (DC): Ticks Since Last Event message
class midi2::ump::utility::delta_clockstamp {
public:
  /// \brief The first word of a Delta Clockstamp Since Last Event message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::utility::delta_clock_since;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0.
    using mt = adt::bit_range<28, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0b0100.
    using status = adt::bit_range<20, 4>;
    using ticks_per_quarter_note = adt::bit_range<0, 20>;
  };

  constexpr delta_clockstamp() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit delta_clockstamp(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(delta_clockstamp const &, delta_clockstamp const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, ticks_per_quarter_note)

private:
  friend struct ::std::tuple_size<delta_clockstamp>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(utility, delta_clockstamp)  // Define tuple_size and tuple_element for delta_clockstamp
MIDI2_SPAN_CTOR(utility, delta_clockstamp)  // Define the span constructor for delta_clockstamp

//*             _              *
//*  ____  _ __| |_ ___ _ __   *
//* (_-< || (_-<  _/ -_) '  \  *
//* /__/\_, /__/\__\___|_|_|_| *
//*     |__/                   *
// 7.6 System Common and System Real Time Messages

/// \brief An integral constant which holds the number of bytes of a UMP system message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::system> : std::integral_constant<unsigned, 1> {};

/// Defines the C++ types that represent System type messages
namespace midi2::ump::system {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

class midi_time_code;
class song_position_pointer;
class song_select;
class tune_request;
class timing_clock;
class sequence_start;
class sequence_continue;
class sequence_stop;
class active_sensing;
class reset;

}  // namespace midi2::ump::system

/// @brief  The contents of the MIDI time code message
class midi2::ump::system::midi_time_code {
public:
  /// \brief The first and only word of the MIDI time code message
  class word0 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::system_crt::timing_code;
    constexpr word0() noexcept { this->init<mt, status>(ump::mt::system_crt::timing_code); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1
    using mt = adt::bit_range<28U, 4U>;
    /// Defines the bit position of the group field
    using group = adt::bit_range<24U, 4U>;
    /// Defines the bit position of the status field. Always 0xF1
    using status = adt::bit_range<16U, 8U>;
    /// Defines a reserved bit
    using reserved0 = adt::bit_range<15, 1>;
    /// 7 bit time code 0xnd
    using time_code = adt::bit_range<8, 7>;
    /// Defines a group of reserved bits
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines a group of reserved bits
    using reserved2 = adt::bit_range<0, 7>;
  };

  constexpr midi_time_code() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit midi_time_code(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(midi_time_code const &, midi_time_code const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, time_code)

private:
  friend struct ::std::tuple_size<midi_time_code>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, midi_time_code)  // Define tuple_size and tuple_element for midi_time_code
MIDI2_SPAN_CTOR(system, midi_time_code)  // Define the span constructor for midi_time_code

/// \brief The contents of the MIDI song position pointer message
class midi2::ump::system::song_position_pointer {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::system_crt::spp;
    constexpr word0() noexcept { this->init<mt, status>(ump::mt::system_crt::spp); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<16, 8>;  ///< Always 0xF2
    /// Defines a reserved bit
    using reserved0 = adt::bit_range<15, 1>;
    using position_lsb = adt::bit_range<8, 7>;
    /// Defines a reserved bit
    using reserved1 = adt::bit_range<7, 1>;
    using position_msb = adt::bit_range<0, 7>;
  };

  constexpr song_position_pointer() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit song_position_pointer(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(song_position_pointer const &, song_position_pointer const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, position_lsb)
  MIDI2_UMP_GETTER_SETTER(word0, position_msb)

private:
  friend struct ::std::tuple_size<song_position_pointer>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, song_position_pointer)  // Define tuple_size and tuple_element for song_position_pointer
MIDI2_SPAN_CTOR(system, song_position_pointer)  // Define the span constructor for song_position_pointer

/// \brief The contents of the MIDI song select message
class midi2::ump::system::song_select {
public:
  /// \brief The first and only word of the MIDI song select message
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::system_crt::song_select;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    ///< Always 0xF3
    using status = adt::bit_range<16, 8>;
    /// Defines a reserved bit
    using reserved0 = adt::bit_range<15, 1>;
    using song = adt::bit_range<8, 7>;
    /// Defines a reserved bit
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines a group of reserved bits
    using reserved2 = adt::bit_range<0, 7>;
  };

  constexpr song_select() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit song_select(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(song_select const &, song_select const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, song)

private:
  friend struct ::std::tuple_size<song_select>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, song_select)  // Define tuple_size and tuple_element for song_select
MIDI2_SPAN_CTOR(system, song_select)  // Define the span constructor for song_select

/// \brief The contents of the MIDI tune-request message
class midi2::ump::system::tune_request {
public:
  /// \brief The contents of the first and only word of a MIDI tune-request message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = mt::system_crt::tune_request;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<16, 8>;  ///< Always 0xF6
    using reserved0 = adt::bit_range<8, 8>;
    using reserved1 = adt::bit_range<0, 8>;
  };

  constexpr tune_request() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit tune_request(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(tune_request const &, tune_request const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<tune_request>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, tune_request)  // Define tuple_size and tuple_element for tune_request
MIDI2_SPAN_CTOR(system, tune_request)  // Define the span constructor for tune_request

//
class midi2::ump::system::timing_clock {
public:
  /// \brief The contents of the MIDI timing-clock message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = mt::system_crt::timing_clock;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<16, 8>;  ///< Always 0xF8
    using reserved0 = adt::bit_range<8, 8>;
    using reserved1 = adt::bit_range<0, 8>;
  };

  constexpr timing_clock() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit timing_clock(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(timing_clock const &, timing_clock const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<timing_clock>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, timing_clock)  // Define tuple_size and tuple_element for timing_clock
MIDI2_SPAN_CTOR(system, timing_clock)  // Define the span constructor for timing_clock

/// \brief Defines the MIDI sequence-start message
class midi2::ump::system::sequence_start {
public:
  /// \brief The contents of the first and only word of a MIDI sequence-start message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = mt::system_crt::sequence_start;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<16, 8>;  ///< Always 0xFA
    using reserved0 = adt::bit_range<8, 8>;
    using reserved1 = adt::bit_range<0, 8>;
  };

  constexpr sequence_start() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit sequence_start(std::span<std::uint32_t, 1> w0) noexcept;
  friend constexpr bool operator==(sequence_start const &, sequence_start const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<sequence_start>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, sequence_start)  // Define tuple_size and tuple_element for sequence_start
MIDI2_SPAN_CTOR(system, sequence_start)  // Define the span constructor for sequence_start

/// \brief Defines the MIDI sequence-continue message
class midi2::ump::system::sequence_continue {
public:
  /// \brief The contents of the first and only word of a MIDI sequence-continue message
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::system_crt::sequence_continue;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<16, 8>;  ///< Always 0xFB
    using reserved0 = adt::bit_range<8, 8>;
    using reserved1 = adt::bit_range<0, 8>;
  };

  constexpr sequence_continue() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit sequence_continue(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(sequence_continue const &, sequence_continue const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<sequence_continue>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, sequence_continue)  // Define tuple_size and tuple_element for sequence_continue
MIDI2_SPAN_CTOR(system, sequence_continue)  // Define the span constructor for sequence_continue

/// \brief Defines the MIDI sequence-stop message
class midi2::ump::system::sequence_stop {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::system_crt::sequence_stop;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<16, 8>;  ///< Always 0xFC
    using reserved0 = adt::bit_range<8, 8>;
    using reserved1 = adt::bit_range<0, 8>;
  };

  constexpr sequence_stop() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit sequence_stop(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(sequence_stop const &, sequence_stop const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<sequence_stop>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, sequence_stop)  // Define tuple_size and tuple_element for sequence_stop
MIDI2_SPAN_CTOR(system, sequence_stop)  // Define the span constructor for sequence_stop

/// \brief Defines the MIDI active-sensing message
class midi2::ump::system::active_sensing {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::system_crt::active_sensing;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<16, 8>;  ///< Always 0xFE
    using reserved0 = adt::bit_range<8, 8>;
    using reserved1 = adt::bit_range<0, 8>;
  };

  constexpr active_sensing() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit active_sensing(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(active_sensing const &, active_sensing const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<active_sensing>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, active_sensing)  // Define tuple_size and tuple_element for active_sensing
MIDI2_SPAN_CTOR(system, active_sensing)  // Define the span constructor for active_sensing

/// \brief Defines the MIDI reset message
class midi2::ump::system::reset {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::system_crt::system_reset;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<16, 8>;  ///< Always 0xFF
    using reserved0 = adt::bit_range<8, 8>;
    using reserved1 = adt::bit_range<0, 8>;
  };

  constexpr reset() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit reset(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(reset const &, reset const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<reset>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(system, reset)  // Define tuple_size and tuple_element for system/reset
MIDI2_SPAN_CTOR(system, reset)  // Define the span constructor for system/reset

//*        _                 *
//*  _ __ / |  ____ ___ __   *
//* | '  \| | / _\ V / '  \  *
//* |_|_|_|_| \__|\_/|_|_|_| *
//*                          *
// F.1.3 Mess Type 0x2: MIDI 1.0 Channel Voice Messages
// Table 28 4-Byte UMP Formats for Message Type 0x2: MIDI 1.0 Channel Voice
// Messages

/// \brief An integral constant which holds the number of bytes of a UMP m1cvm message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::m1cvm> : std::integral_constant<unsigned, 1> {};

/// Defines the C++ types that represent MIDI 1.0 Channel Voice type messages
namespace midi2::ump::m1cvm {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

class note_on;
class note_off;
class poly_pressure;
class control_change;
class program_change;
class channel_pressure;
class pitch_bend;

}  // end namespace midi2::ump::m1cvm

// 7.3.2 MIDI 1.0 Note On Message
class midi2::ump::m1cvm::note_on {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::m1cvm::note_on;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (MIDI 1.0 Channel Voice).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0x09.
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<7, 1>;
    using velocity = adt::bit_range<0, 7>;
  };

  constexpr note_on() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit note_on(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(note_on const &, note_on const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word0, velocity)

private:
  friend struct ::std::tuple_size<note_on>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(m1cvm, note_on)  // Define tuple_size and tuple_element for m1cvm/note_on
MIDI2_SPAN_CTOR(m1cvm, note_on)  // Define the span constructor for m1cvm/note_on

// 7.3.1 MIDI 1.0 Note Off Message
class midi2::ump::m1cvm::note_off {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::m1cvm::note_off;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (MIDI 1.0 Channel Voice).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0x08.
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<7, 1>;
    using velocity = adt::bit_range<0, 7>;
  };

  constexpr note_off() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit note_off(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(note_off const &, note_off const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word0, velocity)

private:
  friend struct ::std::tuple_size<note_off>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(m1cvm, note_off)  // Define tuple_size and tuple_element for m1cvm/note_off
MIDI2_SPAN_CTOR(m1cvm, note_off)  // Define the span constructor for m1cvm/note_off

// 7.3.3 MIDI 1.0 Poly Pressure Message
class midi2::ump::m1cvm::poly_pressure {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::m1cvm::poly_pressure;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (MIDI 1.0 Channel Voice).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0x08.
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<7, 1>;
    using pressure = adt::bit_range<0, 7>;
  };

  constexpr poly_pressure() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit poly_pressure(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(poly_pressure const &, poly_pressure const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word0, pressure)

private:
  friend struct ::std::tuple_size<poly_pressure>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(m1cvm, poly_pressure)  // Define tuple_size and tuple_element for m1cvm/poly_pressure
MIDI2_SPAN_CTOR(m1cvm, poly_pressure)  // Define the span constructor for m1cvm/poly_pressure

// 7.3.4 MIDI 1.0 Control Change Message
class midi2::ump::m1cvm::control_change {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::m1cvm::cc;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (MIDI 1.0 Channel Voice).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  /// Always 0x0B.
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using controller = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<7, 1>;
    using value = adt::bit_range<0, 7>;
  };

  constexpr control_change() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit control_change(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(control_change const &, control_change const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, controller)
  MIDI2_UMP_GETTER_SETTER(word0, value)

  constexpr auto &controller(ump::control const c) noexcept { return this->controller(std::to_underlying(c)); }

private:
  friend struct ::std::tuple_size<control_change>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(m1cvm, control_change)  // Define tuple_size and tuple_element for m1cvm/control_change
MIDI2_SPAN_CTOR(m1cvm, control_change)  // Define the span constructor for m1cvm/control_change

// 7.3.5 MIDI 1.0 Program Change Message
class midi2::ump::m1cvm::program_change {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::m1cvm::program_change;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (MIDI 1.0 Channel Voice).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0x0C.
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using program = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<0, 8>;
  };

  constexpr program_change() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit program_change(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(program_change const &, program_change const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, program)

private:
  friend struct ::std::tuple_size<program_change>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(m1cvm, program_change)  // Define tuple_size and tuple_element for m1cvm/program_change
MIDI2_SPAN_CTOR(m1cvm, program_change)  // Define the span constructor for m1cvm/program_change

// 7.3.6 MIDI 1.0 Channel Pressure Message
class midi2::ump::m1cvm::channel_pressure {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::m1cvm::channel_pressure;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (MIDI 1.0 Channel Voice).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0x08.
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using data = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<0, 8>;
  };

  constexpr channel_pressure() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit channel_pressure(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(channel_pressure const &, channel_pressure const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, data)

private:
  friend struct ::std::tuple_size<channel_pressure>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(m1cvm, channel_pressure)  // Define tuple_size and tuple_element for m1cvm/channel_pressure
MIDI2_SPAN_CTOR(m1cvm, channel_pressure)  // Define the span constructor for m1cvm/channel_pressure

// 7.3.7 MIDI 1.0 Pitch Bend Message
class midi2::ump::m1cvm::pitch_bend {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = ump::mt::m1cvm::pitch_bend;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (MIDI 1.0 Channel Voice).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  // 0b1000..0b1110
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using lsb_data = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<7, 1>;
    using msb_data = adt::bit_range<0, 7>;
  };
  constexpr pitch_bend() noexcept = default;
  /// Constructs from a raw 32-bit message.
  /// In a debug build, checks that the class invariants hold.
  constexpr explicit pitch_bend(std::span<std::uint32_t, 1> m) noexcept;
  friend constexpr bool operator==(pitch_bend const &, pitch_bend const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, lsb_data)
  MIDI2_UMP_GETTER_SETTER(word0, msb_data)

private:
  friend struct ::std::tuple_size<pitch_bend>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};
MIDI2_UMP_TUPLE(m1cvm, pitch_bend)  // Define tuple_size and tuple_element for m1cvm/pitch_bend
MIDI2_SPAN_CTOR(m1cvm, pitch_bend)  // Define the span constructor for m1cvm/pitch_bend

//*     _      _         __ _ _   *
//*  __| |__ _| |_ __ _ / /| | |  *
//* / _` / _` |  _/ _` / _ \_  _| *
//* \__,_\__,_|\__\__,_\___/ |_|  *
//*                               *

/// \brief An integral constant which holds the number of bytes of a UMP data64 message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::data64> : std::integral_constant<unsigned, 2> {};

namespace midi2::ump::data64::details {

// 7.7 System Exclusive (7-Bit) Messages
template <midi2::ump::mt::data64 Status> class sysex7;

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

}  // end namespace midi2::ump::data64::details

template <midi2::ump::mt::data64 Status> class midi2::ump::data64::details::sysex7 {
public:
  class word0 : public ump::details::word_base {
  public:
    static constexpr auto message = Status;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;
    using number_of_bytes = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using data0 = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<7, 1>;
    using data1 = adt::bit_range<0, 7>;
  };
  class word1 : public ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using reserved0 = adt::bit_range<31, 1>;
    using data2 = adt::bit_range<24, 7>;
    using reserved1 = adt::bit_range<23, 1>;
    using data3 = adt::bit_range<16, 7>;
    using reserved2 = adt::bit_range<15, 1>;
    using data4 = adt::bit_range<8, 7>;
    using reserved3 = adt::bit_range<7, 1>;
    using data5 = adt::bit_range<0, 7>;
  };

  constexpr sysex7() noexcept = default;
  constexpr explicit sysex7(std::span<std::uint32_t, 2> const m) noexcept : words_{span_to_tuple(m)} {
    assert(get<0>(words_).check());
  }
  friend constexpr bool operator==(sysex7 const &, sysex7 const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, number_of_bytes)
  MIDI2_UMP_GETTER_SETTER(word0, data0)
  MIDI2_UMP_GETTER_SETTER(word0, data1)
  MIDI2_UMP_GETTER_SETTER(word1, data2)
  MIDI2_UMP_GETTER_SETTER(word1, data3)
  MIDI2_UMP_GETTER_SETTER(word1, data4)
  MIDI2_UMP_GETTER_SETTER(word1, data5)

private:
  friend struct ::std::tuple_size<sysex7>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_{};
};

template <midi2::ump::mt::data64 Status>
struct std::tuple_size<midi2::ump::data64::details::sysex7<Status>> /* NOLINT(cert-dcl58-cpp]*/
    : std::integral_constant<std::size_t,
                             std::tuple_size_v<decltype(midi2::ump::data64::details::sysex7<Status>::words_)>> {};

template <std::size_t I, midi2::ump::mt::data64 Status>
struct std::tuple_element<I, midi2::ump::data64::details::sysex7<Status>> { /* NOLINT(cert-dcl58-cpp] */
  /// The type of Ith element of the tuple-like type midi2::ump::data64::details::sysex7<>.
  using type = std::tuple_element_t<I, decltype(midi2::ump::data64::details::sysex7<Status>::words_)>;
};

/// Defines the C++ types that represent Data 64 Bit messages
namespace midi2::ump::data64 {

using sysex7_in_1 = details::sysex7<mt::data64::sysex7_in_1>;
using sysex7_start = details::sysex7<mt::data64::sysex7_start>;
using sysex7_continue = details::sysex7<mt::data64::sysex7_continue>;
using sysex7_end = details::sysex7<mt::data64::sysex7_end>;

}  // namespace midi2::ump::data64

static_assert(std::tuple_size_v<midi2::ump::data64::sysex7_in_1> ==
              midi2::ump::message_size<midi2::ump::message_type::data64>::value);
static_assert(std::tuple_size_v<midi2::ump::data64::sysex7_start> ==
              midi2::ump::message_size<midi2::ump::message_type::data64>::value);
static_assert(std::tuple_size_v<midi2::ump::data64::sysex7_continue> ==
              midi2::ump::message_size<midi2::ump::message_type::data64>::value);
static_assert(std::tuple_size_v<midi2::ump::data64::sysex7_end> ==
              midi2::ump::message_size<midi2::ump::message_type::data64>::value);

//*        ___               *
//*  _ __ |_  )____ ___ __   *
//* | '  \ / // _\ V / '  \  *
//* |_|_|_/___\__|\_/|_|_|_| *
//*                          *
// F.2.2 Message Type 0x4: MIDI 2.0 Channel Voice Messages
// Table 30 8-Byte UMP Formats for Message Type 0x4: MIDI 2.0 Channel Voice Messages

/// \brief An integral constant which holds the number of bytes of a UMP m2cvm message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::m2cvm> : std::integral_constant<unsigned, 2> {};

/// Defines the C++ types that represent MIDI 2.0 Channel Voice messages
namespace midi2::ump::m2cvm {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

class note_off;
class note_on;
class poly_pressure;
class rpn_per_note_controller;
class nrpn_per_note_controller;
class rpn_controller;
class nrpn_controller;
class rpn_relative_controller;
class nrpn_relative_controller;
class per_note_management;
class control_change;
class program_change;
class channel_pressure;
class pitch_bend;
class per_note_pitch_bend;

}  // end namespace midi2::ump::m2cvm

/// \brief Defines the MIDI 2.0 Note Off Message
class midi2::ump::m2cvm::note_off {
public:
  /// \brief The first word of the MIDI 2.0 Note Off Message
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::note_off;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit range of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit range of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit range of the status field. Always 0x8.
    using status = adt::bit_range<20, 4>;
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using attribute_type = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Note Off Message
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using velocity = adt::bit_range<16, 16>;
    using attribute = adt::bit_range<0, 16>;
  };

  constexpr note_off() noexcept = default;
  constexpr explicit note_off(std::span<std::uint32_t, 2> m) noexcept;
  friend constexpr bool operator==(note_off const &a, note_off const &b) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word0, attribute_type)
  MIDI2_UMP_GETTER_SETTER(word1, velocity)
  MIDI2_UMP_GETTER_SETTER(word1, attribute)

private:
  friend struct ::std::tuple_size<note_off>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, note_off)  // Define tuple_size and tuple_element for m2cvm/note_off
MIDI2_SPAN_CTOR(m2cvm, note_off)  // Define the span constructor for m2cvm/note_off

// 7.4.2 MIDI 2.0 Note On Message
class midi2::ump::m2cvm::note_on {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::note_on;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Note-on=0x9
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using attribute_type = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using velocity = adt::bit_range<16, 16>;
    using attribute = adt::bit_range<0, 16>;
  };

  constexpr note_on() noexcept = default;
  constexpr explicit note_on(std::span<std::uint32_t, 2> m) noexcept;
  friend constexpr bool operator==(note_on const &a, note_on const &b) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word0, attribute_type)
  MIDI2_UMP_GETTER_SETTER(word1, velocity)
  MIDI2_UMP_GETTER_SETTER(word1, attribute)

private:
  friend struct ::std::tuple_size<note_on>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, note_on)  // Define tuple_size and tuple_element for m2cvm/note_on
MIDI2_SPAN_CTOR(m2cvm, note_on)  // Define the span constructor for m2cvm/note_on

// 7.4.3 MIDI 2.0 Poly Pressure Message
class midi2::ump::m2cvm::poly_pressure {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::poly_pressure;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0xA
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using pressure = adt::bit_range<0, 32>;
  };

  constexpr poly_pressure() noexcept = default;
  constexpr explicit poly_pressure(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(poly_pressure const &a, poly_pressure const &b) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word1, pressure)

private:
  friend struct ::std::tuple_size<poly_pressure>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, poly_pressure)  // Define tuple_size and tuple_element for m2cvm/poly_pressure
MIDI2_SPAN_CTOR(m2cvm, poly_pressure)  // Define the span constructor for m2cvm/poly_pressure

// 7.4.4 MIDI 2.0 Registered Per-Note Controller Messages
class midi2::ump::m2cvm::rpn_per_note_controller {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::rpn_per_note;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Registered Per-Note Controller=0x0
    using channel = adt::bit_range<16, 4>;
    using reserved = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using index = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr rpn_per_note_controller() noexcept = default;
  constexpr explicit rpn_per_note_controller(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(rpn_per_note_controller const &, rpn_per_note_controller const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, reserved)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word0, index)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<rpn_per_note_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm,
                rpn_per_note_controller)  // Define tuple_size and tuple_element for m2cvm/rpn_per_note_controller
MIDI2_SPAN_CTOR(m2cvm, rpn_per_note_controller)  // Define the span constructor for m2cvm/rpn_per_note_controller

// 7.4.4 MIDI 2.0 Assignable Per-Note Controller Messages
class midi2::ump::m2cvm::nrpn_per_note_controller {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::nrpn_per_note;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Assignable Per-Note Controller=0x1
    using channel = adt::bit_range<16, 4>;
    using reserved = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using index = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr nrpn_per_note_controller() noexcept = default;
  constexpr explicit nrpn_per_note_controller(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(nrpn_per_note_controller const &,
                                   nrpn_per_note_controller const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, reserved)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word0, index)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<nrpn_per_note_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm,
                nrpn_per_note_controller)  // Define tuple_size and tuple_element for m2cvm/nrpn_per_note_controller
MIDI2_SPAN_CTOR(m2cvm, nrpn_per_note_controller)  // Define the span constructor for m2cvm/nrpn_per_note_controller

// 7.4.7 MIDI 2.0 Registered Controller (RPN) Message
/// "Registered Controllers have specific functions defined by MMA/AMEI specifications. Registered Controllers
/// map and translate directly to MIDI 1.0 Registered Parameter Numbers and use the same
/// definitions as MMA/AMEI approved RPN messages. Registered Controllers are organized in 128 Banks
/// (corresponds to RPN MSB), with 128 controllers per Bank (corresponds to RPN LSB)."
class midi2::ump::m2cvm::rpn_controller {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::rpn;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Registered Control (RPN)=0x2
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using bank = adt::bit_range<8, 7>;  ///< Corresponds to RPN MSB
    using reserved1 = adt::bit_range<7, 1>;
    using index = adt::bit_range<0, 7>;  ///< Corresponds to RPN LSB
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr rpn_controller() noexcept = default;
  constexpr explicit rpn_controller(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(rpn_controller const &, rpn_controller const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, bank)
  MIDI2_UMP_GETTER_SETTER(word0, index)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<rpn_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, rpn_controller)  // Define tuple_size and tuple_element for m2cvm/rpn_controller
MIDI2_SPAN_CTOR(m2cvm, rpn_controller)  // Define the span constructor for m2cvm/rpn_controller

// 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message
class midi2::ump::m2cvm::nrpn_controller {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::nrpn;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Assignable Control (RPN)=0x3
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using bank = adt::bit_range<8, 7>;  ///< Corresponds to NRPN MSB
    using reserved1 = adt::bit_range<7, 1>;
    using index = adt::bit_range<0, 7>;  ///< Corresponds to NRPN LSB
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr nrpn_controller() noexcept = default;
  constexpr explicit nrpn_controller(std::span<std::uint32_t, 2> m) noexcept;
  friend constexpr bool operator==(nrpn_controller const &, nrpn_controller const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, bank)
  MIDI2_UMP_GETTER_SETTER(word0, index)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<nrpn_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, nrpn_controller)  // Define tuple_size and tuple_element for m2cvm/nrpn_controller
MIDI2_SPAN_CTOR(m2cvm, nrpn_controller)  // Define the span constructor for m2cvm/nrpn_controller

// 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message
class midi2::ump::m2cvm::rpn_relative_controller {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::rpn_relative;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Registered Relative Control (RPN)=0x4
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using bank = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<7, 1>;
    using index = adt::bit_range<0, 7>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr rpn_relative_controller() noexcept = default;
  constexpr explicit rpn_relative_controller(std::span<std::uint32_t, 2> m) noexcept;
  friend constexpr bool operator==(rpn_relative_controller const &, rpn_relative_controller const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, bank)
  MIDI2_UMP_GETTER_SETTER(word0, index)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<rpn_relative_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm,
                rpn_relative_controller)  // Define tuple_size and tuple_element for m2cvm/rpn_relative_controller
MIDI2_SPAN_CTOR(m2cvm, rpn_relative_controller)  // Define the span constructor for m2cvm/rpn_relative_controller

// 7.4.8 MIDI 2.0 Assignable Controller (NRPN) Message
class midi2::ump::m2cvm::nrpn_relative_controller {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::nrpn_relative;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Assignable Relative Control (NRPN)=0x5
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using bank = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<7, 1>;
    using index = adt::bit_range<0, 7>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr nrpn_relative_controller() noexcept = default;
  constexpr explicit nrpn_relative_controller(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(nrpn_relative_controller const &,
                                   nrpn_relative_controller const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, reserved0)
  MIDI2_UMP_GETTER_SETTER(word0, bank)
  MIDI2_UMP_GETTER_SETTER(word0, reserved1)
  MIDI2_UMP_GETTER_SETTER(word0, index)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<nrpn_relative_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm,
                nrpn_relative_controller)  // Define tuple_size and tuple_element for m2cvm/nrpn_relative_controller
MIDI2_SPAN_CTOR(m2cvm, nrpn_relative_controller)  // Define the span constructor for m2cvm/nrpn_relative_controller

// 7.4.5 MIDI 2.0 Per-Note Management Message
class midi2::ump::m2cvm::per_note_management {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::per_note_manage;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Per-Note Management=0xF
    using channel = adt::bit_range<16, 4>;
    using reserved = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using option_flags = adt::bit_range<0, 1>;
    using detach = adt::bit_range<1, 1>;          ///< Detach per-note controllers from previously received note(s)
    using set_to_default = adt::bit_range<0, 1>;  ///< Reset (set) per-note controllers to default values
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr per_note_management() noexcept = default;
  constexpr explicit per_note_management(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(per_note_management const &, per_note_management const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, reserved)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word0, option_flags)
  MIDI2_UMP_GETTER_SETTER(word0, detach)
  MIDI2_UMP_GETTER_SETTER(word0, set_to_default)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<per_note_management>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, per_note_management)  // Define tuple_size and tuple_element for m2cvm/per_note_management
MIDI2_SPAN_CTOR(m2cvm, per_note_management)  // Define the span constructor for m2cvm/per_note_management

// 7.4.6 MIDI 2.0 Control Change Message
class midi2::ump::m2cvm::control_change {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::cc;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0xB
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using controller = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr control_change() noexcept = default;
  constexpr explicit control_change(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(control_change const &, control_change const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, controller)
  MIDI2_UMP_GETTER_SETTER(word1, value)

  constexpr auto &controller(control const c) noexcept { return this->controller(std::to_underlying(c)); }

private:
  friend struct ::std::tuple_size<control_change>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, control_change)  // Define tuple_size and tuple_element for m2cvm/control_change
MIDI2_SPAN_CTOR(m2cvm, control_change)  // Define the span constructor for m2cvm/control_change

// 7.4.9 MIDI 2.0 Program Change Message
class midi2::ump::m2cvm::program_change {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::program_change;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0xC
    using channel = adt::bit_range<16, 4>;
    using reserved = adt::bit_range<8, 8>;
    using option_flags = adt::bit_range<1, 7>;  ///< Reserved option flags
    using bank_valid = adt::bit_range<0, 1>;    ///< Bank change is ignored if this bit is zero.
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using program = adt::bit_range<24, 8>;
    using reserved0 = adt::bit_range<16, 8>;
    using reserved1 = adt::bit_range<15, 1>;
    using bank_msb = adt::bit_range<8, 7>;
    using reserved2 = adt::bit_range<7, 1>;
    using bank_lsb = adt::bit_range<0, 7>;
  };

  constexpr program_change() noexcept = default;
  constexpr explicit program_change(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(program_change const &, program_change const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, option_flags)
  MIDI2_UMP_GETTER_SETTER(word0, bank_valid)
  MIDI2_UMP_GETTER_SETTER(word1, program)
  MIDI2_UMP_GETTER_SETTER(word1, bank_msb)
  MIDI2_UMP_GETTER_SETTER(word1, bank_lsb)

private:
  friend struct ::std::tuple_size<program_change>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, program_change)  // Define tuple_size and tuple_element for m2cvm/program_change
MIDI2_SPAN_CTOR(m2cvm, program_change)  // Define the span constructor for m2cvm/program_change

// 7.4.10 MIDI 2.0 Channel Pressure Message
class midi2::ump::m2cvm::channel_pressure {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::channel_pressure;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0xD
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<8, 8>;
    using reserved1 = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr channel_pressure() noexcept = default;
  constexpr explicit channel_pressure(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(channel_pressure const &, channel_pressure const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<channel_pressure>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, channel_pressure)  // Define tuple_size and tuple_element for m2cvm/channel_pressure
MIDI2_SPAN_CTOR(m2cvm, channel_pressure)  // Define the span constructor for m2cvm/channel_pressure

// 7.4.11 MIDI 2.0 Pitch Bend Message
class midi2::ump::m2cvm::pitch_bend {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::pitch_bend;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0xE
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<8, 8>;
    using reserved1 = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr pitch_bend() noexcept = default;
  constexpr explicit pitch_bend(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(pitch_bend const &a, pitch_bend const &b) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<pitch_bend>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, pitch_bend)  // Define tuple_size and tuple_element for m2cvm/pitch_bend
MIDI2_SPAN_CTOR(m2cvm, pitch_bend)  // Define the span constructor for m2cvm/channel_pressure

// 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
class midi2::ump::m2cvm::per_note_pitch_bend {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::m2cvm::pitch_bend_per_note;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0x6
    using channel = adt::bit_range<16, 4>;
    using reserved0 = adt::bit_range<15, 1>;
    using note = adt::bit_range<8, 7>;
    using reserved1 = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value = adt::bit_range<0, 32>;
  };

  constexpr per_note_pitch_bend() noexcept = default;
  constexpr explicit per_note_pitch_bend(std::span<std::uint32_t, 2> const m) noexcept;
  friend constexpr bool operator==(per_note_pitch_bend const &, per_note_pitch_bend const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, note)
  MIDI2_UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<per_note_pitch_bend>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};
MIDI2_UMP_TUPLE(m2cvm, per_note_pitch_bend)  // Define tuple_size and tuple_element for m2cvm/per_note_pitch_bend
MIDI2_SPAN_CTOR(m2cvm, per_note_pitch_bend)  // Define the span constructor for m2cvm/per_note_pitch_bend

//*                       _                       *
//*  _  _ _ __  _ __   __| |_ _ _ ___ __ _ _ __   *
//* | || | '  \| '_ \ (_-<  _| '_/ -_) _` | '  \  *
//*  \_,_|_|_|_| .__/ /__/\__|_| \___\__,_|_|_|_| *
//*            |_|                                *
/// \brief An integral constant which holds the number of bytes of a UMP stream message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::stream> : std::integral_constant<unsigned, 4> {};

/// Defines the C++ types that represent UMP Stream messages
namespace midi2::ump::stream {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

class endpoint_discovery;
class endpoint_info_notification;
class device_identity_notification;
class endpoint_name_notification;
class product_instance_id_notification;
class jr_configuration_request;
class jr_configuration_notification;
class function_block_discovery;
class function_block_info_notification;
class function_block_name_notification;
class start_of_clip;
class end_of_clip;

}  // namespace midi2::ump::stream

// 7.1.1 Endpoint Discovery Message
class midi2::ump::stream::endpoint_discovery {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::endpoint_discovery;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x00
    using version_major = adt::bit_range<8, 8>;
    using version_minor = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using reserved = adt::bit_range<8, 24>;
    using filter = adt::bit_range<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value3 = adt::bit_range<0, 32>;
  };

  constexpr endpoint_discovery() noexcept = default;
  constexpr explicit endpoint_discovery(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(endpoint_discovery const &, endpoint_discovery const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, version_major)
  MIDI2_UMP_GETTER_SETTER(word0, version_minor)
  MIDI2_UMP_GETTER_SETTER(word1, filter)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<endpoint_discovery>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(stream, endpoint_discovery)  // Define tuple_size and tuple_element for stream/endpoint_discovery
MIDI2_SPAN_CTOR(stream, endpoint_discovery)  // Define the span constructor for stream/endpoint_discovery

// 7.1.2 Endpoint Info Notification Message
class midi2::ump::stream::endpoint_info_notification {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::endpoint_info_notification;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x01
    using version_major = adt::bit_range<8, 8>;
    using version_minor = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using static_function_blocks = adt::bit_range<31, 1>;
    using number_function_blocks = adt::bit_range<24, 7>;
    using reserved0 = adt::bit_range<10, 14>;
    using midi2_protocol_capability = adt::bit_range<9, 1>;
    using midi1_protocol_capability = adt::bit_range<8, 1>;
    using reserved1 = adt::bit_range<2, 6>;
    using receive_jr_timestamp_capability = adt::bit_range<1, 1>;
    using transmit_jr_timestamp_capability = adt::bit_range<0, 1>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr endpoint_info_notification() noexcept = default;
  constexpr explicit endpoint_info_notification(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(endpoint_info_notification const &,
                                   endpoint_info_notification const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, version_major)
  MIDI2_UMP_GETTER_SETTER(word0, version_minor)
  MIDI2_UMP_GETTER_SETTER(word1, static_function_blocks)
  MIDI2_UMP_GETTER_SETTER(word1, number_function_blocks)
  MIDI2_UMP_GETTER_SETTER(word1, midi2_protocol_capability)
  MIDI2_UMP_GETTER_SETTER(word1, midi1_protocol_capability)
  MIDI2_UMP_GETTER_SETTER(word1, receive_jr_timestamp_capability)
  MIDI2_UMP_GETTER_SETTER(word1, transmit_jr_timestamp_capability)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<endpoint_info_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/endpoint_info_notification
MIDI2_UMP_TUPLE(stream, endpoint_info_notification)
// Define the span constructor for stream/endpoint_info_notification
MIDI2_SPAN_CTOR(stream, endpoint_info_notification)

// 7.1.3 Device Identity Notification Message
class midi2::ump::stream::device_identity_notification {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::device_identity_notification;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x02
    using reserved0 = adt::bit_range<0, 16>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using reserved0 = adt::bit_range<24, 8>;
    using reserved1 = adt::bit_range<23, 1>;
    using dev_manuf_sysex_id_1 = adt::bit_range<16, 7>;  // device manufacturer sysex id byte 1
    using reserved2 = adt::bit_range<15, 1>;
    using dev_manuf_sysex_id_2 = adt::bit_range<8, 7>;  // device manufacturer sysex id byte 2
    using reserved3 = adt::bit_range<7, 1>;
    using dev_manuf_sysex_id_3 = adt::bit_range<0, 7>;  // device manufacturer sysex id byte 3
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using reserved0 = adt::bit_range<31, 1>;
    using device_family_lsb = adt::bit_range<24, 7>;
    using reserved1 = adt::bit_range<23, 1>;
    using device_family_msb = adt::bit_range<16, 7>;
    using reserved2 = adt::bit_range<15, 1>;
    using device_family_model_lsb = adt::bit_range<8, 7>;
    using reserved3 = adt::bit_range<7, 1>;
    using device_family_model_msb = adt::bit_range<0, 7>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using reserved0 = adt::bit_range<31, 1>;
    using sw_revision_1 = adt::bit_range<24, 7>;  // Software revision level byte 1
    using reserved1 = adt::bit_range<23, 1>;
    using sw_revision_2 = adt::bit_range<16, 7>;  // Software revision level byte 2
    using reserved2 = adt::bit_range<15, 1>;
    using sw_revision_3 = adt::bit_range<8, 7>;  // Software revision level byte 3
    using reserved3 = adt::bit_range<7, 1>;
    using sw_revision_4 = adt::bit_range<0, 7>;  // Software revision level byte 4
  };

  constexpr device_identity_notification() noexcept = default;
  constexpr explicit device_identity_notification(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(device_identity_notification const &,
                                   device_identity_notification const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word1, dev_manuf_sysex_id_1)
  MIDI2_UMP_GETTER_SETTER(word1, dev_manuf_sysex_id_2)
  MIDI2_UMP_GETTER_SETTER(word1, dev_manuf_sysex_id_3)
  MIDI2_UMP_GETTER_SETTER(word2, device_family_lsb)
  MIDI2_UMP_GETTER_SETTER(word2, device_family_msb)
  MIDI2_UMP_GETTER_SETTER(word2, device_family_model_lsb)
  MIDI2_UMP_GETTER_SETTER(word2, device_family_model_msb)
  MIDI2_UMP_GETTER_SETTER(word3, sw_revision_1)
  MIDI2_UMP_GETTER_SETTER(word3, sw_revision_2)
  MIDI2_UMP_GETTER_SETTER(word3, sw_revision_3)
  MIDI2_UMP_GETTER_SETTER(word3, sw_revision_4)

private:
  friend struct ::std::tuple_size<device_identity_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/device_identity_notification
MIDI2_UMP_TUPLE(stream, device_identity_notification)
// Define the span constructor for stream/device_identity_notification
MIDI2_SPAN_CTOR(stream, device_identity_notification)

// 7.1.4 Endpoint Name Notification
class midi2::ump::stream::endpoint_name_notification {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::endpoint_name_notification;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;
    using status = adt::bit_range<16, 10>;  // 0x03
    using name1 = adt::bit_range<8, 8>;
    using name2 = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using name3 = adt::bit_range<24, 8>;
    using name4 = adt::bit_range<16, 8>;
    using name5 = adt::bit_range<8, 8>;
    using name6 = adt::bit_range<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using name7 = adt::bit_range<24, 8>;
    using name8 = adt::bit_range<16, 8>;
    using name9 = adt::bit_range<8, 8>;
    using name10 = adt::bit_range<0, 8>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using name11 = adt::bit_range<24, 8>;
    using name12 = adt::bit_range<16, 8>;
    using name13 = adt::bit_range<8, 8>;
    using name14 = adt::bit_range<0, 8>;
  };

  constexpr endpoint_name_notification() noexcept = default;
  constexpr explicit endpoint_name_notification(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(endpoint_name_notification const &,
                                   endpoint_name_notification const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, name1)
  MIDI2_UMP_GETTER_SETTER(word0, name2)
  MIDI2_UMP_GETTER_SETTER(word1, name3)
  MIDI2_UMP_GETTER_SETTER(word1, name4)
  MIDI2_UMP_GETTER_SETTER(word1, name5)
  MIDI2_UMP_GETTER_SETTER(word1, name6)
  MIDI2_UMP_GETTER_SETTER(word2, name7)
  MIDI2_UMP_GETTER_SETTER(word2, name8)
  MIDI2_UMP_GETTER_SETTER(word2, name9)
  MIDI2_UMP_GETTER_SETTER(word2, name10)
  MIDI2_UMP_GETTER_SETTER(word3, name11)
  MIDI2_UMP_GETTER_SETTER(word3, name12)
  MIDI2_UMP_GETTER_SETTER(word3, name13)
  MIDI2_UMP_GETTER_SETTER(word3, name14)

private:
  friend struct ::std::tuple_size<endpoint_name_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/endpoint_name_notification
MIDI2_UMP_TUPLE(stream, endpoint_name_notification)
// Define the span constructor for stream/endpoint_name_notification
MIDI2_SPAN_CTOR(stream, endpoint_name_notification)

// 7.1.5 Product Instance ID Notification Message
class midi2::ump::stream::product_instance_id_notification {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::product_instance_id_notification;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;
    using status = adt::bit_range<16, 10>;
    using pid1 = adt::bit_range<8, 8>;
    using pid2 = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using pid3 = adt::bit_range<24, 8>;
    using pid4 = adt::bit_range<16, 8>;
    using pid5 = adt::bit_range<8, 8>;
    using pid6 = adt::bit_range<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using pid7 = adt::bit_range<24, 8>;
    using pid8 = adt::bit_range<16, 8>;
    using pid9 = adt::bit_range<8, 8>;
    using pid10 = adt::bit_range<0, 8>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using pid11 = adt::bit_range<24, 8>;
    using pid12 = adt::bit_range<16, 8>;
    using pid13 = adt::bit_range<8, 8>;
    using pid14 = adt::bit_range<0, 8>;
  };

  constexpr product_instance_id_notification() noexcept = default;
  constexpr explicit product_instance_id_notification(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(product_instance_id_notification const &,
                                   product_instance_id_notification const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, pid1)
  MIDI2_UMP_GETTER_SETTER(word0, pid2)
  MIDI2_UMP_GETTER_SETTER(word1, pid3)
  MIDI2_UMP_GETTER_SETTER(word1, pid4)
  MIDI2_UMP_GETTER_SETTER(word1, pid5)
  MIDI2_UMP_GETTER_SETTER(word1, pid6)
  MIDI2_UMP_GETTER_SETTER(word2, pid7)
  MIDI2_UMP_GETTER_SETTER(word2, pid8)
  MIDI2_UMP_GETTER_SETTER(word2, pid9)
  MIDI2_UMP_GETTER_SETTER(word2, pid10)
  MIDI2_UMP_GETTER_SETTER(word3, pid11)
  MIDI2_UMP_GETTER_SETTER(word3, pid12)
  MIDI2_UMP_GETTER_SETTER(word3, pid13)
  MIDI2_UMP_GETTER_SETTER(word3, pid14)

private:
  friend struct ::std::tuple_size<product_instance_id_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/product_instance_id_notification
MIDI2_UMP_TUPLE(stream, product_instance_id_notification)
// Define the span constructor for stream/product_instance_id_notification
MIDI2_SPAN_CTOR(stream, product_instance_id_notification)

// 7.1.6 Selecting a MIDI Protocol and Jitter Reduction Timestamps for a UMP Stream
// 7.1.6.1 Steps to Select Protocol and Jitter Reduction Timestamps
// 7.1.6.2 JR Stream Configuration Request
class midi2::ump::stream::jr_configuration_request {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::jr_configuration_request;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x05
    using protocol = adt::bit_range<8, 8>;
    using reserved0 = adt::bit_range<2, 6>;
    using rxjr = adt::bit_range<1, 1>;
    using txjr = adt::bit_range<0, 1>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value1 = adt::bit_range<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr jr_configuration_request() noexcept = default;
  constexpr explicit jr_configuration_request(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(jr_configuration_request const &,
                                   jr_configuration_request const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, protocol)
  MIDI2_UMP_GETTER_SETTER(word0, rxjr)
  MIDI2_UMP_GETTER_SETTER(word0, txjr)
  MIDI2_UMP_GETTER_SETTER(word1, value1)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<jr_configuration_request>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/jr_configuration_request
MIDI2_UMP_TUPLE(stream, jr_configuration_request)
// Define the span constructor for stream/jr_configuration_request
MIDI2_SPAN_CTOR(stream, jr_configuration_request)

// 7.1.6.3 JR Stream Configuration Notification Message
class midi2::ump::stream::jr_configuration_notification {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::jr_configuration_notification;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x06
    using protocol = adt::bit_range<8, 8>;
    using reserved0 = adt::bit_range<2, 6>;
    using rxjr = adt::bit_range<1, 1>;
    using txjr = adt::bit_range<0, 1>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value1 = adt::bit_range<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr jr_configuration_notification() noexcept = default;
  constexpr explicit jr_configuration_notification(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(jr_configuration_notification const &,
                                   jr_configuration_notification const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, protocol)
  MIDI2_UMP_GETTER_SETTER(word0, rxjr)
  MIDI2_UMP_GETTER_SETTER(word0, txjr)
  MIDI2_UMP_GETTER_SETTER(word1, value1)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<jr_configuration_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/jr_configuration_notification
MIDI2_UMP_TUPLE(stream, jr_configuration_notification)
// Define the span constructor for stream/jr_configuration_notification
MIDI2_SPAN_CTOR(stream, jr_configuration_notification)

// 7.1.7 Function Block Discovery Message
class midi2::ump::stream::function_block_discovery {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::function_block_discovery;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x10
    using block_num = adt::bit_range<8, 8>;
    using filter = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value1 = adt::bit_range<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr function_block_discovery() noexcept = default;
  constexpr explicit function_block_discovery(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(function_block_discovery const &,
                                   function_block_discovery const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, block_num)
  MIDI2_UMP_GETTER_SETTER(word0, filter)
  MIDI2_UMP_GETTER_SETTER(word1, value1)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<function_block_discovery>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/function_block_discovery
MIDI2_UMP_TUPLE(stream, function_block_discovery)
// Define the span constructor for stream/function_block_discovery
MIDI2_SPAN_CTOR(stream, function_block_discovery)

// 7.1.8 Function Block Info Notification
class midi2::ump::stream::function_block_info_notification {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::function_block_info_notification;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x11
    using block_active = adt::bit_range<15, 1>;
    using block_num = adt::bit_range<8, 7>;
    using reserved0 = adt::bit_range<6, 2>;
    using ui_hint = adt::bit_range<4, 2>;
    using midi1 = adt::bit_range<2, 2>;
    using direction = adt::bit_range<0, 2>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using first_group = adt::bit_range<24, 8>;
    using num_spanned = adt::bit_range<16, 8>;
    using ci_message_version = adt::bit_range<8, 8>;
    using max_sys8_streams = adt::bit_range<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr function_block_info_notification() noexcept = default;
  constexpr explicit function_block_info_notification(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(function_block_info_notification const &,
                                   function_block_info_notification const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, block_active)
  MIDI2_UMP_GETTER_SETTER(word0, block_num)
  MIDI2_UMP_GETTER_SETTER(word0, ui_hint)
  MIDI2_UMP_GETTER_SETTER(word0, midi1)
  MIDI2_UMP_GETTER_SETTER(word0, direction)
  MIDI2_UMP_GETTER_SETTER(word1, first_group)
  MIDI2_UMP_GETTER_SETTER(word1, num_spanned)
  MIDI2_UMP_GETTER_SETTER(word1, ci_message_version)
  MIDI2_UMP_GETTER_SETTER(word1, max_sys8_streams)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<function_block_info_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/function_block_info_notification
MIDI2_UMP_TUPLE(stream, function_block_info_notification)
// Define the span constructor for stream/function_block_info_notification
MIDI2_SPAN_CTOR(stream, function_block_info_notification)

// 7.1.9 Function Block Name Notification
class midi2::ump::stream::function_block_name_notification {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::function_block_name_notification;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x12
    using block_num = adt::bit_range<8, 8>;
    using name0 = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using name1 = adt::bit_range<24, 8>;
    using name2 = adt::bit_range<16, 8>;
    using name3 = adt::bit_range<8, 8>;
    using name4 = adt::bit_range<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using name5 = adt::bit_range<24, 8>;
    using name6 = adt::bit_range<16, 8>;
    using name7 = adt::bit_range<8, 8>;
    using name8 = adt::bit_range<0, 8>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using name9 = adt::bit_range<24, 8>;
    using name10 = adt::bit_range<16, 8>;
    using name11 = adt::bit_range<8, 8>;
    using name12 = adt::bit_range<0, 8>;
  };

  constexpr function_block_name_notification() noexcept = default;
  constexpr explicit function_block_name_notification(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(function_block_name_notification const &,
                                   function_block_name_notification const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, block_num)
  MIDI2_UMP_GETTER_SETTER(word0, name0)
  MIDI2_UMP_GETTER_SETTER(word1, name1)
  MIDI2_UMP_GETTER_SETTER(word1, name2)
  MIDI2_UMP_GETTER_SETTER(word1, name3)
  MIDI2_UMP_GETTER_SETTER(word1, name4)
  MIDI2_UMP_GETTER_SETTER(word2, name5)
  MIDI2_UMP_GETTER_SETTER(word2, name6)
  MIDI2_UMP_GETTER_SETTER(word2, name7)
  MIDI2_UMP_GETTER_SETTER(word2, name8)
  MIDI2_UMP_GETTER_SETTER(word3, name9)
  MIDI2_UMP_GETTER_SETTER(word3, name10)
  MIDI2_UMP_GETTER_SETTER(word3, name11)
  MIDI2_UMP_GETTER_SETTER(word3, name12)

private:
  friend struct ::std::tuple_size<function_block_name_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/function_block_name_notification
MIDI2_UMP_TUPLE(stream, function_block_name_notification)
// Define the span constructor for stream/function_block_name_notification
MIDI2_SPAN_CTOR(stream, function_block_name_notification)

// 7.1.10 Start of Clip Message
class midi2::ump::stream::start_of_clip {
public:
  class word0 : public details::word_base {
  public:
    static constexpr auto message = mt::stream::start_of_clip;
    using word_base::word_base;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x20
    using reserved0 = adt::bit_range<0, 16>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value1 = adt::bit_range<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr start_of_clip() noexcept = default;
  constexpr explicit start_of_clip(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(start_of_clip const &, start_of_clip const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word1, value1)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<start_of_clip>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(stream, start_of_clip)  // Define tuple_size and tuple_element for stream/start_of_clip
MIDI2_SPAN_CTOR(stream, start_of_clip)  // Define the span constructor for stream/start_of_clip

// 7.1.11 End of Clip Message
class midi2::ump::stream::end_of_clip {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::stream::end_of_clip;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF.
    using mt = adt::bit_range<28, 4>;
    using format = adt::bit_range<26, 2>;   // 0x00
    using status = adt::bit_range<16, 10>;  // 0x21
    using reserved0 = adt::bit_range<0, 16>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value1 = adt::bit_range<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr end_of_clip() noexcept = default;
  constexpr explicit end_of_clip(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(end_of_clip const &, end_of_clip const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, format)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word1, value1)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<end_of_clip>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(stream, end_of_clip)  // Define tuple_size and tuple_element for stream/end_of_clip
MIDI2_SPAN_CTOR(stream, end_of_clip)  // Define the span constructor for stream/end_of_clip

//*   __ _              _      _         *
//*  / _| |_____ __  __| |__ _| |_ __ _  *
//* |  _| / -_) \ / / _` / _` |  _/ _` | *
//* |_| |_\___/_\_\ \__,_\__,_|\__\__,_| *
//*                                      *

/// \brief An integral constant which holds the number of bytes of a UMP flex_data message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::flex_data> : std::integral_constant<unsigned, 4> {};

/// Defines the C++ types that represent Flex Data messages
namespace midi2::ump::flex_data {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

class set_tempo;
class set_time_signature;
class set_metronome;
class set_key_signature;
class set_chord_name;
class text_common;

}  // namespace midi2::ump::flex_data

// 7.5.3 Set Tempo Message
class midi2::ump::flex_data::set_tempo {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::flex_data::set_tempo;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using form = adt::bit_range<22, 2>;
    using addrs = adt::bit_range<20, 2>;
    using channel = adt::bit_range<16, 4>;
    using status_bank = adt::bit_range<8, 8>;
    using status = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value1 = adt::bit_range<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr set_tempo() noexcept = default;
  constexpr explicit set_tempo(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(set_tempo const &, set_tempo const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER_SETTER(word0, form)
  MIDI2_UMP_GETTER_SETTER(word0, addrs)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, status_bank)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word1, value1)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<set_tempo>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(flex_data, set_tempo)  // Define tuple_size and tuple_element for flex_data/set_tempo
MIDI2_SPAN_CTOR(flex_data, set_tempo)  // Define the span constructor for flex_data/set_tempo

// 7.5.4 Set Time Signature Message
class midi2::ump::flex_data::set_time_signature {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::flex_data::set_time_signature;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using form = adt::bit_range<22, 2>;
    using addrs = adt::bit_range<20, 2>;
    using channel = adt::bit_range<16, 4>;
    using status_bank = adt::bit_range<8, 8>;
    using status = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using numerator = adt::bit_range<24, 8>;
    using denominator = adt::bit_range<16, 8>;
    using number_of_32_notes = adt::bit_range<8, 8>;
    using reserved0 = adt::bit_range<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr set_time_signature() noexcept = default;
  constexpr explicit set_time_signature(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(set_time_signature const &, set_time_signature const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER_SETTER(word0, form)
  MIDI2_UMP_GETTER_SETTER(word0, addrs)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, status_bank)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word1, numerator)
  MIDI2_UMP_GETTER_SETTER(word1, denominator)
  MIDI2_UMP_GETTER_SETTER(word1, number_of_32_notes)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<set_time_signature>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(flex_data, set_time_signature)  // Define tuple_size and tuple_element for flex_data/set_time_signature
MIDI2_SPAN_CTOR(flex_data, set_time_signature)  // Define the span constructor for flex_data/set_time_signature

// 7.5.5 Set Metronome Message
class midi2::ump::flex_data::set_metronome {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::flex_data::set_metronome;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using form = adt::bit_range<22, 2>;
    using addrs = adt::bit_range<20, 2>;
    using channel = adt::bit_range<16, 4>;
    using status_bank = adt::bit_range<8, 8>;
    using status = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using num_clocks_per_primary_click = adt::bit_range<24, 8>;
    using bar_accent_part_1 = adt::bit_range<16, 8>;
    using bar_accent_part_2 = adt::bit_range<8, 8>;
    using bar_accent_part_3 = adt::bit_range<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using num_subdivision_clicks_1 = adt::bit_range<24, 8>;
    using num_subdivision_clicks_2 = adt::bit_range<16, 8>;
    using reserved0 = adt::bit_range<0, 16>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr set_metronome() noexcept = default;
  constexpr explicit set_metronome(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(set_metronome const &, set_metronome const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER_SETTER(word0, form)
  MIDI2_UMP_GETTER_SETTER(word0, addrs)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, status_bank)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word1, num_clocks_per_primary_click)
  MIDI2_UMP_GETTER_SETTER(word1, bar_accent_part_1)
  MIDI2_UMP_GETTER_SETTER(word1, bar_accent_part_2)
  MIDI2_UMP_GETTER_SETTER(word1, bar_accent_part_3)
  MIDI2_UMP_GETTER_SETTER(word2, num_subdivision_clicks_1)
  MIDI2_UMP_GETTER_SETTER(word2, num_subdivision_clicks_2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<set_metronome>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(flex_data, set_metronome)  // Define tuple_size and tuple_element for flex_data/set_metronome
MIDI2_SPAN_CTOR(flex_data, set_metronome)  // Define the span constructor for flex_data/set_metronome

namespace midi2::ump::flex_data {

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
  a = 0x1,
  b = 0x2,
  c = 0x3,
  d = 0x4,
  e = 0x5,
  f = 0x6,
  g = 0x7,
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

}  // end namespace midi2::ump::flex_data

// 7.5.7 Set Key Signature Message
class midi2::ump::flex_data::set_key_signature {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::flex_data::set_key_signature;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using form = adt::bit_range<22, 2>;
    using addrs = adt::bit_range<20, 2>;
    using channel = adt::bit_range<16, 4>;
    using status_bank = adt::bit_range<8, 8>;
    using status = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using sharps_flats = adt::bit_range<28, 4>;
    using tonic_note = adt::bit_range<24, 4>;
    using reserved0 = adt::bit_range<0, 24>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr set_key_signature() noexcept = default;
  constexpr explicit set_key_signature(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(set_key_signature const &, set_key_signature const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER_SETTER(word0, form)
  MIDI2_UMP_GETTER_SETTER(word0, addrs)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, status_bank)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, sharps_flats, midi2::ump::flex_data::sharps_flats)
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, tonic_note, midi2::ump::flex_data::note)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<set_key_signature>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(flex_data, set_key_signature)  // Define tuple_size and tuple_element for flex_data/set_key_signature
MIDI2_SPAN_CTOR(flex_data, set_key_signature)  // Define the span constructor for flex_data/set_key_signature

// 7.5.8 Set Chord Name Message
class midi2::ump::flex_data::set_chord_name {
public:
  /// Defines the fields of the first word of the set-chord-name message.
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::flex_data::set_chord_name;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using form = adt::bit_range<22, 2>;  ///< Always 0
    using addrs = adt::bit_range<20, 2>;
    using channel = adt::bit_range<16, 4>;
    /// Defines the bit position of the status-back field. Always 0.
    using status_bank = adt::bit_range<8, 8>;
    /// Defines the bit position of the status field. Always 6.
    using status = adt::bit_range<0, 8>;
  };
  /// Defines the fields of the second word of the set-chord-name message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using tonic_sharps_flats = adt::bit_range<28, 4>;  // 2's complement
    using chord_tonic = adt::bit_range<24, 4>;
    using chord_type = adt::bit_range<16, 8>;
    using alter_1_type = adt::bit_range<12, 4>;
    using alter_1_degree = adt::bit_range<8, 4>;
    using alter_2_type = adt::bit_range<4, 4>;
    using alter_2_degree = adt::bit_range<0, 4>;
  };
  /// Defines the fields of the third word of the set-chord-name message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using alter_3_type = adt::bit_range<28, 4>;
    using alter_3_degree = adt::bit_range<24, 4>;
    using alter_4_type = adt::bit_range<20, 4>;
    using alter_4_degree = adt::bit_range<16, 4>;
    using reserved = adt::bit_range<0, 16>;  // 0x0000
  };
  /// Defines the fields of the fourth word of the set-chord-name message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using bass_sharps_flats = adt::bit_range<28, 4>;  // 2's complement
    using bass_note = adt::bit_range<24, 4>;
    using bass_chord_type = adt::bit_range<16, 8>;
    using bass_alter_1_type = adt::bit_range<12, 4>;
    using bass_alter_1_degree = adt::bit_range<8, 4>;
    using bass_alter_2_type = adt::bit_range<4, 4>;
    using bass_alter_2_degree = adt::bit_range<0, 4>;
  };

  constexpr set_chord_name() noexcept = default;
  constexpr explicit set_chord_name(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(set_chord_name const &, set_chord_name const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, form)
  MIDI2_UMP_GETTER_SETTER(word0, addrs)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER(word0, status_bank)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, tonic_sharps_flats, midi2::ump::flex_data::sharps_flats)
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, chord_tonic, midi2::ump::flex_data::note)
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, chord_type, midi2::ump::flex_data::chord_type)
  MIDI2_UMP_GETTER_SETTER(word1, alter_1_type)
  MIDI2_UMP_GETTER_SETTER(word1, alter_1_degree)
  MIDI2_UMP_GETTER_SETTER(word1, alter_2_type)
  MIDI2_UMP_GETTER_SETTER(word1, alter_2_degree)
  MIDI2_UMP_GETTER_SETTER(word2, alter_3_type)
  MIDI2_UMP_GETTER_SETTER(word2, alter_3_degree)
  MIDI2_UMP_GETTER_SETTER(word2, alter_4_type)
  MIDI2_UMP_GETTER_SETTER(word2, alter_4_degree)
  MIDI2_UMP_GETTER_SETTER_ENUM(word3, bass_sharps_flats, midi2::ump::flex_data::sharps_flats)
  MIDI2_UMP_GETTER_SETTER_ENUM(word3, bass_note, midi2::ump::flex_data::note)
  MIDI2_UMP_GETTER_SETTER_ENUM(word3, bass_chord_type, midi2::ump::flex_data::chord_type)
  MIDI2_UMP_GETTER_SETTER(word3, bass_alter_1_type)
  MIDI2_UMP_GETTER_SETTER(word3, bass_alter_1_degree)
  MIDI2_UMP_GETTER_SETTER(word3, bass_alter_2_type)
  MIDI2_UMP_GETTER_SETTER(word3, bass_alter_2_degree)

private:
  friend struct ::std::tuple_size<set_chord_name>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(flex_data, set_chord_name)  // Define tuple_size and tuple_element for flex_data/set_chord_name
MIDI2_SPAN_CTOR(flex_data, set_chord_name)  // Define the span constructor for flex_data/set_chord_name

// 7.5.9 Text Messages Common Format
class midi2::ump::flex_data::text_common {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() noexcept { this->set<mt>(std::to_underlying(ump::message_type::flex_data)); }
    [[nodiscard]] constexpr bool check() const noexcept {
      return this->get<mt>() == std::to_underlying(ump::message_type::flex_data);
    }

    /// Defines the bit position of the mt (message-type) field. Always 0xD.
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    using form = adt::bit_range<22, 2>;
    using addrs = adt::bit_range<20, 2>;
    using channel = adt::bit_range<16, 4>;
    using status_bank = adt::bit_range<8, 8>;
    using status = adt::bit_range<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value1 = adt::bit_range<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value3 = adt::bit_range<0, 32>;
  };

  constexpr text_common() noexcept = default;
  constexpr explicit text_common(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(text_common const &, text_common const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER_SETTER(word0, form)
  MIDI2_UMP_GETTER_SETTER(word0, addrs)
  MIDI2_UMP_GETTER_SETTER(word0, channel)
  MIDI2_UMP_GETTER_SETTER(word0, status_bank)
  MIDI2_UMP_GETTER_SETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word1, value1)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<text_common>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(flex_data, text_common)  // Define tuple_size and tuple_element for flex_data/text_common
MIDI2_SPAN_CTOR(flex_data, text_common)  // Define the span constructor for flex_data/text_common

//*     _      _          _ ___ ___  *
//*  __| |__ _| |_ __ _  / |_  | _ ) *
//* / _` / _` |  _/ _` | | |/ // _ \ *
//* \__,_\__,_|\__\__,_| |_/___\___/ *
//*                                  *

/// \brief An integral constant which holds the number of bytes of a UMP data128 message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::data128> : std::integral_constant<unsigned, 4> {};

/// Defines the C++ types that represent Data 128 messages
namespace midi2::ump::data128 {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

namespace details {

// 7.8 System Exclusive 8 (8-Bit) Messages
// SysEx8 in 1 UMP (word 1)
// SysEx8 Start (word 1)
// SysEx8 Continue (word 1)
// SysEx8 End (word 1)
template <ump::mt::data128 Status> class sysex8;

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

}  // end namespace details

class mds_header;
class mds_payload;

}  // namespace midi2::ump::data128

template <midi2::ump::mt::data128 Status> class midi2::ump::data128::details::sysex8 {
public:
  class word0 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = Status;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    using mt = adt::bit_range<28, 4>;  ///< Always 0x05
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;
    using number_of_bytes = adt::bit_range<16, 4>;
    using stream_id = adt::bit_range<8, 8>;
    using data0 = adt::bit_range<0, 8>;
  };
  class word1 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using data1 = adt::bit_range<24, 8>;
    using data2 = adt::bit_range<16, 8>;
    using data3 = adt::bit_range<8, 8>;
    using data4 = adt::bit_range<0, 8>;
  };
  class word2 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using data5 = adt::bit_range<24, 8>;
    using data6 = adt::bit_range<16, 8>;
    using data7 = adt::bit_range<8, 8>;
    using data8 = adt::bit_range<0, 8>;
  };
  class word3 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using data9 = adt::bit_range<24, 8>;
    using data10 = adt::bit_range<16, 8>;
    using data11 = adt::bit_range<8, 8>;
    using data12 = adt::bit_range<0, 8>;
  };

  constexpr sysex8() noexcept = default;
  constexpr explicit sysex8(std::span<std::uint32_t, 4> const m) noexcept : words_{span_to_tuple(m)} {
    assert(get<0>(words_).check());
  }
  friend constexpr bool operator==(sysex8 const &, sysex8 const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER_SETTER(word0, number_of_bytes)
  MIDI2_UMP_GETTER_SETTER(word0, stream_id)
  MIDI2_UMP_GETTER_SETTER(word0, data0)
  MIDI2_UMP_GETTER_SETTER(word1, data1)
  MIDI2_UMP_GETTER_SETTER(word1, data2)
  MIDI2_UMP_GETTER_SETTER(word1, data3)
  MIDI2_UMP_GETTER_SETTER(word1, data4)
  MIDI2_UMP_GETTER_SETTER(word2, data5)
  MIDI2_UMP_GETTER_SETTER(word2, data6)
  MIDI2_UMP_GETTER_SETTER(word2, data7)
  MIDI2_UMP_GETTER_SETTER(word2, data8)
  MIDI2_UMP_GETTER_SETTER(word3, data9)
  MIDI2_UMP_GETTER_SETTER(word3, data10)
  MIDI2_UMP_GETTER_SETTER(word3, data11)
  MIDI2_UMP_GETTER_SETTER(word3, data12)

private:
  friend struct ::std::tuple_size<sysex8>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_{};
};

template <midi2::ump::mt::data128 Status>
struct std::tuple_size<midi2::ump::data128::details::sysex8<Status>> /* NOLINT(cert-dcl58-cpp]*/
    : std::integral_constant<std::size_t,
                             std::tuple_size_v<decltype(midi2::ump::data128::details::sysex8<Status>::words_)>> {};

template <std::size_t I, midi2::ump::mt::data128 Status>
struct std::tuple_element<I, midi2::ump::data128::details::sysex8<Status>> { /* NOLINT(cert-dcl58-cpp] */
  /// The type of Ith element of the tuple-like type midi2::ump::data128::details::sysex8<>.
  using type = std::tuple_element_t<I, decltype(midi2::ump::data128::details::sysex8<Status>::words_)>;
};

namespace midi2::ump::data128 {

/// The message type used for 8-bit wide system exclusive data packed into a single UMP message.
using sysex8_in_1 = details::sysex8<midi2::ump::mt::data128::sysex8_in_1>;
/// The start of a block of 8-bit system exclusive data that is split over zero or more of sysex8_continue messages and
/// completed by a sysex8_end message.
using sysex8_start = details::sysex8<midi2::ump::mt::data128::sysex8_start>;
/// A block of 128 bits of system exclusive data. The can be sent as many times as necessary but the must be preceeded
/// by a sysex8_in_1 message.
using sysex8_continue = details::sysex8<midi2::ump::mt::data128::sysex8_continue>;
/// A message which signals the end of a series of sysex8_start and sysex8_continue messages.
using sysex8_end = details::sysex8<midi2::ump::mt::data128::sysex8_end>;

}  // end namespace midi2::ump::data128

/// \brief The header message for a Mixed Data Set sequence.
///
/// Mixed Data Set messages can carry any data payload, without the 7-bit restriction of the MIDI 1.0
/// Protocol. This mechanism is targeted primarily for use with large data sets, including non-MIDI
/// data.
class midi2::ump::data128::mds_header {
public:
  class word0 : public ump::details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = ump::mt::data128::mixed_data_set_header;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    using mt = adt::bit_range<28, 4>;  ///< Always 0x05
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0x08
    using mds_id = adt::bit_range<16, 4>;
    using bytes_in_chunk = adt::bit_range<0, 16>;
  };
  class word1 : public ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using chunks_in_mds = adt::bit_range<16, 16>;
    using chunk_num = adt::bit_range<0, 16>;
  };
  class word2 : public ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using manufacturer_id = adt::bit_range<16, 16>;
    using device_id = adt::bit_range<0, 16>;
  };
  class word3 : public ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using sub_id_1 = adt::bit_range<16, 16>;
    using sub_id_2 = adt::bit_range<0, 16>;
  };

  constexpr mds_header() noexcept = default;
  constexpr explicit mds_header(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(mds_header const &, mds_header const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, mds_id)
  MIDI2_UMP_GETTER_SETTER(word0, bytes_in_chunk)
  MIDI2_UMP_GETTER_SETTER(word1, chunks_in_mds)
  MIDI2_UMP_GETTER_SETTER(word1, chunk_num)
  MIDI2_UMP_GETTER_SETTER(word2, manufacturer_id)
  MIDI2_UMP_GETTER_SETTER(word2, device_id)
  MIDI2_UMP_GETTER_SETTER(word3, sub_id_1)
  MIDI2_UMP_GETTER_SETTER(word3, sub_id_2)

private:
  friend struct ::std::tuple_size<mds_header>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(data128, mds_header)  // Define tuple_size and tuple_element for data128/mds_header
MIDI2_SPAN_CTOR(data128, mds_header)  // Define the span constructor for data128/mds_header

/// \brief The payload message for a Mixed Data Set (MDS) sequence.
///
/// Mixed Data Set messages can carry any data payload, without the 7-bit restriction of the MIDI 1.0
/// Protocol.
class midi2::ump::data128::mds_payload {
public:
  /// \brief The first 32-bit word of an MDS payload message
  class word0 : public ump::details::word_base {
  public:
    using word_base::word_base;
    static constexpr auto message = midi2::ump::mt::data128::mixed_data_set_payload;
    constexpr word0() noexcept { this->init<mt, status>(message); }
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    using mt = adt::bit_range<28, 4>;  ///< Always 0x05
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    using status = adt::bit_range<20, 4>;  ///< Always 0x09
    using mds_id = adt::bit_range<16, 4>;
    using value0 = adt::bit_range<0, 16>;
  };
  /// \brief The second 32-bit word of an MDS payload message
  class word1 : public ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value1 = adt::bit_range<0, 32>;
  };
  /// \brief The third 32-bit word of an MDS payload message
  class word2 : public ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth 32-bit word of an MDS payload message
  class word3 : public ump::details::word_base {
  public:
    using word_base::word_base;
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    using value3 = adt::bit_range<0, 32>;
  };

  constexpr mds_payload() noexcept = default;
  constexpr explicit mds_payload(std::span<std::uint32_t, 4> m) noexcept;
  friend constexpr bool operator==(mds_payload const &, mds_payload const &) noexcept = default;

  MIDI2_UMP_GETTER(word0, mt)
  MIDI2_UMP_GETTER_SETTER(word0, group)
  MIDI2_UMP_GETTER(word0, status)
  MIDI2_UMP_GETTER_SETTER(word0, mds_id)
  MIDI2_UMP_GETTER_SETTER(word0, value0)
  MIDI2_UMP_GETTER_SETTER(word1, value1)
  MIDI2_UMP_GETTER_SETTER(word2, value2)
  MIDI2_UMP_GETTER_SETTER(word3, value3)

private:
  friend struct ::std::tuple_size<mds_payload>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(data128, mds_payload)  // Define tuple_size and tuple_element for data128/mds_payload
MIDI2_SPAN_CTOR(data128, mds_payload)  // Define the span constructor for data128/mds_payload

/// \brief An integral constant which holds the number of bytes of a UMP reserved32_06 message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved32_06> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of bytes of a UMP reserved32_07 message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved32_07> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of bytes of a UMP reserved64_08 message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved64_08> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of bytes of a UMP reserved64_09 message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved64_09> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of bytes of a UMP reserved64_0a message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved64_0a> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of bytes of a UMP reserved96_0b message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved96_0b> : std::integral_constant<unsigned, 3> {};
/// \brief An integral constant which holds the number of bytes of a UMP reserved96_0c message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved96_0c> : std::integral_constant<unsigned, 3> {};
/// \brief An integral constant which holds the number of bytes of a UMP reserved128_0e message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved128_0e> : std::integral_constant<unsigned, 4> {};

#undef MIDI2_UMP_GETTER
#undef MIDI2_UMP_GETTER_ENUM
#undef MIDI2_UMP_SETTER
#undef MIDI2_UMP_SETTER_ENUM
#undef MIDI2_UMP_GETTER_SETTER
#undef MIDI2_UMP_GETTER_SETTER_ENUM
#undef MIDI2_UMP_TUPLE
#undef MIDI2_SPAN_CTOR

#endif  // MIDI2_UMP_TYPES_HPP
