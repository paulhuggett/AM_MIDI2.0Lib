//===-- UMP Types -------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMP_TYPES_HPP
#define MIDI2_UMP_TYPES_HPP

#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <tuple>
#include <type_traits>

#include "midi2/utils.hpp"

namespace midi2 {
namespace types {

template <typename T>
concept bitfield_type = requires(T) {
  requires std::unsigned_integral<typename T::index::value_type>;
  requires std::unsigned_integral<typename T::bits::value_type>;
};

template <typename T, typename Function, unsigned Index = 0>
  requires(std::tuple_size_v<T> >= 0 && Index <= std::tuple_size_v<T>)
constexpr void apply(T const &value, Function function) {
  if constexpr (Index >= std::tuple_size_v<T>) {
    return;
  } else {
    function(get<Index>(value));
    apply<T, Function, Index + 1>(value, std::move(function));
  }
}

namespace details {

constexpr auto status_to_message_type(status) { return ump_message_type::m1cvm; }
constexpr auto status_to_message_type(system_crt) { return ump_message_type::system; }
constexpr auto status_to_message_type(ump_utility) { return ump_message_type::utility; }
//  X(m1cvm, 0x02)
constexpr auto status_to_message_type(data64) { return ump_message_type::data64; }
constexpr auto status_to_message_type(m2cvm) { return ump_message_type::m2cvm; }
constexpr auto status_to_message_type(data128) { return ump_message_type::data128; }
constexpr auto status_to_message_type(flex_data) { return ump_message_type::flex_data; }
constexpr auto status_to_message_type(ump_stream) { return ump_message_type::ump_stream; }

template <typename T> constexpr auto status_to_ump_status(T status) { return to_underlying(status); }
template <> constexpr auto status_to_ump_status(status status) {
  auto const s = to_underlying(status);
  return static_cast<std::uint8_t>(s < to_underlying(status::sysex_start) ? s >> 4 : s);
}

template <unsigned Index, unsigned Bits> struct bitfield {
  using index = std::integral_constant<unsigned, Index>;
  using bits = std::integral_constant<unsigned, Bits>;
};

class word_base {
public:
  using value_type = std::uint32_t;

  constexpr word_base() noexcept = default;
  constexpr explicit word_base(std::uint32_t const v) noexcept : value_{v} {}

  [[nodiscard]] constexpr auto word() const noexcept { return value_; }
  friend constexpr bool operator==(word_base const &a, word_base const &b) noexcept = default;

  template <bitfield_type BitRange> constexpr auto &set(unsigned v) noexcept {
    constexpr auto index = typename BitRange::index();
    constexpr auto bits = typename BitRange::bits();
    constexpr auto mask = max_value<value_type, bits>();
    assert(v <= mask);
    value_ = static_cast<value_type>(value_ & ~(mask << index)) |
             static_cast<value_type>((static_cast<value_type>(v) & mask) << index);
    return *this;
  }

  template <bitfield_type BitRange> [[nodiscard]] constexpr small_type<BitRange::bits::value> get() const noexcept {
    constexpr auto index = typename BitRange::index();
    constexpr auto bits = typename BitRange::bits();
    constexpr auto mask = max_value<value_type, bits>();
    return (value_ >> index) & mask;
  }

protected:
  template <bitfield_type MtField, bitfield_type StatusField> constexpr void init(auto const status) noexcept {
    this->set<MtField>(to_underlying(status_to_message_type(status)));
    this->set<StatusField>(status_to_ump_status(status));
  }

private:
  ///\returns The maximum value that can be held in \p Bits bits of type \p T.
  template <std::unsigned_integral T, unsigned Bits>
    requires(Bits <= sizeof(T) * 8 && Bits <= 64U)
  static constexpr T max_value() noexcept {
    if constexpr (Bits == 8U) {
      return std::numeric_limits<std::uint8_t>::max();
    } else if constexpr (Bits == 16U) {
      return std::numeric_limits<std::uint16_t>::max();
    } else if constexpr (Bits == 32U) {
      return std::numeric_limits<std::uint32_t>::max();
    } else if constexpr (Bits == 64U) {
      return std::numeric_limits<std::uint64_t>::max();
    } else {
      return static_cast<T>((T{1} << Bits) - 1U);
    }
  }

  value_type value_ = 0;
};

}  // end namespace details

}  // end namespace types
}  // end namespace midi2

#define UMP_GETTER(word, field)                                         \
  constexpr auto field() const noexcept {                               \
    return std::get<word>(words_).template get<typename word::field>(); \
  }
#define UMP_SETTER(word, field)                                                  \
  constexpr auto &field(small_type<word::field::bits::value> const v) noexcept { \
    std::get<word>(words_).template set<typename word::field>(v);                \
    return *this;                                                                \
  }
#define UMP_GETTER_SETTER(word, field) \
  UMP_GETTER(word, field)              \
  UMP_SETTER(word, field)

namespace midi2 {
template <ump_message_type> struct message_size;  // not defined
}  // end namespace midi2
//*       _   _ _ _ _         *
//*  _  _| |_(_) (_) |_ _  _  *
//* | || |  _| | | |  _| || | *
//*  \_,_|\__|_|_|_|\__|\_, | *
//*                     |__/  *
namespace midi2 {
template <> struct message_size<ump_message_type::utility> : std::integral_constant<unsigned, 1> {};
}  // end namespace midi2

namespace midi2::types::utility {

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
class song_position_pointer;

}  // end namespace midi2::types::utility

// F.1.1 Message Type 0x0: Utility
// Table 26 4-Byte UMP Formats for Message Type 0x0: Utility

// 7.2.1 NOOP
class midi2::types::utility::noop {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() noexcept { this->init<mt, status>(ump_utility::noop); }

    using mt = details::bitfield<28, 4>;
    using status = details::bitfield<20, 4>;
  };

  constexpr noop() noexcept = default;
  constexpr explicit noop(std::span<std::uint32_t, 1> m) noexcept : words_{m[0]} {}
  friend constexpr bool operator==(noop const &, noop const &) noexcept = default;

private:
  friend struct ::std::tuple_size<noop>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};

/// Provides access to the number of words in a noop message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::utility::noop>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::utility::noop::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of the noop message.
template <std::size_t I> struct std::tuple_element<I, midi2::types::utility::noop> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::utility::noop::words_)>::type;
};

static_assert(std::tuple_size_v<midi2::types::utility::noop> ==
              midi2::message_size<midi2::ump_message_type::utility>::value);

// 7.2.2.1 JR Clock Message
class midi2::types::utility::jr_clock {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() noexcept { this->init<mt, status>(ump_utility::jr_clock); }

    using mt = details::bitfield<28, 4>;  // 0x0
    using reserved0 = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  // 0b0001
    using reserved1 = details::bitfield<16, 4>;
    using sender_clock_time = details::bitfield<0, 16>;
  };

  constexpr jr_clock() noexcept = default;
  constexpr explicit jr_clock(std::span<std::uint32_t, 1> m) noexcept : words_{m[0]} {}
  friend constexpr bool operator==(jr_clock const &, jr_clock const &) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, sender_clock_time)

private:
  friend struct ::std::tuple_size<jr_clock>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};

/// Provides access to the number of words in a jr_clock message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::utility::jr_clock>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::utility::jr_clock::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of the jr_clock message.
template <std::size_t I> struct std::tuple_element<I, midi2::types::utility::jr_clock> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::utility::jr_clock::words_)>::type;
};

static_assert(std::tuple_size_v<midi2::types::utility::jr_clock> ==
              midi2::message_size<midi2::ump_message_type::utility>::value);

// 7.2.2.2 JR Timestamp Message
class midi2::types::utility::jr_timestamp {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() noexcept { this->init<mt, status>(ump_utility::jr_ts); }

    using mt = details::bitfield<28, 4>;  // 0x0
    using reserved0 = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  // 0b0010
    using reserved1 = details::bitfield<16, 4>;
    using timestamp = details::bitfield<0, 16>;
  };

  constexpr jr_timestamp() noexcept = default;
  constexpr explicit jr_timestamp(std::span<std::uint32_t, 1> m) noexcept : words_{m[0]} {}
  friend constexpr bool operator==(jr_timestamp const &, jr_timestamp const &) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, timestamp)

private:
  friend struct ::std::tuple_size<jr_timestamp>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};

/// Provides access to the number of words in a jr_timestamp message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::utility::jr_timestamp>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::utility::jr_timestamp::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of the jr_timestamp message.
template <std::size_t I> struct std::tuple_element<I, midi2::types::utility::jr_timestamp> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::utility::jr_timestamp::words_)>;
};

static_assert(std::tuple_size_v<midi2::types::utility::jr_timestamp> ==
              midi2::message_size<midi2::ump_message_type::utility>::value);

// 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
class midi2::types::utility::delta_clockstamp_tpqn {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() noexcept { this->init<mt, status>(ump_utility::delta_clock_tick); }

    using mt = details::bitfield<28, 4>;  // 0x0
    using reserved0 = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  // 0b0011
    using reserved1 = details::bitfield<16, 4>;
    using ticks_pqn = details::bitfield<0, 16>;
  };

  constexpr delta_clockstamp_tpqn() noexcept = default;
  constexpr explicit delta_clockstamp_tpqn(std::uint32_t const w0) noexcept : words_{w0} {}
  friend constexpr bool operator==(delta_clockstamp_tpqn const &, delta_clockstamp_tpqn const &) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, ticks_pqn)

