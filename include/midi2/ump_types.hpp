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

#include "midi2/bitfield.hpp"

namespace midi2::types {

template <unsigned Index, unsigned Bits> using ump_bitfield = bitfield<std::uint32_t, Index, Bits>;

// F.1.1 Message Type 0x0: Utility
// Table 26 4-Byte UMP Formats for Message Type 0x0: Utility

// NOOP
union noop {
  ump_bitfield<28, 4> mt;  // 0x0
  ump_bitfield<24, 4> reserved;
  ump_bitfield<20, 4> status;  // 0b0000
  ump_bitfield<0, 20> data;    // 0b0000'00000000'00000000
};

// JR Clock
// JR Timestamp
// Delta Clockstamp Ticks Per Quarter Note
union jr_clock {
  friend constexpr bool operator==(jr_clock const& a, jr_clock const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }
  ump_bitfield<28, 4> mt;  // 0x0
  ump_bitfield<24, 4> reserved1;
  ump_bitfield<20, 4> status;  // 0b0001
  ump_bitfield<16, 4> reserved2;
  ump_bitfield<0, 16> sender_clock_time;
};

// Delta Clockstamp
union delta_clockstamp {
  friend constexpr bool operator==(delta_clockstamp const& a, delta_clockstamp const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }
  ump_bitfield<28, 4> mt;  // 0x0
  ump_bitfield<24, 4> reserved;
  ump_bitfield<20, 4> status;  // 0b0100
  ump_bitfield<0, 20> ticks_per_quarter_note;
};

// F.1.2 Message Type 0x1: System Common & System Real Time
// Table 27 4-Byte UMP Formats for Message Type 0x1: System Common & System Real
// Time
union system_general {
  friend constexpr bool operator==(system_general const& a, system_general const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }
  ump_bitfield<28, 4> mt;  // 0x1
  ump_bitfield<24, 4> group;
  ump_bitfield<16, 8> status;  // 0xF0..0xFF
  ump_bitfield<8, 8> byte2;
  ump_bitfield<0, 8> byte3;
};

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
  friend constexpr bool operator==(m1cvm_w0 const& a, m1cvm_w0 const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }
  ump_bitfield<28, 4> mt;  // 0x2
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  // 0b1000..0b1110
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved0;
  ump_bitfield<8, 7> data_a;
  ump_bitfield<7, 1> reserved1;
  ump_bitfield<0, 7> data_b;
};

// F.2.1 Message Type 0x3: 8-Byte Data Messages
// Table 29 8-Byte UMP Formats for Message Type 0x3: 8-Byte Data Messages

union sysex7_w0 {
  ump_bitfield<28, 4> mt;  ///< Always 0x3
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  // 0b0000..0b0011
  ump_bitfield<16, 4> number_of_bytes;
  ump_bitfield<8, 8> data0;
  ump_bitfield<0, 8> data1;
};

