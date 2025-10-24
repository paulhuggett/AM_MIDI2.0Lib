//===-- UMP -------------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/ump/ump_types.hpp"

// Google test
#include <gtest/gtest.h>

TEST(UMPType, Noop) {
  midi2::ump::utility::noop m;
  EXPECT_TRUE(midi2::ump::check(m));
}

TEST(UMPType, SetTimeSignature) {
  midi2::ump::flex_data::set_time_signature m;
  EXPECT_TRUE(midi2::ump::check(m));
}
