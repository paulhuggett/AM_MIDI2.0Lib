//===-- scale -----------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <gtest/gtest.h>

#include "midi2/utils.hpp"

namespace {

template <typename T, T V> using constant = std::integral_constant<T, V>;
template <unsigned Value> using uconst = constant<unsigned, Value>;

template <typename T> class McmScale : public testing::Test {};

template <unsigned Input, unsigned Expected>
using From7To16 = std::tuple<uconst<7>, uconst<16>, constant<std::uint8_t, Input>, constant<std::uint16_t, Expected>>;
template <unsigned Input, unsigned Expected>
using From7To32 = std::tuple<uconst<7>, uconst<32>, constant<std::uint8_t, Input>, constant<std::uint32_t, Expected>>;
template <unsigned Input, unsigned Expected>
using From16To32 =
    std::tuple<uconst<16>, uconst<32>, constant<std::uint16_t, Input>, constant<std::uint32_t, Expected>>;

template <unsigned Input, unsigned Expected>
using From16To7 = std::tuple<uconst<16>, uconst<7>, constant<std::uint16_t, Input>, constant<std::uint8_t, Expected>>;

// The min-center-max scaling test values are taken from document "M2-115-U Midi 2.0 Bit Scaling and Resolution v1.0.1
// 23-May-2023". Up-scaling is from section 3.3.3; down-scaling is from section 3.4.2.
using TestTypes =
    testing::Types<From7To16<0x05, 0x0A00>, From7To16<0x1E, 0x3C00>, From7To16<0x20, 0x4000>, From7To16<0x40, 0x8000>,
                   From7To16<0x46, 0x8C30>, From7To16<0x60, 0xC104>, From7To16<0x78, 0xF1C7>, From7To16<0x7F, 0xFFFF>,
                   From7To32<0x00, 0x00>, From7To32<0x05, 0x0A000000>, From7To32<0x1E, 0x3C000000>,
                   From7To32<0x20, 0x40000000>, From7To32<0x40, 0x80000000>, From7To32<0x46, 0x8C30C30C>,
                   From7To32<0x60, 0xC1041041>, From7To32<0x78, 0xF1C71C71>, From7To32<0x7F, 0xFFFFFFFF>,

                   From16To32<0x0000, 0x00000000>, From16To32<0x0005, 0x00050000>, From16To32<0x001E, 0x001E0000>,
                   From16To32<0x4000, 0x40000000>, From16To32<0x8000, 0x80000000>, From16To32<0x9C40, 0x9C403880>,
                   From16To32<0xC000, 0xC0008001>, From16To32<0xFDE8, 0xFDE8FBD1>, From16To32<0xFFFF, 0xFFFFFFFF>,

                   From16To7<0x1400, 0x0A>, From16To7<0x8000, 0x40>, From16To7<0xAEBA, 0x57>, From16To7<0xFFFF, 0x7F>>;
// NOLINTNEXTLINE
TYPED_TEST_SUITE(McmScale, TestTypes);

// NOLINTNEXTLINE
TYPED_TEST(McmScale, Ok) {
  constexpr auto source_bits = std::tuple_element_t<0, TypeParam>();
  constexpr auto dest_bits = std::tuple_element_t<1, TypeParam>();
  constexpr auto input = std::tuple_element_t<2, TypeParam>();
  constexpr auto expected = std::tuple_element_t<3, TypeParam>();
  constexpr auto actual = midi2::mcm_scale<source_bits, dest_bits>(input);
  EXPECT_EQ(actual, expected);
}

}  // end anonymous namespace
