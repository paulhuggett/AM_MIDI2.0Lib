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

#ifndef MIDI2_UTILS_H
#define MIDI2_UTILS_H

#include <cassert>
#include <cstdint>
#include <tuple>

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

enum : std::uint8_t {
  MIDICI_DISCOVERY = 0x70,
  MIDICI_DISCOVERYREPLY = 0x71,
  MIDICI_ENDPOINTINFO = 0x72,
  MIDICI_ENDPOINTINFO_REPLY = 0x73,
  MIDICI_INVALIDATEMUID = 0x7E,
  MIDICI_ACK = 0x7D,
  MIDICI_NAK = 0x7F,
};

enum : std::uint8_t {
  MIDICI_PROTOCOL_NEGOTIATION = 0x10,
  MIDICI_PROTOCOL_NEGOTIATION_REPLY = 0x11,
  MIDICI_PROTOCOL_SET = 0x12,
  MIDICI_PROTOCOL_TEST = 0x13,
  MIDICI_PROTOCOL_TEST_RESPONDER = 0x14,
  MIDICI_PROTOCOL_CONFIRM = 0x15,
};

enum : std::uint8_t {
  MIDICI_PROFILE_INQUIRY = 0x20,
  MIDICI_PROFILE_INQUIRYREPLY = 0x21,
  MIDICI_PROFILE_SETON = 0x22,
  MIDICI_PROFILE_SETOFF = 0x23,
  MIDICI_PROFILE_ENABLED = 0x24,
  MIDICI_PROFILE_DISABLED = 0x25,
  MIDICI_PROFILE_ADD = 0x26,
  MIDICI_PROFILE_REMOVE = 0x27,
  MIDICI_PROFILE_DETAILS_INQUIRY = 0x28,
  MIDICI_PROFILE_DETAILS_REPLY = 0x29,
  MIDICI_PROFILE_SPECIFIC_DATA = 0x2F,
};

enum : std::uint8_t {
  MIDICI_PE_CAPABILITY = 0x30,
  MIDICI_PE_CAPABILITYREPLY = 0x31,
  MIDICI_PE_GET = 0x34,
  MIDICI_PE_GETREPLY = 0x35,
  MIDICI_PE_SET = 0x36,
  MIDICI_PE_SETREPLY = 0x37,
  MIDICI_PE_SUB = 0x38,
  MIDICI_PE_SUBREPLY = 0x39,
  MIDICI_PE_NOTIFY = 0x3F,
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
  MIDICI_PI_CAPABILITY = 0x40,
  MIDICI_PI_CAPABILITYREPLY = 0x41,
  MIDICI_PI_MM_REPORT = 0x42,
  MIDICI_PI_MM_REPORT_REPLY = 0x43,
  MIDICI_PI_MM_REPORT_END = 0x44,
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

constexpr auto M2_CI_BROADCAST = std::uint32_t{0xFFFFFFF};

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

inline void clear(uint8_t* const dest, uint8_t const c, std::size_t const n) {
  for (auto i = std::size_t{0}; i < n; i++) {
    dest[i] = c;
  }
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
  uint8_t scaleBits = (dstBits - srcBits);
  auto bitShiftedValue = static_cast<std::uint32_t>(srcVal << scaleBits);
  uint32_t srcCenter = 1 << (srcBits - 1);
  if (srcVal <= srcCenter) {
    return bitShiftedValue;
  }

  // expanded bit repeat scheme
  uint8_t repeatBits = srcBits - 1;
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

constexpr uint32_t scaleDown(uint32_t srcVal, uint8_t srcBits,
                             uint8_t dstBits) {
  assert(srcBits >= dstBits);
  uint8_t const scaleBits = (srcBits - dstBits);
  return srcVal >> scaleBits;
}

}  // end namespace midi2

#endif  // MIDI2_UTILS_H
