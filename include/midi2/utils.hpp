//===-- Utils -----------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UTILS_HPP
#define MIDI2_UTILS_HPP

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace midi2 {

[[noreturn, maybe_unused]] inline void unreachable() {
  // Uses compiler specific extensions if possible. Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#ifdef __GNUC__  // GCC, Clang, ICC
  __builtin_unreachable();
#elif defined(_MSC_VER)  // MSVC
  __assume(false);
#endif
}

/// Converts an enumeration value to its underlying type
///
/// \param e  The enumeration value to convert
/// \returns The integer value of the underlying type of Enum, converted from \p e.
template <typename Enum>
  requires std::is_enum_v<Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum const e) noexcept {
#if defined(__cpp_lib_to_underlying) && __cpp_lib_to_underlying > 202102L
  return std::to_underlying(e);
#else
  return static_cast<std::underlying_type_t<Enum>>(e);
#endif
}

/// Returns true if the argument is a power of two and false otherwise.
///
/// \tparam T An unsigned integer type.
/// \param n An integer value to check whether it is a power of two.
/// \returns True if the input value is a power of 2.
template <std::unsigned_integral T> constexpr bool is_power_of_two(T const n) noexcept {
  // If a number n is a power of 2 then bitwise & of n and n-1 will be zero.
  return n > 0U && !(n & (n - 1U));
}

enum class status : std::uint8_t {
  // Channel voice messages
  note_off = 0x80,
  note_on = 0x90,
  poly_pressure = 0xA0,  // Polyphonic Key Pressure (Aftertouch).
  cc = 0xB0,             // Continuous Controller
  program_change = 0xC0,
  channel_pressure = 0xD0,  // Channel Pressure (Aftertouch).
  pitch_bend = 0xE0,

  // System Common/Real Time/Exclusive Messages
  sysex_start = 0xF0,  ///< Begin system exclusive data
  timing_code = 0xF1,
  spp = 0xF2,  ///< Song Position Pointer
  song_select = 0xF3,
  reserved1 = 0xF4,
  reserved2 = 0xF5,
  tune_request = 0xF6,
  sysex_stop = 0xF7,  ///< End of system exclusive
  timing_clock = 0xF8,
  reserved3 = 0xF9,
  sequence_start = 0xFA,     // Start the current sequence playing
  sequence_continue = 0xFB,  // Continue at the point the sequence was stopped
  sequence_stop = 0xFC,      // Stop the current sequence
  reserved4 = 0xFD,
  active_sensing = 0xFE,
  systemreset = 0xFF,
};

constexpr bool is_system_real_time_message(std::byte const midi1_byte) noexcept {
  switch (static_cast<status>(midi1_byte)) {
  case status::timing_clock:
  case status::sequence_start:
  case status::sequence_continue:
  case status::sequence_stop:
  case status::active_sensing:
  case status::systemreset: return true;
  default: return false;
  }
}

constexpr bool is_status_byte(std::byte const midi1_byte) noexcept {
  return (midi1_byte & std::byte{0x80}) != std::byte{0x00};
}

constexpr auto S7UNIVERSAL_NRT = std::byte{0x7E};
constexpr auto S7MIDICI = std::byte{0x0D};

// The MIDI 1.0 Specification defines Control Change indexes 98, 99, 100, and
// 101 (0x62, 0x63, 0x64, and 0x65) to be used as compound sequences for
// Non-Registered Parameter Number and Registered Parameter Number control
// messages. These set destinations for Control Change index 6/38 (0x06/0x26),
// Data Entry.
enum class control : std::uint8_t {
  bank_select = 0x00,
  bank_select_lsb = 0x20,
  data_entry_msb = 0x06,
  data_entry_lsb = 0x26,
  rpn_lsb = 0x64,
  rpn_msb = 0x65,
  nrpn_lsb = 0x62,
  nrpn_msb = 0x63,

  /// When a device receives the Reset All Controllers message, it should reset the
  /// condition of all its controllers what it considers an ideal initial state.
  reset_all_controllers = 0x79,
};

enum class ci_message : std::uint8_t {
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
  endpoint_info = 0x72,
  endpoint_info_reply = 0x73,
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

constexpr auto M2_CI_BROADCAST = std::uint32_t{0x0FFFFFFF};

enum : std::uint32_t {
  UMP_VER_MAJOR = 1,
  UMP_VER_MINOR = 1,
};

template <unsigned Bits>
using small_type = std::conditional_t<
    Bits <= 8, std::uint8_t,
    std::conditional_t<Bits <= 16, std::uint16_t, std::conditional_t<Bits <= 32, std::uint32_t, std::uint64_t>>>;

/// Implements the "min-center-max" scaling algorithm from section 3 of the document "M2-115-U MIDI 2.0 Bit Scaling and
/// Resolution v1.0.1 23-May-2023"
template <unsigned SourceBits, unsigned DestBits>
  requires(SourceBits > 1 && DestBits <= 32)
constexpr small_type<DestBits> mcm_scale(small_type<SourceBits> const value) noexcept {
  if constexpr (SourceBits >= DestBits) {
    return static_cast<small_type<DestBits>>(value >> (SourceBits - DestBits));
  } else {
    if (value == 0) {
      return 0;
    }
    constexpr auto scale_bits = DestBits - SourceBits;  // Number of bits to upscale
    // Calculate the center value for SourceBits, e.g. 0x40 (64) for 7 bits,  0x2000 (8192) for 14 bits
    constexpr auto center = 1U << (SourceBits - 1);
    // Simple bit shift
    auto bit_shifted_value = static_cast<std::uint32_t>(value << scale_bits);
    if (value <= center) {
      return static_cast<small_type<DestBits>>(bit_shifted_value);
    }

    // expanded bit repeat scheme
    constexpr auto repeat_bits = SourceBits - 1;             // We must repeat all but the highest bit
    auto repeat_value = value & ((1U << repeat_bits) - 1U);  // Repeat bit sequence
    if constexpr (scale_bits > repeat_bits) {
      repeat_value <<= scale_bits - repeat_bits;
    } else {
      repeat_value >>= repeat_bits - scale_bits;
    }
    for (; repeat_value != 0; repeat_value >>= repeat_bits) {
      bit_shifted_value |= repeat_value;  // Fill lower bits with repeat_value
    }
    return static_cast<small_type<DestBits>>(bit_shifted_value);
  }
}

}  // end namespace midi2

#endif  // MIDI2_UTILS_HPP
