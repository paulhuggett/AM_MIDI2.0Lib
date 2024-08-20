//
//  ci_types.h
//  libmidi2
//
//  Created by Paul Bowen-Huggett on 09/08/2024.
//

#ifndef MIDI2_CI_TYPES_HPP
#define MIDI2_CI_TYPES_HPP

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

using byte_array_1 = std::array<std::byte, 1>;
using byte_array_2 = std::array<std::byte, 2>;
using byte_array_3 = std::array<std::byte, 3>;
using byte_array_4 = std::array<std::byte, 4>;
using byte_array_5 = std::array<std::byte, 5>;

constexpr auto FUNCTION_BLOCK = std::uint8_t{0x7F};

struct MIDICI {
  bool operator==(MIDICI const &) const = default;

  std::uint8_t umpGroup = 0xFF;
  std::uint8_t deviceId = 0xFF;
  std::uint8_t ciType = 0xFF;
  std::uint8_t ciVer = 1;
  std::uint32_t remoteMUID = 0;
  std::uint32_t localMUID = 0;
};

namespace packed {

constexpr auto mask7b = std::byte{(1 << 7) - 1};

constexpr std::uint32_t from_le7(byte_array_4 const &v) {
  assert(((v[0] | v[1] | v[2] | v[3]) & (std::byte{1} << 7)) == std::byte{0});
  return (static_cast<std::uint32_t>(v[0] & mask7b) << (7 * 0)) |
         (static_cast<std::uint32_t>(v[1] & mask7b) << (7 * 1)) |
         (static_cast<std::uint32_t>(v[2] & mask7b) << (7 * 2)) |
         (static_cast<std::uint32_t>(v[3] & mask7b) << (7 * 3));
}
constexpr std::uint16_t from_le7(byte_array_2 const &v) {
  assert(((v[0] | v[1]) & (std::byte{1} << 7)) == std::byte{0});
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
static_assert(alignof(discovery_v1) == 1);
static_assert(std::is_trivially_copyable_v<discovery_v1>);

struct discovery_v2 {
  discovery_v1 v1;
  std::byte output_path_id;
};
static_assert(offsetof(discovery_v2, v1) == 0);
static_assert(offsetof(discovery_v2, output_path_id) == 16);
static_assert(sizeof(discovery_v2) == 17);
static_assert(alignof(discovery_v2) == 1);
static_assert(std::is_trivially_copyable_v<discovery_v2>);

static_assert(sizeof(discovery_v2) > sizeof(discovery_v1));

}  // end namespace packed

struct discovery {
  constexpr discovery() = default;
  constexpr discovery(discovery const &) = default;
  constexpr discovery(discovery &&) noexcept = default;
  constexpr explicit discovery(packed::discovery_v1 const &v1);
  constexpr explicit discovery(packed::discovery_v2 const &v2);
  ~discovery() noexcept = default;

  constexpr discovery &operator=(discovery const &) = default;
  constexpr discovery &operator=(discovery &&) noexcept = default;

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
static_assert(alignof(discovery_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<discovery_reply_v1>);

struct discovery_reply_v2 {
  discovery_reply_v1 v1;
  std::byte output_path_id;
  std::byte function_block;
};
static_assert(offsetof(discovery_reply_v2, v1) == 0);
static_assert(offsetof(discovery_reply_v2, output_path_id) == 16);
static_assert(offsetof(discovery_reply_v2, function_block) == 17);
static_assert(sizeof(discovery_reply_v2) == 18);
static_assert(alignof(discovery_reply_v2) == 1);
static_assert(std::is_trivially_copyable_v<discovery_reply_v2>);

static_assert(sizeof(discovery_reply_v1) <= sizeof(discovery_reply_v2));

}  // end namespace packed

struct discovery_reply {
  constexpr discovery_reply() = default;
  constexpr discovery_reply(discovery_reply const &) = default;
  constexpr discovery_reply(discovery_reply &&) noexcept = default;
  constexpr explicit discovery_reply(packed::discovery_reply_v1 const &v1);
  constexpr explicit discovery_reply(packed::discovery_reply_v2 const &v2);
  ~discovery_reply() noexcept = default;

  constexpr discovery_reply &operator=(discovery_reply const &) = default;
  constexpr discovery_reply &operator=(discovery_reply &&) noexcept = default;

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
static_assert(alignof(endpoint_info_v1) == 1);
static_assert(std::is_trivially_copyable_v<endpoint_info_v1>);

}  // end namespace packed

struct endpoint_info {
  constexpr endpoint_info() = default;
  constexpr endpoint_info(endpoint_info const &) = default;
  constexpr endpoint_info(endpoint_info &&) noexcept = default;
  constexpr explicit endpoint_info(packed::endpoint_info_v1 const &);
  ~endpoint_info() noexcept = default;

  constexpr endpoint_info &operator=(endpoint_info const &) = default;
  constexpr endpoint_info &operator=(endpoint_info &&) noexcept = default;

