//===-- Bytestream To UMP -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_BYTESTREAM_TO_UMP_HPP
#define MIDI2_BYTESTREAM_TO_UMP_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "midi2/adt/fifo.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

class bytestream_to_ump {
public:
  using input_type = std::byte;
  using output_type = std::uint32_t;

  bytestream_to_ump() = default;
  explicit bytestream_to_ump(bool const outputMIDI2, std::uint8_t const defaultGroup = 0)
      : outputMIDI2_{outputMIDI2}, defaultGroup_{defaultGroup} {
    assert(defaultGroup <= 0b1111);
  }

  void set_output_midi2(bool enabled) { outputMIDI2_ = enabled; }

  [[nodiscard]] constexpr bool empty() const { return output_.empty(); }
  [[nodiscard]] constexpr output_type pop() {
    assert(!output_.empty());
    return output_.pop_front();
  }

  void push(input_type midi1Byte);

private:
  static constexpr auto unknown = std::byte{0xFF};
  bool outputMIDI2_ = false;
  std::byte defaultGroup_ = std::byte{0};

  std::byte d0_{};
  std::byte d1_ = unknown;

  struct sysex7 {
    enum class status : std::uint8_t {
      single_ump,  ///< A complete sysex message in one UMP
      start,       ///< Sysex start
      cont,        ///< Sysex continue UMP. There might be multiple 'cont' UMPs in a single message.
    };
    status state = status::single_ump;
    /// The number of system exclusive bytes in the current UMP [0,6]
    std::uint8_t pos = 0;
    /// System exclusive message bytes gathered for the current UMP
    std::array<std::byte, 6> bytes{};

    void reset() { std::ranges::fill(bytes, std::byte{0}); }
  };
  sysex7 sysex7_;
  fifo<std::uint32_t, 4> output_{};

  // Channel Based Data
  struct channel {
    std::byte bankMSB = std::byte{0xFF};
    std::byte bankLSB = std::byte{0xFF};
    bool rpnMode = true;
    std::byte rpnMsbValue = std::byte{0xFF};
    std::byte rpnMsb = std::byte{0xFF};
    std::byte rpnLsb = std::byte{0xFF};
  };
  std::array<channel, 16> channel_{};

  [[nodiscard]] static constexpr std::uint32_t pack(std::byte const b0, std::byte const b1, std::byte const b2,
                                                    std::byte const b3) {
    return (std::to_integer<std::uint32_t>(b0) << 24) | (std::to_integer<std::uint32_t>(b1) << 16) |
           (std::to_integer<std::uint32_t>(b2) << 8) | std::to_integer<std::uint32_t>(b3);
  }

  [[nodiscard]] constexpr std::uint32_t pack(ump_message_type const message_type, std::byte const b1,
                                             std::byte const b2, std::byte const b3) const {
    return pack((static_cast<std::byte>(message_type) << 4) | defaultGroup_, b1, b2, b3);
  }

  void controllerToUMP(std::byte b0, std::byte b1, std::byte b2);
  void bsToUMP(std::byte b0, std::byte b1, std::byte b2);

  template <typename T> void push_sysex7();
};

}  // end namespace midi2

#endif  // MIDI2_BYTESTREAM_TO_UMP_HPP
