//===-- plru_cache ------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//
#include "midi2/adt/plru_cache.hpp"

// Standard Library
#include <tuple>
#include <vector>

// Google Test/Mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using midi2::plru_cache;

using testing::ElementsAre;
using testing::MockFunction;
using testing::Return;

namespace {

TEST(PlruCache, Empty) {
  plru_cache<unsigned, int, 4, 2> cache;
  EXPECT_EQ(cache.max_size(), 4 * 2);
  EXPECT_EQ(cache.size(), 0);
}

TEST(PlruCache, InitialAccess) {
  plru_cache<unsigned, std::string, 4, 2> cache;
  std::string const value = "str";
  MockFunction<std::string()> mock_function;

  EXPECT_CALL(mock_function, Call()).WillOnce(Return(value)).RetiresOnSaturation();
  {
    std::string const& actual1 = cache.access(3U, mock_function.AsStdFunction());
    EXPECT_EQ(actual1, value);
    EXPECT_EQ(std::size(cache), 1U);
  }
  {
    // A second call with the same key doesn't create a new member.
    std::string const& actual2 = cache.access(3U, mock_function.AsStdFunction());
    EXPECT_EQ(actual2, value);
    EXPECT_EQ(std::size(cache), 1U);
  }
}

TEST(PlruCache, Fill) {
  plru_cache<unsigned, std::string, 4, 2> cache;

  MockFunction<std::string()> mock_function;
  EXPECT_CALL(mock_function, Call())
      .WillOnce(Return("first"))
      .WillOnce(Return("second"))
      .WillOnce(Return("third"))
      .WillOnce(Return("fourth"))
      .WillOnce(Return("fifth"))
      .WillOnce(Return("sixth"))
      .WillOnce(Return("seventh"))
      .WillOnce(Return("eighth"))
      .RetiresOnSaturation();

  std::string const& first = cache.access(1, mock_function.AsStdFunction());
  EXPECT_EQ(first, "first");
  EXPECT_EQ(cache.size(), 1);
  std::string const& second = cache.access(2, mock_function.AsStdFunction());
  EXPECT_EQ(second, "second");
  EXPECT_EQ(cache.size(), 2);
  std::string const& third = cache.access(3, mock_function.AsStdFunction());
  EXPECT_EQ(third, "third");
  EXPECT_EQ(cache.size(), 3);
  std::string const& fourth = cache.access(4, mock_function.AsStdFunction());
  EXPECT_EQ(fourth, "fourth");
  EXPECT_EQ(cache.size(), 4);
  std::string const& fifth = cache.access(5, mock_function.AsStdFunction());
  EXPECT_EQ(fifth, "fifth");
  EXPECT_EQ(cache.size(), 5);
  std::string const& sixth = cache.access(6, mock_function.AsStdFunction());
  EXPECT_EQ(sixth, "sixth");
  EXPECT_EQ(cache.size(), 6);
  std::string const& seventh = cache.access(7, mock_function.AsStdFunction());
  EXPECT_EQ(seventh, "seventh");
  EXPECT_EQ(cache.size(), 7);
  std::string const& eighth = cache.access(8, mock_function.AsStdFunction());
  EXPECT_EQ(eighth, "eighth");
  EXPECT_EQ(cache.size(), 8);
}

class counted {
public:
  counted() : ctr_{++count_} { actions.emplace_back(action::ctor, ctr_); }
  counted(counted const&) = delete;
  counted(counted&&) = delete;
  ~counted() noexcept {
    try {
      actions.emplace_back(action::dtor, ctr_);
    } catch (...) {
      // Just ignore any exceptions
    }
  }

  counted& operator=(counted const&) = delete;
  counted& operator=(counted&&) = delete;

  enum class action { ctor, dtor };
  static std::vector<std::tuple<action, unsigned>> actions;

private:
  unsigned ctr_ = 0;
  static unsigned count_;
};

unsigned counted::count_ = 0;
std::vector<std::tuple<counted::action, unsigned>> counted::actions;

TEST(PlruCache, OverFill) {
  plru_cache<unsigned, counted, 4, 2> cache;
  using tup = decltype(counted::actions)::value_type;

  auto miss = []() { return counted{}; };

  cache.access(1, miss);
  cache.access(2, miss);
  cache.access(3, miss);
  cache.access(4, miss);
  cache.access(5, miss);
  cache.access(6, miss);
  cache.access(7, miss);
  cache.access(8, miss);
  using enum counted::action;
  EXPECT_THAT(counted::actions, ElementsAre(tup{ctor, 1U}, tup{ctor, 2U}, tup{ctor, 3U}, tup{ctor, 4U}, tup{ctor, 5U},
                                            tup{ctor, 6U}, tup{ctor, 7U}, tup{ctor, 8U}));
  // Accesses of items in the cache. These should now be most-recently used.
  cache.access(1, miss);
  cache.access(2, miss);
  cache.access(3, miss);
  EXPECT_EQ(counted::actions.size(), 8U) << "Expected no new actions to have happened";

  // Reset the actions.
  counted::actions.clear();
  // Accessing a new element will cause an eviction.
  cache.access(9, miss);
  // Check what we threw out and what was added.
  EXPECT_THAT(counted::actions, ElementsAre(tup{dtor, 5U}, tup{ctor, 9U}));

  counted::actions.clear();
  cache.access(10, miss);
  EXPECT_THAT(counted::actions, ElementsAre(tup{dtor, 6U}, tup{ctor, 10U}));
  counted::actions.clear();
  cache.access(11, miss);
  EXPECT_THAT(counted::actions, ElementsAre(tup{dtor, 7U}, tup{ctor, 11U}));
  counted::actions.clear();
  cache.access(12, miss);
  EXPECT_THAT(counted::actions, ElementsAre(tup{dtor, 4U}, tup{ctor, 12U}));
  counted::actions.clear();
  cache.access(13, miss);
  EXPECT_THAT(counted::actions, ElementsAre(tup{dtor, 1U}, tup{ctor, 13U}));
  counted::actions.clear();
  cache.access(14, miss);
  EXPECT_THAT(counted::actions, ElementsAre(tup{dtor, 2U}, tup{ctor, 14U}));
  counted::actions.clear();
  cache.access(15, miss);
  EXPECT_THAT(counted::actions, ElementsAre(tup{dtor, 3U}, tup{ctor, 15U}));
  counted::actions.clear();
  cache.access(16, miss);
  EXPECT_THAT(counted::actions, ElementsAre(tup{dtor, 8U}, tup{ctor, 16U}));
}

}  // end anonymous namespace
