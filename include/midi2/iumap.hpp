//===-- In-place Unordered Map ------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_IUMAP_HPP
#define MIDI2_IUMAP_HPP

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

/// \tparam UInt An unsigned integer type.
/// \param n An integer value to check whether it is a power of two.
/// \returns True if the input value is a power of 2.
template <std::unsigned_integral UInt> constexpr bool is_power_of_two(UInt n) noexcept {
  //  if a number n is a power of 2 then bitwise & of n and n-1 will be zero.
  return n > 0U && !(n & (n - 1U));
}

// An in-place unordered hash table.
template <typename Key, typename Mapped, std::size_t Size, typename Hash = std::hash<std::remove_cv_t<Key>>,
          typename KeyEqual = std::equal_to<Key>>
  requires(is_power_of_two(Size))
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
      return slot_ == other.slot_ && container_ == other.container_;
    }
    constexpr bool operator==(iterator_type<T const> const &other) const noexcept {
      return slot_ == other.slot_ && container_ == other.container_;
    }
    constexpr bool operator!=(iterator_type<std::remove_const_t<T>> const &other) const noexcept {
      return !operator==(other);
    }
    constexpr bool operator!=(iterator_type<T const> const &other) const noexcept { return !operator==(other); }

    constexpr bool operator<=>(iterator_type<T const> const &other) const noexcept {
      return std::make_pair(slot_, container_) <=> std::make_pair(other.slot_, other.container_);
    }

    constexpr auto operator==(sentinel<T>) const noexcept { return slot_ == this->end_limit(); }
    constexpr auto operator!=(sentinel<T> other) const noexcept { return !operator==(other); }

    constexpr reference operator*() const noexcept { return static_cast<reference>(*slot_); }
    constexpr pointer operator->() const noexcept { return &static_cast<reference>(*slot_); }

    iterator_type &operator--() {
      --slot_;
      this->move_backward_to_occupied();
      return *this;
    }
    iterator_type &operator++() {
      ++slot_;
      this->move_forward_to_occupied();
      return *this;
    }
    iterator_type operator++(int) {
      auto const tmp = *this;
      ++*this;
      return tmp;
    }
    iterator_type operator--(int) {
      auto const tmp = *this;
      --*this;
      return tmp;
    }

    iterator_type &operator+=(difference_type n) {
      if (n < 0) {
        *this = *this - (-n);
      } else {
        for (; n > 0; --n) {
          ++(*this);
        }
      }
      return *this;
    }

    iterator_type &operator-=(difference_type n) {
      if (n < 0) {
        *this = *this + (-n);
      } else {
        for (; n > 0; --n) {
          --(*this);
        }
      }
      return *this;
    }

    friend iterator_type operator+(iterator_type it, difference_type n) {
      auto result = it;
      result += n;
      return result;
    }
    friend iterator_type operator+(difference_type n, iterator_type it) { return it + n; }
    friend iterator_type operator-(iterator_type it, difference_type n) {
      auto result = it;
      result -= n;
      return result;
    }
    friend iterator_type operator-(difference_type n, iterator_type it) { return it - n; }

    T *raw() { return slot_; }

  private:
    constexpr auto end_limit() { return container_->data() + container_->size(); }
    constexpr void move_backward_to_occupied() {
      auto const *const limit = container_->data();
      while (slot_ >= limit && slot_->state != state::occupied) {
        --slot_;
      }
    }
    constexpr void move_forward_to_occupied() {
      auto const *const end = this->end_limit();
      while (slot_ < end && slot_->state != state::occupied) {
        ++slot_;
      }
    }

    T *slot_;
    container_type *container_;
  };

  using iterator = iterator_type<member>;
  using const_iterator = iterator_type<member const>;

  constexpr iumap() noexcept = default;
  iumap(iumap const &other);
  iumap(iumap &&other) noexcept;
  ~iumap() noexcept;

  iumap &operator=(iumap const &other);
  iumap &operator=(iumap &&other) noexcept;

  // Iterators
  constexpr auto begin() { return iterator{v_.data(), &v_}; }
  constexpr auto begin() const { return const_iterator{v_.data(), &v_}; }

  constexpr auto end() noexcept { return iterator{v_.data() + v_.size(), &v_}; }
  constexpr auto end() const noexcept { return const_iterator{v_.data() + v_.size(), &v_}; }

  // Capacity
  constexpr auto empty() const noexcept { return size_ == 0; }
  constexpr auto size() const noexcept { return size_; }
  constexpr auto max_size() const noexcept { return v_.max_size(); }
  constexpr auto capacity() const noexcept { return Size; }

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
  iterator find(Key const &k);
  const_iterator find(Key const &k) const;

  // Observers
  hasher hash_function() const { return Hash{}; }
  key_equal key_eq() const { return KeyEqual{}; }

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
  enum class state { occupied, tombstone, unused };
  struct member {
    [[nodiscard]] constexpr explicit operator value_type &() noexcept {
      assert(state == state::occupied);
      return *std::bit_cast<value_type *>(&storage[0]);
    }
    [[nodiscard]] constexpr explicit operator value_type const &() const noexcept {
      assert(state == state::occupied);
      return *std::bit_cast<value_type const *>(&storage[0]);
    }

    enum state state = state::unused;
    alignas(value_type) std::byte storage[sizeof(value_type)]{};
  };
  std::size_t size_ = 0;
  std::size_t tombstones_ = 0;
  std::array<member, Size> v_{};

  template <typename OtherMap> iumap &ctor(OtherMap &&other);
  template <typename OtherMap> iumap &assign(OtherMap &&other);

  template <typename Container> static auto *lookup_slot(Container &container, Key const &key);
  template <typename Container> static auto *find_insert_slot(Container &container, Key const &key);
};

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
iumap<Key, Mapped, Size, Hash, KeyEqual>::~iumap() noexcept {
  this->clear();
}

