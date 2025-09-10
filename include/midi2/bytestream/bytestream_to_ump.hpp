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
#include "midi2/ump/ump_types.hpp"

namespace midi2::bytestream {

class bytestream_to_ump {
public:
  /// \brief The type of input from a bytestream
  using input_type = std::byte;
  /// \brief The type of a UMP message
  using output_type = std::uint32_t;

  constexpr bytestream_to_ump() = default;
  explicit constexpr bytestream_to_ump(std::uint8_t const group) : group_{group} { assert(group <= 0b1111); }

  [[nodiscard]] constexpr bool empty() const { return output_.empty(); }
  [[nodiscard]] constexpr output_type pop() {
    assert(!output_.empty());
    return output_.pop_front();
  }

  void push(input_type midi1_byte);

  /// \brief Restore the translator to its original state.
  /// Any in-flight messages are lost.
  void reset();

private:
  static constexpr auto unknown = std::byte{0xFF};
  std::byte group_ = std::byte{0};

  std::byte d0_ = std::byte{0};
  std::byte d1_ = unknown;

  struct sysex7 {
    enum class status : std::uint8_t {
      none,   ///< Not consuming a sysex message
      start,  ///< Sysex start
      cont,   ///< Sysex continue UMP. There might be multiple 'cont' UMPs in a single message.
    };
    status state = status::none;
    /// The number of system exclusive bytes in the current UMP [0,6]
    std::uint8_t pos = 0;
    /// System exclusive message bytes gathered for the current UMP
    std::array<std::byte, 6> bytes{};

    void reset() {
      state = sysex7::status::none;
      pos = 0;
      std::ranges::fill(bytes, std::byte{0});
    }
  };
  sysex7 sysex7_;
  adt::fifo<std::uint32_t, 4> output_{};

  void to_ump(std::byte b0, std::byte b1, std::byte b2);

  template <typename T> void push_sysex7();
  void sysex_data_byte(std::byte midi1_byte);
};

}  // end namespace midi2::bytestream

#endif  // MIDI2_BYTESTREAM_TO_UMP_HPP
