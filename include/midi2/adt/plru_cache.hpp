//===-- Pseudo LRU Cache ------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file plru_cache.hpp
/// \brief Implements a "Tree-PLRU" (Pseudo Least-recently Used) unordered associative container.
///
/// It is intended as a small cache for objects which are relatively cheap to store and relatively
/// expensive to create. The container's keys must be unsigned integral types.

#ifndef MIDI2_ADT_PLRU_CACHE_HPP
#define MIDI2_ADT_PLRU_CACHE_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <new>
#include <numeric>
#include <ranges>

#if __ARM_NEON
#include <arm_neon.h>
#endif  // __ARM_NEON

#include "midi2/adt/uinteger.hpp"

namespace midi2::adt {

/// \brief Private implementation details of the plru_cache type
namespace details {

template <typename T> struct aligned_storage {
  [[nodiscard]] constexpr T &value() noexcept { return *std::bit_cast<T *>(&v[0]); }
  [[nodiscard]] constexpr T const &value() const noexcept { return *std::bit_cast<T const *>(&v[0]); }
  alignas(T) std::byte v[sizeof(T)];
};

template <std::size_t Ways> class tree {
public:
  /// Flip the access bits of the tree to indicate that \p way is the most recently used member.
  void touch(std::size_t const way) noexcept {
    assert(way < Ways && "Way index is too large");
    auto node = std::size_t{0};
    auto start = std::size_t{0};
    auto end = Ways;
    while (node < Ways - 1U) {
      auto const mid = std::midpoint(start, end);
      auto const is_less = way < mid;
      if (is_less) {
        end = mid;
      } else {
        start = mid;
      }
      bits_[node] = is_less;
      node = 2U * node + 1U + static_cast<unsigned>(!is_less);
    }
  }

  /// Traverses the tree to find the index of the oldest member.
  [[nodiscard]] constexpr std::size_t oldest() const noexcept {
    auto node = std::size_t{0};
    while (node < Ways - 1U) {
      node = 2U * node + 1U + static_cast<unsigned>(bits_[node]);
    }
    return node - (Ways - 1U);
  }

  /// Resets the access bits to their initial state.
  constexpr void reset() { bits_.reset(); }

private:
  std::bitset<Ways - 1U> bits_{};
};

/// Used to store keys in the cache.
///
/// We don't store the key directly in this struct: some of the bits are determined by the key's set so don't need to
/// be recorded here. We do need to record whether a key entry is occupied, so there's a bit used for that.
template <std::unsigned_integral Key, unsigned SetBits> class tagged_key {
public:
  using value_type = uinteger<sizeof(Key) * CHAR_BIT - SetBits + 1>::type;

  constexpr tagged_key() noexcept = default;
  constexpr explicit tagged_key(Key key) noexcept : v_{static_cast<value_type>(1 | (key >> (SetBits - 1)))} {}
  friend constexpr bool operator==(tagged_key const &, tagged_key const &) noexcept = default;
  [[nodiscard]] constexpr bool valid() const noexcept { return static_cast<bool>(v_ & 1); }

  [[nodiscard]] constexpr value_type &get() noexcept { return v_; }
  [[nodiscard]] constexpr value_type const &get() const noexcept { return v_; }

private:
  value_type v_ = 0;
};

template <std::unsigned_integral Key, unsigned SetBits, unsigned Ways> struct match_finder {
  using tagged_key_type = tagged_key<Key, SetBits>;

  /// \return The lane index [0..Ways) if a match is found, or Ways if no match
  static constexpr std::size_t find(tagged_key_type const tk,
                                    std::array<tagged_key_type, Ways> const &values) noexcept {
    auto index = std::size_t{0};
    for (; index < Ways; ++index) {
      if (values[index] == tk) {
        break;
      }
    }
    return index;
  }
};

#if __ARM_NEON
template <std::unsigned_integral Key, unsigned SetBits>
  requires(std::is_same_v<typename tagged_key<Key, SetBits>::value_type, std::uint16_t>)
struct match_finder<Key, SetBits, 4> {
  using tagged_key_type = tagged_key<Key, SetBits>;

