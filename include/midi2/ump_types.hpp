//===-- UMP Types -------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMP_TYPES_HPP
#define MIDI2_UMP_TYPES_HPP

#include <bit>
#include <cstdint>
#include <cstring>
#include <span>
#include <tuple>

#include "midi2/adt/bitfield.hpp"
#include "midi2/utils.hpp"

#define UMP_MEMBERS0(name, st)                                                 \
  name() {                                                                     \
    static_assert(sizeof(name) == sizeof(std::uint32_t));                      \
    std::memset(this, 0, sizeof(*this));                                       \
    this->mt = to_underlying(status_to_message_type(st));                      \
    this->status = status_to_ump_status(st);                                   \
  }                                                                            \
  explicit name(std::uint32_t value_) {                                        \
    std::memcpy(this, &value_, sizeof(*this));                                 \
  }                                                                            \
  [[nodiscard]] constexpr auto word() const {                                  \
    return std::bit_cast<std::uint32_t>(*this);                                \
  }                                                                            \
  friend constexpr bool operator==(name const &a, name const &b) {             \
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b); \
  }

#define UMP_MEMBERS(name)                                                      \
  name() {                                                                     \
    static_assert(sizeof(name) == sizeof(std::uint32_t));                      \
    std::memset(this, 0, sizeof(*this));                                       \
  }                                                                            \
  explicit name(std::uint32_t value) {                                         \
    std::memcpy(this, &value, sizeof(*this));                                  \
  }                                                                            \
  [[nodiscard]] constexpr auto word() const {                                  \
    return std::bit_cast<std::uint32_t>(*this);                                \
  }                                                                            \
  friend constexpr bool operator==(name const& a, name const& b) {             \
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b); \
  }

// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init,cppcoreguidelines-prefer-member-initializer,hicpp-member-init)

namespace midi2::types {

template <typename T>
concept bitfield_type = requires(T) {
  std::unsigned_integral<typename T::index::value_type>;
  std::unsigned_integral<typename T::bits::value_type>;
};

constexpr auto status_to_message_type(status) {
  return ump_message_type::m1cvm;
}
constexpr auto status_to_message_type(system_crt) {
  return ump_message_type::system;
}
constexpr auto status_to_message_type(m2cvm) {
  return ump_message_type::m2cvm;
}
constexpr auto status_to_message_type(ump_utility) {
  return ump_message_type::utility;
}
//  X(m1cvm, 0x02)
constexpr auto status_to_message_type(data64) {
  return ump_message_type::data64;
}
//  X(m2cvm, 0x04)
constexpr auto status_to_message_type(data128) {
  return ump_message_type::data128;
}
constexpr auto status_to_message_type(flex_data) {
  return ump_message_type::flex_data;
}
constexpr auto status_to_message_type(ump_stream) {
  return ump_message_type::ump_stream;
}

template <typename T> constexpr auto status_to_ump_status(T status) {
  return to_underlying(status);
}
template <> constexpr auto status_to_ump_status(status status) {
  auto const s = to_underlying(status);
  return static_cast<std::uint8_t>(s < to_underlying(status::sysex_start) ? s >> 4 : s);
}

namespace details {

template <unsigned Index, unsigned Bits> struct bitfield {
  using index = std::integral_constant<unsigned, Index>;
  using bits = std::integral_constant<unsigned, Bits>;
};

class word_base {
public:
  using value_type = std::uint32_t;

  constexpr word_base() = default;
  constexpr explicit word_base(std::uint32_t const v) : value_{v} {}

  [[nodiscard]] constexpr auto word() const { return value_; }
  friend constexpr bool operator==(word_base const &a, word_base const &b) { return a.value_ == b.value_; }

  template <bitfield_type BitRange> constexpr word_base &set(unsigned v) {
    constexpr auto index = typename BitRange::index();
    constexpr auto bits = typename BitRange::bits();
    constexpr auto mask = max_value<value_type, bits>();
    assert(v <= mask);
    value_ = static_cast<value_type>(value_ & ~(mask << index)) |
             static_cast<value_type>((static_cast<value_type>(v) & mask) << index);
    return *this;
  }

  template <bitfield_type BitRange> constexpr small_type<BitRange::bits::value> get() const {
    constexpr auto index = typename BitRange::index();
    constexpr auto bits = typename BitRange::bits();
    constexpr auto mask = max_value<value_type, bits>();
    return (value_ >> index) & mask;
  }

protected:
  template <bitfield_type MtField, bitfield_type StatusField> constexpr void init(auto const status) {
    this->set<MtField>(to_underlying(status_to_message_type(status)));
    this->set<StatusField>(status_to_ump_status(status));
  }

private:
  value_type value_ = 0;
};

}  // end namespace details

template <unsigned Index, unsigned Bits> using ump_bitfield = bitfield<std::uint32_t, Index, Bits>;

//*       _   _ _ _ _         *
//*  _  _| |_(_) (_) |_ _  _  *
//* | || |  _| | | |  _| || | *
//*  \_,_|\__|_|_|_|\__|\_, | *
//*                     |__/  *
namespace utility {
// F.1.1 Message Type 0x0: Utility
// Table 26 4-Byte UMP Formats for Message Type 0x0: Utility

// 7.2.1 NOOP
struct noop {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(ump_utility::noop); }

    using mt = details::bitfield<28, 4>;
    using status = details::bitfield<20, 4>;
  };

  constexpr noop() = default;
  constexpr explicit noop(std::uint32_t const w0) : w{w0} {}
  friend constexpr bool operator==(noop const &, noop const &) = default;

  std::tuple<word0> w;
};

// 7.2.2.1 JR Clock Message
struct jr_clock {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(ump_utility::jr_clock); }

    using mt = details::bitfield<28, 4>;  // 0x0
    using reserved0 = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  // 0b0001
    using reserved1 = details::bitfield<16, 4>;
    using sender_clock_time = details::bitfield<0, 16>;
  };

  constexpr jr_clock() = default;
  constexpr explicit jr_clock(std::uint32_t const w0) : w{w0} {}
  friend constexpr bool operator==(jr_clock const &, jr_clock const &) = default;

  constexpr auto mt() const { return get<0>(w).get<word0::mt>(); }
  constexpr auto status() const { return get<0>(w).get<word0::status>(); }
  constexpr auto sender_clock_time() const { return get<0>(w).get<word0::sender_clock_time>(); }

  constexpr auto &sender_clock_time(std::uint16_t const v) {
    get<0>(w).set<word0::sender_clock_time>(v);
    return *this;
  }

  std::tuple<word0> w;
};

