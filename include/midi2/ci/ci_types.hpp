//===-- CI Types --------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ci_types.hpp
/// \brief Types and constants for MIDI CI

#ifndef MIDI2_CI_TYPES_HPP
#define MIDI2_CI_TYPES_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string>
#include <type_traits>

#include "midi2/adt/bitfield.hpp"
#include "midi2/adt/uinteger.hpp"
#include "midi2/bytestream/bytestream_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {
namespace ci::details {

/// Represents an unsigned integer with a specific number of bits
template <unsigned Bits, std::unsigned_integral Underlying = midi2::adt::uinteger_t<Bits>>
  requires(adt::max_value<Underlying, Bits>() <= std::numeric_limits<Underlying>::max())
class bn {
public:
  using underlying_type = Underlying;
  static constexpr auto bits = Bits;

  constexpr bn() noexcept = default;
  constexpr bn(bn const &rhs) noexcept = default;
  constexpr bn(bn &&rhs) noexcept = default;
  template <std::unsigned_integral UInt>
  constexpr explicit bn(UInt const v) noexcept : value_{static_cast<Underlying>(v)} {
    assert((v <= adt::max_value<Underlying, Bits>()) && "Value is too large");
  }

  constexpr explicit operator Underlying() const noexcept { return value_; }

  constexpr std::strong_ordering operator<=>(bn const &rhs) const noexcept = default;

  constexpr bn &operator=(bn const &rhs) noexcept = default;
  constexpr bn &operator=(bn &&rhs) noexcept = default;
  constexpr bn &operator=(Underlying const rhs) noexcept {
    assert((rhs <= adt::max_value<Underlying, Bits>()) && "Value is too large");
    value_ = rhs;
    return *this;
  }

  [[nodiscard]] constexpr Underlying get() const noexcept { return value_; }

private:
  Underlying value_ = 0;
};

}  // end namespace ci::details

namespace ci {
using b7 = ci::details::bn<7>;
using b14 = ci::details::bn<14>;
using b28 = ci::details::bn<28>;
}  // namespace ci

namespace literals {

[[nodiscard]] consteval ci::b7 operator""_b7(char const arg) noexcept {
  assert(arg < (1 << 7));
  return ci::b7{static_cast<std::uint8_t>(arg)};
}
[[nodiscard]] consteval ci::b7 operator""_b7(unsigned long long const arg) noexcept {
  assert(arg < (1 << 7));
  return ci::b7{arg};
}
[[nodiscard]] consteval ci::b14 operator""_b14(unsigned long long const arg) noexcept {
  assert(arg < (1 << 14));
  return ci::b14{arg};
}
[[nodiscard]] consteval ci::b28 operator""_b28(unsigned long long const arg) noexcept {
  assert(arg < (1 << 28));
  return ci::b28{arg};
}

}  // end namespace literals

}  // end namespace midi2

template <unsigned Bits, std::unsigned_integral Underlying, typename CharT>
struct std::formatter<midi2::ci::details::bn<Bits, Underlying>, CharT> : public std::formatter<Underlying, CharT> {
  auto format(midi2::ci::details::bn<Bits, Underlying> value, auto &format_ctx) const {
    return std::formatter<Underlying, CharT>::format(Underlying{value}, format_ctx);
  }
};

namespace midi2::ci {

enum class message : std::uint8_t {
  protocol_negotiation = 0x10,
  protocol_negotiation_reply = 0x11,
  protocol_set = 0x12,
  protocol_test = 0x13,
  protocol_test_responder = 0x14,
  protocol_confirm = 0x15,

  profile_inquiry = 0x20,
  profile_inquiry_reply = 0x21,
  profile_set_on = 0x22,
  profile_set_off = 0x23,
  profile_enabled = 0x24,
  profile_disabled = 0x25,
  profile_added = 0x26,
  profile_removed = 0x27,
  profile_details = 0x28,
  profile_details_reply = 0x29,
  profile_specific_data = 0x2F,

  pe_capability = 0x30,
  pe_capability_reply = 0x31,
  pe_get = 0x34,
  pe_get_reply = 0x35,
  pe_set = 0x36,
  pe_set_reply = 0x37,
  pe_sub = 0x38,
  pe_sub_reply = 0x39,
  pe_notify = 0x3F,

  pi_capability = 0x40,
  pi_capability_reply = 0x41,
  pi_mm_report = 0x42,
  pi_mm_report_reply = 0x43,
  pi_mm_report_end = 0x44,

  discovery = 0x70,
  discovery_reply = 0x71,
  endpoint = 0x72,
  endpoint_reply = 0x73,
  ack = 0x7D,
  invalidate_muid = 0x7E,
  nak = 0x7F,
};

enum class pe_status {
  ok = 200,
  accepted = 202,
  resource_unavailable = 341,
  bad_data = 342,
  too_many_reqs = 343,
  bad_req = 400,
  req_unauthorized = 403,
  resource_unsupported = 404,
  resource_not_allowed = 405,
  payload_too_large = 413,
  unsupported_media_type = 415,
  invalid_data_version = 445,
  internal_device_error = 500,
};

enum class pe_command : std::uint8_t {
  start = 1,
  end = 2,
  partial = 3,
  full = 4,
  notify = 5,
};

enum class pe_action : std::uint8_t {
  copy = 1,
  move = 2,
  del = 3,
  create_dir = 4,
};

enum class pe_encoding : std::uint8_t {
  ascii = 1,
  mcoded7 = 2,
  mcoded7zlib = 3,
};

template <std::size_t Size> using byte_array = std::array<std::byte, Size>;
template <std::size_t Size> using b7_array = std::array<b7, Size>;

using muid = b28;

/// MUID is a 28-bit random number generated by a MIDI-CI Device used to uniquely identify MIDI-CI messages.
/// This constant holds the largest available MUID that is neither reserved nor used as a broadcast MUID.
constexpr auto max_user_muid = muid{0x0FFFFF00U - 1U};
/// A special MUID value that is reserved for messages that are directed to all listening MIDI-CI devices.
constexpr auto broadcast_muid = muid{0x0FFFFFFFU};

namespace details {

constexpr auto mask7b = std::byte{(1 << 7) - 1};

[[nodiscard]] constexpr b28 from_le7(byte_array<4> const &v) noexcept {
  assert(((v[0] | v[1] | v[2] | v[3]) & std::byte{0x80}) == std::byte{0});
  using ut = b28::underlying_type;
  return b28{(static_cast<ut>(to_underlying(v[0] & mask7b)) << (7 * 0)) |
             (static_cast<ut>(to_underlying(v[1] & mask7b)) << (7 * 1)) |
             (static_cast<ut>(to_underlying(v[2] & mask7b)) << (7 * 2)) |
             (static_cast<ut>(to_underlying(v[3] & mask7b)) << (7 * 3))};
}
[[nodiscard]] constexpr b14 from_le7(byte_array<2> const &v) noexcept {
  assert(((v[0] | v[1]) & std::byte{0x80}) == std::byte{0});
  using ut = b14::underlying_type;
  return b14{static_cast<ut>((static_cast<ut>(to_underlying(v[0] & mask7b)) << (7 * 0)) |
                             (static_cast<ut>(to_underlying(v[1] & mask7b)) << (7 * 1)))};
}
[[nodiscard]] constexpr b7 from_le7(std::byte const v) noexcept {
  assert((v & std::byte{0x80}) == std::byte{0});
  return b7{to_underlying(v)};
}
[[nodiscard]] constexpr b7_array<5> from_le7(byte_array<5> const &v) noexcept {
  b7_array<5> result;
  std::ranges::transform(v, std::begin(result), [](std::byte const b) { return from_le7(b); });
  return result;
}

[[nodiscard]] constexpr byte_array<4> to_le7(b28 const v28) noexcept {
  auto const v = v28.get();
  assert(v < (std::uint32_t{1} << 28));
  return {static_cast<std::byte>(v >> (7 * 0)) & mask7b, static_cast<std::byte>(v >> (7 * 1)) & mask7b,
          static_cast<std::byte>(v >> (7 * 2)) & mask7b, static_cast<std::byte>(v >> (7 * 3)) & mask7b};
}
[[nodiscard]] constexpr byte_array<2> to_le7(b14 const v14) noexcept {
  auto const v = v14.get();
  assert(v < (std::uint16_t{1} << 14));
  return {static_cast<std::byte>(v >> (7 * 0)) & mask7b, static_cast<std::byte>(v >> (7 * 1)) & mask7b};
}
[[nodiscard]] constexpr std::byte to_le7(b7 const v) noexcept {
  assert(v.get() < (std::uint8_t{1} << 7));
  return static_cast<std::byte>(v.get());
}
[[nodiscard]] constexpr byte_array<5> to_le7(b7_array<5> const &v) noexcept {
  byte_array<5> result;
  std::ranges::transform(v, std::begin(result), [](b7 const b) { return to_le7(b); });
  return result;
}

template <std::size_t Size>
[[nodiscard]] constexpr b7_array<Size> from_byte_array(byte_array<Size> const &other) noexcept {
  b7_array<Size> result{};
  std::ranges::transform(other, std::begin(result), [](std::byte const v) { return from_le7(v); });
  return result;
}

template <std::size_t Size>
[[nodiscard]] constexpr byte_array<Size> to_byte_array(b7_array<Size> const &other) noexcept {
  byte_array<Size> result{};
  std::ranges::transform(other, std::begin(result), [](b7 const v) { return std::byte{v.get()}; });
  return result;
}

}  // end namespace details

namespace packed {

struct header {
  std::byte sysex;  ///< 0x7E
  /// Device ID: Source or Destination (depending on type of message):
  /// - 00–0F: To/from MIDI Channels 1-16
  /// - 10–7D: Reserved
  /// - 7E: To/from Group
  /// - 7F: To/from Function Block
  std::byte source;
  std::byte sub_id_1;  ///< 0x0D
  /// The MIDI-CI message. Will be one of the values from the midi2::ci::message enum.
  std::byte sub_id_2;
  /// MIDI-CI Message Version/Format
  std::byte version;
  /// Source MUID (LSB first)
  byte_array<4> source_muid;
  /// Destination MUID (LSB first)
  byte_array<4> destination_muid;
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

struct header {
  constexpr bool operator==(header const &) const noexcept = default;
  explicit constexpr operator packed::header() const noexcept;

