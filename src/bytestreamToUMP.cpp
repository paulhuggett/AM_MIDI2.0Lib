//===-- Bytestream To UMP -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/bytestreamToUMP.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

void bytestreamToUMP::controllerToUMP(std::byte const b0, std::byte const b1, std::byte const b2) {
  auto const channel = b0 & std::byte{0x0F};
  auto& c = channel_[std::to_integer<unsigned>(channel)];
  switch (std::to_integer<std::underlying_type_t<control>>(b1)) {
  case control::bank_select: c.bankMSB = b2; break;
  case control::bank_select_lsb: c.bankLSB = b2; break;

  case control::data_entry_msb:  // RPN MSB Value
    if (c.rpnMsb != std::byte{0xFF} && c.rpnLsb != std::byte{0xFF}) {
      if (c.rpnMode && c.rpnMsb == std::byte{0} && (c.rpnLsb == std::byte{0} || c.rpnLsb == std::byte{6})) {
        auto const status = static_cast<std::byte>(c.rpnMode ? midi2status::rpn : midi2status::nrpn);
        output_.push_back(pack(ump_message_type::m2cvm, status | channel, c.rpnMsb, c.rpnLsb));
        output_.push_back(midi2::mcm_scale<14, 32>(std::to_integer<std::uint32_t>(b2) << 7));
      } else {
        c.rpnMsbValue = b2;
      }
    }
    break;
  case control::data_entry_lsb:
    // RPN LSB Value
    if (c.rpnMsb != std::byte{0xFF} && c.rpnLsb != std::byte{0xFF}) {
      auto const status = static_cast<std::byte>(c.rpnMode ? midi2status::rpn : midi2status::nrpn);
      output_.push_back(pack(ump_message_type::m2cvm, status | channel, c.rpnMsb, c.rpnLsb));
      output_.push_back(midi2::mcm_scale<14, 32>((std::to_integer<std::uint32_t>(c.rpnMsbValue) << 7) |
                                                 std::to_integer<std::uint32_t>(b2)));
    }
    break;
  case control::nrpn_msb: c.rpnMode = false; c.rpnMsb = b2; break;
  case control::nrpn_lsb: c.rpnMode = false; c.rpnLsb = b2; break;
  case control::rpn_msb: c.rpnMode = true; c.rpnMsb = b2; break;
  case control::rpn_lsb: c.rpnMode = true; c.rpnLsb = b2; break;
  default:
    output_.push_back(pack(ump_message_type::m2cvm, b0, b1, std::byte{0}));
    output_.push_back(midi2::mcm_scale<7, 32>(std::to_integer<std::uint32_t>(b2)));
    break;
  }
}

void bytestreamToUMP::bsToUMP(std::byte b0, std::byte b1, std::byte b2) {
  assert((b1 & std::byte{0x80}) == std::byte{0} && (b2 & std::byte{0x80}) == std::byte{0} &&
         "The top bit of b1 and b2 must be zero");
  using midi2::mcm_scale;
  auto const channel = b0 & std::byte{0x0F};
  auto status = static_cast<enum status>(b0 & std::byte{0xF0});

  if (to_underlying(b0) >= to_underlying(status::timing_code)) {
    output_.push_back(pack(ump_message_type::system, b0, b1, b2));
    return;
  }
  if (status < status::note_off || status > status::pitch_bend) {
    return;
  }
  if (!outputMIDI2_) {
    output_.push_back(pack(ump_message_type::m1cvm, b0, b1, b2));
    return;
  }
  if (status == status::note_on && b2 == std::byte{0}) {
    // Map note-on velocity 0 to note-off,
    status = status::note_off;
    b0 = static_cast<std::byte>(status) | channel;
    b2 = std::byte{0x40};
  }
  auto message = pack(ump_message_type::m2cvm, static_cast<std::byte>(status) | channel, std::byte{0}, std::byte{0});
  switch (status) {
  case status::note_on:
  case status::note_off:
  case status::poly_pressure:
    output_.push_back(message | (std::to_integer<std::uint32_t>(b1) << 8));
    output_.push_back(mcm_scale<7, 16>(std::to_integer<std::uint32_t>(b2)) << 16);
    break;
  case status::pitch_bend:
    output_.push_back(message);
    output_.push_back(
        mcm_scale<14, 32>((std::to_integer<std::uint32_t>(b2) << 7) | std::to_integer<std::uint32_t>(b1)));
    break;
  case status::program_change: {
    auto bank_msb = std::byte{0};
    auto bank_lsb = std::byte{0};
    auto const& c = channel_[std::to_integer<unsigned>(channel)];
    if (c.bankMSB != std::byte{0xFF} && c.bankLSB != std::byte{0xFF}) {
      message |= 0x01U;  // Set the "bank valid" bit.
      bank_msb = c.bankMSB;
      bank_lsb = c.bankLSB;
    }
    output_.push_back(message);
    output_.push_back(pack(b1, std::byte{0}, bank_msb, bank_lsb));
  } break;
  case status::channel_pressure:
    output_.push_back(message);
    output_.push_back(mcm_scale<7, 32>(std::to_integer<unsigned>(b1)));
    break;
  case status::cc: this->controllerToUMP(b0, b1, b2); break;
  default:
    // Unknown message
    break;
  }
}

