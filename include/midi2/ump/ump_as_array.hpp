//===-- UMP As Array ----------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ump_as_array.hpp
/// \brief UMP array type helpers
/// Some of the UMP types can be treated as containing arrays: specifically the four data64::sysex7 messages, the four
/// data128::sysex8 messages, and the flex_data::text_common message. The file contains helper types which those
/// message classes use to implement this behaviour.

#ifndef UMP_AS_ARRAY_HPP
#define UMP_AS_ARRAY_HPP

#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <type_traits>

namespace midi2::ump::details {

template <typename T>
concept accessible_as_array = requires(T&& t) {
  typename T::value_type;
  requires requires { t.data(std::size_t{}, typename T::value_type{}); } || std::is_const_v<T>;
  { t.data(std::size_t{}) } -> std::convertible_to<typename T::value_type>;
};

template <accessible_as_array T> class array_subscript_proxy {
public:
  using value_type = typename T::value_type;

  constexpr array_subscript_proxy() noexcept = default;
  constexpr array_subscript_proxy(T* const owner, std::size_t const index) noexcept : owner_{owner}, index_{index} {
    assert(owner != nullptr);
  }
  template <typename U>
    requires(std::is_same_v<U, std::remove_const_t<T>> || std::is_same_v<U, T const>)
  friend bool operator==(array_subscript_proxy const& lhs, array_subscript_proxy<U> const& rhs) {
    return lhs.owner_ == rhs.owner_ && lhs.index_ == rhs.index_;
  }
  template <typename U>
    requires(std::is_same_v<U, std::remove_const_t<T>> || std::is_same_v<U, T const>)
  friend constexpr std::partial_ordering operator<=>(array_subscript_proxy const& lhs,
                                                     array_subscript_proxy<U> const& rhs) noexcept {
    return std::tuple{lhs.owner_, lhs.index_} <=> std::tuple{rhs.owner_, rhs.index_};
  }
  constexpr array_subscript_proxy& operator=(value_type const v) noexcept
    requires(!std::is_const_v<T>)
  {
    owner_->data(index_, v);
    return *this;
  }
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  constexpr operator value_type() const noexcept { return owner_->data(index_); }

  constexpr array_subscript_proxy& operator++() noexcept {
    ++index_;
    return *this;
  }
  constexpr array_subscript_proxy operator++(int) noexcept {
    constexpr auto result = *this;
    ++*this;
    return result;
  }
  constexpr array_subscript_proxy& operator--() noexcept {
    assert(index_ > 0 && "Index out of range");
    --index_;
    return *this;
  }
  constexpr array_subscript_proxy operator--(int) noexcept {
    constexpr auto result = *this;
    --*this;
    return result;
  }

  template <std::integral U> constexpr auto& operator+=(U const n) noexcept {
    index_ += n;
    return *this;
  }
  template <std::integral U> constexpr auto& operator-=(U const n) noexcept {
    assert(n <= index_ && "Index out of range");
    index_ -= n;
    return *this;
  }

  [[nodiscard]] constexpr T const* owner() const noexcept { return owner_; }
  [[nodiscard]] constexpr T* owner() noexcept
    requires(!std::is_const_v<T>)
  {
    return owner_;
  }
  [[nodiscard]] constexpr std::size_t index() const noexcept { return index_; }

private:
  T* owner_ = nullptr;
  std::size_t index_ = 0;
};

template <typename T> class iterator_base {
public:
  /// Defines this class as fulfilling the requirements of a random-access iterator.
  using iterator_category = std::random_access_iterator_tag;
  /// The type accessible through this iterator.
  using value_type = array_subscript_proxy<T>;
  /// A type that can be used to identify distance between iterators.
  using difference_type = std::ptrdiff_t;
  /// Defines a pointer to the type iterated over.
  using pointer = value_type*;
  /// Defines a reference to the type iterated over.
  using reference = value_type&;

  constexpr iterator_base() noexcept = default;
  constexpr iterator_base(T* const owner, std::size_t const index) noexcept
      : arr_{array_subscript_proxy<T>{owner, index}} {}
  constexpr iterator_base(iterator_base const& other) noexcept = default;
  constexpr iterator_base(iterator_base&& other) noexcept = default;
  ~iterator_base() noexcept = default;

  iterator_base& operator=(iterator_base const& other) noexcept = default;
  iterator_base& operator=(iterator_base&& other) noexcept = default;

  template <typename U>
    requires(std::is_same_v<std::remove_const<T>, std::remove_const<U>> &&
             (std::is_const_v<T> == std::is_const_v<U> || !std::is_const_v<U>))
  iterator_base& operator=(iterator_base<U> const& other) noexcept {
    arr_ = other.arr_;
    return *this;
  }
  template <typename U>
    requires(std::is_same_v<std::remove_const<T>, std::remove_const<U>> &&
             (std::is_const_v<T> == std::is_const_v<U> || !std::is_const_v<U>))
  iterator_base& operator=(iterator_base<U>&& other) noexcept {
    arr_ = std::move(other.arr_);
    return *this;
  }

  template <typename U>
    requires(std::is_same_v<std::remove_const<T>, std::remove_const<U>>)
  friend constexpr bool operator==(iterator_base lhs, iterator_base<U> rhs) noexcept {
    return lhs.arr_ == rhs.arr_;
  }

  template <typename U>
    requires(std::is_same_v<std::remove_const<T>, std::remove_const<U>>)
  friend constexpr auto operator<=>(iterator_base lhs, iterator_base<U> rhs) noexcept {
    return lhs.arr_ <=> rhs.arr_;
  }

  constexpr pointer operator->() const noexcept { return &arr_; }
  constexpr reference operator*() const noexcept { return arr_; }
  constexpr reference operator[](std::size_t const n) const noexcept { return arr_ + n; }

  constexpr iterator_base& operator++() noexcept {
    ++arr_;
    return *this;
  }
  constexpr iterator_base operator++(int) noexcept {
    auto const prev = *this;
    ++*this;
    return prev;
  }
  constexpr iterator_base& operator--() noexcept {
    --arr_;
    return *this;
  }
  constexpr iterator_base operator--(int) noexcept {
    auto const prev = *this;
    --*this;
    return prev;
  }

  template <std::integral U> constexpr iterator_base& operator+=(U const n) noexcept {
    arr_ += n;
    return *this;
  }
  template <std::integral U> constexpr iterator_base& operator-=(U const n) noexcept {
    arr_ -= n;
    return *this;
  }

  /// Returns the distance between two iterators \p b - \p a.
  ///
  /// \param b  The first iterator.
  /// \param a  The second iterator.
  /// \returns  distance between two iterators \p b - \p a.
  friend constexpr difference_type operator-(iterator_base b, iterator_base a) noexcept {
    assert(a.arr_.owner() == b.arr_.owner() && "Cannot get the distance between iterators with different owners");
    return static_cast<difference_type>(b.arr_.index()) - static_cast<difference_type>(a.arr_.index());
  }

  /// @{
  /// Move an iterator \p it forwards by distance \p n. \p n can be both positive
  /// or negative.

  /// \param it  The iterator to be moved.
  /// \param n  The distance by which iterator \p it should be moved.
  /// \returns  The new iterator.
  template <std::integral U> friend constexpr iterator_base operator+(iterator_base const it, U const n) noexcept {
    auto t = it;
    return t += n;
  }
  template <std::integral U> friend constexpr iterator_base operator+(U const n, iterator_base const it) noexcept {
    return it + n;
  }
  /// @}

  /// @{
  /// Move an iterator \p it backwards by distance \p n. \p n can be both positive
  /// or negative.
  /// \param it  The iterator to be moved.
  /// \param n  The distance by which iterator \p it should be moved.
  /// \returns  The new iterator.
  template <std::integral U> friend constexpr iterator_base operator-(iterator_base const it, U const n) noexcept {
    auto t = it;
    return t -= n;
  }
  template <std::integral U> friend constexpr iterator_base operator-(U const n, iterator_base const it) noexcept {
    return it - n;
  }
  /// @}

private:
  mutable value_type arr_;
};

template <typename Message, unsigned Bits> struct data_helper {
  using value_type = typename Message::value_type;

  template <std::integral T> static constexpr std::size_t data(Message& message, std::initializer_list<T> vs) {
    assert(vs.size() <= message.max_size() && "initializer list has too many members");
    auto index = std::size_t{0};
    for (auto const& v : vs) {
      if (index >= message.max_size()) {
        break;
      }
      assert(v >= 0 && v < (1 << Bits) && "data value is out of range");
      message[index] = static_cast<::midi2::adt::uinteger_t<Bits>>(v);
      ++index;
    }
    return index;
  }
  template <std::ranges::input_range Range, typename Proj = std::identity>
    requires std::is_integral_v<std::remove_cvref_t<std::ranges::range_value_t<Range>>>
  static constexpr std::size_t data(Message& message, Range&& range, Proj proj = {}) {
    auto index = std::size_t{0};
    std::ranges::for_each(
        std::forward<Range>(range),
        [&message, &index](auto const& v) constexpr {
          if (index < message.max_size()) {
            assert(v >= 0 && v < (1 << Bits) && "data value is out of range");
            message[index] = static_cast<::midi2::adt::uinteger_t<Bits>>(v);
            ++index;
          }
        },
        proj);
    return index;
  }
  template <std::input_iterator I, std::sentinel_for<I> S>
    requires std::is_integral_v<std::remove_cvref_t<typename std::iterator_traits<I>::value_type>>
  static constexpr std::size_t data(Message& message, I first, S last) {
    auto index = std::size_t{0};
    std::for_each(first, last, [&message, &index](auto const& v) constexpr {
      if (index < message.max_size()) {
        assert(v >= 0 && v < (1 << Bits) && "data value is out of range");
        message[index] = static_cast<::midi2::adt::uinteger_t<Bits>>(v);
        ++index;
      }
    });
    return index;
  }
};

}  // namespace midi2::ump::details

#endif  // UMP_AS_ARRAY_HPP
