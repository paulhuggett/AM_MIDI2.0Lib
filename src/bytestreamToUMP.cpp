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

#include "bytestreamToUMP.h"

void bytestreamToUMP::controllerToUMP(std::uint8_t const b0,
                                      std::uint8_t const b1,
                                      std::uint8_t const b2) {
  std::uint8_t const channel = b0 & 0x0F;
  auto& c = channel_[channel];
  switch (b1) {
  case 0: c.bankMSB = b2; break;
  case 32: c.bankLSB = b2; break;

  case 6:  // RPN MSB Value
    if (c.rpnMsb != 0xFF && c.rpnLsb != 0xFF) {
      if (c.rpnMode && c.rpnMsb == 0 && (c.rpnLsb == 0 || c.rpnLsb == 6)) {
        output_.push_back(pack(ump_message_type::m2cvm,
                               (c.rpnMode ? rpn : nrpn) | channel, c.rpnMsb,
                               c.rpnLsb));
        output_.push_back(
            M2Utils::scaleUp(static_cast<std::uint32_t>(b2) << 7, 14, 32));
      } else {
        c.rpnMsbValue = b2;
      }
    }
    break;
  case 38:
    // RPN LSB Value
    if (c.rpnMsb != 0xFF && c.rpnLsb != 0xFF) {
      output_.push_back(pack(ump_message_type::m2cvm,
                             (c.rpnMode ? rpn : nrpn) | channel, c.rpnMsb,
                             c.rpnLsb));
      output_.push_back(M2Utils::scaleUp(
          (static_cast<std::uint32_t>(c.rpnMsbValue) << 7) | b2, 14, 32));
    }
    break;
  case 99:
    c.rpnMode = false;
    c.rpnMsb = b2;
    break;
  case 98:
    c.rpnMode = false;
    c.rpnLsb = b2;
    break;
  case 101:
    c.rpnMode = true;
    c.rpnMsb = b2;
    break;
  case 100:
    c.rpnMode = true;
    c.rpnLsb = b2;
    break;
  default:
    output_.push_back(pack(ump_message_type::m2cvm, b0, b1, 0));
    output_.push_back(M2Utils::scaleUp(b2, 7, 32));
    break;
  }
}

void bytestreamToUMP::bsToUMP(std::uint8_t b0, std::uint8_t b1,
                              std::uint8_t b2) {
  using M2Utils::scaleUp;
  std::uint8_t const channel = b0 & 0x0F;
  std::uint8_t status = b0 & 0xF0;

  if (b0 >= status::timing_code) {
    output_.push_back(pack(ump_message_type::system, b0, b1, b2));
  } else if (status >= status::note_off && status <= status::pitch_bend) {
    if (!outputMIDI2_) {
      output_.push_back(pack(ump_message_type::m1cvm, b0, b1, b2));
    } else {
      if (status == status::note_on && b2 == 0) {
        status = status::note_off;
        b0 = status | channel;
        b2 = 0x40;
      }

      auto message = pack(ump_message_type::m2cvm, status | channel, 0, 0);
      switch (status) {
      case status::note_on:
      case status::note_off:
      case status::key_pressure:
        output_.push_back(message | static_cast<std::uint32_t>((b1 << 8)));
        output_.push_back(scaleUp(b2, 7, 16) << 16);
        break;
      case status::pitch_bend:
        output_.push_back(message);
        output_.push_back(
            scaleUp(static_cast<std::uint32_t>((b1 << 7) | b2), 14, 32));
        break;
      case status::program_change: {
        auto bank_msb = std::uint8_t{0};
        auto bank_lsb = std::uint8_t{0};
        if (channel_[channel].bankMSB != 0xFF &&
            channel_[channel].bankLSB != 0xFF) {
          message |= 0x01U;
          bank_msb = channel_[channel].bankMSB;
          bank_lsb = channel_[channel].bankLSB;
        }
        output_.push_back(message);
        output_.push_back(pack(b1, 0, bank_msb, bank_lsb));
      } break;
      case status::channel_pressure:
        output_.push_back(message);
        output_.push_back(scaleUp(b1, 7, 32));
        break;
      case status::cc: this->controllerToUMP(b0, b1, b2); break;
      default:
        // Unknown message
        break;
      }
    }
  }
}

void bytestreamToUMP::bytestreamParse(std::uint8_t const midi1Byte) {
  if (midi1Byte == status::tunerequest || midi1Byte >= status::timingclock) {
    d0_ = midi1Byte;
    this->bsToUMP(midi1Byte, 0, 0);
  }

  if ((midi1Byte & 0x80) != 0x00) {  // Status byte received
    d0_ = midi1Byte;
    d1_ = unknown;

    if (midi1Byte == status::sysex_start) {
      sysex7_.state = sysex7::status::start;
      sysex7_.pos = 0;
    } else if (midi1Byte == status::sysex_stop) {
      output_.push_back(
          pack(ump_message_type::sysex7,
               static_cast<std::uint8_t>(
                   (sysex7_.state == sysex7::status::start ? 0 : 3) << 4) |
                   sysex7_.pos,
               sysex7_.bytes[0], sysex7_.bytes[1]));
      output_.push_back(pack(sysex7_.bytes[2], sysex7_.bytes[3],
                             sysex7_.bytes[4], sysex7_.bytes[5]));
      sysex7_.state = sysex7::status::single_ump;
      reset_sysex();
    }
  } else if (sysex7_.state == sysex7::status::start ||
             sysex7_.state == sysex7::status::cont ||
             sysex7_.state == sysex7::status::end) {
    // Check for new UMP Message Type 3
    if (sysex7_.pos % 6 == 0 && sysex7_.pos != 0) {
      static constexpr auto num_sysex_bytes = std::uint8_t{6};
      output_.push_back(
          pack(ump_message_type::sysex7,
               static_cast<std::uint8_t>(
                   (static_cast<std::uint8_t>(sysex7_.state) << 4U) |
                   num_sysex_bytes),
               sysex7_.bytes[0], sysex7_.bytes[1]));
      output_.push_back(pack(sysex7_.bytes[2], sysex7_.bytes[3],
                             sysex7_.bytes[4], sysex7_.bytes[5]));

      reset_sysex();
      sysex7_.state = sysex7::status::cont;
      sysex7_.pos = 0;
    }

    sysex7_.bytes[sysex7_.pos] = midi1Byte;
    sysex7_.pos++;
  } else if (d1_ != unknown) {  // Second byte
    this->bsToUMP(d0_, d1_, midi1Byte);
    d1_ = unknown;
  } else if (d0_ != 0) {  // status byte set
    if ((d0_ & 0xF0) == status::program_change ||
        (d0_ & 0xF0) == status::channel_pressure ||
        d0_ == status::timing_code || d0_ == status::song_select) {
      this->bsToUMP(d0_, midi1Byte, 0);
    } else if (d0_ < status::sysex_start ||
               d0_ == status::spp) {  // First data byte
      d1_ = midi1Byte;
    }
  }
}
