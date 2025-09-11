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

namespace midi2::bytestream {

class usbm1_to_bytestream {
public:
  /// \brief The type of input from a 32-bit UMP stream
  using input_type = std::uint32_t;
  /// \brief The type of output to a bytestream
  using output_type = std::byte;

  explicit constexpr usbm1_to_bytestream(std::uint8_t cable) noexcept : cable_{cable} {
    assert(cable < 16U && "cable number must be four bits");
  }

  /// Checks if the output has no elements
  [[nodiscard]] constexpr bool empty() const noexcept { return output_.empty(); }
  /// \brief Pops and returns the next available byte for the bytestream
  /// \return The next available byte
  /// \pre !empty()
  [[nodiscard]] constexpr output_type pop() noexcept {
    assert(!empty());
    return output_.pop_front();
  }

  /// \brief Provides a word of USBM1 input to the translator
  /// \param usbm1 The word of input to be translated
  constexpr void push(input_type const usbm1) noexcept {
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

  /// \brief Restore the translator to its original state.
  /// Any in-flight messages are lost.
  constexpr void reset() { output_.clear(); }

private:
  std::uint8_t cable_;
  adt::fifo<std::byte, 4> output_;

  [[nodiscard]] static constexpr std::uint8_t cable(std::uint32_t const p) noexcept { return (p >> 28) & 0x0F; }
  [[nodiscard]] static constexpr std::uint8_t get_cin(std::uint32_t const p) noexcept { return (p >> 24) & 0x0F; }
  [[nodiscard]] static constexpr unsigned midi_x_size(std::uint8_t const cin) noexcept {
    assert(cin < 0x10U && "code index number should be four bits");
    // The contents of this switch are based on Table 4-1: "Code Index Number Classifications" in the "Universal
    // Serial Bus Device Class Definition for MIDI Devices" (Release 1.0 Nov 1, 1999)
    switch (cin) {
    case 0x00:  // Reserved for future extension
    case 0x01:  // Reserved for future expansion
      return 0;
    case 0x02:  // Two-byte System Common messages
      return 2;
    case 0x03:  // Three-byte System Common messages
    case 0x04:  // SysEx starts or continues
      return 3;
    case 0x05:  // Single-byte System Common/SysEx end Message
      return 1;
    case 0x06:  // SysEx ends with following two bytes
      return 2;
    case 0x07:  // SysEx ends with following three bytes
    case 0x08:  // Note-off
    case 0x09:  // Note-on
    case 0x0A:  // Poly Key Press
    case 0x0B:  // Control Change
      return 3;
    case 0x0C:  // Program Change
    case 0x0D:  // Channel Pressure
      return 2;
    case 0x0E: return 3;  // Pitch-bend Change
    case 0x0F: return 1;  // Single byte
    default: break;
    }
    return 0;
  }
};

}  // end namespace midi2::bytestream
