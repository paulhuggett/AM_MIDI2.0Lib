//===-- FIFO ------------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file fifo.hpp
/// \brief  Provides an efficient in-place FIFO/circular buffer.

#ifndef MIDI2_FIFO_HPP
#define MIDI2_FIFO_HPP

#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "midi2/adt/uinteger.hpp"
#include "midi2/utils.hpp"

namespace midi2::adt {

namespace details {

/// \returns  The number of bits required for value.
// NOLINTNEXTLINE(misc-no-recursion)
template <std::unsigned_integral T> consteval unsigned bits_required(T const value) noexcept {
  return value == 0U ? 0U : 1U + bits_required(static_cast<T>(value >> 1U));
}

}  // end namespace details

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
  requires(Elements > 1 && is_power_of_two(Elements) && Elements < std::uint32_t{1} << 31)
class fifo {
public:
  /// The type of elements contained in the FIFO
  using value_type = ElementType;
  /// Represents the size of the container
  using size_type = std::size_t;
  /// The type of a reference to elements contained in the FIFO
  using reference = value_type&;
  /// The type of a const-reference to elements contained in the FIFO
  using const_reference = value_type const&;
  /// The type of a pointer to elements contained in the FIFO
  using pointer = value_type*;
  /// The type of a const-pointer to elements contained in the FIFO
  using const_pointer = value_type const*;

  constexpr fifo() noexcept = default;
  constexpr fifo(fifo const& other) noexcept(std::is_nothrow_copy_constructible_v<ElementType>);
  constexpr fifo(fifo&& other) noexcept(std::is_nothrow_move_constructible_v<ElementType>);

  ~fifo() noexcept { this->clear(); }

  constexpr bool operator==(fifo const&) const = delete;

  fifo& operator=(fifo const&) = delete;
  fifo& operator=(fifo&&) noexcept = delete;

  /// \brief Inserts an element at the end.
  /// \param value  The value of the element to append.
  /// \returns True if the element was appended, false if the container was full.
  constexpr bool push_back(value_type const& value) noexcept(std::is_nothrow_copy_constructible_v<value_type>) {
    if (this->full()) {
      return false;
    }
    std::construct_at(this->write_address(), value);
    write_index_++;
    return true;
  }
  /// \brief Inserts an element at the end.
  /// \param value  The value of the element to append.
  /// \returns True if the element was appended, false if the container was full.
  constexpr bool push_back(value_type&& value) noexcept(std::is_nothrow_move_constructible_v<value_type>) {
    if (this->full()) {
      return false;
    }
    std::construct_at(this->write_address(), std::move(value));
    write_index_++;
    return true;
  }
  /// \brief Inserts an element at the end.
  /// \param args  The value of the element to append.
  /// \returns True if the element was appended, false if the container was full.
  template <typename... Args> constexpr bool emplace_back(Args&&... args) {
    if (this->full()) {
      return false;
    }
    std::construct_at(this->write_address(), std::forward<Args>(args)...);
    write_index_++;
    return true;
  }

  /// \brief Removes the first element of the container and returns it.
  /// If there are no elements in the container, the behavior is undefined.
  /// \returns The first element in the container.
  constexpr value_type pop_front() {
    assert(!this->empty());
    auto& v = arr_[read_index_ & mask_].value();
    auto result = std::move(v);
    std::destroy_at(&v);
    ++read_index_;
    return result;
  }
  /// \brief Checks whether the container is empty.
  /// The FIFO is empty when both indices are equal.
  /// \returns True if the container is empty, false otherwise.
  [[nodiscard]] constexpr bool empty() const noexcept { return write_index_ == read_index_; }
  /// \brief Checks whether the container is full.
  /// The FIFO is full then when both indices are equal but the "wrap" fields
  /// different.
  /// \returns True if the container is full, false otherwise.
  [[nodiscard]] constexpr bool full() const noexcept {
    return (write_index_ & mask_) == (read_index_ & mask_) && wrapped();
  }
  /// \brief Returns the number of elements.
  [[nodiscard]] constexpr size_type size() const noexcept {
    auto const w = (write_index_ & mask_) + (wrapped() ? Elements : 0U);
    auto const r = read_index_ & mask_;
    assert(w >= r);
    return w - r;
  }
  /// \brief Returns the maximum possible number of elements.
  [[nodiscard]] static constexpr size_type max_size() noexcept { return Elements; }

  /// \brief Erases all elements from the container. After this call, size() returns zero.
  constexpr void clear() noexcept {
    for (; read_index_ != write_index_; ++write_index_) {
      std::destroy_at(&arr_[read_index_ & mask_].value());
    }
    assert(size() == 0 && empty());
  }

private:
  template <typename T> struct aligned_storage {
    [[nodiscard]] constexpr T& value() noexcept { return *std::bit_cast<T*>(&v[0]); }
    [[nodiscard]] constexpr T const& value() const noexcept { return *std::bit_cast<T const*>(&v[0]); }
    alignas(T) std::byte v[sizeof(T)];
  };
  std::array<aligned_storage<value_type>, Elements> arr_{};

  [[nodiscard]] constexpr bool wrapped() const noexcept { return (write_index_ & ~mask_) != (read_index_ & ~mask_); }
  [[nodiscard]] constexpr value_type* write_address() noexcept { return &arr_[(write_index_)&mask_].value(); }
  /// The number of bits required to represent the maximum index in the arr_
  /// container.
  static constexpr auto bits_ = details::bits_required(Elements - 1U);
  /// An unsigned integer type that can represent bits_ bits.
  using bitfield_type = uinteger_t<bits_>;
  static constexpr auto mask_ = (bitfield_type{1} << bits_) - 1U;

  bitfield_type write_index_ : bits_ + 1 = 0;
  bitfield_type read_index_ : bits_ + 1 = 0;
};

template <typename ElementType, std::uint32_t Elements>
  requires(Elements > 1 && is_power_of_two(Elements) && Elements < std::uint32_t{1} << 31)
constexpr fifo<ElementType, Elements>::fifo(fifo const& other) noexcept(
    std::is_nothrow_copy_constructible_v<ElementType>)
    : write_index_{other.read_index_}, read_index_{other.read_index_} {
  for (write_index_ = other.read_index_; write_index_ != other.write_index_; ++write_index_) {
    auto const wi = write_index_ & mask_;
    std::construct_at(&arr_[wi].value(), other.arr_[wi].value());
  }
}

template <typename ElementType, std::uint32_t Elements>
  requires(Elements > 1 && is_power_of_two(Elements) && Elements < std::uint32_t{1} << 31)
constexpr fifo<ElementType, Elements>::fifo(fifo&& other) noexcept(std::is_nothrow_move_constructible_v<ElementType>)
    : write_index_{other.read_index_}, read_index_{other.read_index_} {
  for (write_index_ = other.read_index_; write_index_ != other.write_index_; ++write_index_) {
    auto const wi = write_index_ & mask_;
    std::construct_at(&arr_[wi].value(), std::move(other.arr_[wi].value()));
  }
}

}  // end namespace midi2::adt

#endif  // MIDI2_FIFO_HPP
