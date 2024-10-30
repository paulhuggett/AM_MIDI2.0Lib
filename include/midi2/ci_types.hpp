//===-- CI Types --------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_CI_TYPES_HPP
#define MIDI2_CI_TYPES_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <type_traits>

#include "midi2/adt/bitfield.hpp"
#include "midi2/utils.hpp"

namespace midi2::ci {

using byte_array_1 = std::array<std::byte, 1>;
using byte_array_2 = std::array<std::byte, 2>;
using byte_array_3 = std::array<std::byte, 3>;
using byte_array_4 = std::array<std::byte, 4>;
using byte_array_5 = std::array<std::byte, 5>;

constexpr auto FUNCTION_BLOCK = std::uint8_t{0x7F};

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

constexpr byte_array_4 to_le7(std::uint32_t const v) {
  assert(v < (std::uint32_t{1} << 28));
  return {static_cast<std::byte>(v >> (7 * 0)) & mask7b, static_cast<std::byte>(v >> (7 * 1)) & mask7b,
          static_cast<std::byte>(v >> (7 * 2)) & mask7b, static_cast<std::byte>(v >> (7 * 3)) & mask7b};
}
constexpr byte_array_2 to_le7(std::uint16_t const v) {
  assert(v < (std::uint16_t{1} << 14));
  return {static_cast<std::byte>(v >> (7 * 0)) & mask7b, static_cast<std::byte>(v >> (7 * 1)) & mask7b};
}
constexpr std::byte to_le7(std::uint8_t v) {
  assert(v < (std::uint8_t{1} << 7));
  return static_cast<std::byte>(v);
}

template <std::size_t Size>
constexpr std::array<std::uint8_t, Size> from_array(std::array<std::byte, Size> const &other) {
  std::array<std::uint8_t, Size> result{};
  std::ranges::transform(other, std::begin(result), [](std::byte v) { return static_cast<std::uint8_t>(v); });
  return result;
}

template <std::size_t Size>
constexpr std::array<std::byte, Size> to_array(std::array<std::uint8_t, Size> const &other) {
  std::array<std::byte, Size> result{};
  std::ranges::transform(other, std::begin(result), [](std::uint8_t v) { return static_cast<std::byte>(v); });
  return result;
}

namespace packed {

struct header {
  std::byte sysex;
  std::byte source;
  std::byte sub_id_1;  // 0x0D
  std::byte sub_id_2;
  std::byte version;
  byte_array_4 source_muid;
  byte_array_4 destination_muid;
};

static_assert(offsetof(header, sysex) == 0);
static_assert(offsetof(header, source) == 1);
static_assert(offsetof(header, sub_id_1) == 2);
static_assert(offsetof(header, sub_id_2) == 3);
static_assert(offsetof(header, version) == 4);
static_assert(offsetof(header, source_muid) == 5);
static_assert(offsetof(header, destination_muid) == 9);
static_assert(sizeof(header) == 13);
static_assert(alignof(header) == 1);
static_assert(std::is_trivially_copyable_v<header>);

}  // end namespace packed

struct params {
  bool operator==(params const &) const = default;

  explicit constexpr operator packed::header() const;

  std::uint8_t deviceId = 0xFF;
  std::uint8_t ciVer = 1;
  std::uint32_t remoteMUID = 0;
  std::uint32_t localMUID = 0;
};

constexpr params::operator packed::header() const {
  return packed::header{.sysex = S7UNIVERSAL_NRT,
                        .source = static_cast<std::byte>(deviceId),
                        .sub_id_1 = S7MIDICI,
                        .sub_id_2 = std::byte{0},  // message type
                        .version = static_cast<std::byte>(ciVer),
                        .source_muid = ci::to_le7(remoteMUID),
                        .destination_muid = ci::to_le7(localMUID)};
}

struct MIDICI {
  bool operator==(MIDICI const &) const = default;

  std::uint8_t umpGroup = 0xFF;
  ci_message ciType = static_cast<ci_message>(0x00);
  struct params params;
};

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

  explicit constexpr operator packed::discovery_v1() const;
  explicit constexpr operator packed::discovery_v2() const;

  std::array<std::uint8_t, 3> manufacturer{};
  std::uint16_t family = 0;
  std::uint16_t model = 0;
  std::array<std::uint8_t, 4> version{};
  std::uint8_t capability = 0;
  std::uint32_t max_sysex_size = 0;
  std::uint8_t output_path_id = 0;
};

