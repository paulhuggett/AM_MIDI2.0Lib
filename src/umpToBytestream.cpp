/**********************************************************
 * MIDI 2.0 Library
 * Author: Andrew Mee
 *
 * MIT License
 * Copyright 2022 Andrew Mee
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

#include "midi2/umpToBytestream.hpp"

#include <cassert>
#include <cstdint>

#include "midi2/utils.hpp"

namespace midi2 {

void umpToBytestream::word1(std::uint32_t const ump) {
  // First part of a UMP Message
  mType_ = static_cast<ump_message_type>(ump >> 28);
  group = ump >> 24 & 0xF;
  switch (mType_) {
  case ump_message_type::utility:  // 32 bits Utility Messages
  case ump_message_type::reserved32_06:
  case ump_message_type::reserved32_07: break;
  // 32 bits System Real Time and System Common Messages (except System
  // Exclusive)
  case ump_message_type::system: {
    auto const sysByte = static_cast<std::byte>((ump >> 16) & 0xFF);
    if (sysByte == std::byte{status::reserved1} || sysByte == std::byte{status::reserved2} ||
        sysByte == std::byte{status::reserved3} || sysByte == std::byte{status::reserved4}) {
      break;
    }
    output_.push_back(sysByte);
    if (sysByte == std::byte{status::timing_code} || sysByte == std::byte{status::spp} ||
        sysByte == std::byte{status::song_select}) {
      output_.push_back(static_cast<std::byte>((ump >> 8) & 0x7F));
      if (sysByte == std::byte{status::spp}) {
        output_.push_back(static_cast<std::byte>(ump & 0x7F));
      }
    }
    break;
  }
    // 32 Bits MIDI 1.0 Channel Voice Message
  case ump_message_type::m1cvm: {
    auto const stsCh = static_cast<std::byte>((ump >> 16) & 0xFF);
    output_.push_back(stsCh);
    output_.push_back(static_cast<std::byte>((ump >> 8) & 0x7F));
    if ((stsCh >> 4) != std::byte{0xC} && (stsCh >> 4) != std::byte{0xD}) {
      output_.push_back(static_cast<std::byte>(ump & 0x7F));
    }
    break;
  }
    // 64 bits Data Messages (including System Exclusive)
  case ump_message_type::sysex7:
    // MIDI2.0 Channel Voice Messages
  case ump_message_type::m2cvm:
    ump64word1_ = ump;
    UMPPos_++;
    break;
  case ump_message_type::data:
  case ump_message_type::reserved64_08:
  case ump_message_type::reserved64_09:
  case ump_message_type::reserved64_0A:
  case ump_message_type::reserved96_0B:
  case ump_message_type::reserved96_0C:
  case ump_message_type::reserved128_0E:
  case ump_message_type::flex_data:
  case ump_message_type::midi_endpoint:
  default: UMPPos_++; break;
  }
}

void umpToBytestream::word2(std::uint32_t UMP) {
  switch (mType_) {
  case ump_message_type::utility:
  case ump_message_type::m1cvm:
  case ump_message_type::data:
  case ump_message_type::reserved32_06:
  case ump_message_type::reserved32_07:
  case ump_message_type::reserved96_0B:
  case ump_message_type::reserved96_0C:
  case ump_message_type::flex_data:
  case ump_message_type::reserved128_0E:
  case ump_message_type::midi_endpoint:
  case ump_message_type::system: assert(false); break;
  case ump_message_type::reserved64_08:  // 64 Reserved
  case ump_message_type::reserved64_09:  // 64 Reserved
  case ump_message_type::reserved64_0A:  // 64 Reserved
    UMPPos_ = 0;
    break;

  case ump_message_type::sysex7: {
    UMPPos_ = 0;
    auto const status = (ump64word1_ >> 20) & 0x0F;
    auto const numSysexBytes = (ump64word1_ >> 16) & 0x0F;

    if (status <= 1) {
      output_.push_back(std::byte{sysex_start});
    }
    if (numSysexBytes > 0) {
      output_.push_back(static_cast<std::byte>((ump64word1_ >> 8) & 0x7F));
    }
    if (numSysexBytes > 1) {
      output_.push_back(static_cast<std::byte>(ump64word1_ & 0x7F));
    }
    if (numSysexBytes > 2) {
      output_.push_back(static_cast<std::byte>((UMP >> 24) & 0x7F));
    }
    if (numSysexBytes > 3) {
      output_.push_back(static_cast<std::byte>((UMP >> 16) & 0x7F));
    }
    if (numSysexBytes > 4) {
      output_.push_back(static_cast<std::byte>((UMP >> 8) & 0x7F));
    }
    if (numSysexBytes > 5) {
      output_.push_back(static_cast<std::byte>(UMP & 0x7F));
    }
    if (status == 0 || status == 3) {
      output_.push_back(std::byte{sysex_stop});
    }
    break;
  }
  case ump_message_type::m2cvm: {
    UMPPos_ = 0;
    std::uint8_t const status = (ump64word1_ >> 16) & 0xF0;
    auto const channel = static_cast<std::byte>((ump64word1_ >> 16) & 0x0F);
    auto const val1 = static_cast<std::byte>((ump64word1_ >> 8) & 0xFF);
    auto const val2 = static_cast<std::byte>(ump64word1_ & 0xFF);

    switch (status) {
    case note_off:
    case note_on: {
      auto velocity = static_cast<std::byte>(scaleDown((UMP >> 16), 16, 7));
      if (velocity == std::byte{0} && status == note_on) {
        velocity = std::byte{1};
      }
      output_.push_back(static_cast<std::byte>((ump64word1_ >> 16) & 0xFF));
      output_.push_back(val1);
      output_.push_back(velocity);
      break;
    }
    case status::key_pressure:
    case status::cc:
      output_.push_back(static_cast<std::byte>((ump64word1_ >> 16) & 0xFF));
      output_.push_back(val1);
      output_.push_back(static_cast<std::byte>(scaleDown(UMP, 32, 7)));
      break;
    case status::channel_pressure:
      output_.push_back(static_cast<std::byte>((ump64word1_ >> 16) & 0xFF));
      output_.push_back(static_cast<std::byte>(scaleDown(UMP, 32, 7)));
      break;
    case midi2status::rpn: {
      output_.push_back(std::byte{status::cc} | channel);
      output_.push_back(std::byte{101});
      output_.push_back(val1);
      output_.push_back(std::byte{status::cc} | channel);
      output_.push_back(std::byte{100});
      output_.push_back(val2);

      auto const val14bit = static_cast<std::uint16_t>(scaleDown(UMP, 32, 14));
      output_.push_back(std::byte{status::cc} | channel);
      output_.push_back(std::byte{6});
      output_.push_back(static_cast<std::byte>((val14bit >> 7) & 0x7F));
      output_.push_back(std::byte{status::cc} | channel);
      output_.push_back(std::byte{38});
      output_.push_back(static_cast<std::byte>(val14bit & 0x7F));
      break;
    }
    case midi2status::nrpn: {  // nrpn
      output_.push_back(std::byte{status::cc} | channel);
      output_.push_back(std::byte{99});
      output_.push_back(val1);
      output_.push_back(std::byte{status::cc} | channel);
      output_.push_back(std::byte{98});
      output_.push_back(val2);

      auto const val14bit = static_cast<std::uint16_t>(scaleDown(UMP, 32, 14));
      output_.push_back(std::byte{status::cc} | channel);
      output_.push_back(std::byte{6});
      output_.push_back(static_cast<std::byte>((val14bit >> 7) & 0x7F));
      output_.push_back(std::byte{status::cc} | channel);
      output_.push_back(std::byte{38});
      output_.push_back(static_cast<std::byte>(val14bit & 0x7F));
      break;
    }
    case status::program_change:
      if (ump64word1_ & 0x1) {
        output_.push_back(std::byte{status::cc} | channel);
        output_.push_back(std::byte{0});
        output_.push_back(static_cast<std::byte>((UMP >> 8) & 0x7F));

        output_.push_back(std::byte{status::cc} | channel);
        output_.push_back(std::byte{32});
        output_.push_back(static_cast<std::byte>(UMP & 0x7F));
      }
      output_.push_back(std::byte{status::program_change} | channel);
      output_.push_back(static_cast<std::byte>((UMP >> 24) & 0x7F));
      break;
    case status::pitch_bend:
      output_.push_back(static_cast<std::byte>((ump64word1_ >> 16) & 0xFF));
      output_.push_back(static_cast<std::byte>((UMP >> 18) & 0x7F));
      output_.push_back(static_cast<std::byte>((UMP >> 25) & 0x7F));
      break;
    default:
      // An unknown message.
      break;
    }
    break;
  }
  default: UMPPos_++; break;
  }
}

void umpToBytestream::word3(std::uint32_t /*UMP*/) {
  switch (mType_) {
  case ump_message_type::reserved96_0B:
  case ump_message_type::reserved96_0C: UMPPos_ = 0; break;
  case ump_message_type::reserved64_09:
  case ump_message_type::reserved64_0A:
  case ump_message_type::utility:
  case ump_message_type::sysex7:
  case ump_message_type::m2cvm:
  case ump_message_type::reserved64_08:
  case ump_message_type::m1cvm:
  case ump_message_type::data:
  case ump_message_type::reserved32_06:
  case ump_message_type::reserved32_07:
  case ump_message_type::flex_data:
  case ump_message_type::reserved128_0E:
  case ump_message_type::midi_endpoint:
  case ump_message_type::system: assert(false); unreachable();
  default: UMPPos_++; break;
  }
}

void umpToBytestream::UMPStreamParse(std::uint32_t UMP) {
  switch (UMPPos_) {
  case 0: this->word1(UMP); break;
  case 1: this->word2(UMP); break;  // 64Bit+ Messages only
  case 2: this->word3(UMP); break;
  default: UMPPos_ = 0; break;
  }
}

}  // end namespace midi2
