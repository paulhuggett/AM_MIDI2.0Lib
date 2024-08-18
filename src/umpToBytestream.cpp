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

void umpToBytestream::word1(uint32_t UMP) {
  // First part of a UMP Message
  mType = static_cast<ump_message_type>(UMP >> 28);
  group = UMP >> 24 & 0xF;
  switch (mType) {
  case ump_message_type::utility:  // 32 bits Utility Messages
  case ump_message_type::reserved32_06:
  case ump_message_type::reserved32_07: break;
  // 32 bits System Real Time and System Common Messages (except System
  // Exclusive)
  case ump_message_type::system: {
    uint8_t const sysByte = (UMP >> 16) & 0xFF;
    if (sysByte == 0xF4 || sysByte == 0xF5 || sysByte == 0xFD ||
        sysByte == 0xF9) {
      break;
    }
    output_.push_back(sysByte);
    if (sysByte == 0xF1 || sysByte == 0xF2 || sysByte == 0xF3) {
      output_.push_back((UMP >> 8) & 0x7F);
      if (sysByte == 0xF2) {
        output_.push_back(UMP & 0x7F);
      }
    }
    break;
  }
    // 32 Bits MIDI 1.0 Channel Voice Message
  case ump_message_type::m1cvm: {
    uint8_t const stsCh = UMP >> 16 & 0xFF;
    output_.push_back(stsCh);
    output_.push_back((UMP >> 8) & 0x7F);
    if (stsCh >> 4 != 0xC && stsCh >> 4 != 0xD) {
      output_.push_back(UMP & 0x7F);
    }
    break;
  }
    // 64 bits Data Messages (including System Exclusive)
  case ump_message_type::sysex7:
    // MIDI2.0 Channel Voice Messages
  case ump_message_type::m2cvm:
    ump64word1 = UMP;
    UMPPos++;
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
  default: UMPPos++; break;
  }
}

void umpToBytestream::word2(std::uint32_t UMP) {
  switch (mType) {
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
    UMPPos = 0;
    break;
    // 64 bits Data Messages (including System Exclusive) part 2
  case ump_message_type::sysex7: {
    UMPPos = 0;
    uint8_t const status = (ump64word1 >> 20) & 0xF;
    uint8_t const numSysexBytes = (ump64word1 >> 16) & 0xF;

    if (status <= 1) {
      output_.push_back(sysex_start);
    }
    if (numSysexBytes > 0) {
      output_.push_back((ump64word1 >> 8) & 0x7F);
    }
    if (numSysexBytes > 1) {
      output_.push_back(ump64word1 & 0x7F);
    }
    if (numSysexBytes > 2) {
      output_.push_back((UMP >> 24) & 0x7F);
    }
    if (numSysexBytes > 3) {
      output_.push_back((UMP >> 16) & 0x7F);
    }
    if (numSysexBytes > 4) {
      output_.push_back((UMP >> 8) & 0x7F);
    }
    if (numSysexBytes > 5) {
      output_.push_back(UMP & 0x7F);
    }
    if (status == 0 || status == 3) {
      output_.push_back(sysex_stop);
    }
    break;
  }
  case ump_message_type::m2cvm: {
    UMPPos = 0;
    uint8_t const status = (ump64word1 >> 16) & 0xF0;
    uint8_t const channel = (ump64word1 >> 16) & 0xF;
    uint8_t const val1 = (ump64word1 >> 8) & 0xFF;
    uint8_t const val2 = ump64word1 & 0xFF;

    switch (status) {
    case note_off:
    case note_on: {
      auto velocity = static_cast<std::uint8_t>(scaleDown((UMP >> 16), 16, 7));
      if (velocity == 0 && status == note_on) {
        velocity = 1;
      }
      output_.push_back((ump64word1 >> 16) & 0xFF);
      output_.push_back(val1);
      output_.push_back(velocity);
      break;
    }
    case status::key_pressure:
    case status::cc:
      output_.push_back((ump64word1 >> 16) & 0xFF);
      output_.push_back(val1);
      output_.push_back(static_cast<std::uint8_t>(scaleDown(UMP, 32, 7)));
      break;
    case status::channel_pressure:
      output_.push_back((ump64word1 >> 16) & 0xFF);
      output_.push_back(static_cast<std::uint8_t>(scaleDown(UMP, 32, 7)));
      break;
    case midi2status::rpn: {
      output_.push_back(status::cc + channel);
      output_.push_back(101);
      output_.push_back(val1);
      output_.push_back(status::cc + channel);
      output_.push_back(100);
      output_.push_back(val2);

      auto const val14bit = static_cast<std::uint16_t>(scaleDown(UMP, 32, 14));
      output_.push_back(status::cc + channel);
      output_.push_back(6);
      output_.push_back((val14bit >> 7) & 0x7F);
      output_.push_back(status::cc + channel);
      output_.push_back(38);
      output_.push_back(val14bit & 0x7F);
      break;
    }
    case midi2status::nrpn: {  // nrpn
      output_.push_back(status::cc + channel);
      output_.push_back(99);
      output_.push_back(val1);
      output_.push_back(status::cc + channel);
      output_.push_back(98);
      output_.push_back(val2);

      auto const val14bit = static_cast<std::uint16_t>(scaleDown(UMP, 32, 14));
      output_.push_back(status::cc + channel);
      output_.push_back(6);
      output_.push_back((val14bit >> 7) & 0x7F);
      output_.push_back(status::cc + channel);
      output_.push_back(38);
      output_.push_back(val14bit & 0x7F);
      break;
    }
    case status::program_change: {  // Program change
      if (ump64word1 & 0x1) {
        output_.push_back(status::cc + channel);
        output_.push_back(0);
        output_.push_back((UMP >> 8) & 0x7F);

        output_.push_back(status::cc + channel);
        output_.push_back(32);
        output_.push_back(UMP & 0x7F);
      }
      output_.push_back(status::program_change + channel);
      output_.push_back((UMP >> 24) & 0x7F);
      break;
    }
    case status::pitch_bend:
      output_.push_back((ump64word1 >> 16) & 0xFF);
      output_.push_back((UMP >> 18) & 0x7F);
      output_.push_back((UMP >> 25) & 0x7F);
      break;
    default:
      // An unknown message.
      break;
    }
    break;
  }
  default: UMPPos++; break;
  }
}

void umpToBytestream::word3(std::uint32_t /*UMP*/) {
  switch (mType) {
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
  case ump_message_type::system: assert(false); break;
  case ump_message_type::reserved96_0B:  // 96 Reserved
  case ump_message_type::reserved96_0C:  // 96 Reserved
    UMPPos = 0;
    break;
  default: UMPPos++; break;
  }
}

void umpToBytestream::UMPStreamParse(uint32_t UMP) {
  switch (UMPPos) {
  case 0: this->word1(UMP); break;
  case 1:
    // 64Bit+ Messages only
    this->word2(UMP);
    break;
  case 2: this->word3(UMP); break;
  default: UMPPos = 0; break;
  }
}

}  // end namespace midi2
