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
#include <generator>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>

#include "midi2/ci/ci_create_message.hpp"
#include "midi2/ci/ci_types.hpp"
#include "midi2/utils.hpp"

using namespace midi2::ci::literals;

class iterator {
public:
  /// Defines this class as fulfilling the requirements of an output iterator.
  using iterator_category = std::output_iterator_tag;
  /// The class is an output iterator and as such does not yield values.
  using value_type = void;
  /// A type that can be used to identify distance between iterators.
  using difference_type = std::ptrdiff_t;
  /// Defines a pointer to the type iterated over (none in the case of this iterator).
  using pointer = void;
  /// Defines a reference to the type iterated over (none in the case of this iterator).
  using reference = void;

  iterator() = default;
  iterator(iterator const& rhs) = default;
  iterator(iterator&& rhs) noexcept = default;
  ~iterator() noexcept = default;

  iterator& operator=(typename Transcoder::input_type const& value) {
    co_yield value;
    return *this;
  }

  iterator& operator=(iterator const& rhs) = default;
  iterator& operator=(iterator&& rhs) noexcept = default;

  constexpr iterator& operator*() noexcept { return *this; }
  constexpr iterator& operator++() noexcept { return *this; }
  constexpr iterator operator++(int) noexcept { return *this; }
};

template <typename T> std::generator<std::byte> create_message(header const& hdr, T const& t) {
  iterator t;
  create_message(iterator{}, trivial_sentinel{}, hdr, t);
}

int main() {
  constexpr auto my_muid = midi2::ci::muid{0x01234567U};  // Use a proper random number!
  constexpr std::size_t buffer_size = 256;
  constexpr midi2::ci::header header{
      .device_id = 0_b7, .version = 2_b7, .remote_muid = my_muid, .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery_reply discovery{
      .manufacturer = {0x12_b7, 0x23_b7, 0x34_b7},
      .family = 0x1779_b14,
      .model = 0x2B5D_b14,
      .version = {0x01_b7, 0x00_b7, 0x00_b7, 0x00_b7},
      .capability = 0x7F_b7,
      .max_sysex_size = midi2::ci::b28{buffer_size},
      .output_path_id = 0_b7,
      .function_block = 0_b7,
  };
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
    std::cout << std::format("{:02X} ", std::to_underlying(b));
  }
  std::cout << '\n';
  return EXIT_SUCCESS;
}
