//===-- In-place Unordered Map ------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_IUMAP_HPP
#define MIDI2_IUMAP_HPP

#include "midi2/utils.hpp"

// Standard library
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

// Standard library: for quick and dirty trace output
#ifdef IUMAP_TRACE
#include <ostream>
#endif  // IUMAP_TRACE

namespace midi2 {

// An in-place unordered hash table.
template <typename Key, typename Mapped, std::size_t Size, typename Hash = std::hash<std::remove_cv_t<Key>>,
          typename KeyEqual = std::equal_to<Key>>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
class iumap {
  friend class iterator;
  struct member;

public:
  using key_type = Key;
  using mapped_type = Mapped;
  using value_type = std::pair<Key const, Mapped>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using hasher = Hash;
  using key_equal = KeyEqual;

  using reference = value_type &;
  using const_reference = value_type const &;
  using pointer = value_type *;
  using const_pointer = value_type const *;

  template <typename T> class sentinel {};

  template <typename T>
    requires(std::is_same_v<T, member> || std::is_same_v<T, member const>)
  class iterator_type {
    friend iterator_type<std::remove_const_t<T>>;
    friend iterator_type<T const>;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = iumap::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<std::is_const_v<T>, value_type const *, value_type *>;
    using reference = std::conditional_t<std::is_const_v<T>, value_type const &, value_type &>;

    using container_type =
        std::conditional_t<std::is_const_v<T>, std::array<member, Size> const, std::array<member, Size>>;

    constexpr iterator_type(T *const slot, container_type *const container) noexcept
        : slot_{slot}, container_{container} {
      this->move_forward_to_occupied();
    }

    constexpr bool operator==(iterator_type<std::remove_const_t<T>> const &other) const noexcept {
      return std::make_pair(slot_, container_) == std::make_pair(other.slot_, other.container_);
    }
    constexpr bool operator==(iterator_type<T const> const &other) const noexcept {
      return std::make_pair(slot_, container_) == std::make_pair(other.slot_, other.container_);
    }
    constexpr bool operator<=>(iterator_type<T const> const &other) const noexcept {
      return std::make_pair(slot_, container_) <=> std::make_pair(other.slot_, other.container_);
    }
    constexpr auto operator==(sentinel<T>) const noexcept { return slot_ == this->end_limit(); }
    constexpr auto operator!=(sentinel<T> other) const noexcept { return !operator==(other); }

    constexpr reference operator*() const noexcept { return *slot_->pointer(); }
    constexpr pointer operator->() const noexcept { return slot_->pointer(); }

    constexpr iterator_type &operator--() noexcept {
      --slot_;
      this->move_backward_to_occupied();
      return *this;
    }
    constexpr iterator_type &operator++() noexcept {
      ++slot_;
      this->move_forward_to_occupied();
      return *this;
    }
    constexpr iterator_type operator++(int) noexcept {
      auto const tmp = *this;
      ++*this;
      return tmp;
    }
    constexpr iterator_type operator--(int) noexcept {
      auto const tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator_type &operator+=(difference_type n) noexcept {
      if (n < 0) {
        return decrement_n(-n);
      }
      return increment_n(n);
    }
    constexpr iterator_type &operator-=(difference_type n) noexcept {
      if (n < 0) {
        return increment_n(-n);
      }
      return decrement_n(n);
    }

    friend iterator_type operator+(iterator_type it, difference_type n) noexcept {
      auto result = it;
      result += n;
      return result;
    }
    friend iterator_type operator+(difference_type n, iterator_type it) noexcept { return it + n; }
    friend iterator_type operator-(iterator_type it, difference_type n) noexcept {
      auto result = it;
      result -= n;
      return result;
    }
    friend iterator_type operator-(difference_type n, iterator_type it) noexcept { return it - n; }

    T *raw() noexcept { return slot_; }

  private:
    constexpr auto end_limit() noexcept { return container_->data() + container_->size(); }
    constexpr void move_backward_to_occupied() noexcept {
      auto const *const limit = container_->data();
      while (slot_ >= limit && slot_->state != state::occupied) {
        --slot_;
      }
    }
    constexpr void move_forward_to_occupied() noexcept {
      auto const *const end = this->end_limit();
      while (slot_ < end && slot_->state != state::occupied) {
        ++slot_;
      }
    }
    constexpr iterator_type &increment_n(difference_type n) noexcept {
      assert(n >= 0);
      for (; n > 0; --n) {
        ++(*this);
      }
      return *this;
    }
    constexpr iterator_type &decrement_n(difference_type n) noexcept {
      assert(n >= 0);
      for (; n > 0; --n) {
        --(*this);
      }
      return *this;
    }

    T *slot_;
    container_type *container_;
  };

