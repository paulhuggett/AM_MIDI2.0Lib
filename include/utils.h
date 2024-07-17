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

#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <tuple>

enum status : std::uint8_t {
  note_off = 0x80,
  note_on = 0x90,
  key_pressure = 0xA0,
  cc = 0xB0,   // Continuous Controller
  rpn = 0x20,  // Registered Parameter Number
  nrpn = 0x30,
  rpn_relative = 0x40,
  nrpn_relative = 0x50,
  program_change = 0xC0,
  channel_pressure = 0xD0,
  pitch_bend = 0xE0,
  pitch_bend_pernote = 0x60,
  nrpn_pernote = 0x10,
  rpn_pernote = 0x00,
  pernote_manage = 0xF0,

  sysex_start = 0xF0,
  timing_code = 0xF1,
  spp = 0xF2,
  song_select = 0xF3,
  tunerequest = 0xF6,
  sysex_stop = 0xF7,
  timingclock = 0xF8,
  seqstart = 0xFA,
  seqcont = 0xFB,
  seqstop = 0xFC,
  activesense = 0xFE,
  systemreset = 0xFF,
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

#define MIDIENDPOINT 0x000
#define MIDIENDPOINT_INFO_NOTIFICATION 0x001
#define MIDIENDPOINT_DEVICEINFO_NOTIFICATION 0x002
#define MIDIENDPOINT_NAME_NOTIFICATION 0x003
#define MIDIENDPOINT_PRODID_NOTIFICATION 0x004
#define MIDIENDPOINT_PROTOCOL_REQUEST 0x005
#define MIDIENDPOINT_PROTOCOL_NOTIFICATION 0x006
#define STARTOFSEQ 0x020
#define ENDOFFILE 0x021

#define FUNCTIONBLOCK 0x010
#define FUNCTIONBLOCK_INFO_NOTFICATION 0x011
#define FUNCTIONBLOCK_NAME_NOTIFICATION 0x012

#ifndef S7_BUFFERLEN
#define S7_BUFFERLEN 36
#endif
#define S7UNIVERSAL_NRT 0x7E
#define S7UNIVERSAL_RT 0x7F
#define S7MIDICI 0x0D

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

#define MIDICI_PE_STATUS_OK 200
#define MIDICI_PE_STATUS_ACCEPTED 202
#define MIDICI_PE_STATUS_RESOURCE_UNAVAILABLE 341
#define MIDICI_PE_STATUS_BAD_DATA 342
#define MIDICI_PE_STATUS_TOO_MANY_REQS 343
#define MIDICI_PE_STATUS_BAD_REQ 400
#define MIDICI_PE_STATUS_REQ_UNAUTHORIZED 403
#define MIDICI_PE_STATUS_RESOURCE_UNSUPPORTED 404
#define MIDICI_PE_STATUS_RESOURCE_NOT_ALLOWED 405
#define MIDICI_PE_STATUS_PAYLOAD_TOO_LARGE 413
#define MIDICI_PE_STATUS_UNSUPPORTED_MEDIA_TYPE 415
#define MIDICI_PE_STATUS_INVALID_DATA_VERSION 445
#define MIDICI_PE_STATUS_INTERNAL_DEVICE_ERROR 500

#define MIDICI_PI_CAPABILITY 0x40
#define MIDICI_PI_CAPABILITYREPLY 0x41
#define MIDICI_PI_MM_REPORT 0x42
#define MIDICI_PI_MM_REPORT_REPLY 0x43
#define MIDICI_PI_MM_REPORT_END 0x44

#define MIDICI_PE_COMMAND_START 1
#define MIDICI_PE_COMMAND_END 2
#define MIDICI_PE_COMMAND_PARTIAL 3
#define MIDICI_PE_COMMAND_FULL 4
#define MIDICI_PE_COMMAND_NOTIFY 5

#define MIDICI_PE_ACTION_COPY 1
#define MIDICI_PE_ACTION_MOVE 2
#define MIDICI_PE_ACTION_DELETE 3
#define MIDICI_PE_ACTION_CREATE_DIR 4

#define MIDICI_PE_ASCII 1
#define MIDICI_PE_MCODED7 2
#define MIDICI_PE_MCODED7ZLIB 3

#define FUNCTION_BLOCK 0x7F
#define M2_CI_BROADCAST 0xFFFFFFF

#define UMP_VER_MAJOR 1
#define UMP_VER_MINOR 1

#ifndef EXP_MIDICI_PE_EXPERIMENTAL_PATH
#define EXP_MIDICI_PE_EXPERIMENTAL_PATH 1
#endif

enum class ump_message_type : std::uint8_t {
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

namespace M2Utils {

/// \tparam ElementType The type of the elements held by this container.
/// \tparam Elements The number of elements in the FIFO. Must be less than 2^32.
template <typename ElementType, std::uint32_t Elements> class fifo {
  static_assert(Elements > 0 && Elements < std::uint32_t{1} << 31,
                "Number of elements (e) must be : 0 < e > 2^32");

public:
  fifo()
      : writeIndex_{0}, writeIndexWrap_{0}, readIndex_{0}, readIndexWrap_{0} {}
  /// \brief Inserts an element at the end.
  /// \param value  The value of the element to append.
  /// \returns True if the element was appended, false if the container was full.
  bool push_back(ElementType const& value) {
    if (this->full()) {
      return false;
    }
    arr_[writeIndex_++] = value;
    if (writeIndex_ >= Elements) {
      writeIndex_ = 0U;
      // Flip the bit indicating that we've wrapped round.
      writeIndexWrap_ = !writeIndexWrap_;
    }
    return true;
  }
  /// \brief Removes the first element of the container and returns it.
  /// If there are no elements in the container, the behavior is undefined.
  ElementType pop_front() {
    assert(!this->empty());
    auto const value = std::move(arr_[readIndex_++]);
    if (readIndex_ >= Elements) {
      readIndex_ = 0U;
      readIndexWrap_ = !readIndexWrap_;
    }
    return value;
  }
  /// \brief Checks whether the container is empty.
  /// The FIFO is empty when both indices, including the "wrap_" fields are
  /// equal. \returns True if the container is empty, false otherwise.
  constexpr bool empty() const {
    return writeIndex_ == readIndex_ && writeIndexWrap_ == readIndexWrap_;
  }
  /// \brief Checks whether the container is full.
  /// The FIFO is full then when both indices are equal but the "wrap_" fields
  /// different. \returns True if the container is full, false otherwise.
  constexpr bool full() const {
    return writeIndex_ == readIndex_ && writeIndexWrap_ != readIndexWrap_;
  }
  /// \brief Returns the number of elements.
  constexpr std::size_t size() const {
    return writeIndex_ + (writeIndexWrap_ != readIndexWrap_ ? Elements : 0U) -
           readIndex_;
  }
  /// \brief Returns the maximum possible number of elements.
  constexpr std::size_t max_size() const { return Elements; }

private:
  /// Returns the number of bits required for value.
  template <typename T, typename = typename std::enable_if<
                            std::is_unsigned<T>::value>::type>
  static constexpr unsigned bitsRequired(T const value) {
    return value == 0U ? 0U : 1U + bitsRequired(static_cast<T>(value >> 1U));
  }

  std::array<ElementType, Elements> arr_{};

  static constexpr auto bits = bitsRequired(Elements);
  using bitfieldType = typename std::conditional<
      (bits < 4U), std::uint8_t,
      typename std::conditional<(bits < 8U), std::uint16_t,
                                std::uint32_t>::type>::type;

  bitfieldType writeIndex_ : bits;
  bitfieldType writeIndexWrap_ : 1;
  bitfieldType readIndex_ : bits;
  bitfieldType readIndexWrap_ : 1;
};

inline void clear(uint8_t* const dest, uint8_t const c, std::size_t const n) {
  for (auto i = std::size_t{0}; i < n; i++) {
    dest[i] = c;
  }
}

inline uint32_t scaleUp(uint32_t srcVal, uint8_t srcBits, uint8_t dstBits) {
  // Handle value of 0 - skip processing
  if (srcVal == 0) {
    return 0L;
  }

  // handle 1-bit (bool) scaling
  if (srcBits == 1) {
    return (1 << dstBits) - 1L;
  }

  // simple bit shift
  uint8_t scaleBits = (dstBits - srcBits);
  uint32_t bitShiftedValue = (srcVal + 0L) << scaleBits;
  uint32_t srcCenter = 1 << (srcBits - 1);
  if (srcVal <= srcCenter) {
    return bitShiftedValue;
  }

  // expanded bit repeat scheme
  uint8_t repeatBits = srcBits - 1;
  auto repeatMask = (1 << repeatBits) - 1;
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

inline uint32_t scaleDown(uint32_t srcVal, uint8_t srcBits, uint8_t dstBits) {
  // simple bit shift
  uint8_t scaleBits = (srcBits - dstBits);
  return srcVal >> scaleBits;
}

}  // namespace M2Utils

#endif
