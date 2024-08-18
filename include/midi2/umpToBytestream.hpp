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

#ifndef MIDI2_UMPTOBYTESTREAM_HPP
#define MIDI2_UMPTOBYTESTREAM_HPP

#include <cstdint>

#include "midi2/fifo.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

class umpToBytestream {
public:
  uint8_t group = 0;

  umpToBytestream() = default;

  [[nodiscard]] constexpr bool availableBS() const { return !output_.empty(); }
  [[nodiscard]] uint8_t readBS() { return output_.pop_front(); }

  void UMPStreamParse(uint32_t UMP);

private:
  ump_message_type mType = ump_message_type::utility;
  std::uint32_t ump64word1 = 0;

  std::uint8_t UMPPos = 0;
  fifo<std::uint8_t, 16> output_;

  void word1(std::uint32_t ump);
  void word2(std::uint32_t ump);
  void word3(std::uint32_t ump);
};

}  // end namespace midi2

#endif  // MIDI2_UMPTOBYTESTREAM_HPP