  using iterator = iterator_type<member>;
  using const_iterator = iterator_type<member const>;

  constexpr iumap() noexcept = default;
  iumap(iumap const &other) = default;
  iumap(iumap &&other) noexcept = default;
  ~iumap() noexcept = default;

  iumap &operator=(iumap const &other) = default;
  iumap &operator=(iumap &&other) noexcept = default;

  // Iterators
  [[nodiscard]] constexpr auto begin() noexcept { return iterator{v_.data(), &v_}; }
  [[nodiscard]] constexpr auto begin() const noexcept { return const_iterator{v_.data(), &v_}; }

  [[nodiscard]] constexpr auto end() noexcept { return iterator{v_.data() + v_.size(), &v_}; }
  [[nodiscard]] constexpr auto end() const noexcept { return const_iterator{v_.data() + v_.size(), &v_}; }

  // Capacity
  [[nodiscard]] constexpr auto empty() const noexcept { return size_ == 0; }
  [[nodiscard]] constexpr auto size() const noexcept { return size_; }
  [[nodiscard]] constexpr auto max_size() const noexcept { return v_.max_size(); }
  [[nodiscard]] constexpr auto capacity() const noexcept { return Size; }

  // Modifiers
  void clear() noexcept;
  /// inserts elements
  std::pair<iterator, bool> insert(value_type const &value);
  /// inserts an element or assigns to the current element if the key already exists
  template <typename M> std::pair<iterator, bool> insert_or_assign(Key const &key, M &&value);
  /// inserts in-place if the key does not exist, does nothing if the key exists
  template <typename... Args> std::pair<iterator, bool> try_emplace(Key const &key, Args &&...args);
  /// erases elements
  iterator erase(iterator pos);

  // Lookup
  [[nodiscard]] iterator find(Key const &k);
  [[nodiscard]] const_iterator find(Key const &k) const;

  // Observers
  [[nodiscard]] hasher hash_function() const { return Hash{}; }
  [[nodiscard]] key_equal key_eq() const { return KeyEqual{}; }

#ifdef IUMAP_TRACE
  void dump(std::ostream &os) const {
    std::cout << "size=" << size_ << '\n';
    for (auto index = std::size_t{0}; auto const &slot : v_) {
      os << '[' << index << "] ";
      ++index;
      switch (slot.state) {
      case state::unused: os << '*'; break;
      case state::tombstone: os << "\xF0\x9F\xAA\xA6"; break;  // UTF-8 U+1fAA6 tomestone
      case state::occupied: {
        auto const *const kvp = slot.cast();
        os << "> " << kvp->first << '=' << kvp->second;
      } break;
      }
      os << '\n';
    }
  }
#endif  // IUMAP_TRACE

private:
  enum class state : std::uint8_t { occupied, tombstone, unused };
  struct member {
    member() noexcept = default;
    member(member const &other) noexcept(std::is_nothrow_copy_constructible_v<value_type>);
    member(member &&other) noexcept(std::is_nothrow_move_constructible_v<value_type>);
    ~member() noexcept { this->destroy(); }
    member &operator=(member const &other) noexcept(std::is_nothrow_copy_constructible_v<value_type>);
    member &operator=(member &&other) noexcept(std::is_nothrow_move_constructible_v<value_type>);

    void destroy() noexcept;

