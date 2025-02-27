//===-- Demo CI Discovery Message Creation ------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <array>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>

#include "midi2/ci_create_message.hpp"
#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

using namespace midi2::ci::literals;

int main() {
  constexpr auto my_muid = midi2::ci::muid{0x01234567U};  // Use a proper random number!
  constexpr std::size_t buffer_size = 256;
  constexpr midi2::ci::header header{
      .device_id = 0_b7, .version = 2_b7, .remote_muid = my_muid, .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery_reply discovery{.manufacturer = {0x12_b7, 0x23_b7, 0x34_b7},
                                                 .family = 0x1779_b14,
                                                 .model = 0x2B5D_b14,
                                                 .version = {0x01_b7, 0x00_b7, 0x00_b7, 0x00_b7},
                                                 .capability = 0x7F_b7,
                                                 .max_sysex_size = midi2::ci::b28{buffer_size},
                                                 .output_path_id = 0_b7};
  std::array<std::byte, buffer_size> message{};
  // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
  auto const first = std::begin(message);
  // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
  auto const last = std::end(message);
  // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
  auto const pos = midi2::ci::create_message(first, last, header, discovery);
  if (pos == last) {
    std::cerr << "Buffer too small\n";
    return EXIT_FAILURE;
  }
  for (auto const b : std::ranges::subrange(first, pos)) {
    std::cout << std::format("{:02X} ", midi2::to_underlying(b));
  }
  std::cout << '\n';
  return EXIT_SUCCESS;
}
