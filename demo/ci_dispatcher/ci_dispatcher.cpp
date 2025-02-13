#include "midi2/ci_dispatcher.hpp"

#include <array>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iostream>
#include <random>
#include <ranges>

namespace {

template <std::ranges::input_range Range> std::string as_string(Range const &range) {
  std::string result;
  char const *separator = "";
  for (auto const v : range) {
    result += std::format("{}0x{:X}", separator, v);
    separator = ", ";
  }
  return result;
}

}  // end anonymous namespace

int main() {
  // Generate a random local MUID.
  std::mt19937 engine{std::random_device{}()};
  std::uniform_int_distribution<std::uint32_t> distribution;
  auto const local_muid = distribution(engine) % midi2::ci::max_user_muid;

  struct context {};
  // Create a CI dispatcher instance using std::function<> for all of its handler functions.
  auto dispatcher = midi2::ci::make_function_dispatcher(context{});
  auto &config = dispatcher.config();
  // Register a handler for checking whether a message is address to this MUID.
  config.system.on_check_muid(
      [&local_muid](context, std::uint8_t /*group*/, std::uint32_t const muid) { return local_muid == muid; });
  // Register a handler for Discovery messages.
  config.management.on_discovery([](context, midi2::ci::header const &hdr, midi2::ci::discovery const &d) {
    // Display the header fields
    std::cout << std::format("device-id=0x{:X}\nversion=0x{:X}\n", hdr.device_id, hdr.version)
              << std::format("remote-MUID=0x{:08X}\nlocal-MUID=0x{:08X}\n\n", hdr.remote_muid, hdr.local_muid);

    // Display the discovery data fields
    std::cout << std::format("manufacturer=[{}]\nfamily=0x{:X}\nmodel=0x{:X}\n", as_string(d.manufacturer), d.family,
                             d.model)
              << std::format("version=[{}]\n", as_string(d.version))
              << std::format("capability=0x{:X}\nmax-sysex-size=0x{:X}\noutput-path-id=0x{:X}\n", d.capability,
                             d.max_sysex_size, d.output_path_id);
    // Send a reply to this message...
  });

  constexpr std::array message{0x7E, 0x7F, 0x0D, 0x70, 0x02, 0x00, 0x00, 0x00, 0x00, 0x7F,
                               0x7F, 0x7F, 0x7F, 0x12, 0x23, 0x34, 0x79, 0x2E, 0x5D, 0x56,
                               0x4E, 0x3C, 0x2A, 0x18, 0x7F, 0x00, 0x02, 0x00, 0x00, 0x71};
  constexpr auto group = std::uint8_t{0};
  constexpr auto device_id = std::byte{0};
  dispatcher.start(group, device_id);
  for (auto const b : message) {
    dispatcher.processMIDICI(static_cast<std::byte>(b));
  }
  dispatcher.finish();
}
