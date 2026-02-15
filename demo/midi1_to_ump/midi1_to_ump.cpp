//===-- Demo MIDI 1 bytestream to UMP -----------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// Standard library
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <print>
#include <utility>

// Local includes
#include <midi2/bytestream/bytestream_to_ump.hpp>
#include <midi2/utils.hpp>

using namespace midi2::literals;

int main() {
  // A bytestream containing MIDI1 note-on events with running status.
  // The '_b' operator (from midi2::literals) turns an integer constant into a std::byte constant.
  constexpr std::array input = {0x81_b, 0x60_b, 0x50_b, 0x70_b, 0x70_b};
  std::print("Bytestream input: ");
  for (auto const in : input) {
    std::print("0x{:02X} ", std::to_underlying(in));
  }
  std::println();

  std::print("UMP Packets: ");
  // Convert the bytestream to UMP group 0 and write it to stdout.
  constexpr auto group = std::uint8_t{0};
  midi2::bytestream::bytestream_to_ump bs2ump{group};
  for (auto const b : input) {
    // Push each byte into the translator instance.
    bs2ump.push(b);
    // Pull as may 32-bit UMP values as are available and display them.
    while (!bs2ump.empty()) {
      std::print("0x{:08X} ", bs2ump.pop());
    }
  }
  std::println();
}