private:
  friend struct ::std::tuple_size<delta_clockstamp_tpqn>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};

/// Provides access to the number of words in a delta_clockstamp_tpqn message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::utility::delta_clockstamp_tpqn>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::utility::delta_clockstamp_tpqn::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of the delta_clockstamp_tpqn message.
template <std::size_t I>
struct std::tuple_element<I, midi2::types::utility::delta_clockstamp_tpqn> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::utility::delta_clockstamp_tpqn::words_)>;
};

static_assert(std::tuple_size_v<midi2::types::utility::delta_clockstamp_tpqn> ==
              midi2::message_size<midi2::ump_message_type::utility>::value);

// 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
class midi2::types::utility::delta_clockstamp {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() noexcept { this->init<mt, status>(ump_utility::delta_clock_since); }

    using mt = details::bitfield<28, 4>;  // 0x0
    using reserved0 = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  // 0b0100
    using ticks_per_quarter_note = details::bitfield<0, 20>;
  };

  constexpr delta_clockstamp() noexcept = default;
  constexpr explicit delta_clockstamp(std::uint32_t const w0) noexcept : words_{w0} {}
  friend constexpr bool operator==(delta_clockstamp const &, delta_clockstamp const &) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, ticks_per_quarter_note)

private:
  friend struct ::std::tuple_size<delta_clockstamp>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};

/// Provides access to the number of words in a delta_clockstamp message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::utility::delta_clockstamp>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::utility::delta_clockstamp::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of the delta_clockstamp message.
template <std::size_t I>
struct std::tuple_element<I, midi2::types::utility::delta_clockstamp> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element_t<I, decltype(midi2::types::utility::delta_clockstamp::words_)>;
};

static_assert(std::tuple_size_v<midi2::types::utility::delta_clockstamp> ==
              midi2::message_size<midi2::ump_message_type::utility>::value);

//*             _              *
//*  ____  _ __| |_ ___ _ __   *
//* (_-< || (_-<  _/ -_) '  \  *
//* /__/\_, /__/\__\___|_|_|_| *
//*     |__/                   *
// 7.6 System Common and System Real Time Messages

template <> struct midi2::message_size<midi2::ump_message_type::system> : std::integral_constant<unsigned, 1> {};

namespace midi2::types::system {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

class midi_time_code;
class song_position_pointer;

}  // namespace midi2::types::system

class midi2::types::system::midi_time_code {
public:
  class word0 : public midi2::types::details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::timing_code); }

    using mt = details::bitfield<28U, 4U>;  ///< Always 0x1
    using group = details::bitfield<24U, 4U>;
    using status = details::bitfield<16U, 8U>;  ///< Always 0xF1
    using reserved0 = details::bitfield<15, 1>;
    using time_code = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using reserved2 = details::bitfield<0, 7>;
  };

  constexpr midi_time_code() noexcept = default;
  constexpr explicit midi_time_code(std::uint32_t const w0) noexcept : words_{w0} {}
  friend constexpr bool operator==(midi_time_code const &, midi_time_code const &) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, time_code)

private:
  friend struct ::std::tuple_size<midi_time_code>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};

/// Provides access to the number of words in a midi_time_code message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::system::midi_time_code>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::system::midi_time_code::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of the midi_time_code message.
template <std::size_t I> struct std::tuple_element<I, midi2::types::system::midi_time_code> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element_t<I, decltype(midi2::types::system::midi_time_code::words_)>;
};

static_assert(std::tuple_size_v<midi2::types::system::midi_time_code> ==
              midi2::message_size<midi2::ump_message_type::system>::value);

class midi2::types::system::song_position_pointer {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::spp); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  ///< Always 0xF2
    using reserved0 = details::bitfield<15, 1>;
    using position_lsb = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using position_msb = details::bitfield<0, 7>;
  };

  constexpr song_position_pointer() noexcept = default;
  constexpr explicit song_position_pointer(std::uint32_t const w0) noexcept : words_{w0} {}
  friend constexpr bool operator==(song_position_pointer const &, song_position_pointer const &) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, position_lsb)
  UMP_GETTER_SETTER(word0, position_msb)

private:
  friend struct ::std::tuple_size<song_position_pointer>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};

/// Provides access to the number of words in a song_position_pointer message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::system::song_position_pointer>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::system::song_position_pointer::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of the song_position_pointer message.
template <std::size_t I>
struct std::tuple_element<I, midi2::types::system::song_position_pointer> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element_t<I, decltype(midi2::types::system::midi_time_code::words_)>;
};

static_assert(std::tuple_size_v<midi2::types::system::song_position_pointer> ==
              midi2::message_size<midi2::ump_message_type::system>::value);

namespace midi2 {
namespace types::system {

struct song_select {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::song_select); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  ///< Always 0xF3
    using reserved0 = details::bitfield<15, 1>;
    using song = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using reserved2 = details::bitfield<0, 7>;
  };

  constexpr song_select() = default;
  constexpr explicit song_select(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(song_select const &, song_select const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, song)

  std::tuple<word0> words_;
};

struct tune_request {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::tune_request); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  ///< Always 0xF6
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };

  constexpr tune_request() = default;
  constexpr explicit tune_request(std::uint32_t const w0_) : words_{w0_} {}
  friend constexpr bool operator==(tune_request const &, tune_request const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)

  std::tuple<word0> words_;
};

struct timing_clock {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::timing_clock); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  ///< Always 0xF8
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };

  constexpr timing_clock() = default;
  constexpr explicit timing_clock(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(timing_clock const &, timing_clock const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)

  std::tuple<word0> words_;
};

struct sequence_start {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::sequence_start); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  ///< Always 0xFA
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };

  constexpr sequence_start() = default;
  constexpr explicit sequence_start(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(sequence_start const &, sequence_start const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)

  std::tuple<word0> words_;
};

struct sequence_continue {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::sequence_continue); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  ///< Always 0xFB
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };

  constexpr sequence_continue() = default;
  constexpr explicit sequence_continue(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(sequence_continue const &, sequence_continue const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)

  std::tuple<word0> words_;
};

struct sequence_stop {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::sequence_stop); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  ///< Always 0xFC
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };

  constexpr sequence_stop() = default;
  constexpr explicit sequence_stop(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(sequence_stop const &, sequence_stop const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)

  std::tuple<word0> words_;
};

struct active_sensing {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::active_sensing); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  ///< Always 0xFE
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };

  constexpr active_sensing() = default;
  constexpr explicit active_sensing(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(active_sensing const &, active_sensing const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)

  std::tuple<word0> words_;
};

struct reset {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::system_reset); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  ///< Always 0xFF
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };

  constexpr reset() = default;
  constexpr explicit reset(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(reset const &, reset const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)

  std::tuple<word0> words_;
};

}  // end namespace types::system

//*        _                 *
//*  _ __ / |  ____ ___ __   *
//* | '  \| | / _\ V / '  \  *
//* |_|_|_|_| \__|\_/|_|_|_| *
//*                          *
// F.1.3 Mess Type 0x2: MIDI 1.0 Channel Voice Messages
// Table 28 4-Byte UMP Formats for Message Type 0x2: MIDI 1.0 Channel Voice
// Messages
template <> struct message_size<ump_message_type::m1cvm> : std::integral_constant<unsigned, 1> {};

namespace types::m1cvm {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

class note_on;
class note_off;
struct poly_pressure;
struct control_change;
struct program_change;
struct channel_pressure;
struct pitch_bend;

}  // end namespace types::m1cvm
}  // end namespace midi2

// 7.3.2 MIDI 1.0 Note On Message
class midi2::types::m1cvm::note_on {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::status::note_on); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0x09.
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using velocity = details::bitfield<0, 7>;
  };

  constexpr note_on() noexcept = default;
  constexpr explicit note_on(std::uint32_t const w0) noexcept : words_{w0} {}
  friend constexpr bool operator==(note_on const &, note_on const &) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word0, velocity)

private:
  friend struct ::std::tuple_size<note_on>;
  friend struct ::std::tuple_element<0, note_on>;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};

template <>
struct std::tuple_size<midi2::types::m1cvm::note_on>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::m1cvm::note_on::words_)>::value> {};

template <std::size_t I> struct std::tuple_element<I, midi2::types::m1cvm::note_on> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::m1cvm::note_on::words_)>::type;
};

static_assert(std::tuple_size_v<midi2::types::m1cvm::note_on> ==
              midi2::message_size<midi2::ump_message_type::system>::value);

// 7.3.1 MIDI 1.0 Note Off Message
class midi2::types::m1cvm::note_off {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::status::note_off); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0x08.
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using velocity = details::bitfield<0, 7>;
  };

  constexpr note_off() = default;
  constexpr explicit note_off(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(note_off const &, note_off const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word0, velocity)

private:
  friend struct ::std::tuple_size<note_off>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0> words_;
};

template <>
struct std::tuple_size<midi2::types::m1cvm::note_off>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::m1cvm::note_off::words_)>::value> {};

template <std::size_t I> struct std::tuple_element<I, midi2::types::m1cvm::note_off> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::m1cvm::note_off::words_)>::type;
};

static_assert(std::tuple_size_v<midi2::types::m1cvm::note_on> ==
              midi2::message_size<midi2::ump_message_type::system>::value);