  /// \brief Source or Destination (depending on type of message):
  /// - 00–0F: To/from MIDI Channels 1-16
  /// - 10–7D: Reserved
  /// - 7E: To/from Group
  /// - 7F: To/from Function Block
  b7 device_id;
  /// MIDI-CI Message Version/Format
  b7 version = b7{1U};
  muid remote_muid;
  muid local_muid;
};

constexpr header::operator packed::header() const noexcept {
  return packed::header{.sysex = bytestream::s7_universal_nrt,
                        .source = static_cast<std::byte>(device_id.get()),
                        .sub_id_1 = bytestream::s7_midi_ci,
                        .sub_id_2 = std::byte{0},  // message type
                        .version = static_cast<std::byte>(version.get()),
                        .source_muid = ci::details::to_le7(remote_muid),
                        .destination_muid = ci::details::to_le7(local_muid)};
}

//*     _ _                              *
//*  __| (_)___ __ _____ _____ _ _ _  _  *
//* / _` | (_-</ _/ _ \ V / -_) '_| || | *
//* \__,_|_/__/\__\___/\_/\___|_|  \_, | *
//*                                |__/  *
namespace packed {

struct discovery_v1 {
  byte_array<3> manufacturer;
  byte_array<2> family;
  byte_array<2> model;
  byte_array<4> version;
  std::byte capability;
  byte_array<4> max_sysex_size;
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

/// \brief The fields of a CI discovery message.
/// An Initiator shall establish connections to MIDI-CI Responders by sending a Discovery message.
struct discovery {
  [[nodiscard]] static constexpr discovery make(packed::discovery_v1 const &v1, b7 output_path_id = b7{}) noexcept;
  [[nodiscard]] static constexpr discovery make(packed::discovery_v2 const &v2) noexcept;
  explicit constexpr operator packed::discovery_v1() const noexcept;
  explicit constexpr operator packed::discovery_v2() const noexcept;
  constexpr bool operator==(discovery const &) const noexcept = default;

  /// Device Manufacturer (System Exclusive ID Number)
  b7_array<3> manufacturer{};
  /// Device Family
  b14 family;
  /// Device Family Model Number
  b14 model;
  /// Software Revision Level (Format is Device specific)
  b7_array<4> version{};
  /// Capability Inquiry Category Supported (bitmap)
  b7 capability;
  /// Receivable Maximum SysEx Message Size
  b28 max_sysex_size;
  /// Initiator's Output Path ID
  b7 output_path_id;
};

constexpr discovery discovery::make(packed::discovery_v1 const &v1, b7 output_path_id) noexcept {
  return discovery{.manufacturer = details::from_byte_array(v1.manufacturer),
                   .family = details::from_le7(v1.family),
                   .model = details::from_le7(v1.model),
                   .version = details::from_byte_array(v1.version),
                   .capability = details::from_le7(v1.capability),
                   .max_sysex_size = details::from_le7(v1.max_sysex_size),
                   .output_path_id = output_path_id};
}
constexpr discovery discovery::make(packed::discovery_v2 const &v2) noexcept {
  return make(v2.v1, details::from_le7(v2.output_path_id));
}

constexpr discovery::operator packed::discovery_v1() const noexcept {
  return {.manufacturer = details::to_byte_array(manufacturer),
          .family = details::to_le7(family),
          .model = details::to_le7(model),
          .version = details::to_byte_array(version),
          .capability = static_cast<std::byte>(capability.get()),
          .max_sysex_size = details::to_le7(max_sysex_size)};
}
constexpr discovery::operator packed::discovery_v2() const noexcept {
  return {.v1 = static_cast<packed::discovery_v1>(*this), .output_path_id = details::to_le7(output_path_id)};
}

//*     _ _                                            _       *
//*  __| (_)___ __ _____ _____ _ _ _  _   _ _ ___ _ __| |_  _  *
//* / _` | (_-</ _/ _ \ V / -_) '_| || | | '_/ -_) '_ \ | || | *
//* \__,_|_/__/\__\___/\_/\___|_|  \_, | |_| \___| .__/_|\_, | *
//*                                |__/          |_|     |__/  *
namespace packed {

struct discovery_reply_v1 {
  byte_array<3> manufacturer;
  byte_array<2> family;
  byte_array<2> model;
  byte_array<4> version;
  std::byte capability;
  byte_array<4> max_sysex_size;
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

/// \brief Reply to Discovery Message.
///
/// When a MIDI-CI Device receives a Discovery message it shall become a Responder and send this
/// Reply to Discovery message. This message declares the MUID of the Responder.
struct discovery_reply {
  [[nodiscard]] static constexpr discovery_reply make(packed::discovery_reply_v1 const &v1, b7 output_path_id = b7{},
                                                      b7 function_block = b7{}) noexcept;
  [[nodiscard]] static constexpr discovery_reply make(packed::discovery_reply_v2 const &v2) noexcept;
  explicit constexpr operator packed::discovery_reply_v1() const noexcept;
  explicit constexpr operator packed::discovery_reply_v2() const noexcept;
  constexpr bool operator==(discovery_reply const &) const noexcept = default;

