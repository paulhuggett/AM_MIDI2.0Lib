//===-- Demo MIDI 1 bytestream to UMP -----------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <utility>

#include "midi2/bytestream/bytestream_to_ump.hpp"

namespace {

[[nodiscard]] consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

}  // end anonymous namespace

int main() {
  int exit_code = EXIT_SUCCESS;
  try {
    // A bytestream containing MIDI1 note-on events with running status.
    constexpr std::array input = {0x81_b, 0x60_b, 0x50_b, 0x70_b, 0x70_b};
    std::cout << "Bytestream input: ";
    for (auto const in : input) {
      std::cout << std::format("0x{:02X} ", std::to_underlying(in));
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
        std::cout << std::format("0x{:08X} ", bs2ump.pop());
      }
    }
    std::cout << '\n';
  } catch (std::exception const& ex) {
    // The midi2 library doesn't throw but something else might...
    std::cerr << "Error: " << ex.what() << '\n';
    exit_code = EXIT_FAILURE;
  } catch (...) {
    std::cerr << "An unknown error\n";
    exit_code = EXIT_FAILURE;
  }
  return exit_code;
}
