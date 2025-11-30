//===-- Portable Bitfields ----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file bitfield.hpp
/// \brief  Portable bit-fields

#ifndef MIDI2_ADT_BITFIELD_HPP
#define MIDI2_ADT_BITFIELD_HPP

#include <cassert>
#include <climits>
#include <concepts>
#include <limits>
#include <type_traits>

#include "midi2/adt/uinteger.hpp"

namespace midi2::adt {

/// \brief Defines the starting bit index and number of bits for a bitfield.
///
/// \tparam Index The index of the first bit in the bitfield (0 = least significant bit).
/// \tparam Bits The number of bits in the bitfield.
template <unsigned Index, unsigned Bits>
  requires(Bits > 0 && Index + Bits <= 64)
struct bit_range {
  /// \brief The index of the first bit in the bitfield.
  using index = std::integral_constant<unsigned, Index>;
  /// \brief The number of bits in the bitfield.
  using bits = std::integral_constant<unsigned, Bits>;

  /// \brief An unsigned integer type with at least \p Bits bits.
  using uinteger = uinteger_t<bits::value>;
  /// \brief An signed integer type with at least \p Bits + 1 bits.
  using sinteger = std::make_signed_t<uinteger_t<bits::value>>;
};

template <typename T>
concept bit_range_type = std::is_same_v<T, bit_range<T::index::value, T::bits::value>>;

/// \returns The maximum value that can be held in \p Bits of type \p T.
template <std::unsigned_integral T, unsigned Bits>
  requires(Bits <= sizeof(T) * CHAR_BIT && Bits <= 64U)
[[nodiscard]] static consteval T max_value() noexcept {
  if constexpr (Bits == 0U) {
    return T{0};
  } else if constexpr (Bits == 8U) {
    return std::numeric_limits<std::uint8_t>::max();
  } else if constexpr (Bits == 16U) {
    return std::numeric_limits<std::uint16_t>::max();
  } else if constexpr (Bits == 32U) {
    return std::numeric_limits<std::uint32_t>::max();
  } else if constexpr (Bits == 64U) {
    return std::numeric_limits<std::uint64_t>::max();
  } else {
    return static_cast<T>((T{1} << Bits) - 1U);
  }
}

template <std::unsigned_integral T> class bit_field {
public:
  using value_type = T;

  constexpr bit_field() noexcept = default;
  constexpr explicit bit_field(value_type const v) noexcept : value_{v} {}

  [[nodiscard]] constexpr explicit operator value_type() const noexcept { return value_; }

  friend constexpr bool operator==(bit_field const& a, bit_field const& b) noexcept = default;

  /// Sets the bits described by \p BitRange to the value \p v.
  ///
  /// \tparam BitRange  A bitfield_type which describes a range of bits.
  /// \param v  The new value for the bitfield.
  /// \returns *this
  /// \pre v < 2^BitRange::bits()
  template <bit_range_type BitRange> constexpr auto& set(BitRange::uinteger const v) noexcept {
    constexpr auto index = typename BitRange::index();
    constexpr auto bits = typename BitRange::bits();
    constexpr auto mask = max_value<T, bits>();
    assert(v <= mask && "bitfield value is out-of-range");
    value_ = static_cast<value_type>(value_ & ~(mask << index)) |
             static_cast<value_type>((static_cast<value_type>(v) & mask) << index);
    return *this;
  }

  /// Sets the bits described by \p BitRange to the signed value \p v.
  ///
  /// \tparam BitRange  A bitfield_type which describes a range of bits.
  /// \param v  The new value for the bitfield.
  /// \returns *this
  /// \pre v -(2^b + 1) <= v <= 2^b where b is BitRange::bits()-1.
  template <bit_range_type BitRange> constexpr auto& set_signed(BitRange::sinteger const v) noexcept {
    constexpr auto index = typename BitRange::index();
    constexpr auto bits = typename BitRange::bits();
    constexpr auto mask = max_value<T, bits>();

    assert((v >= -static_cast<typename BitRange::sinteger>(max_value<typename BitRange::uinteger, bits - 1>()) - 1) &&
           (v <= static_cast<typename BitRange::sinteger>(max_value<typename BitRange::uinteger, bits - 1>())) &&
           "bit-field value is out-of-range");

    value_ = static_cast<value_type>(value_ & ~(mask << index)) |
             static_cast<value_type>((static_cast<value_type>(v) & mask) << index);
    return *this;
  }

  /// Returns the value held by the bits described by \p BitRange.
  ///
  /// \tparam BitRange  A bitfield_type which describes a range of bits.
  /// \returns The value held by the bits described by \p BitRange.
  template <bit_range_type BitRange> [[nodiscard]] constexpr BitRange::uinteger get() const noexcept {
    constexpr auto index = typename BitRange::index();
    constexpr auto bits = typename BitRange::bits();
    constexpr auto mask = max_value<value_type, bits>();
    return (value_ >> index) & mask;
  }

  [[nodiscard]] constexpr value_type get(unsigned index, unsigned bits) const noexcept {
    return (value_ >> index) & ((1U << bits) - 1U);
  }

  /// Returns the value stored in the bitfield as a signed quantity.
  ///
  /// \tparam BitRange  A bitfield_type which describes a range of bits.
  /// \returns The signed value held by the bits described by \p BitRange.
  template <bit_range_type BitRange> [[nodiscard]] constexpr BitRange::sinteger get_signed() const noexcept {
    // Taken from Bit Twiddling Hacks by Sean Eron Anderson:
    // https://graphics.stanford.edu/~seander/bithacks.html#VariableSignExtend
    constexpr auto m = value_type{1} << (typename BitRange::bits() - 1U);
    auto const u = static_cast<BitRange::uinteger>(this->template get<BitRange>());
    return static_cast<BitRange::sinteger>((u ^ m) - m);
  }

private:
  value_type value_ = 0;
};

}  // end namespace midi2::adt

#endif  // MIDI2_ADT_BITFIELD_HPP