  /// Device Manufacturer (System Exclusive ID Number)
  b7_array<3> manufacturer{};
  /// Device Family
  b14 family;
  /// Device Family Model Number
  b14 model;
  /// Software Revision Level (Format is Device specific)
  b7_array<4> version{};
  /// Capability Inquiry Category Supported (bitmap)
  b7 capability;
  /// Receivable Maximum SysEx Message Size
  b28 max_sysex_size;
  /// Initiator's Output Path Instance ID (from the Discovery message received)
  b7 output_path_id;
  /// Function Block
  b7 function_block;
};
constexpr discovery_reply discovery_reply::make(packed::discovery_reply_v1 const &v1, b7 output_path_id,
                                                b7 function_block) noexcept {
  return {.manufacturer = details::from_byte_array(v1.manufacturer),
          .family = details::from_le7(v1.family),
          .model = details::from_le7(v1.model),
          .version = details::from_byte_array(v1.version),
          .capability = details::from_le7(v1.capability),
          .max_sysex_size = details::from_le7(v1.max_sysex_size),
          .output_path_id = output_path_id,
          .function_block = function_block};
}
constexpr discovery_reply discovery_reply::make(packed::discovery_reply_v2 const &v2) noexcept {
  return make(v2.v1, details::from_le7(v2.output_path_id), details::from_le7(v2.function_block));
}

constexpr discovery_reply::operator packed::discovery_reply_v1() const noexcept {
  return packed::discovery_reply_v1{.manufacturer = details::to_byte_array(manufacturer),
                                    .family = details::to_le7(family),
                                    .model = details::to_le7(model),
                                    .version = details::to_byte_array(version),
                                    .capability = details::to_le7(capability),
                                    .max_sysex_size = details::to_le7(max_sysex_size)};
}
constexpr discovery_reply::operator packed::discovery_reply_v2() const noexcept {
  return {.v1 = static_cast<packed::discovery_reply_v1>(*this),
          .output_path_id = details::to_le7(output_path_id),
          .function_block = details::to_le7(function_block)};
}

//*              _           _     _    *
//*  ___ _ _  __| |_ __  ___(_)_ _| |_  *
//* / -_) ' \/ _` | '_ \/ _ \ | ' \  _| *
//* \___|_||_\__,_| .__/\___/_|_||_\__| *
//*               |_|                   *
namespace packed {

struct endpoint_v1 {
  std::byte status;
};
static_assert(offsetof(endpoint_v1, status) == 0);
static_assert(sizeof(endpoint_v1) == 1);
static_assert(alignof(endpoint_v1) == 1);
static_assert(std::is_trivially_copyable_v<endpoint_v1>);

}  // end namespace packed

/// \brief Inquiry: Endpoint Message.
///
/// An Initiator may send the Inquiry: Endpoint Message to a Function Block in a Responder to get
/// information about the UMP Endpoint which has the Function Block. A Status field selects the
/// target data.
struct endpoint {
  [[nodiscard]] static constexpr endpoint make(packed::endpoint_v1 const &) noexcept;
  explicit constexpr operator packed::endpoint_v1() const noexcept;
  constexpr bool operator==(endpoint const &) const noexcept = default;

  /// The Status field defines which information to retrieve from the Responder.
  b7 status;
};

constexpr endpoint endpoint::make(packed::endpoint_v1 const &other) noexcept {
  return {.status = b7{to_underlying(other.status)}};
}

constexpr endpoint::operator packed::endpoint_v1() const noexcept {
  return {.status = static_cast<std::byte>(status.get())};
}

//*              _           _     _                  _       *
//*  ___ _ _  __| |_ __  ___(_)_ _| |_   _ _ ___ _ __| |_  _  *
//* / -_) ' \/ _` | '_ \/ _ \ | ' \  _| | '_/ -_) '_ \ | || | *
//* \___|_||_\__,_| .__/\___/_|_||_\__| |_| \___| .__/_|\_, | *
//*               |_|                           |_|     |__/  *
namespace packed {

struct endpoint_reply_v1 {
  std::byte status;
  byte_array<2> data_length;
  // Don't be tempted to change to std::array<>. MSVC's iterator checks will trip.
  std::byte data[1];  ///< an array of size given by data_length
};
static_assert(offsetof(endpoint_reply_v1, status) == 0);
static_assert(offsetof(endpoint_reply_v1, data_length) == 1);
static_assert(offsetof(endpoint_reply_v1, data) == 3);
static_assert(sizeof(endpoint_reply_v1) == 4);
static_assert(alignof(endpoint_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<endpoint_reply_v1>);

}  // end namespace packed

struct endpoint_reply {
  [[nodiscard]] static constexpr endpoint_reply make(packed::endpoint_reply_v1 const &) noexcept;
  explicit constexpr operator packed::endpoint_reply_v1() const noexcept;
  constexpr bool operator==(endpoint_reply const &other) const noexcept {
    return status == other.status && std::ranges::equal(information, other.information);
  }

  b7 status;
  std::span<b7 const> information;
};

constexpr endpoint_reply endpoint_reply::make(packed::endpoint_reply_v1 const &other) noexcept {
  return {.status = details::from_le7(other.status),
          .information{std::bit_cast<b7 const *>(&other.data[0]), details::from_le7(other.data_length).get()}};
}

constexpr endpoint_reply::operator packed::endpoint_reply_v1() const noexcept {
  return {.status = details::to_le7(status),
          .data_length = details::to_le7(static_cast<b14>(information.size())),
          .data = {std::byte{0}}};
}

//*  _              _ _    _      _         __  __ _   _ ___ ___   *
//* (_)_ ___ ____ _| (_)__| |__ _| |_ ___  |  \/  | | | |_ _|   \  *
//* | | ' \ V / _` | | / _` / _` |  _/ -_) | |\/| | |_| || || |) | *
//* |_|_||_\_/\__,_|_|_\__,_\__,_|\__\___| |_|  |_|\___/|___|___/  *
//*                                                                *
namespace packed {

struct invalidate_muid_v1 {
  byte_array<4> target_muid;
};
static_assert(offsetof(invalidate_muid_v1, target_muid) == 0);
static_assert(sizeof(invalidate_muid_v1) == 4);
static_assert(alignof(invalidate_muid_v1) == 1);
static_assert(std::is_trivially_copyable_v<invalidate_muid_v1>);

}  // end namespace packed

struct invalidate_muid {
  [[nodiscard]] static constexpr invalidate_muid make(packed::invalidate_muid_v1 const &) noexcept;
  explicit constexpr operator packed::invalidate_muid_v1() const noexcept;
  constexpr bool operator==(invalidate_muid const &) const noexcept = default;

  muid target_muid;
};

constexpr invalidate_muid invalidate_muid::make(packed::invalidate_muid_v1 const &other) noexcept {
  return {.target_muid = details::from_le7(other.target_muid)};
}

constexpr invalidate_muid::operator packed::invalidate_muid_v1() const noexcept {
  return {.target_muid = details::to_le7(target_muid)};
}

//*          _    *
//*  __ _ __| |__ *
//* / _` / _| / / *
//* \__,_\__|_\_\ *
//*               *
namespace packed {

/// \brief Version 1 of the CI Ack message
struct ack_v1 {
  std::byte original_id;         ///< Original Transaction Sub-ID#2 Classification
  std::byte status_code;         ///< ACK Status Code
  std::byte status_data;         ///< ACK Status Data
  byte_array<5> details;         ///< ACK details for each SubID Classification
  byte_array<2> message_length;  ///< Message Length (LSB first)
  b7 message[1];                 ///< Message text (array of size given by message_length)
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

/// \brief MIDI-CI ACK Message.
///
/// The MIDI-CI ACK Message is a message for dealing with positive acknowledgement of an action, or
/// to provide a notice of ongoing activity, such as timeout wait messages.
struct ack {
  [[nodiscard]] static constexpr ack make(packed::ack_v1 const &) noexcept;
  explicit constexpr operator packed::ack_v1() const noexcept;
  constexpr bool operator==(ack const &other) const noexcept;

