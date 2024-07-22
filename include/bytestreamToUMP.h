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
 * ********************************************************/

#ifndef BSUMP_H
#define BSUMP_H

#include <array>
#include <cassert>
#include <cstdint>

#include "fifo.h"
#include "utils.h"

class bytestreamToUMP {
public:
  bytestreamToUMP() = default;
  explicit bytestreamToUMP(bool const outputMIDI2,
                           std::uint8_t const defaultGroup = 0)
      : outputMIDI2_{outputMIDI2}, defaultGroup_{defaultGroup} {
    assert(defaultGroup <= 0b1111);
  }

  constexpr bool availableUMP() const { return !output_.empty(); }
  std::uint32_t readUMP() {
    assert(!output_.empty());
    return output_.pop_front();
  }

  void bytestreamParse(std::uint8_t midi1Byte);

private:
  static constexpr auto unknown = std::uint8_t{0xFF};
  bool outputMIDI2_ = false;
  std::uint8_t defaultGroup_ = 0;

  std::uint8_t d0_ = 0;
  std::uint8_t d1_ = unknown;

  struct sysex7 {
    enum class status : std::uint8_t {
      /// A complete system exclusive message in one UMP
      single_ump = 0x0,
      /// System exclusive Start UMP
      start = 0x1,
      /// System exclusive continue UMP. There might be multiple 'cont' UMPs in
      /// a single message.
      cont = 0x02,
      /// System exclusive end UMP
      end = 0x03,
    };
    status state = status::single_ump;
    /// The number of system exclusive bytes in the current UMP [0,6]
    std::uint8_t pos = 0;
    /// System exclusive message bytes gathered for the current UMP
    std::array<std::uint8_t, 6> bytes{};
  };
  sysex7 sysex7_;
  M2Utils::fifo<std::uint32_t, 4> output_;

  void reset_sysex() {
    std::fill(std::begin(sysex7_.bytes), std::end(sysex7_.bytes),
              std::uint8_t{0});
  }

  // Channel Based Data
  struct channel {
    std::uint8_t bankMSB = 0xFF;
    std::uint8_t bankLSB = 0xFF;
    bool rpnMode = true;
    std::uint8_t rpnMsbValue = 0xFF;
    std::uint8_t rpnMsb = 0xFF;
    std::uint8_t rpnLsb = 0xFF;
  };
  std::array<channel, 16> channel_;

  static constexpr std::uint32_t pack(std::uint8_t const b0,
                                      std::uint8_t const b1,
                                      std::uint8_t const b2,
                                      std::uint8_t const b3) {
    return (static_cast<std::uint32_t>(b0) << 24) |
           (static_cast<std::uint32_t>(b1) << 16) |
           (static_cast<std::uint32_t>(b2) << 8) |
           static_cast<std::uint32_t>(b3);
  }

  constexpr std::uint32_t pack(ump_message_type const message_type,
                               std::uint8_t const b1, std::uint8_t const b2,
                               std::uint8_t const b3) {
    return pack(
        static_cast<std::uint8_t>(
            (static_cast<std::uint8_t>(message_type) << 4) | defaultGroup_),
        b1, b2, b3);
  }

  void controllerToUMP(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2);
  void bsToUMP(std::uint8_t b0, std::uint8_t b1, std::uint8_t b2);
};

#endif
