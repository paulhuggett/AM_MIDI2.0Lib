//
//  test_ci7.cpp
//  midi2
//
//  Created by Paul Bowen-Huggett on 01/09/2024.
//

#include "midi2/ci7text.hpp"

#include <string>

#include <gtest/gtest.h>

using namespace std::string_view_literals;

// A specialization of the gtest GetTypeName<char8_t>() function. This is required for compiling with (at least)
// Xcode 14.1/15.2 where we have a link error due to missing typeinfo for char8_t. This code should be removed once it
// is no longer needed for any of our targets.
#if defined(__cpp_char8_t) && defined(__cpp_lib_char8_t)
namespace testing::internal {

template <> inline std::string GetTypeName<char8_t> () {
  return "char8_t";
}

}  // end namespace testing::internal
#endif

[[noreturn, maybe_unused]] inline void unreachable () {
  // Uses compiler specific extensions if possible. Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#ifdef __GNUC__  // GCC, Clang, ICC
  __builtin_unreachable ();
#elif defined(_MSC_VER)  // MSVC
  __assume (false);
#endif
}
/// A type that is always false. Used to improve the failure mesages from
/// static_assert().
template <typename... T> [[maybe_unused]] constexpr bool always_false = false;

class OutputTypeNames {
public:
  template <typename T> static std::string GetName (int index) {
    (void)index;
    if constexpr (std::is_same<T, char8_t> ()) {
      return "char8_t";
    } else if constexpr (std::is_same<T, char16_t> ()) {
      return "char16_t";
    } else if constexpr (std::is_same<T, char32_t> ()) {
      return "char32_t";
    } else {
      static_assert (always_false<T>, "non-exhaustive visitor");
      unreachable ();
    }
  }
};

using OutputTypes = testing::Types<icubaby::char8, char16_t, char32_t>;

template <icubaby::unicode_char_type T> class CI7TextEncode : public testing::Test {
protected:
  std::string convert (std::u32string const & in32) {
    std::basic_string<T> out;
    std::ranges::copy(in32 | icubaby::views::transcode<char32_t, T>, std::back_inserter(out));

    std::string output;
    auto dest = std::back_inserter(output);
    for (auto const c: out) {
      dest = t_(c, dest);
    }
    t_.end_cp(dest);
    return output;
  }

  transcoder<T, char> t_;
};

using OutputCharTypes = testing::Types<char32_t, char16_t, char8_t>;
// NOLINTNEXTLINE
TYPED_TEST_SUITE(CI7TextEncode, OutputCharTypes, OutputTypeNames);

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, SimpleASCII) {
  std::u32string const str32{
      'H', 'e', 'l', 'l', 'o',
  };
  EXPECT_EQ(this->convert (str32), "Hello");
  EXPECT_FALSE(this->t_.partial());
  EXPECT_TRUE(this->t_.well_formed());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, BeatNote) {
  constexpr auto eighth_note = char32_t{0x266A};
  std::u32string const str32{
      'B', 'e', 'a', 't', eighth_note,
  };
  EXPECT_EQ(this->convert (str32), "Beat\\u266A");
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
    cjk_unified_ideograph_6B8B,
    hiragana_letter_ri,
    hiragana_letter_wa,
    hiragana_letter_zu,
    hiragana_letter_ka,
    char32_t{'5'},
    katakana_letter_ba,
    katakana_letter_i,
    katakana_letter_to,
  };
  EXPECT_EQ(this->convert (str32), R"(\u6B8B\u308A\u308F\u305A\u304B5\u30D0\u30A4\u30C8)");
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, Backslash) {
  std::u32string const str32 {'a', '\\', 'b',};
  EXPECT_EQ(this->convert (str32), R"(a\\b)");
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, ThresholdBetweenASCIIAndEscapes) {
  std::u32string const str32 { static_cast<char32_t> (0x7F), static_cast<char32_t> (0x80), };
  EXPECT_EQ(this->convert (str32), "\x7F\\u0080");
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextEncode, Utf16SurrogatePairs) {
  constexpr auto linear_bs_syllable_b015_mo = char32_t{0x10017};
  constexpr auto linear_bs_syllable_b030_mi = char32_t{0x1001B};
  // A pair of characters from the Linear B script which must be encoded as UTF-16 surrogate pairs.
  std::u32string const str32 { linear_bs_syllable_b015_mo, linear_bs_syllable_b030_mi, };
  EXPECT_EQ(this->convert (str32), R"(\uD800\uDC17\uD800\uDC1B)");
}

template <typename T> class CI7TextDecode : public testing::Test {
protected:
  std::basic_string<T> convert(transcoder<char, T>& t2, std::string_view input) {
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
    std::ranges::copy(in32 | icubaby::views::transcode<char32_t, T>, std::back_inserter(out));
    return out;
  }
};

// NOLINTNEXTLINE
TYPED_TEST_SUITE(CI7TextDecode, OutputCharTypes, OutputTypeNames);
// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, Empty) {
  transcoder<char, TypeParam> t2;
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, Hello) {
  auto const expected = this->expected(std::u32string{
      'H', 'e', 'l', 'l', 'o',
  });
  transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, "Hello"sv);
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
  EXPECT_EQ(output, expected);
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, BackslashEscape) {
  auto const expected = this->expected(std::u32string{'a', '\\', 'b',});
  transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(a\\b)"sv);
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
  EXPECT_EQ(output, expected);
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, BeatNote) {
  constexpr auto eighth_note = char32_t{0x266A};
  auto const expected = this->expected(std::u32string{
      'B', 'e', 'a', 't', eighth_note,
  });
  transcoder<char, TypeParam> t2;
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
  transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(\u6B8B\u308A\u308F\u305A\u304B5\u30D0\u30A4\u30C8)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, PartialHexMidString) {
  auto const expected = this->expected(std::u32string{
      'B', 'e', 'a', 't', 'N', 'o', 't', 'e',
  });
  transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, "Beat\\u26 Note"sv);
  EXPECT_EQ(output, expected);
  EXPECT_FALSE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, PartialHexAtEndOfString) {
  auto const expected = this->expected(std::u32string{'B', 'e', 'a', 't',});
  transcoder<char, TypeParam> t2;
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
  transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(\uD800\uDC17\uD800\uDC1B)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_TRUE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, MissingLowSurrogateAtEnd) {
  auto const expected = this->expected(std::u32string{'A', icubaby::replacement_char});
  transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(A\uD800)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_FALSE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, MissingLowSurrogateFollowedByEscape) {
  auto const expected = this->expected(std::u32string{'A', icubaby::replacement_char, '\n'});
  transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(A\uD800\n)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_FALSE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}

// NOLINTNEXTLINE
TYPED_TEST(CI7TextDecode, MissingLowSurrogateFollowedByNormal) {
  auto const expected = this->expected(std::u32string{'A', icubaby::replacement_char, 'B'});
  transcoder<char, TypeParam> t2;
  auto const output = this->convert(t2, R"(A\uD800B)"sv);
  EXPECT_EQ(output, expected);
  EXPECT_FALSE(t2.well_formed());
  EXPECT_FALSE(t2.partial());
}
