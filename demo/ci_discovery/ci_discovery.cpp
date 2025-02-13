//===-- Demo CI Discovery Message Creation ------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <cstddef>
#include <cstdlib>
#include <format>
#include <iostream>
#include <iterator>
#include <vector>

#include "midi2/ci_create_message.hpp"
#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

int main() {
  constexpr std::size_t buffer_size = 256;
  constexpr midi2::ci::header header{
      .device_id = 0x7F, .version = 2, .remote_muid = 0, .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery_reply discovery{.manufacturer = {0x12, 0x23, 0x34},
                                                 .family = 0x1779,
                                                 .model = 0x2B5D,
                                                 .version = {0x4E, 0x3C, 0x2A, 0x18},
                                                 .capability = 0x7F,
                                                 .max_sysex_size = buffer_size,
                                                 .output_path_id = 0};
  std::array<std::byte, buffer_size> message{};
  auto const first = std::begin(message);
  auto const last = std::end(message);
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
