/// \file ump_utils.hpp
/// \brief UMP utility functions

#ifndef MIDI2_UMP_UMP_UTILS_HPP
#define MIDI2_UMP_UMP_UTILS_HPP

#include "midi2/adt/uinteger.hpp"

namespace midi2::ump {

/// The MIDI 1.0 Specification defines Control Change indexes 98, 99, 100, and
/// 101 (0x62, 0x63, 0x64, and 0x65) to be used as compound sequences for
/// Non-Registered Parameter Number and Registered Parameter Number control
/// messages. These set destinations for Control Change index 6/38 (0x06/0x26),
/// Data Entry.
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

/// Implements the "min-center-max" scaling algorithm from section 3 of the document "M2-115-U MIDI 2.0 Bit Scaling and
/// Resolution v1.0.1 23-May-2023"
///
/// \tparam SourceBits The number of bits in the source value
/// \tparam DestBits The number of bits in the destination value
/// \param value The value whose scale it to be adjusted expressed in \p SourceBits bits
/// \returns The adjusted integer value in \p DestBits bits
template <unsigned SourceBits, unsigned DestBits>
  requires(SourceBits > 1 && DestBits <= 32)
[[nodiscard]] constexpr adt::uinteger_t<DestBits> mcm_scale(adt::uinteger_t<SourceBits> const value) noexcept {
  if constexpr (SourceBits >= DestBits) {
    return static_cast<adt::uinteger_t<DestBits>>(value >> (SourceBits - DestBits));
  } else {
    if (value == 0) {
      return 0;
    }
    constexpr auto scale_bits = DestBits - SourceBits;  // Number of bits to upscale
    // Calculate the center value for SourceBits, e.g. 0x40 (64) for 7 bits,  0x2000 (8192) for 14 bits
    constexpr auto center = 1U << (SourceBits - 1);
    // Simple bit shift
    auto bit_shifted_value = static_cast<adt::uinteger_t<DestBits>>(value << scale_bits);
    if (value <= center) {
      return bit_shifted_value;
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
    return bit_shifted_value;
  }
}

}  // namespace midi2::ump

#endif  // MIDI2_UMP_UMP_UTILS_HPP