namespace midi2 {
namespace types::m1cvm {

// 7.3.3 MIDI 1.0 Poly Pressure Message
struct poly_pressure {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::status::poly_pressure); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0x08.
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using pressure = details::bitfield<0, 7>;
  };

  constexpr poly_pressure() = default;
  constexpr explicit poly_pressure(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(poly_pressure const &, poly_pressure const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word0, pressure)

  std::tuple<word0> words_;
};

// 7.3.4 MIDI 1.0 Control Change Message
struct control_change {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::status::cc); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  /// Always 0x0B.
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using controller = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using value = details::bitfield<0, 7>;
  };

  constexpr control_change() = default;
  constexpr explicit control_change(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(control_change const &, control_change const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, controller)
  UMP_GETTER_SETTER(word0, value)

  std::tuple<word0> words_;
};

// 7.3.5 MIDI 1.0 Program Change Message
struct program_change {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::status::program_change); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0x0C.
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using program = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<0, 8>;
  };

  constexpr program_change() = default;
  constexpr explicit program_change(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(program_change const &, program_change const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, program)

  std::tuple<word0> words_;
};

// 7.3.6 MIDI 1.0 Channel Pressure Message
struct channel_pressure {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::status::channel_pressure); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0x08.
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using data = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<0, 8>;
  };

  constexpr channel_pressure() = default;
  constexpr explicit channel_pressure(std::uint32_t const w0_) : words_{w0_} {}
  friend constexpr bool operator==(channel_pressure const &, channel_pressure const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, data)

  std::tuple<word0> words_;
};

// 7.3.7 MIDI 1.0 Pitch Bend Message
struct pitch_bend {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::status::pitch_bend); }

    using mt = details::bitfield<28, 4>;  // 0x2
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  // 0b1000..0b1110
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using lsb_data = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using msb_data = details::bitfield<0, 7>;
  };
  constexpr pitch_bend() = default;
  constexpr explicit pitch_bend(std::uint32_t const w0) : words_{w0} {}
  friend constexpr bool operator==(pitch_bend const &, pitch_bend const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, lsb_data)
  UMP_GETTER_SETTER(word0, msb_data)

  std::tuple<word0> words_;
};

}  // end namespace types::m1cvm

//*     _      _         __ _ _   *
//*  __| |__ _| |_ __ _ / /| | |  *
//* / _` / _` |  _/ _` / _ \_  _| *
//* \__,_\__,_|\__\__,_\___/ |_|  *
//*                               *

template <> struct message_size<ump_message_type::data64> : std::integral_constant<unsigned, 2> {};

namespace types::data64 {

// 7.7 System Exclusive (7-Bit) Messages
namespace details {

template <midi2::data64 Status> class sysex7 {
public:
  friend std::tuple_size<sysex7>;

  class word0 : public ::midi2::types::details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(Status); }
    using mt = midi2::types::details::bitfield<28, 4>;
    using group = midi2::types::details::bitfield<24, 4>;
    using status = midi2::types::details::bitfield<20, 4>;
    using number_of_bytes = midi2::types::details::bitfield<16, 4>;
    using reserved0 = midi2::types::details::bitfield<15, 1>;
    using data0 = midi2::types::details::bitfield<8, 7>;
    using reserved1 = midi2::types::details::bitfield<7, 1>;
    using data1 = midi2::types::details::bitfield<0, 7>;
  };
  class word1 : public midi2::types::details::word_base {
  public:
    using word_base::word_base;

    using reserved0 = midi2::types::details::bitfield<31, 1>;
    using data2 = midi2::types::details::bitfield<24, 7>;
    using reserved1 = midi2::types::details::bitfield<23, 1>;
    using data3 = midi2::types::details::bitfield<16, 7>;
    using reserved2 = midi2::types::details::bitfield<15, 1>;
    using data4 = midi2::types::details::bitfield<8, 7>;
    using reserved3 = midi2::types::details::bitfield<7, 1>;
    using data5 = midi2::types::details::bitfield<0, 7>;
  };

  constexpr sysex7() = default;
  constexpr explicit sysex7(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(sysex7 const &, sysex7 const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, number_of_bytes)
  UMP_GETTER_SETTER(word0, data0)
  UMP_GETTER_SETTER(word0, data1)
  UMP_GETTER_SETTER(word1, data2)
  UMP_GETTER_SETTER(word1, data3)
  UMP_GETTER_SETTER(word1, data4)
  UMP_GETTER_SETTER(word1, data5)

  template <std::size_t I> constexpr auto const &get() const noexcept { return std::get<I>(words_); }
  template <std::size_t I> constexpr auto &get() noexcept { return std::get<I>(words_); }

private:
  std::tuple<word0, word1> words_{};
};

template <std::size_t I, midi2::data64 Status> auto const &get(sysex7<Status> const &t) noexcept {
  return t.template get<I>();
}
template <std::size_t I, midi2::data64 Status> auto &get(sysex7<Status> &t) noexcept {
  return t.template get<I>();
}

}  // end namespace details

using sysex7_in_1 = details::sysex7<midi2::data64::sysex7_in_1>;
using sysex7_start = details::sysex7<midi2::data64::sysex7_start>;
using sysex7_continue = details::sysex7<midi2::data64::sysex7_continue>;
using sysex7_end = details::sysex7<midi2::data64::sysex7_end>;

}  // end namespace types::data64

//*        ___               *
//*  _ __ |_  )____ ___ __   *
//* | '  \ / // _\ V / '  \  *
//* |_|_|_/___\__|\_/|_|_|_| *
//*                          *
// F.2.2 Message Type 0x4: MIDI 2.0 Channel Voice Messages
// Table 30 8-Byte UMP Formats for Message Type 0x4: MIDI 2.0 Channel Voice Messages

}  // end namespace midi2

template <> struct midi2::message_size<midi2::ump_message_type::m2cvm> : std::integral_constant<unsigned, 2> {};

namespace midi2::types::m2cvm {

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
struct nrpn_per_note_controller;
struct rpn_controller;
struct nrpn_controller;
struct rpn_relative_controller;
struct nrpn_relative_controller;
struct per_note_management;
struct control_change;
struct program_change;
struct channel_pressure;
struct pitch_bend;
struct per_note_pitch_bend;

}  // end namespace midi2::types::m2cvm

// 7.4.1 MIDI 2.0 Note Off Message
class midi2::types::m2cvm::note_off {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::note_off); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Note-off=0x8, note-on=0x9
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using attribute_type = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;

    using velocity = details::bitfield<16, 16>;
    using attribute = details::bitfield<0, 16>;
  };

  constexpr note_off() noexcept = default;
  constexpr explicit note_off(std::span<std::uint32_t, 2> m) noexcept : words_{m[0], m[1]} {}
  friend constexpr bool operator==(note_off const &a, note_off const &b) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word0, attribute_type)
  UMP_GETTER_SETTER(word1, velocity)
  UMP_GETTER_SETTER(word1, attribute)

private:
  friend struct ::std::tuple_size<note_off>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};

/// Provides access to the number of words in a MIDI 2 note-off message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::m2cvm::note_off>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::m2cvm::note_off::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of MIDI 2 note-off message.
template <std::size_t I> struct std::tuple_element<I, midi2::types::m2cvm::note_off> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::m2cvm::note_off::words_)>::type;
};

static_assert(std::tuple_size_v<midi2::types::m2cvm::note_off> ==
              midi2::message_size<midi2::ump_message_type::m2cvm>::value);

// 7.4.2 MIDI 2.0 Note On Message
class midi2::types::m2cvm::note_on {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::note_on); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Note-on=0x9
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using attribute_type = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;

    using velocity = details::bitfield<16, 16>;
    using attribute = details::bitfield<0, 16>;
  };

  constexpr note_on() noexcept = default;
  constexpr explicit note_on(std::span<std::uint32_t, 2> m) noexcept : words_{m[0], m[1]} {}
  friend constexpr bool operator==(note_on const &a, note_on const &b) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word0, attribute_type)
  UMP_GETTER_SETTER(word1, velocity)
  UMP_GETTER_SETTER(word1, attribute)

private:
  friend struct ::std::tuple_size<note_on>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};

/// Provides access to the number of words in a MIDI 2 note-on message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::m2cvm::note_on>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::m2cvm::note_on::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of MIDI 2 note-on message.
template <std::size_t I> struct std::tuple_element<I, midi2::types::m2cvm::note_on> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::m2cvm::note_on::words_)>::type;
};

static_assert(std::tuple_size_v<midi2::types::m2cvm::note_on> ==
              midi2::message_size<midi2::ump_message_type::m2cvm>::value);

// 7.4.3 MIDI 2.0 Poly Pressure Message
class midi2::types::m2cvm::poly_pressure {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::poly_pressure); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0xA
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using pressure = details::bitfield<0, 32>;
  };

  constexpr poly_pressure() noexcept = default;
  constexpr explicit poly_pressure(std::span<std::uint32_t, 2> m) noexcept : words_{m[0], m[1]} {}
  friend constexpr bool operator==(poly_pressure const &a, poly_pressure const &b) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word1, pressure)

private:
  friend struct ::std::tuple_size<poly_pressure>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};

