// DUT
#include "bytestreamToUMP.h"

// Standard library
#include <algorithm>
#include <array>
#include <vector>

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace {

using testing::ElementsAre;
using testing::IsEmpty;

class BytestreamToUMP : public testing::Test {
protected:
  template <std::size_t Size>
  auto convert(bytestreamToUMP&& bs2ump,
               std::array<std::uint8_t, Size> const& input) {
    std::vector<std::uint32_t> output;
    for (std::uint8_t const b : input) {
      bs2ump.bytestreamParse(b);
      while (bs2ump.availableUMP()) {
        output.push_back(bs2ump.readUMP());
      }
    }
    return output;
  }
};

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, NoteOnWithRunningStatus) {
  std::array const input{std::uint8_t{0x81}, std::uint8_t{0x60},
                         std::uint8_t{0x50}, std::uint8_t{0x70},
                         std::uint8_t{0x70}};
  EXPECT_THAT(convert(bytestreamToUMP{}, input),
              ElementsAre(UINT32_C(0x20816050), UINT32_C(0x20817070)));
}

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, SystemMessageOneByte) {
  std::array const input{std::uint8_t{0xF8}};
  EXPECT_THAT(convert(bytestreamToUMP{}, input),
              ElementsAre(UINT32_C(0x10f80000)));
}

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, PCTwoBytes) {
  std::array<uint8_t, 2> const input{0xC6, 0x40};
  EXPECT_THAT(convert(bytestreamToUMP{}, input),
              ElementsAre(UINT32_C(0x20c64000)));
}

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, SysEx) {
  std::array const input{
      std::uint8_t{0xF0}, std::uint8_t{0x7E}, std::uint8_t{0x7F},
      std::uint8_t{0x0D}, std::uint8_t{0x70}, std::uint8_t{0x02},
      std::uint8_t{0x4B}, std::uint8_t{0x60}, std::uint8_t{0x7A},
      std::uint8_t{0x73}, std::uint8_t{0x7F}, std::uint8_t{0x7F},
      std::uint8_t{0x7F}, std::uint8_t{0x7F}, std::uint8_t{0x7D},
      std::uint8_t{0x00}, std::uint8_t{0x00}, std::uint8_t{0x00},
      std::uint8_t{0x00}, std::uint8_t{0x01}, std::uint8_t{0x00},
      std::uint8_t{0x00}, std::uint8_t{0x00}, std::uint8_t{0x03},
      std::uint8_t{0x00}, std::uint8_t{0x00}, std::uint8_t{0x00},
      std::uint8_t{0x10}, std::uint8_t{0x00}, std::uint8_t{0x00},
      std::uint8_t{0x00}, std::uint8_t{0xF7}};
  EXPECT_THAT(convert(bytestreamToUMP{}, input),
              ElementsAre(UINT32_C(0x30167e7f), UINT32_C(0x0d70024b),
                          UINT32_C(0x3026607a), UINT32_C(0x737f7f7f),
                          UINT32_C(0x30267f7d), UINT32_C(0x00000000),
                          UINT32_C(0x30260100), UINT32_C(0x00000300),
                          UINT32_C(0x30360000), UINT32_C(0x10000000)));
}

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, MT4NoteOnWithRunningStatus) {
  std::array const input{std::uint8_t{0x81}, std::uint8_t{0x60},
                         std::uint8_t{0x50}, std::uint8_t{0x70},
                         std::uint8_t{0x70}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40816000), UINT32_C(0xA0820000),
                          UINT32_C(0x40817000), UINT32_C(0xe1860000)));
}

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, MT4PCTwoBytes) {
  std::array const input{std::uint8_t{0xC6}, std::uint8_t{0x40}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40c60000), UINT32_C(0x40000000)));
}

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, MT4PC2BytesWithBankMsbLsb) {
  std::array const input{std::uint8_t{0xB6}, std::uint8_t{0x00},
                         std::uint8_t{0x01}, std::uint8_t{0x20},
                         std::uint8_t{0x0A}, std::uint8_t{0xC6},
                         std::uint8_t{0x41}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40c60001), UINT32_C(0x4100010A)));
}

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, MT4RPN) {
  std::array const input{std::uint8_t{0xB6}, std::uint8_t{0x65},
                         std::uint8_t{0x00}, std::uint8_t{0x64},
                         std::uint8_t{0x06}, std::uint8_t{0x06},
                         std::uint8_t{0x08}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input),
              ElementsAre(UINT32_C(0x40260006), UINT32_C(0x10000000)));
}

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, Midi1TwoNoteOffs) {
  std::array const input{std::uint8_t{0x80}, std::uint8_t{0x80}};
  EXPECT_THAT(convert(bytestreamToUMP{}, input), IsEmpty());
}

// NOLINTNEXTLINE
TEST_F(BytestreamToUMP, Midi2TwoNoteOffs) {
  std::array const input{std::uint8_t{0x80}, std::uint8_t{0x80}};
  EXPECT_THAT(convert(bytestreamToUMP{true}, input), IsEmpty());
}

void NeverCrashes(std::vector<std::uint8_t> const& bytes) {
  // This test simply gets bytestreamToUMP to consume a random buffer.
  bytestreamToUMP bs2ump;
  for (auto const b : bytes) {
    bs2ump.bytestreamParse(b);
    while (bs2ump.availableUMP()) {
      bs2ump.readUMP();
    }
  }
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(BytestreamToUMPFuzz, NeverCrashes);
#endif

}  // end anonymous namespace
