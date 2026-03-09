//===-- bitfield --------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
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

namespace {

using midi2::adt::bit_field;
using midi2::adt::bit_range;
using midi2::adt::max_value;
using testing::Types;

template <typename T> class BitFieldAssignment : public testing::Test {};

template <typename Type, unsigned Index, unsigned Bits> struct param {
  using value_type = Type;
  using br = bit_range<Index, Bits>;
};

using assign_test_types =
    Types<param<std::uint8_t, 0U, 1U>,  // testing bits [0,1)
          param<std::uint8_t, 1U, 1U>,  // testing bits [1,2)
          param<std::uint8_t, 7U, 1U>,  // ... and so on.
          param<std::uint16_t, 8U, 1U>, param<std::uint16_t, 15U, 1U>, param<std::uint32_t, 16U, 1U>,
          param<std::uint32_t, 31U, 1U>, param<std::uint64_t, 32U, 1U>, param<std::uint64_t, 63U, 1U>,

          param<std::uint8_t, 0U, 2U>, param<std::uint8_t, 1U, 2U>, param<std::uint8_t, 6U, 2U>,
          param<std::uint16_t, 7U, 2U>, param<std::uint16_t, 8U, 2U>, param<std::uint16_t, 14U, 2U>,
          param<std::uint32_t, 15U, 2U>, param<std::uint32_t, 16U, 2U>, param<std::uint64_t, 31U, 2U>,
          param<std::uint64_t, 32U, 2U>, param<std::uint64_t, 62U, 2U>,

          param<std::uint8_t, 0U, 7U>, param<std::uint8_t, 0U, 8U>, param<std::uint16_t, 0U, 9U>,
          param<std::uint16_t, 0U, 15U>, param<std::uint16_t, 0U, 16U>, param<std::uint32_t, 0U, 17U>,
          param<std::uint32_t, 0U, 31U>, param<std::uint32_t, 0U, 32U>, param<std::uint64_t, 0U, 63U>>;

TYPED_TEST_SUITE(BitFieldAssignment, assign_test_types, );

TYPED_TEST(BitFieldAssignment, Signed) {
  using value_type = TypeParam::value_type;
  using br = TypeParam::br;
  bit_field<value_type> f1{std::numeric_limits<value_type>::max()};
  auto const actual = f1.template get_signed<br>();
  EXPECT_EQ(actual, -1);
}

TYPED_TEST(BitFieldAssignment, Signed2) {
  using value_type = TypeParam::value_type;
  using br = TypeParam::br;
  bit_field<value_type> f1;
  EXPECT_EQ(f1.template get_signed<br>(), 0);
  f1.template set_signed<br>(-1);
  EXPECT_EQ(f1.template get_signed<br>(), -1);
  f1.template set_signed<br>(0);
  EXPECT_EQ(f1.template get_signed<br>(), 0);

  if constexpr (typename br::bits() > 1) {
    f1.template set_signed<br>(1);
    EXPECT_EQ(f1.template get_signed<br>(), 1);
  }
}

TYPED_TEST(BitFieldAssignment, Assignment) {
  using value_type = typename TypeParam::value_type;
  using br = typename TypeParam::br;
  constexpr auto bits = typename br::bits();
  bit_field<value_type> value{0};

  // All bits are 0.
  EXPECT_EQ(value.template get<br>(), 0U);
  // Set all bits to 1.
  auto const vt = static_cast<value_type>(~value_type{0U}) & max_value<value_type, bits>();
  value.template set<br>(vt);
  EXPECT_EQ((value.template get<br>()), (max_value<value_type, bits>()));
  value.template set<br>(0U);
  EXPECT_EQ((value.template get<br>()), 0U);
  value.template set<br>(1U);
  EXPECT_EQ((value.template get<br>()), 1U);
}

TEST(Bitfield, IsolationFromOtherBitfields) {
  using midi2::adt::max_value;

  using bf1 = bit_range<0, 2>;
  using bf2 = bit_range<2, 6>;
  bit_field<std::uint8_t> value{0};

  EXPECT_EQ(value.get<bf1>(), 0U);
  EXPECT_EQ(value.get<bf2>(), 0U);

  constexpr auto max1 = max_value<std::uint8_t, bf1::bits::value>();
  value.set<bf1>(max1);
  EXPECT_EQ(value.get<bf1>(), max1);
  EXPECT_EQ(value.get<bf2>(), 0U);
  value.set<bf1>(0);
  EXPECT_EQ(value.get<bf1>(), 0U);
  EXPECT_EQ(value.get<bf2>(), 0U);

  constexpr auto max2 = max_value<std::uint8_t, bf2::bits::value>();
  value.set<bf2>(max2);
  EXPECT_EQ(value.get<bf1>(), 0);
  EXPECT_EQ(value.get<bf2>(), max2);
}

TEST(Bitfield, Max) {
  using midi2::adt::max_value;
  EXPECT_EQ((max_value<std::uint8_t, 1>()), 1U);
  EXPECT_EQ((max_value<std::uint16_t, 1>()), 1U);
  EXPECT_EQ((max_value<std::uint32_t, 1>()), 1U);
  EXPECT_EQ((max_value<std::uint64_t, 1>()), 1U);

  EXPECT_EQ((max_value<std::uint8_t, 2>()), 3U);
  EXPECT_EQ((max_value<std::uint16_t, 2>()), 3U);
  EXPECT_EQ((max_value<std::uint32_t, 2>()), 3U);
  EXPECT_EQ((max_value<std::uint64_t, 2>()), 3U);

  EXPECT_EQ((max_value<std::uint8_t, 8>()), std::numeric_limits<std::uint8_t>::max());
  EXPECT_EQ((max_value<std::uint16_t, 8>()), std::numeric_limits<std::uint8_t>::max());
  EXPECT_EQ((max_value<std::uint32_t, 8>()), std::numeric_limits<std::uint8_t>::max());
  EXPECT_EQ((max_value<std::uint64_t, 8>()), std::numeric_limits<std::uint8_t>::max());

  EXPECT_EQ((max_value<std::uint16_t, 16>()), std::numeric_limits<std::uint16_t>::max());
  EXPECT_EQ((max_value<std::uint32_t, 16>()), std::numeric_limits<std::uint16_t>::max());
  EXPECT_EQ((max_value<std::uint64_t, 16>()), std::numeric_limits<std::uint16_t>::max());

  EXPECT_EQ((max_value<std::uint32_t, 32>()), std::numeric_limits<std::uint32_t>::max());
  EXPECT_EQ((max_value<std::uint64_t, 32>()), std::numeric_limits<std::uint32_t>::max());

  EXPECT_EQ((max_value<std::uint64_t, 64>()), std::numeric_limits<std::uint64_t>::max());
}

}  // end anonymous namespace