  /// Original Transaction Sub-ID#2 Classification
  b7 original_id;
  /// ACK Status Code
  b7 status_code;
  /// ACK Status Data
  b7 status_data;
  /// ACK details for each SubID Classification
  b7_array<5> details{};
  /// Message text
  std::span<b7 const> message;
};

constexpr ack ack::make(packed::ack_v1 const &v1) noexcept {
  return {.original_id = details::from_le7(v1.original_id),
          .status_code = details::from_le7(v1.status_code),
          .status_data = details::from_le7(v1.status_data),
          .details = details::from_le7(v1.details),
          .message = std::span<b7 const>{std::begin(v1.message), details::from_le7(v1.message_length).get()}};
}

constexpr ack::operator packed::ack_v1() const noexcept {
  return {.original_id = details::to_le7(original_id),
          .status_code = details::to_le7(status_code),
          .status_data = details::to_le7(status_data),
          .details = details::to_le7(details),
          .message_length = details::to_le7(static_cast<b14>(message.size())),
          .message{b7{}}};
}

constexpr bool ack::operator==(ack const &other) const noexcept {
  return original_id == other.original_id && status_code == other.status_code && status_data == other.status_data &&
         details == other.details && std::ranges::equal(message, other.message);
}

//*            _    *
//*  _ _  __ _| |__ *
//* | ' \/ _` | / / *
//* |_||_\__,_|_\_\ *
//*                 *
namespace packed {

/// \brief Version 1 of the CI Nak message
struct nak_v1 {};
/// \brief Version 2 of the CI Nak message
struct nak_v2 {
  std::byte original_id;         ///< Original transaction sub-ID#2 classification
  std::byte status_code;         ///< ACK Status Code
  std::byte status_data;         ///< ACK Status Data
  byte_array<5> details;         ///< ACK details for each SubID Classification
  byte_array<2> message_length;  ///< Message Length (LSB first)
  b7 message[1];                 ///< Message text (length given by message_length)
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
  [[nodiscard]] static constexpr nak make(packed::nak_v1 const &) noexcept;
  [[nodiscard]] static constexpr nak make(packed::nak_v2 const &) noexcept;
  explicit constexpr operator packed::nak_v1() const noexcept;
  explicit constexpr operator packed::nak_v2() const noexcept;
  constexpr bool operator==(nak const &) const noexcept;

  b7 original_id;         ///< Original transaction sub-ID#2 classification
  b7 status_code;         ///< NAK Status Code
  b7 status_data;         ///< NAK Status Data
  b7_array<5> details{};  ///< NAK details for each SubID Classification
  std::span<b7 const> message;
};

constexpr nak nak::make(packed::nak_v1 const &) noexcept {
  return {};
}

constexpr nak nak::make(packed::nak_v2 const &v2) noexcept {
  return nak{.original_id = b7{to_underlying(v2.original_id)},
             .status_code = b7{to_underlying(v2.status_code)},
             .status_data = b7{to_underlying(v2.status_data)},
             .details = details::from_le7(v2.details),
             .message = std::span<b7 const>{std::begin(v2.message), details::from_le7(v2.message_length).get()}};
}

constexpr nak::operator packed::nak_v1() const noexcept {
  return {};
}
constexpr nak::operator packed::nak_v2() const noexcept {
  return {.original_id = details::to_le7(original_id),
          .status_code = details::to_le7(status_code),
          .status_data = details::to_le7(status_data),
          .details = details::to_le7(details),
          .message_length = details::to_le7(static_cast<b14>(message.size())),
          .message = {b7{0U}}};
}

constexpr bool nak::operator==(nak const &other) const noexcept {
  return original_id == other.original_id && status_code == other.status_code && status_data == other.status_data &&
         details == other.details && std::ranges::equal(message, other.message);
}

/// \brief Types for MIDI CI Profile Configuration Messages.
namespace profile_configuration {

using profile = b7_array<5>;

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

/// \brief Part 1 of version 1 of the CI Inquiry Reply message
struct inquiry_reply_v1_pt1 {
  byte_array<2> num_enabled;  ///< Number of currently enabled profiles
  profile ids[1];             ///< Profile ID of currently enabled profiles (array length given by num_enabled)
};

static_assert(offsetof(inquiry_reply_v1_pt1, num_enabled) == 0);
static_assert(offsetof(inquiry_reply_v1_pt1, ids) == 2);
static_assert(sizeof(inquiry_reply_v1_pt1) == 7);
static_assert(alignof(inquiry_reply_v1_pt1) == 1);
static_assert(std::is_trivially_copyable_v<inquiry_reply_v1_pt1>);

/// \brief Part 2 of version 1 of the CI Inquiry Reply message
struct inquiry_reply_v1_pt2 {
  byte_array<2> num_disabled;  ///< Number of currently disabled profiles
  profile ids[1];              ///< Profile ID of currently enabled profiles (array length given by num_disabled)
};

static_assert(offsetof(inquiry_reply_v1_pt2, num_disabled) == 0);
static_assert(offsetof(inquiry_reply_v1_pt2, ids) == 2);
static_assert(sizeof(inquiry_reply_v1_pt2) == 7);
static_assert(alignof(inquiry_reply_v1_pt2) == 1);
static_assert(std::is_trivially_copyable_v<inquiry_reply_v1_pt2>);

}  // end namespace packed

struct inquiry_reply {
  [[nodiscard]] static constexpr inquiry_reply make(packed::inquiry_reply_v1_pt1 const &,
                                                    packed::inquiry_reply_v1_pt2 const &) noexcept;
  explicit constexpr operator packed::inquiry_reply_v1_pt1() const noexcept;
  explicit constexpr operator packed::inquiry_reply_v1_pt2() const noexcept;
  constexpr bool operator==(inquiry_reply const &other) const noexcept {
    return std::ranges::equal(enabled, other.enabled) && std::ranges::equal(disabled, other.disabled);
  }

  std::span<profile const> enabled;
  std::span<profile const> disabled;
};

constexpr inquiry_reply inquiry_reply::make(packed::inquiry_reply_v1_pt1 const &v1_pt1,
                                            packed::inquiry_reply_v1_pt2 const &v1_pt2) noexcept {
  return {
      .enabled = std::span<profile const>{std::begin(v1_pt1.ids), details::from_le7(v1_pt1.num_enabled).get()},
      .disabled = std::span<profile const>{std::begin(v1_pt2.ids), details::from_le7(v1_pt2.num_disabled).get()},
  };
}

constexpr inquiry_reply::operator packed::inquiry_reply_v1_pt1() const noexcept {
  return {.num_enabled = details::to_le7(static_cast<b14>(enabled.size())), .ids = {profile{}}};
}
constexpr inquiry_reply::operator packed::inquiry_reply_v1_pt2() const noexcept {
  return {.num_disabled = details::to_le7(static_cast<b14>(disabled.size())), .ids = {profile{}}};
}

//*                __ _ _               _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   __ _ __| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / _` / _` / _` / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \__,_\__,_\__,_\___\__,_| *
//* |_|                                                  *
namespace packed {

/// \brief Version 1 of the CI Profile Added message
struct added_v1 {
  profile pid{};  ///< Profile ID of profile being added
};
static_assert(offsetof(added_v1, pid) == 0);
static_assert(sizeof(added_v1) == 5);
static_assert(alignof(added_v1) == 1);
static_assert(std::is_trivially_copyable_v<added_v1>);

}  // end namespace packed

struct added {
  [[nodiscard]] static constexpr added make(packed::added_v1 const &other) noexcept { return {.pid = other.pid}; }
  explicit constexpr operator packed::added_v1() const noexcept { return {.pid = pid}; }
  constexpr bool operator==(added const &) const noexcept = default;