  static constexpr std::size_t find(tagged_key_type const new_tag,
                                    std::array<tagged_key_type, 4> const &values) noexcept {
    uint16x4_t const a = vdup_n_u16(new_tag.get());
    uint16x4_t const b = vld1_u16(std::bit_cast<std::uint16_t const *>(values.data()));

    uint16x4_t const cmp = vceq_u16(a, b);  // Compare lanes: 0xFFFF if equal
    std::uint64_t const packed = vget_lane_u64(vreinterpret_u64_u16(cmp), 0);
    std::uint64_t const nz_or = packed | (static_cast<std::uint64_t>(packed == 0) << 63);
    return (static_cast<std::size_t>(static_cast<unsigned>(std::countr_zero(nz_or)) + 1U)) >> 3;
  }
};

template <std::unsigned_integral Key, unsigned SetBits>
  requires(std::is_same_v<typename tagged_key<Key, SetBits>::value_type, std::uint16_t>)
struct match_finder<Key, SetBits, 8> {
  using tagged_key_type = tagged_key<Key, SetBits>;

  static constexpr std::size_t find(tagged_key_type const new_tag,
                                    std::array<tagged_key_type, 8> const &values) noexcept {
    uint16x8_t const a = vdupq_n_u16(new_tag.get());
    uint16x8_t const b = vld1q_u16(std::bit_cast<std::uint16_t const *>(values.data()));

    uint16x8_t const cmp = vceqq_u16(a, b);   // Compare lanes: 0xFFFF if equal
    uint8x8_t const narrow = vmovn_u16(cmp);  // Narrow to 8-bit: 0xFF if equal, 0x00 if not
    std::uint64_t const packed =
        vget_lane_u64(vreinterpret_u64_u8(narrow), 0);  // Treat 8 lanes as a single 64-bit scalar

    // Extract the matching lane by counting trailing groups of 8 bits. If packed is zero, no lane matched.
    // For nonzero packed: ctz = 8*i (i = matching lane).
    // For zero packed:    ctz = 63  -> (ctz+1)>>3 = 8  (no match).
    std::uint64_t const nz_or = packed | (static_cast<std::uint64_t>(packed == 0) << 63);
    return (static_cast<std::size_t>(static_cast<unsigned>(std::countr_zero(nz_or)) + 1U)) >> 3;
  }
};

template <std::unsigned_integral Key, unsigned SetBits>
  requires(std::is_same_v<typename tagged_key<Key, SetBits>::value_type, std::uint32_t>)
struct match_finder<Key, SetBits, 4> {
  using tagged_key_type = tagged_key<Key, SetBits>;

