//===-- bitfield --------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/adt/bitfield.hpp"

// Standard Library
#include <bit>
#include <cstdint>
#include <limits>
#include <type_traits>

// Google Test
#include <gtest/gtest.h>

using midi2::bitfield;

namespace {

using testing::Types;

template <typename T> class BitfieldAssignment : public testing::Test {};

template <typename Type, unsigned Index, unsigned Bits> struct param {
  using value_type = Type;
  using index = std::integral_constant<unsigned, Index>;
  using bits = std::integral_constant<unsigned, Bits>;
};

using assign_test_types =
    Types<param<std::uint8_t, 0, 1>,  // testing bits [0,1)
          param<std::uint8_t, 1, 1>,  // testing bits [1,2)
          param<std::uint8_t, 7, 1>,  // ... and so on.
          param<std::uint16_t, 8, 1>, param<std::uint16_t, 15, 1>, param<std::uint32_t, 16, 1>,
          param<std::uint32_t, 31, 1>, param<std::uint64_t, 32, 1>, param<std::uint64_t, 63, 1>,

          param<std::uint8_t, 0, 2>, param<std::uint8_t, 1, 2>, param<std::uint8_t, 6, 2>, param<std::uint16_t, 7, 2>,
          param<std::uint16_t, 8, 2>, param<std::uint16_t, 14, 2>, param<std::uint32_t, 15, 2>,
          param<std::uint32_t, 16, 2>, param<std::uint64_t, 31, 2>, param<std::uint64_t, 32, 2>,
          param<std::uint64_t, 62, 2>,

          param<std::uint8_t, 0, 7>, param<std::uint8_t, 0, 8>, param<std::uint16_t, 0, 9>, param<std::uint16_t, 0, 15>,
          param<std::uint16_t, 0, 16>, param<std::uint32_t, 0, 17>, param<std::uint32_t, 0, 31>,
          param<std::uint32_t, 0, 32>, param<std::uint64_t, 0, 63>, param<std::uint64_t, 0, 64>>;

TYPED_TEST_SUITE(BitfieldAssignment, assign_test_types, );

TYPED_TEST(BitfieldAssignment, Signed) {
  using bf = bitfield<typename TypeParam::value_type, TypeParam::index::value, TypeParam::bits::value>;
  bf f1{};
  f1 = bf::max();
  EXPECT_EQ(f1.signed_value(), -1);
}

TYPED_TEST(BitfieldAssignment, Assignment) {
  using value_type = typename TypeParam::value_type;
  constexpr auto index = typename TypeParam::index();
  constexpr auto bits = typename TypeParam::bits();

  using bf = bitfield<value_type, index, bits>;
  union {
    value_type vt;
    bf f1;
  };
  // Set all bits to 0.
  vt = 0;
  EXPECT_EQ(std::bit_cast<bf>(vt).value(), 0U);
  // Set all bits to 1.
  vt = static_cast<value_type>(~value_type{0U});
  EXPECT_EQ(std::bit_cast<bf>(vt).value(), (bitfield<value_type, index, bits>::max()));
  f1 = 0U;
  EXPECT_EQ(f1.value(), 0U);
  f1 = 1U;
  EXPECT_EQ(f1.value(), 1U);
  constexpr auto v = decltype(f1)::max();
  f1 = v;
  // access the value of f1 by casting directly rather than using the value()
  // member function.
  EXPECT_EQ(static_cast<value_type>(f1), v);
  // Access the unlerlying memory.
  EXPECT_EQ(std::bit_cast<value_type>(f1), std::numeric_limits<value_type>::max());
}

TEST(Bitfield, IsolationFromOtherBitfields) {
  using bf1 = bitfield<std::uint8_t, 0, 2>;
  using bf2 = bitfield<std::uint8_t, 2, 6>;
  union {
    std::uint8_t value;
    bf1 f1;  // f1 is bits [0-2)
    bf2 f2;  // f2 is bits [2-8)
  };

  value = 0;
  EXPECT_EQ(std::bit_cast<bf1>(value).value(), 0U);
  EXPECT_EQ(std::bit_cast<bf2>(value).value(), 0U);

  f1 = decltype(f1)::max();
  EXPECT_EQ(f1.value(), decltype(f1)::max());
  EXPECT_EQ(std::bit_cast<std::uint8_t>(f1), 0x03);

  f1 = std::uint8_t{0};
  f2 = decltype(f2)::max();
  EXPECT_EQ(std::bit_cast<std::uint8_t>(f2), 0xFC);
}

TEST(Bitfield, Max) {
  EXPECT_EQ((bitfield<std::uint8_t, 0, 1>::max()), 1U);
  EXPECT_EQ((bitfield<std::uint16_t, 0, 1>::max()), 1U);
  EXPECT_EQ((bitfield<std::uint32_t, 0, 1>::max()), 1U);
  EXPECT_EQ((bitfield<std::uint64_t, 0, 1>::max()), 1U);

  EXPECT_EQ((bitfield<std::uint8_t, 0, 8>::max()), std::numeric_limits<std::uint8_t>::max());
  EXPECT_EQ((bitfield<std::uint16_t, 0, 8>::max()), std::numeric_limits<std::uint8_t>::max());
  EXPECT_EQ((bitfield<std::uint32_t, 0, 8>::max()), std::numeric_limits<std::uint8_t>::max());
  EXPECT_EQ((bitfield<std::uint64_t, 0, 8>::max()), std::numeric_limits<std::uint8_t>::max());

  EXPECT_EQ((bitfield<std::uint16_t, 0, 16>::max()), std::numeric_limits<std::uint16_t>::max());
  EXPECT_EQ((bitfield<std::uint32_t, 0, 16>::max()), std::numeric_limits<std::uint16_t>::max());
  EXPECT_EQ((bitfield<std::uint64_t, 0, 16>::max()), std::numeric_limits<std::uint16_t>::max());

  EXPECT_EQ((bitfield<std::uint32_t, 0, 32>::max()), std::numeric_limits<std::uint32_t>::max());
  EXPECT_EQ((bitfield<std::uint64_t, 0, 32>::max()), std::numeric_limits<std::uint32_t>::max());

  EXPECT_EQ((bitfield<std::uint64_t, 0, 64>::max()), std::numeric_limits<std::uint64_t>::max());
}

}  // end anonymous namespace