  bool operator==(endpoint_info const &) const = default;

  std::uint8_t status = 0;
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
  std::array<std::byte, 1> data;  // an array of size given by data_length
};
static_assert(offsetof(endpoint_info_reply_v1, status) == 0);
static_assert(offsetof(endpoint_info_reply_v1, data_length) == 1);
static_assert(offsetof(endpoint_info_reply_v1, data) == 3);
static_assert(sizeof(endpoint_info_reply_v1) == 4);
static_assert(alignof(endpoint_info_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<endpoint_info_reply_v1>);

}  // end namespace packed

struct endpoint_info_reply {
  constexpr endpoint_info_reply() = default;
  constexpr endpoint_info_reply(endpoint_info_reply const &) = default;
  constexpr endpoint_info_reply(endpoint_info_reply &&) noexcept = default;
  constexpr explicit endpoint_info_reply(packed::endpoint_info_reply_v1 const &);
  ~endpoint_info_reply() noexcept = default;

  constexpr endpoint_info_reply &operator=(endpoint_info_reply const &) = default;
  constexpr endpoint_info_reply &operator=(endpoint_info_reply &&) noexcept = default;

  std::byte status{};
  std::span<std::byte const> information{};
};

constexpr endpoint_info_reply::endpoint_info_reply(packed::endpoint_info_reply_v1 const &other)
    : status{packed::from_le7(other.status)}, information{std::begin(other.data), packed::from_le7(other.data_length)} {
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
static_assert(alignof(invalidate_muid_v1) == 1);
static_assert(std::is_trivially_copyable_v<invalidate_muid_v1>);

}  // end namespace packed

struct invalidate_muid {
  constexpr invalidate_muid() = default;
  constexpr invalidate_muid(invalidate_muid const &) = default;
  constexpr invalidate_muid(invalidate_muid &&) noexcept = default;
  constexpr explicit invalidate_muid(packed::invalidate_muid_v1 const &);
  ~invalidate_muid() noexcept = default;

  constexpr invalidate_muid &operator=(invalidate_muid const &) = default;
  constexpr invalidate_muid &operator=(invalidate_muid &&) noexcept = default;

  constexpr bool operator==(invalidate_muid const &) const = default;

  std::uint32_t target_muid = 0;
};

constexpr invalidate_muid::invalidate_muid(packed::invalidate_muid_v1 const &other)
    : target_muid{packed::from_le7(other.target_muid)} {
}

//*          _    *
//*  __ _ __| |__ *
//* / _` / _| / / *
//* \__,_\__|_\_\ *
//*               *
namespace packed {

struct ack_v1 {
  std::byte original_id;             ///< Original Transaction Sub-ID#2 Classification
  std::byte status_code;             ///< ACK Status Code
  std::byte status_data;             ///< ACK Status Data
  byte_array_5 details;              ///< ACK details for each SubID Classification
  byte_array_2 message_length;       ///< Message Length (LSB firt)
  std::array<std::byte, 1> message;  ///< Message text (array of size given by message_length)
};
static_assert(offsetof(ack_v1, original_id) == 0);
static_assert(offsetof(ack_v1, status_code) == 1);
static_assert(offsetof(ack_v1, status_data) == 2);
static_assert(offsetof(ack_v1, details) == 3);
static_assert(offsetof(ack_v1, message_length) == 8);
static_assert(offsetof(ack_v1, message) == 10);
static_assert(sizeof(ack_v1) == 11);
static_assert(alignof(ack_v1) == 1);

}  // end namespace packed

struct ack {
  constexpr ack() = default;
  constexpr ack(ack const &) = default;
  constexpr ack(ack &&) noexcept = default;
  constexpr explicit ack(packed::ack_v1 const &);
  ~ack() noexcept = default;

  constexpr ack &operator=(ack const &) = default;
  constexpr ack &operator=(ack &&) noexcept = default;