/// Provides access to the number of words in a MIDI 2 poly-pressure message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::m2cvm::poly_pressure>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::poly_pressure::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of MIDI 2 poly-pressure message.
template <std::size_t I> struct std::tuple_element<I, midi2::types::m2cvm::poly_pressure> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::m2cvm::poly_pressure::words_)>::type;
};

static_assert(std::tuple_size_v<midi2::types::m2cvm::poly_pressure> ==
              midi2::message_size<midi2::ump_message_type::m2cvm>::value);

// 7.4.4 MIDI 2.0 Registered Per-Note Controller Messages
class midi2::types::m2cvm::rpn_per_note_controller {
public:
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::m2cvm::rpn_pernote); }
    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Registered Per-Note Controller=0x0
    using channel = details::bitfield<16, 4>;
    using reserved = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using index = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr rpn_per_note_controller() noexcept = default;
  constexpr explicit rpn_per_note_controller(std::span<std::uint32_t, 2> m) noexcept : words_{m[0], m[1]} {}
  friend constexpr bool operator==(rpn_per_note_controller const &a,
                                   rpn_per_note_controller const &b) noexcept = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, reserved)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word0, index)
  UMP_GETTER_SETTER(word1, value)

private:
  friend struct ::std::tuple_size<rpn_per_note_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const &get(T const &) noexcept;
  template <std::size_t I, typename T> friend auto &get(T &) noexcept;

  std::tuple<word0, word1> words_;
};

/// Provides access to the number of words in a MIDI 2 poly-pressure message as a compile-time constant expression.
template <>
struct std::tuple_size<midi2::types::m2cvm::rpn_per_note_controller>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::rpn_per_note_controller::words_)>::value> {};

/// Provides compile-time indexed access to the types of the elements of MIDI 2 poly-pressure message.
template <std::size_t I>
struct std::tuple_element<I, midi2::types::m2cvm::rpn_per_note_controller> {  // NOLINT(cert-dcl58-cpp]
  using type = std::tuple_element<I, decltype(midi2::types::m2cvm::rpn_per_note_controller::words_)>::type;
};

static_assert(std::tuple_size_v<midi2::types::m2cvm::rpn_per_note_controller> ==
              midi2::message_size<midi2::ump_message_type::m2cvm>::value);

