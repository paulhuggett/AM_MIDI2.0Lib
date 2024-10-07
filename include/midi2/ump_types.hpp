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

#include "midi2/bitfield.hpp"

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

namespace midi2::types {

template <unsigned Index, unsigned Bits> using ump_bitfield = bitfield<std::uint32_t, Index, Bits>;

// F.1.1 Message Type 0x0: Utility
// Table 26 4-Byte UMP Formats for Message Type 0x0: Utility

// NOOP
union noop {
  UMP_MEMBERS(noop)
  ump_bitfield<28, 4> mt;  // 0x0
  ump_bitfield<24, 4> reserved;
  ump_bitfield<20, 4> status;  // 0b0000
  ump_bitfield<0, 20> data;    // 0b0000'00000000'00000000
};

// JR Clock
// JR Timestamp
// Delta Clockstamp Ticks Per Quarter Note
union jr_clock {
  UMP_MEMBERS(jr_clock)
  ump_bitfield<28, 4> mt;  // 0x0
  ump_bitfield<24, 4> reserved1;
  ump_bitfield<20, 4> status;  // 0b0001
  ump_bitfield<16, 4> reserved2;
  ump_bitfield<0, 16> sender_clock_time;
};

// Delta Clockstamp
union delta_clockstamp {
  UMP_MEMBERS(delta_clockstamp)
  ump_bitfield<28, 4> mt;  // 0x0
  ump_bitfield<24, 4> reserved;
  ump_bitfield<20, 4> status;  // 0b0100
  ump_bitfield<0, 20> ticks_per_quarter_note;
};

// F.1.2 Message Type 0x1: System Common & System Real Time
// Table 27 4-Byte UMP Formats for Message Type 0x1: System Common & System Real
// Time
union system_general {
  UMP_MEMBERS(system_general)
  ump_bitfield<28, 4> mt;  // 0x1
  ump_bitfield<24, 4> group;
  ump_bitfield<16, 8> status;  // 0xF0..0xFF
  ump_bitfield<8, 8> byte2;
  ump_bitfield<0, 8> byte3;
};

//*             _              *
//*  ____  _ __| |_ ___ _ __   *
//* (_-< || (_-<  _/ -_) '  \  *
//* /__/\_, /__/\__\___|_|_|_| *
//*     |__/                   *
namespace system {

struct midi_time_code {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xF1
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> time_code;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> reeserved2;
  };
  midi_time_code() = default;
  midi_time_code(midi_time_code const &) = default;
  explicit midi_time_code(word0 const w0_) : w0{w0_} {}
  explicit midi_time_code(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(midi_time_code const &, midi_time_code const &) = default;
  word0 w0{};
};
struct song_position_pointer {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xF2
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> position_lsb;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> position_msb;
  };
  song_position_pointer() = default;
  song_position_pointer(song_position_pointer const &) = default;
  explicit song_position_pointer(word0 const w0_) : w0{w0_} {}
  explicit song_position_pointer(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(song_position_pointer const &, song_position_pointer const &) = default;
  word0 w0{};
};
struct song_select {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xF3
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> song;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> reserved2;
  };
  song_select() = default;
  song_select(song_select const &) = default;
  explicit song_select(word0 const w0_) : w0{w0_} {}
  explicit song_select(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(song_select const &, song_select const &) = default;
  word0 w0{};
};
struct tune_request {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xF6
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  tune_request() = default;
  tune_request(tune_request const &) = default;
  explicit tune_request(word0 const w0_) : w0{w0_} {}
  explicit tune_request(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(tune_request const &, tune_request const &) = default;
  word0 w0{};
};
struct timing_clock {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xF8
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  timing_clock() = default;
  timing_clock(timing_clock const &) = default;
  explicit timing_clock(word0 const w0_) : w0{w0_} {}
  explicit timing_clock(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(timing_clock const &, timing_clock const &) = default;
  word0 w0{};
};
struct seq_start {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xFA
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  seq_start() = default;
  seq_start(seq_start const &) = default;
  explicit seq_start(word0 const w0_) : w0{w0_} {}
  explicit seq_start(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(seq_start const &, seq_start const &) = default;
  word0 w0{};
};
struct seq_continue {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xFB
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  seq_continue() = default;
  seq_continue(seq_continue const &) = default;
  explicit seq_continue(word0 const w0_) : w0{w0_} {}
  explicit seq_continue(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(seq_continue const &, seq_continue const &) = default;
  word0 w0{};
};
struct seq_stop {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xFC
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  seq_stop() = default;
  seq_stop(seq_stop const &) = default;
  explicit seq_stop(word0 const w0_) : w0{w0_} {}
  explicit seq_stop(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(seq_stop const &, seq_stop const &) = default;
  word0 w0{};
};
struct active_sensing {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xFE
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  active_sensing() = default;
  active_sensing(active_sensing const &) = default;
  explicit active_sensing(word0 const w0_) : w0{w0_} {}
  explicit active_sensing(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(active_sensing const &, active_sensing const &) = default;
  word0 w0{};
};
struct reset {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x1
    ump_bitfield<24, 4> group;
    ump_bitfield<16, 8> status;  // Always 0xFF
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  reset() = default;
  reset(reset const &) = default;
  explicit reset(word0 const w0_) : w0{w0_} {}
  explicit reset(std::uint32_t const w0_) : w0{w0_} {}
  friend bool operator==(reset const &, reset const &) = default;
  word0 w0{};
};

}  // end namespace system

// F.1.3 Mess Type 0x2: MIDI 1.0 Channel Voice Messages
// Table 28 4-Byte UMP Formats for Message Type 0x2: MIDI 1.0 Channel Voice
// Messages

// Note Off
// Note On
// Poly Pressure
// Control Change
// Program Change
// Channel Pressure
// Pitch Bend
union m1cvm_w0 {
  UMP_MEMBERS(m1cvm_w0)
  ump_bitfield<28, 4> mt;  // 0x2
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  // 0b1000..0b1110
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved0;
  ump_bitfield<8, 7> data_a;
  ump_bitfield<7, 1> reserved1;
  ump_bitfield<0, 7> data_b;
};

//*     _      _         __ _ _   *
//*  __| |__ _| |_ __ _ / /| | |  *
//* / _` / _` |  _/ _` / _ \_  _| *
//* \__,_\__,_|\__\__,_\___/ |_|  *
//*                               *
namespace data64 {

// 7.7 System Exclusive (7-Bit) Messages
struct sysex7 {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  ///< Always 0x3
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  // 0b0000..0b0011
    ump_bitfield<16, 4> number_of_bytes;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> data0;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> data1;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<31, 1> reserved0;
    ump_bitfield<24, 7> data2;
    ump_bitfield<23, 1> reserved1;
    ump_bitfield<16, 7> data3;
    ump_bitfield<15, 1> reserved2;
    ump_bitfield<8, 7> data4;
    ump_bitfield<7, 1> reserved3;
    ump_bitfield<0, 7> data5;
  };