  std::uint8_t original_id = 0;
  std::uint8_t status_code = 0;
  std::uint8_t status_data = 0;
  byte_array_5 details{};
  std::span<std::byte const> message{};
};

constexpr ack::ack(packed::ack_v1 const &other)
    : original_id{packed::from_le7(other.original_id)},
      status_code{packed::from_le7(other.status_code)},
      status_data{packed::from_le7(other.status_data)},
      details{other.details},
      message{std::begin(other.message), packed::from_le7(other.message_length)} {
}

//*            _    *
//*  _ _  __ _| |__ *
//* | ' \/ _` | / / *
//* |_||_\__,_|_\_\ *
//*                 *
namespace packed {

struct nak_v1 {};
struct nak_v2 {
  std::byte original_id;             ///< Original transaction sub-ID#2 classification
  std::byte status_code;             ///< ACK Status Code
  std::byte status_data;             ///< ACK Status Data
  byte_array_5 details;              ///< ACK details for each SubID Classification
  byte_array_2 message_length;       ///< Message Length (LSB firt)
  std::array<std::byte, 1> message;  ///< Message text (length given by message_length)
};
static_assert(offsetof(nak_v2, original_id) == 0);
static_assert(offsetof(nak_v2, status_code) == 1);
static_assert(offsetof(nak_v2, status_data) == 2);
static_assert(offsetof(nak_v2, details) == 3);
static_assert(offsetof(nak_v2, message_length) == 8);
static_assert(offsetof(nak_v2, message) == 10);
static_assert(sizeof(nak_v2) == 11);
static_assert(alignof(nak_v2) == 1);
static_assert(std::is_trivially_copyable_v<nak_v2>);

static_assert(sizeof(nak_v1) <= sizeof(nak_v2));

}  // end namespace packed

struct nak {
  constexpr nak() = default;
  constexpr nak(nak const &) = default;
  constexpr nak(nak &&) noexcept = default;
  constexpr explicit nak(packed::nak_v1 const &);
  constexpr explicit nak(packed::nak_v2 const &);
  ~nak() noexcept = default;

  constexpr nak &operator=(nak const &) = default;
  constexpr nak &operator=(nak &&) noexcept = default;

  constexpr bool operator==(nak const &) const;

  std::uint8_t original_id = 0;  // Original transaction sub-ID#2 classification
  std::uint8_t status_code = 0;  // NAK Status Code
  std::uint8_t status_data = 0;  // NAK Status Data
  byte_array_5 details{};        // NAK details for each SubID Classification
  std::span<std::byte const> message{};
};

constexpr nak::nak(packed::nak_v1 const &) {
}
constexpr nak::nak(packed::nak_v2 const &other)
    : original_id{static_cast<std::uint8_t>(other.original_id)},
      status_code{static_cast<std::uint8_t>(other.status_code)},
      status_data{static_cast<std::uint8_t>(other.status_data)},
      details{other.details},
      message{std::begin(other.message), packed::from_le7(other.message_length)} {
}

//*                __ _ _       _                _                         _       *
//*  _ __ _ _ ___ / _(_) |___  (_)_ _  __ _ _  _(_)_ _ _  _   _ _ ___ _ __| |_  _  *
//* | '_ \ '_/ _ \  _| | / -_) | | ' \/ _` | || | | '_| || | | '_/ -_) '_ \ | || | *
//* | .__/_| \___/_| |_|_\___| |_|_||_\__, |\_,_|_|_|  \_, | |_| \___| .__/_|\_, | *
//* |_|                                  |_|           |__/          |_|     |__/  *

namespace packed {

struct profile_inquiry_reply_v1_pt1 {
  byte_array_2 num_enabled;         ///< Number of currently enabled profiles
  std::array<byte_array_5, 1> ids;  ///< Profile ID of currently enabled profiles (array length given by num_enabled)
};

static_assert(offsetof(profile_inquiry_reply_v1_pt1, num_enabled) == 0);
static_assert(offsetof(profile_inquiry_reply_v1_pt1, ids) == 2);
static_assert(sizeof(profile_inquiry_reply_v1_pt1) == 7);
static_assert(alignof(profile_inquiry_reply_v1_pt1) == 1);
static_assert(std::is_trivially_copyable_v<profile_inquiry_reply_v1_pt1>);

struct profile_inquiry_reply_v1_pt2 {
  byte_array_2 num_disabled;        ///< Number of currently disabled profiles
  std::array<byte_array_5, 1> ids;  ///< Profile ID of currently enabled profiles (array length given by num_disabled)
};

static_assert(offsetof(profile_inquiry_reply_v1_pt2, num_disabled) == 0);
static_assert(offsetof(profile_inquiry_reply_v1_pt2, ids) == 2);
static_assert(sizeof(profile_inquiry_reply_v1_pt2) == 7);
static_assert(alignof(profile_inquiry_reply_v1_pt2) == 1);
static_assert(std::is_trivially_copyable_v<profile_inquiry_reply_v1_pt2>);

}  // end namespace packed

struct profile_inquiry_reply {
  constexpr profile_inquiry_reply() = default;
  constexpr profile_inquiry_reply(profile_inquiry_reply const &) = default;
  constexpr profile_inquiry_reply(profile_inquiry_reply &&) noexcept = default;
  constexpr profile_inquiry_reply(packed::profile_inquiry_reply_v1_pt1 const &,
                                  packed::profile_inquiry_reply_v1_pt2 const &);
  ~profile_inquiry_reply() noexcept = default;

  constexpr profile_inquiry_reply &operator=(profile_inquiry_reply const &) = default;
  constexpr profile_inquiry_reply &operator=(profile_inquiry_reply &&) noexcept = default;

