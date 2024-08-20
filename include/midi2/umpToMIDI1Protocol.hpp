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

#ifndef MIDI2_UMPTOMIDI1PROTOCOL_HPP
#define MIDI2_UMPTOMIDI1PROTOCOL_HPP

#include <cstdint>

#include "midi2/fifo.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

class umpToMIDI1Protocol {
public:
  [[nodiscard]] constexpr bool availableUMP() const { return !output_.empty(); }
  [[nodiscard]] std::uint32_t readUMP() { return output_.pop_front(); }
  void UMPStreamParse(uint32_t UMP);

private:
  ump_message_type mType;
  std::uint32_t ump64word1;
  std::uint8_t UMPPos = 0;
  fifo<std::uint32_t, 4> output_;
};

}  // end namespace midi2

#endif  // MIDI2_UMPTOMIDI1PROTOCOL_HPP