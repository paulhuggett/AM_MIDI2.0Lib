/**********************************************************
 * MIDI 2.0 Library
 * Author: Andrew Mee
 *
 * MIT License
 * Copyright 2024 Andrew Mee
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

#include "umpToMIDI1Protocol.h"

#include "umpMessageCreate.h"
#include "utils.h"

void umpToMIDI1Protocol::UMPStreamParse(uint32_t UMP) {
  switch (UMPPos) {
  case 0: {  // First UMP Packet
    // First part of a UMP Message
    mType = static_cast<ump_message_type>(UMP >> 28);
    switch (mType) {
    case ump_message_type::utility:        // 32 bits Utility Messages
    case ump_message_type::reserved32_06:  // 32 Reserved
    case ump_message_type::reserved32_07:  // 32 Reserved
      break;
      // 32-bit MIDI 1.0 Channel Voice Messages
    case ump_message_type::m1cvm:
      // 32-bit System Real Time and System Common Messages (except
      // System Exclusive)
    case ump_message_type::system:
      output_.push_back(UMP);
      break;

      // 64-bit Data Messages (including System Exclusive)
    case ump_message_type::sysex7:
      // MIDI2.0 Channel Voice Messages
    case ump_message_type::m2cvm:
      ump64word1 = UMP;
      UMPPos++;
      break;
    default: UMPPos++; break;
    }
    break;
  }
  case 1: {  // 64Bit+ Messages only
    switch (mType) {
    case ump_message_type::reserved64_08:  // 64 Reserved
    case ump_message_type::reserved64_09:  // 64 Reserved
    case ump_message_type::reserved64_0A:  // 64 Reserved
      UMPPos = 0;
      break;
      // 64 bits Data Messages (including System Exclusive) part 2
    case ump_message_type::sysex7: {
      UMPPos = 0;
      output_.push_back(ump64word1);
      output_.push_back(UMP);
      break;
    }
    case ump_message_type::m2cvm: {
      UMPPos = 0;
      uint8_t status = (ump64word1 >> 16) & 0xFF;
      uint8_t channel = (ump64word1 >> 16) & 0xF;
      uint8_t val1 = (ump64word1 >> 8) & 0xFF;
      uint8_t val2 = ump64word1 & 0xFF;
      uint8_t group = (ump64word1 >> 24) & 0xF;

      switch (status) {
      case note_off:
        output_.push_back(UMPMessage::mt2NoteOff(
            group, channel, val1,
            static_cast<std::uint8_t>(M2Utils::scaleDown((UMP >> 16), 16, 7))));
        break;
      case note_on: {
        uint8_t velocity = (uint8_t)M2Utils::scaleDown((UMP >> 16), 16, 7);
        if (velocity == 0) {
          velocity = 1;
        }
        output_.push_back(
            UMPMessage::mt2NoteOn(group, channel, val1, velocity));
        break;
      }
      case key_pressure:
        output_.push_back(UMPMessage::mt2PolyPressure(
            group, channel, val1,
            static_cast<std::uint8_t>(M2Utils::scaleDown(UMP, 32, 7))));
        break;
      case cc:
        output_.push_back(UMPMessage::mt2CC(
            group, channel, val1,
            static_cast<std::uint8_t>(M2Utils::scaleDown(UMP, 32, 7))));
        break;
      case channel_pressure:
        output_.push_back(UMPMessage::mt2ChannelPressure(
            group, channel,
            static_cast<std::uint8_t>(M2Utils::scaleDown(UMP, 32, 7))));
        break;
      case nrpn: {
        output_.push_back(UMPMessage::mt2CC(group, channel, 99, val1));
        output_.push_back(UMPMessage::mt2CC(group, channel, 98, val2));
        auto const val14bit =
            static_cast<std::uint16_t>(M2Utils::scaleDown(UMP, 32, 14));
        output_.push_back(
            UMPMessage::mt2CC(group, channel, 6, (val14bit >> 7) & 0x7F));
        output_.push_back(
            UMPMessage::mt2CC(group, channel, 38, val14bit & 0x7F));
        break;
      }
      case rpn: {
        output_.push_back(UMPMessage::mt2CC(group, channel, 101, val1));
        output_.push_back(UMPMessage::mt2CC(group, channel, 100, val2));
        auto const val14bit =
            static_cast<std::uint16_t>(M2Utils::scaleDown(UMP, 32, 14));
        output_.push_back(
            UMPMessage::mt2CC(group, channel, 6, (val14bit >> 7) & 0x7F));
        output_.push_back(
            UMPMessage::mt2CC(group, channel, 38, val14bit & 0x7F));
        break;
      }
      case program_change: {
        if (ump64word1 & 0x1) {
          output_.push_back(
              UMPMessage::mt2CC(group, channel, 0, (UMP >> 8) & 0x7F));
          output_.push_back(UMPMessage::mt2CC(group, channel, 32, UMP & 0x7F));
        }
        output_.push_back(
            UMPMessage::mt2ProgramChange(group, channel, (UMP >> 24) & 0x7F));
        break;
      }
      case pitch_bend:
        output_.push_back(UMPMessage::mt2PitchBend(group, channel, UMP >> 18));
        break;
      }
      break;
    }
    default: UMPPos++; break;
    }
    break;
  }
  case 2: {
    switch (mType) {
    case ump_message_type::reserved96_0B:  // 96 Reserved
    case ump_message_type::reserved96_0C:  // 96 Reserved
      UMPPos = 0;
      break;
    default: UMPPos++; break;
    }
    break;
  }
  case 3: {
    UMPPos = 0;
    break;
  }
  }
}
