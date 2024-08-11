//
//  ci_types.h
//  libmidi2
//
//  Created by Paul Bowen-Huggett on 09/08/2024.
//

#ifndef MIDI2_CI_TYPES_H
#define MIDI2_CI_TYPES_H

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>

namespace midi2::ci {

using byte_array_2 = std::array<std::byte, 2>;
using byte_array_3 = std::array<std::byte, 3>;
using byte_array_4 = std::array<std::byte, 4>;

constexpr auto FUNCTION_BLOCK = std::uint8_t{0x7F};

using reqId = std::tuple<uint32_t, std::byte>;  // muid-requestId

struct MIDICI {
  bool operator==(MIDICI const&) const = default;

  std::uint8_t umpGroup = 0xFF;
  std::uint8_t deviceId = 0xFF;
  std::uint8_t ciType = 0xFF;
  std::uint8_t ciVer = 1;
  std::uint32_t remoteMUID = 0;
  std::uint32_t localMUID = 0;
  std::optional<reqId> _peReqIdx;

  std::uint8_t totalChunks = 0;
  std::uint8_t numChunk = 0;
  std::uint8_t partialChunkCount = 0;
  std::byte requestId{0xFF};
};

namespace packed {
constexpr auto mask7b = std::byte{(1 << 7) - 1};

constexpr std::uint32_t from_le7(byte_array_4 const &v) {
  return (static_cast<std::uint32_t>(v[0] & mask7b) << (7 * 0)) |
         (static_cast<std::uint32_t>(v[1] & mask7b) << (7 * 1)) |
         (static_cast<std::uint32_t>(v[2] & mask7b) << (7 * 2)) |
         (static_cast<std::uint32_t>(v[3] & mask7b) << (7 * 3));
}
constexpr std::uint16_t from_le7(byte_array_2 const &v) {
  return static_cast<std::uint16_t>((static_cast<std::uint16_t>(v[0] & mask7b) << (7 * 0)) |
                                    (static_cast<std::uint16_t>(v[1] & mask7b) << (7 * 1)));
}
constexpr std::uint8_t from_le7(std::byte v) {
  return static_cast<std::uint8_t>(v);
}

template <std::size_t Size>
constexpr std::array<std::uint8_t, Size> from_array(std::array<std::byte, Size> const &other) {
  std::array<std::uint8_t, Size> result;
  std::transform(std::begin(other), std::end(other), std::begin(result),
                 [](std::byte v) { return static_cast<std::uint8_t>(v); });
  return result;
}

}  // end namespace packed

//*     _ _                              *
//*  __| (_)___ __ _____ _____ _ _ _  _  *
//* / _` | (_-</ _/ _ \ V / -_) '_| || | *
//* \__,_|_/__/\__\___/\_/\___|_|  \_, | *
//*                                |__/  *
namespace packed {

struct discovery_v1 {
  byte_array_3 manufacturer;
  byte_array_2 family;
  byte_array_2 model;
  byte_array_4 version;
  std::byte capability;
  byte_array_4 max_sysex_size;
};
static_assert(offsetof(discovery_v1, manufacturer) == 0);
static_assert(offsetof(discovery_v1, family) == 3);
static_assert(offsetof(discovery_v1, model) == 5);
static_assert(offsetof(discovery_v1, version) == 7);
static_assert(offsetof(discovery_v1, capability) == 11);
static_assert(offsetof(discovery_v1, max_sysex_size) == 12);
static_assert(sizeof(discovery_v1) == 16);

struct discovery_v2 {
  discovery_v1 v1;
  std::byte output_path_id;
};

}  // end namespace packed

struct discovery {
  constexpr discovery() = default;
  constexpr discovery(discovery const &) = default;
  constexpr discovery(discovery &&) noexcept = default;
  constexpr explicit discovery(packed::discovery_v1 const &v1);
  constexpr explicit discovery(packed::discovery_v2 const &v2);
  bool operator==(discovery const &) const = default;