    [[nodiscard]] constexpr value_type *pointer() noexcept { return std::bit_cast<value_type *>(&storage[0]); }
    [[nodiscard]] constexpr value_type const *pointer() const noexcept {
      return std::bit_cast<value_type const *>(&storage[0]);
    }
    [[nodiscard]] constexpr value_type &reference() noexcept { return *this->pointer(); }
    [[nodiscard]] constexpr value_type const &reference() const noexcept { return *this->pointer(); }

    enum state state = state::unused;
    alignas(value_type) std::byte storage[sizeof(value_type)]{};
  };
  std::size_t size_ = 0;
  std::size_t tombstones_ = 0;
  std::array<member, Size> v_{};

  template <typename Container> static auto *lookup_slot(Container &container, Key const &key);
  template <typename Container> static auto *find_insert_slot(Container &container, Key const &key);
};

// ctor
// ~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
iumap<Key, Mapped, Size, Hash, KeyEqual>::member::member(member const &other) noexcept(
    std::is_nothrow_copy_constructible_v<value_type>)
    : state{other.state} {
  if (state == state::occupied) {
    new (this->pointer()) value_type(*other.pointer());
  }
}
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
iumap<Key, Mapped, Size, Hash, KeyEqual>::member::member(member &&other) noexcept(
    std::is_nothrow_move_constructible_v<value_type>)
    : state{other.state} {
  if (state == state::occupied) {
    new (this->pointer()) value_type(std::move(*other.pointer()));
  }
}

// operator=
// ~~~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::member::operator=(member const &other) noexcept(
    std::is_nothrow_copy_constructible_v<value_type>) -> member & {
  if (this == &other) {
    return *this;
  }
  if (this->state == state::occupied) {
    this->destroy();
  }
  if (other.state == state::occupied) {
    new (this->pointer()) value_type(other.reference());
  }
  state = other.state;
  return *this;
}

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::member::operator=(member &&other) noexcept(
    std::is_nothrow_move_constructible_v<value_type>) -> member & {
  if (this == &other) {
    return *this;
  }
  if (this->state == state::occupied) {
    this->destroy();
  }
  if (other.state == state::occupied) {
    new (pointer()) value_type(std::move(other.reference()));
  }
  state = other.state;
  return *this;
}

// destroy
// ~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
void iumap<Key, Mapped, Size, Hash, KeyEqual>::member::destroy() noexcept {
  if (state == state::occupied) {
    pointer()->~value_type();
  }
  state = state::unused;
}

// clear
// ~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
void iumap<Key, Mapped, Size, Hash, KeyEqual>::clear() noexcept {
  for (auto &entry : v_) {
    entry.destroy();
  }
  size_ = 0;
  tombstones_ = 0;
}

// try emplace
// ~~~~~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
template <typename... Args>
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::try_emplace(Key const &key, Args &&...args)
    -> std::pair<iterator, bool> {
  auto *const slot = iumap::find_insert_slot(*this, key);
  if (slot == nullptr) {
    // The map is full and the key was not found. Insertion failed.
    return std::make_pair(this->end(), false);
  }
  auto const do_insert = slot->state != state::occupied;
  if (do_insert) {
    // Not found. Add a new key/value pair.
    new (slot->pointer()) value_type(key, std::forward<Args>(args)...);
    ++size_;
    if (slot->state == state::tombstone) {
      --tombstones_;
    }
    slot->state = state::occupied;
  }
  return std::make_pair(iterator{slot, &v_}, do_insert);
}

// insert
// ~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::insert(value_type const &value) -> std::pair<iterator, bool> {
  return try_emplace(value.first, value.second);
}