  sysex7() = default;
  sysex7(sysex7 const &) = default;
  explicit sysex7(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}

  friend bool operator==(sysex7 const &, sysex7 const &) = default;
  word0 w0{};
  word1 w1{};
};

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
// 7.4.2 MIDI 2.0 Note On Message
struct note {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Note-off=0x8, note-on=0x9
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> note;
    ump_bitfield<0, 8> attribute;
  };
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<16, 16> velocity;
    ump_bitfield<0, 16> attribute;
  };
  note() = default;
  note(note const &) = default;
  explicit note(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(note const &a, note const &b) = default;
  word0 w0{};
  word1 w1{};
};

// 7.4.3 MIDI 2.0 Poly Pressure Message
struct poly_pressure {
  union word0 {
    UMP_MEMBERS(word0)
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
  poly_pressure(poly_pressure const &) = default;
  explicit poly_pressure(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(poly_pressure const &a, poly_pressure const &b) = default;
  word0 w0{};
  word1 w1{};
};

// 7.4.4 MIDI 2.0 Registered Per-Note Controller and Assignable Per-Note Controller Messages
struct per_note_controller {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Registered Controller=0x0, Assignable Controller=0x1
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved;
    ump_bitfield<8, 7> note;
    ump_bitfield<0, 8> index;
  };
  using word1 = std::uint32_t;