  static constexpr std::size_t find(tagged_key_type const new_tag,
                                    std::array<tagged_key_type, 4> const &values) noexcept {
    uint32x4_t const a = vdupq_n_u32(new_tag.get());
    uint32x4_t const b = vld1q_u32(std::bit_cast<std::uint32_t const *>(values.data()));

    uint32x4_t const cmp = vceqq_u32(a, b);    // Compare lanes: 0xFFFF if equal
    uint16x4_t const narrow = vmovn_u32(cmp);  // Narrow to 8-bit: 0xFF if equal, 0x00 if not
    std::uint64_t const packed =
        vget_lane_u64(vreinterpret_u64_u16(narrow), 0);  // Treat 4b lanes as a single 64-bit scalar

    // Extract the matching lane by counting trailing groups of 8 bits. If packed is zero, no lane matched.
    // For nonzero packed: ctz = 8*i (i = matching lane).
    // For zero packed:    ctz = 63  -> (ctz+1)>>3 = 8  (no match).
    std::uint64_t const nz_or = packed | (static_cast<std::uint64_t>(packed == 0) << 63);
    return (static_cast<unsigned>(std::countr_zero(nz_or)) + 1U) / 16U;
  }
};

#endif  // __ARM_NEON

template <std::unsigned_integral Key, typename MappedType, unsigned SetBits, std::size_t Ways> class cache_set {
  using tagged_key_type = tagged_key<Key, SetBits>;

public:
  template <typename MissFn, typename ValidFn>
    requires(std::is_invocable_r_v<MappedType, MissFn> && std::is_invocable_r_v<bool, ValidFn, MappedType const &>)
  MappedType &access(Key key, MissFn miss, ValidFn valid) {
    auto const tag = tagged_key_type{key};
    if (auto const index = find_matching(tag); index < Ways) {
      assert(keys_[index].valid());
      auto &found_result = values_[index].value();
      if (!valid(found_result)) {
        found_result = miss();
      }
      plru_.touch(index);
      return found_result;
    }

    // Find the array member that is to be re-used by traversing the tree.
    std::size_t const victim = plru_.oldest();
    // The key was not found: call miss() to populate it.
    auto &result = values_[victim].value();
    if (keys_[victim].valid()) {
      result = miss();
    } else {
      std::construct_at(&result, miss());
    }
    keys_[victim] = std::move(tag);
    plru_.touch(victim);
    return result;
  }

  [[nodiscard]] constexpr bool contains(Key key) const noexcept { return find_matching(tagged_key_type{key}) < Ways; }
  constexpr void clear() noexcept {
    for (auto index = std::size_t{0}; index < Ways; ++index) {
      if (keys_[index].valid()) {
        std::destroy_at(&values_[index].value());
      }
      keys_[index] = tagged_key_type{};
    }
    plru_.reset();
  }
  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return static_cast<std::size_t>(std::ranges::count_if(keys_, [](tagged_key_type const &v) { return v.valid(); }));
  }

  [[nodiscard]] constexpr bool valid(std::size_t index) const noexcept {
    assert(index < Ways);
    return keys_[index].valid();
  }
  [[nodiscard]] constexpr Key key(std::size_t const index) const noexcept {
    assert(index < keys_.size());
    return (keys_[index].get() & ~0x01U) << (SetBits - 1);  // this not the entire key!
  }
  [[nodiscard]] constexpr MappedType const &value(std::size_t const index) const noexcept {
    assert(index < values_.size());
    return values_[index].value();
  }
  [[nodiscard]] constexpr MappedType &value(std::size_t const index) noexcept {
    assert(index < values_.size());
    return values_[index].value();
  }

private:
  /// Search the keys_ array for the key specified by \p tk.
  ///
  /// \param tk The key for which the function will search
  /// \returns  The index of the located key if present otherwise \p Ways.
  [[nodiscard]] constexpr std::size_t find_matching(tagged_key_type tk) const noexcept {
    return match_finder<Key, SetBits, Ways>::find(tk, keys_);
  }

  std::array<tagged_key_type, Ways> keys_{};
  std::array<aligned_storage<MappedType>, Ways> values_{};
  tree<Ways> plru_{};
};

}  // end namespace details

/// \brief A "Tree-PLRU" (Pseudo Least-recently Used) unordered associative container.
///
/// It is intended as a small cache for objects which are relatively cheap to store and
/// relatively expensive to create. The container's keys must be unsigned integral types.
///
/// The total number of cache entries is given by Sets * Ways.
///
/// \tparam Key  The key type.
/// \tparam Mapped  The value type.
/// \tparam Sets  The number of entries that share the same lookup key fragment or hash bucket
///   index. All entries in a set compete to be stored in that group. Must be a power of 2.
/// \tparam Ways  The number of slots within a set that can hold a single entry. The number of
///   ways in a set determines how many entries with the same key fragment or bucket index can
///   coexist. Must be a power of 2.
template <std::unsigned_integral Key, typename Mapped, std::size_t Sets, std::size_t Ways>
  requires(std::popcount(Sets) == 1 && std::popcount(Ways) == 1)
class plru_cache {
public:
  using key_type = Key;
  using mapped_type = Mapped;
  using value_type = std::pair<Key const, Mapped>;

  static constexpr std::size_t const sets = Sets;
  static constexpr std::size_t const ways = Ways;