namespace midi2 {
namespace types::m2cvm {

// 7.4.4 MIDI 2.0 Assignable Per-Note Controller Messages
struct nrpn_per_note_controller {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::nrpn_pernote); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Assignable Per-Note Controller=0x1
    using channel = details::bitfield<16, 4>;
    using reserved = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using index = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr nrpn_per_note_controller() = default;
  constexpr explicit nrpn_per_note_controller(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(nrpn_per_note_controller const &a, nrpn_per_note_controller const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, reserved)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word0, index)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

// 7.4.7 MIDI 2.0 Registered Controller (RPN) Message
/// "Registered Controllers have specific functions defined by MMA/AMEI specifications. Registered Controllers
/// map and translate directly to MIDI 1.0 Registered Parameter Numbers and use the same
/// definitions as MMA/AMEI approved RPN messages. Registered Controllers are organized in 128 Banks
/// (corresponds to RPN MSB), with 128 controllers per Bank (corresponds to RPN LSB).
struct rpn_controller {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::rpn); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Registered Control (RPN)=0x2
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using bank = details::bitfield<8, 7>;  ///< Corresponds to RPN MSB
    using reserved1 = details::bitfield<7, 1>;
    using index = details::bitfield<0, 7>;  ///< Corresponds to RPN LSB
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr rpn_controller() = default;
  constexpr explicit rpn_controller(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(rpn_controller const &a, rpn_controller const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, bank)
  UMP_GETTER_SETTER(word0, index)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

// 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message
struct nrpn_controller {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::nrpn); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Assignable Control (RPN)=0x3
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using bank = details::bitfield<8, 7>;  ///< Corresponds to NRPN MSB
    using reserved1 = details::bitfield<7, 1>;
    using index = details::bitfield<0, 7>;  ///< Corresponds to NRPN LSB
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr nrpn_controller() = default;
  constexpr explicit nrpn_controller(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(nrpn_controller const &a, nrpn_controller const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, bank)
  UMP_GETTER_SETTER(word0, index)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

// 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message
struct rpn_relative_controller {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::rpn_relative); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Registered Relative Control (RPN)=0x4
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using bank = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using index = details::bitfield<0, 7>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr rpn_relative_controller() = default;
  constexpr explicit rpn_relative_controller(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(rpn_relative_controller const &a, rpn_relative_controller const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, bank)
  UMP_GETTER_SETTER(word0, index)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

// 7.4.8 MIDI 2.0 Assignable Controller (NRPN) Message
struct nrpn_relative_controller {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::m2cvm::nrpn_relative); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Assignable Relative Control (NRPN)=0x5
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using bank = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<7, 1>;
    using index = details::bitfield<0, 7>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr nrpn_relative_controller() = default;
  constexpr explicit nrpn_relative_controller(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(nrpn_relative_controller const &a, nrpn_relative_controller const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, reserved0)
  UMP_GETTER_SETTER(word0, bank)
  UMP_GETTER_SETTER(word0, reserved1)
  UMP_GETTER_SETTER(word0, index)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

// 7.4.5 MIDI 2.0 Per-Note Management Message
struct per_note_management {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::m2cvm::pernote_manage); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Per-Note Management=0xF
    using channel = details::bitfield<16, 4>;
    using reserved = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using option_flags = details::bitfield<0, 1>;
    using detach = details::bitfield<1, 1>;          ///< Detach per-note controllers from previously received note(s)
    using set_to_default = details::bitfield<0, 1>;  ///< Reset (set) per-note controllers to default values
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr per_note_management() = default;
  constexpr explicit per_note_management(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(per_note_management const &a, per_note_management const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, reserved)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word0, option_flags)
  UMP_GETTER_SETTER(word0, detach)
  UMP_GETTER_SETTER(word0, set_to_default)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

// 7.4.6 MIDI 2.0 Control Change Message
struct control_change {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::cc); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0xB
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using controller = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr control_change() = default;
  constexpr explicit control_change(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(control_change const &a, control_change const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, controller)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

// 7.4.9 MIDI 2.0 Program Change Message
struct program_change {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::program_change); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0xC
    using channel = details::bitfield<16, 4>;
    using reserved = details::bitfield<8, 8>;
    using option_flags = details::bitfield<1, 7>;  ///< Reserved option flags
    using bank_valid = details::bitfield<0, 1>;    ///< Bank change is ignored if this bit is zero.
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;

    using program = details::bitfield<24, 8>;
    using reserved0 = details::bitfield<16, 8>;
    using reserved1 = details::bitfield<15, 1>;
    using bank_msb = details::bitfield<8, 7>;
    using reserved2 = details::bitfield<7, 1>;
    using bank_lsb = details::bitfield<0, 7>;
  };

  constexpr program_change() = default;
  constexpr explicit program_change(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(program_change const &a, program_change const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, option_flags)
  UMP_GETTER_SETTER(word0, bank_valid)
  UMP_GETTER_SETTER(word1, program)
  UMP_GETTER_SETTER(word1, bank_msb)
  UMP_GETTER_SETTER(word1, bank_lsb)

  std::tuple<word0, word1> words_;
};

// 7.4.10 MIDI 2.0 Channel Pressure Message
struct channel_pressure {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::channel_pressure); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0xD
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr channel_pressure() = default;
  constexpr explicit channel_pressure(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(channel_pressure const &a, channel_pressure const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

// 7.4.11 MIDI 2.0 Pitch Bend Message
struct pitch_bend {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::pitch_bend); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0xE
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr pitch_bend() = default;
  constexpr explicit pitch_bend(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(pitch_bend const &a, pitch_bend const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

// 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
struct per_note_pitch_bend {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::m2cvm::pitch_bend_pernote); }

    using mt = details::bitfield<28, 4>;  ///< Always 0x4
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  ///< Always 0x6
    using channel = details::bitfield<16, 4>;
    using reserved0 = details::bitfield<15, 1>;
    using note = details::bitfield<8, 7>;
    using reserved1 = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value = details::bitfield<0, 32>;
  };

  constexpr per_note_pitch_bend() = default;
  constexpr explicit per_note_pitch_bend(std::span<std::uint32_t, 2> m) : words_{m[0], m[1]} {}
  friend constexpr bool operator==(per_note_pitch_bend const &a, per_note_pitch_bend const &b) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, note)
  UMP_GETTER_SETTER(word1, value)

  std::tuple<word0, word1> words_;
};

}  // end namespace types::m2cvm

//*                       _                       *
//*  _  _ _ __  _ __   __| |_ _ _ ___ __ _ _ __   *
//* | || | '  \| '_ \ (_-<  _| '_/ -_) _` | '  \  *
//*  \_,_|_|_|_| .__/ /__/\__|_| \___\__,_|_|_|_| *
//*            |_|                                *
template <> struct message_size<ump_message_type::ump_stream> : std::integral_constant<unsigned, 4> {};
namespace types::ump_stream {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

// 7.1.1 Endpoint Discovery Message
struct endpoint_discovery {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::endpoint_discovery); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x00
    using version_major = details::bitfield<8, 8>;
    using version_minor = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using reserved = details::bitfield<8, 24>;
    using filter = details::bitfield<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr endpoint_discovery() = default;
  constexpr explicit endpoint_discovery(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(endpoint_discovery const &lhs, endpoint_discovery const &rhs) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, version_major)
  UMP_GETTER_SETTER(word0, version_minor)
  UMP_GETTER_SETTER(word1, filter)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.2 Endpoint Info Notification Message
struct endpoint_info_notification {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::endpoint_info_notification); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x01
    using version_major = details::bitfield<8, 8>;
    using version_minor = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using static_function_blocks = details::bitfield<31, 1>;
    using number_function_blocks = details::bitfield<24, 7>;
    using reserved0 = details::bitfield<10, 14>;
    using midi2_protocol_capability = details::bitfield<9, 1>;
    using midi1_protocol_capability = details::bitfield<8, 1>;
    using reserved1 = details::bitfield<2, 6>;
    using receive_jr_timestamp_capability = details::bitfield<1, 1>;
    using transmit_jr_timestamp_capability = details::bitfield<0, 1>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr endpoint_info_notification() = default;
  constexpr explicit endpoint_info_notification(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(endpoint_info_notification const &, endpoint_info_notification const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, version_major)
  UMP_GETTER_SETTER(word0, version_minor)
  UMP_GETTER_SETTER(word1, static_function_blocks)
  UMP_GETTER_SETTER(word1, number_function_blocks)
  UMP_GETTER_SETTER(word1, midi2_protocol_capability)
  UMP_GETTER_SETTER(word1, midi1_protocol_capability)
  UMP_GETTER_SETTER(word1, receive_jr_timestamp_capability)
  UMP_GETTER_SETTER(word1, transmit_jr_timestamp_capability)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.3 Device Identity Notification Message
struct device_identity_notification {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::device_identity_notification); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x02
    using reserved0 = details::bitfield<0, 16>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using reserved0 = details::bitfield<24, 8>;
    using reserved1 = details::bitfield<23, 1>;
    using dev_manuf_sysex_id_1 = details::bitfield<16, 7>;  // device manufacturer sysex id byte 1
    using reserved2 = details::bitfield<15, 1>;
    using dev_manuf_sysex_id_2 = details::bitfield<8, 7>;  // device manufacturer sysex id byte 2
    using reserved3 = details::bitfield<7, 1>;
    using dev_manuf_sysex_id_3 = details::bitfield<0, 7>;  // device manufacturer sysex id byte 3
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using reserved0 = details::bitfield<31, 1>;
    using device_family_lsb = details::bitfield<24, 7>;
    using reserved1 = details::bitfield<23, 1>;
    using device_family_msb = details::bitfield<16, 7>;
    using reserved2 = details::bitfield<15, 1>;
    using device_family_model_lsb = details::bitfield<8, 7>;
    using reserved3 = details::bitfield<7, 1>;
    using device_family_model_msb = details::bitfield<0, 7>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using reserved0 = details::bitfield<31, 1>;
    using sw_revision_1 = details::bitfield<24, 7>;  // Software revision level byte 1
    using reserved1 = details::bitfield<23, 1>;
    using sw_revision_2 = details::bitfield<16, 7>;  // Software revision level byte 2
    using reserved2 = details::bitfield<15, 1>;
    using sw_revision_3 = details::bitfield<8, 7>;  // Software revision level byte 3
    using reserved3 = details::bitfield<7, 1>;
    using sw_revision_4 = details::bitfield<0, 7>;  // Software revision level byte 4
  };

  constexpr device_identity_notification() = default;
  constexpr explicit device_identity_notification(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(device_identity_notification const &,
                                   device_identity_notification const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word1, dev_manuf_sysex_id_1)
  UMP_GETTER_SETTER(word1, dev_manuf_sysex_id_2)
  UMP_GETTER_SETTER(word1, dev_manuf_sysex_id_3)
  UMP_GETTER_SETTER(word2, device_family_lsb)
  UMP_GETTER_SETTER(word2, device_family_msb)
  UMP_GETTER_SETTER(word2, device_family_model_lsb)
  UMP_GETTER_SETTER(word2, device_family_model_msb)
  UMP_GETTER_SETTER(word3, sw_revision_1)
  UMP_GETTER_SETTER(word3, sw_revision_2)
  UMP_GETTER_SETTER(word3, sw_revision_3)
  UMP_GETTER_SETTER(word3, sw_revision_4)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.4 Endpoint Name Notification
struct endpoint_name_notification {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::endpoint_name_notification); }

    using mt = details::bitfield<28, 4>;  // 0x0F
    using format = details::bitfield<26, 2>;
    using status = details::bitfield<16, 10>;  // 0x03
    using name1 = details::bitfield<8, 8>;
    using name2 = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using name3 = details::bitfield<24, 8>;
    using name4 = details::bitfield<16, 8>;
    using name5 = details::bitfield<8, 8>;
    using name6 = details::bitfield<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using name7 = details::bitfield<24, 8>;
    using name8 = details::bitfield<16, 8>;
    using name9 = details::bitfield<8, 8>;
    using name10 = details::bitfield<0, 8>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using name11 = details::bitfield<24, 8>;
    using name12 = details::bitfield<16, 8>;
    using name13 = details::bitfield<8, 8>;
    using name14 = details::bitfield<0, 8>;
  };

  constexpr endpoint_name_notification() = default;
  constexpr explicit endpoint_name_notification(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(endpoint_name_notification const &, endpoint_name_notification const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, name1)
  UMP_GETTER_SETTER(word0, name2)
  UMP_GETTER_SETTER(word1, name3)
  UMP_GETTER_SETTER(word1, name4)
  UMP_GETTER_SETTER(word1, name5)
  UMP_GETTER_SETTER(word1, name6)
  UMP_GETTER_SETTER(word2, name7)
  UMP_GETTER_SETTER(word2, name8)
  UMP_GETTER_SETTER(word2, name9)
  UMP_GETTER_SETTER(word2, name10)
  UMP_GETTER_SETTER(word3, name11)
  UMP_GETTER_SETTER(word3, name12)
  UMP_GETTER_SETTER(word3, name13)
  UMP_GETTER_SETTER(word3, name14)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.5 Product Instance Id Notification Message
struct product_instance_id_notification {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::product_instance_id_notification); }

    using mt = details::bitfield<28, 4>;
    using format = details::bitfield<26, 2>;
    using status = details::bitfield<16, 10>;
    using pid1 = details::bitfield<8, 8>;
    using pid2 = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using pid3 = details::bitfield<24, 8>;
    using pid4 = details::bitfield<16, 8>;
    using pid5 = details::bitfield<8, 8>;
    using pid6 = details::bitfield<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using pid7 = details::bitfield<24, 8>;
    using pid8 = details::bitfield<16, 8>;
    using pid9 = details::bitfield<8, 8>;
    using pid10 = details::bitfield<0, 8>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using pid11 = details::bitfield<24, 8>;
    using pid12 = details::bitfield<16, 8>;
    using pid13 = details::bitfield<8, 8>;
    using pid14 = details::bitfield<0, 8>;
  };

  constexpr product_instance_id_notification() = default;
  constexpr explicit product_instance_id_notification(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(product_instance_id_notification const &,
                                   product_instance_id_notification const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, pid1)
  UMP_GETTER_SETTER(word0, pid2)
  UMP_GETTER_SETTER(word1, pid3)
  UMP_GETTER_SETTER(word1, pid4)
  UMP_GETTER_SETTER(word1, pid5)
  UMP_GETTER_SETTER(word1, pid6)
  UMP_GETTER_SETTER(word2, pid7)
  UMP_GETTER_SETTER(word2, pid8)
  UMP_GETTER_SETTER(word2, pid9)
  UMP_GETTER_SETTER(word2, pid10)
  UMP_GETTER_SETTER(word3, pid11)
  UMP_GETTER_SETTER(word3, pid12)
  UMP_GETTER_SETTER(word3, pid13)
  UMP_GETTER_SETTER(word3, pid14)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.6 Selecting a MIDI Protocol and Jitter Reduction Timestamps for a UMP Stream
// 7.1.6.1 Steps to Select Protocol and Jitter Reduction Timestamps
// 7.1.6.2 JR Stream Configuration Request
struct jr_configuration_request {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::jr_configuration_request); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x05
    using protocol = details::bitfield<8, 8>;
    using reserved0 = details::bitfield<2, 6>;
    using rxjr = details::bitfield<1, 1>;
    using txjr = details::bitfield<0, 1>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value1 = details::bitfield<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr jr_configuration_request() = default;
  constexpr explicit jr_configuration_request(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(jr_configuration_request const &, jr_configuration_request const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, protocol)
  UMP_GETTER_SETTER(word0, rxjr)
  UMP_GETTER_SETTER(word0, txjr)
  UMP_GETTER_SETTER(word1, value1)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.6.3 JR Stream Configuration Notification Message
struct jr_configuration_notification {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::jr_configuration_notification); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x06
    using protocol = details::bitfield<8, 8>;
    using reserved0 = details::bitfield<2, 6>;
    using rxjr = details::bitfield<1, 1>;
    using txjr = details::bitfield<0, 1>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value1 = details::bitfield<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr jr_configuration_notification() = default;
  constexpr explicit jr_configuration_notification(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(jr_configuration_notification const &,
                                   jr_configuration_notification const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, protocol)
  UMP_GETTER_SETTER(word0, rxjr)
  UMP_GETTER_SETTER(word0, txjr)
  UMP_GETTER_SETTER(word1, value1)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.7 Function Block Discovery Message
struct function_block_discovery {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::function_block_discovery); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x10
    using block_num = details::bitfield<8, 8>;
    using filter = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value1 = details::bitfield<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr function_block_discovery() = default;
  constexpr explicit function_block_discovery(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(function_block_discovery const &, function_block_discovery const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, block_num)
  UMP_GETTER_SETTER(word0, filter)
  UMP_GETTER_SETTER(word1, value1)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.8 Function Block Info Notification
struct function_block_info_notification {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::function_block_info_notification); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x11
    using block_active = details::bitfield<15, 1>;
    using block_num = details::bitfield<8, 7>;
    using reserved0 = details::bitfield<6, 2>;
    using ui_hint = details::bitfield<4, 2>;
    using midi1 = details::bitfield<2, 2>;
    using direction = details::bitfield<0, 2>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using first_group = details::bitfield<24, 8>;
    using num_spanned = details::bitfield<16, 8>;
    using ci_message_version = details::bitfield<8, 8>;
    using max_sys8_streams = details::bitfield<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr function_block_info_notification() = default;
  constexpr explicit function_block_info_notification(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(function_block_info_notification const &,
                                   function_block_info_notification const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, block_active)
  UMP_GETTER_SETTER(word0, block_num)
  UMP_GETTER_SETTER(word0, ui_hint)
  UMP_GETTER_SETTER(word0, midi1)
  UMP_GETTER_SETTER(word0, direction)
  UMP_GETTER_SETTER(word1, first_group)
  UMP_GETTER_SETTER(word1, num_spanned)
  UMP_GETTER_SETTER(word1, ci_message_version)
  UMP_GETTER_SETTER(word1, max_sys8_streams)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.9 Function Block Name Notification
struct function_block_name_notification {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::function_block_name_notification); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x12
    using block_num = details::bitfield<8, 8>;
    using name0 = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using name1 = details::bitfield<24, 8>;
    using name2 = details::bitfield<16, 8>;
    using name3 = details::bitfield<8, 8>;
    using name4 = details::bitfield<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using name5 = details::bitfield<24, 8>;
    using name6 = details::bitfield<16, 8>;
    using name7 = details::bitfield<8, 8>;
    using name8 = details::bitfield<0, 8>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using name9 = details::bitfield<24, 8>;
    using name10 = details::bitfield<16, 8>;
    using name11 = details::bitfield<8, 8>;
    using name12 = details::bitfield<0, 8>;
  };

  constexpr function_block_name_notification() = default;
  constexpr explicit function_block_name_notification(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(function_block_name_notification const &,
                                   function_block_name_notification const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, block_num)
  UMP_GETTER_SETTER(word0, name0)
  UMP_GETTER_SETTER(word1, name1)
  UMP_GETTER_SETTER(word1, name2)
  UMP_GETTER_SETTER(word1, name3)
  UMP_GETTER_SETTER(word1, name4)
  UMP_GETTER_SETTER(word2, name5)
  UMP_GETTER_SETTER(word2, name6)
  UMP_GETTER_SETTER(word2, name7)
  UMP_GETTER_SETTER(word2, name8)
  UMP_GETTER_SETTER(word3, name9)
  UMP_GETTER_SETTER(word3, name10)
  UMP_GETTER_SETTER(word3, name11)
  UMP_GETTER_SETTER(word3, name12)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.10 Start of Clip Message
struct start_of_clip {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::start_of_clip); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x20
    using reserved0 = details::bitfield<0, 16>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value1 = details::bitfield<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr start_of_clip() = default;
  constexpr explicit start_of_clip(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(start_of_clip const &, start_of_clip const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word1, value1)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.1.11 End of Clip Message
struct end_of_clip {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::ump_stream::end_of_clip); }

    using mt = details::bitfield<28, 4>;       // 0x0F
    using format = details::bitfield<26, 2>;   // 0x00
    using status = details::bitfield<16, 10>;  // 0x21
    using reserved0 = details::bitfield<0, 16>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value1 = details::bitfield<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr end_of_clip() = default;
  constexpr explicit end_of_clip(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(end_of_clip const &, end_of_clip const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, format)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word1, value1)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

};  // end namespace types::ump_stream

//*   __ _              _      _         *
//*  / _| |_____ __  __| |__ _| |_ __ _  *
//* |  _| / -_) \ / / _` / _` |  _/ _` | *
//* |_| |_\___/_\_\ \__,_\__,_|\__\__,_| *
//*                                      *
template <> struct message_size<ump_message_type::flex_data> : std::integral_constant<unsigned, 4> {};

namespace types::flex_data {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

// 7.5.3 Set Tempo Message
struct set_tempo {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::flex_data::set_tempo); }

    using mt = details::bitfield<28, 4>;  // 0x0D
    using group = details::bitfield<24, 4>;
    using form = details::bitfield<22, 2>;
    using addrs = details::bitfield<20, 2>;
    using channel = details::bitfield<16, 4>;
    using status_bank = details::bitfield<8, 8>;
    using status = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value1 = details::bitfield<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr set_tempo() = default;
  constexpr explicit set_tempo(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(set_tempo const &, set_tempo const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER_SETTER(word0, form)
  UMP_GETTER_SETTER(word0, addrs)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, status_bank)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word1, value1)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.5.4 Set Time Signature Message
struct set_time_signature {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::flex_data::set_time_signature); }

    using mt = details::bitfield<28, 4>;  // 0x0D
    using group = details::bitfield<24, 4>;
    using form = details::bitfield<22, 2>;
    using addrs = details::bitfield<20, 2>;
    using channel = details::bitfield<16, 4>;
    using status_bank = details::bitfield<8, 8>;
    using status = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;

    using numerator = details::bitfield<24, 8>;
    using denominator = details::bitfield<16, 8>;
    using number_of_32_notes = details::bitfield<8, 8>;
    using reserved0 = details::bitfield<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr set_time_signature() = default;
  constexpr explicit set_time_signature(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(set_time_signature const &, set_time_signature const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER_SETTER(word0, form)
  UMP_GETTER_SETTER(word0, addrs)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, status_bank)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word1, numerator)
  UMP_GETTER_SETTER(word1, denominator)
  UMP_GETTER_SETTER(word1, number_of_32_notes)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.5.5 Set Metronome Message

struct set_metronome {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::flex_data::set_metronome); }

    using mt = details::bitfield<28, 4>;  // 0x0D
    using group = details::bitfield<24, 4>;
    using form = details::bitfield<22, 2>;
    using addrs = details::bitfield<20, 2>;
    using channel = details::bitfield<16, 4>;
    using status_bank = details::bitfield<8, 8>;
    using status = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;

    using num_clocks_per_primary_click = details::bitfield<24, 8>;
    using bar_accent_part_1 = details::bitfield<16, 8>;
    using bar_accent_part_2 = details::bitfield<8, 8>;
    using bar_accent_part_3 = details::bitfield<0, 8>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;

    using num_subdivision_clicks_1 = details::bitfield<24, 8>;
    using num_subdivision_clicks_2 = details::bitfield<16, 8>;
    using reserved0 = details::bitfield<0, 16>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;

    using value3 = details::bitfield<0, 32>;
  };

  constexpr set_metronome() = default;
  constexpr explicit set_metronome(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(set_metronome const &, set_metronome const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER_SETTER(word0, form)
  UMP_GETTER_SETTER(word0, addrs)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, status_bank)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word1, num_clocks_per_primary_click)
  UMP_GETTER_SETTER(word1, bar_accent_part_1)
  UMP_GETTER_SETTER(word1, bar_accent_part_2)
  UMP_GETTER_SETTER(word1, bar_accent_part_3)
  UMP_GETTER_SETTER(word2, num_subdivision_clicks_1)
  UMP_GETTER_SETTER(word2, num_subdivision_clicks_2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.5.7 Set Key Signature Message
struct set_key_signature {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::flex_data::set_key_signature); }

    using mt = details::bitfield<28, 4>;  // 0x0D
    using group = details::bitfield<24, 4>;
    using form = details::bitfield<22, 2>;
    using addrs = details::bitfield<20, 2>;
    using channel = details::bitfield<16, 4>;
    using status_bank = details::bitfield<8, 8>;
    using status = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using sharps_flats = details::bitfield<28, 4>;
    using tonic_note = details::bitfield<24, 4>;
    using reserved0 = details::bitfield<0, 24>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr set_key_signature() = default;
  constexpr explicit set_key_signature(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(set_key_signature const &, set_key_signature const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER_SETTER(word0, form)
  UMP_GETTER_SETTER(word0, addrs)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, status_bank)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word1, sharps_flats)
  UMP_GETTER_SETTER(word1, tonic_note)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.5.8 Set Chord Name Message
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

struct set_chord_name {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(midi2::flex_data::set_chord_name); }

    using mt = details::bitfield<28, 4>;  // 0x0D
    using group = details::bitfield<24, 4>;
    using form = details::bitfield<22, 2>;
    using addrs = details::bitfield<20, 2>;
    using channel = details::bitfield<16, 4>;
    using status_bank = details::bitfield<8, 8>;
    using status = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;

    using tonic_sharps_flats = details::bitfield<28, 4>;  // 2's compl
    using chord_tonic = details::bitfield<24, 4>;
    using chord_type = details::bitfield<16, 8>;
    using alter_1_type = details::bitfield<12, 4>;
    using alter_1_degree = details::bitfield<8, 4>;
    using alter_2_type = details::bitfield<4, 4>;
    using alter_2_degree = details::bitfield<0, 4>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;

    using alter_3_type = details::bitfield<28, 4>;
    using alter_3_degree = details::bitfield<24, 4>;
    using alter_4_type = details::bitfield<20, 4>;
    using alter_4_degree = details::bitfield<16, 4>;
    using reserved = details::bitfield<0, 16>;  // 0x0000
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;

    using bass_sharps_flats = details::bitfield<28, 4>;  // 2's compl
    using bass_note = details::bitfield<24, 4>;
    using bass_chord_type = details::bitfield<16, 8>;
    using bass_alter_1_type = details::bitfield<12, 4>;
    using bass_alter_1_degree = details::bitfield<8, 4>;
    using bass_alter_2_type = details::bitfield<4, 4>;
    using bass_alter_2_degree = details::bitfield<0, 4>;
  };

  constexpr set_chord_name() = default;
  constexpr explicit set_chord_name(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(set_chord_name const &, set_chord_name const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER_SETTER(word0, form)
  UMP_GETTER_SETTER(word0, addrs)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, status_bank)
  UMP_GETTER_SETTER(word0, status)
  UMP_GETTER_SETTER(word1, tonic_sharps_flats)
  UMP_GETTER_SETTER(word1, chord_tonic)
  UMP_GETTER_SETTER(word1, chord_type)
  UMP_GETTER_SETTER(word1, alter_1_type)
  UMP_GETTER_SETTER(word1, alter_1_degree)
  UMP_GETTER_SETTER(word1, alter_2_type)
  UMP_GETTER_SETTER(word1, alter_2_degree)
  UMP_GETTER_SETTER(word2, alter_3_type)
  UMP_GETTER_SETTER(word2, alter_3_degree)
  UMP_GETTER_SETTER(word2, alter_4_type)
  UMP_GETTER_SETTER(word2, alter_4_degree)
  UMP_GETTER_SETTER(word3, bass_sharps_flats)
  UMP_GETTER_SETTER(word3, bass_note)
  UMP_GETTER_SETTER(word3, bass_chord_type)
  UMP_GETTER_SETTER(word3, bass_alter_1_type)
  UMP_GETTER_SETTER(word3, bass_alter_1_degree)
  UMP_GETTER_SETTER(word3, bass_alter_2_type)
  UMP_GETTER_SETTER(word3, bass_alter_2_degree)

  std::tuple<word0, word1, word2, word3> words_;
};

// 7.5.9 Text Messages Common Format
struct text_common {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->set<mt>(to_underlying(ump_message_type::flex_data)); }

    using mt = details::bitfield<28, 4>;  // 0x0D
    using group = details::bitfield<24, 4>;
    using form = details::bitfield<22, 2>;
    using addrs = details::bitfield<20, 2>;
    using channel = details::bitfield<16, 4>;
    using status_bank = details::bitfield<8, 8>;
    using status = details::bitfield<0, 8>;
  };
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    using value1 = details::bitfield<0, 32>;
  };
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    using value2 = details::bitfield<0, 32>;
  };
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    using value3 = details::bitfield<0, 32>;
  };

  constexpr text_common() = default;
  constexpr explicit text_common(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(text_common const &, text_common const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER_SETTER(word0, form)
  UMP_GETTER_SETTER(word0, addrs)
  UMP_GETTER_SETTER(word0, channel)
  UMP_GETTER_SETTER(word0, status_bank)
  UMP_GETTER_SETTER(word0, status)
  UMP_GETTER_SETTER(word1, value1)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

}  // end namespace types::flex_data

//*     _      _          _ ___ ___  *
//*  __| |__ _| |_ __ _  / |_  | _ ) *
//* / _` / _` |  _/ _` | | |/ // _ \ *
//* \__,_\__,_|\__\__,_| |_/___\___/ *
//*                                  *

template <> struct message_size<ump_message_type::data128> : std::integral_constant<unsigned, 4> {};

namespace types::data128 {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

// 7.8 System Exclusive 8 (8-Bit) Messages

// SysEx8 in 1 UMP (word 1)
// SysEx8 Start (word 1)
// SysEx8 Continue (word 1)
// SysEx8 End (word 1)
namespace details {

template <std::size_t I, typename T> auto const &get(T const &t) noexcept {
  return get<I>(t.words_);
}
template <std::size_t I, typename T> auto &get(T &t) noexcept {
  return get<I>(t.words_);
}

template <midi2::data128 Status> struct sysex8 {
  class word0 : public types::details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(Status); }

    using mt = types::details::bitfield<28, 4>;  ///< Always 0x05
    using group = types::details::bitfield<24, 4>;
    using status = types::details::bitfield<20, 4>;
    using number_of_bytes = types::details::bitfield<16, 4>;
    using stream_id = types::details::bitfield<8, 8>;
    using data0 = types::details::bitfield<0, 8>;
  };
  class word1 : public types::details::word_base {
  public:
    using word_base::word_base;

    using data1 = types::details::bitfield<24, 8>;
    using data2 = types::details::bitfield<16, 8>;
    using data3 = types::details::bitfield<8, 8>;
    using data4 = types::details::bitfield<0, 8>;
  };
  class word2 : public types::details::word_base {
  public:
    using word_base::word_base;

    using data5 = types::details::bitfield<24, 8>;
    using data6 = types::details::bitfield<16, 8>;
    using data7 = types::details::bitfield<8, 8>;
    using data8 = types::details::bitfield<0, 8>;
  };
  class word3 : public types::details::word_base {
  public:
    using word_base::word_base;

    using data9 = types::details::bitfield<24, 8>;
    using data10 = types::details::bitfield<16, 8>;
    using data11 = types::details::bitfield<8, 8>;
    using data12 = types::details::bitfield<0, 8>;
  };

  constexpr sysex8() = default;
  constexpr explicit sysex8(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(sysex8 const &, sysex8 const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER_SETTER(word0, number_of_bytes)
  UMP_GETTER_SETTER(word0, stream_id)
  UMP_GETTER_SETTER(word0, data0)
  UMP_GETTER_SETTER(word1, data1)
  UMP_GETTER_SETTER(word1, data2)
  UMP_GETTER_SETTER(word1, data3)
  UMP_GETTER_SETTER(word1, data4)
  UMP_GETTER_SETTER(word2, data5)
  UMP_GETTER_SETTER(word2, data6)
  UMP_GETTER_SETTER(word2, data7)
  UMP_GETTER_SETTER(word2, data8)
  UMP_GETTER_SETTER(word3, data9)
  UMP_GETTER_SETTER(word3, data10)
  UMP_GETTER_SETTER(word3, data11)
  UMP_GETTER_SETTER(word3, data12)

  std::tuple<word0, word1, word2, word3> words_;
};

}  // end namespace details

using sysex8_in_1 = details::sysex8<midi2::data128::sysex8_in_1>;
using sysex8_start = details::sysex8<midi2::data128::sysex8_start>;
using sysex8_continue = details::sysex8<midi2::data128::sysex8_continue>;
using sysex8_end = details::sysex8<midi2::data128::sysex8_end>;

// 7.9 Mixed Data Set Message
// Mixed Data Set Header (word 1)
// Mixed Data Set Payload (word 1)
struct mds_header {
  class word0 : public types::details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(midi2::data128::mixed_data_set_header); }

    using mt = types::details::bitfield<28, 4>;  ///< Always 0x05
    using group = types::details::bitfield<24, 4>;
    using status = types::details::bitfield<20, 4>;  ///< Always 0x08
    using mds_id = types::details::bitfield<16, 4>;
    using bytes_in_chunk = types::details::bitfield<0, 16>;
  };
  class word1 : public types::details::word_base {
  public:
    using word_base::word_base;

    using chunks_in_mds = types::details::bitfield<16, 16>;
    using chunk_num = types::details::bitfield<0, 16>;
  };
  class word2 : public types::details::word_base {
  public:
    using word_base::word_base;

    using manufacturer_id = types::details::bitfield<16, 16>;
    using device_id = types::details::bitfield<0, 16>;
  };
  class word3 : public types::details::word_base {
  public:
    using word_base::word_base;

    using sub_id_1 = types::details::bitfield<16, 16>;
    using sub_id_2 = types::details::bitfield<0, 16>;
  };

  constexpr mds_header() = default;
  constexpr explicit mds_header(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(mds_header const &, mds_header const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, mds_id)
  UMP_GETTER_SETTER(word0, bytes_in_chunk)
  UMP_GETTER_SETTER(word1, chunks_in_mds)
  UMP_GETTER_SETTER(word1, chunk_num)
  UMP_GETTER_SETTER(word2, manufacturer_id)
  UMP_GETTER_SETTER(word2, device_id)
  UMP_GETTER_SETTER(word3, sub_id_1)
  UMP_GETTER_SETTER(word3, sub_id_2)

  std::tuple<word0, word1, word2, word3> words_;
};

struct mds_payload {
  class word0 : public ::midi2::types::details::word_base {
  public:
    using word_base::word_base;
    constexpr word0() { this->init<mt, status>(::midi2::data128::mixed_data_set_payload); }

    using mt = ::midi2::types::details::bitfield<28, 4>;  ///< Always 0x05
    using group = ::midi2::types::details::bitfield<24, 4>;
    using status = ::midi2::types::details::bitfield<20, 4>;  ///< Always 0x09
    using mds_id = ::midi2::types::details::bitfield<16, 4>;
    using value0 = ::midi2::types::details::bitfield<0, 16>;
  };
  class word1 : public ::midi2::types::details::word_base {
  public:
    using word_base::word_base;
    using value1 = ::midi2::types::details::bitfield<0, 32>;
  };
  class word2 : public ::midi2::types::details::word_base {
  public:
    using word_base::word_base;
    using value2 = ::midi2::types::details::bitfield<0, 32>;
  };
  class word3 : public ::midi2::types::details::word_base {
  public:
    using word_base::word_base;
    using value3 = ::midi2::types::details::bitfield<0, 32>;
  };

  constexpr mds_payload() = default;
  constexpr explicit mds_payload(std::span<std::uint32_t, 4> m) : words_{m[0], m[1], m[2], m[3]} {}
  friend constexpr bool operator==(mds_payload const &, mds_payload const &) = default;

  UMP_GETTER(word0, mt)
  UMP_GETTER_SETTER(word0, group)
  UMP_GETTER(word0, status)
  UMP_GETTER_SETTER(word0, mds_id)
  UMP_GETTER_SETTER(word0, value0)
  UMP_GETTER_SETTER(word1, value1)
  UMP_GETTER_SETTER(word2, value2)
  UMP_GETTER_SETTER(word3, value3)

  std::tuple<word0, word1, word2, word3> words_;
};

}  // end namespace types::data128