// 7.2.2.2 JR Timestamp Message
struct jr_timestamp {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(ump_utility::jr_ts); }

    using mt = details::bitfield<28, 4>;  // 0x0
    using reserved0 = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  // 0b0010
    using reserved1 = details::bitfield<16, 4>;
    using timestamp = details::bitfield<0, 16>;
  };

  jr_timestamp() = default;
  explicit jr_timestamp(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(jr_timestamp const &, jr_timestamp const &) = default;

  std::tuple<word0> w;
};

// 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (DCTPQ)
struct delta_clockstamp_tpqn {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(ump_utility::delta_clock_tick); }

    using mt = details::bitfield<28, 4>;  // 0x0
    using reserved0 = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  // 0b0011
    using reserved1 = details::bitfield<16, 4>;
    using ticks_pqn = details::bitfield<0, 16>;
  };

  delta_clockstamp_tpqn() = default;
  explicit delta_clockstamp_tpqn(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(delta_clockstamp_tpqn const &, delta_clockstamp_tpqn const &) = default;

  std::tuple<word0> w;
};

// 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
struct delta_clockstamp {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(ump_utility::delta_clock_since); }

    using mt = details::bitfield<28, 4>;  // 0x0
    using reserved0 = details::bitfield<24, 4>;
    using status = details::bitfield<20, 4>;  // 0b0100
    using ticks_per_quarter_note = details::bitfield<0, 20>;
  };

  delta_clockstamp() = default;
  explicit delta_clockstamp(word0 const w0_) : w{w0_} {}
  explicit delta_clockstamp(std::uint32_t const w0_) : w{w0_} {}
  friend bool operator==(delta_clockstamp const &, delta_clockstamp const &) = default;

  std::tuple<word0> w;
};

}  // end namespace utility

//*             _              *
//*  ____  _ __| |_ ___ _ __   *
//* (_-< || (_-<  _/ -_) '  \  *
//* /__/\_, /__/\__\___|_|_|_| *
//*     |__/                   *
// 7.6 System Common and System Real Time Messages
namespace system {

struct midi_time_code {
  class word0 : public details::word_base {
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

  midi_time_code() = default;
  explicit midi_time_code(std::uint32_t const w0_) : w{w0_} {}
  friend bool operator==(midi_time_code const &, midi_time_code const &) = default;

  constexpr auto mt() const { return get<word0>(w).get<word0::mt>(); }
  constexpr auto group() const { return get<word0>(w).get<word0::group>(); }
  constexpr auto status() const { return get<word0>(w).get<word0::status>(); }
  constexpr auto time_code() const { return get<word0>(w).get<word0::time_code>(); }

  constexpr auto & group(std::uint8_t const v) { get<word0>(w).set<word0::group>(v); return *this; }
  constexpr auto & time_code(std::uint8_t const v) { get<word0>(w).set<word0::time_code>(v); return *this; }

  std::tuple<word0> w;
};
struct song_position_pointer {
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

  song_position_pointer() = default;
  explicit song_position_pointer(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(song_position_pointer const &, song_position_pointer const &) = default;

  std::tuple<word0> w;
};

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

