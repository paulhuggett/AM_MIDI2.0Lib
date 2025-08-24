//===-- Pseudo LRU Cache ------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// Implements a "Tree-PLRU" (Pseudo Least-recently Used) unordered associative container. It is
/// intended as a small cache for objects which are relatively cheap to store and relatively expensive
/// to create. The container's keys must be unsigned integral types.

#ifndef MIDI2_PLRU_CACHE_HPP
#define MIDI2_PLRU_CACHE_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <climits>
#include <cstddef>
#include <functional>
#include <new>
#include <numeric>

namespace midi2 {

namespace details {

/// \brief Yields the smallest unsigned integral type with at least \p N bits.
template <std::size_t N>
  requires(N <= 64)
struct uinteger {
  /// The type of an unsigned integral with at least \p N bits.
  using type = typename uinteger<N + 1>::type;
};
/// \brief A helper type for convenient use of uinteger<N>::type.
template <std::size_t N> using uinteger_t = typename uinteger<N>::type;
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

private:
  std::bitset<Ways - 1U> bits_{};
};

template <unsigned SetBits, typename Key, typename MappedType, std::size_t Ways> class cache_set {
public:
  template <typename MissFn>
    requires(std::is_invocable_r_v<MappedType, MissFn>)
  MappedType &access(Key key, MissFn miss) {
    auto const new_tag = tag_and_valid{key};
    // Linear search. The "Ways" argument should be small enough that tags_ fits within a cache line or two.
    for (auto ctr = std::size_t{0}; ctr < Ways; ++ctr) {
      if (values_[ctr] == new_tag) {
        plru_.touch(ctr);
        return ways_[ctr].value();
      }
    }

    // Find the array member that is to be re-used by traversing the tree.
    std::size_t const victim = plru_.oldest();
    // If this slot is occupied, evict its contents
    if (values_[victim].valid()) {
      ways_[victim].value().~MappedType();
    }

    // The key was not found: call miss() to populate it.
    auto *const result = new (&ways_[victim].v[0]) MappedType{miss()};
    values_[victim] = std::move(new_tag);
    plru_.touch(victim);
    return *result;
  }

  constexpr auto size() const noexcept {
    return static_cast<std::size_t>(std::ranges::count_if(values_, [](tag_and_valid const &v) { return v.valid(); }));
  }

private:
  class tag_and_valid {
  public:
    constexpr tag_and_valid() noexcept = default;
    constexpr explicit tag_and_valid(Key key) noexcept : v_{1 | static_cast<decltype(v_)>(key >> (SetBits - 1))} {}
    friend constexpr bool operator==(tag_and_valid const &, tag_and_valid const &) noexcept = default;
    [[nodiscard]] constexpr auto valid() const noexcept { return static_cast<bool>(v_ & 1); }

  private:
    static constexpr auto KeyBits = sizeof(Key) * CHAR_BIT;
    uinteger<KeyBits - SetBits + 1>::type v_ = 0;
  };

  std::array<aligned_storage<MappedType>, Ways> ways_;
  std::array<tag_and_valid, Ways> values_{};
  tree<Ways> plru_{};
};

}  // end namespace details

/// A "Tree-PLRU" (Pseudo Least-recently Used) unordered associative container. It is
/// intended as a small cache for objects which are relatively cheap to store and relatively
/// expensive to create. The container's keys must be unsigned integral types.
///
/// The total number of cache entries is given by Sets * Ways.
///
/// \tparam Sets  The number of entries that share the same lookup key fragment or hash bucket
///   index. All entries in a set compete to be stored in that group.
/// \tparam Ways  The number of slots within a set that can hold a single entry. The number of
///   ways in a set determines how many entries with the same key fragment or bucket index can
///   coexist.
template <std::unsigned_integral Key, typename T, std::size_t Sets, std::size_t Ways>
  requires(std::popcount(Sets) == 1 && std::popcount(Ways) == 1)
class plru_cache {
public:
  using key_type = Key;
  using mapped_type = T;

  static constexpr std::size_t const sets = Sets;
  static constexpr std::size_t const ways = Ways;

  /// Searches the cache for the \p key and returns a reference to it if present. If not present
  /// and the cache is fully occupied, the likely least recently used value is evicted from the
  /// cache. The \p miss function is called to instantiate the mapped type associated with \p key :
  /// this value is then stored in the cache.
  ///
  /// \param key  Key value of the element to search for.
  /// \param miss  The function that will be called to instantiate the associated value if \p key
  ///   is not present in the cache.
  /// \returns The cached value.
  template <typename MissFn>
    requires(std::is_invocable_r_v<T, MissFn>)
  mapped_type &access(key_type key, MissFn miss) {
    assert(plru_cache::set(key) < Sets);
    return sets_[plru_cache::set(key)].access(key, miss);
  }
  /// \returns The maximum possible number of elements that can be held by the cache.
  [[nodiscard]] constexpr std::size_t max_size() const noexcept { return Sets * Ways; }
  /// \returns The number of elements held by the cache.
  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return std::ranges::fold_left(sets_, std::size_t{0},
                                  [](std::size_t acc, auto const &set) { return acc + std::size(set); });
  }

  [[nodiscard]] static constexpr std::size_t set(key_type key) noexcept { return key & (Sets - 1U); }
  [[nodiscard]] static constexpr std::size_t way(key_type key) noexcept { return (key >> set_bits_) & (Ways - 1U); }

private:
  static constexpr std::size_t set_bits_ = std::bit_width(Sets - 1U);
  using ways_type = details::cache_set<set_bits_, key_type, mapped_type, ways>;
  std::array<ways_type, Sets> sets_{};
};

}  // end namespace midi2

#endif  // MIDI2_PLRU_CACHE_HPP
