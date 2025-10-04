//===-- Utils -----------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

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

template <typename Function, typename... Args>
  requires(std::is_invocable_v<Function, Args...>)
inline void call(Function&& function, Args&&... args) {
  if (function) {
    std::invoke(std::forward<Function>(function), std::forward<Args>(args)...);
  }
}

constexpr std::uint8_t lo7(std::unsigned_integral auto v) noexcept {
  return v & 0x7F;
}
constexpr std::uint8_t hi7(std::unsigned_integral auto v) noexcept {
  return (v >> 7) & 0x7F;
}

namespace literals {

[[nodiscard]] consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

}  // end namespace literals

}  // end namespace midi2

#endif  // MIDI2_UTILS_HPP