  song_select() = default;
  explicit song_select(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(song_select const &, song_select const &) = default;

  std::tuple<word0> w;
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

  tune_request() = default;
  explicit tune_request(std::uint32_t const w0_) : w{w0_} {}
  friend bool operator==(tune_request const &, tune_request const &) = default;

  std::tuple<word0> w;
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

  timing_clock() = default;
  explicit timing_clock(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(timing_clock const &, timing_clock const &) = default;

  std::tuple<word0> w;
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

  sequence_start() = default;
  explicit sequence_start(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(sequence_start const &, sequence_start const &) = default;

  std::tuple<word0> w;
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

  sequence_continue() = default;
  explicit sequence_continue(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(sequence_continue const &, sequence_continue const &) = default;

  std::tuple<word0> w;
};

struct sequence_stop {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::sequence_stop); }

    using mt = details::bitfield<28, 4>;  // Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  // Always 0xFC
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };

  sequence_stop() = default;
  explicit sequence_stop(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(sequence_stop const &, sequence_stop const &) = default;

  std::tuple<word0> w;
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

  active_sensing() = default;
  explicit active_sensing(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(active_sensing const &, active_sensing const &) = default;

  std::tuple<word0> w;
};

struct reset {
  class word0 : public details::word_base {
  public:
    using word_base::word_base;

    constexpr word0() { this->init<mt, status>(system_crt::system_reset); }

    using mt = details::bitfield<28, 4>;  // Always 0x1
    using group = details::bitfield<24, 4>;
    using status = details::bitfield<16, 8>;  // Always 0xFF
    using reserved0 = details::bitfield<8, 8>;
    using reserved1 = details::bitfield<0, 8>;
  };

  reset() = default;
  explicit reset(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(reset const &, reset const &) = default;

  std::tuple<word0> w;
};

}  // end namespace system

//*        _                 *
//*  _ __ / |  ____ ___ __   *
//* | '  \| | / _\ V / '  \  *
//* |_|_|_|_| \__|\_/|_|_|_| *
//*                          *
// F.1.3 Mess Type 0x2: MIDI 1.0 Channel Voice Messages
// Table 28 4-Byte UMP Formats for Message Type 0x2: MIDI 1.0 Channel Voice
// Messages
namespace m1cvm {

// 7.3.2 MIDI 1.0 Note On Message
struct note_on {
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

  note_on() = default;
  explicit note_on(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(note_on const &, note_on const &) = default;

  constexpr std::uint8_t mt() const { return get<word0>(w).get<word0::mt>(); }
  constexpr std::uint8_t group() const { return get<word0>(w).get<word0::group>(); }
  constexpr std::uint8_t status() const { return get<word0>(w).get<word0::status>(); }
  constexpr std::uint8_t channel() const { return get<word0>(w).get<word0::channel>(); }
  constexpr std::uint8_t note() const { return get<word0>(w).get<word0::note>(); }
  constexpr std::uint8_t velocity() const { return get<word0>(w).get<word0::velocity>(); }

  constexpr auto & group(std::uint8_t const v){ get<word0>(w).set<word0::group>(v); return *this; }
  constexpr auto & channel(std::uint8_t const v) { get<word0>(w).set<word0::channel>(v); return *this; }
  constexpr auto & note(std::uint8_t const v) { get<word0>(w).set<word0::note>(v); return *this; }
  constexpr auto & velocity(std::uint8_t const v) { get<word0>(w).set<word0::velocity>(v); return *this; }

  std::tuple<word0> w;
};

// 7.3.1 MIDI 1.0 Note Off Message
struct note_off {
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

  note_off() = default;
  explicit note_off(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(note_off const &, note_off const &) = default;

  constexpr std::uint8_t mt() const { return get<word0>(w).get<word0::mt>(); }
  constexpr std::uint8_t group() const { return get<word0>(w).get<word0::group>(); }
  constexpr std::uint8_t status() const { return get<word0>(w).get<word0::status>(); }
  constexpr std::uint8_t channel() const { return get<word0>(w).get<word0::channel>(); }
  constexpr std::uint8_t note() const { return get<word0>(w).get<word0::note>(); }
  constexpr std::uint8_t velocity() const { return get<word0>(w).get<word0::velocity>(); }

  constexpr auto & group(std::uint8_t const v){ get<word0>(w).set<word0::group>(v); return *this; }
  constexpr auto & channel(std::uint8_t const v) { get<word0>(w).set<word0::channel>(v); return *this; }
  constexpr auto & note(std::uint8_t const v) { get<word0>(w).set<word0::note>(v); return *this; }
  constexpr auto & velocity(std::uint8_t const v) { get<word0>(w).set<word0::velocity>(v); return *this; }

  std::tuple<word0> w;
};

// 7.3.3 MIDI 1.0 Poly Pressure Message
struct poly_pressure {
  union word0 {
    UMP_MEMBERS0(word0, status::poly_pressure)
    ump_bitfield<28, 4> mt;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  /// Always 0x0A.
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> note;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> pressure;
  };

  poly_pressure() = default;
  explicit poly_pressure(std::uint32_t const w0_) : w{w0_} {}
  friend bool operator==(poly_pressure const &, poly_pressure const &) = default;

  std::tuple<word0> w;
};

// 7.3.4 MIDI 1.0 Control Change Message
struct control_change {
  union word0 {
    UMP_MEMBERS0(word0, status::cc)
    ump_bitfield<28, 4> mt;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  /// Always 0x0B.
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> controller;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> value;
  };

  control_change() = default;
  explicit control_change(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(control_change const &, control_change const &) = default;

  std::tuple<word0> w;
};

// 7.3.5 MIDI 1.0 Program Change Message
struct program_change {
  union word0 {
    UMP_MEMBERS0(word0, status::program_change)
    ump_bitfield<28, 4> mt;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0x0C.
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> program;
    ump_bitfield<0, 8> reserved1;
  };

  program_change() = default;
  explicit program_change(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(program_change const &, program_change const &) = default;

  std::tuple<word0> w;
};

// 7.3.6 MIDI 1.0 Channel Pressure Message
struct channel_pressure {
  union word0 {
    UMP_MEMBERS0(word0, status::channel_pressure)
    ump_bitfield<28, 4> mt;  ///< Always 0x2 (MIDI 1.0 Channel Voice)
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0x0D.
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> data;
    ump_bitfield<0, 8> reserved1;
  };

  channel_pressure() = default;
  explicit channel_pressure(std::uint32_t const w0_) : w{w0_} {}
  friend bool operator==(channel_pressure const &, channel_pressure const &) = default;

  std::tuple<word0> w;
};

// 7.3.7 MIDI 1.0 Pitch Bend Message
struct pitch_bend {
  union word0 {
    UMP_MEMBERS0(word0, status::pitch_bend)
    ump_bitfield<28, 4> mt;  // 0x2
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  // 0b1000..0b1110
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> lsb_data;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> msb_data;
  };
  pitch_bend() = default;
  explicit pitch_bend(std::uint32_t const w0) : w{w0} {}
  friend bool operator==(pitch_bend const &, pitch_bend const &) = default;

  std::tuple<word0> w;
};

}  // end namespace m1cvm

//*     _      _         __ _ _   *
//*  __| |__ _| |_ __ _ / /| | |  *
//* / _` / _` |  _/ _` / _ \_  _| *
//* \__,_\__,_|\__\__,_\___/ |_|  *
//*                               *
namespace data64 {

// 7.7 System Exclusive (7-Bit) Messages
namespace details {

template <midi2::data64 Status> struct sysex7 {
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

  constexpr sysex7() : w{word0{}, word1{}} {}

  explicit sysex7(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(sysex7 const &, sysex7 const &) = default;

  constexpr auto mt() const { return get<word0>(w).template get<typename word0::mt>(); }
  constexpr auto group() const { return get<word0>(w).template get<typename word0::group>(); }
  constexpr auto status() const { return get<word0>(w).template get<typename word0::status>(); }
  constexpr auto number_of_bytes() const { return get<word0>(w).template get<typename word0::number_of_bytes>(); }
  constexpr auto data0() const { return get<word0>(w).template get<typename word0::data0>(); }
  constexpr auto data1() const { return get<word0>(w).template get<typename word0::data1>(); }
  constexpr auto data2() const { return get<word1>(w).template get<typename word1::data2>(); }
  constexpr auto data3() const { return get<word1>(w).template get<typename word1::data3>(); }
  constexpr auto data4() const { return get<word1>(w).template get<typename word1::data4>(); }
  constexpr auto data5() const { return get<word1>(w).template get<typename word1::data5>(); }

  constexpr auto & group(std::uint8_t const v) { get<word0>(w).template set<typename word0::group>(v); return *this; }
  constexpr auto & number_of_bytes(std::uint8_t const v) { get<word0>(w).template set<typename word0::number_of_bytes>(v); return *this; }
  constexpr auto & data0(std::uint8_t const v) { get<word0>(w).template set<typename word0::data0>(v); return *this; }
  constexpr auto & data1(std::uint8_t const v) { get<word0>(w).template set<typename word0::data1>(v); return *this; }
  constexpr auto & data2(std::uint8_t const v) { get<word1>(w).template set<typename word1::data2>(v); return *this; }
  constexpr auto & data3(std::uint8_t const v) { get<word1>(w).template set<typename word1::data3>(v); return *this; }
  constexpr auto & data4(std::uint8_t const v) { get<word1>(w).template set<typename word1::data4>(v); return *this; }
  constexpr auto & data5(std::uint8_t const v) { get<word1>(w).template set<typename word1::data5>(v); return *this; }

  std::tuple<word0, word1> w{};
};

}  // end namespace details

using sysex7_in_1 = details::sysex7<midi2::data64::sysex7_in_1>;
using sysex7_start = details::sysex7<midi2::data64::sysex7_start>;
using sysex7_continue = details::sysex7<midi2::data64::sysex7_continue>;
using sysex7_end = details::sysex7<midi2::data64::sysex7_end>;

}  // end namespace data64

//*        ___               *
//*  _ __ |_  )____ ___ __   *
//* | '  \ / // _\ V / '  \  *
//* |_|_|_/___\__|\_/|_|_|_| *
//*                          *
// F.2.2 Message Type 0x4: MIDI 2.0 Channel Voice Messages
// Table 30 8-Byte UMP Formats for Message Type 0x4: MIDI 2.0 Channel Voice Messages
namespace m2cvm {

// 7.4.1 MIDI 2.0 Note Off Message
struct note_off {
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

  constexpr note_off() = default;
  constexpr explicit note_off(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(note_off const &a, note_off const &b) = default;

  constexpr std::uint8_t mt() const { return get<word0>(w).get<word0::mt>(); }
  constexpr std::uint8_t group() const { return get<word0>(w).get<word0::group>(); }
  constexpr std::uint8_t status() const { return get<word0>(w).get<word0::status>(); }
  constexpr std::uint8_t channel() const { return get<word0>(w).get<word0::channel>(); }
  constexpr std::uint8_t note() const { return get<word0>(w).get<word0::note>(); }
  constexpr std::uint8_t attribute_type() const { return get<word0>(w).get<word0::attribute_type>(); }
  constexpr std::uint16_t velocity() const { return get<word1>(w).get<word1::velocity>(); }
  constexpr std::uint16_t attribute() const { return get<word1>(w).get<word1::attribute>(); }

  constexpr auto & group(std::uint8_t const v){ get<word0>(w).set<word0::group>(v); return *this; }
  constexpr auto & channel(std::uint8_t const v) { get<word0>(w).set<word0::channel>(v); return *this; }
  constexpr auto & note(std::uint8_t const v) { get<word0>(w).set<word0::note>(v); return *this; }
  constexpr auto & attribute_type(std::uint8_t const v) { get<word0>(w).set<word0::attribute_type>(v); return *this; }
  constexpr auto & velocity(std::uint16_t const v) { get<word1>(w).set<word1::velocity>(v); return *this; }
  constexpr auto & attribute(std::uint16_t const v) { get<word1>(w).set<word1::attribute>(v); return *this; }

  std::tuple<word0, word1> w;
};

// 7.4.2 MIDI 2.0 Note On Message
struct note_on {
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

  constexpr note_on() = default;
  constexpr explicit note_on(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(note_on const &a, note_on const &b) = default;

  constexpr std::uint8_t mt() const { return get<word0>(w).get<word0::mt>(); }
  constexpr std::uint8_t group() const { return get<word0>(w).get<word0::group>(); }
  constexpr std::uint8_t status() const { return get<word0>(w).get<word0::status>(); }
  constexpr std::uint8_t channel() const { return get<word0>(w).get<word0::channel>(); }
  constexpr std::uint8_t note() const { return get<word0>(w).get<word0::note>(); }
  constexpr std::uint8_t attribute_type() const { return get<word0>(w).get<word0::attribute_type>(); }
  constexpr std::uint16_t velocity() const { return get<word1>(w).get<word1::velocity>(); }
  constexpr std::uint16_t attribute() const { return get<word1>(w).get<word1::attribute>(); }

  constexpr auto & group(std::uint8_t const v){ get<word0>(w).set<word0::group>(v); return *this; }
  constexpr auto & channel(std::uint8_t const v) { get<word0>(w).set<word0::channel>(v); return *this; }
  constexpr auto & note(std::uint8_t const v) { get<word0>(w).set<word0::note>(v); return *this; }
  constexpr auto & attribute_type(std::uint8_t const v) { get<word0>(w).set<word0::attribute_type>(v); return *this; }
  constexpr auto & velocity(std::uint16_t const v) { get<word1>(w).set<word1::velocity>(v); return *this; }
  constexpr auto & attribute(std::uint16_t const v) { get<word1>(w).set<word1::attribute>(v); return *this; }

  std::tuple<word0, word1> w;
};

// 7.4.3 MIDI 2.0 Poly Pressure Message
struct poly_pressure {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::poly_pressure)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0xA
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> note;
    ump_bitfield<0, 8> reserved1;
  };
  using word1 = std::uint32_t;

  poly_pressure() = default;
  explicit poly_pressure(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(poly_pressure const &a, poly_pressure const &b) = default;

  std::tuple<word0, word1> w;
};

// 7.4.4 MIDI 2.0 Registered Per-Note Controller Messages
struct rpn_per_note_controller {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::rpn_pernote)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Registered Per-Note Controller=0x0
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved;
    ump_bitfield<8, 7> note;
    ump_bitfield<0, 8> index;
  };
  using word1 = std::uint32_t;

  rpn_per_note_controller() = default;
  explicit rpn_per_note_controller(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(rpn_per_note_controller const &a, rpn_per_note_controller const &b) = default;

  std::tuple<word0, word1> w;
};

// 7.4.4 MIDI 2.0 Assignable Per-Note Controller Messages
struct nrpn_per_note_controller {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::nrpn_pernote)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Assignable Per-Note Controller=0x1
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved;
    ump_bitfield<8, 7> note;
    ump_bitfield<0, 8> index;
  };
  using word1 = std::uint32_t;

  nrpn_per_note_controller() = default;
  explicit nrpn_per_note_controller(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(nrpn_per_note_controller const &a, nrpn_per_note_controller const &b) = default;

  std::tuple<word0, word1> w;
};
// 7.4.7 MIDI 2.0 Registered Controller (RPN) Message
/// "Registered Controllers have specific functions defined by MMA/AMEI specifications. Registered Controllers
/// map and translate directly to MIDI 1.0 Registered Parameter Numbers and use the same
/// definitions as MMA/AMEI approved RPN messages. Registered Controllers are organized in 128 Banks
/// (corresponds to RPN MSB), with 128 controllers per Bank (corresponds to RPN LSB).
struct rpn_controller {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::rpn)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Registered Control (RPN)=0x2
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> bank;  ///< Corresponds to RPN MSB
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> index;  ///< Corresponds to RPN LSB
  };
  using word1 = std::uint32_t;

  rpn_controller() = default;
  explicit rpn_controller(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(rpn_controller const &a, rpn_controller const &b) = default;

  std::tuple<word0, word1> w;
};

// 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message
struct nrpn_controller {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::nrpn)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Assignable Control (RPN)=0x3
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> bank;  ///< Corresponds to NRPN MSB
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> index;  ///< Corresponds to NRPN LSB
  };
  using word1 = std::uint32_t;

  nrpn_controller() = default;
  explicit nrpn_controller(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(nrpn_controller const &a, nrpn_controller const &b) = default;

  std::tuple<word0, word1> w;
};

// 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message
struct rpn_relative_controller {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::rpn_relative)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Registered Relative Control (RPN)=0x4
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> bank;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> index;
  };
  using word1 = std::uint32_t;

  rpn_relative_controller() = default;
  explicit rpn_relative_controller(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(rpn_relative_controller const &a, rpn_relative_controller const &b) = default;

  std::tuple<word0, word1> w;
};
// 7.4.8 MIDI 2.0 Assignable Controller (NRPN) Message
struct nrpn_relative_controller {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::nrpn_relative)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Assignable Relative Control (NRPN)=0x5
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> bank;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> index;
  };
  using word1 = std::uint32_t;

  nrpn_relative_controller() = default;
  explicit nrpn_relative_controller(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(nrpn_relative_controller const &a, nrpn_relative_controller const &b) = default;

  std::tuple<word0, word1> w;
};
// 7.4.5 MIDI 2.0 Per-Note Management Message
struct per_note_management {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::pernote_manage)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Per-Note Management=0xF
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved;
    ump_bitfield<8, 7> note;
    ump_bitfield<0, 1> option_flags;
    ump_bitfield<1, 1> detach;  ///< Detach per-note controllers from previously received note(s)
    ump_bitfield<0, 1> set;     ///< Reset (set) per-note controllers to default values
  };
  using word1 = std::uint32_t;

  per_note_management() = default;
  explicit per_note_management(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(per_note_management const &a, per_note_management const &b) = default;

  std::tuple<word0, word1> w;
};

// 7.4.6 MIDI 2.0 Control Change Message
struct control_change {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::cc)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0xB
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> controller;
    ump_bitfield<0, 8> reserved1;
  };
  using word1 = std::uint32_t;

  control_change() = default;
  explicit control_change(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(control_change const &a, control_change const &b) = default;

  std::tuple<word0, word1> w;
};

// 7.4.9 MIDI 2.0 Program Change Message
struct program_change {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::program_change)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0xC
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> reserved;
    ump_bitfield<1, 7> option_flags;  ///< Reserved option flags
    ump_bitfield<0, 1> bank_valid;    ///< Bank change is ignored if this bit is zero.
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<24, 8> program;
    ump_bitfield<16, 8> reserved0;
    ump_bitfield<15, 1> reserved1;
    ump_bitfield<8, 7> bank_msb;
    ump_bitfield<7, 1> reserved2;
    ump_bitfield<0, 7> bank_lsb;
  };

