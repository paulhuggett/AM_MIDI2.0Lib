//===-- LRU Cache -=============-----------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_CACHE_HPP
#define MIDI2_CACHE_HPP

#include <cassert>
#include <cstddef>
#include <utility>

#ifdef CACHE_TRACE
#include <ostream>
#endif

#include "iumap.hpp"
#include "lru_list.hpp"

namespace midi2 {

template <typename Key, typename Mapped, std::size_t Size> class cache {
public:
  using key_type = Key;
  using mapped_type = Mapped;
  using value_type = std::pair<key_type, mapped_type>;

  mapped_type *find(key_type const &k);
  /// \returns true if the key and value were found in the cache; false otherwise.
  bool set(key_type const &k, mapped_type const &v);

#ifdef CACHE_TRACE
  void dump(std::ostream &os) {
    lru.dump(os);
    h.dump(os);
  }
#endif

private:
  lru_list<value_type, Size> lru_;
  iumap<Key const, typename decltype(lru_)::node *, Size> h_;
};

template <typename Key, typename Mapped, std::size_t Size>
auto cache<Key, Mapped, Size>::find(key_type const &k) -> mapped_type * {
  auto const pos = h_.find(k);
  if (pos == h_.end()) {
    return nullptr;
  }
  auto *const node = pos->second;
  lru_.touch(*node);
  assert(lru_.size() == h_.size());
  return &(static_cast<value_type &>(*node).second);
}

template <typename Key, typename Mapped, std::size_t Size>
bool cache<Key, Mapped, Size>::set(key_type const &k, mapped_type const &v) {
  auto pos = h_.find(k);
  if (pos == h_.end()) {
    auto const fn = [this](value_type &kvp) {
      // delete the key being evicted from the LUR-list from the hash table so that
      // they always match.
      if (auto const evict_pos = h_.find(kvp.first); evict_pos != h_.end()) {
        h_.erase(evict_pos);
      }
    };
    h_.insert(std::make_pair(k, &lru_.add(std::make_pair(k, v), fn)));
    assert(lru_.size() == h_.size());
    return false;
  }

  // The key _was_ found in the cache.
  auto &lru_entry = *pos->second;
  lru_.touch(lru_entry);
  auto &cached_value = static_cast<value_type &>(lru_entry).second;
  if (cached_value == v) {
    // The key was in the cache and the values are equal.
    assert(lru_.size() == h_.size());
    return true;
  }
  cached_value = v;
  assert(lru_.size() == h_.size());
  return false;
}

}  // end namespace midi2

#endif  // MIDI2_CACHE_HPP
