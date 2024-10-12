//===-- UMP to Bytestream -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMPTOBYTESTREAM_HPP
#define MIDI2_UMPTOBYTESTREAM_HPP

#include <cstddef>
#include <cstdint>

#include "midi2/fifo.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

class umpToBytestream {
public:
  std::uint8_t group = 0;

  umpToBytestream() = default;

  [[nodiscard]] constexpr bool availableBS() const { return !output_.empty(); }
  [[nodiscard]] std::byte readBS() { return output_.pop_front(); }

  void UMPStreamParse(uint32_t UMP);

private:
  ump_message_type mType_ = ump_message_type::utility;
  std::uint32_t ump64word1_ = 0;

  std::uint8_t UMPPos_ = 0;
  fifo<std::byte, 16> output_;

  void word1(std::uint32_t ump);
  void word2(std::uint32_t ump);
  void word3(std::uint32_t ump);
};

}  // end namespace midi2

#endif  // MIDI2_UMPTOBYTESTREAM_HPP