template <> struct message_size<ump_message_type::reserved32_06> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<ump_message_type::reserved32_07> : std::integral_constant<unsigned, 1> {};
template <> struct message_size<ump_message_type::reserved64_08> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<ump_message_type::reserved64_09> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<ump_message_type::reserved64_0A> : std::integral_constant<unsigned, 2> {};
template <> struct message_size<ump_message_type::reserved96_0B> : std::integral_constant<unsigned, 3> {};
template <> struct message_size<ump_message_type::reserved96_0C> : std::integral_constant<unsigned, 3> {};
template <> struct message_size<ump_message_type::reserved128_0E> : std::integral_constant<unsigned, 4> {};

}  // end namespace midi2

namespace std {

template <>
struct tuple_size<midi2::types::system::song_select>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::system::song_select::words_)>::value> {
};
template <>
struct tuple_size<midi2::types::system::tune_request>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::system::tune_request::words_)>::value> {};
template <>
struct tuple_size<midi2::types::system::timing_clock>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::system::timing_clock::words_)>::value> {};

template <>
struct tuple_size<midi2::types::system::sequence_start>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::system::sequence_start::words_)>::value> {};

template <>
struct tuple_size<midi2::types::system::sequence_continue>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::system::sequence_continue::words_)>::value> {};

template <>
struct tuple_size<midi2::types::system::sequence_stop>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::system::sequence_stop::words_)>::value> {};