// ctor
// ~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
template <typename OtherMap>
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::ctor(OtherMap &&other) -> iumap & {
  member const *in = other.v_.data();
  for (member &entry : v_) {
    switch (in->state) {
    case state::occupied:
      if constexpr (std::is_rvalue_reference_v<OtherMap>) {
        new (entry.storage) value_type{std::move(static_cast<value_type &>(*in))};
      } else {
        new (entry.storage) value_type{static_cast<value_type const &>(*in)};
      }
      ++size_;
      break;
    case state::tombstone: ++tombstones_; break;
    case state::unused: break;
    }
    entry.state = in->state;
    ++in;
  }
  assert(size_ == other.size_);
  assert(tombstones_ == other.tombstones_);
  return *this;
}

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
iumap<Key, Mapped, Size, Hash, KeyEqual>::iumap(iumap const &other) {
  this->ctor(other);
}

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
iumap<Key, Mapped, Size, Hash, KeyEqual>::iumap(iumap &&other) noexcept {
  this->ctor(std::move(other));
}

// operator=
// ~~~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
template <typename OtherMap>
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::assign(OtherMap &&other) -> iumap & {
  if (this == &other) {
    return *this;
  }
  auto *in = other.v_.data();
  for (member &entry : v_) {
    switch (in->state) {
    case state::occupied:
      if (entry.state == state::occupied) {
        static_cast<value_type &>(entry).~value_type();
        --size_;
        entry.state = state::unused;
      }
      if constexpr (std::is_rvalue_reference_v<decltype(other)>) {
        new (entry.storage) value_type{std::move(static_cast<value_type &>(*in))};
      } else {
        new (entry.storage) value_type{static_cast<value_type const &>(*in)};
      }
      if (entry.state == state::tombstone) {
        --tombstones_;
      }
      ++size_;
      break;

    case state::tombstone:
      if (entry.state == state::occupied) {
        static_cast<value_type &>(entry).~value_type();
        --size_;
      }
      if (entry.state != state::tombstone) {
        ++tombstones_;
      }
      break;

    case state::unused:
      switch (entry.state) {
      case state::occupied:
        static_cast<value_type &>(entry).~value_type();
        --size_;
        break;
      case state::tombstone: --tombstones_; break;
      case state::unused: break;
      }
      break;
    }
    entry.state = in->state;
    ++in;
  }
  assert(size_ == other.size_);
  assert(tombstones_ == other.tombstones_);
  return *this;
}

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::operator=(iumap const &other) -> iumap & {
  this->assign(other);
  return *this;
}

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::operator=(iumap &&other) noexcept -> iumap & {
  this->assign(std::move(other));
  return *this;
}

// clear
// ~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
void iumap<Key, Mapped, Size, Hash, KeyEqual>::clear() noexcept {
  for (auto &entry : v_) {
    if (entry.state == state::occupied) {
      (void)static_cast<reference>(entry).~value_type();
    }
    entry.state = state::unused;
  }
  size_ = 0;
  tombstones_ = 0;
}

// try emplace
// ~~~~~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
template <typename... Args>
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::try_emplace(Key const &key, Args &&...args)
    -> std::pair<iterator, bool> {
  auto *const slot = iumap::find_insert_slot(*this, key);
  if (slot == nullptr) {
    // The map is full and the key was not found. Insertion failed.
    return std::make_pair(this->end(), false);
  }
  auto const do_insert = slot->state == state::unused || slot->state == state::tombstone;
  if (do_insert) {
    // Not found. Add a new key/value pair.
    new (slot->storage) value_type(key, std::forward<Args>(args)...);
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
  requires(is_power_of_two(Size))
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::insert(value_type const &value) -> std::pair<iterator, bool> {
  return try_emplace(value.first, value.second);
}

// insert or assign
// ~~~~~~~~~~~~~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
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
  static_cast<reference>(*slot).second = value;  // Overwrite the existing value.
  return std::make_pair(iterator{slot, &v_}, false);
}

// find
// ~~~~
template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::find(Key const &k) const -> const_iterator {
  auto *const slot = iumap::lookup_slot(*this, k);
  if (slot == nullptr || slot->state != state::occupied) {
    return this->end();  // Not found
  }
  return {slot, &v_};  // Found
}

template <typename Key, typename Mapped, std::size_t Size, typename Hash, typename KeyEqual>
  requires(is_power_of_two(Size))
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
  requires(is_power_of_two(Size))
auto iumap<Key, Mapped, Size, Hash, KeyEqual>::erase(iterator pos) -> iterator {
  member *const slot = pos.raw();
  auto const result = pos + 1;
  if (slot->state == state::occupied) {
    assert(size_ > 0);
    (void)static_cast<reference>(*slot).~value_type();
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
  requires(is_power_of_two(Size))
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
      if (equal(static_cast<const_reference>(*slot).first, key)) {
        return slot;
      }
      break;
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
  requires(is_power_of_two(Size))
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
      if (equal(static_cast<reference>(*slot).first, key)) {
        return slot;
      }
      break;
    case state::unused: return first_tombstone != nullptr ? first_tombstone : slot;
    }
    // The next quadratic probing location
    pos = (pos + iteration) % size;
  }
  using slot_ptr = slot_type *;
  return first_tombstone != nullptr ? first_tombstone : slot_ptr{nullptr};
}

}  // end namespace midi2

#endif  // MIDI2_IUMAP_HPP
