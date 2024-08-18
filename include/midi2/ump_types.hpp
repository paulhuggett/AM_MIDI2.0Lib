#ifndef MIDI2_UMP_TYPES_HPP
#define MIDI2_UMP_TYPES_HPP

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
  ump_bitfield<28, 4> mt;  // 0x0
  ump_bitfield<24, 4> reserved1;
  ump_bitfield<20, 4> status;  // 0b0001
  ump_bitfield<16, 4> reserved2;
  ump_bitfield<0, 16> sender_clock_time;
};

// Delta Clockstamp
union delta_clockstamp {
  ump_bitfield<28, 4> mt;  // 0x0
  ump_bitfield<24, 4> reserved;
  ump_bitfield<20, 4> status;  // 0b0100
  ump_bitfield<0, 20> ticks_per_quarter_note;
};

// F.1.2 Message Type 0x1: System Common & System Real Time
// Table 27 4-Byte UMP Formats for Message Type 0x1: System Common & System Real
// Time

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
union m1cvm_w1 {
  ump_bitfield<28, 4> mt;  // 0x2
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  // 0b1000..0b1110
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved1;
  ump_bitfield<8, 7> byte_a;
  ump_bitfield<7, 1> reserved2;
  ump_bitfield<0, 7> byte_b;
};

// F.2.1 Message Type 0x3: 8-Byte Data Messages
// Table 29 8-Byte UMP Formats for Message Type 0x3: 8-Byte Data Messages

union sysex7_w1 {
  ump_bitfield<28, 4> mt;  // 0x3
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  // 0b0000..0b0011
  ump_bitfield<16, 4> number_of_bytes;
  ump_bitfield<8, 8> data0;
  ump_bitfield<0, 8> data1;
};

// F.3.1 Message Type 0x5: 16-byte Data Messages (System Exclusive 8 and Mixed
// Data Set) Table 31 16-Byte UMP Formats for Message Type 0x5: System Exclusive
// 8 and Mixed Data Set

// SysEx8 in 1 UMP (word 1)
// SysEx8 Start (word 1)
// SysEx8 Continue (word 1)
// SysEx8 End (word 1)
union sysex8_w1 {
  ump_bitfield<28, 4> mt;
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;
  ump_bitfield<16, 4> number_of_bytes;
  ump_bitfield<8, 8> stream_id;
  ump_bitfield<0, 8> data;
};

// Mixed Data Set Header (word 1)
// Mixed Data Set Payload (word 1)
union mixed_data_set_w1 {
  ump_bitfield<28, 4> mt;
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;
  ump_bitfield<16, 4> mds_id;
  ump_bitfield<0, 16> data;
};

// F.3.1 Message Type 0xD: Flex Data Messages
// Table 32 128 bit UMP Formats for Message Type 0xD: Flex Data Messages

// Set Chord Name
union set_chord_name_w1 {
  ump_bitfield<28, 4> mt;  // 0xD
  ump_bitfield<24, 4> group;
  ump_bitfield<22, 2> format;  // 0x0
  ump_bitfield<20, 2> addrs;
  ump_bitfield<16, 4> channel;
  ump_bitfield<8, 8> status_bank;  // 0x00
  ump_bitfield<0, 8> status;       // 0x06
};
union set_chord_name_w2 {
  ump_bitfield<28, 4> tonic_sharps_flats;  // 2's compl
  ump_bitfield<24, 4> chord_tonic;
  ump_bitfield<16, 8> chord_type;
  ump_bitfield<12, 4> alter_1_type;
  ump_bitfield<8, 4> alter_1_degree;
  ump_bitfield<4, 4> alter_2_type;
  ump_bitfield<0, 4> alter_2_degree;
};
union set_chord_name_w3 {
  ump_bitfield<28, 4> alter_3_type;
  ump_bitfield<24, 4> alter_3_degree;
  ump_bitfield<20, 4> alter_4_type;
  ump_bitfield<16, 4> alter_4_degree;
  ump_bitfield<0, 16> reserved;  // 0x0000
};
union set_chord_name_w4 {
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
union function_block_info_w1 {
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
union function_block_info_w2 {
  ump_bitfield<24, 8> first_group;
  ump_bitfield<16, 8> groups_spanned;
  ump_bitfield<8, 8> message_version;
  ump_bitfield<0, 8> num_sysex8_streams;
};

// Function Block Name Notification (word1)
union function_block_name_w1 {
  ump_bitfield<28, 4> mt;
  ump_bitfield<26, 2> format;
  ump_bitfield<16, 10> status;
  ump_bitfield<8, 8> block_number;
  ump_bitfield<0, 8> name;
};

}  // end namespace midi2::types

#endif  // MIDI2_UMP_TYPES_HPP
