#include <gmock/gmock.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#include "midi2/umpToBytestream.h"

namespace {

template <typename InputIterator>
auto convert(InputIterator first, InputIterator last) {
  midi2::umpToBytestream ump2bs;
  std::vector<std::uint8_t> output;
  std::for_each(first, last, [&output, &ump2bs](std::uint32_t const ump) {
    ump2bs.UMPStreamParse(ump);
    while (ump2bs.availableBS()) {
      output.push_back(ump2bs.readBS());
    }
  });
  return output;
}

using testing::ElementsAre;

TEST(UMPToBytestream, NoteOn) {
  std::array const input{std::uint32_t{0x20816050}, std::uint32_t{0x20817070}};
  EXPECT_THAT(
      convert(std::begin(input), std::end(input)),
      ElementsAre(std::uint8_t{0x81}, std::uint8_t{0x60}, std::uint8_t{0x50},
                  std::uint8_t{0x81}, std::uint8_t{0x70}, std::uint8_t{0x70}));
}

TEST(UMPToBytestream, SystemMessageOneByte) {
  std::array const input{std::uint32_t{0x10F80000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAre(std::uint8_t{0xF8}));
}

TEST(UMPToBytestream, PCTwoBytes) {
  std::array const input{std::uint32_t{0x20C64000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAre(std::uint8_t{0xC6}, std::uint8_t{0x40}));
}

TEST(UMPToBytestream, Sysex) {
  std::array const input{std::uint32_t{0x30167e7f}, std::uint32_t{0x0d70024b},
                         std::uint32_t{0x3026607a}, std::uint32_t{0x737f7f7f},
                         std::uint32_t{0x30267f7d}, std::uint32_t{0x00000000},
                         std::uint32_t{0x30260100}, std::uint32_t{0x00000300},
                         std::uint32_t{0x30360000}, std::uint32_t{0x10000000}};
  EXPECT_THAT(convert(std::begin(input), std::end(input)),
              ElementsAre(0xF0, 0x7E, 0x7F, 0x0D, 0x70, 0x02, 0x4B, 0x60, 0x7A,
                          0x73, 0x7F, 0x7F, 0x7F, 0x7F, 0x7D, 0x00, 0x00, 0x00,
                          0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                          0x10, 0x00, 0x00, 0x00, 0xF7));
}

}  // end anonymous namespace
