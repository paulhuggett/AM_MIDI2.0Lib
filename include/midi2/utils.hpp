//===-- Utils -----------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file utils.hpp
/// \brief Miscellaneous utility functions and definitions used by the midi2 library.

#ifndef MIDI2_UTILS_HPP
#define MIDI2_UTILS_HPP

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace midi2 {

/// \brief Indicates that the code path is not expected to be reachable.
///
/// This function is intended to be used in places where the programmer
/// believes that a particular code path should never be executed. The function
/// will be removed once all target toolchains support std::unreachable().
[[noreturn, maybe_unused]] inline void unreachable() {
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
  std::unreachable();
#else
  // Uses compiler specific extensions if possible. Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#if defined(__GNUC__)    // GCC, Clang, ICC
  __builtin_unreachable();
#elif defined(_MSC_VER)  // MSVC
  __assume(false);
#endif
#endif
}

/// Converts a suitable enumeration to std::byte.
///
/// \param e  The enumeration value to convert
/// \returns The value of the enum \p e, converted to std::byte.
template <typename Enum>
  requires(std::is_enum_v<Enum> && sizeof(std::underlying_type_t<Enum>) == sizeof(std::byte))
[[nodiscard]] constexpr std::byte to_byte(Enum const e) noexcept {
  return std::byte{std::to_underlying(e)};
}

/// Returns true if the argument is a power of two and false otherwise.
///
/// \param n An integer value to check whether it is a power of two.
/// \returns True if the input value is a power of 2.
[[nodiscard]] constexpr bool is_power_of_two(std::unsigned_integral auto const n) noexcept {
#if defined(__cpp_lib_int_pow2) && __cpp_lib_int_pow2 >= 202002L
  return std::has_single_bit(n);
#else
  // If a number n is a power of 2 then bitwise & of n and n-1 will be zero.
  return n > 0U && !(n & (n - 1U));
#endif
}

/// \brief Calls a function with the given argument list if it evaluates true.
///
/// An invocable type will evaluate true if it is a non-null function pointer or a std::function<> instance which
/// contains a target.
///
/// \tparam Function An invocable type.
/// \tparam Args  The types of the arguments to be forwarded to the function.
/// \param function  The function to be called.
/// \param args  Arguments to be forwarded to the function.
template <typename Function, typename... Args>
  requires(std::is_invocable_v<Function, Args...>)
inline void call(Function&& function, Args&&... args) {
  if (function) {
    std::invoke(std::forward<Function>(function), std::forward<Args>(args)...);
  }
}

/// \brief Returns the low 7 bits (that is, the bits in the interval [0..7] of the unsigned integral argument \p v.
/// \param v  A value from which the low 7 bits will be extracted.
/// \returns The low 7 bits of the unsigned integral argument \p v.
[[nodiscard]] constexpr std::uint8_t lo7(std::unsigned_integral auto v) noexcept {
  return v & 0x7F;
}
/// Returns bits in the range [8..14] of the unsigned integral argument \p v.
/// \param v  A value from which the low 7 bits will be extracted.
/// \returns The bits in the range [8..14] of the unsigned integral argument \p v.
[[nodiscard]] constexpr std::uint8_t hi7(std::unsigned_integral auto v) noexcept {
  return (v >> 7) & 0x7F;
}

namespace literals {

/// \brief User-defined literal for std::byte.
/// \param arg  The value of the byte literal.
/// \returns The value as a std::byte
[[nodiscard]] consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

}  // end namespace literals

}  // end namespace midi2

#endif  // MIDI2_UTILS_HPP