  std::span<byte_array_5 const> enabled{};
  std::span<byte_array_5 const> disabled{};
};

constexpr profile_inquiry_reply::profile_inquiry_reply(packed::profile_inquiry_reply_v1_pt1 const &v1_pt1,
                                                       packed::profile_inquiry_reply_v1_pt2 const &v1_pt2)
    : enabled{std::begin(v1_pt1.ids), packed::from_le7(v1_pt1.num_enabled)},
      disabled{std::begin(v1_pt2.ids), packed::from_le7(v1_pt2.num_disabled)} {
}

//*                __ _ _               _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   __ _ __| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / _` / _` / _` / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \__,_\__,_\__,_\___\__,_| *
//* |_|                                                  *
namespace packed {

struct profile_added_v1 {
  byte_array_5 pid;  // Profile ID of profile being added
};
static_assert(offsetof(profile_added_v1, pid) == 0);
static_assert(sizeof(profile_added_v1) == 5);
static_assert(alignof(profile_added_v1) == 1);
static_assert(std::is_trivially_copyable_v<profile_added_v1>);

}  // end namespace packed

struct profile_added {
  constexpr profile_added() = default;
  constexpr profile_added(profile_added const &) = default;
  constexpr profile_added(profile_added &&) noexcept = default;
  constexpr explicit profile_added(packed::profile_added_v1 const &);
  ~profile_added() noexcept = default;

  constexpr profile_added &operator=(profile_added const &) = default;
  constexpr profile_added &operator=(profile_added &&) noexcept = default;

  constexpr bool operator==(profile_added const &) const = default;

  byte_array_5 pid{};
};

constexpr profile_added::profile_added(packed::profile_added_v1 const &other) : pid{other.pid} {
}

//*                __ _ _                                    _  *
//*  _ __ _ _ ___ / _(_) |___   _ _ ___ _ __  _____ _____ __| | *
//* | '_ \ '_/ _ \  _| | / -_) | '_/ -_) '  \/ _ \ V / -_) _` | *
//* | .__/_| \___/_| |_|_\___| |_| \___|_|_|_\___/\_/\___\__,_| *
//* |_|                                                         *
namespace packed {

struct profile_removed_v1 {
  byte_array_5 pid;  // Profile ID of profile being removed
};
static_assert(offsetof(profile_removed_v1, pid) == 0);
static_assert(sizeof(profile_removed_v1) == 5);
static_assert(alignof(profile_removed_v1) == 1);
static_assert(std::is_trivially_copyable_v<profile_removed_v1>);

}  // end namespace packed

struct profile_removed {
  constexpr profile_removed() = default;
  constexpr profile_removed(profile_removed const &) = default;
  constexpr profile_removed(profile_removed &&) noexcept = default;
  constexpr explicit profile_removed(packed::profile_removed_v1 const &);
  ~profile_removed() noexcept = default;

  constexpr profile_removed &operator=(profile_removed const &) = default;
  constexpr profile_removed &operator=(profile_removed &&) noexcept = default;

  constexpr bool operator==(profile_removed const &) const = default;

  byte_array_5 pid{};
};

constexpr profile_removed::profile_removed(packed::profile_removed_v1 const &other) : pid{other.pid} {
}

//*                __      _     _        _ _      _                _           *
//*  _ __ _ _ ___ / _|  __| |___| |_ __ _(_) |___ (_)_ _  __ _ _  _(_)_ _ _  _  *
//* | '_ \ '_/ _ \  _| / _` / -_)  _/ _` | | (_-< | | ' \/ _` | || | | '_| || | *
//* | .__/_| \___/_|   \__,_\___|\__\__,_|_|_/__/ |_|_||_\__, |\_,_|_|_|  \_, | *
//* |_|                                                     |_|           |__/  *
namespace packed {

struct profile_details_inquiry_v1 {
  byte_array_5 pid;  // Profile ID of profile
  std::byte target;
};
static_assert(offsetof(profile_details_inquiry_v1, pid) == 0);
static_assert(offsetof(profile_details_inquiry_v1, target) == 5);
static_assert(sizeof(profile_details_inquiry_v1) == 6);
static_assert(alignof(profile_details_inquiry_v1) == 1);
static_assert(std::is_trivially_copyable_v<profile_details_inquiry_v1>);

}  // end namespace packed

struct profile_details_inquiry {
  constexpr profile_details_inquiry() = default;
  constexpr profile_details_inquiry(profile_details_inquiry const &) = default;
  constexpr profile_details_inquiry(profile_details_inquiry &&) noexcept = default;
  constexpr explicit profile_details_inquiry(packed::profile_details_inquiry_v1 const &);
  ~profile_details_inquiry() noexcept = default;

  constexpr profile_details_inquiry &operator=(profile_details_inquiry const &) = default;
  constexpr profile_details_inquiry &operator=(profile_details_inquiry &&) noexcept = default;

  constexpr bool operator==(profile_details_inquiry const &) const = default;