  /// A proxy type that pretends to be type pair<const Key, T>&
  template <typename T>
    requires(std::is_same_v<T, mapped_type> || std::is_same_v<T, mapped_type const>)
  struct proxy {
    Key const first;
    T &second;
    constexpr operator std::pair<Key const, std::remove_const_t<T>>() const noexcept { return {first, second}; }
    constexpr bool operator==(proxy const &) const noexcept = default;
    constexpr bool operator==(std::pair<Key const, std::remove_const_t<T>> const &other) const noexcept {
      return first == other.first && second == other.second;
    }
  };

  template <typename T>
    requires(std::is_same_v<T, mapped_type> || std::is_same_v<T, mapped_type const>)
  class iterator_type {
  public:
    using owner_type = std::conditional_t<std::is_const_v<T>, plru_cache const, plru_cache>;

    using value_type = std::pair<Key const, T>;
    using reference = proxy<T>;
    using pointer = proxy<T> *;
    using difference_type = std::ptrdiff_t;

    using iterator_category = std::forward_iterator_tag;

    constexpr explicit iterator_type(owner_type *const owner) noexcept : owner_{owner} {}
    constexpr iterator_type(owner_type *const owner, std::pair<std::size_t, std::size_t> const &indexes) noexcept
        : owner_{owner}, set_index_{indexes.first}, way_index_{indexes.second} {}
    constexpr bool operator==(iterator_type const &other) const noexcept = default;

    constexpr reference operator*() const
      requires(std::is_const_v<T>)
    {
      auto const &s = owner_->sets_[set_index_];
      return proxy<T>{static_cast<Key>(s.key(way_index_) | set_index_), s.value(way_index_)};
    }
    constexpr reference operator*()
      requires(!std::is_const_v<T>)
    {
      auto &s = owner_->sets_[set_index_];
      return proxy<Mapped>{static_cast<Key>(s.key(way_index_) | set_index_), s.value(way_index_)};
    }
    constexpr iterator_type &operator++() {
      do {
        ++way_index_;
        if (way_index_ >= plru_cache::ways) {
          ++set_index_;
          if (set_index_ >= plru_cache::sets) {
            break;
          }
          way_index_ = 0;
        }
      } while (!this->valid());
      assert(((way_index_ == plru_cache::ways && set_index_ == plru_cache::sets) || this->valid()) &&
             R"(post-condition says that we reference a valid object or "end")");
      return *this;
    }

  private:
    owner_type *owner_ = nullptr;
    std::size_t set_index_ = sets;
    std::size_t way_index_ = ways;

    [[nodiscard]] constexpr bool valid() const { return owner_->sets_[set_index_].valid(way_index_); }
  };

  template <typename T> iterator_type(T *) -> iterator_type<typename T::mapped_type>;
  template <typename T> iterator_type(T const *) -> iterator_type<typename T::mapped_type const>;

  using iterator = iterator_type<Mapped>;
  using const_iterator = iterator_type<Mapped const>;

  constexpr plru_cache() noexcept = default;
  // (note that the following member functions aren't provided simply because I don't currently
  // have a need for them.)
  plru_cache(plru_cache const &) = delete;
  plru_cache(plru_cache &&) noexcept = delete;
  ~plru_cache() noexcept { clear(); }
  plru_cache &operator=(plru_cache const &) noexcept = delete;
  plru_cache &operator=(plru_cache &&) noexcept = delete;

  bool operator==(plru_cache const &) const = delete;

  /// Searches the cache for the \p key and returns a reference to it if present. If not present
  /// and the cache is fully occupied, the likely least recently used value is evicted from the
  /// cache then the \p miss function is called to instantiate the mapped type associated with
  /// \p key. This value is then stored in the cache.
  ///
  /// \tparam MissFn  The type of the function called to instantiate a value in the cache.
  /// \param key  Key value of the element to search for.
  /// \param miss  The function that will be called to instantiate the associated value if \p key
  ///   is not present in the cache. The function must be compatible with the signature T().
  /// \returns The cached value.
  template <typename MissFn>
    requires(std::is_invocable_r_v<mapped_type, MissFn>)
  mapped_type &access(key_type key, MissFn miss) {
    assert(plru_cache::set(key) < Sets);
    return sets_[plru_cache::set(key)].access(key, std::move(miss),
                                              [](mapped_type const &) constexpr noexcept { return true; });
  }