template <>
struct tuple_size<midi2::types::system::active_sensing>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::system::active_sensing::words_)>::value> {};

template <>
struct tuple_size<midi2::types::system::reset>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::system::reset::words_)>::value> {};

template <midi2::data64 Status>
struct tuple_size<midi2::types::data64::details::sysex7<Status>>  // NOLINT(cert-dcl58-cpp]
    : public std::integral_constant<std::size_t,
                                    tuple_size_v<decltype(midi2::types::data64::details::sysex7<Status>::words_)>> {};

template <>
struct tuple_size<midi2::types::m1cvm::poly_pressure>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m1cvm::poly_pressure::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m1cvm::program_change>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m1cvm::program_change::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m1cvm::channel_pressure>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m1cvm::channel_pressure::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m1cvm::control_change>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m1cvm::control_change::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m1cvm::pitch_bend>
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::m1cvm::pitch_bend::words_)>::value> {};

template <>
struct tuple_size<midi2::types::m2cvm::program_change>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::program_change::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m2cvm::channel_pressure>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::channel_pressure::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m2cvm::rpn_controller>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::rpn_controller::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m2cvm::nrpn_controller>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::nrpn_controller::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m2cvm::nrpn_per_note_controller>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::nrpn_per_note_controller::words_)>::value> {
};
template <>
struct tuple_size<midi2::types::m2cvm::rpn_relative_controller>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::rpn_relative_controller::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m2cvm::nrpn_relative_controller>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::nrpn_relative_controller::words_)>::value> {
};
template <>
struct tuple_size<midi2::types::m2cvm::per_note_management>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::per_note_management::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m2cvm::control_change>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::control_change::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m2cvm::pitch_bend>
    : std::integral_constant<std::size_t, std::tuple_size<decltype(midi2::types::m2cvm::pitch_bend::words_)>::value> {};