  byte_array_5 pid{};
  std::uint8_t target = 0;
};

constexpr profile_details_inquiry::profile_details_inquiry(packed::profile_details_inquiry_v1 const &other)
    : pid{other.pid}, target{static_cast<std::uint8_t>(other.target)} {
}

namespace packed {

struct profile_details_reply_v1 {
  byte_array_5 pid;          ///< Profile ID of profile
  std::byte target;          ///< Inquiry target
  byte_array_2 data_length;  ///< Inquiry target data length (LSB first)
  std::array<std::byte, 1> data;  ///< Array length given by data_length
};

static_assert(offsetof(profile_details_reply_v1, pid) == 0);
static_assert(offsetof(profile_details_reply_v1, target) == 5);
static_assert(offsetof(profile_details_reply_v1, data_length) == 6);
static_assert(offsetof(profile_details_reply_v1, data) == 8);
static_assert(sizeof(profile_details_reply_v1) == 9);
static_assert(alignof(profile_details_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<profile_details_reply_v1>);

}  // end namespace packed

struct profile_details_reply {
  constexpr profile_details_reply() = default;
  constexpr profile_details_reply(profile_details_reply const &) = default;
  constexpr profile_details_reply(profile_details_reply &&) noexcept = default;
  constexpr explicit profile_details_reply(packed::profile_details_reply_v1 const &);
  ~profile_details_reply() noexcept = default;

  constexpr profile_details_reply &operator=(profile_details_reply const &) = default;
  constexpr profile_details_reply &operator=(profile_details_reply &&) noexcept = default;

  byte_array_5 pid{};       ///< Profile ID of profile
  std::uint8_t target = 0;  ///< Inquiry target
  std::span<std::byte const> data{};
};

constexpr profile_details_reply::profile_details_reply(packed::profile_details_reply_v1 const &other)
    : pid{other.pid},
      target{static_cast<std::uint8_t>(other.target)},
      data{std::begin(other.data), packed::from_le7(other.data_length)} {
}

//*                __ _ _                 *
//*  _ __ _ _ ___ / _(_) |___   ___ _ _   *
//* | '_ \ '_/ _ \  _| | / -_) / _ \ ' \  *
//* | .__/_| \___/_| |_|_\___| \___/_||_| *
//* |_|                                   *
namespace packed {

struct profile_on_v1 {
  byte_array_5 pid;  // Profile ID of Profile to be Set to On (to be Enabled)
};
static_assert(offsetof(profile_on_v1, pid) == 0);
static_assert(sizeof(profile_on_v1) == 5);
static_assert(alignof(profile_on_v1) == 1);
static_assert(std::is_trivially_copyable_v<profile_on_v1>);

struct profile_on_v2 {
  profile_on_v1 v1;
  byte_array_2 num_channels;  // Number Channels Requested (LSB First) to assign to this Profile when it is enabled
};
static_assert(offsetof(profile_on_v2, v1) == 0);
static_assert(offsetof(profile_on_v2, num_channels) == 5);
static_assert(sizeof(profile_on_v2) == 7);
static_assert(alignof(profile_on_v2) == 1);
static_assert(std::is_trivially_copyable_v<profile_on_v2>);

static_assert(sizeof(profile_on_v1) <= sizeof(profile_on_v2));

}  // end namespace packed

struct profile_on {
  constexpr profile_on() = default;
  constexpr profile_on(profile_on const &) = default;
  constexpr profile_on(profile_on &&) noexcept = default;
  constexpr explicit profile_on(packed::profile_on_v1 const &);
  constexpr explicit profile_on(packed::profile_on_v2 const &);
  ~profile_on() noexcept = default;

  constexpr profile_on &operator=(profile_on const &) = default;
  constexpr profile_on &operator=(profile_on &&) noexcept = default;

  constexpr bool operator==(profile_on const &) const = default;

  byte_array_5 pid{};
  std::uint16_t num_channels = 0;
};

constexpr profile_on::profile_on(packed::profile_on_v1 const &other) : pid{other.pid} {
}
constexpr profile_on::profile_on(packed::profile_on_v2 const &other) : profile_on(other.v1) {
  num_channels = packed::from_le7(other.num_channels);
}

//*                __ _ _            __  __  *
//*  _ __ _ _ ___ / _(_) |___   ___ / _|/ _| *
//* | '_ \ '_/ _ \  _| | / -_) / _ \  _|  _| *
//* | .__/_| \___/_| |_|_\___| \___/_| |_|   *
//* |_|                                      *
namespace packed {

struct profile_off_v1 {
  byte_array_5 pid;  // Profile ID of Profile to be Set to On (to be Enabled)
};
static_assert(offsetof(profile_off_v1, pid) == 0);
static_assert(sizeof(profile_off_v1) == 5);
static_assert(alignof(profile_off_v1) == 1);
static_assert(std::is_trivially_copyable_v<profile_off_v1>);

struct profile_off_v2 {
  profile_off_v1 v1;
  byte_array_2 reserved;
};
static_assert(offsetof(profile_off_v2, v1) == 0);
static_assert(offsetof(profile_off_v2, reserved) == 5);
static_assert(sizeof(profile_off_v2) == 7);
static_assert(alignof(profile_off_v2) == 1);
static_assert(std::is_trivially_copyable_v<profile_off_v2>);

static_assert(sizeof(profile_off_v1) <= sizeof(profile_off_v2));

}  // end namespace packed

struct profile_off {
  constexpr profile_off() = default;
  constexpr profile_off(profile_off const &) = default;
  constexpr profile_off(profile_off &&) noexcept = default;
  constexpr explicit profile_off(packed::profile_off_v1 const &);
  constexpr explicit profile_off(packed::profile_off_v2 const &);
  ~profile_off() noexcept = default;

