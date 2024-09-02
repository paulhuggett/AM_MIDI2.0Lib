//
//  test_ci7.cpp
//  midi2
//
//  Created by Paul Bowen-Huggett on 01/09/2024.
//

#include "midi2/ci7text.hpp"

#include <string>

#include <gtest/gtest.h>

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


template <icubaby::unicode_char_type T>
class CI7Text : public testing::Test {
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

  transcoder<T> t_;
};


using OutputCharTypes = testing::Types<char32_t, char16_t, char8_t>;
TYPED_TEST_SUITE(CI7Text, OutputCharTypes, OutputTypeNames);

TYPED_TEST(CI7Text, SimpleASCII) {
  std::u32string const str32 {
    static_cast<char32_t> ('H'),
    static_cast<char32_t> ('e'),
    static_cast<char32_t> ('l'),
    static_cast<char32_t> ('l'),
    static_cast<char32_t> ('o'),
  };
  EXPECT_EQ(this->convert (str32), "Hello");
  EXPECT_FALSE (this->t_.partial());
  EXPECT_TRUE (this->t_.well_formed());
}

TYPED_TEST(CI7Text, BeatNote) {
  constexpr auto eighth_note = char32_t{0x266A};
  std::u32string const str32 {
    static_cast<char32_t> ('B'),
    static_cast<char32_t> ('e'),
    static_cast<char32_t> ('a'),
    static_cast<char32_t> ('t'),
    static_cast<char32_t> (eighth_note),
  };
  EXPECT_EQ(this->convert (str32), "Beat\\u266A");
}

TYPED_TEST(CI7Text, Only5BytesLeft) {
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

TYPED_TEST(CI7Text, Backslash) {
  std::u32string const str32 {
    static_cast<char32_t> ('a'),
    static_cast<char32_t> ('\\'),
    static_cast<char32_t> ('b'),
  };
  EXPECT_EQ(this->convert (str32), R"(a\\b)");
}

TYPED_TEST(CI7Text, ThresholdBetweenASCIIAndEscapes) {
  std::u32string const str32 { static_cast<char32_t> (0x7F), static_cast<char32_t> (0x80), };
  EXPECT_EQ(this->convert (str32), "\x7F\\u0080");
}

TYPED_TEST(CI7Text, Utf16SurrogatePairs) {
  constexpr auto linear_bs_syllable_b015_mo = char32_t{0x10017};
  constexpr auto linear_bs_syllable_b030_mi = char32_t{0x1001B};
  transcoder<TypeParam> t;
  // A pair of characters from the Linear B script which must be encoded as UTF-16 surrogate pairs.
  std::u32string const str32 { linear_bs_syllable_b015_mo, linear_bs_syllable_b030_mi, };
  EXPECT_EQ(this->convert (str32), R"(\uD800\uDC17\uD800\uDC1B)");
}

