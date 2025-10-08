//===-- ci7 -------------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ci/ci7text.hpp"
#include "midi2/utils.hpp"

// Standard library
#include <string>

// Google Test/Mock/FuzzTest
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

using namespace std::string_view_literals;

// A specialization of the gtest GetTypeName<char8_t>() function. This is required for compiling with (at least)
// Xcode 14.1/15.2 where we have a link error due to missing typeinfo for char8_t. This code should be removed once it
// is no longer needed for any of our targets.
#if defined(__cpp_char8_t) && defined(__cpp_lib_char8_t)
namespace testing::internal {

template <> inline std::string GetTypeName<char8_t>() {
  return "char8_t";
}

}  // end namespace testing::internal
#endif  // defined(__cpp_char8_t) && defined(__cpp_lib_char8_t)

/// A type that is always false. Used to improve the failure mesages from
/// static_assert().
template <typename... T> [[maybe_unused]] constexpr bool always_false = false;

namespace {

class OutputTypeNames {
public:
  template <typename T> static std::string GetName(int index) {
    (void)index;
    if constexpr (std::is_same<T, char8_t>()) {
      return "char8_t";
    } else if constexpr (std::is_same<T, char16_t>()) {
      return "char16_t";
    } else if constexpr (std::is_same<T, char32_t>()) {
      return "char32_t";
    } else {
      static_assert(always_false<T>, "non-exhaustive visitor");
      midi2::unreachable();
    }
  }
};

template <midi2::icubaby::unicode_char_type T> class CI7TextEncode : public testing::Test {
public:
  std::string convert(std::u32string const& in32) {
    std::basic_string<T> out;
    std::ranges::copy(in32 | midi2::icubaby::views::transcode<char32_t, T>, std::back_inserter(out));

    std::string output;
    auto dest = std::back_inserter(output);
    for (auto const c : out) {
      dest = t_(c, dest);
    }
    t_.end_cp(dest);
    return output;
  }
  auto const& transcoder() const { return t_; }

private:
  midi2::transcoder<T, char> t_;
};

using OutputCharTypes = testing::Types<char32_t, char16_t, char8_t>;

// NOLINTNEXTLINE
TYPED_TEST_SUITE(CI7TextEncode, OutputCharTypes, OutputTypeNames);

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, SimpleASCII) {
  std::u32string const str32{
      'H', 'e', 'l', 'l', 'o',
  };
  EXPECT_EQ(this->convert(str32), "Hello");
  EXPECT_FALSE(this->transcoder().partial());
  EXPECT_TRUE(this->transcoder().well_formed());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, BeatNote) {
  constexpr auto eighth_note = char32_t{0x266A};
  std::u32string const str32{
      'B', 'e', 'a', 't', eighth_note,
  };
  EXPECT_EQ(this->convert(str32), "Beat\\u266A");
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, Only5BytesLeft) {
  constexpr auto cjk_unified_ideograph_6B8B = char32_t{0x6B8B};
  constexpr auto hiragana_letter_ri = char32_t{0x308A};
  constexpr auto hiragana_letter_wa = char32_t{0x308F};
  constexpr auto hiragana_letter_zu = char32_t{0x305A};
  constexpr auto hiragana_letter_ka = char32_t{0x304B};
  constexpr auto katakana_letter_ba = char32_t{0x30D0};
  constexpr auto katakana_letter_i = char32_t{0x30A4};
  constexpr auto katakana_letter_to = char32_t{0x30C8};
  std::u32string const str32{
      cjk_unified_ideograph_6B8B, hiragana_letter_ri, hiragana_letter_wa,
      hiragana_letter_zu,         hiragana_letter_ka, char32_t{'5'},
      katakana_letter_ba,         katakana_letter_i,  katakana_letter_to,
  };
  EXPECT_EQ(this->convert(str32), R"(\u6B8B\u308A\u308F\u305A\u304B5\u30D0\u30A4\u30C8)");
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, Backslash) {
  std::u32string const str32{
      'a',
      '\\',
      'b',
  };
  EXPECT_EQ(this->convert(str32), R"(a\\b)");
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, ThresholdBetweenASCIIAndEscapes) {
  std::u32string const str32{
      static_cast<char32_t>(0x7F),
      static_cast<char32_t>(0x80),
  };
  EXPECT_EQ(this->convert(str32), "\x7F\\u0080");
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, Utf16SurrogatePairs) {
  constexpr auto linear_bs_syllable_b015_mo = char32_t{0x10017};
  constexpr auto linear_bs_syllable_b030_mi = char32_t{0x1001B};
  // A pair of characters from the Linear B script which must be encoded as UTF-16 surrogate pairs.
  std::u32string const str32{
      linear_bs_syllable_b015_mo,
      linear_bs_syllable_b030_mi,
  };
  EXPECT_EQ(this->convert(str32), R"(\uD800\uDC17\uD800\uDC1B)");
}

