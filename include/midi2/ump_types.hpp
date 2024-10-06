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

namespace data64 {

// 7.7 System Exclusive (7-Bit) Messages
union sysex7_w0 {
  UMP_MEMBERS(sysex7_w0)
  ump_bitfield<28, 4> mt;  ///< Always 0x3
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  // 0b0000..0b0011
  ump_bitfield<16, 4> number_of_bytes;
  ump_bitfield<15, 1> reserved0;
  ump_bitfield<8, 7> data0;
  ump_bitfield<7, 1> reserved1;
  ump_bitfield<0, 7> data1;
};
union sysex7_w1 {
  UMP_MEMBERS(sysex7_w1)
  ump_bitfield<31, 1> reserved0;
  ump_bitfield<24, 7> data2;
  ump_bitfield<23, 1> reserved1;
  ump_bitfield<16, 7> data3;
  ump_bitfield<15, 1> reserved2;
  ump_bitfield<8, 7> data4;
  ump_bitfield<7, 1> reserved3;
  ump_bitfield<0, 7> data5;
};

}  // end namespace data64

// F.2.2 Message Type 0x4: MIDI 2.0 Channel Voice Messages
// Table 30 8-Byte UMP Formats for Message Type 0x4: MIDI 2.0 Channel Voice Messages
namespace m2cvm {
// 7.4.1 MIDI 2.0 Note Off Message
// 7.4.2 MIDI 2.0 Note On Message
union note_w0 {
  UMP_MEMBERS(note_w0)
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Note-off=0x8, note-on=0x9
  ump_bitfield<16, 4> channel;
  ump_bitfield<15, 1> reserved0;
  ump_bitfield<8, 7> note;
  ump_bitfield<0, 8> attribute;
};
union note_w1 {
  UMP_MEMBERS(note_w1)
  ump_bitfield<16, 16> velocity;
  ump_bitfield<0, 16> attribute;
};

// 7.4.3 MIDI 2.0 Poly Pressure Message
union poly_pressure_w0 {
  UMP_MEMBERS(poly_pressure_w0)
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
  UMP_MEMBERS(controller_w0)
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
  UMP_MEMBERS(per_note_management_w0)
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
  UMP_MEMBERS(control_change_w0)
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
  UMP_MEMBERS(controller_message_w0)
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
  UMP_MEMBERS(program_change_w0)
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Always 0xC
  ump_bitfield<16, 4> channel;
  ump_bitfield<8, 8> reserved;
  ump_bitfield<1, 7> option_flags;  ///< Reserved option flags
  ump_bitfield<0, 1> bank_valid;    ///< Bank change is ignored if this bit is zero.
};
union program_change_w1 {
  UMP_MEMBERS(program_change_w1)
  ump_bitfield<24, 8> program;
  ump_bitfield<16, 8> reserved;
  ump_bitfield<15, 1> r0;  // reserved
  ump_bitfield<8, 7> bank_msb;
  ump_bitfield<7, 1> r1;  // reserved
  ump_bitfield<0, 7> bank_lsb;
};

// 7.4.10 MIDI 2.0 Channel Pressure Message
union channel_pressure_w0 {
  UMP_MEMBERS(channel_pressure_w0)
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Always 0xA
  ump_bitfield<16, 4> channel;
  ump_bitfield<8, 8> reserved0;
  ump_bitfield<0, 8> reserved1;
};

// 7.4.11 MIDI 2.0 Pitch Bend Message
union pitch_bend_w0 {
  UMP_MEMBERS(pitch_bend_w0)
  ump_bitfield<28, 4> mt;  ///< Always 0x4
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  ///< Always 0xE
  ump_bitfield<16, 4> channel;
  ump_bitfield<8, 8> reserved0;
  ump_bitfield<0, 8> reserved1;
};

// 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
union per_note_pitch_bend_w0 {
  UMP_MEMBERS(per_note_pitch_bend_w0)
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
  UMP_MEMBERS(endpoint_discovery_w0)
  ump_bitfield<28, 4> mt;       // 0x0F
  ump_bitfield<26, 2> format;   // 0x00
  ump_bitfield<16, 10> status;  // 0x00
  ump_bitfield<8, 8> version_major;
  ump_bitfield<0, 8> version_minor;
};
union endpoint_discovery_w1 {
  UMP_MEMBERS(endpoint_discovery_w1)
  ump_bitfield<8, 24> reserved;
  ump_bitfield<0, 8> filter;
};
using endpoint_discovery_w2 = std::uint32_t;
using endpoint_discovery_w3 = std::uint32_t;

// 7.1.2 Endpoint Info Notification Message
union endpoint_info_notification_w0 {
  UMP_MEMBERS(endpoint_info_notification_w0)
  ump_bitfield<28, 4> mt;       // 0x0F
  ump_bitfield<26, 2> format;   // 0x00
  ump_bitfield<16, 10> status;  // 0x01
  ump_bitfield<8, 8> version_major;
  ump_bitfield<0, 8> version_minor;
};
union endpoint_info_notification_w1 {
  UMP_MEMBERS(endpoint_info_notification_w1)
  ump_bitfield<31, 1> static_function_blocks;
  ump_bitfield<24, 7> number_function_blocks;
  ump_bitfield<10, 14> reserved0;
  ump_bitfield<9, 1> midi2_protocol_capability;
  ump_bitfield<8, 1> midi1_protocol_capability;
  ump_bitfield<2, 6> reserved1;
  ump_bitfield<1, 1> receive_jr_timestamp_capability;
  ump_bitfield<0, 1> transmit_jr_timestamp_capability;
};
using endpoint_info_notification_w2 = std::uint32_t;
using endpoint_info_notification_w3 = std::uint32_t;

// 7.1.3 Device Identity Notification Message
union device_identity_notification_w0 {
  UMP_MEMBERS(device_identity_notification_w0)
  ump_bitfield<28, 4> mt;       // 0x0F
  ump_bitfield<26, 2> format;   // 0x00
  ump_bitfield<16, 10> status;  // 0x02
  ump_bitfield<0, 16> reserved0;
};
union device_identity_notification_w1 {
  UMP_MEMBERS(device_identity_notification_w1)
  ump_bitfield<24, 8> reserved0;
  ump_bitfield<23, 1> reserved1;
  ump_bitfield<16, 7> dev_manuf_sysex_id_1;  // device manufacturer sysex id byte 1
  ump_bitfield<15, 1> reserved2;
  ump_bitfield<8, 7> dev_manuf_sysex_id_2;  // device manufacturer sysex id byte 2
  ump_bitfield<7, 1> reserved3;
  ump_bitfield<0, 7> dev_manuf_sysex_id_3;  // device manufacturer sysex id byte 3
};
union device_identity_notification_w2 {
  UMP_MEMBERS(device_identity_notification_w2)
  ump_bitfield<31, 1> reserved0;
  ump_bitfield<24, 7> device_family_lsb;
  ump_bitfield<23, 1> reserved1;
  ump_bitfield<16, 7> device_family_msb;
  ump_bitfield<15, 1> reserved2;
  ump_bitfield<8, 7> device_family_model_lsb;
  ump_bitfield<7, 1> reserved3;
  ump_bitfield<0, 7> device_family_model_msb;
};
union device_identity_notification_w3 {
  UMP_MEMBERS(device_identity_notification_w3)
  ump_bitfield<31, 1> reserved0;
  ump_bitfield<24, 7> sw_revision_1;  // Software revision level byte 1
  ump_bitfield<23, 1> reserved1;
  ump_bitfield<16, 7> sw_revision_2;  // Software revision level byte 2
  ump_bitfield<15, 1> reserved2;
  ump_bitfield<8, 7> sw_revision_3;  // Software revision level byte 3
  ump_bitfield<7, 1> reserved3;
  ump_bitfield<0, 7> sw_revision_4;  // Software revision level byte 4
};

// 7.1.4 Endpoint Name Notification
union endpoint_name_notification_w0 {
  UMP_MEMBERS(endpoint_name_notification_w0)
  ump_bitfield<28, 4> mt;  // 0x0F
  ump_bitfield<26, 2> format;
  ump_bitfield<16, 10> status;  // 0x03
  ump_bitfield<8, 8> name1;
  ump_bitfield<0, 8> name2;
};
union endpoint_name_notification_w1 {
  UMP_MEMBERS(endpoint_name_notification_w1)
  ump_bitfield<24, 8> name3;
  ump_bitfield<16, 8> name4;
  ump_bitfield<8, 8> name5;
  ump_bitfield<0, 8> name6;
};
union endpoint_name_notification_w2 {
  UMP_MEMBERS(endpoint_name_notification_w2)
  ump_bitfield<24, 8> name7;
  ump_bitfield<16, 8> name8;
  ump_bitfield<8, 8> name9;
  ump_bitfield<0, 8> name10;
};
union endpoint_name_notification_w3 {
  UMP_MEMBERS(endpoint_name_notification_w3)
  ump_bitfield<24, 8> name11;
  ump_bitfield<16, 8> name12;
  ump_bitfield<8, 8> name13;
  ump_bitfield<0, 8> name14;
};

// 7.1.5 Product Instance Id Notification Message
union product_instance_id_notification_w0 {
  UMP_MEMBERS(product_instance_id_notification_w0)
  ump_bitfield<28, 4> mt;
  ump_bitfield<26, 2> format;
  ump_bitfield<16, 10> status;
  ump_bitfield<8, 8> pid1;
  ump_bitfield<0, 8> pid2;
};
union product_instance_id_notification_w1 {
  UMP_MEMBERS(product_instance_id_notification_w1)
  ump_bitfield<24, 8> pid3;
  ump_bitfield<16, 8> pid4;
  ump_bitfield<8, 8> pid5;
  ump_bitfield<0, 8> pid6;
};
union product_instance_id_notification_w2 {
  UMP_MEMBERS(product_instance_id_notification_w2)
  ump_bitfield<24, 8> pid7;
  ump_bitfield<16, 8> pid8;
  ump_bitfield<8, 8> pid9;
  ump_bitfield<0, 8> pid10;
};
union product_instance_id_notification_w3 {
  UMP_MEMBERS(product_instance_id_notification_w3)
  ump_bitfield<24, 8> pid11;
  ump_bitfield<16, 8> pid12;
  ump_bitfield<8, 8> pid13;
  ump_bitfield<0, 8> pid14;
};

// 7.1.6 Selecting a MIDI Protocol and Jitter Reduction Timestamps for a UMP Stream
// 7.1.6.1 Steps to Select Protocol and Jitter Reduction Timestamps
// 7.1.6.2 JR Stream Configuration Request
union jr_configuration_request_w0 {
  UMP_MEMBERS(jr_configuration_request_w0)
  ump_bitfield<28, 4> mt;       // 0x0F
  ump_bitfield<26, 2> format;   // 0x00
  ump_bitfield<16, 10> status;  // 0x05
  ump_bitfield<8, 8> protocol;
  ump_bitfield<2, 6> reserved0;
  ump_bitfield<1, 1> rxjr;
  ump_bitfield<0, 1> txjr;
};
using jr_configuration_request_w1 = std::uint32_t;
using jr_configuration_request_w2 = std::uint32_t;
using jr_configuration_request_w3 = std::uint32_t;

// 7.1.6.3 JR Stream Configuration Notification Message
union jr_configuration_notification_w0 {
  UMP_MEMBERS(jr_configuration_notification_w0)
  ump_bitfield<28, 4> mt;       // 0x0F
  ump_bitfield<26, 2> format;   // 0x00
  ump_bitfield<16, 10> status;  // 0x06
  ump_bitfield<8, 8> protocol;
  ump_bitfield<2, 6> reserved0;
  ump_bitfield<1, 1> rxjr;
  ump_bitfield<0, 1> txjr;
};
using jr_configuration_notification_w1 = std::uint32_t;
using jr_configuration_notification_w2 = std::uint32_t;
using jr_configuration_notification_w3 = std::uint32_t;

// 7.1.7 Function Block Discovery Message
union function_block_discovery_w0 {
  UMP_MEMBERS(function_block_discovery_w0)
  ump_bitfield<28, 4> mt;       // 0x0F
  ump_bitfield<26, 2> format;   // 0x00
  ump_bitfield<16, 10> status;  // 0x10
  ump_bitfield<8, 8> block_num;
  ump_bitfield<0, 8> filter;
};
using function_block_discovery_w1 = std::uint32_t;
using function_block_discovery_w2 = std::uint32_t;
using function_block_discovery_w3 = std::uint32_t;

// 7.1.8 Function Block Info Notification
union function_block_info_notification_w0 {
  UMP_MEMBERS(function_block_info_notification_w0)
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
union function_block_info_notification_w1 {
  UMP_MEMBERS(function_block_info_notification_w1)
  ump_bitfield<24, 8> first_group;
  ump_bitfield<16, 8> num_spanned;
  ump_bitfield<8, 8> ci_message_version;
  ump_bitfield<0, 8> max_sys8_streams;
};
using function_block_info_notification_w2 = std::uint32_t;
using function_block_info_notification_w3 = std::uint32_t;

// 7.1.9 Function Block Name Notification
union function_block_name_notification_w0 {
  UMP_MEMBERS(function_block_name_notification_w0)
  ump_bitfield<28, 4> mt;       // 0x0F
  ump_bitfield<26, 2> format;   // 0x00
  ump_bitfield<16, 10> status;  // 0x12
  ump_bitfield<8, 8> block_num;
  ump_bitfield<0, 8> name0;
};
union function_block_name_notification_w1 {
  UMP_MEMBERS(function_block_name_notification_w1)
  ump_bitfield<24, 8> name1;
  ump_bitfield<16, 8> name2;
  ump_bitfield<8, 8> name3;
  ump_bitfield<0, 8> name4;
};
union function_block_name_notification_w2 {
  UMP_MEMBERS(function_block_name_notification_w2)
  ump_bitfield<24, 8> name5;
  ump_bitfield<16, 8> name6;
  ump_bitfield<8, 8> name7;
  ump_bitfield<0, 8> name8;
};
union function_block_name_notification_w3 {
  UMP_MEMBERS(function_block_name_notification_w3)
  ump_bitfield<24, 8> name9;
  ump_bitfield<16, 8> name10;
  ump_bitfield<8, 8> name11;
  ump_bitfield<0, 8> name12;
};

// 7.1.10 Start of Clip Message
union start_of_clip_w0 {
  UMP_MEMBERS(start_of_clip_w0)
  ump_bitfield<28, 4> mt;       // 0x0F
  ump_bitfield<26, 2> format;   // 0x00
  ump_bitfield<16, 10> status;  // 0x20
  ump_bitfield<0, 16> reserved0;
};
using start_of_clip_w1 = std::uint32_t;
using start_of_clip_w2 = std::uint32_t;
using start_of_clip_w3 = std::uint32_t;

// 7.1.11 End of Clip Message
union end_of_clip_w0 {
  UMP_MEMBERS(end_of_clip_w0)
  ump_bitfield<28, 4> mt;       // 0x0F
  ump_bitfield<26, 2> format;   // 0x00
  ump_bitfield<16, 10> status;  // 0x21
  ump_bitfield<0, 16> reserved0;
};
using end_of_clip_w1 = std::uint32_t;
using end_of_clip_w2 = std::uint32_t;
using end_of_clip_w3 = std::uint32_t;

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
using set_tempo_w0 = flex_data_w0;
using set_tempo_w1 = std::uint32_t;
using set_tempo_w2 = std::uint32_t;
using set_tempo_w3 = std::uint32_t;

// 7.5.4 Set Time Signature Message
using set_time_signature_w0 = flex_data_w0;
union set_time_signature_w1 {
  UMP_MEMBERS(set_time_signature_w1)
  ump_bitfield<24, 8> numerator;
  ump_bitfield<16, 8> denominator;
  ump_bitfield<8, 8> number_of_32_notes;
  ump_bitfield<0, 8> reserved0;
};
using set_time_signature_w2 = std::uint32_t;
using set_time_signature_w3 = std::uint32_t;

// 7.5.5 Set Metronome Message

using set_metronome_w0 = flex_data_w0;
union set_metronome_w1 {
  UMP_MEMBERS(set_metronome_w1)
  ump_bitfield<24, 8> num_clocks_per_primary_click;
  ump_bitfield<16, 8> bar_accent_part_1;
  ump_bitfield<8, 8> bar_accent_part_2;
  ump_bitfield<0, 8> bar_accent_part_3;
};
union set_metronome_w2 {
  UMP_MEMBERS(set_metronome_w2)
  ump_bitfield<24, 8> num_subdivision_clicks_1;
  ump_bitfield<16, 8> num_subdivision_clicks_2;
  ump_bitfield<0, 16> reserved0;
};
using set_metronome_w3 = std::uint32_t;

// 7.5.7 Set Key Signature Message
using set_key_signature_w0 = flex_data_w0;
union set_key_signature_w1 {
  UMP_MEMBERS(set_key_signature_w1)
  ump_bitfield<28, 4> sharps_flats;
  ump_bitfield<24, 4> tonic_note;
  ump_bitfield<0, 24> reserved0;
};
using set_key_signature_w2 = std::uint32_t;
using set_key_signature_w3 = std::uint32_t;

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
using set_chord_name_w0 = flex_data_w0;
union set_chord_name_w1 {
  UMP_MEMBERS(set_chord_name_w1)
  ump_bitfield<28, 4> tonic_sharps_flats;  // 2's compl
  ump_bitfield<24, 4> chord_tonic;
  ump_bitfield<16, 8> chord_type;
  ump_bitfield<12, 4> alter_1_type;
  ump_bitfield<8, 4> alter_1_degree;
  ump_bitfield<4, 4> alter_2_type;
  ump_bitfield<0, 4> alter_2_degree;
};
union set_chord_name_w2 {
  UMP_MEMBERS(set_chord_name_w2)
  ump_bitfield<28, 4> alter_3_type;
  ump_bitfield<24, 4> alter_3_degree;
  ump_bitfield<20, 4> alter_4_type;
  ump_bitfield<16, 4> alter_4_degree;
  ump_bitfield<0, 16> reserved;  // 0x0000
};
union set_chord_name_w3 {
  UMP_MEMBERS(set_chord_name_w3)
  ump_bitfield<28, 4> bass_sharps_flats;  // 2's compl
  ump_bitfield<24, 4> bass_note;
  ump_bitfield<16, 8> bass_chord_type;
  ump_bitfield<12, 4> alter_1_type;
  ump_bitfield<8, 4> alter_1_degree;
  ump_bitfield<4, 4> alter_2_type;
  ump_bitfield<0, 4> alter_2_degree;
};

// 7.5.9 Text Messages Common Format
using text_common_w0 = flex_data_w0;
using text_common_w1 = std::uint32_t;
using text_common_w2 = std::uint32_t;
using text_common_w3 = std::uint32_t;

}  // end namespace flex_data

namespace data128 {

// F.3.1 Message Type 0x5: 16-byte Data Messages (System Exclusive 8 and Mixed
// Data Set) Table 31 16-Byte UMP Formats for Message Type 0x5: System Exclusive
// 8 and Mixed Data Set

// SysEx8 in 1 UMP (word 1)
// SysEx8 Start (word 1)
// SysEx8 Continue (word 1)
// SysEx8 End (word 1)
union sysex8_w0 {
  UMP_MEMBERS(sysex8_w0)
  ump_bitfield<28, 4> mt;  // Always 0x05
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;
  ump_bitfield<16, 4> number_of_bytes;
  ump_bitfield<8, 8> stream_id;
  ump_bitfield<0, 8> data0;
};
union sysex8_w1 {
  UMP_MEMBERS(sysex8_w1)
  ump_bitfield<24, 8> data1;
  ump_bitfield<16, 8> data2;
  ump_bitfield<8, 8> data3;
  ump_bitfield<0, 8> data4;
};
union sysex8_w2 {
  UMP_MEMBERS(sysex8_w2)
  ump_bitfield<24, 8> data5;
  ump_bitfield<16, 8> data6;
  ump_bitfield<8, 8> data7;
  ump_bitfield<0, 8> data8;
};
union sysex8_w3 {
  UMP_MEMBERS(sysex8_w3)
  ump_bitfield<24, 8> data9;
  ump_bitfield<16, 8> data10;
  ump_bitfield<8, 8> data11;
  ump_bitfield<0, 8> data12;
};
struct sysex8 {
  sysex8_w0 w0;
  sysex8_w1 w1;
  sysex8_w2 w2;
  sysex8_w3 w3;
};

// 7.9 Mixed Data Set Message
// Mixed Data Set Header (word 1)
// Mixed Data Set Payload (word 1)
union mds_header_w0 {
  UMP_MEMBERS(mds_header_w0)
  ump_bitfield<28, 4> mt;  // Always 0x05
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  // Always 0x08
  ump_bitfield<16, 4> mds_id;
  ump_bitfield<0, 16> bytes_in_chunk;
};
union mds_header_w1 {
  UMP_MEMBERS(mds_header_w1)
  ump_bitfield<16, 16> chunks_in_mds;
  ump_bitfield<0, 16> chunk_num;
};
union mds_header_w2 {
  UMP_MEMBERS(mds_header_w2)
  ump_bitfield<16, 16> manufacturer_id;
  ump_bitfield<0, 16> device_id;
};
union mds_header_w3 {
  UMP_MEMBERS(mds_header_w3)
  ump_bitfield<16, 16> sub_id_1;
  ump_bitfield<0, 16> sub_id_2;
};

struct mds_header {
  mds_header_w0 w0;
  mds_header_w1 w1;
  mds_header_w2 w2;
  mds_header_w3 w3;
};

union mds_payload_w0 {
  UMP_MEMBERS(mds_payload_w0)
  ump_bitfield<28, 4> mt;  // Always 0x05
  ump_bitfield<24, 4> group;
  ump_bitfield<20, 4> status;  // Always 0x09
  ump_bitfield<16, 4> mds_id;
  ump_bitfield<0, 16> data0;
};
using mds_payload_w1 = std::uint32_t;
using mds_payload_w2 = std::uint32_t;
using mds_payload_w3 = std::uint32_t;

struct mds_payload {
  mds_payload_w0 w0;
  mds_payload_w1 w1;
  mds_payload_w2 w2;
  mds_payload_w3 w3;
};

}  // end namespace data128

}  // end namespace midi2::types

#undef UMP_MEMBERS

#endif  // MIDI2_UMP_TYPES_HPP
