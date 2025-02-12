//===-- ci types --------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <gtest/gtest.h>

#include "midi2/ci_types.hpp"

namespace {

using midi2::ci::byte_array;
using midi2::ci::from_le7;
constexpr auto zero = std::byte{0};
constexpr auto one = std::byte{1};
constexpr auto max = std::byte{0x7F};

TEST(LE7, TwoByte) {
  EXPECT_EQ(from_le7(byte_array<2>{zero, zero}), 0);
  EXPECT_EQ(from_le7(byte_array<2>{one, zero}), 1);
  EXPECT_EQ(from_le7(byte_array<2>{std::byte{2}, zero}), 2);
  EXPECT_EQ(from_le7(byte_array<2>{max, zero}), 0x7F);
  EXPECT_EQ(from_le7(byte_array<2>{zero, one}), 0x80);
  EXPECT_EQ(from_le7(byte_array<2>{zero, max}), 0x3F80);
  EXPECT_EQ(from_le7(byte_array<2>{max, max}), 0x3FFF);
}

TEST(LE7, FourByte) {
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, zero, zero}), 0);
  EXPECT_EQ(from_le7(byte_array<4>{one, zero, zero, zero}), 1);
  EXPECT_EQ(from_le7(byte_array<4>{std::byte{2}, zero, zero, zero}), 2);
  EXPECT_EQ(from_le7(byte_array<4>{max, zero, zero, zero}), 0x7F);
  EXPECT_EQ(from_le7(byte_array<4>{zero, one, zero, zero}), 0x80);
  EXPECT_EQ(from_le7(byte_array<4>{zero, max, zero, zero}), 0x3F80);
  EXPECT_EQ(from_le7(byte_array<4>{max, max, zero, zero}), 0x3FFF);
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, one, zero}), 0x4000);
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, max, zero}), 0x1FC000);
  EXPECT_EQ(from_le7(byte_array<4>{max, max, max, zero}), 0x1FFFFF);
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, zero, one}), 0x200000);
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, zero, max}), 0xFE00000);
  EXPECT_EQ(from_le7(byte_array<4>{max, max, max, max}), 0xFFFFFFF);
}

}  // end anonymous namespace