  constexpr profile_off &operator=(profile_off const &) = default;
  constexpr profile_off &operator=(profile_off &&) noexcept = default;

  constexpr bool operator==(profile_off const &) const = default;

  byte_array_5 pid{};
  // There's a 14 bit field in the specification that's "reserved"
};

constexpr profile_off::profile_off(packed::profile_off_v1 const &other) : pid{other.pid} {
}
constexpr profile_off::profile_off(packed::profile_off_v2 const &other) : profile_off(other.v1) {
}

//*                __ _ _                     _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   ___ _ _  __ _| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / -_) ' \/ _` | '_ \ / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \___|_||_\__,_|_.__/_\___\__,_| *
//* |_|                                                        *
namespace packed {

struct profile_enabled_v1 {
  byte_array_5 pid;  // Profile ID of Profile that is now enabled
};
static_assert(offsetof(profile_enabled_v1, pid) == 0);
static_assert(sizeof(profile_enabled_v1) == 5);
static_assert(alignof(profile_enabled_v1) == 1);
static_assert(std::is_trivially_copyable_v<profile_enabled_v1>);

struct profile_enabled_v2 {
  profile_enabled_v1 v1;
  byte_array_2 num_channels;
};
static_assert(offsetof(profile_enabled_v2, v1) == 0);
static_assert(offsetof(profile_enabled_v2, num_channels) == 5);
static_assert(sizeof(profile_enabled_v2) == 7);
static_assert(alignof(profile_enabled_v2) == 1);
static_assert(std::is_trivially_copyable_v<profile_enabled_v2>);

static_assert(sizeof(profile_enabled_v1) <= sizeof(profile_enabled_v2));

}  // end namespace packed

struct profile_enabled {
  constexpr profile_enabled() = default;
  constexpr profile_enabled(profile_enabled const &) = default;
  constexpr profile_enabled(profile_enabled &&) noexcept = default;
  constexpr explicit profile_enabled(packed::profile_enabled_v1 const &);
  constexpr explicit profile_enabled(packed::profile_enabled_v2 const &);
  ~profile_enabled() noexcept = default;

  constexpr profile_enabled &operator=(profile_enabled const &) = default;
  constexpr profile_enabled &operator=(profile_enabled &&) noexcept = default;

  constexpr bool operator==(profile_enabled const &) const = default;

  byte_array_5 pid{};
  std::uint16_t num_channels = 0;
};

constexpr profile_enabled::profile_enabled(packed::profile_enabled_v1 const &other) : pid{other.pid} {
}
constexpr profile_enabled::profile_enabled(packed::profile_enabled_v2 const &other) : profile_enabled(other.v1) {
  num_channels = packed::from_le7(other.num_channels);
}

//*                __ _ _          _ _          _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   __| (_)___ __ _| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / _` | (_-</ _` | '_ \ / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \__,_|_/__/\__,_|_.__/_\___\__,_| *
//* |_|                                                          *
namespace packed {

struct profile_disabled_v1 {
  byte_array_5 pid;  // Profile ID of Profile that is now disabled
};
static_assert(offsetof(profile_disabled_v1, pid) == 0);
static_assert(sizeof(profile_disabled_v1) == 5);
static_assert(alignof(profile_disabled_v1) == 1);
static_assert(std::is_trivially_copyable_v<profile_disabled_v1>);

struct profile_disabled_v2 {
  profile_disabled_v1 v1;
  byte_array_2 num_channels;
};
static_assert(offsetof(profile_disabled_v2, v1) == 0);
static_assert(offsetof(profile_disabled_v2, num_channels) == 5);
static_assert(sizeof(profile_disabled_v2) == 7);
static_assert(alignof(profile_disabled_v2) == 1);
static_assert(std::is_trivially_copyable_v<profile_disabled_v2>);

static_assert(sizeof(profile_disabled_v1) <= sizeof(profile_disabled_v2));

}  // end namespace packed

struct profile_disabled {
  constexpr profile_disabled() = default;
  constexpr profile_disabled(profile_disabled const &) = default;
  constexpr profile_disabled(profile_disabled &&) noexcept = default;
  constexpr explicit profile_disabled(packed::profile_disabled_v1 const &);
  constexpr explicit profile_disabled(packed::profile_disabled_v2 const &);
  ~profile_disabled() noexcept = default;

