#include "midi2/ci_dispatcher.hpp"

#include <array>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iostream>

namespace {

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

template <typename Element, std::size_t Size> std::string as_string(std::array<Element, Size> const &arr) {
  std::string result;
  char const *separator = "";
  for (auto const v : arr) {
    result += std::format("{}0x{:X}", separator, v);
    separator = ", ";
  }
  return result;
}

}  // end anonymous namespace

int main() {
  try {
    std::srand(static_cast<unsigned>(std::time(nullptr)));  // use current time as seed for random generator
    auto const local_muid = std::uint32_t{static_cast<unsigned>(std::rand()) % midi2::ci::max_user_muid};

    struct context {};
    auto dispatcher = midi2::ci::make_function_dispatcher(context{});
    dispatcher.config().system.on_check_muid(
        [&local_muid](context, std::uint8_t /*group*/, std::uint32_t const muid) { return local_muid == muid; });
    dispatcher.config().management.on_discovery(
        [](context, midi2::ci::header const &hdr, midi2::ci::discovery const &d) {
          std::cout << std::format("device-id=0x{:X}\nversion=0x{:X}\n", hdr.device_id, hdr.version)
                    << std::format("remote-MUID=0x{:08X}\nlocal-MUID=0x{:08X}\n\n", hdr.remote_muid, hdr.local_muid);

          std::cout << std::format("manufacturer=[{}]\nfamily=0x{:X}\nmodel=0x{:X}\n", as_string(d.manufacturer),
                                   d.family, d.model)
                    << std::format("version=[{}]\n", as_string(d.version))
                    << std::format("capability=0x{:X}\nmax-sysex-size=0x{:X}\noutput-path-id=0x{:X}\n", d.capability,
                                   d.max_sysex_size, d.output_path_id);
        });

    std::array const message{0x7E_b, 0x7F_b, 0x0D_b, 0x70_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x7F_b,
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