  per_note_controller() = default;
  per_note_controller(per_note_controller const &) = default;
  explicit per_note_controller(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(per_note_controller const &a, per_note_controller const &b) = default;

  word0 w0{};
  word1 w1{};
};

// 7.4.5 MIDI 2.0 Per-Note Management Message
struct per_note_management {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved;
    ump_bitfield<8, 7> note;
    ump_bitfield<0, 1> option_flags;
    ump_bitfield<1, 1> detach;  ///< Detach per-note controllers from previously received note(s)
    ump_bitfield<0, 1> set;     ///< Reset (set) per-note controllers to default values
  };
  using word1 = std::uint32_t;

  per_note_management() = default;
  per_note_management(per_note_management const &) = default;
  explicit per_note_management(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(per_note_management const &a, per_note_management const &b) = default;

  word0 w0{};
  word1 w1{};
};

// 7.4.6 MIDI 2.0 Control Change Message
struct control_change {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0xB
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved;
    ump_bitfield<8, 7> note;
  };
  using word1 = std::uint32_t;

  control_change() = default;
  control_change(control_change const &) = default;
  explicit control_change(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(control_change const &a, control_change const &b) = default;

  word0 w0{};
  word1 w1{};
};

// 7.4.7 MIDI 2.0 Registered Controller (RPN) and Assignable Controller (NRPN) Message
// 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) and Assignable Controller (NRPN) Message
struct controller_message {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Absolute RPN=0x2, NRPN=0x3; relative RPN=0x4, NRPN=0x5
    ump_bitfield<16, 4> channel;
    ump_bitfield<15, 1> reserved0;
    ump_bitfield<8, 7> bank;
    ump_bitfield<7, 1> reserved1;
    ump_bitfield<0, 7> index;
  };
  using word1 = std::uint32_t;
  controller_message() = default;
  controller_message(controller_message const &) = default;
  explicit controller_message(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(controller_message const &a, controller_message const &b) = default;
  word0 w0{};
  word1 w1{};
};

// 7.4.9 MIDI 2.0 Program Change Message
struct program_change {
  union word0 {
    UMP_MEMBERS(word0)
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
  program_change(program_change const &) = default;
  explicit program_change(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(program_change const &a, program_change const &b) = default;
  word0 w0{};
  word1 w1{};
};

// 7.4.10 MIDI 2.0 Channel Pressure Message
struct channel_pressure {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0xD
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  using word1 = std::uint32_t;

  channel_pressure() = default;
  channel_pressure(channel_pressure const &) = default;
  explicit channel_pressure(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(channel_pressure const &a, channel_pressure const &b) = default;

  word0 w0{};
  word1 w1{};
};

// 7.4.11 MIDI 2.0 Pitch Bend Message
struct pitch_bend {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  ///< Always 0x4
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  ///< Always 0xE
    ump_bitfield<16, 4> channel;
    ump_bitfield<8, 8> reserved0;
    ump_bitfield<0, 8> reserved1;
  };
  using word1 = std::uint32_t;