  constexpr profile_disabled &operator=(profile_disabled const &) = default;
  constexpr profile_disabled &operator=(profile_disabled &&) noexcept = default;

  constexpr bool operator==(profile_disabled const &) const = default;

  byte_array_5 pid{};
  std::uint16_t num_channels = 0;
};

constexpr profile_disabled::profile_disabled(packed::profile_disabled_v1 const &other) : pid{other.pid} {
}
constexpr profile_disabled::profile_disabled(packed::profile_disabled_v2 const &other) : profile_disabled(other.v1) {
  num_channels = packed::from_le7(other.num_channels);
}

//*                __ _ _                       _  __ _         _      _         *
//*  _ __ _ _ ___ / _(_) |___   ____ __  ___ __(_)/ _(_)__   __| |__ _| |_ __ _  *
//* | '_ \ '_/ _ \  _| | / -_) (_-< '_ \/ -_) _| |  _| / _| / _` / _` |  _/ _` | *
//* | .__/_| \___/_| |_|_\___| /__/ .__/\___\__|_|_| |_\__| \__,_\__,_|\__\__,_| *
//* |_|                           |_|                                            *
namespace packed {

struct profile_specific_data_v1 {
  byte_array_5 pid;          ///< Profile ID
  byte_array_2 data_length;  ///< Length of following profile specific data (LSB first)
  std::array<std::byte, 1> data;  ///< Profile specific data (array length given by data_length)
};
static_assert(offsetof(profile_specific_data_v1, pid) == 0);
static_assert(offsetof(profile_specific_data_v1, data_length) == 5);
static_assert(offsetof(profile_specific_data_v1, data) == 7);
static_assert(sizeof(profile_specific_data_v1) == 8);
static_assert(alignof(profile_specific_data_v1) == 1);
static_assert(std::is_trivially_copyable_v<profile_specific_data_v1>);

}  // end namespace packed

struct profile_specific_data {
  constexpr profile_specific_data() = default;
  constexpr profile_specific_data(profile_specific_data const &) = default;
  constexpr profile_specific_data(profile_specific_data &&) noexcept = default;
  constexpr explicit profile_specific_data(packed::profile_specific_data_v1 const &);
  ~profile_specific_data() noexcept = default;

  constexpr profile_specific_data &operator=(profile_specific_data const &) = default;
  constexpr profile_specific_data &operator=(profile_specific_data &&) noexcept = default;

  byte_array_5 pid{};                 ///< Profile ID
  std::span<std::byte const> data{};  ///< Profile specific data
};

constexpr profile_specific_data::profile_specific_data(packed::profile_specific_data_v1 const &other)
    : pid{other.pid}, data{std::begin(other.data), packed::from_le7(other.data_length)} {
}

//*                                 _    _ _ _ _   _         *
//*  _ __  ___   __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___ *
//* | '_ \/ -_) / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< *
//* | .__/\___| \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ *
//* |_|                 |_|                                  *
namespace packed {

struct pe_capabilities_v1 {
  std::byte num_simultaneous;
};
static_assert(offsetof(pe_capabilities_v1, num_simultaneous) == 0);
static_assert(sizeof(pe_capabilities_v1) == 1);
static_assert(alignof(pe_capabilities_v1) == 1);
static_assert(std::is_trivially_copyable_v<pe_capabilities_v1>);

struct pe_capabilities_v2 {
  pe_capabilities_v1 v1;
  std::byte major_version;
  std::byte minor_version;
};
static_assert(offsetof(pe_capabilities_v2, v1) == 0);
static_assert(offsetof(pe_capabilities_v2, major_version) == 1);
static_assert(offsetof(pe_capabilities_v2, minor_version) == 2);
static_assert(sizeof(pe_capabilities_v2) == 3);
static_assert(alignof(pe_capabilities_v2) == 1);
static_assert(std::is_trivially_copyable_v<pe_capabilities_v2>);

}  // end namespace packed

struct pe_capabilities {
  constexpr pe_capabilities() = default;
  constexpr pe_capabilities(pe_capabilities const &) = default;
  constexpr pe_capabilities(pe_capabilities &&) noexcept = default;
  constexpr explicit pe_capabilities(packed::pe_capabilities_v1 const &other);
  constexpr explicit pe_capabilities(packed::pe_capabilities_v2 const &other);
  ~pe_capabilities() noexcept = default;

  pe_capabilities &operator=(pe_capabilities const &other) = default;
  pe_capabilities &operator=(pe_capabilities &&other) = default;

  bool operator==(pe_capabilities const &) const = default;

