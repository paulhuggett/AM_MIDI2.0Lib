// DUT
#include "midi2/ump_to_midi2.hpp"

// Standard Library
#include <array>
#include <ranges>
#include <vector>

// Google Test/Mock
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::ElementsAre;

namespace {

template <std::ranges::input_range Range> auto convert(Range const& input) {
  std::vector<std::uint32_t> output;
  midi2::ump_to_midi2 ump2m2;
  for (auto const message : input) {
    ump2m2.push(message);
    while (!ump2m2.empty()) {
      output.push_back(ump2m2.pop());
    }
  }
  return output;
}

TEST(UMPToMidi2, NoteOff) {
  constexpr auto note = 64;

  midi2::types::m1cvm::note_off in;
  auto& in0 = get<0>(in.w);
  in0.group = 0;
  in0.channel = 0;
  in0.note = note;
  in0.velocity = 0x60;

  midi2::types::m2cvm::note_off expected;
  auto& expected0 = get<0>(expected.w);
  auto& expected1 = get<1>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.note = note;
  expected0.attribute = 0;
  expected1.velocity = 0xC104;
  expected1.attribute = 0;

  std::array const input{std::bit_cast<std::uint32_t>(in0)};
  EXPECT_THAT(convert(input),
              ElementsAre(std::bit_cast<std::uint32_t>(expected0), std::bit_cast<std::uint32_t>(expected1)));
}

TEST(UMPToMidi2, NoteOn) {
  constexpr auto note = 64;

  midi2::types::m1cvm::note_on in;
  auto& in0 = get<0>(in.w);
  in0.group = 0;
  in0.channel = 0;
  in0.note = note;
  in0.velocity = 0x60;

  midi2::types::m2cvm::note_on expected;
  auto& expected0 = get<0>(expected.w);
  auto& expected1 = get<1>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.note = note;
  expected0.attribute = 0;
  expected1.velocity = 0xC104;
  expected1.attribute = 0;

  std::array const input{std::bit_cast<std::uint32_t>(in0)};
  EXPECT_THAT(convert(input),
              ElementsAre(std::bit_cast<std::uint32_t>(expected0), std::bit_cast<std::uint32_t>(expected1)));
}

TEST(UMPToMidi2, PolyPressure) {
  constexpr auto note = 64;

  midi2::types::m1cvm::poly_pressure in;
  auto& in0 = get<0>(in.w);
  in0.group = 0;
  in0.channel = 0;
  in0.note = note;
  in0.pressure = 0x60;

  midi2::types::m2cvm::poly_pressure expected;
  auto& expected0 = get<0>(expected.w);
  auto& expected1 = get<1>(expected.w);
  expected0.group = 0;
  expected0.channel = 0;
  expected0.note = note;
  expected1 = mcm_scale<7, 32>(in0.pressure);

  std::array const input{std::bit_cast<std::uint32_t>(in0)};
  EXPECT_THAT(convert(input),
              ElementsAre(std::bit_cast<std::uint32_t>(expected0), std::bit_cast<std::uint32_t>(expected1)));
}

TEST(UMPToMidi2, PitchBend) {
  constexpr auto pb14 = 0b0010101010101010U;  // A 14-bit value for the pitch bend

  midi2::types::m1cvm::pitch_bend m1;
  auto& m10 = get<0>(m1.w);
  m10.group = 0;
  m10.channel = 0;
  m10.lsb_data = pb14 & ((1 << 7) - 1);
  m10.msb_data = pb14 >> 7;

  midi2::types::m2cvm::pitch_bend m2;
  auto& m20 = get<0>(m2.w);
  auto& m21 = get<1>(m2.w);
  m20.group = 0;
  m20.channel = 0;
  m21 = midi2::mcm_scale<14, 32>(pb14);

  std::array const input{std::bit_cast<std::uint32_t>(m10)};
  EXPECT_THAT(convert(input), ElementsAre(std::bit_cast<std::uint32_t>(m20), std::bit_cast<std::uint32_t>(m21)));
}

}  // end anonymous namespace
