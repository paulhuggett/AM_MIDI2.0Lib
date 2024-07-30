// DUT
#include "midi2/umpToMIDI1Protocol.h"

// Standard Library
#include <algorithm>
#include <vector>

// Google Test/Mock
#include <gmock/gmock.h>

namespace {

template <typename InputIterator>
auto convert(InputIterator first, InputIterator last) {
  std::vector<std::uint32_t> output;
  midi2::umpToMIDI1Protocol UMP2M1;
  std::for_each(first, last, [&output, &UMP2M1](std::uint32_t const ump) {
    UMP2M1.UMPStreamParse(ump);
    while (UMP2M1.availableUMP()) {
      output.push_back(UMP2M1.readUMP());
    }
  });
  return output;
}

using testing::ElementsAre;
using testing::ElementsAreArray;

TEST(UMPToMIDI1, Foo) {
  std::array const input{std::uint32_t{0x20816050}, std::uint32_t{0x20817070}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAreArray(input));
}
TEST(UMPToMIDI1, Sysex) {
  std::array const input{std::uint32_t{0x30167E7F}, std::uint32_t{0x0D70024B},
                         std::uint32_t{0x3026607A}, std::uint32_t{0x737F7F7F},
                         std::uint32_t{0x30267F7D}, std::uint32_t{0x00000000},
                         std::uint32_t{0x30260100}, std::uint32_t{0x00000300},
                         std::uint32_t{0x30360000}, std::uint32_t{0x10000000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAreArray(input));
}
TEST(UMPToMIDI1, SystemMessageOneByte) {
  std::array const input{std::uint32_t{0x10F80000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAreArray(input));
}
TEST(UMPToMIDI1, MT4) {
  std::array const input{std::uint32_t{0x40904000}, std::uint32_t{0xC1040000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAre(0x20904060));
}

}  // end anonymous namespace
