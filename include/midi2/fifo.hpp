#ifndef MIDI2_FIFO_HPP
#define MIDI2_FIFO_HPP

#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace midi2 {

/// \returns True if the input value is a power of 2.
template <std::unsigned_integral T> constexpr bool is_power_of_two(T const n) noexcept {
  // If a number n is a power of 2 then bitwise & of n and n-1 will be zero.
  return n > 0U && !(n & (n - 1U));
}

/// \returns  The number of bits required for value.
// NOLINTNEXTLINE(misc-no-recursion)
template <std::unsigned_integral T> consteval unsigned bits_required(T const value) {
  return value == 0U ? 0U : 1U + bits_required(static_cast<T>(value >> 1U));
}

/// \brief A FIFO/circular buffer containing a maximum of \p Elements instances
///        of type \p ElementType.
///
/// The hardest part about implementing a FIFO is that both full and empty are
/// indicated by the fact that read and write indices are equal. This
/// implementation resolves this by using an extra bit for both values and
/// comparing the extra bit for equality (for FIFO empty) or inequality (for
/// FIFO full), along with equality of the other read and write index bits.
///
/// \tparam ElementType The type of the elements held by this container.
/// \tparam Elements The number of elements in the FIFO. Must be less than 2^32.
template <typename ElementType, std::uint32_t Elements>
requires(Elements > 1 && is_power_of_two(Elements) && Elements < std::uint32_t{1} << 31) class fifo {
public:
  /// \brief Inserts an element at the end.
  /// \param value  The value of the element to append.
  /// \returns True if the element was appended, false if the container was full.
  bool push_back(ElementType const& value) {
    if (this->full()) {
      return false;
    }
    arr_[(writeIndex_++) & mask_] = value;
    return true;
  }
  /// \brief Removes the first element of the container and returns it.
  /// If there are no elements in the container, the behavior is undefined.
  /// \returns The first element in the container.
  ElementType pop_front() {
    assert(!this->empty());
    auto result = std::move(arr_[readIndex_ & mask_]);
    ++readIndex_;
    return result;
  }
  /// \brief Checks whether the container is empty.
  /// The FIFO is empty when both indices are equal.
  /// \returns True if the container is empty, false otherwise.
  [[nodiscard]] constexpr bool empty() const { return writeIndex_ == readIndex_; }
  /// \brief Checks whether the container is full.
  /// The FIFO is full then when both indices are equal but the "wrap" fields
  /// different.
  /// \returns True if the container is full, false otherwise.
  [[nodiscard]] constexpr bool full() const { return (writeIndex_ & mask_) == (readIndex_ & mask_) && wrapped(); }
  /// \brief Returns the number of elements.
  [[nodiscard]] constexpr std::size_t size() const {
    auto const w = (writeIndex_ & mask_) + (wrapped() ? Elements : 0U);
    auto const r = readIndex_ & mask_;
    assert(w >= r);
    return w - r;
  }
  /// \brief Returns the maximum possible number of elements.
  [[nodiscard]] constexpr std::size_t max_size() const { return Elements; }

private:
  std::array<ElementType, Elements> arr_{};

  [[nodiscard]] constexpr bool wrapped() const { return (writeIndex_ & ~mask_) != (readIndex_ & ~mask_); }

  // The number of bits required to represent the maximum index in the arr_
  // container.
  static constexpr auto bits_ = bits_required(Elements - 1U);
  using bitfield_type =
      std::conditional_t<(bits_ < 4U), std::uint8_t, std::conditional_t<(bits_ < 8U), std::uint16_t, std::uint32_t>>;
  static constexpr auto mask_ = (bitfield_type{1} << bits_) - 1U;

  bitfield_type writeIndex_ : bits_ + 1 = 0;
  bitfield_type readIndex_ : bits_ + 1 = 0;
};

}  // end namespace midi2

#endif  // MIDI2_FIFO_HPP