  std::array<std::uint8_t, 3> manufacturer{};
  std::uint16_t family = 0;
  std::uint16_t model = 0;
  std::array<std::uint8_t, 4> version{};
  std::uint8_t capability = 0;
  std::uint32_t max_sysex_size = 0;
  std::uint8_t output_path_id = 0;
};

constexpr discovery::discovery(packed::discovery_v1 const &v1)
    : manufacturer{packed::from_array(v1.manufacturer)},
      family{packed::from_le7(v1.family)},
      model{packed::from_le7(v1.model)},
      version{packed::from_array(v1.version)},
      capability{packed::from_le7(v1.capability)},
      max_sysex_size{packed::from_le7(v1.max_sysex_size)} {
}
constexpr discovery::discovery(packed::discovery_v2 const &v2) : discovery(v2.v1) {
  output_path_id = packed::from_le7(v2.output_path_id);
}

//*     _ _                                            _       *
//*  __| (_)___ __ _____ _____ _ _ _  _   _ _ ___ _ __| |_  _  *
//* / _` | (_-</ _/ _ \ V / -_) '_| || | | '_/ -_) '_ \ | || | *
//* \__,_|_/__/\__\___/\_/\___|_|  \_, | |_| \___| .__/_|\_, | *
//*                                |__/          |_|     |__/  *
namespace packed {

struct discovery_reply_v1 {
  byte_array_3 manufacturer;
  byte_array_2 family;
  byte_array_2 model;
  byte_array_4 version;
  std::byte capability;
  byte_array_4 max_sysex_size;
};
static_assert(offsetof(discovery_reply_v1, manufacturer) == 0);
static_assert(offsetof(discovery_reply_v1, family) == 3);
static_assert(offsetof(discovery_reply_v1, model) == 5);
static_assert(offsetof(discovery_reply_v1, version) == 7);
static_assert(offsetof(discovery_reply_v1, capability) == 11);
static_assert(offsetof(discovery_reply_v1, max_sysex_size) == 12);
static_assert(sizeof(discovery_reply_v1) == 16);

struct discovery_reply_v2 {
  discovery_reply_v1 v1;
  std::byte output_path_id;
  std::byte function_block;
};
static_assert(offsetof(discovery_reply_v2, v1) == 0);
static_assert(offsetof(discovery_reply_v2, output_path_id) == 16);
static_assert(offsetof(discovery_reply_v2, function_block) == 17);
static_assert(sizeof(discovery_reply_v2) == 18);

}  // end namespace packed

struct discovery_reply {
  constexpr discovery_reply() = default;
  constexpr discovery_reply(discovery_reply const &) = default;
  constexpr discovery_reply(discovery_reply &&) noexcept = default;
  constexpr explicit discovery_reply(packed::discovery_reply_v1 const &v1);
  constexpr explicit discovery_reply(packed::discovery_reply_v2 const &v2);
  bool operator==(discovery_reply const &) const = default;

  std::array<std::uint8_t, 3> manufacturer{};
  std::uint16_t family = 0;
  std::uint16_t model = 0;
  std::array<std::uint8_t, 4> version{};
  std::uint8_t capability = 0;
  std::uint32_t max_sysex_size = 0;
  std::uint8_t output_path_id = 0;
  std::uint8_t function_block = 0;
};
constexpr discovery_reply::discovery_reply(packed::discovery_reply_v1 const &v1)
    : manufacturer{packed::from_array(v1.manufacturer)},
      family{packed::from_le7(v1.family)},
      model{packed::from_le7(v1.model)},
      version{packed::from_array(v1.version)},
      capability{packed::from_le7(v1.capability)},
      max_sysex_size{packed::from_le7(v1.max_sysex_size)} {
}
constexpr discovery_reply::discovery_reply(packed::discovery_reply_v2 const &v2) : discovery_reply(v2.v1) {
  output_path_id = packed::from_le7(v2.output_path_id);
  function_block = packed::from_le7(v2.function_block);
}

//*              _           _     _     _       __      *
//*  ___ _ _  __| |_ __  ___(_)_ _| |_  (_)_ _  / _|___  *
//* / -_) ' \/ _` | '_ \/ _ \ | ' \  _| | | ' \|  _/ _ \ *
//* \___|_||_\__,_| .__/\___/_|_||_\__| |_|_||_|_| \___/ *
//*               |_|                                    *
namespace packed {

struct endpoint_info_v1 {
  std::byte status;
};
static_assert(offsetof(endpoint_info_v1, status) == 0);
static_assert(sizeof(endpoint_info_v1) == 1);

}  // end namespace packed

struct endpoint_info {
  constexpr endpoint_info() = default;
  constexpr endpoint_info(endpoint_info const &) = default;
  constexpr endpoint_info(endpoint_info &&) noexcept = default;
  constexpr explicit endpoint_info(packed::endpoint_info_v1 const &);
  bool operator==(endpoint_info const &) const = default;

