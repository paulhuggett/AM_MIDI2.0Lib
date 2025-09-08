//===-- scale -----------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ump/ump_utils.hpp"

// Google test
#include <gtest/gtest.h>

namespace {

template <auto V> using constant = std::integral_constant<decltype(V), V>;

template <unsigned SourceBits, unsigned DestBits, auto Input, auto Expected> struct test_param {
  using source_bits = constant<SourceBits>;
  using dest_bits = constant<DestBits>;
  using input = constant<Input>;
  using expected = constant<Expected>;
};
template <std::uint8_t Input, std::uint16_t Expected> using from7_to_16 = test_param<7U, 16U, Input, Expected>;
template <std::uint8_t Input, std::uint32_t Expected> using from_7_to_32 = test_param<7U, 32, Input, Expected>;
template <std::uint16_t Input, std::uint32_t Expected> using from_16_to_32 = test_param<16U, 32U, Input, Expected>;
template <std::uint16_t Input, std::uint8_t Expected> using from_16_to_7 = test_param<16U, 7U, Input, Expected>;

// The min-center-max scaling test values are taken from document "M2-115-U Midi 2.0 Bit Scaling and Resolution v1.0.1
// 23-May-2023". Up-scaling is from section 3.3.3; down-scaling is from section 3.4.2.
using TestTypes = testing::Types<
    from7_to_16<0x05, 0x0A00>, from7_to_16<0x1E, 0x3C00>, from7_to_16<0x20, 0x4000>, from7_to_16<0x40, 0x8000>,
    from7_to_16<0x46, 0x8C30>, from7_to_16<0x60, 0xC104>, from7_to_16<0x78, 0xF1C7>, from7_to_16<0x7F, 0xFFFF>,

    from_7_to_32<0x00, 0x00>, from_7_to_32<0x05, 0x0A000000>, from_7_to_32<0x1E, 0x3C000000>,
    from_7_to_32<0x20, 0x40000000>, from_7_to_32<0x40, 0x80000000>, from_7_to_32<0x46, 0x8C30C30C>,
    from_7_to_32<0x60, 0xC1041041>, from_7_to_32<0x78, 0xF1C71C71>, from_7_to_32<0x7F, 0xFFFFFFFF>,

    from_16_to_32<0x0000, 0x00000000>, from_16_to_32<0x0005, 0x00050000>, from_16_to_32<0x001E, 0x001E0000>,
    from_16_to_32<0x4000, 0x40000000>, from_16_to_32<0x8000, 0x80000000>, from_16_to_32<0x9C40, 0x9C403880>,
    from_16_to_32<0xC000, 0xC0008001>, from_16_to_32<0xFDE8, 0xFDE8FBD1>, from_16_to_32<0xFFFF, 0xFFFFFFFF>,

    from_16_to_7<0x1400, 0x0A>, from_16_to_7<0x8000, 0x40>, from_16_to_7<0xAEBA, 0x57>, from_16_to_7<0xFFFF, 0x7F>>;

template <typename T> class McmScale : public testing::Test {};

// NOLINTNEXTLINE
TYPED_TEST_SUITE(McmScale, TestTypes);

// NOLINTNEXTLINE
TYPED_TEST(McmScale, Ok) {
  constexpr auto actual =
      midi2::ump::mcm_scale<TypeParam::source_bits::value, TypeParam::dest_bits::value>(TypeParam::input::value);
  EXPECT_EQ(actual, TypeParam::expected::value);
}

}  // end anonymous namespace
