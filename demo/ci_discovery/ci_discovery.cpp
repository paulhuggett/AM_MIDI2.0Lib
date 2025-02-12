//===-- Demo CI Discovery Message Creation ------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <vector>

#include "midi2/ci_create_message.hpp"
#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

namespace {

auto discovery() {
  constexpr midi2::ci::header hdr{
      .device_id = 0x7F, .version = 2, .remote_muid = 0, .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery discovery{.manufacturer = {0x12, 0x23, 0x34},
                                           .family = 0x1779,
                                           .model = 0x2B5D,
                                           .version = {0x4E, 0x3C, 0x2A, 0x18},
                                           .capability = 0x7F,
                                           .max_sysex_size = 256,
                                           .output_path_id = 0x71};
  std::vector<std::byte> message;
  midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{}, hdr, discovery);
  return message;
}

}  // end anonymous namespace

int main() {
  for (auto const b : discovery()) {
    std::cout << std::format("{:02X} ", midi2::to_underlying(b));
  }
  std::cout << '\n';
}