  pitch_bend() = default;
  pitch_bend(pitch_bend const &) = default;
  explicit pitch_bend(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(pitch_bend const &a, pitch_bend const &b) = default;

  word0 w0{};
  word1 w1{};
};

// 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
struct per_note_pitch_bend {
  union word0 {
    UMP_MEMBERS(word0)
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
  per_note_pitch_bend(per_note_pitch_bend const &) = default;
  explicit per_note_pitch_bend(std::span<std::uint32_t, 2> m) : w0{m[0]}, w1{m[1]} {}
  friend constexpr bool operator==(per_note_pitch_bend const &a, per_note_pitch_bend const &b) = default;

  word0 w0{};
  word1 w1{};
};

}  // end namespace m2cvm

namespace ump_stream {

// 7.1.1 Endpoint Discovery Message
struct endpoint_discovery {
  union word0 {
    UMP_MEMBERS(word0)
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
  endpoint_discovery(endpoint_discovery const &) = default;
  explicit endpoint_discovery(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(endpoint_discovery const &lhs, endpoint_discovery const &rhs) = default;
  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.2 Endpoint Info Notification Message
struct endpoint_info_notification {
  union word0 {
    UMP_MEMBERS(word0)
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
  endpoint_info_notification(endpoint_info_notification const &) = default;
  explicit endpoint_info_notification(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(endpoint_info_notification const &, endpoint_info_notification const &) = default;
  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.3 Device Identity Notification Message
struct device_identity_notification {
  union word0 {
    UMP_MEMBERS(word0)
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
  device_identity_notification(device_identity_notification const &) = default;
  explicit device_identity_notification(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(device_identity_notification const &, device_identity_notification const &) = default;
  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.4 Endpoint Name Notification
struct endpoint_name_notification {
  union word0 {
    UMP_MEMBERS(word0)
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
  endpoint_name_notification(endpoint_name_notification const &) = default;
  explicit endpoint_name_notification(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(endpoint_name_notification const &, endpoint_name_notification const &) = default;
  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.5 Product Instance Id Notification Message
struct product_instance_id_notification {
  union word0 {
    UMP_MEMBERS(word0)
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
  product_instance_id_notification(product_instance_id_notification const &) = default;
  explicit product_instance_id_notification(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(product_instance_id_notification const &, product_instance_id_notification const &) = default;
  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.6 Selecting a MIDI Protocol and Jitter Reduction Timestamps for a UMP Stream
// 7.1.6.1 Steps to Select Protocol and Jitter Reduction Timestamps
// 7.1.6.2 JR Stream Configuration Request
struct jr_configuration_request {
  union word0 {
    UMP_MEMBERS(word0)
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
  jr_configuration_request(jr_configuration_request const &) = default;
  explicit jr_configuration_request(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(jr_configuration_request const &, jr_configuration_request const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.6.3 JR Stream Configuration Notification Message
struct jr_configuration_notification {
  union word0 {
    UMP_MEMBERS(word0)
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
  jr_configuration_notification(jr_configuration_notification const &) = default;
  explicit jr_configuration_notification(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(jr_configuration_notification const &, jr_configuration_notification const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.7 Function Block Discovery Message
struct function_block_discovery {
  union word0 {
    UMP_MEMBERS(word0)
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
  function_block_discovery(function_block_discovery const &) = default;
  explicit function_block_discovery(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(function_block_discovery const &, function_block_discovery const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.8 Function Block Info Notification
struct function_block_info_notification {
  union word0 {
    UMP_MEMBERS(word0)
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
  function_block_info_notification(function_block_info_notification const &) = default;
  explicit function_block_info_notification(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(function_block_info_notification const &, function_block_info_notification const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.9 Function Block Name Notification
struct function_block_name_notification {
  union word0 {
    UMP_MEMBERS(word0)
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
  function_block_name_notification(function_block_name_notification const &) = default;
  explicit function_block_name_notification(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(function_block_name_notification const &, function_block_name_notification const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.10 Start of Clip Message
struct start_of_clip {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x20
    ump_bitfield<0, 16> reserved0;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  start_of_clip() = default;
  start_of_clip(start_of_clip const &) = default;
  explicit start_of_clip(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(start_of_clip const &, start_of_clip const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.1.11 End of Clip Message
struct end_of_clip {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;       // 0x0F
    ump_bitfield<26, 2> format;   // 0x00
    ump_bitfield<16, 10> status;  // 0x21
    ump_bitfield<0, 16> reserved0;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  end_of_clip() = default;
  end_of_clip(end_of_clip const &) = default;
  explicit end_of_clip(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(end_of_clip const &, end_of_clip const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

};  // end namespace ump_stream

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
  using word0 = flex_data_w0;
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  set_tempo() = default;
  set_tempo(set_tempo const &) = default;
  explicit set_tempo(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(set_tempo const &, set_tempo const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.5.4 Set Time Signature Message
struct set_time_signature {
  using word0 = flex_data_w0;
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
  set_time_signature(set_time_signature const &) = default;
  explicit set_time_signature(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(set_time_signature const &, set_time_signature const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.5.5 Set Metronome Message

struct set_metronome {
  using word0 = flex_data_w0;
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
  set_metronome(set_metronome const &) = default;
  explicit set_metronome(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(set_metronome const &, set_metronome const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.5.7 Set Key Signature Message
struct set_key_signature {
  using word0 = flex_data_w0;
  union word1 {
    UMP_MEMBERS(word1)
    ump_bitfield<28, 4> sharps_flats;
    ump_bitfield<24, 4> tonic_note;
    ump_bitfield<0, 24> reserved0;
  };
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  set_key_signature() = default;
  set_key_signature(set_key_signature const &) = default;
  explicit set_key_signature(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(set_key_signature const &, set_key_signature const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
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
  using word0 = flex_data_w0;
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
  set_chord_name(set_chord_name const &) = default;
  explicit set_chord_name(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(set_chord_name const &, set_chord_name const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.5.9 Text Messages Common Format
struct text_common {
  using word0 = flex_data_w0;
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  text_common() = default;
  text_common(text_common const &) = default;
  explicit text_common(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(text_common const &, text_common const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

}  // end namespace flex_data

namespace data128 {

// F.3.1 Message Type 0x5: 16-byte Data Messages (System Exclusive 8 and Mixed
// Data Set) Table 31 16-Byte UMP Formats for Message Type 0x5: System Exclusive
// 8 and Mixed Data Set

// SysEx8 in 1 UMP (word 1)
// SysEx8 Start (word 1)
// SysEx8 Continue (word 1)
// SysEx8 End (word 1)
struct sysex8 {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x05
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
  sysex8(sysex8 const &) = default;
  explicit sysex8(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(sysex8 const &, sysex8 const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

// 7.9 Mixed Data Set Message
// Mixed Data Set Header (word 1)
// Mixed Data Set Payload (word 1)

struct mds_header {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x05
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  // Always 0x08
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
  mds_header(mds_header const &) = default;
  explicit mds_header(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(mds_header const &, mds_header const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

struct mds_payload {
  union word0 {
    UMP_MEMBERS(word0)
    ump_bitfield<28, 4> mt;  // Always 0x05
    ump_bitfield<24, 4> group;
    ump_bitfield<20, 4> status;  // Always 0x09
    ump_bitfield<16, 4> mds_id;
    ump_bitfield<0, 16> data0;
  };
  using word1 = std::uint32_t;
  using word2 = std::uint32_t;
  using word3 = std::uint32_t;

  mds_payload() = default;
  mds_payload(mds_payload const &) = default;
  explicit mds_payload(std::span<std::uint32_t, 4> m) : w0{m[0]}, w1{m[1]}, w2{m[2]}, w3{m[3]} {}
  friend bool operator==(mds_payload const &, mds_payload const &) = default;

  word0 w0{};
  word1 w1{};
  word2 w2{};
  word3 w3{};
};

}  // end namespace data128

}  // end namespace midi2::types

#undef UMP_MEMBERS

#endif  // MIDI2_UMP_TYPES_HPP