constexpr discovery::discovery(packed::discovery_v1 const &v1)
    : manufacturer{from_array(v1.manufacturer)},
      family{from_le7(v1.family)},
      model{from_le7(v1.model)},
      version{from_array(v1.version)},
      capability{from_le7(v1.capability)},
      max_sysex_size{from_le7(v1.max_sysex_size)} {
}
constexpr discovery::discovery(packed::discovery_v2 const &v2) : discovery(v2.v1) {
  output_path_id = from_le7(v2.output_path_id);
}
constexpr discovery::operator packed::discovery_v1() const {
  return {.manufacturer = to_array(manufacturer),
          .family = to_le7(family),
          .model = to_le7(model),
          .version = to_array(version),
          .capability = static_cast<std::byte>(capability),
          .max_sysex_size = to_le7(max_sysex_size)};
}
constexpr discovery::operator packed::discovery_v2() const {
  return {.v1 = static_cast<packed::discovery_v1>(*this), .output_path_id = to_le7(output_path_id)};
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

  explicit constexpr operator packed::discovery_reply_v1() const;
  explicit constexpr operator packed::discovery_reply_v2() const;

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
    : manufacturer{from_array(v1.manufacturer)},
      family{from_le7(v1.family)},
      model{from_le7(v1.model)},
      version{from_array(v1.version)},
      capability{from_le7(v1.capability)},
      max_sysex_size{from_le7(v1.max_sysex_size)} {
}
constexpr discovery_reply::discovery_reply(packed::discovery_reply_v2 const &v2) : discovery_reply(v2.v1) {
  output_path_id = from_le7(v2.output_path_id);
  function_block = from_le7(v2.function_block);
}

constexpr discovery_reply::operator packed::discovery_reply_v1() const {
  return packed::discovery_reply_v1{.manufacturer = to_array(manufacturer),
                                    .family = to_le7(family),
                                    .model = to_le7(model),
                                    .version = to_array(version),
                                    .capability = static_cast<std::byte>(capability),
                                    .max_sysex_size = to_le7(max_sysex_size)};
}
constexpr discovery_reply::operator packed::discovery_reply_v2() const {
  return {.v1 = static_cast<packed::discovery_reply_v1>(*this),
          .output_path_id = static_cast<std::byte>(output_path_id),
          .function_block = static_cast<std::byte>(function_block)};
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

  explicit constexpr operator packed::endpoint_info_v1() const;

  std::uint8_t status = 0;
};

constexpr endpoint_info::endpoint_info(packed::endpoint_info_v1 const &other)
    : status{static_cast<std::uint8_t>(other.status)} {
}

constexpr endpoint_info::operator packed::endpoint_info_v1() const {
  return {.status = static_cast<std::byte>(status)};
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
  // Don't be tempted to change to std::array<>. MSVC's iterator checks will trip.
  std::byte data[1];  ///< an array of size given by data_length
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

  explicit constexpr operator packed::endpoint_info_reply_v1() const;

  std::byte status{};
  std::span<std::byte const> information;
};

constexpr endpoint_info_reply::endpoint_info_reply(packed::endpoint_info_reply_v1 const &other)
    : status{from_le7(other.status)}, information{std::begin(other.data), from_le7(other.data_length)} {
}

constexpr endpoint_info_reply::operator packed::endpoint_info_reply_v1() const {
  return {
      .status = status, .data_length = to_le7(static_cast<std::uint16_t>(information.size())), .data = {std::byte{0}}};
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

  explicit constexpr operator packed::invalidate_muid_v1() const;

  std::uint32_t target_muid = 0;
};

constexpr invalidate_muid::invalidate_muid(packed::invalidate_muid_v1 const &other)
    : target_muid{from_le7(other.target_muid)} {
}

constexpr invalidate_muid::operator packed::invalidate_muid_v1() const {
  return {to_le7(target_muid)};
}

//*          _    *
//*  __ _ __| |__ *
//* / _` / _| / / *
//* \__,_\__|_\_\ *
//*               *
namespace packed {

struct ack_v1 {
  std::byte original_id;        ///< Original Transaction Sub-ID#2 Classification
  std::byte status_code;        ///< ACK Status Code
  std::byte status_data;        ///< ACK Status Data
  byte_array_5 details;         ///< ACK details for each SubID Classification
  byte_array_2 message_length;  ///< Message Length (LSB firt)
  std::byte message[1];         ///< Message text (array of size given by message_length)
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

  explicit constexpr operator packed::ack_v1() const;

  std::uint8_t original_id = 0;
  std::uint8_t status_code = 0;
  std::uint8_t status_data = 0;
  byte_array_5 details{};
  std::span<std::byte const> message;
};

constexpr ack::ack(packed::ack_v1 const &other)
    : original_id{from_le7(other.original_id)},
      status_code{from_le7(other.status_code)},
      status_data{from_le7(other.status_data)},
      details{other.details},
      message{std::begin(other.message), from_le7(other.message_length)} {
}

constexpr ack::operator packed::ack_v1() const {
  return {to_le7(original_id),
          to_le7(status_code),
          to_le7(status_data),
          details,
          to_le7(static_cast<std::uint16_t>(message.size())),
          {std::byte{0}}};
}

//*            _    *
//*  _ _  __ _| |__ *
//* | ' \/ _` | / / *
//* |_||_\__,_|_\_\ *
//*                 *
namespace packed {

struct nak_v1 {};
struct nak_v2 {
  std::byte original_id;        ///< Original transaction sub-ID#2 classification
  std::byte status_code;        ///< ACK Status Code
  std::byte status_data;        ///< ACK Status Data
  byte_array_5 details;         ///< ACK details for each SubID Classification
  byte_array_2 message_length;  ///< Message Length (LSB firt)
  std::byte message[1];         ///< Message text (length given by message_length)
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

  explicit constexpr operator packed::nak_v1() const;
  explicit constexpr operator packed::nak_v2() const;

  std::uint8_t original_id = 0;  // Original transaction sub-ID#2 classification
  std::uint8_t status_code = 0;  // NAK Status Code
  std::uint8_t status_data = 0;  // NAK Status Data
  byte_array_5 details{};        // NAK details for each SubID Classification
  std::span<std::byte const> message;
};

constexpr nak::nak(packed::nak_v1 const &) {
}
constexpr nak::nak(packed::nak_v2 const &other)
    : original_id{static_cast<std::uint8_t>(other.original_id)},
      status_code{static_cast<std::uint8_t>(other.status_code)},
      status_data{static_cast<std::uint8_t>(other.status_data)},
      details{other.details},
      message{std::begin(other.message), from_le7(other.message_length)} {
}

constexpr nak::operator packed::nak_v1() const {
  return {};
}
constexpr nak::operator packed::nak_v2() const {
  return {to_le7(original_id),
          to_le7(status_code),
          to_le7(status_data),
          details,
          to_le7(static_cast<std::uint16_t>(message.size())),
          {std::byte{0}}};
}

namespace profile_configuration {

//*                __ _ _       _                _           *
//*  _ __ _ _ ___ / _(_) |___  (_)_ _  __ _ _  _(_)_ _ _  _  *
//* | '_ \ '_/ _ \  _| | / -_) | | ' \/ _` | || | | '_| || | *
//* | .__/_| \___/_| |_|_\___| |_|_||_\__, |\_,_|_|_|  \_, | *
//* |_|                                  |_|           |__/  *
struct inquiry {};

//*                __ _ _       _                _                         _       *
//*  _ __ _ _ ___ / _(_) |___  (_)_ _  __ _ _  _(_)_ _ _  _   _ _ ___ _ __| |_  _  *
//* | '_ \ '_/ _ \  _| | / -_) | | ' \/ _` | || | | '_| || | | '_/ -_) '_ \ | || | *
//* | .__/_| \___/_| |_|_\___| |_|_||_\__, |\_,_|_|_|  \_, | |_| \___| .__/_|\_, | *
//* |_|                                  |_|           |__/          |_|     |__/  *

namespace packed {

struct inquiry_reply_v1_pt1 {
  byte_array_2 num_enabled;  ///< Number of currently enabled profiles
  byte_array_5 ids[1];       ///< Profile ID of currently enabled profiles (array length given by num_enabled)
};

static_assert(offsetof(inquiry_reply_v1_pt1, num_enabled) == 0);
static_assert(offsetof(inquiry_reply_v1_pt1, ids) == 2);
static_assert(sizeof(inquiry_reply_v1_pt1) == 7);
static_assert(alignof(inquiry_reply_v1_pt1) == 1);
static_assert(std::is_trivially_copyable_v<inquiry_reply_v1_pt1>);

struct inquiry_reply_v1_pt2 {
  byte_array_2 num_disabled;  ///< Number of currently disabled profiles
  byte_array_5 ids[1];        ///< Profile ID of currently enabled profiles (array length given by num_disabled)
};

static_assert(offsetof(inquiry_reply_v1_pt2, num_disabled) == 0);
static_assert(offsetof(inquiry_reply_v1_pt2, ids) == 2);
static_assert(sizeof(inquiry_reply_v1_pt2) == 7);
static_assert(alignof(inquiry_reply_v1_pt2) == 1);
static_assert(std::is_trivially_copyable_v<inquiry_reply_v1_pt2>);

}  // end namespace packed

struct inquiry_reply {
  constexpr inquiry_reply() = default;
  constexpr inquiry_reply(inquiry_reply const &) = default;
  constexpr inquiry_reply(inquiry_reply &&) noexcept = default;
  constexpr inquiry_reply(std::span<byte_array_5 const> enabled_, std::span<byte_array_5 const> disabled_)
      : enabled{enabled_}, disabled{disabled_} {}
  constexpr inquiry_reply(packed::inquiry_reply_v1_pt1 const &, packed::inquiry_reply_v1_pt2 const &);
  ~inquiry_reply() noexcept = default;

  constexpr inquiry_reply &operator=(inquiry_reply const &) = default;
  constexpr inquiry_reply &operator=(inquiry_reply &&) noexcept = default;

  explicit constexpr operator packed::inquiry_reply_v1_pt1() const;
  explicit constexpr operator packed::inquiry_reply_v1_pt2() const;

  std::span<byte_array_5 const> enabled;
  std::span<byte_array_5 const> disabled;
};

constexpr inquiry_reply::inquiry_reply(packed::inquiry_reply_v1_pt1 const &v1_pt1,
                                       packed::inquiry_reply_v1_pt2 const &v1_pt2)
    : enabled{std::begin(v1_pt1.ids), from_le7(v1_pt1.num_enabled)},
      disabled{std::begin(v1_pt2.ids), from_le7(v1_pt2.num_disabled)} {
}

constexpr inquiry_reply::operator packed::inquiry_reply_v1_pt1() const {
  return {to_le7(static_cast<std::uint16_t>(enabled.size())), {byte_array_5{}}};
}
constexpr inquiry_reply::operator packed::inquiry_reply_v1_pt2() const {
  return {to_le7(static_cast<std::uint16_t>(disabled.size())), {byte_array_5{}}};
}

//*                __ _ _               _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   __ _ __| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / _` / _` / _` / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \__,_\__,_\__,_\___\__,_| *
//* |_|                                                  *
namespace packed {

struct added_v1 {
  byte_array_5 pid;  // Profile ID of profile being added
};
static_assert(offsetof(added_v1, pid) == 0);
static_assert(sizeof(added_v1) == 5);
static_assert(alignof(added_v1) == 1);
static_assert(std::is_trivially_copyable_v<added_v1>);

}  // end namespace packed

struct added {
  constexpr added() = default;
  constexpr added(added const &) = default;
  constexpr added(added &&) noexcept = default;
  constexpr explicit added(packed::added_v1 const &);
  ~added() noexcept = default;

  constexpr added &operator=(added const &) = default;
  constexpr added &operator=(added &&) noexcept = default;

  constexpr bool operator==(added const &) const = default;

  explicit constexpr operator packed::added_v1() const;

  byte_array_5 pid{};
};

constexpr added::added(packed::added_v1 const &other) : pid{other.pid} {
}

constexpr added::operator packed::added_v1() const {
  return {pid};
}

//*                __ _ _                                    _  *
//*  _ __ _ _ ___ / _(_) |___   _ _ ___ _ __  _____ _____ __| | *
//* | '_ \ '_/ _ \  _| | / -_) | '_/ -_) '  \/ _ \ V / -_) _` | *
//* | .__/_| \___/_| |_|_\___| |_| \___|_|_|_\___/\_/\___\__,_| *
//* |_|                                                         *
namespace packed {

struct removed_v1 {
  byte_array_5 pid;  // Profile ID of profile being removed
};
static_assert(offsetof(removed_v1, pid) == 0);
static_assert(sizeof(removed_v1) == 5);
static_assert(alignof(removed_v1) == 1);
static_assert(std::is_trivially_copyable_v<removed_v1>);

}  // end namespace packed

struct removed {
  constexpr removed() = default;
  constexpr removed(removed const &) = default;
  constexpr removed(removed &&) noexcept = default;
  constexpr explicit removed(packed::removed_v1 const &);
  ~removed() noexcept = default;

  constexpr removed &operator=(removed const &) = default;
  constexpr removed &operator=(removed &&) noexcept = default;

  constexpr bool operator==(removed const &) const = default;

  explicit constexpr operator packed::removed_v1() const;

  byte_array_5 pid{};
};

constexpr removed::removed(packed::removed_v1 const &other) : pid{other.pid} {
}
constexpr removed::operator packed::removed_v1() const {
  return {pid};
}

//*                __      _     _        _ _      _                _           *
//*  _ __ _ _ ___ / _|  __| |___| |_ __ _(_) |___ (_)_ _  __ _ _  _(_)_ _ _  _  *
//* | '_ \ '_/ _ \  _| / _` / -_)  _/ _` | | (_-< | | ' \/ _` | || | | '_| || | *
//* | .__/_| \___/_|   \__,_\___|\__\__,_|_|_/__/ |_|_||_\__, |\_,_|_|_|  \_, | *
//* |_|                                                     |_|           |__/  *
namespace packed {

struct details_v1 {
  byte_array_5 pid;  // Profile ID of profile
  std::byte target;
};
static_assert(offsetof(details_v1, pid) == 0);
static_assert(offsetof(details_v1, target) == 5);
static_assert(sizeof(details_v1) == 6);
static_assert(alignof(details_v1) == 1);
static_assert(std::is_trivially_copyable_v<details_v1>);

}  // end namespace packed

struct details {
  constexpr details() = default;
  constexpr details(details const &) = default;
  constexpr details(details &&) noexcept = default;
  constexpr explicit details(packed::details_v1 const &);
  ~details() noexcept = default;

  constexpr details &operator=(details const &) = default;
  constexpr details &operator=(details &&) noexcept = default;

  constexpr bool operator==(details const &) const = default;

  explicit constexpr operator packed::details_v1() const;

  byte_array_5 pid{};
  std::uint8_t target = 0;
};

constexpr details::details(packed::details_v1 const &other)
    : pid{other.pid}, target{static_cast<std::uint8_t>(other.target)} {
}
constexpr details::operator packed::details_v1() const {
  return {pid, to_le7(target)};
}

namespace packed {

struct details_reply_v1 {
  byte_array_5 pid;          ///< Profile ID of profile
  std::byte target;          ///< Inquiry target
  byte_array_2 data_length;  ///< Inquiry target data length (LSB first)
  std::byte data[1];         ///< Array length given by data_length
};

static_assert(offsetof(details_reply_v1, pid) == 0);
static_assert(offsetof(details_reply_v1, target) == 5);
static_assert(offsetof(details_reply_v1, data_length) == 6);
static_assert(offsetof(details_reply_v1, data) == 8);
static_assert(sizeof(details_reply_v1) == 9);
static_assert(alignof(details_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<details_reply_v1>);

}  // end namespace packed

struct details_reply {
  constexpr details_reply() = default;
  constexpr details_reply(byte_array_5 const &pid_, std::uint8_t target_, std::span<std::byte const> data_)
      : pid{pid_}, target{target_}, data{data_} {}
  constexpr details_reply(details_reply const &) = default;
  constexpr details_reply(details_reply &&) noexcept = default;
  constexpr explicit details_reply(packed::details_reply_v1 const &);
  ~details_reply() noexcept = default;

  constexpr details_reply &operator=(details_reply const &) = default;
  constexpr details_reply &operator=(details_reply &&) noexcept = default;

  explicit constexpr operator packed::details_reply_v1() const;

  byte_array_5 pid{};       ///< Profile ID of profile
  std::uint8_t target = 0;  ///< Inquiry target
  std::span<std::byte const> data;
};

constexpr details_reply::details_reply(packed::details_reply_v1 const &other)
    : pid{other.pid},
      target{static_cast<std::uint8_t>(other.target)},
      data{std::begin(other.data), from_le7(other.data_length)} {
}

constexpr details_reply::operator packed::details_reply_v1() const {
  return {
      pid,
      static_cast<std::byte>(target),
      to_le7(static_cast<std::uint16_t>(data.size_bytes())),  ///< Inquiry target data length (LSB first)
      {std::byte{0}},
  };
}

//*                __ _ _                 *
//*  _ __ _ _ ___ / _(_) |___   ___ _ _   *
//* | '_ \ '_/ _ \  _| | / -_) / _ \ ' \  *
//* | .__/_| \___/_| |_|_\___| \___/_||_| *
//* |_|                                   *
namespace packed {

struct on_v1 {
  byte_array_5 pid;  // Profile ID of profile to be set to on (to be enabled)
};
static_assert(offsetof(on_v1, pid) == 0);
static_assert(sizeof(on_v1) == 5);
static_assert(alignof(on_v1) == 1);
static_assert(std::is_trivially_copyable_v<on_v1>);

struct on_v2 {
  on_v1 v1;
  byte_array_2 num_channels;  // Number Channels Requested (LSB First) to assign to this profile when it is enabled
};
static_assert(offsetof(on_v2, v1) == 0);
static_assert(offsetof(on_v2, num_channels) == 5);
static_assert(sizeof(on_v2) == 7);
static_assert(alignof(on_v2) == 1);
static_assert(std::is_trivially_copyable_v<on_v2>);

static_assert(sizeof(on_v1) <= sizeof(on_v2));

}  // end namespace packed

struct on {
  constexpr on() = default;
  constexpr on(byte_array_5 const &pid_, std::uint16_t num_channels_) : pid{pid_}, num_channels{num_channels_} {}
  constexpr on(on const &) = default;
  constexpr on(on &&) noexcept = default;
  constexpr explicit on(packed::on_v1 const &);
  constexpr explicit on(packed::on_v2 const &);
  ~on() noexcept = default;

  constexpr on &operator=(on const &) = default;
  constexpr on &operator=(on &&) noexcept = default;

  constexpr bool operator==(on const &) const = default;

  explicit constexpr operator packed::on_v1() const;
  explicit constexpr operator packed::on_v2() const;

  byte_array_5 pid{};
  std::uint16_t num_channels = 0;
};

constexpr on::on(packed::on_v1 const &other) : pid{other.pid} {
}
constexpr on::on(packed::on_v2 const &other) : on(other.v1) {
  num_channels = from_le7(other.num_channels);
}
constexpr on::operator packed::on_v1() const {
  return {pid};
}
constexpr on::operator packed::on_v2() const {
  return {static_cast<packed::on_v1>(*this), to_le7(num_channels)};
}

//*                __ _ _            __  __  *
//*  _ __ _ _ ___ / _(_) |___   ___ / _|/ _| *
//* | '_ \ '_/ _ \  _| | / -_) / _ \  _|  _| *
//* | .__/_| \___/_| |_|_\___| \___/_| |_|   *
//* |_|                                      *
namespace packed {

struct off_v1 {
  byte_array_5 pid;  // Profile ID of Profile to be Set to On (to be Enabled)
};
static_assert(offsetof(off_v1, pid) == 0);
static_assert(sizeof(off_v1) == 5);
static_assert(alignof(off_v1) == 1);
static_assert(std::is_trivially_copyable_v<off_v1>);

struct off_v2 {
  off_v1 v1;
  byte_array_2 reserved;
};
static_assert(offsetof(off_v2, v1) == 0);
static_assert(offsetof(off_v2, reserved) == 5);
static_assert(sizeof(off_v2) == 7);
static_assert(alignof(off_v2) == 1);
static_assert(std::is_trivially_copyable_v<off_v2>);

static_assert(sizeof(off_v1) <= sizeof(off_v2));

}  // end namespace packed

struct off {
  constexpr off() = default;
  explicit constexpr off(byte_array_5 const &pid_) : pid{pid_} {}
  constexpr off(off const &) = default;
  constexpr off(off &&) noexcept = default;
  constexpr explicit off(packed::off_v1 const &);
  constexpr explicit off(packed::off_v2 const &);
  ~off() noexcept = default;

  constexpr off &operator=(off const &) = default;
  constexpr off &operator=(off &&) noexcept = default;

  constexpr bool operator==(off const &) const = default;

  explicit constexpr operator packed::off_v1() const;
  explicit constexpr operator packed::off_v2() const;

  byte_array_5 pid{};
  // There's a 14 bit field in the specification that's "reserved"
};

constexpr off::off(packed::off_v1 const &other) : pid{other.pid} {
}
constexpr off::off(packed::off_v2 const &other) : off(other.v1) {
}
constexpr off::operator packed::off_v1() const {
  return {pid};
}
constexpr off::operator packed::off_v2() const {
  return {static_cast<packed::off_v1>(*this), byte_array_2{}};
}

//*                __ _ _                     _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   ___ _ _  __ _| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / -_) ' \/ _` | '_ \ / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \___|_||_\__,_|_.__/_\___\__,_| *
//* |_|                                                        *
namespace packed {

struct enabled_v1 {
  byte_array_5 pid;  // Profile ID of Profile that is now enabled
};
static_assert(offsetof(enabled_v1, pid) == 0);
static_assert(sizeof(enabled_v1) == 5);
static_assert(alignof(enabled_v1) == 1);
static_assert(std::is_trivially_copyable_v<enabled_v1>);

struct enabled_v2 {
  enabled_v1 v1;
  byte_array_2 num_channels;
};
static_assert(offsetof(enabled_v2, v1) == 0);
static_assert(offsetof(enabled_v2, num_channels) == 5);
static_assert(sizeof(enabled_v2) == 7);
static_assert(alignof(enabled_v2) == 1);
static_assert(std::is_trivially_copyable_v<enabled_v2>);

static_assert(sizeof(enabled_v1) <= sizeof(enabled_v2));

}  // end namespace packed

struct enabled {
  constexpr enabled() = default;
  constexpr enabled(enabled const &) = default;
  constexpr enabled(enabled &&) noexcept = default;
  constexpr explicit enabled(packed::enabled_v1 const &);
  constexpr explicit enabled(packed::enabled_v2 const &);
  ~enabled() noexcept = default;

  constexpr enabled &operator=(enabled const &) = default;
  constexpr enabled &operator=(enabled &&) noexcept = default;

  constexpr bool operator==(enabled const &) const = default;

  explicit constexpr operator packed::enabled_v1() const;
  explicit constexpr operator packed::enabled_v2() const;

  byte_array_5 pid{};
  std::uint16_t num_channels = 0;
};

constexpr enabled::enabled(packed::enabled_v1 const &other) : pid{other.pid} {
}
constexpr enabled::enabled(packed::enabled_v2 const &other) : enabled(other.v1) {
  num_channels = from_le7(other.num_channels);
}
constexpr enabled::operator packed::enabled_v1() const {
  return {pid};
}
constexpr enabled::operator packed::enabled_v2() const {
  return {static_cast<packed::enabled_v1>(*this), to_le7(num_channels)};
}

//*                __ _ _          _ _          _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   __| (_)___ __ _| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / _` | (_-</ _` | '_ \ / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \__,_|_/__/\__,_|_.__/_\___\__,_| *
//* |_|                                                          *
namespace packed {

struct disabled_v1 {
  byte_array_5 pid;  // Profile ID of Profile that is now disabled
};
static_assert(offsetof(disabled_v1, pid) == 0);
static_assert(sizeof(disabled_v1) == 5);
static_assert(alignof(disabled_v1) == 1);
static_assert(std::is_trivially_copyable_v<disabled_v1>);

struct disabled_v2 {
  disabled_v1 v1;
  byte_array_2 num_channels;
};
static_assert(offsetof(disabled_v2, v1) == 0);
static_assert(offsetof(disabled_v2, num_channels) == 5);
static_assert(sizeof(disabled_v2) == 7);
static_assert(alignof(disabled_v2) == 1);
static_assert(std::is_trivially_copyable_v<disabled_v2>);

static_assert(sizeof(disabled_v1) <= sizeof(disabled_v2));

}  // end namespace packed

struct disabled {
  constexpr disabled() = default;
  constexpr disabled(disabled const &) = default;
  constexpr disabled(disabled &&) noexcept = default;
  constexpr explicit disabled(packed::disabled_v1 const &);
  constexpr explicit disabled(packed::disabled_v2 const &);
  ~disabled() noexcept = default;

  constexpr disabled &operator=(disabled const &) = default;
  constexpr disabled &operator=(disabled &&) noexcept = default;

  constexpr bool operator==(disabled const &) const = default;

  explicit constexpr operator packed::disabled_v1() const;
  explicit constexpr operator packed::disabled_v2() const;

  byte_array_5 pid{};
  std::uint16_t num_channels = 0;
};

constexpr disabled::disabled(packed::disabled_v1 const &other) : pid{other.pid} {
}
constexpr disabled::disabled(packed::disabled_v2 const &other) : disabled(other.v1) {
  num_channels = from_le7(other.num_channels);
}
constexpr disabled::operator packed::disabled_v1() const {
  return {pid};
}
constexpr disabled::operator packed::disabled_v2() const {
  return {static_cast<packed::disabled_v1>(*this), to_le7(num_channels)};
}

//*                __ _ _                       _  __ _         _      _         *
//*  _ __ _ _ ___ / _(_) |___   ____ __  ___ __(_)/ _(_)__   __| |__ _| |_ __ _  *
//* | '_ \ '_/ _ \  _| | / -_) (_-< '_ \/ -_) _| |  _| / _| / _` / _` |  _/ _` | *
//* | .__/_| \___/_| |_|_\___| /__/ .__/\___\__|_|_| |_\__| \__,_\__,_|\__\__,_| *
//* |_|                           |_|                                            *
namespace packed {

struct specific_data_v1 {
  byte_array_5 pid;          ///< Profile ID
  byte_array_2 data_length;  ///< Length of following profile specific data (LSB first)
  std::byte data[1];         ///< Profile specific data (array length given by data_length)
};
static_assert(offsetof(specific_data_v1, pid) == 0);
static_assert(offsetof(specific_data_v1, data_length) == 5);
static_assert(offsetof(specific_data_v1, data) == 7);
static_assert(sizeof(specific_data_v1) == 8);
static_assert(alignof(specific_data_v1) == 1);
static_assert(std::is_trivially_copyable_v<specific_data_v1>);

}  // end namespace packed

struct specific_data {
  constexpr specific_data() = default;
  constexpr specific_data(byte_array_5 const &pid, std::span<std::byte const> data);
  constexpr specific_data(specific_data const &) = default;
  constexpr specific_data(specific_data &&) noexcept = default;
  constexpr explicit specific_data(packed::specific_data_v1 const &);
  ~specific_data() noexcept = default;

  constexpr specific_data &operator=(specific_data const &) = default;
  constexpr specific_data &operator=(specific_data &&) noexcept = default;

  explicit constexpr operator packed::specific_data_v1() const;

  byte_array_5 pid{};               ///< Profile ID
  std::span<std::byte const> data;  ///< Profile specific data
};

constexpr specific_data::specific_data(byte_array_5 const &pid_, std::span<std::byte const> data_)
    : pid{pid_}, data{data_} {
}
constexpr specific_data::specific_data(packed::specific_data_v1 const &other)
    : pid{other.pid}, data{std::begin(other.data), from_le7(other.data_length)} {
}
constexpr specific_data::operator packed::specific_data_v1() const {
  return {pid, to_le7(static_cast<std::uint16_t>(data.size_bytes())), {std::byte{0}}};
}

}  // namespace profile_configuration

namespace property_exchange {

//*                                 _    _ _ _ _   _         *
//*  _ __  ___   __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___ *
//* | '_ \/ -_) / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< *
//* | .__/\___| \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ *
//* |_|                 |_|                                  *
namespace packed {

struct capabilities_v1 {
  std::byte num_simultaneous;
};
static_assert(offsetof(capabilities_v1, num_simultaneous) == 0);
static_assert(sizeof(capabilities_v1) == 1);
static_assert(alignof(capabilities_v1) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_v1>);

struct capabilities_v2 {
  capabilities_v1 v1;
  std::byte major_version;
  std::byte minor_version;
};
static_assert(offsetof(capabilities_v2, v1) == 0);
static_assert(offsetof(capabilities_v2, major_version) == 1);
static_assert(offsetof(capabilities_v2, minor_version) == 2);
static_assert(sizeof(capabilities_v2) == 3);
static_assert(alignof(capabilities_v2) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_v2>);

}  // end namespace packed

struct capabilities {
  constexpr capabilities() = default;
  constexpr capabilities(std::uint8_t ns, std::uint8_t major, std::uint8_t minor);
  constexpr capabilities(capabilities const &) = default;
  constexpr capabilities(capabilities &&) noexcept = default;
  constexpr explicit capabilities(packed::capabilities_v1 const &other);
  constexpr explicit capabilities(packed::capabilities_v2 const &other);
  ~capabilities() noexcept = default;

  capabilities &operator=(capabilities const &other) = default;
  capabilities &operator=(capabilities &&other) noexcept = default;

  bool operator==(capabilities const &) const = default;

  explicit constexpr operator packed::capabilities_v1() const;
  explicit constexpr operator packed::capabilities_v2() const;

  std::uint8_t num_simultaneous = 0;
  std::uint8_t major_version = 0;
  std::uint8_t minor_version = 0;
};

constexpr capabilities::capabilities(std::uint8_t ns, std::uint8_t major, std::uint8_t minor)
    : num_simultaneous{ns}, major_version{major}, minor_version{minor} {
}
constexpr capabilities::capabilities(packed::capabilities_v1 const &other)
    : num_simultaneous{static_cast<std::uint8_t>(other.num_simultaneous)} {
}
constexpr capabilities::capabilities(packed::capabilities_v2 const &other) : capabilities{other.v1} {
  major_version = static_cast<std::uint8_t>(other.major_version);
  minor_version = static_cast<std::uint8_t>(other.minor_version);
}
constexpr capabilities::operator packed::capabilities_v1() const {
  return {static_cast<std::byte>(num_simultaneous)};
}
constexpr capabilities::operator packed::capabilities_v2() const {
  return {static_cast<packed::capabilities_v1>(*this), static_cast<std::byte>(major_version),
          static_cast<std::byte>(minor_version)};
}

//*                                 _    _ _ _ _   _                       _       *
//*  _ __  ___   __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___  _ _ ___ _ __| |_  _  *
//* | '_ \/ -_) / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< | '_/ -_) '_ \ | || | *
//* | .__/\___| \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ |_| \___| .__/_|\_, | *
//* |_|                 |_|                                          |_|     |__/  *
namespace packed {

struct capabilities_reply_v1 {
  std::byte num_simultaneous;
};
static_assert(offsetof(capabilities_reply_v1, num_simultaneous) == 0);
static_assert(sizeof(capabilities_reply_v1) == 1);
static_assert(alignof(capabilities_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_reply_v1>);

struct capabilities_reply_v2 {
  capabilities_reply_v1 v1;
  std::byte major_version;
  std::byte minor_version;
};
static_assert(offsetof(capabilities_reply_v2, v1) == 0);
static_assert(offsetof(capabilities_reply_v2, major_version) == 1);
static_assert(offsetof(capabilities_reply_v2, minor_version) == 2);
static_assert(sizeof(capabilities_reply_v2) == 3);
static_assert(alignof(capabilities_reply_v2) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_reply_v2>);

}  // end namespace packed

struct capabilities_reply {
  constexpr capabilities_reply() = default;
  constexpr capabilities_reply(std::uint8_t ns, std::uint8_t major, std::uint8_t minor);
  constexpr capabilities_reply(capabilities_reply const &) = default;
  constexpr capabilities_reply(capabilities_reply &&) noexcept = default;
  constexpr explicit capabilities_reply(packed::capabilities_reply_v1 const &other);
  constexpr explicit capabilities_reply(packed::capabilities_reply_v2 const &other);
  ~capabilities_reply() noexcept = default;

  capabilities_reply &operator=(capabilities_reply const &other) = default;
  capabilities_reply &operator=(capabilities_reply &&other) = default;

  bool operator==(capabilities_reply const &) const = default;

  explicit constexpr operator packed::capabilities_reply_v1() const;
  explicit constexpr operator packed::capabilities_reply_v2() const;

  std::uint8_t num_simultaneous = 0;
  std::uint8_t major_version = 0;
  std::uint8_t minor_version = 0;
};

constexpr capabilities_reply::capabilities_reply(std::uint8_t ns, std::uint8_t major, std::uint8_t minor)
    : num_simultaneous{ns}, major_version{major}, minor_version{minor} {
}
constexpr capabilities_reply::capabilities_reply(packed::capabilities_reply_v1 const &other)
    : num_simultaneous{static_cast<std::uint8_t>(other.num_simultaneous)} {
}
constexpr capabilities_reply::capabilities_reply(packed::capabilities_reply_v2 const &other)
    : capabilities_reply{other.v1} {
  major_version = static_cast<std::uint8_t>(other.major_version);
  minor_version = static_cast<std::uint8_t>(other.minor_version);
}
constexpr capabilities_reply::operator packed::capabilities_reply_v1() const {
  return {static_cast<std::byte>(num_simultaneous)};
}
constexpr capabilities_reply::operator packed::capabilities_reply_v2() const {
  return {static_cast<packed::capabilities_reply_v1>(*this), static_cast<std::byte>(major_version),
          static_cast<std::byte>(minor_version)};
}

//*                             _                     _                        *
//*  _ __ _ _ ___ _ __  ___ _ _| |_ _  _   _____ ____| |_  __ _ _ _  __ _ ___  *
//* | '_ \ '_/ _ \ '_ \/ -_) '_|  _| || | / -_) \ / _| ' \/ _` | ' \/ _` / -_) *
//* | .__/_| \___/ .__/\___|_|  \__|\_, | \___/_\_\__|_||_\__,_|_||_\__, \___| *
//* |_|          |_|                |__/                            |___/      *
namespace packed {

struct property_exchange_pt1 {
  std::byte request_id;
  byte_array_2 header_length;
  std::byte header[1];
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
  std::byte data[1];
};
static_assert(offsetof(property_exchange_pt2, number_of_chunks) == 0);
static_assert(offsetof(property_exchange_pt2, chunk_number) == 2);
static_assert(offsetof(property_exchange_pt2, data_length) == 4);
static_assert(offsetof(property_exchange_pt2, data) == 6);
static_assert(alignof(property_exchange_pt2) == 1);
static_assert(sizeof(property_exchange_pt2) == 7);
static_assert(std::is_trivially_copyable_v<property_exchange_pt2>);

}  // end namespace packed

struct property_exchange {
  struct chunk_info {
    chunk_info() = default;
    chunk_info(std::uint16_t const number_of_chunks_, std::uint16_t const chunk_number_)
        : number_of_chunks{number_of_chunks_}, chunk_number{chunk_number_} {}
    bool operator==(chunk_info const &) const = default;
    std::uint16_t number_of_chunks = 0;
    std::uint16_t chunk_number = 0;
  };
  chunk_info chunk;
  std::uint8_t request = 0;
  std::span<char const> header;
  std::span<char const> data;

  explicit constexpr operator packed::property_exchange_pt1() const;
  explicit constexpr operator packed::property_exchange_pt2() const;

protected:
  constexpr property_exchange() = default;

  constexpr property_exchange(chunk_info const &chunk_, std::uint8_t request_, std::span<char const> header_,
                              std::span<char const> data_)
      : chunk{chunk_}, request{request_}, header{header_}, data{data_} {}
};

constexpr property_exchange::operator packed::property_exchange_pt1() const {
  return {
      static_cast<std::byte>(request),
      to_le7(static_cast<std::uint16_t>(header.size())),
      {std::byte{0}},
  };
}

constexpr property_exchange::operator packed::property_exchange_pt2() const {
  return {
      to_le7(chunk.number_of_chunks),
      to_le7(chunk.chunk_number),
      to_le7(static_cast<std::uint16_t>(data.size())),
      {std::byte{0}},
  };
}

struct get : public property_exchange {
public:
  constexpr get() = default;
  constexpr get(chunk_info const &chunk_, std::uint8_t const request_, std::span<char const> const header_)
      : property_exchange(chunk_, request_, header_, {}) {}

private:
  using property_exchange::data;
};
struct get_reply : public property_exchange {
  constexpr get_reply() = default;
  constexpr get_reply(chunk_info const &chunk_, std::uint8_t const request_, std::span<char const> const header_,
                      std::span<char const> const data_)
      : property_exchange(chunk_, request_, header_, data_) {}
};
struct set : public property_exchange {
  constexpr set() = default;
  constexpr set(chunk_info const &chunk_, std::uint8_t const request_, std::span<char const> const header_,
                std::span<char const> const data_)
      : property_exchange(chunk_, request_, header_, data_) {}
};
struct set_reply : public property_exchange {
  constexpr set_reply() = default;
  constexpr set_reply(chunk_info const &chunk_, std::uint8_t const request_, std::span<char const> const header_,
                      std::span<char const> const data_)
      : property_exchange(chunk_, request_, header_, data_) {}

private:
  using property_exchange::data;
};
struct subscription : public property_exchange {
  constexpr subscription() = default;
  constexpr subscription(chunk_info const &chunk_, std::uint8_t const request_, std::span<char const> const header_,
                         std::span<char const> const data_)
      : property_exchange(chunk_, request_, header_, data_) {}
};
struct subscription_reply : public property_exchange {
  constexpr subscription_reply() = default;
  constexpr subscription_reply(chunk_info const &chunk_, std::uint8_t const request_,
                               std::span<char const> const header_, std::span<char const> const data_)
      : property_exchange(chunk_, request_, header_, data_) {}
};
struct notify : public property_exchange {
  constexpr notify() = default;
  constexpr notify(chunk_info const &chunk_, std::uint8_t const request_, std::span<char const> const header_,
                   std::span<char const> const data_)
      : property_exchange(chunk_, request_, header_, data_) {}
};

}  // end namespace property_exchange

namespace process_inquiry {

//*                     _    _ _ _ _   _         *
//*  __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___ *
//* / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< *
//* \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ *
//*         |_|                                  *
struct capabilities {};

//*                     _    _ _ _ _   _                       _       *
//*  __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___  _ _ ___ _ __| |_  _  *
//* / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< | '_/ -_) '_ \ | || | *
//* \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ |_| \___| .__/_|\_, | *
//*         |_|                                          |_|     |__/  *
namespace packed {

struct capabilities_reply_v2 {
  std::byte features;
};

static_assert(offsetof(capabilities_reply_v2, features) == 0);
static_assert(alignof(capabilities_reply_v2) == 1);
static_assert(sizeof(capabilities_reply_v2) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_reply_v2>);

}  // end namespace packed

struct capabilities_reply {
  constexpr capabilities_reply() = default;
  constexpr capabilities_reply(capabilities_reply const &) = default;
  constexpr capabilities_reply(capabilities_reply &&) noexcept = default;
  constexpr explicit capabilities_reply(std::byte features_);
  constexpr explicit capabilities_reply(packed::capabilities_reply_v2 const &v2);
  ~capabilities_reply() noexcept = default;

  capabilities_reply &operator=(capabilities_reply const &) = default;
  capabilities_reply &operator=(capabilities_reply &&) noexcept = default;

  bool operator==(capabilities_reply const &) const = default;

  explicit constexpr operator packed::capabilities_reply_v2() const;

  std::byte features = std::byte{0};
};

constexpr capabilities_reply::capabilities_reply(std::byte features_) : features{features_} {
}
constexpr capabilities_reply::capabilities_reply(packed::capabilities_reply_v2 const &v2) : features{v2.features} {
}
constexpr capabilities_reply::operator packed::capabilities_reply_v2() const {
  return {features};
}

//*        _    _ _                                                          _    *
//*  _ __ (_)__| (_)  _ __  ___ ______ __ _ __ _ ___   _ _ ___ _ __  ___ _ _| |_  *
//* | '  \| / _` | | | '  \/ -_|_-<_-</ _` / _` / -_) | '_/ -_) '_ \/ _ \ '_|  _| *
//* |_|_|_|_\__,_|_| |_|_|_\___/__/__/\__,_\__, \___| |_| \___| .__/\___/_|  \__| *
//*                                        |___/              |_|                 *
namespace packed {

struct midi_message_report_v2 {
  union system_message {
    bitfield<std::uint8_t, 3, 5> reserved0;
    bitfield<std::uint8_t, 2, 1> song_select;
    bitfield<std::uint8_t, 1, 1> song_position;
    bitfield<std::uint8_t, 0, 1> mtc_quarter_frame;
  };
  union channel_controller {
    bitfield<std::uint8_t, 6, 2> reserved0;
    bitfield<std::uint8_t, 5, 1> channel_pressure;
    bitfield<std::uint8_t, 4, 1> program_change;
    bitfield<std::uint8_t, 3, 1> nrpn_assignable_controller;
    bitfield<std::uint8_t, 2, 1> rpn_registered_controller;
    bitfield<std::uint8_t, 1, 1> control_change;
    bitfield<std::uint8_t, 0, 1> pitchbend;
  };
  union note_data_messages {
    bitfield<std::uint8_t, 5, 3> reserved0;
    bitfield<std::uint8_t, 4, 1> assignable_per_note_controller;
    bitfield<std::uint8_t, 3, 1> registered_per_note_controller;
    bitfield<std::uint8_t, 2, 1> per_note_pitchbend;
    bitfield<std::uint8_t, 1, 1> poly_pressure;
    bitfield<std::uint8_t, 0, 1> notes;
  };

  std::byte message_data_control;
  union system_message system_message;
  std::byte reserved;
  union channel_controller channel_controller;
  union note_data_messages note_data_messages;
};

static_assert(offsetof(midi_message_report_v2, message_data_control) == 0);
static_assert(offsetof(midi_message_report_v2, system_message) == 1);
static_assert(offsetof(midi_message_report_v2, reserved) == 2);
static_assert(offsetof(midi_message_report_v2, channel_controller) == 3);
static_assert(offsetof(midi_message_report_v2, note_data_messages) == 4);
static_assert(sizeof(midi_message_report_v2) == 5);
static_assert(alignof(midi_message_report_v2) == 1);
static_assert(std::is_trivially_copyable_v<midi_message_report_v2>);

}  // end namespace packed

struct midi_message_report {
  constexpr midi_message_report() = default;
  constexpr midi_message_report(midi_message_report const &) = default;
  constexpr midi_message_report(midi_message_report &&) noexcept = default;
  constexpr explicit midi_message_report(packed::midi_message_report_v2 const &v2);
  ~midi_message_report() noexcept = default;

  midi_message_report &operator=(midi_message_report const &) = default;
  midi_message_report &operator=(midi_message_report &&) noexcept = default;

  bool operator==(midi_message_report const &) const = default;

  constexpr explicit operator packed::midi_message_report_v2() const;

  enum class control : std::uint8_t {
    no_data = 0x00,
    only_non_default = 0x01,
    full = 0x7F,
  };

  control message_data_control = control::no_data;
  // system messages
  unsigned mtc_quarter_frame : 1 = 0;
  unsigned song_position : 1 = 0;
  unsigned song_select : 1 = 0;
  // channel controller messages
  unsigned pitchbend : 1 = 0;
  unsigned control_change : 1 = 0;
  unsigned rpn_registered_controller : 1 = 0;
  unsigned nrpn_assignable_controller : 1 = 0;
  unsigned program_change : 1 = 0;
  unsigned channel_pressure : 1 = 0;
  // note data messages
  unsigned notes : 1 = 0;
  unsigned poly_pressure : 1 = 0;
  unsigned per_note_pitchbend : 1 = 0;
  unsigned registered_per_note_controller : 1 = 0;
  unsigned assignable_per_note_controller : 1 = 0;
};

constexpr midi_message_report::midi_message_report(packed::midi_message_report_v2 const &v2)
    : message_data_control{static_cast<enum control>(v2.message_data_control)},

      mtc_quarter_frame{v2.system_message.mtc_quarter_frame},
      song_position{v2.system_message.song_position},
      song_select{v2.system_message.song_select},

      pitchbend{v2.channel_controller.pitchbend},
      control_change{v2.channel_controller.control_change},
      rpn_registered_controller{v2.channel_controller.rpn_registered_controller},
      nrpn_assignable_controller{v2.channel_controller.nrpn_assignable_controller},
      program_change{v2.channel_controller.program_change},
      channel_pressure{v2.channel_controller.channel_pressure},

      notes{v2.note_data_messages.notes},
      poly_pressure{v2.note_data_messages.poly_pressure},
      per_note_pitchbend{v2.note_data_messages.per_note_pitchbend},
      registered_per_note_controller{v2.note_data_messages.registered_per_note_controller},
      assignable_per_note_controller{v2.note_data_messages.assignable_per_note_controller} {
}

constexpr midi_message_report::operator packed::midi_message_report_v2() const {
  union packed::midi_message_report_v2::system_message sm{};
  sm.mtc_quarter_frame = mtc_quarter_frame;
  sm.song_position = song_position;
  sm.song_select = song_select;

  union packed::midi_message_report_v2::channel_controller cc{};
  cc.pitchbend = pitchbend;
  cc.control_change = control_change;
  cc.rpn_registered_controller = rpn_registered_controller;
  cc.nrpn_assignable_controller = nrpn_assignable_controller;
  cc.program_change = program_change;
  cc.channel_pressure = channel_pressure;

  union packed::midi_message_report_v2::note_data_messages ndm{};
  ndm.notes = notes;
  ndm.poly_pressure = poly_pressure;
  ndm.per_note_pitchbend = per_note_pitchbend;
  ndm.registered_per_note_controller = registered_per_note_controller;
  ndm.assignable_per_note_controller = assignable_per_note_controller;

  return {.message_data_control = static_cast<std::byte>(message_data_control),
          .system_message = sm,
          .reserved = std::byte{0},
          .channel_controller = cc,
          .note_data_messages = ndm};
}

namespace packed {

struct midi_message_report_reply_v2 {
  union system_message {
    bitfield<std::uint8_t, 3, 5> reserved0;
    bitfield<std::uint8_t, 2, 1> song_select;
    bitfield<std::uint8_t, 1, 1> song_position;
    bitfield<std::uint8_t, 0, 1> mtc_quarter_frame;
  };
  union channel_controller {
    bitfield<std::uint8_t, 6, 2> reserved0;
    bitfield<std::uint8_t, 5, 1> channel_pressure;
    bitfield<std::uint8_t, 4, 1> program_change;
    bitfield<std::uint8_t, 3, 1> nrpn_assignable_controller;
    bitfield<std::uint8_t, 2, 1> rpn_registered_controller;
    bitfield<std::uint8_t, 1, 1> control_change;
    bitfield<std::uint8_t, 0, 1> pitchbend;
  };
  union note_data_messages {
    bitfield<std::uint8_t, 5, 3> reserved0;
    bitfield<std::uint8_t, 4, 1> assignable_per_note_controller;
    bitfield<std::uint8_t, 3, 1> registered_per_note_controller;
    bitfield<std::uint8_t, 2, 1> per_note_pitchbend;
    bitfield<std::uint8_t, 1, 1> poly_pressure;
    bitfield<std::uint8_t, 0, 1> notes;
  };

  union system_message system_message;
  std::byte reserved;
  union channel_controller channel_controller;
  union note_data_messages note_data_messages;
};

static_assert(offsetof(midi_message_report_reply_v2, system_message) == 0);
static_assert(offsetof(midi_message_report_reply_v2, reserved) == 1);
static_assert(offsetof(midi_message_report_reply_v2, channel_controller) == 2);
static_assert(offsetof(midi_message_report_reply_v2, note_data_messages) == 3);
static_assert(sizeof(midi_message_report_reply_v2) == 4);
static_assert(alignof(midi_message_report_reply_v2) == 1);
static_assert(std::is_trivially_copyable_v<midi_message_report_reply_v2>);

}  // end namespace packed

struct midi_message_report_reply {
  constexpr midi_message_report_reply() = default;
  constexpr midi_message_report_reply(midi_message_report_reply const &) = default;
  constexpr midi_message_report_reply(midi_message_report_reply &&) noexcept = default;
  constexpr explicit midi_message_report_reply(packed::midi_message_report_reply_v2 const &v2);
  ~midi_message_report_reply() noexcept = default;

  midi_message_report_reply &operator=(midi_message_report_reply const &) = default;
  midi_message_report_reply &operator=(midi_message_report_reply &&) noexcept = default;

  bool operator==(midi_message_report_reply const &) const = default;

  constexpr explicit operator packed::midi_message_report_reply_v2() const;

  // system messages
  unsigned mtc_quarter_frame : 1 = 0;
  unsigned song_position : 1 = 0;
  unsigned song_select : 1 = 0;
  // channel controller messages
  unsigned pitchbend : 1 = 0;
  unsigned control_change : 1 = 0;
  unsigned rpn_registered_controller : 1 = 0;
  unsigned nrpn_assignable_controller : 1 = 0;
  unsigned program_change : 1 = 0;
  unsigned channel_pressure : 1 = 0;
  // note data messages
  unsigned notes : 1 = 0;
  unsigned poly_pressure : 1 = 0;
  unsigned per_note_pitchbend : 1 = 0;
  unsigned registered_per_note_controller : 1 = 0;
  unsigned assignable_per_note_controller : 1 = 0;
};

constexpr midi_message_report_reply::midi_message_report_reply(packed::midi_message_report_reply_v2 const &v2)
    : mtc_quarter_frame{v2.system_message.mtc_quarter_frame},
      song_position{v2.system_message.song_position},
      song_select{v2.system_message.song_select},

      pitchbend{v2.channel_controller.pitchbend},
      control_change{v2.channel_controller.control_change},
      rpn_registered_controller{v2.channel_controller.rpn_registered_controller},
      nrpn_assignable_controller{v2.channel_controller.nrpn_assignable_controller},
      program_change{v2.channel_controller.program_change},
      channel_pressure{v2.channel_controller.channel_pressure},

      notes{v2.note_data_messages.notes},
      poly_pressure{v2.note_data_messages.poly_pressure},
      per_note_pitchbend{v2.note_data_messages.per_note_pitchbend},
      registered_per_note_controller{v2.note_data_messages.registered_per_note_controller},
      assignable_per_note_controller{v2.note_data_messages.assignable_per_note_controller} {
}

constexpr midi_message_report_reply::operator packed::midi_message_report_reply_v2() const {
  union packed::midi_message_report_reply_v2::system_message sm{};
  sm.mtc_quarter_frame = mtc_quarter_frame;
  sm.song_position = song_position;
  sm.song_select = song_select;

  union packed::midi_message_report_reply_v2::channel_controller cc{};
  cc.pitchbend = pitchbend;
  cc.control_change = control_change;
  cc.rpn_registered_controller = rpn_registered_controller;
  cc.nrpn_assignable_controller = nrpn_assignable_controller;
  cc.program_change = program_change;
  cc.channel_pressure = channel_pressure;

  union packed::midi_message_report_reply_v2::note_data_messages ndm{};
  ndm.notes = notes;
  ndm.poly_pressure = poly_pressure;
  ndm.per_note_pitchbend = per_note_pitchbend;
  ndm.registered_per_note_controller = registered_per_note_controller;
  ndm.assignable_per_note_controller = assignable_per_note_controller;

  return {.system_message = sm, .reserved = std::byte{0}, .channel_controller = cc, .note_data_messages = ndm};
}

//*                                      _                 _  *
//*  _ __  _ __    _ _ ___ _ __  ___ _ _| |_   ___ _ _  __| | *
//* | '  \| '  \  | '_/ -_) '_ \/ _ \ '_|  _| / -_) ' \/ _` | *
//* |_|_|_|_|_|_| |_| \___| .__/\___/_|  \__| \___|_||_\__,_| *
//*                       |_|                                 *
struct midi_message_report_end {};

}  // end namespace process_inquiry
}  // end namespace midi2::ci

#endif  // MIDI2_CI_TYPES_HPP