  std::uint8_t status;
};

constexpr endpoint_info::endpoint_info(packed::endpoint_info_v1 const &other)
    : status{static_cast<std::uint8_t>(other.status)} {
}

//*              _           _     _     _       __                    _       *
//*  ___ _ _  __| |_ __  ___(_)_ _| |_  (_)_ _  / _|___   _ _ ___ _ __| |_  _  *
//* / -_) ' \/ _` | '_ \/ _ \ | ' \  _| | | ' \|  _/ _ \ | '_/ -_) '_ \ | || | *
//* \___|_||_\__,_| .__/\___/_|_||_\__| |_|_||_|_| \___/ |_| \___| .__/_|\_, | *
//*               |_|                                            |_|     |__/  *
namespace packed {

struct endpoint_info_reply_v1 {
  std::byte status;
  byte_array_2 data_length;
  std::byte data[1];  // an array of size given by data_length
};
static_assert(offsetof(endpoint_info_reply_v1, status) == 0);
static_assert(offsetof(endpoint_info_reply_v1, data_length) == 1);
static_assert(offsetof(endpoint_info_reply_v1, data) == 3);
static_assert(sizeof(endpoint_info_reply_v1) == 4);

}  // end namespace packed

struct endpoint_info_reply {
  constexpr endpoint_info_reply() = default;
  constexpr endpoint_info_reply(endpoint_info_reply const &) = default;
  constexpr endpoint_info_reply(endpoint_info_reply &&) noexcept = default;
  constexpr explicit endpoint_info_reply(packed::endpoint_info_reply_v1 const &);

  std::byte status{};
  std::span<std::byte const> information;
};

constexpr endpoint_info_reply::endpoint_info_reply(packed::endpoint_info_reply_v1 const &other)
    : status{packed::from_le7(other.status)}, information{other.data, packed::from_le7(other.data_length)} {
}

//*  _              _ _    _      _         __  __ _   _ ___ ___   *
//* (_)_ ___ ____ _| (_)__| |__ _| |_ ___  |  \/  | | | |_ _|   \  *
//* | | ' \ V / _` | | / _` / _` |  _/ -_) | |\/| | |_| || || |) | *
//* |_|_||_\_/\__,_|_|_\__,_\__,_|\__\___| |_|  |_|\___/|___|___/  *
//*                                                                *
namespace packed {

struct invalidate_muid_v1 {
  byte_array_4 target_muid;
};
static_assert(offsetof(invalidate_muid_v1, target_muid) == 0);
static_assert(sizeof(invalidate_muid_v1) == 4);

}  // end namespace packed

struct invalidate_muid {
  constexpr invalidate_muid() = default;
  constexpr invalidate_muid(invalidate_muid const &) = default;
  constexpr invalidate_muid(invalidate_muid &&) noexcept = default;
  constexpr explicit invalidate_muid(packed::invalidate_muid_v1 const &);
  constexpr bool operator==(invalidate_muid const &) const = default;

  std::uint32_t target_muid = 0;
};

constexpr invalidate_muid::invalidate_muid(packed::invalidate_muid_v1 const &other)
    : target_muid{packed::from_le7(other.target_muid)} {
}

}  // end namespace midi2::ci

#endif  // MIDI2_CI_TYPES_H
