//===-- UInteger --------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file uinteger.hpp
/// \brief  uinteger<> selects an unsigned integer type with at least a specified number of bits
#ifndef MIDI2_ADT_INTEGER_HPP
#define MIDI2_ADT_INTEGER_HPP

#include <cstddef>
#include <cstdint>

namespace midi2::adt {

/// \brief Yields the smallest unsigned integral type with at least \p N bits.
template <std::size_t N>
  requires(N <= 64)
struct uinteger {
  /// The type of an unsigned integral with at least \p N bits.
  using type = uinteger<N + 1>::type;
};
/// \brief A helper type for convenient use of uinteger<N>::type.
template <std::size_t N> using uinteger_t = uinteger<N>::type;
/// \brief Yields an unsigned integral type of 8 bits or more.
template <> struct uinteger<8> {
  /// Smallest unsigned integer type with width of at least 8 bits.
  using type = std::uint_least8_t;
};
/// \brief Yields an unsigned integral type of 16 bits or more.
template <> struct uinteger<16> {
  /// Smallest unsigned integer type with width of at least 16 bits.
  using type = std::uint_least16_t;
};
/// \brief Yields an unsigned integral type of 32 bits or more.
template <> struct uinteger<32> {
  /// Smallest unsigned integer type with width of at least 32 bits.
  using type = std::uint_least32_t;
};
/// \brief Yields an unsigned integral type of 64 bits or more.
template <> struct uinteger<64> {
  /// Smallest unsigned integer type with width of at least 64 bits.
  using type = std::uint_least64_t;
};

}  // end namespace midi2::adt

#endif  // MIDI2_ADT_INTEGER_HPP
