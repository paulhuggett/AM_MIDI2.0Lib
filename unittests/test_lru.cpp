//===-- LRU Doubly-linked List ------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/adt/lru_list.hpp"

// Google Test/Mock/Fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace {

using testing::Eq;
using testing::InSequence;
using testing::StrictMock;

TEST(LruList, Empty) {
  midi2::adt::lru_list<int, 4> lru;
  EXPECT_TRUE(lru.empty());
  EXPECT_EQ(lru.size(), 0);
}

template <typename T> class evictor_base {
public:
  virtual ~evictor_base() noexcept = default;
  void operator()(T &v) { return this->evict(v); }
  virtual void evict(T &v) = 0;
};

template <typename T> class mock_evictor : public evictor_base<T> {
public:
  MOCK_METHOD(void, evict, (T &), (override));
};

TEST(LruList, AddToFull) {
  StrictMock<mock_evictor<int>> evictor;
  midi2::adt::lru_list<int, 4> lru;
  using int_ref = int &;
  EXPECT_THAT(static_cast<int_ref>(lru.add(1, std::ref(evictor))), 1);
  EXPECT_FALSE(lru.empty());
  EXPECT_EQ(lru.size(), 1);

  EXPECT_THAT(static_cast<int_ref>(lru.add(2, std::ref(evictor))), 2);
  EXPECT_FALSE(lru.empty());
  EXPECT_EQ(lru.size(), 2);

  EXPECT_THAT(static_cast<int_ref>(lru.add(3, std::ref(evictor))), 3);
  EXPECT_FALSE(lru.empty());
  EXPECT_EQ(lru.size(), 3);

  EXPECT_THAT(static_cast<int_ref>(lru.add(4, std::ref(evictor))), 4);
  EXPECT_FALSE(lru.empty());
  EXPECT_EQ(lru.size(), 4);
}

TEST(LruList, EvictFirst) {
  StrictMock<mock_evictor<int>> evictor;
  auto one = 1;
  EXPECT_CALL(evictor, evict(one)).Times(1);
  midi2::adt::lru_list<int, 4> lru;
  lru.add(1, std::ref(evictor));
  lru.add(2, std::ref(evictor));
  lru.add(3, std::ref(evictor));
  lru.add(4, std::ref(evictor));
  lru.add(5, std::ref(evictor));
  EXPECT_FALSE(lru.empty());
  EXPECT_EQ(lru.size(), 4);
}

TEST(LruList, TouchOneEvictTwo) {
  StrictMock<mock_evictor<int>> evictor;
  auto two = 2;
  EXPECT_CALL(evictor, evict(two)).Times(1);
  midi2::adt::lru_list<int, 4> lru;
  auto &one = lru.add(1, std::ref(evictor));
  lru.add(2, std::ref(evictor));
  lru.add(3, std::ref(evictor));
  lru.add(4, std::ref(evictor));
  lru.touch(one);
  lru.add(5, std::ref(evictor));
  EXPECT_FALSE(lru.empty());
  EXPECT_EQ(lru.size(), 4);
}

TEST(LruList, Sequence) {
  StrictMock<mock_evictor<int>> evictor;

  auto two = 2;
  auto three = 3;
  auto four = 4;
  {
    InSequence _;
    EXPECT_CALL(evictor, evict(two)).Times(1);
    EXPECT_CALL(evictor, evict(three)).Times(1);
    EXPECT_CALL(evictor, evict(four)).Times(1);
  }

  midi2::adt::lru_list<int, 4> lru;
  auto &t1 = lru.add(1, std::ref(evictor));
  lru.touch(t1);  // do nothing!
  auto &t2 = lru.add(2, std::ref(evictor));
  lru.touch(t2);  // do nothing!
  auto &t3 = lru.add(3, std::ref(evictor));
  lru.add(4, std::ref(evictor));
  lru.touch(t1);

  lru.add(5, std::ref(evictor));  // evicts 2
  lru.add(6, std::ref(evictor));  // evicts 3

  lru.touch(t3);                  // 3 is now at the front of the list
  lru.add(7, std::ref(evictor));  // evicts 4
}

void Thrash(std::vector<int> const &a) {
  midi2::adt::lru_list<int, 4> lru;
  auto evictor = [](int &) {};
  for (auto value : a) {
    lru.add(value, evictor);
  }
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
FUZZ_TEST(LruList, Thrash);
#endif
TEST(LruList, NoThrash) {
  Thrash({});
}

class move_only {
public:
  constexpr explicit move_only(int a) noexcept : a_{a} {}
  constexpr move_only(move_only const &) = delete;
  constexpr move_only(move_only &&) noexcept = default;

  constexpr move_only &operator=(move_only const &) = delete;
  constexpr move_only &operator=(move_only &&) noexcept = default;

  constexpr bool operator==(move_only const &) const noexcept = default;

private:
  int a_;
};

TEST(LruList, MoveOnly) {
  midi2::adt::lru_list<move_only, 2> lru;
  StrictMock<mock_evictor<move_only>> evictor;

  move_only m3{3};
  move_only m5{5};
  move_only m7{7};
  InSequence _;
  EXPECT_CALL(evictor, evict(Eq(std::ref(m3))));
  EXPECT_CALL(evictor, evict(Eq(std::ref(m5))));
  EXPECT_CALL(evictor, evict(Eq(std::ref(m7))));

  auto &node0 = lru.add(move_only{3}, std::ref(evictor));
  EXPECT_EQ(static_cast<move_only &>(node0), m3);
  auto &node1 = lru.add(move_only{5}, std::ref(evictor));
  EXPECT_EQ(static_cast<move_only &>(node1), m5);
  auto &node2 = lru.add(move_only{7}, std::ref(evictor));
  EXPECT_EQ(static_cast<move_only &>(node2), m7);
  auto &node3 = lru.add(move_only{11}, std::ref(evictor));
  EXPECT_EQ(static_cast<move_only &>(node3), move_only{11});
  auto &node4 = lru.add(move_only{13}, std::ref(evictor));
  EXPECT_EQ(static_cast<move_only &>(node4), move_only{13});
}

}  // end anonymous namespace
