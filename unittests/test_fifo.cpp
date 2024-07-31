// DUT
#include "midi2/fifo.h"

// Standard library
#include <queue>

// Google Test/Mock
#include <gmock/gmock.h>

namespace {

TEST(IsPowerOfTwo, IsPowerOfTwo) {
  using midi2::is_power_of_two;
  EXPECT_FALSE(is_power_of_two(0U));
  EXPECT_TRUE(is_power_of_two(1U));
  EXPECT_TRUE(is_power_of_two(2U));
  EXPECT_FALSE(is_power_of_two(3U));
  EXPECT_TRUE(is_power_of_two(4U));
  EXPECT_FALSE(is_power_of_two(7U));
  EXPECT_TRUE(is_power_of_two(8U));
  EXPECT_TRUE(is_power_of_two(65536U));
  EXPECT_FALSE(is_power_of_two(65537U));
}
TEST(BitsRequired, BitsRequired) {
  using midi2::bits_required;
  EXPECT_EQ(bits_required(0U), 0U);
  EXPECT_EQ(bits_required(1U), 1U);
  EXPECT_EQ(bits_required(2U), 2U);
  EXPECT_EQ(bits_required(3U), 2U);
  EXPECT_EQ(bits_required(4U), 3U);
  EXPECT_EQ(bits_required(7U), 3U);
  EXPECT_EQ(bits_required(8U), 4U);
  EXPECT_EQ(bits_required(65534U), 16U);
  EXPECT_EQ(bits_required(65535U), 16U);
  EXPECT_EQ(bits_required(65536U), 17U);
}

std::vector<unsigned> toVector(std::queue<unsigned> q) {
  std::vector<unsigned> result;
  result.reserve(q.size());
  while (!q.empty()) {
    result.push_back(q.front());
    q.pop();
  }
  return result;
}
template <std::uint32_t Elements>
std::vector<unsigned> toVector(midi2::fifo<unsigned, Elements> q) {
  std::vector<unsigned> result;
  result.reserve(q.size());
  while (!q.empty()) {
    result.push_back(q.pop_front());
  }
  return result;
}

template <typename Size> class Fifo : public testing::Test {};

// These types represent the maximum number of elements in the FIFO to be
// tested. They have to be std::integral_constant<> because we need them to be
// compile-time constants for use as template arguments. Values were chosen to
// exercise the boundary values for the choice of bitfield type in the FIFO
// instance.
using TestTypes =
    ::testing::Types<std::integral_constant<unsigned, 2>,   // 2 bits
                     std::integral_constant<unsigned, 4>,   // 3 bits
                     std::integral_constant<unsigned, 8>,   // 4 bits
                     std::integral_constant<unsigned, 128>  // 8 bits
                     >;
TYPED_TEST_SUITE(Fifo, TestTypes);

TYPED_TEST(Fifo, InitialState) {
  midi2::fifo<unsigned, TypeParam::value> fifo;
  EXPECT_TRUE(fifo.empty());
  EXPECT_FALSE(fifo.full());
  EXPECT_EQ(fifo.size(), 0U);
  EXPECT_EQ(fifo.max_size(), TypeParam::value);
}

TYPED_TEST(Fifo, Push) {
  static constexpr auto elements = TypeParam::value;
  midi2::fifo<unsigned, elements> fifo;
  auto ctr = 1U;
  // Push elements until there is space for only one more.
  for (; ctr < elements; ++ctr) {
    EXPECT_TRUE(fifo.push_back(ctr));
    EXPECT_FALSE(fifo.empty());
    EXPECT_FALSE(fifo.full());
    EXPECT_EQ(fifo.size(), ctr);
  }

  EXPECT_TRUE(fifo.push_back(ctr));
  EXPECT_FALSE(fifo.empty());
  EXPECT_TRUE(fifo.full()) << "Container should now be full";
  EXPECT_EQ(fifo.size(), fifo.max_size());

  EXPECT_FALSE(fifo.push_back(ctr))
      << "A push when the FIFO is at maximum capacity should fail";
  EXPECT_FALSE(fifo.empty());
  EXPECT_TRUE(fifo.full());
  EXPECT_EQ(fifo.size(), fifo.max_size());
}

TYPED_TEST(Fifo, Pop) {
  static constexpr auto elements = TypeParam::value;
  midi2::fifo<unsigned, elements> fifo;
  // Push elements onto the container to fill it.
  for (auto ctr = 0U; ctr < elements; ++ctr) {
    fifo.push_back(ctr);
  }
  // Pop those elements checking that we get the expected values.
  for (auto ctr = 0U; ctr < elements; ++ctr) {
    ASSERT_FALSE(fifo.empty());
    ASSERT_EQ(fifo.pop_front(), ctr);
  }
  EXPECT_TRUE(fifo.empty());
}

TYPED_TEST(Fifo, PushTwoPopOne) {
  static constexpr auto elements = TypeParam::value;

  // It's generally good practice to avoid putting logic into unit tests,
  // a guideline this test blatantly flaunts. The algorithm repeatedly pushes
  // two values into the FIFO then pops a single value out. This is done to
  // both a FIFO and to a std::queue<> instance. We expect their contents to
  // always match.
  //
  // The motivation is that, for 'elements' greater than two, we will cause
  // the FIFO's internal container to wrap and fully exercise the full/empty
  // conditions.
  midi2::fifo<unsigned, elements> fifo;
  std::queue<unsigned> queue;
  // A monotonically increasing value for pushing into the containers.
  auto value = 0U;
  for (auto iteration = 0U; iteration < elements - 1U; ++iteration) {
    // Push two values
    EXPECT_LE(fifo.size() + 2U, elements)
        << "Not enough room to push two values";
    for (auto i = 0; i < 2; ++i) {
      ++value;
      EXPECT_TRUE(fifo.push_back(value));
      queue.push(value);
      EXPECT_EQ(fifo.size(), queue.size())
          << "Queue and FIFO should have the same number of elements";
    }
    EXPECT_THAT(toVector(fifo), testing::ContainerEq(toVector(queue)));
    // Pop one.
    ASSERT_FALSE(fifo.empty());
    EXPECT_EQ(fifo.pop_front(), queue.front());
    queue.pop();
    EXPECT_THAT(toVector(fifo), testing::ContainerEq(toVector(queue)));
  }
}

TYPED_TEST(Fifo, PushUntilFullPopOne) {
  static constexpr auto elements = TypeParam::value;

  // This test is much like the PushTwoPopOne version except that this time we
  // push until the FIFO reports that it is full before popping all but one
  // element.
  midi2::fifo<unsigned, elements> fifo;
  std::queue<unsigned> queue;
  // A monotonically increasing value for pushing into the containers.
  auto value = 0U;
  for (auto iteration = 0U; iteration < elements - 1U; ++iteration) {
    while (!fifo.full()) {
      ++value;
      EXPECT_TRUE(fifo.push_back(value));
      queue.push(value);
      EXPECT_EQ(fifo.size(), queue.size())
          << "Queue and FIFO should have the same number of elements";
    }
    EXPECT_THAT(toVector(fifo), testing::ContainerEq(toVector(queue)));

    while (fifo.size() > 1) {
      EXPECT_EQ(fifo.pop_front(), queue.front());
      queue.pop();
      EXPECT_EQ(fifo.size(), queue.size())
          << "Queue and FIFO should have the same number of elements";
    }
    EXPECT_THAT(toVector(fifo), testing::ContainerEq(toVector(queue)));
  }
}

}  // end anonymous namespace