  std::uint8_t num_simultaneous = 0;
  std::uint8_t major_version = 0;
  std::uint8_t minor_version = 0;
};

constexpr pe_capabilities::pe_capabilities(packed::pe_capabilities_v1 const &other)
    : num_simultaneous{static_cast<std::uint8_t>(other.num_simultaneous)} {
}
constexpr pe_capabilities::pe_capabilities(packed::pe_capabilities_v2 const &other) : pe_capabilities{other.v1} {
  major_version{static_cast<std::uint8_t>(other.major_version)};
  minor_version = static_cast<std::uint8_t>(other.minor_version);
}

//*                                 _    _ _ _ _   _                       _       *
//*  _ __  ___   __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___  _ _ ___ _ __| |_  _  *
//* | '_ \/ -_) / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< | '_/ -_) '_ \ | || | *
//* | .__/\___| \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ |_| \___| .__/_|\_, | *
//* |_|                 |_|                                          |_|     |__/  *
namespace packed {

struct pe_capabilities_reply_v1 {
  std::byte num_simultaneous;
};
static_assert(offsetof(pe_capabilities_reply_v1, num_simultaneous) == 0);
static_assert(sizeof(pe_capabilities_reply_v1) == 1);
static_assert(alignof(pe_capabilities_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<pe_capabilities_reply_v1>);

struct pe_capabilities_reply_v2 {
  pe_capabilities_reply_v1 v1;
  std::byte major_version;
  std::byte minor_version;
};
static_assert(offsetof(pe_capabilities_reply_v2, v1) == 0);
static_assert(offsetof(pe_capabilities_reply_v2, major_version) == 1);
static_assert(offsetof(pe_capabilities_reply_v2, minor_version) == 2);
static_assert(sizeof(pe_capabilities_reply_v2) == 3);
static_assert(alignof(pe_capabilities_reply_v2) == 1);
static_assert(std::is_trivially_copyable_v<pe_capabilities_reply_v2>);

}  // end namespace packed

struct pe_capabilities_reply {
  constexpr pe_capabilities_reply() = default;
  constexpr pe_capabilities_reply(pe_capabilities_reply const &) = default;
  constexpr pe_capabilities_reply(pe_capabilities_reply &&) noexcept = default;
  constexpr explicit pe_capabilities_reply(packed::pe_capabilities_reply_v1 const &other);
  constexpr explicit pe_capabilities_reply(packed::pe_capabilities_reply_v2 const &other);
  ~pe_capabilities_reply() noexcept = default;

  pe_capabilities_reply &operator=(pe_capabilities_reply const &other) = default;
  pe_capabilities_reply &operator=(pe_capabilities_reply &&other) = default;

  bool operator==(pe_capabilities_reply const &) const = default;

  std::uint8_t num_simultaneous = 0;
  std::uint8_t major_version = 0;
  std::uint8_t minor_version = 0;
};

constexpr pe_capabilities_reply::pe_capabilities_reply(packed::pe_capabilities_reply_v1 const &other)
    : num_simultaneous{static_cast<std::uint8_t>(other.num_simultaneous)} {
}
constexpr pe_capabilities_reply::pe_capabilities_reply(packed::pe_capabilities_reply_v2 const &other)
    : pe_capabilities_reply{other.v1} {
  major_version = static_cast<std::uint8_t>(other.major_version);
  minor_version = static_cast<std::uint8_t>(other.minor_version);
}

namespace packed {

struct property_exchange_pt1 {
  std::byte request_id;
  byte_array_2 header_length;
  std::array<std::byte, 1> header;
};
static_assert(offsetof(property_exchange_pt1, request_id) == 0);
static_assert(offsetof(property_exchange_pt1, header_length) == 1);
static_assert(offsetof(property_exchange_pt1, header) == 3);
static_assert(alignof(property_exchange_pt1) == 1);
static_assert(sizeof(property_exchange_pt1) == 4);
static_assert(std::is_trivially_copyable_v<property_exchange_pt1>);

struct property_exchange_pt2 {
  byte_array_2 number_of_chunks;
  byte_array_2 chunk_number;
  byte_array_2 data_length;
  std::array<std::byte, 1> data;
};
static_assert(offsetof(property_exchange_pt2, number_of_chunks) == 0);
static_assert(offsetof(property_exchange_pt2, chunk_number) == 2);
static_assert(offsetof(property_exchange_pt2, data_length) == 4);
static_assert(offsetof(property_exchange_pt2, data) == 6);
static_assert(alignof(property_exchange_pt2) == 1);
static_assert(sizeof(property_exchange_pt2) == 7);
static_assert(std::is_trivially_copyable_v<property_exchange_pt2>);

}  // end namespace packed

// This is all related to property exchange...
struct pe_chunk_info {
  bool operator==(pe_chunk_info const &) const = default;

  std::uint16_t number_of_chunks = 0;
  std::uint16_t chunk_number = 0;
};

struct property_exchange {
  std::uint8_t request_id = 0;
  std::span<char const> header;
  std::span<char const> data;
};

}  // end namespace midi2::ci

#endif  // MIDI2_CI_TYPES_HPP
