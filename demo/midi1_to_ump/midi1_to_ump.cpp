//===-- Demo MIDI 1 bytestream to UMP -----------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <midi2/bytestream_to_ump.hpp>

namespace {

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

}  // end anonymous namespace

int main() {
  // Convert the bytestream to UMP group 0 and write it to stdout.
  std::uint8_t group = 0;
  midi2::bytestream_to_ump bs2ump{group};

  // A bytestream containing MIDI1 note-on events with running status.
  for (auto b : {0x81_b, 0x60_b, 0x50_b, 0x70_b, 0x70_b}) {
    // Push each byte into the bs2ump instance.
    bs2ump.push(b);
    // Pull as may 32-bit UMP values as are available and display them.
    while (!bs2ump.empty()) {
      std::cout << std::hex << bs2ump.pop() << '\n';
    }
  }
}
