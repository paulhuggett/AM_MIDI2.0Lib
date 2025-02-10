#include "midi2/ci_dispatcher.hpp"

#include <format>
#include <iostream>
#include <vector>

namespace {

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

std::ostream &operator<<(std::ostream &os, midi2::ci::header const &hdr) {
  return os << std::format("device-id=0x{:X}, version=0x{:X}", hdr.device_id, hdr.version)
            << std::format(", remote-MUID=0x{:08X}, local-MUID=0x{:08X}", hdr.remote_muid, hdr.local_muid);
}

std::ostream &operator<<(std::ostream &os, midi2::ci::discovery const &d) {
  return os << std::format("manufacturer=[0x{:X}, 0x{:X}, 0x{:X}]", d.manufacturer[0], d.manufacturer[1],
                           d.manufacturer[2])
            << std::format(", family=0x{:X}, model=0x{:X}", d.family, d.model)
            << std::format(", version=[0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}]", d.version[0], d.version[1], d.version[2],
                           d.version[3])
            << std::format(", capability=0x{:X}", d.capability)
            << std::format(", max-sysex-size=0x{:X}", d.max_sysex_size)
            << std::format(", output-path-id=0x{:X}", d.output_path_id);
}

}  // end anonymous namespace

int main() {
  try {
    struct context {};
    auto dispatcher = midi2::ci::make_function_dispatcher(context{});
    dispatcher.config()
        .management.on_check_muid([](context, std::uint8_t /*group*/, std::uint32_t /*muid*/) { return true; })
        .on_discovery([](context, midi2::ci::header const &hdr, midi2::ci::discovery const &d) {
          std::cout << hdr << '\n' << d << '\n';
        });

    std::vector const message{0x7E_b, 0x7F_b, 0x0D_b, 0x70_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x7F_b,
                              0x7F_b, 0x7F_b, 0x7F_b, 0x12_b, 0x23_b, 0x34_b, 0x79_b, 0x2E_b, 0x5D_b, 0x56_b,
                              0x4E_b, 0x3C_b, 0x2A_b, 0x18_b, 0x7F_b, 0x00_b, 0x02_b, 0x00_b, 0x00_b, 0x71_b};
    constexpr auto group = std::uint8_t{0xFF};
    constexpr auto device_id = 0x7F_b;
    dispatcher.start(group, device_id);
    for (auto const b : message) {
      dispatcher.processMIDICI(b);
    }
    dispatcher.finish();
  } catch (std::exception const &ex) {
    std::cerr << "An error occurred: " << ex.what() << '\n';
  } catch (...) {
    std::cerr << "An unknown error occurred\n";
  }
}
