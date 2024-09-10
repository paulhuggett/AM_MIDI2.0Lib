/**********************************************************
 * MIDI 2.0 Library
 * Author: Andrew Mee
 *
 * MIT License
 * Copyright 2021 Andrew Mee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ********************************************************/

#ifndef MIDI2_UTILS_HPP
#define MIDI2_UTILS_HPP

#include <cassert>
#include <cstdint>

namespace midi2 {

enum status : std::uint8_t {
  // Channel voice messages
  note_off = 0x80,
  note_on = 0x90,
  key_pressure = 0xA0,  // Polyphonic Key Pressure (Aftertouch).
  cc = 0xB0,            // Continuous Controller
  program_change = 0xC0,
  channel_pressure = 0xD0,  // Channel Pressure (Aftertouch).
  pitch_bend = 0xE0,

  // System Common Messages
  sysex_start = 0xF0,
  timing_code = 0xF1,
  spp = 0xF2,  // Song Position Pointer
  song_select = 0xF3,
  reserved1 = 0xF4,
  reserved2 = 0xF5,
  tunerequest = 0xF6,
  sysex_stop = 0xF7,  // End of system exclusive
  timingclock = 0xF8,
  reserved3 = 0xF9,
  seqstart = 0xFA,  // Start the current sequence playing
  seqcont = 0xFB,   // Continue at the point the sequence was stopped
  seqstop = 0xFC,   // Stop the current sequence
  reserved4 = 0xFD,
  activesense = 0xFE,
  systemreset = 0xFF,
};

constexpr auto S7UNIVERSAL_NRT = std::byte{0x7E};
// constexpr auto S7UNIVERSAL_RT = 0x7F;
constexpr auto S7MIDICI = std::byte{0x0D};

// Status codes added in MIDI 2.
enum midi2status : std::uint8_t {
  pernote_manage = 0xF0,
  rpn_pernote = 0x00,
  nrpn_pernote = 0x10,
  rpn = 0x20,  // Registered Parameter Number
  nrpn = 0x30,
  rpn_relative = 0x40,
  nrpn_relative = 0x50,
  pitch_bend_pernote = 0x60,
};

// The MIDI 1.0 Specification defines Control Change indexes 98, 99, 100, and
// 101 (0x62, 0x63, 0x64, and 0x65) to be used as compound sequences for
// Non-Registered Parameter Number and Registered Parameter Number control
// messages. These set destinations for Control Change index 6/38 (0x06/0x26),
// Data Entry.
enum control : std::uint8_t {
  bank_select = 0x00,
  bank_select_lsb = 0x20,
  data_entry_msb = 0x06,
  data_entry_lsb = 0x26,
  rpn_lsb = 0x64,
  rpn_msb = 0x65,
  nrpn_lsb = 0x62,
  nrpn_msb = 0x63,
};

enum : std::uint8_t {
  UTILITY_NOOP = 0x0,
  UTILITY_JRCLOCK = 0x1,
  UTILITY_JRTS = 0x2,
  UTILITY_DELTACLOCKTICK = 0x3,
  UTILITY_DELTACLOCKSINCE = 0x4,
};

enum : std::uint8_t {
  FLEXDATA_COMMON = 0x00,
  FLEXDATA_COMMON_TEMPO = 0x00,
  FLEXDATA_COMMON_TIMESIG = 0x01,
  FLEXDATA_COMMON_METRONOME = 0x02,
  FLEXDATA_COMMON_KEYSIG = 0x05,
  FLEXDATA_COMMON_CHORD = 0x06,
  FLEXDATA_PERFORMANCE = 0x01,
  FLEXDATA_LYRIC = 0x02,
};

enum : std::uint32_t {
  MIDIENDPOINT = 0x000,
  MIDIENDPOINT_INFO_NOTIFICATION = 0x001,
  MIDIENDPOINT_DEVICEINFO_NOTIFICATION = 0x002,
  MIDIENDPOINT_NAME_NOTIFICATION = 0x003,
  MIDIENDPOINT_PRODID_NOTIFICATION = 0x004,
  MIDIENDPOINT_PROTOCOL_REQUEST = 0x005,
  MIDIENDPOINT_PROTOCOL_NOTIFICATION = 0x006,
  STARTOFSEQ = 0x020,
  ENDOFFILE = 0x021,
};

enum : std::uint32_t {
  FUNCTIONBLOCK = 0x010,
  FUNCTIONBLOCK_INFO_NOTFICATION = 0x011,
  FUNCTIONBLOCK_NAME_NOTIFICATION = 0x012,
};

enum class ci_message : std::uint8_t {
  protocol_negotiation = 0x10,
  protocol_negotiation_reply = 0x11,
  protocol_set = 0x12,
  protocol_test = 0x13,
  protocol_test_responder = 0x14,
  protocol_confirm = 0x15,

  profile_inquiry = 0x20,
  profile_inquiry_reply = 0x21,
  profile_set_on = 0x22,
  profile_set_off = 0x23,
  profile_enabled = 0x24,
  profile_disabled = 0x25,
  profile_added = 0x26,
  profile_removed = 0x27,
  profile_details = 0x28,
  profile_details_reply = 0x29,
  profile_specific_data = 0x2F,