  /// \tparam MissFn  The type of the function called to instantiate a value in the cache.
  /// \tparam ValidFn  The type of the function called to check the validity of a value in the cache.
  /// \param key  Key value of the element to search for.
  /// \param miss  The function that will be called to instantiate the associated value if \p key
  ///   is not present in the cache. The function must be compatible with the signature T().
  /// \param valid  The function that will be called to determine whether the cached value remains
  ///   valid. It must be compatible with the signature bool(T).
  /// \returns The cached value.
  template <typename MissFn, typename ValidFn>
    requires(std::is_invocable_r_v<mapped_type, MissFn> && std::is_invocable_r_v<bool, ValidFn, mapped_type const &>)
  mapped_type &access(key_type key, MissFn miss, ValidFn valid) {
    assert(plru_cache::set(key) < Sets);
    return sets_[plru_cache::set(key)].access(key, std::move(miss), std::move(valid));
  }

  /// Checks if there is an element with a key that compares equivalent to \p key
  ///
  /// \return true if there is such an element, otherwise false.
  [[nodiscard]] constexpr bool contains(key_type key) const noexcept {
    assert(plru_cache::set(key) < Sets);
    return sets_[plru_cache::set(key)].contains(key);
  }

  /// \brief Clears the contents of the cache.
  constexpr void clear() noexcept {
    for (ways_type &w : sets_) {
      w.clear();
    }
  }

  ///@{
  /// Returns an iterator to the beginning
  [[nodiscard]] constexpr iterator begin() noexcept { return {this, this->first_valid()}; }
  [[nodiscard]] constexpr const_iterator begin() const noexcept { return {this, this->first_valid()}; }
  [[nodiscard]] constexpr const_iterator cbegin() noexcept { return {this, this->first_valid()}; }
  ///@}

  ///@{
  /// Returns an iterator to the end
  [[nodiscard]] constexpr iterator end() noexcept { return iterator_type{this}; }
  [[nodiscard]] constexpr const_iterator end() const noexcept { return iterator_type{this}; }
  [[nodiscard]] constexpr const_iterator cend() noexcept { return iterator_type{this}; }
  ///@}

  /// \returns The maximum possible number of elements that can be held by the cache.
  [[nodiscard]] constexpr std::size_t max_size() const noexcept { return Sets * Ways; }
  /// \returns The number of elements held by the cache.
  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return std::ranges::fold_left(sets_, std::size_t{0},
                                  [](std::size_t acc, auto const &set) { return acc + std::size(set); });
  }

  /// \param key  The key
  /// \returns The set number of a supplied key
  [[nodiscard]] static constexpr std::size_t set(key_type key) noexcept { return key & (Sets - 1U); }
  /// \param key  The key
  /// \returns The way number of a supplied key
  [[nodiscard]] static constexpr std::size_t way(key_type key) noexcept { return (key >> set_bits_) & (Ways - 1U); }

private:
  static constexpr unsigned set_bits_ = std::bit_width(Sets - 1U);
  using ways_type = details::cache_set<key_type, mapped_type, set_bits_, ways>;
  std::array<ways_type, Sets> sets_{};

  [[nodiscard]] constexpr std::pair<std::size_t, std::size_t> first_valid() const {
    // Search for the first in-use slot.
    auto set = std::size_t{0};
    auto way = std::size_t{0};
    while (!sets_[set].valid(way)) {
      ++way;
      if (way >= plru_cache::ways) {
        ++set;
        if (set >= plru_cache::sets) {
          break;
        }
        way = 0;
      }
    }
    return {set, way};
  }
};

}  // end namespace midi2::adt

#endif  // MIDI2_ADT_PLRU_CACHE_HPP
