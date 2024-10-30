//===-- cache ===--------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/adt/cache.hpp"

// Google Test/Mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::Pointee;

namespace {

// NOLINTNEXTLINE
TEST(Cache, Fill) {
  midi2::cache<int, int, 4> c;
  EXPECT_FALSE(c.set(1, 10));
  EXPECT_FALSE(c.set(2, 20));
  EXPECT_FALSE(c.set(3, 30));
  EXPECT_FALSE(c.set(4, 40));
}

// NOLINTNEXTLINE
TEST(Cache, Find) {
  midi2::cache<int, int, 4> c;
  c.set(1, 10);
  c.set(2, 20);
  c.set(3, 30);
  c.set(4, 40);

  EXPECT_THAT(c.find(1), Pointee(10));
  EXPECT_THAT(c.find(2), Pointee(20));
  EXPECT_THAT(c.find(3), Pointee(30));
  EXPECT_THAT(c.find(4), Pointee(40));
  EXPECT_THAT(c.find(5), nullptr);
}

// NOLINTNEXTLINE
TEST(Cache, FindEvicted) {
  midi2::cache<int, int, 4> c;
  c.set(1, 10);
  c.set(2, 20);
  c.set(3, 30);
  c.set(4, 40);
  EXPECT_FALSE(c.set(5, 50));
  EXPECT_TRUE(c.set(4, 40));
  EXPECT_TRUE(c.set(5, 50));

  EXPECT_THAT(c.find(1), nullptr) << "expected key 1 to have been evicted";
  EXPECT_THAT(c.find(2), Pointee(20));
  EXPECT_THAT(c.find(3), Pointee(30));
  EXPECT_THAT(c.find(4), Pointee(40));
  EXPECT_THAT(c.find(5), Pointee(50));

  EXPECT_FALSE(c.set(5, 60)) << "value is different so not cached";
  EXPECT_TRUE(c.set(5, 60));
}

}  // end anonymous namespace