  profile pid{};  ///< Profile ID of profile being added
};

//*                __ _ _                                    _  *
//*  _ __ _ _ ___ / _(_) |___   _ _ ___ _ __  _____ _____ __| | *
//* | '_ \ '_/ _ \  _| | / -_) | '_/ -_) '  \/ _ \ V / -_) _` | *
//* | .__/_| \___/_| |_|_\___| |_| \___|_|_|_\___/\_/\___\__,_| *
//* |_|                                                         *
namespace packed {

/// \brief Version 1 of the CI Profile Removed message
struct removed_v1 {
  profile pid{};  ///< Profile ID of profile being removed
};
static_assert(offsetof(removed_v1, pid) == 0);
static_assert(sizeof(removed_v1) == 5);
static_assert(alignof(removed_v1) == 1);
static_assert(std::is_trivially_copyable_v<removed_v1>);

}  // end namespace packed

struct removed {
  [[nodiscard]] static constexpr removed make(packed::removed_v1 const &other) noexcept { return {.pid = other.pid}; }
  explicit constexpr operator packed::removed_v1() const noexcept { return {.pid = pid}; }
  constexpr bool operator==(removed const &) const noexcept = default;

  profile pid{};  ///< Profile ID of profile being removed
};

//*                __      _     _        _ _      _                _           *
//*  _ __ _ _ ___ / _|  __| |___| |_ __ _(_) |___ (_)_ _  __ _ _  _(_)_ _ _  _  *
//* | '_ \ '_/ _ \  _| / _` / -_)  _/ _` | | (_-< | | ' \/ _` | || | | '_| || | *
//* | .__/_| \___/_|   \__,_\___|\__\__,_|_|_/__/ |_|_||_\__, |\_,_|_|_|  \_, | *
//* |_|                                                     |_|           |__/  *
namespace packed {

/// \brief Version 1 of the CI Profile Details Inquiry message
struct details_v1 {
  profile pid{};  ///< Profile ID of profile
  std::byte target{};
};
static_assert(offsetof(details_v1, pid) == 0);
static_assert(offsetof(details_v1, target) == 5);
static_assert(sizeof(details_v1) == 6);
static_assert(alignof(details_v1) == 1);
static_assert(std::is_trivially_copyable_v<details_v1>);

}  // end namespace packed

struct details {
  [[nodiscard]] static constexpr details make(packed::details_v1 const &other) noexcept {
    return {.pid = other.pid, .target = b7{to_underlying(other.target)}};
  }
  explicit constexpr operator packed::details_v1() const noexcept {
    return {.pid = pid, .target = ci::details::to_le7(target)};
  }
  constexpr bool operator==(details const &) const noexcept = default;

  profile pid{};
  b7 target{};
};

namespace packed {

/// \brief Version 1 of the CI Profile Details Reply message
struct details_reply_v1 {
  profile pid{};                ///< Profile ID of profile
  std::byte target{};           ///< Inquiry target
  byte_array<2> data_length{};  ///< Inquiry target data length (LSB first)
  b7 data[1]{};                 ///< Array length given by data_length
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
  [[nodiscard]] static constexpr details_reply make(packed::details_reply_v1 const &) noexcept;
  explicit constexpr operator packed::details_reply_v1() const noexcept;
  constexpr bool operator==(details_reply const &other) const noexcept {
    return pid == other.pid && target == other.target && std::ranges::equal(data, other.data);
  }

  profile pid{};  ///< Profile ID of profile
  b7 target{};    ///< Inquiry target
  std::span<b7 const> data;
};

constexpr details_reply details_reply::make(packed::details_reply_v1 const &other) noexcept {
  return {.pid = other.pid,
          .target = b7{to_underlying(other.target)},
          .data = std::span<b7 const>{std::begin(other.data), ci::details::from_le7(other.data_length).get()}};
}

constexpr details_reply::operator packed::details_reply_v1() const noexcept {
  return {
      .pid = pid,
      .target = static_cast<std::byte>(target.get()),
      .data_length =
          ci::details::to_le7(static_cast<b14>(data.size_bytes())),  ///< Inquiry target data length (LSB first)
      .data = {b7{0U}},
  };
}

//*                __ _ _                 *
//*  _ __ _ _ ___ / _(_) |___   ___ _ _   *
//* | '_ \ '_/ _ \  _| | / -_) / _ \ ' \  *
//* | .__/_| \___/_| |_|_\___| \___/_||_| *
//* |_|                                   *
namespace packed {

/// \brief Version 1 of the CI Profile On message
struct on_v1 {
  profile pid{};  ///< Profile ID of profile to be set to on (to be enabled)
};
static_assert(offsetof(on_v1, pid) == 0);
static_assert(sizeof(on_v1) == 5);
static_assert(alignof(on_v1) == 1);
static_assert(std::is_trivially_copyable_v<on_v1>);

/// \brief Version 2 of the CI Profile On message
struct on_v2 {
  on_v1 v1;
  /// Number of channels requested (LSB First) to assign to this profile when it is enabled
  byte_array<2> num_channels;
};
static_assert(offsetof(on_v2, v1) == 0);
static_assert(offsetof(on_v2, num_channels) == 5);
static_assert(sizeof(on_v2) == 7);
static_assert(alignof(on_v2) == 1);
static_assert(std::is_trivially_copyable_v<on_v2>);

static_assert(sizeof(on_v1) <= sizeof(on_v2));

}  // end namespace packed

struct on {
  [[nodiscard]] static constexpr on make(packed::on_v1 const &, b14 num_channels = b14{}) noexcept;
  [[nodiscard]] static constexpr on make(packed::on_v2 const &) noexcept;
  explicit constexpr operator packed::on_v1() const noexcept;
  explicit constexpr operator packed::on_v2() const noexcept;
  constexpr bool operator==(on const &) const noexcept = default;

  profile pid{};
  b14 num_channels;
};

constexpr on on::make(packed::on_v1 const &other, b14 num_channels) noexcept {
  return {.pid = other.pid, .num_channels = num_channels};
}
constexpr on on::make(packed::on_v2 const &other) noexcept {
  return make(other.v1, ci::details::from_le7(other.num_channels));
}
constexpr on::operator packed::on_v1() const noexcept {
  return {.pid = pid};
}
constexpr on::operator packed::on_v2() const noexcept {
  return packed::on_v2{.v1 = static_cast<packed::on_v1>(*this), .num_channels = ci::details::to_le7(num_channels)};
}

//*                __ _ _            __  __  *
//*  _ __ _ _ ___ / _(_) |___   ___ / _|/ _| *
//* | '_ \ '_/ _ \  _| | / -_) / _ \  _|  _| *
//* | .__/_| \___/_| |_|_\___| \___/_| |_|   *
//* |_|                                      *
namespace packed {

/// \brief Version 1 of the CI Profile Off message
struct off_v1 {
  profile pid{};  ///< Profile ID of Profile to be Set to Off (to be Dsiable)
};
static_assert(offsetof(off_v1, pid) == 0);
static_assert(sizeof(off_v1) == 5);
static_assert(alignof(off_v1) == 1);
static_assert(std::is_trivially_copyable_v<off_v1>);

/// \brief Version 2 of the CI Profile Off message
struct off_v2 {
  off_v1 v1;
  byte_array<2> reserved{};
};
static_assert(offsetof(off_v2, v1) == 0);
static_assert(offsetof(off_v2, reserved) == 5);
static_assert(sizeof(off_v2) == 7);
static_assert(alignof(off_v2) == 1);
static_assert(std::is_trivially_copyable_v<off_v2>);

static_assert(sizeof(off_v1) <= sizeof(off_v2));

}  // end namespace packed

struct off {
  [[nodiscard]] static constexpr off make(packed::off_v1 const &) noexcept;
  [[nodiscard]] static constexpr off make(packed::off_v2 const &) noexcept;
  explicit constexpr operator packed::off_v1() const noexcept;
  explicit constexpr operator packed::off_v2() const noexcept;
  constexpr bool operator==(off const &) const noexcept = default;