// insert or assign
// ~~~~~~~~~~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
template <typename M>
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::insert_or_assign(Key const &key, M &&value)
    -> std::pair<iterator, bool> {
  auto *const slot = iumap::find_insert_slot(*this, key);
  if (slot == nullptr) {
    // The map is full and the key was not found. Insertion failed.
    return std::make_pair(this->end(), false);
  }
  if (slot->state == state::unused || slot->state == state::tombstone) {
    // Not found. Add a new key/value pair.
    new (slot->storage) value_type(key, std::forward<M>(value));
    ++size_;
    if (slot->state == state::tombstone) {
      --tombstones_;
    }
    slot->state = state::occupied;
    return std::make_pair(iterator{slot, &v_}, true);
  }
  slot->pointer()->second = value;  // Overwrite the existing value.
  return std::make_pair(iterator{slot, &v_}, false);
}

// find
// ~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::find(Key const &k) const -> const_iterator {
  auto *const slot = iumap::lookup_slot(*this, k);
  if (slot == nullptr || slot->state != state::occupied) {
    return this->end();  // Not found
  }
  return {slot, &v_};  // Found
}

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::find(Key const &k) -> iterator {
  auto *const slot = iumap::lookup_slot(*this, k);
  if (slot == nullptr || slot->state != state::occupied) {
    return this->end();  // Not found
  }
  return {slot, &v_};  // Found
}

// erase
// ~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::erase(iterator pos) -> iterator {
  member *const slot = pos.raw();
  auto const result = pos + 1;
  if (slot->state == state::occupied) {
    assert(size_ > 0);
    slot->destroy();
    slot->state = state::tombstone;
    --size_;
    ++tombstones_;
    if (this->empty()) {
      this->clear();
    }
  }
  return result;
}

/// Searches the container for a specified key. Stops when the key is found or an unused slot is probed.
/// Tombstones are ignored.
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
template <typename Container>
auto *iumap<Key, Mapped, Size, Hash, KeyEqual>::lookup_slot(Container &container, Key const &key) {
  using slot_type = std::remove_pointer_t<decltype(container.v_.data())>;
  auto const size = container.v_.size();
  auto const equal = KeyEqual{};

  auto pos = Hash{}(key) % size;
  for (auto iteration = 1U; iteration <= size; ++iteration) {
    switch (slot_type *const slot = &container.v_[pos]; slot->state) {
    case state::unused: return slot;
    case state::tombstone:
      // Keep searching.
      break;
    case state::occupied:
      if (equal(slot->pointer()->first, key)) {
        return slot;
      }
      break;
    default: assert(false && "Slot is in an invalid state"); break;
    }
    pos = (pos + iteration) % size;
  }
  using slot_ptr = slot_type *;
  return slot_ptr{nullptr};  // No available slot.
}

/// Searches the container for a key or a potential insertion position for that key. It stops when either the
/// key or an unused slot are found. If tombstones are encountered, then returns the first tombstone slot
/// so that when inserted, the key's probing distance is as short as possible.
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size) && std::is_nothrow_destructible_v<Key> && std::is_nothrow_destructible_v<Mapped>)
template <typename Container>
auto *iumap<Key, Mapped, Size, Hash, KeyEqual>::find_insert_slot(Container &container, Key const &key) {
  using slot_type = std::remove_pointer_t<decltype(container.v_.data())>;
  auto const size = container.v_.size();
  auto const equal = KeyEqual{};

  auto pos = Hash{}(key) % size;  // The probing position.
  slot_type *first_tombstone = nullptr;
  for (auto iteration = 1U; iteration <= size; ++iteration) {
    switch (slot_type *const slot = &container.v_[pos]; slot->state) {
    case state::tombstone:
      if (first_tombstone == nullptr) {
        // Remember this tombstone's slot so it can be returned later.
        first_tombstone = slot;
      }
      break;
    case state::occupied:
      if (equal(slot->pointer()->first, key)) {
        return slot;
      }
      break;
    case state::unused: return first_tombstone != nullptr ? first_tombstone : slot;
    default: assert(false && "Slot is in an invalid state"); break;
    }
    // The next quadratic probing location
    pos = (pos + iteration) % size;
  }
  using slot_ptr = slot_type *;
  return first_tombstone != nullptr ? first_tombstone : slot_ptr{nullptr};
}

}  // end namespace midi2

#endif  // MIDI2_IUMAP_HPP