  program_change() = default;
  explicit program_change(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(program_change const &a, program_change const &b) = default;

  std::tuple<word0, word1> w;
};

// 7.4.10 MIDI 2.0 Channel Pressure Message
struct channel_pressure {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::channel_pressure)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0xD
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  using word1 = std::uint32_t;

  channel_pressure() = default;
  explicit channel_pressure(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(channel_pressure const &a, channel_pressure const &b) = default;

  std::tuple<word0, word1> w;
};

// 7.4.11 MIDI 2.0 Pitch Bend Message
struct pitch_bend {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::pitch_bend)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0xE
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  using word1 = std::uint32_t;

  pitch_bend() = default;
  explicit pitch_bend(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(pitch_bend const &a, pitch_bend const &b) = default;

  std::tuple<word0, word1> w;
};

// 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
struct per_note_pitch_bend {
  union word0 {
    UMP_MEMBERS0(word0, midi2::m2cvm::pitch_bend_pernote)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0x6
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> note;
    ump_bitfield<0, 8> reserved1;
  };
  using word1 = std::uint32_t;

  per_note_pitch_bend() = default;
  explicit per_note_pitch_bend(std::span<std::uint32_t, 2> m) : w{m[0], m[1]} {}
  friend constexpr bool operator==(per_note_pitch_bend const &a, per_note_pitch_bend const &b) = default;

