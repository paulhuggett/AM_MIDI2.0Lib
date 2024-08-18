// DUT
#include "midi2/mcoded7.hpp"

// standard library
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace {

struct raw_and_encoded {
  std::vector<std::uint8_t> raw;
  std::vector<std::uint8_t> encoded;
};

class Mcoded7 : public testing::TestWithParam<raw_and_encoded> {
public:
  /// Takes a vector of bytes and returns the mcoded7 encoded equivalent vector.
  static std::vector<std::uint8_t> encode(
      std::vector<std::uint8_t> const& input) {
    midi2::mcoded7::encoder encoder;
    std::vector<std::uint8_t> output;
    auto out = std::back_inserter(output);
    std::for_each(std::begin(input), std::end(input),
                  [&encoder, &out](std::uint8_t const b) {
                    out = encoder.parse_byte(b, out);
                  });
    encoder.flush(out);
    return output;
  }

  /// Decodes a vector of mcoded7 bytes.
  static std::vector<std::uint8_t> decode(
      std::vector<std::uint8_t> const& input) {
    midi2::mcoded7::decoder decoder;
    std::vector<std::uint8_t> output;
    auto out = std::back_inserter(output);
    std::for_each(std::begin(input), std::end(input),
                  [&decoder, &out](std::uint8_t const b) {
                    out = decoder.parse_byte(b, out);
                  });
    decoder.flush(out);
    return output;
  }
};

}  // end anonymous namespace

// NOLINTNEXTLINE
TEST_P(Mcoded7, Encode) {
  auto const& param = GetParam();
  EXPECT_THAT(encode(param.raw), testing::ContainerEq(param.encoded));
}
// NOLINTNEXTLINE
TEST_P(Mcoded7, Decode) {
  auto const& param = GetParam();
  EXPECT_THAT(decode(param.encoded), testing::ContainerEq(param.raw));
}

namespace {

// A small collection of test vectors.
raw_and_encoded const empty;
raw_and_encoded const four{
    std::vector<std::uint8_t>{0b00010010, 0b00110100, 0b01010110, 0b01111000},
    std::vector<std::uint8_t>{0b00000000,  // MSBs
                              0b00010010, 0b00110100, 0b01010110, 0b01111000}};
raw_and_encoded const seven{
    std::vector<std::uint8_t>{0b00010010, 0b00110100, 0b01010110, 0b01111000,
                              0b10011010, 0b10111100, 0b11011110},
    std::vector<std::uint8_t>{0b00000111,  // MSBs
                              0b00010010, 0b00110100, 0b01010110, 0b01111000,
                              0b00011010, 0b00111100, 0b01011110}};
raw_and_encoded const eight{
    std::vector<std::uint8_t>{0b00010010, 0b00110100, 0b01010110, 0b01111000,
                              0b10011010, 0b10111100, 0b11011110, 0b11110000},
    std::vector<std::uint8_t>{// block #1
                              0b00000111, 0b00010010, 0b00110100, 0b01010110,
                              0b01111000, 0b00011010, 0b00111100, 0b01011110,
                              // block #2
                              0b01000000, 0b01110000}};

}  // end anonymous namespace

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(Mcoded7, Mcoded7,
                         testing::Values(empty, four, seven, eight));

namespace {

void Mcoded7RoundTrip(std::vector<std::uint8_t> const& input) {
  auto const encoded = Mcoded7::encode(input);
  auto const decoded = Mcoded7::decode(encoded);
  EXPECT_THAT(decoded, testing::ContainerEq(input));
}

}  // end anonymous namespace

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(Mcoded7, Mcoded7RoundTrip);
#endif
// NOLINTNEXTLINE
TEST(Mcoded7, EmptyRoundTrip) {
  Mcoded7RoundTrip(std::vector<std::uint8_t>{});
}

// NOLINTNEXTLINE
TEST(Mcoded7, GoodInput) {
  midi2::mcoded7::decoder decoder;
  std::array<std::uint8_t, 1> output;
  auto* out = output.data();
  out = decoder.parse_byte(0b00000000, out);
  EXPECT_TRUE(decoder.good());
  out = decoder.parse_byte(0b00010010, out);
  EXPECT_TRUE(decoder.good());
  EXPECT_EQ(out, output.data() + 1);
}

// NOLINTNEXTLINE
TEST(Mcoded7, BadInput) {
  midi2::mcoded7::decoder decoder;
  std::array<std::uint8_t, 2> output;
  auto* out = output.data();
  out = decoder.parse_byte(0b00000000, out);
  EXPECT_TRUE(decoder.good());
  out = decoder.parse_byte(0b10010010, out);
  EXPECT_FALSE(decoder.good())
      << "Most significant bit was set: state should be bad";
  out = decoder.parse_byte(0b00010010, out);
  EXPECT_FALSE(decoder.good()) << "Expected the 'good' state to be sticky";
}