template <typename T> class CI7TextDecode : public testing::Test {
protected:
  std::basic_string<T> convert(midi2::transcoder<char, T>& t2, std::string_view input) {
    std::basic_string<T> output;
    auto dest = std::back_inserter(output);
    for (auto const c : input) {
      dest = t2(c, dest);
    }
    t2.end_cp(dest);
    return output;
  }

  std::basic_string<T> expected(std::u32string_view in32) {
    std::basic_string<T> out;
    std::ranges::copy(in32 | midi2::icubaby::views::transcode<char32_t, T>, std::back_inserter(out));
    return out;
  }
};

// NOLINTNEXTLINE
TYPED_TEST_SUITE(CI7TextDecode, OutputCharTypes, OutputTypeNames);
// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, Empty) {
  midi2::transcoder<char, TypeParam> t2;
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, Hello) {
  auto const expected = this->expected(std::u32string{
      'H',
      'e',
      'l',
      'l',
      'o',
  });
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, "Hello"sv);
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
  EXPECT_EQ(output, expected);
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, BackslashEscape) {
  auto const expected = this->expected(std::u32string{
      'a',
      '\\',
      'b',
  });
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(a\\b)"sv);
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
  EXPECT_EQ(output, expected);
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, BeatNote) {
  constexpr auto eighth_note = char32_t{0x266A};
  auto const expected = this->expected(std::u32string{
      'B',
      'e',
      'a',
      't',
      eighth_note,
  });
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, "Beat\\u266A"sv);
  EXPECT_EQ(output, expected);
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, Only5BytesLeft) {
  constexpr auto cjk_unified_ideograph_6B8B = char32_t{0x6B8B};
  constexpr auto hiragana_letter_ri = char32_t{0x308A};
  constexpr auto hiragana_letter_wa = char32_t{0x308F};
  constexpr auto hiragana_letter_zu = char32_t{0x305A};
  constexpr auto hiragana_letter_ka = char32_t{0x304B};
  constexpr auto katakana_letter_ba = char32_t{0x30D0};
  constexpr auto katakana_letter_i = char32_t{0x30A4};
  constexpr auto katakana_letter_to = char32_t{0x30C8};
  auto const expected = this->expected(std::u32string{
      cjk_unified_ideograph_6B8B,
      hiragana_letter_ri,
      hiragana_letter_wa,
      hiragana_letter_zu,
      hiragana_letter_ka,
      char32_t{'5'},
      katakana_letter_ba,
      katakana_letter_i,
      katakana_letter_to,
  });
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(\u6B8B\u308A\u308F\u305A\u304B5\u30D0\u30A4\u30C8)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, PartialHexMidString) {
  auto const expected = this->expected(std::u32string{
      'B',
      'e',
      'a',
      't',
      'N',
      'o',
      't',
      'e',
  });
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, "Beat\\u26 Note"sv);
  EXPECT_EQ(output, expected);
  EXPECT_FALSE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, PartialHexAtEndOfString) {
  auto const expected = this->expected(std::u32string{
      'B',
      'e',
      'a',
      't',
  });
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, "Beat\\u26"sv);
  EXPECT_EQ(output, expected);
  EXPECT_FALSE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, Utf16SurrogatePairs) {
  constexpr auto linear_bs_syllable_b015_mo = char32_t{0x10017};
  constexpr auto linear_bs_syllable_b030_mi = char32_t{0x1001B};
  auto const expected = this->expected(std::u32string{
      linear_bs_syllable_b015_mo,
      linear_bs_syllable_b030_mi,
  });
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(\uD800\uDC17\uD800\uDC1B)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, MissingLowSurrogateAtEnd) {
  auto const expected = this->expected(std::u32string{'A', midi2::icubaby::replacement_char});
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(A\uD800)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_FALSE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, MissingLowSurrogateFollowedByEscape) {
  auto const expected = this->expected(std::u32string{'A', midi2::icubaby::replacement_char, '\n'});
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(A\uD800\n)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_FALSE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, MissingLowSurrogateFollowedByNormal) {
  auto const expected = this->expected(std::u32string{'A', midi2::icubaby::replacement_char, 'B'});
  midi2::transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(A\uD800B)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_FALSE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

template <typename OutputEncoding, typename InputEncoding>
std::vector<OutputEncoding> convert(std::vector<InputEncoding> const& input) {
  midi2::transcoder<InputEncoding, OutputEncoding> t2;

  std::vector<OutputEncoding> output;
  auto dest = std::back_inserter(output);
  for (auto const c : input) {
    dest = t2(c, dest);
  }
  t2.end_cp(dest);
  return output;
}

void RoundTrip(std::vector<char32_t> const& input) {
  std::vector<char32_t> sanitized_input;
  sanitized_input.reserve(input.size());
  std::ranges::copy_if(input, std::back_inserter(sanitized_input), [](char32_t cp) {
    return cp <= midi2::icubaby::max_code_point && !midi2::icubaby::is_surrogate(cp);
  });

  auto const intermediate = convert<char>(sanitized_input);
  auto const output = convert<char32_t>(intermediate);
  EXPECT_THAT(output, testing::ContainerEq(sanitized_input));
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(CI7RoundTrip, RoundTrip);
#endif
// NOLINTNEXTLINE
TEST(CI7RoundTrip, Empty) {
  RoundTrip(std::vector<char32_t>{});
}

}  // end anonymous namespace
