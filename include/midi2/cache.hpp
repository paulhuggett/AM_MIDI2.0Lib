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
#include <ostream>
#include <utility>

#include "iumap.hpp"
#include "lru_list.hpp"

namespace midi2 {

template <typename Key, typename Mapped, std::size_t Size> class cache {
public:
  using key_type = Key;
  using mapped_type = Mapped;
  using value_type = std::pair<key_type, mapped_type>;

  mapped_type *find(key_type const &k);
  bool set(key_type const &k, mapped_type const &v);

  void dump(std::ostream &os) {
    lru.dump(os);
    h.dump(os);
  }

private:
  using lru_container = lru_list<value_type, Size>;

  lru_container lru;
  iumap<Key const, typename decltype(lru)::node *, Size> h;
};

template <typename Key, typename Mapped, std::size_t Size>
auto cache<Key, Mapped, Size>::find(key_type const &k) -> mapped_type * {
  auto const pos = h.find(k);
  if (pos == h.end()) {
    return nullptr;
  }
  auto *const node = pos->second;
  lru.touch(*node);
  assert(lru.size() == h.size());
  return &(static_cast<value_type &>(*node).second);
}

template <typename Key, typename Mapped, std::size_t Size>
bool cache<Key, Mapped, Size>::set(key_type const &k, mapped_type const &v) {
  auto pos = h.find(k);
  if (pos == h.end()) {
    auto const fn = [this](value_type &kvp) {
      // delete the key being evicted from the LUR-list from the hash table so that
      // they always match.
      if (auto const evict_pos = this->h.find(kvp.first); evict_pos != this->h.end()) {
        this->h.erase(evict_pos);
      }
    };
    h.insert(std::make_pair(k, &lru.add(std::make_pair(k, v), fn)));
    assert(lru.size() == h.size());
    return false;
  }

  // The key _was_ found in the cache.
  typename lru_container::node &lru_entry = *pos->second;
  lru.touch(lru_entry);
  auto &cached_value = static_cast<value_type &>(lru_entry).second;
  if (cached_value == v) {
    // The key was in the cache and the values are equal.
    assert(lru.size() == h.size());
    return true;
  }
  cached_value = v;
  assert(lru.size() == h.size());
  return false;
}

}  // end namespace midi2

#endif  // MIDI2_CACHE_HPP