  std::tuple<word0, word1> w;
};

}  // end namespace m2cvm

//*                       _                       *
//*  _  _ _ __  _ __   __| |_ _ _ ___ __ _ _ __   *
//* | || | '  \| '_ \ (_-<  _| '_/ -_) _` | '  \  *
//*  \_,_|_|_|_| .__/ /__/\__|_| \___\__,_|_|_|_| *
//*            |_|                                *
namespace ump_stream {

// 7.1.1 Endpoint Discovery Message
struct endpoint_discovery {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::endpoint_discovery)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x00
    ump_bitfield<8, 8> version_major;
    ump_bitfield<0, 8> version_minor;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<8, 24> reserved;
    ump_bitfield<0, 8> filter;
  };
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  endpoint_discovery() = default;
  explicit endpoint_discovery(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(endpoint_discovery const &lhs, endpoint_discovery const &rhs) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.2 Endpoint Info Notification Message
struct endpoint_info_notification {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::endpoint_info_notification)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x01
    ump_bitfield<8, 8> version_major;
    ump_bitfield<0, 8> version_minor;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<31, 1> static_function_blocks;
    ump_bitfield<24, 7> number_function_blocks;
    ump_bitfield<10, 14> reserved0;
    ump_bitfield<9, 1> midi2_protocol_capability;
    ump_bitfield<8, 1> midi1_protocol_capability;
    ump_bitfield<2, 6> reserved1;
    ump_bitfield<1, 1> receive_jr_timestamp_capability;
    ump_bitfield<0, 1> transmit_jr_timestamp_capability;
  };
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  endpoint_info_notification() = default;
  explicit endpoint_info_notification(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(endpoint_info_notification const &, endpoint_info_notification const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.3 Device Identity Notification Message
struct device_identity_notification {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::device_identity_notification)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x02
    ump_bitfield<0, 16> reserved0;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<24, 8> reserved0;
    ump_bitfield<23, 1> reserved1;
    ump_bitfield<16, 7> dev_manuf_sysex_id_1;  // device manufacturer sysex id byte 1
    ump_bitfield<15, 1> reserved2;
    ump_bitfield<8, 7> dev_manuf_sysex_id_2;  // device manufacturer sysex id byte 2
    ump_bitfield<7, 1> reserved3;
    ump_bitfield<0, 7> dev_manuf_sysex_id_3;  // device manufacturer sysex id byte 3
  };
  union word2 {
    UMP_MEMBERS(word2)
    ump_bitfield<31, 1> reserved0;
    ump_bitfield<24, 7> device_family_lsb;
    ump_bitfield<23, 1> reserved1;
    ump_bitfield<16, 7> device_family_msb;
    ump_bitfield<15, 1> reserved2;
    ump_bitfield<8, 7> device_family_model_lsb;
    ump_bitfield<7, 1> reserved3;
    ump_bitfield<0, 7> device_family_model_msb;
  };
  union word3 {
    UMP_MEMBERS(word3)
    ump_bitfield<31, 1> reserved0;
    ump_bitfield<24, 7> sw_revision_1;  // Software revision level byte 1
    ump_bitfield<23, 1> reserved1;
    ump_bitfield<16, 7> sw_revision_2;  // Software revision level byte 2
    ump_bitfield<15, 1> reserved2;
    ump_bitfield<8, 7> sw_revision_3;  // Software revision level byte 3
    ump_bitfield<7, 1> reserved3;
    ump_bitfield<0, 7> sw_revision_4;  // Software revision level byte 4
  };
  device_identity_notification() = default;
  explicit device_identity_notification(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(device_identity_notification const &, device_identity_notification const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.4 Endpoint Name Notification
struct endpoint_name_notification {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::endpoint_name_notification)
    ump_bitfield<28, 4> mt;  // 0x0F
    ump_bitfield<26, 2> format;
    ump_bitfield<16, 10> status;  // 0x03
    ump_bitfield<8, 8> name1;
    ump_bitfield<0, 8> name2;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<24, 8> name3;
    ump_bitfield<16, 8> name4;
    ump_bitfield<8, 8> name5;
    ump_bitfield<0, 8> name6;
  };
  union word2 {
    UMP_MEMBERS(word2)
    ump_bitfield<24, 8> name7;
    ump_bitfield<16, 8> name8;
    ump_bitfield<8, 8> name9;
    ump_bitfield<0, 8> name10;
  };
  union word3 {
    UMP_MEMBERS(word3)
    ump_bitfield<24, 8> name11;
    ump_bitfield<16, 8> name12;
    ump_bitfield<8, 8> name13;
    ump_bitfield<0, 8> name14;
  };
  endpoint_name_notification() = default;
  explicit endpoint_name_notification(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(endpoint_name_notification const &, endpoint_name_notification const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.5 Product Instance Id Notification Message
struct product_instance_id_notification {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::product_instance_id_notification)
    ump_bitfield<28, 4> mt;
    ump_bitfield<26, 2> format;
    ump_bitfield<16, 10> status;
    ump_bitfield<8, 8> pid1;
    ump_bitfield<0, 8> pid2;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<24, 8> pid3;
    ump_bitfield<16, 8> pid4;
    ump_bitfield<8, 8> pid5;
    ump_bitfield<0, 8> pid6;
  };
  union word2 {
    UMP_MEMBERS(word2)
    ump_bitfield<24, 8> pid7;
    ump_bitfield<16, 8> pid8;
    ump_bitfield<8, 8> pid9;
    ump_bitfield<0, 8> pid10;
  };
  union word3 {
    UMP_MEMBERS(word3)
    ump_bitfield<24, 8> pid11;
    ump_bitfield<16, 8> pid12;
    ump_bitfield<8, 8> pid13;
    ump_bitfield<0, 8> pid14;
  };
  product_instance_id_notification() = default;
  explicit product_instance_id_notification(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(product_instance_id_notification const &, product_instance_id_notification const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.6 Selecting a MIDI Protocol and Jitter Reduction Timestamps for a UMP Stream
// 7.1.6.1 Steps to Select Protocol and Jitter Reduction Timestamps
// 7.1.6.2 JR Stream Configuration Request
struct jr_configuration_request {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::jr_configuration_request)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x05
    ump_bitfield<8, 8> protocol;
    ump_bitfield<2, 6> reserved0;
    ump_bitfield<1, 1> rxjr;
    ump_bitfield<0, 1> txjr;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  jr_configuration_request() = default;
  explicit jr_configuration_request(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(jr_configuration_request const &, jr_configuration_request const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.6.3 JR Stream Configuration Notification Message
struct jr_configuration_notification {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::jr_configuration_notification)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x06
    ump_bitfield<8, 8> protocol;
    ump_bitfield<2, 6> reserved0;
    ump_bitfield<1, 1> rxjr;
    ump_bitfield<0, 1> txjr;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  jr_configuration_notification() = default;
  explicit jr_configuration_notification(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(jr_configuration_notification const &, jr_configuration_notification const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.7 Function Block Discovery Message
struct function_block_discovery {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::function_block_discovery)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x10
    ump_bitfield<8, 8> block_num;
    ump_bitfield<0, 8> filter;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  function_block_discovery() = default;
  explicit function_block_discovery(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(function_block_discovery const &, function_block_discovery const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.8 Function Block Info Notification
struct function_block_info_notification {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::function_block_info_notification)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x11
    ump_bitfield<15, 1> block_active;
    ump_bitfield<8, 7> block_num;
    ump_bitfield<6, 2> reserved0;
    ump_bitfield<4, 2> ui_hint;
    ump_bitfield<2, 2> midi1;
    ump_bitfield<0, 2> direction;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<24, 8> first_group;
    ump_bitfield<16, 8> num_spanned;
    ump_bitfield<8, 8> ci_message_version;
    ump_bitfield<0, 8> max_sys8_streams;
  };
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  function_block_info_notification() = default;
  explicit function_block_info_notification(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(function_block_info_notification const &, function_block_info_notification const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.9 Function Block Name Notification
struct function_block_name_notification {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::function_block_name_notification)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x12
    ump_bitfield<8, 8> block_num;
    ump_bitfield<0, 8> name0;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<24, 8> name1;
    ump_bitfield<16, 8> name2;
    ump_bitfield<8, 8> name3;
    ump_bitfield<0, 8> name4;
  };
  union word2 {
    UMP_MEMBERS(word2)
    ump_bitfield<24, 8> name5;
    ump_bitfield<16, 8> name6;
    ump_bitfield<8, 8> name7;
    ump_bitfield<0, 8> name8;
  };
  union word3 {
    UMP_MEMBERS(word3)
    ump_bitfield<24, 8> name9;
    ump_bitfield<16, 8> name10;
    ump_bitfield<8, 8> name11;
    ump_bitfield<0, 8> name12;
  };

  function_block_name_notification() = default;
  explicit function_block_name_notification(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(function_block_name_notification const &, function_block_name_notification const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.10 Start of Clip Message
struct start_of_clip {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::start_of_clip)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x20
    ump_bitfield<0, 16> reserved0;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  start_of_clip() = default;
  explicit start_of_clip(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(start_of_clip const &, start_of_clip const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.1.11 End of Clip Message
struct end_of_clip {
  union word0 {
    UMP_MEMBERS0(word0, midi2::ump_stream::end_of_clip)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x21
    ump_bitfield<0, 16> reserved0;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  end_of_clip() = default;
  explicit end_of_clip(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(end_of_clip const &, end_of_clip const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

};  // end namespace ump_stream

//*   __ _              _      _         *
//*  / _| |_____ __  __| |__ _| |_ __ _  *
//* |  _| / -_) \ / / _` / _` |  _/ _` | *
//* |_| |_\___/_\_\ \__,_\__,_|\__\__,_| *
//*                                      *
namespace flex_data {

union flex_data_w0 {
  UMP_MEMBERS(flex_data_w0)
  ump_bitfield<28, 4> mt;  // 0x0D
  ump_bitfield<24, 4> group;
  ump_bitfield<22, 2> form;
  ump_bitfield<20, 2> addrs;
  ump_bitfield<16, 4> channel;
  ump_bitfield<8, 8> status_bank;
  ump_bitfield<0, 8> status;
};

// 7.5.3 Set Tempo Message
struct set_tempo {
  union word0 {
    UMP_MEMBERS0(word0, midi2::flex_data::set_tempo)
    ump_bitfield<28, 4> mt;  // 0x0D
    ump_bitfield<24, 4> group;
    ump_bitfield<22, 2> form;
    ump_bitfield<20, 2> addrs;
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> status_bank;
    ump_bitfield<0, 8> status;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  set_tempo() = default;
  explicit set_tempo(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(set_tempo const &, set_tempo const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.5.4 Set Time Signature Message
struct set_time_signature {
  union word0 {
    UMP_MEMBERS0(word0, midi2::flex_data::set_time_signature)
    ump_bitfield<28, 4> mt;  // 0x0D
    ump_bitfield<24, 4> group;
    ump_bitfield<22, 2> form;
    ump_bitfield<20, 2> addrs;
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> status_bank;
    ump_bitfield<0, 8> status;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<24, 8> numerator;
    ump_bitfield<16, 8> denominator;
    ump_bitfield<8, 8> number_of_32_notes;
    ump_bitfield<0, 8> reserved0;
  };
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  set_time_signature() = default;
  explicit set_time_signature(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(set_time_signature const &, set_time_signature const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.5.5 Set Metronome Message

struct set_metronome {
  union word0 {
    UMP_MEMBERS0(word0, midi2::flex_data::set_metronome)
    ump_bitfield<28, 4> mt;  // 0x0D
    ump_bitfield<24, 4> group;
    ump_bitfield<22, 2> form;
    ump_bitfield<20, 2> addrs;
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> status_bank;
    ump_bitfield<0, 8> status;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<24, 8> num_clocks_per_primary_click;
    ump_bitfield<16, 8> bar_accent_part_1;
    ump_bitfield<8, 8> bar_accent_part_2;
    ump_bitfield<0, 8> bar_accent_part_3;
  };
  union word2 {
    UMP_MEMBERS(word2)
    ump_bitfield<24, 8> num_subdivision_clicks_1;
    ump_bitfield<16, 8> num_subdivision_clicks_2;
    ump_bitfield<0, 16> reserved0;
  };
  using word3 = std::uint32_t;

  set_metronome() = default;
  explicit set_metronome(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(set_metronome const &, set_metronome const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.5.7 Set Key Signature Message
struct set_key_signature {
  union word0 {
    UMP_MEMBERS0(word0, midi2::flex_data::set_key_signature)
    ump_bitfield<28, 4> mt;  // 0x0D
    ump_bitfield<24, 4> group;
    ump_bitfield<22, 2> form;
    ump_bitfield<20, 2> addrs;
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> status_bank;
    ump_bitfield<0, 8> status;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<28, 4> sharps_flats;
    ump_bitfield<24, 4> tonic_note;
    ump_bitfield<0, 24> reserved0;
  };
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  set_key_signature() = default;
  explicit set_key_signature(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(set_key_signature const &, set_key_signature const &) = default;

  std::tuple<word0, word1, word2, word3> w;
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
  union word0 {
    UMP_MEMBERS0(word0, midi2::flex_data::set_chord_name)
    ump_bitfield<28, 4> mt;  // 0x0D
    ump_bitfield<24, 4> group;
    ump_bitfield<22, 2> form;
    ump_bitfield<20, 2> addrs;
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> status_bank;
    ump_bitfield<0, 8> status;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<28, 4> tonic_sharps_flats;  // 2's compl
    ump_bitfield<24, 4> chord_tonic;
    ump_bitfield<16, 8> chord_type;
    ump_bitfield<12, 4> alter_1_type;
    ump_bitfield<8, 4> alter_1_degree;
    ump_bitfield<4, 4> alter_2_type;
    ump_bitfield<0, 4> alter_2_degree;
  };
  union word2 {
    UMP_MEMBERS(word2)
    ump_bitfield<28, 4> alter_3_type;
    ump_bitfield<24, 4> alter_3_degree;
    ump_bitfield<20, 4> alter_4_type;
    ump_bitfield<16, 4> alter_4_degree;
    ump_bitfield<0, 16> reserved;  // 0x0000
  };
  union word3 {
    UMP_MEMBERS(word3)
    ump_bitfield<28, 4> bass_sharps_flats;  // 2's compl
    ump_bitfield<24, 4> bass_note;
    ump_bitfield<16, 8> bass_chord_type;
    ump_bitfield<12, 4> alter_1_type;
    ump_bitfield<8, 4> alter_1_degree;
    ump_bitfield<4, 4> alter_2_type;
    ump_bitfield<0, 4> alter_2_degree;
  };

  set_chord_name() = default;
  explicit set_chord_name(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(set_chord_name const &, set_chord_name const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

// 7.5.9 Text Messages Common Format
struct text_common {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // 0x0D
    ump_bitfield<24, 4> group;
    ump_bitfield<22, 2> form;
    ump_bitfield<20, 2> addrs;
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> status_bank;
    ump_bitfield<0, 8> status;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  text_common() = default;
  explicit text_common(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(text_common const &, text_common const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

}  // end namespace flex_data

//*     _      _          _ ___ ___  *
//*  __| |__ _| |_ __ _  / |_  | _ ) *
//* / _` / _` |  _/ _` | | |/ // _ \ *
//* \__,_\__,_|\__\__,_| |_/___\___/ *
//*                                  *
namespace data128 {

// 7.8 System Exclusive 8 (8-Bit) Messages

// SysEx8 in 1 UMP (word 1)
// SysEx8 Start (word 1)
// SysEx8 Continue (word 1)
// SysEx8 End (word 1)
namespace details {

template <midi2::data128 Status> struct sysex8 {
  union word0 {
    UMP_MEMBERS0(word0, Status)
    ump_bitfield<28, 4> mt;  ///< Always 0x05
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;
    ump_bitfield<16, 4> number_of_bytes;
    ump_bitfield<8, 8> stream_id;
    ump_bitfield<0, 8> data0;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<24, 8> data1;
    ump_bitfield<16, 8> data2;
    ump_bitfield<8, 8> data3;
    ump_bitfield<0, 8> data4;
  };
  union word2 {
    UMP_MEMBERS(word2)
    ump_bitfield<24, 8> data5;
    ump_bitfield<16, 8> data6;
    ump_bitfield<8, 8> data7;
    ump_bitfield<0, 8> data8;
  };
  union word3 {
    UMP_MEMBERS(word3)
    ump_bitfield<24, 8> data9;
    ump_bitfield<16, 8> data10;
    ump_bitfield<8, 8> data11;
    ump_bitfield<0, 8> data12;
  };

  sysex8() = default;
  explicit sysex8(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(sysex8 const &, sysex8 const &) = default;

  std::tuple<word0, word1, word2, word3> w;
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
  union word0 {
    UMP_MEMBERS0(word0, midi2::data128::mixed_data_set_header)
    ump_bitfield<28, 4> mt;  ///< Always 0x05
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0x08
    ump_bitfield<16, 4> mds_id;
    ump_bitfield<0, 16> bytes_in_chunk;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<16, 16> chunks_in_mds;
    ump_bitfield<0, 16> chunk_num;
  };
  union word2 {
    UMP_MEMBERS(word2)
    ump_bitfield<16, 16> manufacturer_id;
    ump_bitfield<0, 16> device_id;
  };
  union word3 {
    UMP_MEMBERS(word3)
    ump_bitfield<16, 16> sub_id_1;
    ump_bitfield<0, 16> sub_id_2;
  };

  mds_header() = default;
  explicit mds_header(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(mds_header const &, mds_header const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

struct mds_payload {
  union word0 {
    UMP_MEMBERS0(word0, midi2::data128::mixed_data_set_payload)
    ump_bitfield<28, 4> mt;  ///< Always 0x05
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0x09
    ump_bitfield<16, 4> mds_id;
    ump_bitfield<0, 16> data0;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  mds_payload() = default;
  explicit mds_payload(std::span<std::uint32_t, 4> m) : w{m[0], m[1], m[2], m[3]} {}
  friend bool operator==(mds_payload const &, mds_payload const &) = default;

  std::tuple<word0, word1, word2, word3> w;
};

}  // end namespace data128

}  // end namespace midi2::types

// NOLINTEND(cppcoreguidelines-pro-type-member-init,cppcoreguidelines-prefer-member-initializer,hicpp-member-init)

#undef UMP_MEMBERS

#endif  // MIDI2_UMP_TYPES_HPP