  profile pid{};
  // There's a 14 bit field in the specification that's "reserved"
};

constexpr off off::make(packed::off_v1 const &other) noexcept {
  return {.pid = other.pid};
}
constexpr off off::make(packed::off_v2 const &other) noexcept {
  return make(static_cast<packed::off_v1>(other.v1));
}
constexpr off::operator packed::off_v1() const noexcept {
  return {.pid = pid};
}
constexpr off::operator packed::off_v2() const noexcept {
  return {.v1 = static_cast<packed::off_v1>(*this), .reserved = byte_array<2>{}};
}

//*                __ _ _                     _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   ___ _ _  __ _| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / -_) ' \/ _` | '_ \ / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \___|_||_\__,_|_.__/_\___\__,_| *
//* |_|                                                        *
namespace packed {

/// \brief Version 1 of the CI Profile Enabled message
struct enabled_v1 {
  profile pid{};  ///< Profile ID of Profile that is now enabled
};
static_assert(offsetof(enabled_v1, pid) == 0);
static_assert(sizeof(enabled_v1) == 5);
static_assert(alignof(enabled_v1) == 1);
static_assert(std::is_trivially_copyable_v<enabled_v1>);

/// \brief Version 2 of the CI Profile Enabled message
struct enabled_v2 {
  enabled_v1 v1;
  byte_array<2> num_channels{};
};
static_assert(offsetof(enabled_v2, v1) == 0);
static_assert(offsetof(enabled_v2, num_channels) == 5);
static_assert(sizeof(enabled_v2) == 7);
static_assert(alignof(enabled_v2) == 1);
static_assert(std::is_trivially_copyable_v<enabled_v2>);

static_assert(sizeof(enabled_v1) <= sizeof(enabled_v2));

}  // end namespace packed

struct enabled {
  [[nodiscard]] static constexpr enabled make(packed::enabled_v1 const &, b14 num_channels = b14{}) noexcept;
  [[nodiscard]] static constexpr enabled make(packed::enabled_v2 const &) noexcept;
  explicit constexpr operator packed::enabled_v1() const noexcept;
  explicit constexpr operator packed::enabled_v2() const noexcept;
  constexpr bool operator==(enabled const &) const noexcept = default;

  profile pid{};
  b14 num_channels;
};

constexpr enabled enabled::make(packed::enabled_v1 const &other, b14 num_channels) noexcept {
  return {.pid = other.pid, .num_channels = num_channels};
}
constexpr enabled enabled::make(packed::enabled_v2 const &other) noexcept {
  return make(other.v1, ci::details::from_le7(other.num_channels));
}
constexpr enabled::operator packed::enabled_v1() const noexcept {
  return {.pid = pid};
}
constexpr enabled::operator packed::enabled_v2() const noexcept {
  return {
      .v1 = static_cast<packed::enabled_v1>(*this),
      .num_channels = ci::details::to_le7(num_channels),
  };
}

//*                __ _ _          _ _          _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   __| (_)___ __ _| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / _` | (_-</ _` | '_ \ / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \__,_|_/__/\__,_|_.__/_\___\__,_| *
//* |_|                                                          *
namespace packed {

/// \brief Version 1 of the CI Profile Disabled message
struct disabled_v1 {
  profile pid;  ///< Profile ID of Profile that is now disabled
};
static_assert(offsetof(disabled_v1, pid) == 0);
static_assert(sizeof(disabled_v1) == 5);
static_assert(alignof(disabled_v1) == 1);
static_assert(std::is_trivially_copyable_v<disabled_v1>);

/// \brief Version 2 of the CI Profile Disabled message
struct disabled_v2 {
  disabled_v1 v1;
  byte_array<2> num_channels;
};
static_assert(offsetof(disabled_v2, v1) == 0);
static_assert(offsetof(disabled_v2, num_channels) == 5);
static_assert(sizeof(disabled_v2) == 7);
static_assert(alignof(disabled_v2) == 1);
static_assert(std::is_trivially_copyable_v<disabled_v2>);

static_assert(sizeof(disabled_v1) <= sizeof(disabled_v2));

}  // end namespace packed

struct disabled {
  [[nodiscard]] static constexpr disabled make(packed::disabled_v1 const &) noexcept;
  [[nodiscard]] static constexpr disabled make(packed::disabled_v2 const &) noexcept;
  explicit constexpr operator packed::disabled_v1() const noexcept;
  explicit constexpr operator packed::disabled_v2() const noexcept;

  constexpr bool operator==(disabled const &) const noexcept = default;

  profile pid{};
  b14 num_channels;
};

constexpr disabled disabled::make(packed::disabled_v1 const &other) noexcept {
  return {.pid = other.pid, .num_channels = b14{}};
}
constexpr disabled disabled::make(packed::disabled_v2 const &other) noexcept {
  return {.pid = other.v1.pid, .num_channels = ci::details::from_le7(other.num_channels)};
}
constexpr disabled::operator packed::disabled_v1() const noexcept {
  return {.pid = pid};
}
constexpr disabled::operator packed::disabled_v2() const noexcept {
  return {.v1 = static_cast<packed::disabled_v1>(*this), .num_channels = ci::details::to_le7(num_channels)};
}

//*                __ _ _                       _  __ _         _      _         *
//*  _ __ _ _ ___ / _(_) |___   ____ __  ___ __(_)/ _(_)__   __| |__ _| |_ __ _  *
//* | '_ \ '_/ _ \  _| | / -_) (_-< '_ \/ -_) _| |  _| / _| / _` / _` |  _/ _` | *
//* | .__/_| \___/_| |_|_\___| /__/ .__/\___\__|_|_| |_\__| \__,_\__,_|\__\__,_| *
//* |_|                           |_|                                            *
namespace packed {

/// \brief Version 1 of the CI Profile Specific message
struct specific_data_v1 {
  profile pid;                ///< Profile ID
  byte_array<2> data_length;  ///< Length of following profile specific data (LSB first)
  b7 data[1];                 ///< Profile specific data (array length given by data_length)
};
static_assert(offsetof(specific_data_v1, pid) == 0);
static_assert(offsetof(specific_data_v1, data_length) == 5);
static_assert(offsetof(specific_data_v1, data) == 7);
static_assert(sizeof(specific_data_v1) == 8);
static_assert(alignof(specific_data_v1) == 1);
static_assert(std::is_trivially_copyable_v<specific_data_v1>);

}  // end namespace packed

struct specific_data {
  [[nodiscard]] static constexpr specific_data make(packed::specific_data_v1 const &) noexcept;
  explicit constexpr operator packed::specific_data_v1() const noexcept;
  constexpr bool operator==(specific_data const &other) const noexcept {
    return pid == other.pid && std::ranges::equal(data, other.data);
  }

  profile pid{};             ///< Profile ID
  std::span<b7 const> data;  ///< Profile specific data
};

constexpr specific_data specific_data::make(packed::specific_data_v1 const &other) noexcept {
  return {
      .pid = other.pid,
      .data = std::span<b7 const>{std::begin(other.data), ci::details::from_le7(other.data_length).get()},
  };
}
constexpr specific_data::operator packed::specific_data_v1() const noexcept {
  return {
      .pid = pid,
      .data_length = ci::details::to_le7(static_cast<b14>(data.size_bytes())),
      .data = {b7{0U}},
  };
}

}  // namespace profile_configuration

/// \brief Types for MIDI-CI Property Exchange Messages.
namespace property_exchange {

//*                                 _    _ _ _ _   _         *
//*  _ __  ___   __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___ *
//* | '_ \/ -_) / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< *
//* | .__/\___| \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ *
//* |_|                 |_|                                  *
namespace packed {

/// \brief Version 1 of the CI PE Capabilities message
struct capabilities_v1 {
  std::byte num_simultaneous;
};
static_assert(offsetof(capabilities_v1, num_simultaneous) == 0);
static_assert(sizeof(capabilities_v1) == 1);
static_assert(alignof(capabilities_v1) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_v1>);

/// \brief Version 2 of the CI PE Capabilities message
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
  [[nodiscard]] static constexpr capabilities make(packed::capabilities_v1 const &other) noexcept;
  [[nodiscard]] static constexpr capabilities make(packed::capabilities_v2 const &other) noexcept;
  explicit constexpr operator packed::capabilities_v1() const noexcept;
  explicit constexpr operator packed::capabilities_v2() const noexcept;
  constexpr bool operator==(capabilities const &) const noexcept = default;

