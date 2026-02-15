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
#include <iomanip>
#include <iostream>
#include <utility>

// Midi2 library
#include <midi2/midi2.hpp>

using namespace midi2::literals;

namespace {
template <std::integral T, unsigned Width = sizeof(T) * 2> void print_hex(T v) {
  std::cout << "0x" << std::setw(Width) << std::setfill('0') << std::hex << v << ' ';
}
}  // end anonymous namespace

int main() {
  // A bytestream containing MIDI1 note-on events with running status.
  // The '_b' operator (from midi2::literals) turns an integer constant into a std::byte constant.
  constexpr std::array input = {0x81_b, 0x60_b, 0x50_b, 0x70_b, 0x70_b};
  std::cout << "Bytestream input: ";
  for (auto const in : input) {
    print_hex<unsigned, 2>(std::to_underlying(in));
  }
  std::cout << '\n';

  std::cout << "UMP Packets: ";
  // Convert the bytestream to UMP group 0 and write it to stdout.
  constexpr auto group = std::uint8_t{0};
  midi2::bytestream::bytestream_to_ump bs2ump{group};
  for (auto const b : input) {
    // Push each byte into the translator instance.
    bs2ump.push(b);
    // Pull as may 32-bit UMP values as are available and display them.
    while (!bs2ump.empty()) {
      print_hex(bs2ump.pop());
    }
  }
  std::cout << '\n';
}