  pe_capability = 0x30,
  pe_capability_reply = 0x31,
  pe_get = 0x34,
  pe_get_reply = 0x35,
  pe_set = 0x36,
  pe_set_reply = 0x37,
  pe_sub = 0x38,
  pe_sub_reply = 0x39,
  pe_notify = 0x3F,

  pi_capability = 0x40,
  pi_capability_reply = 0x41,
  pi_mm_report = 0x42,
  pi_mm_report_reply = 0x43,
  pi_mm_report_end = 0x44,

  discovery = 0x70,
  discovery_reply = 0x71,
  endpoint_info = 0x72,
  endpoint_info_reply = 0x73,
  ack = 0x7D,
  invalidate_muid = 0x7E,
  nak = 0x7F,
};

enum {
  MIDICI_PE_STATUS_OK = 200,
  MIDICI_PE_STATUS_ACCEPTED = 202,
  MIDICI_PE_STATUS_RESOURCE_UNAVAILABLE = 341,
  MIDICI_PE_STATUS_BAD_DATA = 342,
  MIDICI_PE_STATUS_TOO_MANY_REQS = 343,
  MIDICI_PE_STATUS_BAD_REQ = 400,
  MIDICI_PE_STATUS_REQ_UNAUTHORIZED = 403,
  MIDICI_PE_STATUS_RESOURCE_UNSUPPORTED = 404,
  MIDICI_PE_STATUS_RESOURCE_NOT_ALLOWED = 405,
  MIDICI_PE_STATUS_PAYLOAD_TOO_LARGE = 413,
  MIDICI_PE_STATUS_UNSUPPORTED_MEDIA_TYPE = 415,
  MIDICI_PE_STATUS_INVALID_DATA_VERSION = 445,
  MIDICI_PE_STATUS_INTERNAL_DEVICE_ERROR = 500,
};

enum : std::uint8_t {
  MIDICI_PE_COMMAND_START = 1,
  MIDICI_PE_COMMAND_END = 2,
  MIDICI_PE_COMMAND_PARTIAL = 3,
  MIDICI_PE_COMMAND_FULL = 4,
  MIDICI_PE_COMMAND_NOTIFY = 5,
};

enum : std::uint8_t {
  MIDICI_PE_ACTION_COPY = 1,
  MIDICI_PE_ACTION_MOVE = 2,
  MIDICI_PE_ACTION_DELETE = 3,
  MIDICI_PE_ACTION_CREATE_DIR = 4,
};

enum : std::uint8_t {
  MIDICI_PE_ASCII = 1,
  MIDICI_PE_MCODED7 = 2,
  MIDICI_PE_MCODED7ZLIB = 3,
};

constexpr auto M2_CI_BROADCAST = std::uint32_t{0x0FFFFFFF};

enum : std::uint32_t {
  UMP_VER_MAJOR = 1,
  UMP_VER_MINOR = 1,
};

enum class ump_message_type : std::uint32_t {
  utility = 0x00,
  system = 0x01,
  m1cvm = 0x02,
  sysex7 = 0x03,
  m2cvm = 0x04,
  data = 0x05,
  reserved32_06 = 0x06,
  reserved32_07 = 0x07,
  reserved64_08 = 0x08,
  reserved64_09 = 0x09,
  reserved64_0A = 0x0A,
  reserved96_0B = 0x0B,
  reserved96_0C = 0x0C,
  flex_data = 0x0D,
  reserved128_0E = 0x0E,
  midi_endpoint = 0x0F,
};

constexpr std::uint32_t pack(std::uint8_t const b0, std::uint8_t const b1, std::uint8_t const b2,
                             std::uint8_t const b3) {
  return (std::uint32_t{b0} << 24) | (std::uint32_t{b1} << 16) | (std::uint32_t{b2} << 8) | std::uint32_t{b3};
}

constexpr uint32_t scaleUp(uint32_t srcVal, uint8_t srcBits, uint8_t dstBits) {
  assert(dstBits >= srcBits);
  // Handle value of 0 - skip processing
  if (srcVal == 0) {
    return std::uint32_t{0};
  }

  // handle 1-bit (bool) scaling
  if (srcBits == 1) {
    return static_cast<std::uint32_t>((std::uint32_t{1} << dstBits) - 1U);
  }

  // simple bit shift
  uint8_t const scaleBits = (dstBits - srcBits);
  auto bitShiftedValue = static_cast<std::uint32_t>(srcVal << scaleBits);
  uint32_t const srcCenter = 1 << (srcBits - 1);
  if (srcVal <= srcCenter) {
    return bitShiftedValue;
  }

  // expanded bit repeat scheme
  uint8_t const repeatBits = srcBits - 1;
  auto repeatMask = (std::uint32_t{1} << repeatBits) - std::uint32_t{1};
  uint32_t repeatValue = srcVal & repeatMask;
  if (scaleBits > repeatBits) {
    repeatValue <<= scaleBits - repeatBits;
  } else {
    repeatValue >>= repeatBits - scaleBits;
  }

  while (repeatValue != 0) {
    bitShiftedValue |= repeatValue;
    repeatValue >>= repeatBits;
  }
  return bitShiftedValue;
}

constexpr uint32_t scaleDown(uint32_t srcVal, uint8_t srcBits, uint8_t dstBits) {
  assert(srcBits >= dstBits);
  uint8_t const scaleBits = (srcBits - dstBits);
  return srcVal >> scaleBits;
}

}  // end namespace midi2

#endif  // MIDI2_UTILS_HPP