  b7 num_simultaneous;
  b7 major_version;
  b7 minor_version;
};

constexpr capabilities capabilities::make(packed::capabilities_v1 const &other) noexcept {
  return {
      .num_simultaneous = b7{to_underlying(other.num_simultaneous)},
      .major_version = b7{},
      .minor_version = b7{},
  };
}
constexpr capabilities capabilities::make(packed::capabilities_v2 const &other) noexcept {
  return {
      .num_simultaneous = b7{to_underlying(other.v1.num_simultaneous)},
      .major_version = b7{to_underlying(other.major_version)},
      .minor_version = b7{to_underlying(other.minor_version)},
  };
}
constexpr capabilities::operator packed::capabilities_v1() const noexcept {
  return {.num_simultaneous = static_cast<std::byte>(num_simultaneous.get())};
}
constexpr capabilities::operator packed::capabilities_v2() const noexcept {
  return {
      .v1 = static_cast<packed::capabilities_v1>(*this),
      .major_version = static_cast<std::byte>(major_version.get()),
      .minor_version = static_cast<std::byte>(minor_version.get()),
  };
}

//*                                 _    _ _ _ _   _                       _       *
//*  _ __  ___   __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___  _ _ ___ _ __| |_  _  *
//* | '_ \/ -_) / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< | '_/ -_) '_ \ | || | *
//* | .__/\___| \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ |_| \___| .__/_|\_, | *
//* |_|                 |_|                                          |_|     |__/  *
namespace packed {

/// \brief Version 1 of the CI PE Capabilities Reply message
struct capabilities_reply_v1 {
  std::byte num_simultaneous;
};
static_assert(offsetof(capabilities_reply_v1, num_simultaneous) == 0);
static_assert(sizeof(capabilities_reply_v1) == 1);
static_assert(alignof(capabilities_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_reply_v1>);

/// \brief Version 2 of the CI PE Capabilities Reply message
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
  [[nodiscard]] static constexpr capabilities_reply make(packed::capabilities_reply_v1 const &other) noexcept;
  [[nodiscard]] static constexpr capabilities_reply make(packed::capabilities_reply_v2 const &other) noexcept;
  explicit constexpr operator packed::capabilities_reply_v1() const noexcept;
  explicit constexpr operator packed::capabilities_reply_v2() const noexcept;
  constexpr bool operator==(capabilities_reply const &) const noexcept = default;

  b7 num_simultaneous;
  b7 major_version;
  b7 minor_version;
};

constexpr capabilities_reply capabilities_reply::make(packed::capabilities_reply_v1 const &other) noexcept {
  return {
      .num_simultaneous = b7{to_underlying(other.num_simultaneous)},
      .major_version = b7{},
      .minor_version = b7{},
  };
}
constexpr capabilities_reply capabilities_reply::make(packed::capabilities_reply_v2 const &other) noexcept {
  return {
      .num_simultaneous = b7{to_underlying(other.v1.num_simultaneous)},
      .major_version = b7{to_underlying(other.major_version)},
      .minor_version = b7{to_underlying(other.minor_version)},
  };
}
constexpr capabilities_reply::operator packed::capabilities_reply_v1() const noexcept {
  return {
      .num_simultaneous = static_cast<std::byte>(num_simultaneous.get()),
  };
}
constexpr capabilities_reply::operator packed::capabilities_reply_v2() const noexcept {
  return {
      .v1 = static_cast<packed::capabilities_reply_v1>(*this),
      .major_version = static_cast<std::byte>(major_version.get()),
      .minor_version = static_cast<std::byte>(minor_version.get()),
  };
}

//*                             _                     _                        *
//*  _ __ _ _ ___ _ __  ___ _ _| |_ _  _   _____ ____| |_  __ _ _ _  __ _ ___  *
//* | '_ \ '_/ _ \ '_ \/ -_) '_|  _| || | / -_) \ / _| ' \/ _` | ' \/ _` / -_) *
//* | .__/_| \___/ .__/\___|_|  \__|\_, | \___/_\_\__|_||_\__,_|_||_\__, \___| *
//* |_|          |_|                |__/                            |___/      *
namespace packed {

/// \brief Part 1 of the CI Property Exchange message
struct property_exchange_pt1 {
  std::byte request_id;
  byte_array<2> header_length;
  char header[1];
};
static_assert(offsetof(property_exchange_pt1, request_id) == 0);
static_assert(offsetof(property_exchange_pt1, header_length) == 1);
static_assert(offsetof(property_exchange_pt1, header) == 3);
static_assert(alignof(property_exchange_pt1) == 1);
static_assert(sizeof(property_exchange_pt1) == 4);
static_assert(std::is_trivially_copyable_v<property_exchange_pt1>);

/// \brief Part 2 of the CI Property Exchange message
struct property_exchange_pt2 {
  byte_array<2> number_of_chunks;
  byte_array<2> chunk_number;
  byte_array<2> data_length;
  char data[1];
};
static_assert(offsetof(property_exchange_pt2, number_of_chunks) == 0);
static_assert(offsetof(property_exchange_pt2, chunk_number) == 2);
static_assert(offsetof(property_exchange_pt2, data_length) == 4);
static_assert(offsetof(property_exchange_pt2, data) == 6);
static_assert(alignof(property_exchange_pt2) == 1);
static_assert(sizeof(property_exchange_pt2) == 7);
static_assert(std::is_trivially_copyable_v<property_exchange_pt2>);

}  // end namespace packed

struct chunk_info {
  constexpr bool operator==(chunk_info const &) const noexcept = default;
  b14 number_of_chunks;
  b14 chunk_number;
};

enum class property_exchange_type { get, get_reply, set, set_reply, subscription, subscription_reply, notify };
template <property_exchange_type Pet> class property_exchange {
public:
  [[nodiscard]] static constexpr property_exchange make(chunk_info const &chunk, b7 const request,
                                                        std::span<char const> const header,
                                                        std::span<char const> const data = {}) noexcept {
    return {.chunk = chunk, .request = request, .header = header, .data = data};
  }
  explicit constexpr operator packed::property_exchange_pt1() const noexcept {
    return {
        .request_id = static_cast<std::byte>(request.get()),
        .header_length = details::to_le7(static_cast<b14>(header.size())),
        .header = {'\0'},
    };
  }
  explicit constexpr operator packed::property_exchange_pt2() const noexcept {
    return {
        .number_of_chunks = details::to_le7(chunk.number_of_chunks),
        .chunk_number = details::to_le7(chunk.chunk_number),
        .data_length = details::to_le7(static_cast<b14>(data.size_bytes())),
        .data = {'\0'},
    };
  }
  constexpr bool operator==(property_exchange const &other) const noexcept {
    return chunk == other.chunk && request == other.request && std::ranges::equal(header, other.header) &&
           std::ranges::equal(data, other.data);
  }
  chunk_info chunk;
  b7 request;
  std::span<char const> header;
  std::span<char const> data;
};

using get = property_exchange<property_exchange_type::get>;
using get_reply = property_exchange<property_exchange_type::get_reply>;
using set = property_exchange<property_exchange_type::set>;
using set_reply = property_exchange<property_exchange_type::set_reply>;
using subscription = property_exchange<property_exchange_type::subscription>;
using subscription_reply = property_exchange<property_exchange_type::subscription_reply>;
using notify = property_exchange<property_exchange_type::notify>;

}  // end namespace property_exchange

/// \brief Types for MIDI CI Process Inquiry messages.
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
  [[nodiscard]] static constexpr capabilities_reply make(packed::capabilities_reply_v2 const &v2) noexcept;
  explicit constexpr operator packed::capabilities_reply_v2() const noexcept;
  constexpr bool operator==(capabilities_reply const &) const noexcept = default;

