//===-- USB MIDI1.0 to Bytestream ---------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <cassert>
#include <cstddef>
#include <cstdint>

#include "midi2/adt/fifo.hpp"

namespace midi2 {

class usbm1_to_bytestream {
public:
  explicit constexpr usbm1_to_bytestream(std::uint8_t cable) : cable_{cable} {
    assert(cable < 16U && "cable number must be four bits");
  }

  [[nodiscard]] constexpr bool available() const { return !output_.empty(); }
  [[nodiscard]] std::byte read() { return output_.pop_front(); }

  void receive(std::uint32_t const usbm1) {
    if (cable(usbm1) != cable_) {
      return;
    }
    auto const bytes = midi_x_size(get_cin(usbm1));
    assert(bytes < 4);
    if (bytes > 0) {
      output_.push_back(static_cast<std::byte>((usbm1 >> 16) & 0xFF));
    }
    if (bytes > 1) {
      output_.push_back(static_cast<std::byte>((usbm1 >> 8) & 0xFF));
    }
    if (bytes > 2) {
      output_.push_back(static_cast<std::byte>(usbm1 & 0xFF));
    }
  }

private:
  std::uint8_t cable_;
  fifo<std::byte, 4> output_;

  static constexpr std::uint8_t cable(std::uint32_t const p) noexcept { return (p >> 28) & 0x0F; }
  static constexpr std::uint8_t get_cin(std::uint32_t const p) noexcept { return (p >> 24) & 0x0F; }
  static constexpr unsigned midi_x_size(std::uint8_t const cin) noexcept {
    assert(cin < 0x10U);
    switch (cin) {
    case 0x00: return 0;  // Reserved for future extension
    case 0x01: return 0;  // Reserved for future expansion
    case 0x02: return 2;  // Two-byte System Common messages
    case 0x03: return 3;  // Three-byte System Common messages
    case 0x04: return 3;  // SysEx starts or continues
    case 0x05: return 1;  // Single-byte System Common/SysEx end Message
    case 0x06: return 2;  // SysEx ends with following two bytes
    case 0x07: return 3;  // SysEx ends with following three bytes
    case 0x08: return 3;  // Note-off
    case 0x09: return 3;  // Note-on
    case 0x0A: return 3;  // Poly Key Press
    case 0x0B: return 3;  // Control Change
    case 0x0C: return 2;  // Program Change
    case 0x0D: return 2;  // Channel Pressure
    case 0x0E: return 3;  // Pitch-bend Change
    case 0x0F: return 1;  // Single byte
    }
    return 0;
  }
};

}  // end namespace midi2