template <>
struct tuple_size<midi2::types::m2cvm::per_note_pitch_bend>
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::m2cvm::per_note_pitch_bend::words_)>::value> {};

template <>
struct tuple_size<midi2::types::ump_stream::endpoint_discovery>
    : std::integral_constant<std::size_t,
                             std::tuple_size_v<decltype(midi2::types::ump_stream::endpoint_discovery::words_)>> {};
template <>
struct tuple_size<midi2::types::ump_stream::endpoint_info_notification>
    : std::integral_constant<
          std::size_t, std::tuple_size_v<decltype(midi2::types::ump_stream::endpoint_info_notification::words_)>> {};
template <>
struct tuple_size<midi2::types::ump_stream::device_identity_notification>
    : std::integral_constant<
          std::size_t, std::tuple_size_v<decltype(midi2::types::ump_stream::device_identity_notification::words_)>> {};
template <>
struct tuple_size<midi2::types::ump_stream::endpoint_name_notification>
    : std::integral_constant<
          std::size_t, std::tuple_size_v<decltype(midi2::types::ump_stream::endpoint_name_notification::words_)>> {};
template <>
struct tuple_size<midi2::types::ump_stream::product_instance_id_notification>
    : std::integral_constant<
          std::size_t,
          std::tuple_size_v<decltype(midi2::types::ump_stream::product_instance_id_notification::words_)>> {};
template <>
struct tuple_size<midi2::types::ump_stream::jr_configuration_request>
    : std::integral_constant<std::size_t,
                             std::tuple_size_v<decltype(midi2::types::ump_stream::jr_configuration_request::words_)>> {
};
template <>
struct tuple_size<midi2::types::ump_stream::jr_configuration_notification>
    : std::integral_constant<
          std::size_t, std::tuple_size_v<decltype(midi2::types::ump_stream::jr_configuration_notification::words_)>> {};
template <>
struct tuple_size<midi2::types::ump_stream::function_block_discovery>
    : std::integral_constant<std::size_t,
                             std::tuple_size_v<decltype(midi2::types::ump_stream::function_block_discovery::words_)>> {
};
template <>
struct tuple_size<midi2::types::ump_stream::function_block_info_notification>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<
          std::size_t,
          std::tuple_size_v<decltype(midi2::types::ump_stream::function_block_info_notification::words_)>> {};
template <>
struct tuple_size<midi2::types::ump_stream::function_block_name_notification>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<
          std::size_t,
          std::tuple_size_v<decltype(midi2::types::ump_stream::function_block_name_notification::words_)>> {};
template <>
struct tuple_size<midi2::types::ump_stream::start_of_clip>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size_v<decltype(midi2::types::ump_stream::start_of_clip::words_)>> {};
template <>
struct tuple_size<midi2::types::ump_stream::end_of_clip>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size_v<decltype(midi2::types::ump_stream::end_of_clip::words_)>> {
};

// NOLINTNEXTLINE(cert-dcl58-cpp]
template <midi2::data128 Status>
struct tuple_size<midi2::types::data128::details::sysex8<Status>>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size_v<decltype(midi2::types::data128::details::sysex8<Status>::words_)>> {};
template <>
struct tuple_size<midi2::types::data128::mds_header>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size_v<decltype(midi2::types::data128::mds_header::words_)>> {};
template <>
struct tuple_size<midi2::types::data128::mds_payload>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t, std::tuple_size_v<decltype(midi2::types::data128::mds_payload::words_)>> {};

template <>
struct tuple_size<midi2::types::flex_data::set_chord_name>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::flex_data::set_chord_name::words_)>::value> {};
template <>
struct tuple_size<midi2::types::flex_data::set_key_signature>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::flex_data::set_key_signature::words_)>::value> {};
template <>
struct tuple_size<midi2::types::flex_data::set_metronome>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::flex_data::set_metronome::words_)>::value> {};
template <>
struct tuple_size<midi2::types::flex_data::set_time_signature>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::flex_data::set_time_signature::words_)>::value> {};
template <>
struct tuple_size<midi2::types::flex_data::set_tempo>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::flex_data::set_tempo::words_)>::value> {};
template <>
struct tuple_size<midi2::types::flex_data::text_common>  // NOLINT(cert-dcl58-cpp]
    : std::integral_constant<std::size_t,
                             std::tuple_size<decltype(midi2::types::flex_data::text_common::words_)>::value> {};

}  // end namespace std

#undef UMP_GETTER
#undef UMP_SETTER
#undef UMP_GETTER_SETTER

#endif  // MIDI2_UMP_TYPES_HPP