// F.2.2 Message Type 0x4: MIDI 2.0 Channel Voice Messages
// Table 30 8-Byte UMP Formats for Message Type 0x4: MIDI 2.0 Channel Voice Messages
namespace m2cvm {
// 7.4.1 MODI 2.0 Note Off Message
// 7.4.2 MIDI 2.0 Note On Message
union note_w0 {
  friend constexpr bool operator==(note_w0 const& a, note_w0 const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }

  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Note-off=0x8, note-on=0x9
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved;
  ump_bitfield<8, 7> note;
  ump_bitfield<0, 8> attribute;
};
union note_w1 {
  friend constexpr bool operator==(note_w1 const& a, note_w1 const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }
  ump_bitfield<16, 16> velocity;
  ump_bitfield<0, 16> attribute;
};

// 7.4.3 MIDI 2.0 Poly Pressure Message
union poly_pressure_w0 {
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Always 0xA
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved0;
  ump_bitfield<8, 7> note;
  ump_bitfield<0, 8> reserved1;
};

// 7.4.4 MIDI 2.0 Registered Per-Note Controller and Assignable Per-Note Controller Messages
union controller_w0 {
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Registered Controller=0x0, Assignable Controller=0x1
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved;
  ump_bitfield<8, 7> note;
  ump_bitfield<0, 8> index;
};

// 7.4.5 MIDI 2.0 Per-Note Management Message
union per_note_management_w0 {
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

// 7.4.6 MIDI 2.0 Control Change Message
union control_change_w0 {
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Always 0xB
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved;
  ump_bitfield<8, 7> note;
};

// 7.4.7 MIDI 2.0 Registered Controller (RPN) and Assignable Controller (NRPN) Message
// 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) and Assignable Controller (NRPN) Message
union controller_message_w0 {
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Absolute RPN=0x2, NRPN=0x3; relative RPN=0x4, NRPN=0x5
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved0;
  ump_bitfield<8, 7> bank;
  ump_bitfield<7, 1> reserved1;
  ump_bitfield<0, 7> index;
};

// 7.4.9 MIDI 2.0 Program Change Message
union program_change_w0 {
  friend constexpr bool operator==(program_change_w0 const& a, program_change_w0 const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }

  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Always 0xC
  ump_bitfield<16, 4> channel;
  ump_bitfield<8, 8> reserved;
  ump_bitfield<1, 7> option_flags;  ///< Reserved option flags
  ump_bitfield<0, 1> bank_valid;    ///< Bank change is ignored if this bit is zero.
};
union program_change_w1 {
  friend constexpr bool operator==(program_change_w1 const& a, program_change_w1 const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }
  ump_bitfield<24, 8> program;
  ump_bitfield<16, 8> reserved;
  ump_bitfield<15, 1> r0;  // reserved
  ump_bitfield<8, 7> bank_msb;
  ump_bitfield<7, 1> r1;  // reserved
  ump_bitfield<0, 7> bank_lsb;
};

// 7.4.10 MIDI 2.0 Channel Pressure Message
union channel_pressure_w0 {
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Always 0xA
  ump_bitfield<16, 4> channel;
  ump_bitfield<8, 8> reserved0;
  ump_bitfield<0, 8> reserved1;
};

// 7.4.11 MIDI 2.0 Pitch Bend Message
union pitch_bend_w0 {
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Always 0xE
  ump_bitfield<16, 4> channel;
  ump_bitfield<8, 8> reserved0;
  ump_bitfield<0, 8> reserved1;
};

// 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
union per_note_pitch_bend_w0 {
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Always 0x6
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved0;
  ump_bitfield<8, 7> note;
  ump_bitfield<0, 8> reserved1;
};

}  // end namespace m2cvm

namespace ump_stream {

// 7.1.1 Endpoint Discovery Message
union endpoint_discovery_w0 {
  friend constexpr bool operator==(endpoint_discovery_w0 const& a, endpoint_discovery_w0 const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }
  ump_bitfield<28, 4> mt;
  ump_bitfield<26, 2> format;
  ump_bitfield<16, 10> status;
  ump_bitfield<8, 8> version_major;
  ump_bitfield<0, 8> version_minor;
};
union endpoint_discovery_w1 {
  friend constexpr bool operator==(endpoint_discovery_w1 const& a, endpoint_discovery_w1 const& b) {
    return std::bit_cast<std::uint32_t>(a) == std::bit_cast<std::uint32_t>(b);
  }
  ump_bitfield<8, 24> reserved;
  ump_bitfield<0, 8> filter;
};
using endpoint_discovery_w2 = std::uint32_t;
using endpoint_discovery_w3 = std::uint32_t;

};  // end namespace ump_stream

// F.3.1 Message Type 0x5: 16-byte Data Messages (System Exclusive 8 and Mixed
// Data Set) Table 31 16-Byte UMP Formats for Message Type 0x5: System Exclusive
// 8 and Mixed Data Set

// SysEx8 in 1 UMP (word 1)
// SysEx8 Start (word 1)
// SysEx8 Continue (word 1)
// SysEx8 End (word 1)
union sysex8_w0 {
  ump_bitfield<28, 4> mt;
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;
  ump_bitfield<16, 4> number_of_bytes;
  ump_bitfield<8, 8> stream_id;
  ump_bitfield<0, 8> data;
};

// Mixed Data Set Header (word 1)
// Mixed Data Set Payload (word 1)
union mixed_data_set_w0 {
  ump_bitfield<28, 4> mt;
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;
  ump_bitfield<16, 4> mds_id;
  ump_bitfield<0, 16> data;
};

// F.3.1 Message Type 0xD: Flex Data Messages
// Table 32 128 bit UMP Formats for Message Type 0xD: Flex Data Messages

// Set Chord Name
union set_chord_name_w0 {
  ump_bitfield<28, 4> mt;  // 0xD
  ump_bitfield<24, 4> group;
  ump_bitfield<22, 2> format;  // 0x0
  ump_bitfield<20, 2> addrs;
  ump_bitfield<16, 4> channel;
  ump_bitfield<8, 8> status_bank;  // 0x00
  ump_bitfield<0, 8> status;       // 0x06
};
union set_chord_name_w1 {
  ump_bitfield<28, 4> tonic_sharps_flats;  // 2's compl
  ump_bitfield<24, 4> chord_tonic;
  ump_bitfield<16, 8> chord_type;
  ump_bitfield<12, 4> alter_1_type;
  ump_bitfield<8, 4> alter_1_degree;
  ump_bitfield<4, 4> alter_2_type;
  ump_bitfield<0, 4> alter_2_degree;
};
union set_chord_name_w2 {
  ump_bitfield<28, 4> alter_3_type;
  ump_bitfield<24, 4> alter_3_degree;
  ump_bitfield<20, 4> alter_4_type;
  ump_bitfield<16, 4> alter_4_degree;
  ump_bitfield<0, 16> reserved;  // 0x0000
};
union set_chord_name_w3 {
  ump_bitfield<28, 4> bass_sharps_flats;  // 2's compl
  ump_bitfield<24, 4> bass_note;
  ump_bitfield<16, 8> bass_chord_type;
  ump_bitfield<12, 4> alter_1_type;
  ump_bitfield<8, 4> alter_1_degree;
  ump_bitfield<4, 4> alter_2_type;
  ump_bitfield<0, 4> alter_2_degree;
};

// F.3.2 Message Type 0xF: UMP Stream Messages
// Table 33 128 bit UMP Formats for Message Type 0xF: UMP Stream Messages

// Function Block Info Notification (word 1)
union function_block_info_w0 {
  ump_bitfield<28, 4> mt;
  ump_bitfield<26, 2> format;
  ump_bitfield<16, 10> status;
  ump_bitfield<15, 1> a;
  ump_bitfield<8, 7> block_number;
  ump_bitfield<6, 2> reserv;
  ump_bitfield<4, 2> ui_hint;
  ump_bitfield<2, 2> m1;
  ump_bitfield<0, 2> dir;
};
/// Function Block Info Notification (word 2)
union function_block_info_w1 {
  ump_bitfield<24, 8> first_group;
  ump_bitfield<16, 8> groups_spanned;
  ump_bitfield<8, 8> message_version;
  ump_bitfield<0, 8> num_sysex8_streams;
};

// Function Block Name Notification (word1)
union function_block_name_w0 {
  ump_bitfield<28, 4> mt;
  ump_bitfield<26, 2> format;
  ump_bitfield<16, 10> status;
  ump_bitfield<8, 8> block_number;
  ump_bitfield<0, 8> name;
};

}  // end namespace midi2::types

#endif  // MIDI2_UMP_TYPES_HPP
