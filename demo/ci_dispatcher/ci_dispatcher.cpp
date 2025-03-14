//===-- Demo CI Message Dispatch ----------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/ci_dispatcher.hpp"

#include <array>
#include <cstdlib>
#include <format>
#include <iostream>
#include <type_traits>

using namespace midi2::ci::literals;

// A formatter for arrays of 7-bit integer values (b7). These will be represented to
// comma-separated hex values.
template <std::size_t Size, typename CharT> struct std::formatter<std::array<midi2::ci::b7, Size>, CharT> {
  constexpr auto parse(auto &parse_ctx) const { return parse_ctx.begin(); }
  auto format(std::array<midi2::ci::b7, Size> const &arr, auto &format_ctx) const {
    // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
    auto out = format_ctx.out();
    char const *separator = "";
    for (auto const value : arr) {
      out = std::format_to(out, "{}0x{:X}", separator, value);
      separator = ",";
    }
    return out;
  }
};

namespace {
// Display the header fields
void print_header(std::ostream &os, midi2::ci::header const &h) {
  os << std::format("device-id=0x{:X}\nversion=0x{:X}\n", h.device_id, h.version)
     << std::format("remote-MUID=0x{:X}\nlocal-MUID=0x{:X}\n\n", h.remote_muid, h.local_muid);
}
// Display the discovery data fields
void print_discovery(std::ostream &os, midi2::ci::discovery const &d) {
  os << std::format("manufacturer={}\nfamily=0x{:X}\nmodel=0x{:X}\n", d.manufacturer, d.family, d.model)
     << std::format("version={}\ncapability=0x{:X}\n", d.version, d.capability)
     << std::format("max-sysex-size=0x{:X}\noutput-path-id=0x{:X}\n", d.max_sysex_size, d.output_path_id);
}

// We must pass a “context” to the dispatcher, which will be forwarded to each of the dispatcher's callbacks.
// The context lets message handlers share state but we don’t need that here, so a struct with no members will suffice.
struct context {};

// Normally, these typedefs are not necessary: just use auto! However,
// they are included here for clarity.
constexpr auto buffer_size = std::size_t{256};
using function_config = midi2::ci::function_config<context, buffer_size>;
static_assert(std::is_same_v<std::remove_cvref_t<decltype(function_config::buffer_size)>, std::size_t>);
using dispatcher = midi2::ci::ci_dispatcher<function_config>;

dispatcher setup_ci_dispatcher(midi2::ci::muid const my_muid) {
  // Create a CI dispatcher instance using std::function<> for all of its handler functions.
  auto dispatcher = midi2::ci::make_function_dispatcher<context, buffer_size>();
  auto &config = dispatcher.config();

  // Register a handler for checking whether a message is addressed to this receiver.
  config.system.on_check_muid(
      [my_muid](context, std::uint8_t /*group*/, midi2::ci::muid const m) { return m == my_muid; });

  // Register a handler for Discovery messages.
  config.management.on_discovery([](context, midi2::ci::header const &h, midi2::ci::discovery const &d) {
    print_header(std::cout, h);
    print_discovery(std::cout, d);
    // Send a reply to this message...
  });
  return dispatcher;
}
}  // end anonymous namespace

int main() {
  constexpr auto my_muid = midi2::ci::muid{0x01234567U};  // Use a proper random number!
  constexpr auto my_group = std::uint8_t{0};
  constexpr auto device_id = 0_b7;
  auto dispatcher = setup_ci_dispatcher(my_muid);

  // A system exclusive message containing a CI discovery request.
  constexpr std::array message{0x7E, 0x7F, 0x0D, 0x70, 0x02, 0x00, 0x00, 0x00, 0x00, 0x7F,
                               0x7F, 0x7F, 0x7F, 0x12, 0x23, 0x34, 0x79, 0x2E, 0x5D, 0x56,
                               0x01, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x02, 0x00, 0x00, 0x00};
  dispatcher.start(my_group, device_id);
  for (auto const b : message) {
    dispatcher.processMIDICI(static_cast<std::byte>(b));
  }
  dispatcher.finish();
}