  b7 features;
};

constexpr capabilities_reply capabilities_reply::make(packed::capabilities_reply_v2 const &v2) noexcept {
  return {.features = b7{to_underlying(v2.features)}};
}
constexpr capabilities_reply::operator packed::capabilities_reply_v2() const noexcept {
  return {.features = std::byte{features.get()}};
}

//*        _    _ _                                                          _    *
//*  _ __ (_)__| (_)  _ __  ___ ______ __ _ __ _ ___   _ _ ___ _ __  ___ _ _| |_  *
//* | '  \| / _` | | | '  \/ -_|_-<_-</ _` / _` / -_) | '_/ -_) '_ \/ _ \ '_|  _| *
//* |_|_|_|_\__,_|_| |_|_|_\___/__/__/\__,_\__, \___| |_| \___| .__/\___/_|  \__| *
//*                                        |___/              |_|                 *
namespace packed {

struct midi_message_report_v2 {
  union system_message {
    adt::bitfield<std::uint8_t, 3, 5> reserved0;
    adt::bitfield<std::uint8_t, 2, 1> song_select;
    adt::bitfield<std::uint8_t, 1, 1> song_position;
    adt::bitfield<std::uint8_t, 0, 1> mtc_quarter_frame;
  };
  union channel_controller {
    adt::bitfield<std::uint8_t, 6, 2> reserved0;
    adt::bitfield<std::uint8_t, 5, 1> channel_pressure;
    adt::bitfield<std::uint8_t, 4, 1> program_change;
    adt::bitfield<std::uint8_t, 3, 1> nrpn_assignable_controller;
    adt::bitfield<std::uint8_t, 2, 1> rpn_registered_controller;
    adt::bitfield<std::uint8_t, 1, 1> control_change;
    adt::bitfield<std::uint8_t, 0, 1> pitchbend;
  };
  union note_data_messages {
    adt::bitfield<std::uint8_t, 5, 3> reserved0;
    adt::bitfield<std::uint8_t, 4, 1> assignable_per_note_controller;
    adt::bitfield<std::uint8_t, 3, 1> registered_per_note_controller;
    adt::bitfield<std::uint8_t, 2, 1> per_note_pitchbend;
    adt::bitfield<std::uint8_t, 1, 1> poly_pressure;
    adt::bitfield<std::uint8_t, 0, 1> notes;
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
  [[nodiscard]] static constexpr midi_message_report make(packed::midi_message_report_v2 const &v2) noexcept;
  constexpr explicit operator packed::midi_message_report_v2() const noexcept;
  constexpr bool operator==(midi_message_report const &) const noexcept = default;

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

constexpr midi_message_report midi_message_report::make(packed::midi_message_report_v2 const &v2) noexcept {
  return {.message_data_control = static_cast<enum control>(v2.message_data_control),

          .mtc_quarter_frame = v2.system_message.mtc_quarter_frame,
          .song_position = v2.system_message.song_position,
          .song_select = v2.system_message.song_select,

          .pitchbend = v2.channel_controller.pitchbend,
          .control_change = v2.channel_controller.control_change,
          .rpn_registered_controller = v2.channel_controller.rpn_registered_controller,
          .nrpn_assignable_controller = v2.channel_controller.nrpn_assignable_controller,
          .program_change = v2.channel_controller.program_change,
          .channel_pressure = v2.channel_controller.channel_pressure,

          .notes = v2.note_data_messages.notes,
          .poly_pressure = v2.note_data_messages.poly_pressure,
          .per_note_pitchbend = v2.note_data_messages.per_note_pitchbend,
          .registered_per_note_controller = v2.note_data_messages.registered_per_note_controller,
          .assignable_per_note_controller = v2.note_data_messages.assignable_per_note_controller};
}

constexpr midi_message_report::operator packed::midi_message_report_v2() const noexcept {
  union packed::midi_message_report_v2::system_message sm {};
  sm.mtc_quarter_frame = mtc_quarter_frame;
  sm.song_position = song_position;
  sm.song_select = song_select;

  union packed::midi_message_report_v2::channel_controller cc {};
  cc.pitchbend = pitchbend;
  cc.control_change = control_change;
  cc.rpn_registered_controller = rpn_registered_controller;
  cc.nrpn_assignable_controller = nrpn_assignable_controller;
  cc.program_change = program_change;
  cc.channel_pressure = channel_pressure;

  union packed::midi_message_report_v2::note_data_messages ndm {};
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
    adt::bitfield<std::uint8_t, 3, 5> reserved0;
    adt::bitfield<std::uint8_t, 2, 1> song_select;
    adt::bitfield<std::uint8_t, 1, 1> song_position;
    adt::bitfield<std::uint8_t, 0, 1> mtc_quarter_frame;
  };
  union channel_controller {
    adt::bitfield<std::uint8_t, 6, 2> reserved0;
    adt::bitfield<std::uint8_t, 5, 1> channel_pressure;
    adt::bitfield<std::uint8_t, 4, 1> program_change;
    adt::bitfield<std::uint8_t, 3, 1> nrpn_assignable_controller;
    adt::bitfield<std::uint8_t, 2, 1> rpn_registered_controller;
    adt::bitfield<std::uint8_t, 1, 1> control_change;
    adt::bitfield<std::uint8_t, 0, 1> pitchbend;
  };
  union note_data_messages {
    adt::bitfield<std::uint8_t, 5, 3> reserved0;
    adt::bitfield<std::uint8_t, 4, 1> assignable_per_note_controller;
    adt::bitfield<std::uint8_t, 3, 1> registered_per_note_controller;
    adt::bitfield<std::uint8_t, 2, 1> per_note_pitchbend;
    adt::bitfield<std::uint8_t, 1, 1> poly_pressure;
    adt::bitfield<std::uint8_t, 0, 1> notes;
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
  [[nodiscard]] static constexpr midi_message_report_reply make(
      packed::midi_message_report_reply_v2 const &v2) noexcept;
  constexpr explicit operator packed::midi_message_report_reply_v2() const noexcept;
  constexpr bool operator==(midi_message_report_reply const &) const noexcept = default;

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

constexpr midi_message_report_reply midi_message_report_reply::make(
    packed::midi_message_report_reply_v2 const &v2) noexcept {
  return {
      .mtc_quarter_frame = v2.system_message.mtc_quarter_frame,
      .song_position = v2.system_message.song_position,
      .song_select = v2.system_message.song_select,

      .pitchbend = v2.channel_controller.pitchbend,
      .control_change = v2.channel_controller.control_change,
      .rpn_registered_controller = v2.channel_controller.rpn_registered_controller,
      .nrpn_assignable_controller = v2.channel_controller.nrpn_assignable_controller,
      .program_change = v2.channel_controller.program_change,
      .channel_pressure = v2.channel_controller.channel_pressure,

      .notes = v2.note_data_messages.notes,
      .poly_pressure = v2.note_data_messages.poly_pressure,
      .per_note_pitchbend = v2.note_data_messages.per_note_pitchbend,
      .registered_per_note_controller = v2.note_data_messages.registered_per_note_controller,
      .assignable_per_note_controller = v2.note_data_messages.assignable_per_note_controller,
  };
}
constexpr midi_message_report_reply::operator packed::midi_message_report_reply_v2() const noexcept {
  union packed::midi_message_report_reply_v2::system_message sm {};
  sm.mtc_quarter_frame = mtc_quarter_frame;
  sm.song_position = song_position;
  sm.song_select = song_select;

  union packed::midi_message_report_reply_v2::channel_controller cc {};
  cc.pitchbend = pitchbend;
  cc.control_change = control_change;
  cc.rpn_registered_controller = rpn_registered_controller;
  cc.nrpn_assignable_controller = nrpn_assignable_controller;
  cc.program_change = program_change;
  cc.channel_pressure = channel_pressure;

  union packed::midi_message_report_reply_v2::note_data_messages ndm {};
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
