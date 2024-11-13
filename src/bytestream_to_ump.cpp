//===-- Bytestream To UMP -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/bytestream_to_ump.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace {

/// \returns True if the supplied byte represents a MIDI 1.0 status code which is follow by one data byte.
constexpr bool isOneByteMessage(std::byte const midi1Byte) {
  using midi2::status;
  using status_type = std::underlying_type_t<status>;
  auto const value = std::to_integer<status_type>(midi1Byte);
  auto const top_nibble = std::to_integer<status_type>(midi1Byte & std::byte{0xF0});
  return top_nibble == to_underlying(status::program_change) || top_nibble == to_underlying(status::channel_pressure) ||
         value == to_underlying(status::timing_code) || value == to_underlying(status::song_select);
}

}  // end anonymous namespace

namespace midi2 {

void bytestream_to_ump::to_ump(std::byte b0, std::byte b1, std::byte b2) {
  assert((b0 & std::byte{0x80}) != std::byte{0} && "Top bit of b0 must be set");
  assert((b1 & std::byte{0x80}) == std::byte{0} && (b2 & std::byte{0x80}) == std::byte{0} &&
         "The top bit of b1 and b2 must be zero");

  assert(to_underlying(b0) != to_underlying(status::sysex_start));
  if (to_underlying(b0) >= to_underlying(status::sysex_start)) {
    output_.push_back(pack((static_cast<std::byte>(ump_message_type::system) << 4) | group_, b0, b1, b2));
    return;
  }
  output_.push_back(pack((static_cast<std::byte>(ump_message_type::m1cvm) << 4) | group_, b0, b1, b2));
}

template <typename T> void bytestream_to_ump::push_sysex7() {
  T t;
  static_assert(std::tuple_size_v<decltype(T::w)> == 2);
  auto& w0 = get<0>(t.w);
  auto& w1 = get<1>(t.w);
  w0.group = std::to_integer<std::uint8_t>(group_);
  w0.number_of_bytes = sysex7_.pos;
  w0.data0 = std::to_integer<std::uint8_t>(sysex7_.bytes[0]);
  w0.data1 = std::to_integer<std::uint8_t>(sysex7_.bytes[1]);
  w1.data2 = std::to_integer<std::uint8_t>(sysex7_.bytes[2]);
  w1.data3 = std::to_integer<std::uint8_t>(sysex7_.bytes[3]);
  w1.data4 = std::to_integer<std::uint8_t>(sysex7_.bytes[4]);
  w1.data5 = std::to_integer<std::uint8_t>(sysex7_.bytes[5]);
  output_.push_back(std::bit_cast<std::uint32_t>(w0));
  output_.push_back(std::bit_cast<std::uint32_t>(w1));

  sysex7_.reset();
  sysex7_.state = sysex7::status::none;
}

void bytestream_to_ump::push(std::byte const midi1Byte) {
  auto const midi1int = static_cast<status>(midi1Byte);

  if (is_status_byte(midi1Byte)) {
    if (midi1int == status::tune_request || is_system_real_time_message(midi1Byte)) {
      if (midi1int == status::tune_request) {
        d0_ = midi1Byte;
      }
      this->to_ump(midi1Byte, std::byte{0}, std::byte{0});
      return;
    }

    d0_ = midi1Byte;
    d1_ = unknown;

    switch (sysex7_.state) {
    case sysex7::status::start: this->push_sysex7<types::data64::sysex7_in_1>(); break;
    case sysex7::status::cont: this->push_sysex7<types::data64::sysex7_end>(); break;
    case sysex7::status::none:
    default: break;
    }

    if (midi1int == status::sysex_start) {
      sysex7_.state = sysex7::status::start;
      sysex7_.pos = 0;
    }
  } else {
    // Data byte handling.
    if (sysex7_.state == sysex7::status::start || sysex7_.state == sysex7::status::cont) {
      if (sysex7_.pos % 6 == 0 && sysex7_.pos != 0) {
        switch (sysex7_.state) {
        case sysex7::status::start: push_sysex7<types::data64::sysex7_start>(); break;
        case sysex7::status::cont: push_sysex7<types::data64::sysex7_continue>(); break;
        default: assert(false); break;
        }
        sysex7_.reset();
        sysex7_.state = sysex7::status::cont;
        sysex7_.pos = 0;
      }
      sysex7_.bytes[sysex7_.pos] = midi1Byte;
      ++sysex7_.pos;
    } else if (d1_ != unknown) {  // Second byte
      this->to_ump(d0_, d1_, midi1Byte);
      d1_ = unknown;
    } else if (d0_ != std::byte{0}) {  // status byte set
      if (isOneByteMessage(d0_)) {
        this->to_ump(d0_, midi1Byte, std::byte{0});
      } else if (d0_ < std::byte{to_underlying(status::sysex_start)} || d0_ == std::byte{to_underlying(status::spp)}) {
        // This is the first of a two data byte message.
        d1_ = midi1Byte;
      }
    }
  }
}

}  // end namespace midi2
