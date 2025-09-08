//===-- LRU Doubly-linked List ------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_LRU_LIST_HPP
#define MIDI2_LRU_LIST_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <utility>

#ifndef NDEBUG
#include <ostream>
#endif

namespace midi2::adt {

template <typename ValueType, std::size_t Size>
  requires(Size > 1)
class lru_list {
public:
  class node {
    friend class lru_list;

  public:
    explicit constexpr operator ValueType &() noexcept { return *std::bit_cast<ValueType *>(&payload_[0]); }
    explicit constexpr operator ValueType const &() const noexcept {
      return *std::bit_cast<ValueType const *>(&payload_[0]);
    }

  private:
    alignas(ValueType) std::byte payload_[sizeof(ValueType)]{};
    node *prev_ = nullptr;
    node *next_ = nullptr;
  };

  void clear();
  [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
  void touch(node &n);

  template <std::invocable<ValueType &> Evictor = void(ValueType &)> node &add(ValueType &&payload, Evictor evictor);
  template <std::invocable<ValueType &> Evictor = void(ValueType &)>
  node &add(ValueType const &payload, Evictor evictor);

#ifndef NDEBUG
  void dump(std::ostream &os) const;
#endif

private:
  node *first_ = nullptr;
  node *last_ = nullptr;
  std::size_t size_ = 0;
  std::array<node, Size> v_;

  template <typename OtherValueType, std::invocable<ValueType &> Evictor>
  node &add_impl(OtherValueType &&payload, Evictor evictor);

  void check_invariants() const noexcept;
};

// clear
// ~~~~~
template <typename ValueType, std::size_t Size>
  requires(Size > 1)
void lru_list<ValueType, Size>::clear() {
  auto *const data = v_.data();
  std::for_each(data, data + size_, [](node *const n) {
    n->value()->~ValueType();  // Evict the old value. Bye bye.
  });
  size_ = 0;
  first_ = nullptr;
  last_ = nullptr;
}

// touch
// ~~~~~
template <typename ValueType, std::size_t Size>
  requires(Size > 1)
void lru_list<ValueType, Size>::touch(node &n) {
  assert(first_ != nullptr && last_ != nullptr);
  if (first_ == &n) {
    return;
  }
  // Unhook 'n' from the list in its current position.
  if (last_ == &n) {
    last_ = n.prev_;
  }
  if (n.next_ != nullptr) {
    n.next_->prev_ = n.prev_;
  }
  if (n.prev_ != nullptr) {
    n.prev_->next_ = n.next_;
  }
  // Push on the front of the list.
  n.prev_ = nullptr;
  n.next_ = first_;
  first_->prev_ = &n;
  first_ = &n;
  this->check_invariants();
}

// add
// ~~~
template <typename ValueType, std::size_t Size>
  requires(Size > 1)
template <std::invocable<ValueType &> Evictor>
auto lru_list<ValueType, Size>::add(ValueType const &payload, Evictor const evictor) -> node & {
  return this->add_impl(payload, evictor);
}

template <typename ValueType, std::size_t Size>
  requires(Size > 1)
template <std::invocable<ValueType &> Evictor>
auto lru_list<ValueType, Size>::add(ValueType &&payload, Evictor const evictor) -> node & {
  return this->add_impl(std::move(payload), evictor);
}

template <typename ValueType, std::size_t Size>
  requires(Size > 1)
template <typename OtherValueType, std::invocable<ValueType &> Evictor>
auto lru_list<ValueType, Size>::add_impl(OtherValueType &&payload, Evictor const evictor) -> node & {
  node *result = nullptr;
  if (size_ < v_.size()) {
    // Add this is and push it onto the front of the list.
    result = &v_[size_];
    new (std::to_address(result)) ValueType{std::forward<OtherValueType>(payload)};
    ++size_;
    if (last_ == nullptr) {
      last_ = result;
    }
  } else {
    assert(first_ != nullptr && last_ != nullptr);
    // The list is full so we must evict the last item.
    auto &lru_value = static_cast<ValueType &>(*last_);
    evictor(lru_value);
    // Re-use the array entry for the new value.
    lru_value = std::forward<OtherValueType>(payload);
    // Set about moving this element to the front of the list as the most recently used.
    result = last_;
    assert(last_->prev_ != nullptr);
    last_ = last_->prev_;
    assert(last_->next_ != nullptr);
    last_->next_ = nullptr;
  }

  result->prev_ = nullptr;
  result->next_ = first_;

  if (first_ != nullptr) {
    assert(first_->prev_ == nullptr);
    first_->prev_ = result;
  }
  first_ = result;
  this->check_invariants();
  return *result;
}

// check invariants
// ~~~~~~~~~~~~~~~~
template <typename ValueType, std::size_t Size>
  requires(Size > 1)
void lru_list<ValueType, Size>::check_invariants() const noexcept {
#ifndef NDEBUG
  assert((first_ == nullptr) == (size_ == 0) && "first_ must be null if and only if the container is empty");
  assert((first_ == last_) == (size_ < 2) && "with < 2 members, first_ and last_ must be equal");
  assert((first_ == nullptr || first_->prev_ == nullptr) && "prev of the first element must be null");
  assert((last_ == nullptr || last_->next_ == nullptr) && "next of the last element must be null");

  node const *prev = nullptr;
  for (auto const *n = first_; n != nullptr; n = n->next_) {
    assert(n->prev_ == prev && "next and prev pointers are inconsistent between two nodes");
    prev = n;
  }
  assert(last_ == prev && "the last pointer is not correct");
#endif
}

// dump
// ~~~~
#ifndef NDEBUG
template <typename ValueType, std::size_t Size>
  requires(Size > 1)
void lru_list<ValueType, Size>::dump(std::ostream &os) const {
  this->check_invariants();
  char const *separator = "";
  for (node const *n = first_; n != nullptr; n = n->next) {
    os << separator << *(n->value());
    separator = " ";
  }
  os << '\n';
}
#endif

}  // end namespace midi2::adt

#endif  // MIDI2_LRU_LIST_HPP
