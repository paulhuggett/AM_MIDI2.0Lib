//===-- ump bytestream round trip ---------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT includes
#include "midi2/bytestream_to_ump.hpp"
#include "midi2/ump_to_bytestream.hpp"

// Standard Library
#include <algorithm>
#include <cstdint>
#include <format>
#include <ios>
#include <ostream>
#include <vector>

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace {

using byte_vector = std::vector<std::byte>;
using ump_vector = std::vector<std::uint32_t>;

ump_vector bytesToUMP(byte_vector const& in) {
  midi2::bytestream_to_ump bs2ump;
  ump_vector out;
  for (auto const v : in) {
    bs2ump.push(v);
    while (!bs2ump.empty()) {
      out.push_back(bs2ump.pop());
    }
  }
  return out;
}

byte_vector umpToBytes(ump_vector const& in) {
  midi2::ump_to_bytestream ump2bs;
  byte_vector out;
  for (auto const v : in) {
    ump2bs.push(v);
    while (!ump2bs.empty()) {
      out.push_back(ump2bs.pop());
    }
  }
  return out;
}

template <typename T> struct hex_values {
  std::vector<T> const& bytes;
};
template <typename Vt> hex_values(Vt a) -> hex_values<typename Vt::value_type>;

template <typename T> std::ostream& operator<<(std::ostream& os, hex_values<T> const& v) {
  auto const* separator = "";
  for (auto const b : v.bytes) {
    os << std::format("{}0x{:02X}", separator, static_cast<unsigned>(b));
    separator = ", ";
  }
  return os;
}

void UMPByteStreamRoundTrip(byte_vector const& b1) {
  // The presence of partial system exclusive messages causes the test to
  // fail. For the time being just filter out any buffers with even a hint
  // of sysex.
  if (auto const end = std::end(b1); std::find_if(std::begin(b1), end, [](std::byte const b) {
                                       using enum midi2::status;
                                       return b == std::byte{to_underlying(midi2::status::sysex_start)} ||
                                              b == std::byte{to_underlying(midi2::status::sysex_stop)};
                                     }) != end) {
    return;
  }
  // The test first converts the original byte-stream to UMP, converts the
  // result back to a byte-stream, and then repeats that operation.
  //
  //   b_1 -> UMP_1 -> b_2 -> UMP_2 -> b_3.
  //
  // We finally compare b_2 and b_3. The initial step of converting the
  // original stream to UMP and back ensures that we remove any partial or
  // unrecognized messages.
  auto const ump1 = bytesToUMP(b1);
  auto const b2 = umpToBytes(ump1);
  auto const ump2 = bytesToUMP(b2);
  auto const b3 = umpToBytes(ump2);
  EXPECT_THAT(b3, testing::ContainerEq(b2)) << "Converting from " << hex_values{b1};
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPByteStreamRoundTrip, UMPByteStreamRoundTrip);
#endif
// NOLINTNEXTLINE
TEST(UMPByteStreamRoundTrip, Empty) {
  UMPByteStreamRoundTrip({});
}

}  // end anonymous namespace