namespace {

constexpr bool isSystemRealTimeMessage(std::byte const midi1Byte) {
  switch (static_cast<status>(midi1Byte)) {
  case status::timingclock:
  case status::seqstart:
  case status::seqcont:
  case status::seqstop:
  case status::activesense:
  case status::systemreset: return true;
  default: return false;
  }
}

constexpr bool isStatusByte(std::byte const midi1Byte) {
  return (midi1Byte & std::byte{0x80}) != std::byte{0x00};
}

/// \returns True if the supplied byte represents a MIDI 1.0 status code which is follow by one data byte.
constexpr bool isOneByteMessage(std::byte const midi1Byte) {
  using status_type = std::underlying_type_t<status>;
  auto const value = std::to_integer<status_type>(midi1Byte);
  auto const top_nibble = std::to_integer<status_type>(midi1Byte & std::byte{0xF0});
  return top_nibble == to_underlying(status::program_change) || top_nibble == to_underlying(status::channel_pressure) ||
         value == to_underlying(status::timing_code) || value == to_underlying(status::song_select);
}

}  // end anonymous namespace

void bytestreamToUMP::bytestreamParse(std::byte const midi1Byte) {
  auto const midi1int = static_cast<status>(midi1Byte);

  if (isStatusByte(midi1Byte)) {
    if (midi1int == status::tunerequest || isSystemRealTimeMessage(midi1Byte)) {
      if (midi1int == status::tunerequest) {
        d0_ = midi1Byte;
      }
      this->bsToUMP(midi1Byte, std::byte{0}, std::byte{0});
      return;
    }

    d0_ = midi1Byte;
    d1_ = unknown;

    if (midi1int == status::sysex_start) {
      sysex7_.state = sysex7::status::start;
      sysex7_.pos = 0;
    } else if (midi1int == status::sysex_stop) {
      using enum sysex7::status;
      types::data64::sysex7 message;
      auto& w0 = get<0>(message.w);
      auto& w1 = get<1>(message.w);
      w0.mt = to_underlying(ump_message_type::data64);
      w0.group = std::to_integer<std::uint8_t>(defaultGroup_);
      w0.status = to_underlying(sysex7_.state == start ? data64::sysex7_in_1 : data64::sysex7_end);
      w0.number_of_bytes = sysex7_.pos;
      w0.data0 = std::to_integer<std::uint8_t>(sysex7_.bytes[0]);
      w0.data1 = std::to_integer<std::uint8_t>(sysex7_.bytes[1]);
      w1.data2 = std::to_integer<std::uint8_t>(sysex7_.bytes[2]);
      w1.data3 = std::to_integer<std::uint8_t>(sysex7_.bytes[3]);
      w1.data4 = std::to_integer<std::uint8_t>(sysex7_.bytes[4]);
      w1.data5 = std::to_integer<std::uint8_t>(sysex7_.bytes[5]);
      auto const w0_32 = std::bit_cast<std::uint32_t>(w0);
      output_.push_back(w0_32);
      auto const w1_32 = std::bit_cast<std::uint32_t>(w1);
      output_.push_back(w1_32);

      sysex7_.reset();
      sysex7_.state = single_ump;
    }
  } else if (sysex7_.state == sysex7::status::start || sysex7_.state == sysex7::status::cont ||
             sysex7_.state == sysex7::status::end) {
    if (sysex7_.pos % 6 == 0 && sysex7_.pos != 0) {
      types::data64::sysex7 message;
      auto& w0 = get<0>(message.w);
      auto& w1 = get<1>(message.w);
      w0.mt = to_underlying(ump_message_type::data64);
      w0.group = std::to_integer<std::uint8_t>(defaultGroup_);
      w0.status = static_cast<std::uint8_t>(sysex7_.state);
      w0.number_of_bytes = std::uint8_t{6};
      w0.data0 = std::to_integer<std::uint8_t>(sysex7_.bytes[0]);
      w0.data1 = std::to_integer<std::uint8_t>(sysex7_.bytes[1]);
      w1.data2 = std::to_integer<std::uint8_t>(sysex7_.bytes[2]);
      w1.data3 = std::to_integer<std::uint8_t>(sysex7_.bytes[3]);
      w1.data4 = std::to_integer<std::uint8_t>(sysex7_.bytes[4]);
      w1.data5 = std::to_integer<std::uint8_t>(sysex7_.bytes[5]);
      auto const w0_32 = std::bit_cast<std::uint32_t>(w0);
      output_.push_back(w0_32);
      auto const w1_32 = std::bit_cast<std::uint32_t>(w1);
      output_.push_back(w1_32);

      sysex7_.reset();
      sysex7_.state = sysex7::status::cont;
      sysex7_.pos = 0;
    }

    sysex7_.bytes[sysex7_.pos] = midi1Byte;
    ++sysex7_.pos;
  } else if (d1_ != unknown) {  // Second byte
    this->bsToUMP(d0_, d1_, midi1Byte);
    d1_ = unknown;
  } else if (d0_ != std::byte{0}) {  // status byte set
    if (isOneByteMessage(d0_)) {
      this->bsToUMP(d0_, midi1Byte, std::byte{0});
    } else if (d0_ < std::byte{to_underlying(status::sysex_start)} || d0_ == std::byte{to_underlying(status::spp)}) {
      // This is the first of a two data byte message.
      d1_ = midi1Byte;
    }
  }
}

}  // end namespace midi2
