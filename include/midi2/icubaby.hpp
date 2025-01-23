//*  _         _          _          *
//* (_)__ _  _| |__  __ _| |__ _  _  *
//* | / _| || | '_ \/ _` | '_ \ || | *
//* |_\__|\_,_|_.__/\__,_|_.__/\_, | *
//*                            |__/  *
// Home page:
// https://paulhuggett-icubaby.rtfd.io
//
// MIT License
//
// Copyright (c) 2022-2024 Paul Bowen-Huggett
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// \file   icubaby.hpp
///
/// \brief  A C++ Baby Library to Immediately Convert Unicode. A header only, dependency free,
///         library for C++ 17 or later. Fast, minimal, and easy to use for converting a sequence
///         in any of UTF-8, UTF-16, or UTF-32.
///
/// \mainpage
/// A C++ Library to Immediately Convert Unicode. It is a portable, header-only, dependency-free library for C++ 17 or
/// later. Fast, minimal, and easy to use for converting sequences of text between any of the Unicode UTF encodings. It
/// does not allocate dynamic memory and neither throws or catches exceptions.
///
/// \example view_utf32_to_16.cpp
/// An example showing conversion of an array UTF-32 encoded code points can be converted to UTF-16 using the C++ 20
/// ranges interface.
///
/// \example iterator.cpp
/// The icubaby::iterator<> class offers a familiar output iterator for using a transcoder. Each code unit from the
/// input encoding is written to the iterator and this in turn writes the output encoding to a second iterator. This
/// enables use of standard algorithms such as std::copy() with the library.
///
/// \example bytes_to_utf8.cpp
/// This code converts an array of bytes containing the string "Hello World" in UTF-16 BE with an initial byte order
/// mark first to UTF-8 and then to an array of std::uint_least8_t. We finally copy these values to std::cout.
///
/// \example manual_bytes_to_utf8.cpp
/// This code shows how icubaby makes it straightforward to convert a byte array to a sequence of Unicode code units
/// passing one byte at a time to a transcoder instance. We take the bytes making up the string "Hello World" expressed
/// in big endian UTF-16 (with a byte order marker) and convert them to UTF-8 which is written directly to `std::cout`.

// UTF-8 to UTF-32 conversion is based on the "Flexible and Economical UTF-8
// Decoder" by Bjoern Hoehrmann <bjoern@hoehrmann.de> See
// http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
//
// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MIDI2_ICUBABY_HPP
#define MIDI2_ICUBABY_HPP

#define ICUBABY_INSIDE_NS midi2

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

/// \brief ICUBABY_CXX20 has value 1 when compiling with C++ 20 or later and 0
///   otherwise.
/// \hideinitializer
#if __cplusplus >= 202002L
#define ICUBABY_CXX20 (1)
#elif defined(_MSVC_LANG) && _MSVC_LANG >= 202002L
// MSVC does not set the value of __cplusplus correctly unless the
// /Zc:__cplusplus is supplied. We have to detect C++20 using its
// compiler-specific macros instead.
#define ICUBABY_CXX20 (1)
#else
#define ICUBABY_CXX20 (0)
#endif

#ifndef ICUBABY_CXX20
#include <version>
#endif

/// \brief Defined as 1 if the standard library's __cpp_lib_ranges macro is available and 0 otherwise.
/// \hideinitializer
#ifdef __cpp_lib_ranges
#define ICUBABY_CPP_LIB_RANGES_DEFINED (1)
#else
#define ICUBABY_CPP_LIB_RANGES_DEFINED (0)
#endif

/// \brief Tests for the availability of library support for C++ 20 ranges.
/// \hideinitializer
#define ICUBABY_HAVE_RANGES (ICUBABY_CPP_LIB_RANGES_DEFINED && __cpp_lib_ranges >= 201811L)
#if ICUBABY_HAVE_RANGES
#include <ranges>
#endif

/// \brief Defined as true if compiler and library support for concepts are available.
/// \hideinitializer
#ifdef __cpp_concepts
#define ICUBABY_CPP_CONCEPTS_DEFINED (1)
#else
#define ICUBABY_CPP_CONCEPTS_DEFINED (0)
#endif

/// \brief Defined as 1 if the standard library's __cpp_lib_concepts macro is available and 0 otherwise.
/// \hideinitializer
#ifdef __cpp_lib_concepts
#define ICUBABY_CPP_LIB_CONCEPTS_DEFINED (1)
#else
#define ICUBABY_CPP_LIB_CONCEPTS_DEFINED (0)
#endif

/// \brief A macro that evaluates true if the compiler and library have support for C++ 20 concepts.
/// \hideinitializer
#define ICUBABY_HAVE_CONCEPTS                                                                       \
  (ICUBABY_CPP_CONCEPTS_DEFINED && __cpp_concepts >= 201907L && ICUBABY_CPP_LIB_CONCEPTS_DEFINED && \
   __cpp_lib_concepts >= 202002L)

#if ICUBABY_HAVE_CONCEPTS
#include <concepts>
/// \brief Defined as `requires x` if C++ 20 concepts are supported and as nothing otherwise.
/// \hideinitializer
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ICUBABY_REQUIRES(x) requires x
/// \brief Defined as `std::output_iterator<x>` if C++ 20 concepts are supported and as `typename` otherwise.
/// \hideinitializer
///
/// Used as a shortcut to restrict a specific template argument to be an output iterator in C++ 20 and simply
/// declaring a typename in C++ 17.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ICUBABY_CONCEPT_OUTPUT_ITERATOR(x) std::output_iterator<x>
/// \brief A convenience macro for defining a template argument that must be icubaby::unicode_char_type.
/// \hideinitializer
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ICUBABY_CONCEPT_UNICODE_CHAR_TYPE unicode_char_type
#else
#define ICUBABY_REQUIRES(x)
#define ICUBABY_CONCEPT_OUTPUT_ITERATOR(x) typename
#define ICUBABY_CONCEPT_UNICODE_CHAR_TYPE typename
#endif  // ICUBABY_HAVE_CONCEPTS

/// \brief Defined as `[[no_unique_address]]` if the attribute is supported and
///   as nothing otherwise.
/// \hideinitializer
#if ICUBABY_CXX20
#define ICUBABY_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define ICUBABY_NO_UNIQUE_ADDRESS
#endif

#ifdef ICUBABY_INSIDE_NS
namespace ICUBABY_INSIDE_NS {
#endif

/// \brief The namespace for the icubaby library.
namespace icubaby {

/// \brief Private implementation details of the icubaby interface.
///
/// Functions and types defined in this namespace are not part of the icubaby public interface
/// and should not be used in client code. They may change at any time!
namespace details {

/// \brief A compile-time list of types.
///
/// An instance of type_list represents an element in a list. It is somewhat
/// like a cons cell in Lisp: it has two slots, and each slot holds a type.
/// A list is formed when a series of type_list instances are chained together,
/// so that each cell refers to the next one. There is one type_list instance
/// for each list member. The 'first' member holds a member type and the 'rest'
/// field is used to chain together type_list instances. The end of the list is
/// represented by a type_list specialization which takes no arguments and
/// contains no members.
template <typename... Types> struct type_list;

/// \brief A compile-time list of types. This specialization defines the end of the list.
/// \see type_list, type_list<First, Rest>.
template <> struct type_list<> {};

/// \brief A compile-time list of types. This specialization holds a member of the list.
/// \see type_list, type_list<>.
template <typename First, typename Rest> struct type_list<First, Rest> {
  using first = First;  ///< The first member of a list of types.
  using rest = Rest;    ///< The remaining members of the type list.
};

#if ICUBABY_HAVE_CONCEPTS
/// \brief Defines the requirements of a type that is a member of a type list.
///
/// An element in a type list must contain member types names 'first' and
/// 'rest'. The end of the list is given by the type_list<> specialization.
template <typename T>
concept is_type_list = requires {
  typename T::first;
  typename T::rest;
} || std::is_same_v<T, type_list<>>;
#endif

/// \brief Constructs a type_list from a template parameter pack.
/// \see make<>, make<T, Ts...>, make_t
template <typename... Types> struct make;
/// \brief Constructs an empty type_list.
/// \see make, make<T, Ts...>, make_t
template <> struct make<> {
  using type = type_list<>;  ///< An empty type list.
};
/// \brief Constructs a type_list from a template parameter pack.
/// \see make, make<>, make_t
template <typename T, typename... Ts> struct make<T, Ts...> {
  /// A list of types.
  ///
  /// The first element of the list is type \p T: the remaining members
  /// are \p Ts.
  using type = type_list<T, typename make<Ts...>::type>;
};
/// \brief A helper template for make<>.
template <typename... Types> using make_t = typename make<Types...>::type;

/// \brief Yields true if the type list contains a type matching \p Element
///   and false otherwise.
/// \see contains_v
/// \tparam TypeList A list of types (formed by type_list).
/// \tparam Element  The type to be checked.
template <typename TypeList, typename Element>
ICUBABY_REQUIRES(is_type_list<TypeList>)
struct contains : std::bool_constant<std::is_same_v<Element, typename TypeList::first> ||
                                     contains<typename TypeList::rest, Element>::value> {};
/// \brief Yields false: the empty type list does not contain a type matching \p Element.
/// \see contains_v
/// \tparam Element  The type to be checked.
template <typename Element> struct contains<type_list<>, Element> : std::bool_constant<false> {};

/// A helper variable template for contains<>.
template <typename TypeList, typename Element> inline constexpr bool contains_v = contains<TypeList, Element>::value;

}  // end namespace details

/// \brief The type of a UTF-8 code unit.
/// Defined as `char8_t` when the native type is available and `char` otherwise.
/// \hideinitializer
#if defined(__cpp_char8_t) && defined(__cpp_lib_char8_t)
using char8 = char8_t;
#else
using char8 = char;
#endif

/// A UTF-8 string.
using u8string = std::basic_string<char8>;
/// A UTF-8 string_view.
using u8string_view = std::basic_string_view<char8>;

/// A constant for the U+FFFD REPLACEMENT CHARACTER code point
inline constexpr auto replacement_char = char32_t{0xFFFD};
/// A constant for the U+FEFF ZERO WIDTH NO-BREAK SPACE (BYTE ORDER MARK) code point
inline constexpr auto zero_width_no_break_space = char32_t{0xFEFF};
/// A constant for the U+FEFF ZERO WIDTH NO-BREAK SPACE (BYTE ORDER MARK) code point
inline constexpr auto byte_order_mark = zero_width_no_break_space;

/// \brief The number of bits required to represent a code point.
///
/// Starting with Unicode 2.0, characters are encoded in the range
/// U+0000..U+10FFFF, which amounts to a 21-bit code space.
inline constexpr auto code_point_bits = 21U;

/// The code point of the first UTF-16 high surrogate.
inline constexpr auto first_high_surrogate = char32_t{0xD800};
/// The code point of the last UTF-16 high surrogate.
inline constexpr auto last_high_surrogate = char32_t{0xDBFF};
/// The code point of the first UTF-16 low surrogate.
inline constexpr auto first_low_surrogate = char32_t{0xDC00};
/// The code point of the last UTF-16 low surrogate.
inline constexpr auto last_low_surrogate = char32_t{0xDFFF};

/// The number of the last code point.
inline constexpr auto max_code_point = char32_t{0x10FFFF};
static_assert(std::uint_least32_t{1} << code_point_bits > max_code_point);

/// A list of the character types used for UTF-8 UTF-16, and UTF-32 encoded
/// text.
using character_types = details::make_t<char8, char16_t, char32_t>;

/// \brief Checks whether the argument is one of the unicode character types
///
/// Provides the boolean constant `value` which is true if T is one of the unicode character types as defined by
/// icubaby::character_types and false otherwise.
///
/// \tparam T  The type to be checked.
template <typename T> struct is_unicode_char_type : std::bool_constant<details::contains_v<character_types, T>> {};
/// \brief A helper variable template to simplify use of icubaby::is_unicode_char_type.
template <typename T> inline constexpr bool is_unicode_char_type_v = is_unicode_char_type<T>::value;

/// \brief Checks whether the argument is one of the unicode data source types
///
/// Provides the constant `value` which is equal to true if T is one of the types which may contain unicode data
/// otherwise, value is equal to false. The unicode data types are the types allowed by
/// icubaby::is_unicode_char_type_v plus ``std::byte``.
///
/// \tparam T  The type to be checked.
template <typename T>
struct is_unicode_input_type : std::bool_constant<is_unicode_char_type_v<T> || std::is_same_v<T, std::byte>> {};

/// \brief A helper variable template to simplify use of icubaby::is_unicode_input_type.
template <typename T> inline constexpr bool is_unicode_input_v = is_unicode_input_type<T>::value;

#if ICUBABY_HAVE_CONCEPTS

/// \brief Checks whether the argument is one of the unicode character types
///
/// The unicode_char_type concept defines the requires of a type that matches one of the types that denote a
/// Unicode encoding.
template <typename T>
concept unicode_char_type = is_unicode_char_type_v<T>;

/// \brief Checks whether the argument is one of the unicode data source types
///
/// The unicode_char_type concept defines the requires of a type that matches one of the types that denote a
/// Unicode data source.
template <typename T>
concept unicode_input = is_unicode_input_v<T>;

#endif  // ICUBABY_HAVE_CONCEPTS

/// \brief The number of code-units in the longest legal representation of a code-point.
///
/// Provides the constant `value` which is of type `std::size_t`.
///
/// \tparam Encoding The encoding to be used.
template <ICUBABY_CONCEPT_UNICODE_CHAR_TYPE Encoding> struct longest_sequence {};
/// \brief The number of code-units in the longest legal UTF-8 representation of a code-point.
template <> struct longest_sequence<char8> : std::integral_constant<std::size_t, 4> {};
/// \brief The number of code-units in the longest legal UTF-16 representation of a code-point.
template <> struct longest_sequence<char16_t> : std::integral_constant<std::size_t, 2> {};
/// \brief The number of code-units in the longest legal UTF-32 representation of a code-point.
template <> struct longest_sequence<char32_t> : std::integral_constant<std::size_t, 1> {};
/// \brief A helper variable template to simplify use of icubaby::longest_sequence<>.
template <ICUBABY_CONCEPT_UNICODE_CHAR_TYPE Encoding>
inline constexpr auto longest_sequence_v = longest_sequence<Encoding>::value;

/// \brief Returns true if the code point \p code_point represents a UTF-16 high surrogate.
///
/// \param code_point  The code point to be tested.
/// \returns true if the code point \p code_point represents a UTF-16 high surrogate.
[[nodiscard]] constexpr bool is_high_surrogate(char32_t const code_point) noexcept {
  return code_point >= first_high_surrogate && code_point <= last_high_surrogate;
}
/// \brief Returns true if the code point \p code_point represents a UTF-16 low surrogate.
///
/// \param code_point  The code point to be tested.
/// \returns true if the code point \p code_point represents a UTF-16 low surrogate.
[[nodiscard]] constexpr bool is_low_surrogate(char32_t const code_point) noexcept {
  return code_point >= first_low_surrogate && code_point <= last_low_surrogate;
}
/// \brief Returns true if the code point \p code_point represents a UTF-16 low or high surrogate.
///
/// \param code_point  The code point to be tested.
/// \returns true if the code point \p c represents a UTF-16 high or low surrogate.
[[nodiscard]] constexpr bool is_surrogate(char32_t const code_point) noexcept {
  return is_high_surrogate(code_point) || is_low_surrogate(code_point);
}

/// \name Code Point Start
///@{

/// \brief Returns true if \p code_unit represents the start of a multi-byte UTF-8 sequence.
///
/// \param code_unit  The UTF-8 code unit to be tested.
/// \returns true if \p code_unit represents the start of a multi-byte UTF-8 sequence.
[[nodiscard]] constexpr bool is_code_point_start(char8 const code_unit) noexcept {
  static_assert(sizeof(code_unit) == sizeof(std::byte));
  return (static_cast<std::byte>(code_unit) & std::byte{0b1100'0000}) != std::byte{0b1000'0000};
}
/// \brief Returns true if \p code_unit represents the start of a UTF-16 high/low surrogate pair.
///
/// \param code_unit  The UTF-16 code unit to be tested.
/// \returns  true if \p code_unit represents the start of a UTF-16 high/low surrogate pair.
[[nodiscard]] constexpr bool is_code_point_start(char16_t const code_unit) noexcept {
  return !is_low_surrogate(code_unit);
}
/// \brief Returns true if \p code_unit represents a valid UTF-32 code point.
///
/// \param code_unit  The UTF-32 code unit to be tested.
/// \returns  true if \p code_unit represents a valid UTF-32 code point.
[[nodiscard]] constexpr bool is_code_point_start(char32_t const code_unit) noexcept {
  return !is_surrogate(code_unit) && code_unit <= max_code_point;
}
///@}

#if ICUBABY_HAVE_RANGES && ICUBABY_HAVE_CONCEPTS

/// \brief Returns the number of code points in a sequence.
///
/// \note The input sequence must be well formed for the result to be accurate.
/// \tparam Range  An input range.
/// \tparam Proj  Type of the projection applied to elements.
/// \param range The range of the elements to examine.
/// \param proj  Projection to apply to the elements.
/// \returns  The number of code points.
template <std::ranges::input_range Range, typename Proj = std::identity>
  requires unicode_char_type<std::ranges::range_value_t<Range>>
[[nodiscard]] constexpr std::ranges::range_difference_t<Range> length(Range&& range, Proj proj = {}) {
  return std::ranges::count_if(
      std::forward<Range>(range), [](unicode_char_type auto const code_unit) { return is_code_point_start(code_unit); },
      proj);
}

/// \brief Returns the number of code points in a sequence.
///
/// \note The input sequence must be well formed for the result to be accurate.
/// \param first  The start of the range of code units to examine.
/// \param last  The end of the range of code units to examine.
/// \param proj  Projection to apply to the elements.
/// \returns  The number of code points.
template <std::input_iterator I, std::sentinel_for<I> S, typename Proj = std::identity>
  requires unicode_char_type<typename std::iterator_traits<I>::value_type>
[[nodiscard]] constexpr std::iter_difference_t<I> length(I first, S last, Proj proj = {}) {
  return length(std::ranges::subrange{first, last}, proj);
}

#else

/// \brief Returns the number of code points in a sequence.
///
/// \note The input sequence must be well formed for the result to be accurate.
/// \param first  The start of the range of code units to examine.
/// \param last  The end of the range of code units to examine.
/// \returns  The number of code points.
template <typename InputIterator,
          typename = std::enable_if_t<is_unicode_char_type_v<typename std::iterator_traits<InputIterator>::value_type>>>
[[nodiscard]] constexpr typename std::iterator_traits<InputIterator>::difference_type length(InputIterator first,
                                                                                             InputIterator last) {
  return std::count_if(first, last, [](auto c) { return is_code_point_start(c); });
}

#endif  // ICUBABY_HAVE_RANGES && ICUBABY_HAVE_CONCEPTS

#if ICUBABY_HAVE_RANGES && ICUBABY_HAVE_CONCEPTS

/// Returns an iterator to the beginning of the pos'th code point in the range of code-units given by \p range.
///
/// \tparam Range  An input range.
/// \tparam Proj   The type of the projection applied to elements.
/// \param range  The range of code units to examine.
/// \param pos  The number of code points to move.
/// \param proj  Projection to apply to the elements.
/// \returns  Iterator to the start of the selected code point or iterator equal to last if no such element is found.
template <std::ranges::input_range Range, typename Proj = std::identity>
[[nodiscard]] constexpr std::ranges::borrowed_iterator_t<Range> index(Range&& range, std::size_t pos, Proj proj = {}) {
  auto count = std::size_t{0};
  return std::ranges::find_if(
      std::forward<Range>(range),
      [&count, pos](unicode_char_type auto const code_unit) {
        return is_code_point_start(code_unit) ? (count++ == pos) : false;
      },
      proj);
}

/// Returns an iterator to the beginning of the pos'th code point in the code
/// unit sequence [first, last).
///
/// \param first  The start of the range of code units to examine.
/// \param last  The end of the range of code units to examine.
/// \param pos  The number of code points to move.
/// \param proj  Projection to apply to the elements.
/// \returns  An iterator that is 'pos' code points after the start of the range or
///           'last' if the end of the range was encountered.
template <std::input_iterator I, std::sentinel_for<I> S, typename Proj = std::identity>
[[nodiscard]] constexpr I index(I first, S last, std::size_t pos, Proj proj = {}) {
  return index(std::ranges::subrange{first, last}, pos, proj);
}

#else

/// Returns an iterator to the beginning of the pos'th code point in the code
/// unit sequence [first, last).
///
/// \param first  The start of the range of code units to examine.
/// \param last  The end of the range of code units to examine.
/// \param pos  The number of code points to move.
/// \returns  An iterator that is 'pos' code points after the start of the range or
///           'last' if the end of the range was encountered.
template <typename InputIterator,
          typename = std::enable_if_t<is_unicode_char_type_v<typename std::iterator_traits<InputIterator>::value_type>>>
[[nodiscard]] constexpr InputIterator index(InputIterator first, InputIterator last, std::size_t pos) {
  auto count = std::size_t{0};
  return std::find_if(first, last, [&count, pos](auto c) {
    static_assert(is_unicode_char_type_v<std::decay_t<decltype(c)>>);
    return is_code_point_start(c) ? (count++ == pos) : false;
  });
}

#endif  // ICUBABY_HAVE_RANGES && ICUBABY_HAVE_CONCEPTS

#if ICUBABY_HAVE_CONCEPTS
/// \brief Defines the requirements of a type that provides the transcoder interface.
template <typename T>
concept is_transcoder = requires(T coder) {
  typename T::input_type;
  typename T::output_type;
  // we must also have operator() and end_cp() which
  // both take template arguments.
  { coder.well_formed() } -> std::convertible_to<bool>;
  { coder.partial() } -> std::convertible_to<bool>;
};

#endif  // ICUBABY_HAVE_CONCEPTS

template <typename Transcoder, typename OutputIterator>
ICUBABY_REQUIRES((is_transcoder<Transcoder> && std::output_iterator<OutputIterator, typename Transcoder::output_type>))
class iterator;

/// \brief A transcoder takes a sequence of either bytes or Unicode code-units (one of UTF-8, 16 or 32) and
///   converts it to another Unicode encoding.
///
/// Each of the specializations of this template (there is one for each input/output combination) supplies the same
/// interface.
template <typename FromEncoding, typename ToEncoding> class transcoder {
public:
  /// The type of the code units consumed by this transcoder.
  using input_type = FromEncoding;
  /// The type of the code units produced by this transcoder.
  using output_type = ToEncoding;

  /// \anchor transcoder-call-operator
  /// This member function is the heart of the transcoder. It accepts a single byte or code unit in the input encoding
  /// and, once an entire code point has been consumed, produces the equivalent code point expressed in the output
  /// encoding. Malformed input is detected and replaced with the Unicode replacement character (U+FFFD REPLACEMENT
  /// CHARACTER).
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type transcoder::output_type can be written.
  /// \param code_unit  A code unit in the source encoding.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  OutputIterator operator()(input_type code_unit, OutputIterator dest) noexcept;

  /// Call once the entire input sequence has been fed to \ref transcoder-call-operator "operator()". This function
  /// ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type transcoder::output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr OutputIterator end_cp(OutputIterator dest) const;

  /// Call once the entire input sequence has been fed to \ref transcoder-call-operator "operator()". This function
  /// ensures that the sequence did not end with a partial code point and flushes any remaining output.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type transcoder::output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr iterator<transcoder, OutputIterator> end_cp(iterator<transcoder, OutputIterator> dest);

  /// Indicates whether the input was well formed
  /// \returns True if the input was well formed.
  [[nodiscard]] constexpr bool well_formed() const noexcept;

  /// \brief Indicates whether a "partial" code point has been passed to \ref transcoder-call-operator "operator()".
  ///
  /// If true, one or more code units are required to build the complete code point.
  ///
  /// \returns True if a partial code-point has been passed to \ref transcoder-call-operator "operator()" and
  ///   false otherwise.
  [[nodiscard]] constexpr bool partial() const noexcept;
};

/// \brief An output iterator which passes code units being output through a transcoder.
///
/// This iterator simplifies the job of converting unicode representation and storing the results of that conversion.
/// Each time that a code point is recovered from the sequence written to this class, the equivalent sequence is written
/// to the iterator with which the object was constructed.
///
/// For example, a function to convert a UTF-16 string to UTF-8 becomes very
/// simple:
/// ~~~
/// std::u8string utf16_to_8_demo (std::u16string u16) {
///   std::u8string u8;
///   icubaby::t16_8 t;
///   icubaby::iterator out{&t, std::back_inserter(u8)};
///   t.end_cp (std::copy (u16.begin(), u16.end(), out));
///   return u8;
/// }
/// ~~~
///
/// \tparam Transcoder  A transcoder type.
/// \tparam OutputIterator  An output iterator type.
template <typename Transcoder, typename OutputIterator>
ICUBABY_REQUIRES((is_transcoder<Transcoder> && std::output_iterator<OutputIterator, typename Transcoder::output_type>))
class iterator {
public:
  /// Defines this class as fulfilling the requirements of an output iterator.
  using iterator_category = std::output_iterator_tag;
  /// The class is an output iterator and as such does not yield values.
  using value_type = void;
  /// A type that can be used to identify distance between iterators.
  using difference_type = std::ptrdiff_t;
  /// Defines a pointer to the type iterated over (none in the case of this iterator).
  using pointer = void;
  /// Defines a reference to the type iterated over (none in the case of this iterator).
  using reference = void;

  /// Initializes the underlying transcoder and the output iterator to which elements will be written.
  ///
  /// \param transcoder  The underlying transcoder. This class does not take ownership of the pointer.
  /// \param out  An output iterator to which code units produced by the \p transcoder will be written.
  iterator(Transcoder* transcoder, OutputIterator out) : transcoder_{transcoder}, out_{out} {}
  iterator(iterator const& rhs) = default;
  iterator(iterator&& rhs) noexcept = default;

  ~iterator() noexcept = default;

  /// Passes a code unit to the associated transcoder.
  /// \param value The code unit to be passed to the transcoder.
  /// \returns \*this
  iterator& operator=(typename Transcoder::input_type const& value) {
    out_ = (*transcoder_)(value, out_);
    return *this;
  }

  iterator& operator=(iterator const& rhs) = default;
  iterator& operator=(iterator&& rhs) noexcept = default;

  /// \brief no-op
  /// \returns \*this
  constexpr iterator& operator*() noexcept { return *this; }
  /// \brief no-op
  /// \returns \*this
  constexpr iterator& operator++() noexcept { return *this; }
  /// \brief no-op
  /// \returns \*this
  constexpr iterator operator++(int) noexcept { return *this; }

  /// Accesses the underlying iterator.
  [[nodiscard]] constexpr OutputIterator base() const noexcept { return out_; }
  /// Accesses the underlying transcoder.
  [[nodiscard]] constexpr Transcoder* transcoder() noexcept { return transcoder_; }
  /// Accesses the underlying transcoder.
  [[nodiscard]] constexpr Transcoder const* transcoder() const noexcept { return transcoder_; }

private:
  /// The transcoder that will be used to convert code units when they are assigned.
  Transcoder* transcoder_;
  /// An output iterator to which code units produced by the transcoder will be written.
  ICUBABY_NO_UNIQUE_ADDRESS OutputIterator out_;
};

/// A class template argument deduction guide for icubaby::iterator.
template <typename Transcoder, typename OutputIterator>
iterator(Transcoder& transcoder, OutputIterator out) -> iterator<Transcoder, OutputIterator>;

namespace details {

/// Each UTF-8 continuation byte has space for 6 bits of payload.
inline constexpr auto utf8_shift = 6U;
/// A value with the least significant 6 bits set. Used to create or extract the payload from a UTF-8 continuation byte.
inline constexpr auto utf8_mask = static_cast<std::uint_least8_t>((1U << utf8_shift) - 1U);

/// A constant representing the first code point which must be represented as a UTF-16 surrogate pair.
inline constexpr auto utf16_first_surrogate_pair = 0x10000U;
/// The number of payload bits in a high or low surrogate value.
inline constexpr auto utf16_shift = 10U;
/// A value with the least significant 10 bits set. Used to create a UTF-16 low surrogate value.
inline constexpr auto utf16_mask = static_cast<std::uint_least16_t>((1U << utf16_shift) - 1U);

}  // end namespace details

/// Takes a sequence of UTF-32 code units and converts them to UTF-8.
template <> class transcoder<char32_t, char8> {
public:
  /// The type of the code units consumed by this transcoder.
  using input_type = char32_t;
  /// The type of the code units produced by this transcoder.
  using output_type = char8;

  constexpr transcoder() noexcept = default;
  /// Initializes a transcoder instance with an initial value for its "well formed" state. This can be useful if
  /// converting a stream of data which may be using different encodings.
  ///
  /// \param well_formed The initial value for the transcoder's "well formed" state.
  explicit constexpr transcoder(bool well_formed) noexcept : well_formed_{well_formed} {}

  /// Accepts a code unit in the UTF-32 source encoding. As UTF-8 output code units are generated, they are written to
  /// the output iterator \p dest.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param code_unit  A code unit in the source encoding.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  OutputIterator operator()(input_type code_unit, OutputIterator dest) noexcept {
    if (code_unit < 0x80) {
      *(dest++) = static_cast<output_type>(code_unit);
      return dest;
    }
    if (code_unit < 0x800) {
      return transcoder::write2(code_unit, dest);
    }
    if (is_surrogate(code_unit)) {
      return transcoder::not_well_formed(dest);
    }
    if (code_unit < 0x10000) {
      return transcoder::write3(code_unit, dest);
    }
    if (code_unit <= max_code_point) {
      return transcoder::write4(code_unit, dest);
    }
    return transcoder::not_well_formed(dest);
  }

  /// Call once the entire input sequence has been fed to operator(). This
  /// function ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr OutputIterator end_cp(OutputIterator dest) const {
    return dest;
  }

  /// Call once the entire input sequence has been fed to operator(). This
  /// function ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr iterator<transcoder, OutputIterator> end_cp(iterator<transcoder, OutputIterator> dest) {
    auto coder = dest.transcoder();
    assert(coder == this);
    return {coder, coder->end_cp(dest.base())};
  }

  /// \returns True if the input represented well formed UTF-32.
  [[nodiscard]] constexpr bool well_formed() const noexcept { return well_formed_; }
  /// \returns True if a partial code-point has been passed to operator() and
  ///   false otherwise.
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  [[nodiscard]] constexpr bool partial() const noexcept { return false; }

private:
  /// True if the input consumed is well formed, false otherwise.
  bool well_formed_ = true;

  // The following table shows how each range of code points is converted
  // to a series of UTF-8 bytes.
  //
  // | First CP | Last CP  | Byte 1   | Byte 2   | Byte 3   | Byte 4   |
  // | -------- | -------- | -------- | -------- | -------- | -------- |
  // | U+0000   | U+007F   | 0xxxxxxx |          |          |          |
  // | U+0080   | U+07FF   | 110xxxxx | 10xxxxxx |          |          |
  // | U+0800   | U+FFFF   | 1110xxxx | 10xxxxxx | 10xxxxxx |          |
  // | U+010000 | U+10FFFF | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |

  /// The a mask to indicate the first byte of a two byte sequence.
  static constexpr auto byte_1_of_2 = std::uint_least8_t{0b1100'0000};
  /// The a mask to indicate the first byte of a three byte sequence.
  static constexpr auto byte_1_of_3 = std::uint_least8_t{0b1110'0000};
  /// The a mask to indicate the first byte of a four byte sequence.
  static constexpr auto byte_1_of_4 = std::uint_least8_t{0b1111'0000};
  /// The a mask to indicate a continuation byte (that is byte two of a two byte sequence, bytes two and three of a
  /// three byte sequence, and so on.
  static constexpr auto continuation = std::uint_least8_t{0b1000'0000};

  /// Writes a series of "continuation" bytes to the output.
  ///
  /// Continuation bytes are all but the first byte of a two, three, or four byte encoding and each follows the same
  /// format of having the two most significant bits set to 0b10 and six bits of the input code unit filling the rest of
  /// the byte.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param number  The number of continuation bytes to be written.
  /// \param code_unit  The code unit to be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  static constexpr OutputIterator write_continuation(std::uint_least8_t const number, input_type const code_unit,
                                                     OutputIterator dest) {
    if (number == 0U) {
      return dest;
    }
    *(dest++) = static_cast<output_type>(((code_unit >> (details::utf8_shift * (number - 1U))) & details::utf8_mask) |
                                         continuation);
    return transcoder::write_continuation(number - 1U, code_unit, dest);
  }

  /// Writes a two CU value to the output.
  ///
  /// Code points in the range [U+80, U+800) are represented as two UTF-8 code units.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param code_unit  The code unit to be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  static OutputIterator write2(input_type code_unit, OutputIterator dest) {
    assert(code_unit >= 0x80U && code_unit <= 0x7FFU && "Code point is out-of-range for 2 byte UTF-8");
    *(dest++) = static_cast<output_type>((code_unit >> details::utf8_shift) | byte_1_of_2);
    return transcoder::write_continuation(std::uint_least8_t{1}, code_unit, dest);
  }
  /// Writes a three CU value to the output.
  ///
  /// Code points in the range [U+800, U+10000) are represented as three UTF-8 code units.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param code_unit  The code unit to be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  static OutputIterator write3(input_type code_unit, OutputIterator dest) {
    assert(code_unit >= 0x800U && code_unit <= 0xFFFFU && "Code point is out-of-range for 3 byte UTF-8");
    *(dest++) = static_cast<output_type>((code_unit >> (details::utf8_shift * 2U)) | byte_1_of_3);
    return transcoder::write_continuation(std::uint_least8_t{2}, code_unit, dest);
  }
  /// Writes a four CU value to the output.
  ///
  /// Code points in the range [U+10000, U+10FFFF] are represented as four UTF-8 code units.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param code_unit  The code unit to be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  static OutputIterator write4(input_type code_unit, OutputIterator dest) {
    assert(code_unit >= 0x10000U && code_unit <= 0x10FFFFU && "Code point is out-of-range for 4 byte UTF-8");
    *(dest++) = static_cast<output_type>((code_unit >> (details::utf8_shift * 3U)) | byte_1_of_4);
    return transcoder::write_continuation(std::uint_least8_t{3}, code_unit, dest);
  }
  /// Writes U+FFFD REPLACEMENT CHAR to the output and records the input as not well formed.
  ///
  /// \param dest  An output iterator to which the output sequence is written.
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  OutputIterator not_well_formed(OutputIterator dest) {
    well_formed_ = false;
    static_assert(!is_surrogate(replacement_char));
    return (*this)(replacement_char, dest);
  }
};

/// Takes a sequence of UTF-8 code units and converts them to UTF-32.
template <> class transcoder<char8, char32_t> {
public:
  /// The type of the code units consumed by this transcoder.
  using input_type = char8;
  /// The type of the code units produced by this transcoder.
  using output_type = char32_t;

  constexpr transcoder() noexcept : transcoder(true) {}
  /// Initializes a transcoder instance with an initial value for its "well formed" state. This can be useful if
  /// converting a stream of data which may be using different encodings.
  ///
  /// \param well_formed The initial value for the transcoder's "well formed" state.
  explicit constexpr transcoder(bool well_formed) noexcept
      : code_point_{0}, well_formed_{static_cast<std::uint_least32_t>(well_formed)}, state_{accept} {}

  /// Accepts a code unit in the UTF-8 source encoding. As UTF-32 output code units are generated, they are written to
  /// the output iterator \p dest.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of
  ///   output_type can be written.
  /// \param code_unit  A UTF-8 code unit,
  /// \param dest  Iterator to which the output should be written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  OutputIterator operator()(input_type code_unit, OutputIterator dest) {
    // Prior to C++20, char8 might be signed.
    static_assert(sizeof(input_type) <= sizeof(std::uint_least8_t));
    auto ucu = static_cast<std::uint_least8_t>(code_unit);
    // Clamp ucu in the event that it has more than 8 bits.
    if constexpr (CHAR_BIT > 8 || sizeof(std::uint_least8_t) > 1) {
      ucu = std::max(ucu, std::uint_least8_t{0xFF});
    }
    static_assert(utf8d_.size() > 255);
    assert(ucu < utf8d_.size());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    auto const type = utf8d_[ucu];
    code_point_ = (state_ != accept) ? static_cast<std::uint_least32_t>(ucu & details::utf8_mask) |
                                           static_cast<std::uint_least32_t>(code_point_ << details::utf8_shift)
                                     : (0xFFU >> type) & ucu;
    auto const idx = 256U + state_ + type;
    assert(idx < utf8d_.size());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    state_ = utf8d_[idx];
    switch (state_) {
    case accept: *(dest++) = code_point_; break;
    case reject:
      well_formed_ = false;
      state_ = accept;
      *(dest++) = replacement_char;
      break;
    default: break;
    }
    return dest;
  }

  /// Call once the entire input sequence has been fed to operator(). This function ensures that the sequence did not
  /// end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr OutputIterator end_cp(OutputIterator dest) {
    if (state_ != accept) {
      state_ = reject;
      *(dest++) = replacement_char;
      well_formed_ = false;
    }
    return dest;
  }

  /// Call once the entire input sequence has been fed to operator(). This function ensures that the sequence did not
  /// end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr iterator<transcoder, OutputIterator> end_cp(iterator<transcoder, OutputIterator> dest) {
    auto coder = dest.transcoder();
    assert(coder == this);
    return {coder, coder->end_cp(dest.base())};
  }

  /// \returns True if the input represented well formed UTF-8.
  [[nodiscard]] constexpr bool well_formed() const noexcept { return well_formed_; }
  /// \returns True if a partial code-point has been passed to operator() and false otherwise.
  [[nodiscard]] constexpr bool partial() const noexcept { return state_ != accept; }

private:
  /// The utf8d_ table consists of two parts. The first part maps bytes to character classes, the
  /// second part encodes a deterministic finite automaton using these character classes as
  /// transitions.
  /// \hideinitializer
  static inline std::array<std::uint_least8_t, 364> const utf8d_ = {{
      // clang-format off
    // The first part of the table maps bytes to character classes that
    // reduce the size of the transition table and create bitmasks.
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
     7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
     8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

    // The second part is a transition table that maps a combination
    // of a state of the automaton and a character class to a state.
     0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
    12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
    12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
    12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
    12,36,12,12,12,12,12,12,12,12,12,12,
      // clang-format on
  }};
  /// The code point value being assembled from input code units.
  std::uint_least32_t code_point_ : code_point_bits;
  /// True if the input consumed is well formed, false otherwise.
  std::uint_least32_t well_formed_ : 1;
  /// Pad bits intended to put the next value to a byte boundary.
  std::uint_least32_t : 2;
  enum : std::uint_least8_t { accept = 0, reject = 12 };
  /// The state of the converter.
  std::uint_least32_t state_ : 8;
};

/// Takes a sequence of UTF-32 code units and converts them to UTF-16.
template <> class transcoder<char32_t, char16_t> {
public:
  /// The type of the code units consumed by this transcoder.
  using input_type = char32_t;
  /// The type of the code units produced by this transcoder.
  using output_type = char16_t;

  constexpr transcoder() noexcept = default;
  /// Initializes a transcoder instance with an initial value for its "well formed" state. This can be useful if
  /// converting a stream of data which may be using different encodings.
  ///
  /// \param well_formed The initial value for the transcoder's "well formed" state.
  explicit constexpr transcoder(bool well_formed) noexcept : well_formed_{well_formed} {}

  /// Accepts a code unit in the UTF-32 source encoding. As UTF-16 output code units are generated, they are written to
  /// the output iterator \p dest.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of output_type can be written.
  /// \param code_unit  A UTF-32 code unit,
  /// \param dest  Iterator to which the output should be written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  OutputIterator operator()(input_type code_unit, OutputIterator dest) noexcept {
    if (is_surrogate(code_unit) || code_unit > max_code_point) {
      dest = (*this)(replacement_char, dest);
      well_formed_ = false;
    } else if (code_unit <= 0xFFFF) {
      *(dest++) = static_cast<output_type>(code_unit);
    } else {
      // Code points from beyond plane 0 are encoded as a two 16-bit code unit surrogate pair. The first code
      // unit is the high surrogate and the second is the low surrogate.
      // - 0x10000 is subtracted from the code point, leaving a 20-bit number (0x00000–0xFFFFF).
      // - The high ten bits (0x0000–0x03FF) are added to 0xD800 to give the high surrogate (0xD800–0xDBFF).
      // - The low ten bits (0x000–0x3FF) are added to 0xDC00 to give the low surrogate (0xDC00–0xDFFF).
      *(dest++) = static_cast<output_type>(static_cast<std::uint_least32_t>(first_high_surrogate) -
                                           (details::utf16_first_surrogate_pair >> details::utf16_shift) +
                                           (code_unit >> details::utf16_shift));
      *(dest++) = static_cast<output_type>(static_cast<std::uint_least32_t>(first_low_surrogate) +
                                           (code_unit & details::utf16_mask));
    }
    return dest;
  }

  /// Call once the entire input sequence has been fed to operator(). This function ensures that the sequence did not
  /// end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  The output iterator.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr OutputIterator end_cp(OutputIterator dest) noexcept {
    return dest;
  }

  /// Call once the entire input sequence has been fed to operator(). This function ensures that the sequence did not
  /// end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr iterator<transcoder, OutputIterator> end_cp(iterator<transcoder, OutputIterator> dest) {
    auto coder = dest.transcoder();
    assert(coder == this);
    return {coder, coder->end_cp(dest.base())};
  }

  /// \returns True if the input represented valid UTF-32.
  [[nodiscard]] constexpr bool well_formed() const noexcept { return well_formed_; }
  /// \returns True if a partial code-point has been passed to operator() and
  ///   false otherwise.
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  [[nodiscard]] constexpr bool partial() const noexcept { return false; }

private:
  /// True if the input consumed is well formed, false otherwise.
  bool well_formed_ = true;
};

/// Takes a sequence of UTF-16 code units and converts them to UTF-32.
template <> class transcoder<char16_t, char32_t> {
public:
  /// The type of the code units consumed by this transcoder.
  using input_type = char16_t;
  /// The type of the code units produced by this transcoder.
  using output_type = char32_t;

  constexpr transcoder() noexcept : transcoder(true) {}
  /// Initializes a transcoder instance with an initial value for its "well formed" state. This can be useful if
  /// converting a stream of data which may be using different encodings.
  ///
  /// \param well_formed The initial value for the transcoder's "well formed" state.
  explicit constexpr transcoder(bool well_formed) noexcept
      : high_{0},
        has_high_{static_cast<uint_least16_t>(false)},
        well_formed_{static_cast<uint_least16_t>(well_formed)} {}

  /// Accepts a code unit in the UTF-16 source encoding. As UTF-32 output code units are generated, they are written to
  /// the output iterator \p dest.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param code_unit  A code unit in the source encoding.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  OutputIterator operator()(input_type code_unit, OutputIterator dest) noexcept {
    if (!has_high_) {
      if (is_high_surrogate(code_unit)) {
        // A high surrogate code unit indicates that this is the first of a
        // high/low surrogate pair.
        high_ = adjusted_high(code_unit);
        has_high_ = true;
        return dest;
      }

      // A low-surrogate without a preceding high-surrogate.
      if (is_low_surrogate(code_unit)) {
        well_formed_ = false;
        code_unit = replacement_char;
      }
      *(dest++) = code_unit;
      return dest;
    }

    // A high surrogate followed by a low-surrogate.
    if (is_low_surrogate(code_unit)) {
      *(dest++) = static_cast<char32_t>(((static_cast<std::uint_least32_t>(high_) << details::utf16_shift) |
                                         (static_cast<std::uint_least32_t>(code_unit) - first_low_surrogate)) +
                                        details::utf16_first_surrogate_pair);
      high_ = 0;
      has_high_ = false;
      return dest;
    }
    // There was a high-surrogate followed by something other than a low surrogate. A high-surrogate followed by a
    // second high-surrogate yields a single REPLACEMENT CHARACTER. A high-surrogate followed by something other than
    // a low-surrogate gives REPLACEMENT CHARACTER followed by the second input code point.
    *(dest++) = replacement_char;
    well_formed_ = false;
    if (is_high_surrogate(code_unit)) {
      // There was a high surrogate followed by a second high-surrogate: remember the latter.
      high_ = adjusted_high(code_unit);
      assert(has_high_);
      return dest;
    }

    *(dest++) = code_unit;
    high_ = 0;
    has_high_ = false;
    return dest;
  }

  /// Call once the entire input sequence has been fed to operator(). This function ensures that the sequence did not
  /// end with a partial code point.
  ///
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  The output iterator.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator> OutputIterator end_cp(OutputIterator dest) {
    if (has_high_) {
      *(dest++) = replacement_char;
      high_ = 0;
      has_high_ = false;
      well_formed_ = false;
    }
    return dest;
  }

  /// Call once the entire input sequence has been fed to operator(). This function ensures that the sequence did not
  /// end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr iterator<transcoder, OutputIterator> end_cp(iterator<transcoder, OutputIterator> dest) {
    auto coder = dest.transcoder();
    assert(coder == this);
    return {coder, coder->end_cp(dest.base())};
  }

  /// \returns True if the input represented well formed UTF-16.
  [[nodiscard]] constexpr bool well_formed() const noexcept { return well_formed_; }
  /// \returns True if a partial code-point has been passed to operator() and false otherwise.
  [[nodiscard]] constexpr bool partial() const noexcept { return has_high_; }

private:
  /// The previous high surrogate that was passed to operator(). Valid if has_high_ is true.
  uint_least16_t high_ : details::utf16_shift;
  /// true if the previous code unit passed to operator() was a high surrogate, false otherwise.
  uint_least16_t has_high_ : 1;
  /// true if the code units passed to operator() represent well formed UTF-16 input, false otherwise.
  uint_least16_t well_formed_ : 1;

  /// \brief This function returns a high surrogate value that can be stored in the high_ field.
  ///
  /// The high surrogate value is stored after the first_high_surrogate value has been subtracted. This reduces the
  /// number of bits that we need to remember.
  ///
  /// \param code_unit A UTF-16 code unit for which icubaby::is_high_surrogate() returns true.
  /// \returns A high surrogate value that can be stored in the class's high_ field.
  static std::uint_least16_t adjusted_high(std::uint_least16_t code_unit) noexcept {
    assert(code_unit >= first_high_surrogate && "A high surrogate must be at least first_high_surrogate");
    auto const high_cu = code_unit - first_high_surrogate;
    assert(high_cu < std::numeric_limits<decltype(high_)>::max() && high_cu < (1U << details::utf16_shift) &&
           "high_cu won't fit in the high_ field!");
    return static_cast<uint_least16_t>(high_cu);
  }
};

/// \brief An enumeration representing the encoding detected by transcoder<std::byte, X>.
enum class encoding : std::uint_least8_t {
  unknown,  ///< No encoding has yet been determined.
  utf8,     ///< The detected encoding is UTF-8.
  utf16be,  ///< The detected encoding is big-endian UTF-16.
  utf16le,  ///< The detected encoding is little-endian UTF-16.
  utf32be,  ///< The detected encoding is big-endian UTF-32.
  utf32le,  ///< The detected encoding is little-endian UTF-32.
};

namespace details {

/// An alias template for a two-dimensional std::array
template <typename T, std::size_t Rows, std::size_t Columns> using array2d = std::array<std::array<T, Columns>, Rows>;

/// \brief A two-dimensional array containing the bytes that make up the encoded value of U+FEFF BYTE ORDER MARK.
///
/// \note The indices of the outer array must agree with the [encoding_utf16](\ref transcoder-encoding_utf16),
///       [encoding_utf32](\ref transcoder-encoding_utf32), [encoding_utf8](\ref transcoder-encoding_utf8),
///       [big_endian](\ref transcoder-big_endian), and [little_endian](\ref transcoder-little_endian) constants from
///       the transcoder<std::byte, ToEncoding> specialization. We use the encoding and endian fields together to
///       produce the outer index into this array. UTF-8 encoding has no inherent endianness so is modelled as
///       big-endian to enable it to occupy just the final entry in this array
inline array2d<std::byte, 5, 4> const boms{{
    {std::byte{0xFE}, std::byte{0xFF}},                                    // UTF-16 BE
    {std::byte{0xFF}, std::byte{0xFE}},                                    // UTF-16 LE
    {std::byte{0x00}, std::byte{0x00}, std::byte{0xFE}, std::byte{0xFF}},  // UTF-32 BE
    {std::byte{0xFF}, std::byte{0xFE}, std::byte{0x00}, std::byte{0x00}},  // UTF-32 LE
    {std::byte{0xEF}, std::byte{0xBB}, std::byte{0xBF}},                   // UTF-8
}};

}  // end namespace details

/// \brief The "byte transcoder" takes a sequence of bytes, determines their encoding and converts
///    to a specified encoding.
///
/// This transcoder is used when the input encoding is not known at compile-time. If present, a leading
/// byte-order-mark is interpreted to select the source encoding; if not present, UTF-8 encoding is assumed.
///
/// The byte transcoder is implemented as a finite state machine. The following diagram shows the state transitions that
/// occur as input bytes are received. Each vertex rectangle represents a state (the upper half has the state name
/// and the lower briefly describes the meaning of that state). Each edge describes the condition for that transition to
/// be made.
///
/// - An edge with description of the form *x=y* (where *y* is a hexadecimal constant) is taken if the input value *x*
///   is equal to the constant *y*.
/// - An edge with the description "otherwise" is taken if no other edges with the same origin are matched.
/// - An edge without a description is unconditionally taken for the next byte
///
/// \dotfile byte_transcoder.dot
template <ICUBABY_CONCEPT_UNICODE_CHAR_TYPE ToEncoding> class transcoder<std::byte, ToEncoding> {
public:
  /// The type of the values consumed by this transcoder.
  using input_type = std::byte;
  /// The type of the code units produced by this transcoder.
  using output_type = ToEncoding;

  /// \brief Accepts a byte for decoding. Output is written to a supplied output iterator.
  ///
  /// As output code units are generated, they are written to the output iterator \p dest.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param value  A byte of input.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  OutputIterator operator()(input_type value, OutputIterator dest) noexcept {
    switch (state_) {
    case states::start: dest = this->start_state(value, dest); break;
    case states::utf8_bom_byte2:
      assert(this->get_byte_no() == 2 && "Expected this state to target byte #2");
      buffer_[this->get_byte_no()] = value;
      // Start decoding as UTF-8. If we have a complete UTF-8 BOM drop it, otherwise output the code units seen so far.
      dest = this->run8_start(value != this->bom_value(), dest);
      break;

    case states::utf16_be_bom_byte1:
      assert(this->get_byte_no() == 1 && "Expected this state to target byte #1");
      buffer_[this->get_byte_no()] = value;
      // We either have a complete UTF-16 BE BOM, in which case we start transcoding, or we default to UTF-8 emitting
      // the bytes consumed so far.
      dest = value == this->bom_value() ? this->run16_start(dest) : this->run8_start(true, dest);
      break;

    case states::utf32_or_16_le_bom_byte2:
      assert(this->get_byte_no() == 2 && "Expected this state to target byte #2");
      if (value != transcoder::bom_value(encoding_utf32 | little_endian, this->get_byte_no())) {
        // This isn't a UTF-32 LE BOM but the start of a UTF-16 run.
        dest = this->run16_start(dest);
        state_ = states::run_16le_byte1;
        buffer_[0] = value;
        break;
      }
      [[fallthrough]];

    case states::utf8_bom_byte1:
    case states::utf32_or_16_le_bom_byte1:
    case states::utf32_or_16_be_bom_byte1:
    case states::utf32_be_bom_byte2:
      assert(this->get_byte_no() == 1 || this->get_byte_no() == 2 && "This must be byte #1 or #2");
      buffer_[this->get_byte_no()] = value;
      if (value == this->bom_value()) {
        state_ = this->next_byte();
      } else {
        // Default input encoding. Emit buffer.
        dest = this->run8_start(true, dest);
      }
      break;

    case states::utf32_le_bom_byte3:
    case states::utf32_be_bom_byte3:
      assert(this->get_byte_no() == 3 && "Expected this to be byte #3");
      buffer_[this->get_byte_no()] = value;
      if (value ==
          transcoder::bom_value(encoding_utf32 | (static_cast<std::byte>(state_) & endian_mask), this->get_byte_no())) {
        (void)transcoder_variant_.template emplace<t32_type>();
        state_ = transcoder::set_run_mode(transcoder::set_byte(state_, 0));
      } else {
        // Default input encoding. Emit buffer.
        dest = this->run8_start(true, dest);
      }
      break;

    case states::run_16be_byte0:
    case states::run_16le_byte0:
    case states::run_32be_byte0:
    case states::run_32be_byte1:
    case states::run_32be_byte2:
    case states::run_32le_byte0:
    case states::run_32le_byte1:
    case states::run_32le_byte2:
      assert(this->get_byte_no() < 3 && "Expected byte [0..3)");
      buffer_[this->get_byte_no()] = value;
      state_ = this->next_byte();
      break;

    case states::run_8:
      assert(std::holds_alternative<t8_type>(transcoder_variant_));
      if (auto* const utf8_input = std::get_if<t8_type>(&transcoder_variant_)) {
        dest = (*utf8_input)(static_cast<char8>(value), dest);
      }
      break;

    case states::run_16be_byte1:
    case states::run_16le_byte1: dest = this->run16(value, dest); break;

    case states::run_32be_byte3:
    case states::run_32le_byte3: dest = this->run32(value, dest); break;
    }
    return dest;
  }

  /// \brief Call once the entire input sequence has been fed to operator().
  ///
  /// This function ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(ToEncoding) OutputIterator>
  OutputIterator end_cp(OutputIterator dest) noexcept {
    if (transcoder_variant_.valueless_by_exception()) {
      return dest;
    }
    return std::visit(
        [this, &dest](auto& arg) {
          if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::monostate>) {
            return this->run8_start(state_ != states::start, dest);
          } else {
            return arg.end_cp(dest);
          }
        },
        transcoder_variant_);
  }

  /// \brief Call once the entire input sequence has been fed to operator().
  ///
  /// This function ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(ToEncoding) OutputIterator>
  constexpr iterator<transcoder, OutputIterator> end_cp(iterator<transcoder, OutputIterator> dest) {
    auto coder = dest.transcoder();
    assert(coder == this);
    return {coder, coder->end_cp(dest.base())};
  }

  /// \brief Returns true if the input represents well formed Unicode.
  /// \returns True if the input represents well formed Unicode.
  [[nodiscard]] constexpr bool well_formed() const noexcept;

  /// \brief Return true if a partial code-point has been passed to operator().
  /// \returns True if a partial code-point has been passed to operator() and
  /// false otherwise.
  [[nodiscard]] constexpr bool partial() const noexcept;

  /// \brief The detected encoding of the input stream.
  /// \returns The encoding of the input stream as detected by consuming an optional leading byte order mark. Initially
  ///          encoding::unknown.
  [[nodiscard]] constexpr encoding selected_encoding() const noexcept;

private:
  /// \anchor transcoder-byte_no
  /// \param index  The byte number within the BOM encoding. Must be in the range [0,3].
  /// \returns A value which can be bitwise ORed to represent a particular byte within a BOM encoding.
  static constexpr std::byte byte_no(std::uint_least8_t index) noexcept {
    assert(index < 4U);
    return static_cast<std::byte>(index);
  }

  /// The number of places to left shift when constructing encoding values for the FSM state enumeration.
  static constexpr auto encoding_shift = 4U;
  /// The number of places to left shift when constructing endian values for the FSM state enumeration.
  static constexpr auto endian_shift = 3U;
  /// The number of places to left shift when constructing mode values for the FSM state enumeration.
  static constexpr auto run_shift = 2U;

  static constexpr auto encoding_mask = std::byte{0b11 << encoding_shift};  ///< One of unknown or UTF-8/16/32.
  static constexpr auto endian_mask = std::byte{1U << endian_shift};        ///< One of big_endian or little_endian.
  static constexpr auto run_mask = std::byte{1U << run_shift};              ///< Run or bom mode.
  static constexpr auto byte_no_mask = std::byte{0b11};                     ///< Values from 0-3.

  /// \brief UTF-16 BE or UTF-16 LE encoding.
  /// \anchor transcoder-encoding_utf16
  /// Bitwise-or this value to create a state representing UTF-16 (BE or LE) encoding.
  static constexpr auto encoding_utf16 = std::byte{0b00 << encoding_shift};
  /// \brief UTF-32 BE or UTF-32 LE encoding.
  /// \anchor transcoder-encoding_utf32
  /// Bitwise-or this value to create a state representing UTF-32 (BE or LE) encoding.
  static constexpr auto encoding_utf32 = std::byte{0b01 << encoding_shift};
  /// \brief UTF-8 encoding.
  /// \anchor transcoder-encoding_utf8
  /// Bitwise-or this value to create a state representing UTF-8 encoding.
  static constexpr auto encoding_utf8 = std::byte{0b10 << encoding_shift};
  /// \brief The input encoding is not yet known.
  static constexpr auto encoding_unknown = std::byte{0b11 << encoding_shift};

  /// \brief Bitwise-or this value to create a state representing the FSM identifying the BOM.
  /// \anchor transcoder-bom_mode
  static constexpr auto bom_mode = std::byte{0};
  /// \brief Bitwise-or this value to create a state representing the FSM consuming runs of bytes and emitting
  ///        code-units.
  /// \anchor transcoder-run_mode
  static constexpr auto run_mode = run_mask;

  /// \brief Bitwise-or this value to create a state consuming big-endian values.
  /// \anchor transcoder-big_endian
  static constexpr auto big_endian = std::byte{0};
  /// \brief Bitwise-or this value to create a state consuming little-endian values.
  /// \anchor transcoder-little_endian
  static constexpr auto little_endian = endian_mask;

  /// \brief The states that the finite state machine can occupy.
  ///
  /// The values for each state is made from a collection of bits that not only uniquely identify the state, but which
  /// can be used to share a great deal of code between similar states. For example, the code for handling all but the
  /// final byte of a UTF-16 BE, UTF-16 LE, UTF-32 BE and UTF-32 LE code unit is shared. We can extract the encoding,
  /// endianness, and byte numbers from the state codes.
  ///
  // clang-format off
  /// Bit | Interpretation
  /// --- | --------------
  /// 0   | 2 bits to describe the encoding being processed: UTF-16 (\link encoding_utf16 \endlink), UTF-32 (\link encoding_utf32 \endlink), UTF-8 (\link encoding_utf8 \endlink), unknown (\link encoding_unknown \endlink). Note that the values are combined with the value of bit 2 (the endianness) to produce an index to the first dimension of the \link details::boms \endlink array.
  /// 1   | ^
  /// 2   | 1 bit to identify the input as big (\link big_endian \endlink) or little (\link little_endian \endlink) endian.
  /// 3   | 1 bit to identify when the state machine is in "Run" or "BOM" mode. In BOM mode (\link bom_mode \endlink), we are in the process of identifying the input encoding. In run mode (\link run_mode \endlink), we are consuming and emitting code-units.
  /// 4   | 2 bits provide the index of the byte within the BOM that we are processing. This value corresponds to an index into the second dimension of the \link details::boms \endlink array.
  /// 5   | ^
  /// 6   | Unused. Always 0.
  /// 7   | Unused. Always 0.
  // clang-format on
  ///
  /// There are constants which may be bit-wise ORed together to create the appropriate value for each of the FSM's
  /// states.
  ///
  /// \see encoding_utf16, encoding_utf32, encoding_utf8, encoding_unknown, bom_mode, run_mode, big_endian,
  ///      little_endian, [byte_no(std::uint_least8_t)](\ref transcoder-byte_no).

  enum class states : std::uint_least8_t {
    /// The FSM's initial state.
    start = static_cast<std::uint_least8_t>(encoding_unknown | bom_mode | byte_no(0)),

    /// The state if the second byte of a UTF-8 BOM was identified.
    utf8_bom_byte1 = static_cast<std::uint_least8_t>(encoding_utf8 | big_endian | bom_mode | byte_no(1U)),
    /// The state if the third byte of a UTF-8 BOM was identified.
    utf8_bom_byte2 = static_cast<std::uint_least8_t>(encoding_utf8 | big_endian | bom_mode | byte_no(2U)),

    /// The state if the second byte of a UTF-16 BOM was identified.
    utf16_be_bom_byte1 = static_cast<std::uint_least8_t>(encoding_utf16 | big_endian | bom_mode | byte_no(1U)),
    /// The state if the third byte of a UTF-32 BE BOM was identified.
    utf32_be_bom_byte2 = static_cast<std::uint_least8_t>(encoding_utf32 | big_endian | bom_mode | byte_no(2U)),
    /// The state if the fourth byte of a UTF-32 BE BOM was identified.
    utf32_be_bom_byte3 = static_cast<std::uint_least8_t>(encoding_utf32 | big_endian | bom_mode | byte_no(3U)),

    /// The state if the second byte of a UTF-32 BE or UTF-16 BE BOM was identified.
    utf32_or_16_be_bom_byte1 = static_cast<std::uint_least8_t>(encoding_utf32 | big_endian | bom_mode | byte_no(1U)),

    /// The state if the second byte of a UTF-32 LE or UTF-16 LE BOM was identified.
    utf32_or_16_le_bom_byte1 = static_cast<std::uint_least8_t>(encoding_utf32 | little_endian | bom_mode | byte_no(1U)),
    /// The state when the state machine is checking for the third byte of a UTF-32 LE BOM or the start of a UTF-16 LE
    /// run.
    utf32_or_16_le_bom_byte2 = static_cast<std::uint_least8_t>(encoding_utf32 | little_endian | bom_mode | byte_no(2U)),
    /// The state when the state machine is checking for the third byte of a UTF-32 LE BOM or the start of a UTF-16 LE
    /// run.
    utf32_le_bom_byte3 = static_cast<std::uint_least8_t>(encoding_utf32 | little_endian | bom_mode | byte_no(3U)),

    run_8 = static_cast<std::uint_least8_t>(encoding_utf8 | big_endian | run_mode | byte_no(0U)),

    /// The state when the state machine is handling the first byte of a UTF-16 BE code-unit.
    run_16be_byte0 = static_cast<std::uint_least8_t>(encoding_utf16 | big_endian | run_mode | byte_no(0U)),
    /// The state when the state machine is handling the second and final byte of a UTF-32 BE code-unit.
    run_16be_byte1 = static_cast<std::uint_least8_t>(encoding_utf16 | big_endian | run_mode | byte_no(1U)),

    /// The state when the state machine is handling the first byte of a UTF-16 LE code-unit.
    run_16le_byte0 = static_cast<std::uint_least8_t>(encoding_utf16 | little_endian | run_mode | byte_no(0U)),
    /// The state when the state machine is handling the second and final byte of a UTF-32 LE code-unit.
    run_16le_byte1 = static_cast<std::uint_least8_t>(encoding_utf16 | little_endian | run_mode | byte_no(1U)),

    /// The state when the state machine is handling the first byte of a UTF-32 BE code-unit.
    run_32be_byte0 = static_cast<std::uint_least8_t>(encoding_utf32 | big_endian | run_mode | byte_no(0U)),
    /// The state when the state machine is handling the second byte of a UTF-32 BE code-unit.
    run_32be_byte1 = static_cast<std::uint_least8_t>(encoding_utf32 | big_endian | run_mode | byte_no(1U)),
    /// The state when the state machine is handling the third byte of a UTF-32 BE code-unit.
    run_32be_byte2 = static_cast<std::uint_least8_t>(encoding_utf32 | big_endian | run_mode | byte_no(2U)),
    /// The state when the state machine is handling the fourth and final byte of a UTF-32 BE code-unit.
    run_32be_byte3 = static_cast<std::uint_least8_t>(encoding_utf32 | big_endian | run_mode | byte_no(3U)),

    /// The state when the state machine is handling the first byte of a UTF-32 LE code-unit.
    run_32le_byte0 = static_cast<std::uint_least8_t>(encoding_utf32 | little_endian | run_mode | byte_no(0U)),
    /// The state when the state machine is handling the second byte of a UTF-32 LE code-unit.
    run_32le_byte1 = static_cast<std::uint_least8_t>(encoding_utf32 | little_endian | run_mode | byte_no(1U)),
    /// The state when the state machine is handling the third byte of a UTF-32 LE code-unit.
    run_32le_byte2 = static_cast<std::uint_least8_t>(encoding_utf32 | little_endian | run_mode | byte_no(2U)),
    /// The state when the state machine is handling the fourth and final byte of a UTF-32 LE code-unit.
    run_32le_byte3 = static_cast<std::uint_least8_t>(encoding_utf32 | little_endian | run_mode | byte_no(3U)),
  };

  /// \brief Returns true if the argument represents a state where the FSM is consuming and producing code-units.
  ///
  /// \returns True if the parameter represents a state where the FSM is consuming and producing code-units.
  [[nodiscard]] constexpr bool is_run_mode() const noexcept {
    return (static_cast<std::byte>(state_) & run_mask) == run_mode;
  }
  /// \brief Returns true if the argument represents a state in which the FSM is consuming little endian code units.
  ///
  /// \returns True if the transcoder is consuming little-endian values, false otherwise.
  [[nodiscard]] constexpr bool is_little_endian() const noexcept {
    return (static_cast<std::byte>(state_) & endian_mask) == little_endian;
  }

  /// \brief Extracts the byte number referenced by the argument.
  ///
  /// Each of the valid FSM states has an embedded byte number in the range [0..4). This is the current byte of the
  /// current code unit as it is being assembled by the FSM.
  ///
  /// \param state  A valid state machine state.
  /// \returns The byte number referenced by \p state.
  [[nodiscard]] static constexpr std::uint_least8_t get_byte_no(states const state) noexcept {
    return static_cast<std::uint_least8_t>(static_cast<std::byte>(state) & byte_no_mask);
  }
  /// \brief Extracts the byte number referenced the current state.
  ///
  /// Each of the valid FSM states has an embedded byte number in the range [0..4). This is the current byte of the
  /// current code unit as it is being assembled by the FSM.
  ///
  /// \returns The byte number referenced by the current state.
  [[nodiscard]] constexpr std::uint_least8_t get_byte_no() const noexcept { return transcoder::get_byte_no(state_); }

  /// \brief Returns a state which references a specific byte number.
  ///
  /// \param state  A valid state machine state. The returned state will be the same as this argument but with the byte
  ///               number set to \p byte_number.
  /// \param byte_number The byte to be referenced. Must be in the range [0..4).
  /// \returns A state referencing the supplied byte number.
  [[nodiscard]] static constexpr states set_byte(states const state, std::uint_least8_t const byte_number) noexcept {
    assert(byte_number < 4 && "States must not try to address a byte number > 3");
    return static_cast<states>((static_cast<std::byte>(state) & ~byte_no_mask) | static_cast<std::byte>(byte_number));
  }
  /// \brief Returns a state which references the next byte number.
  ///
  /// \param state  A valid state machine state. The returned state will be the same as this argument but with the byte
  ///               number incremented.
  /// \returns A state referencing the next byte number.
  [[nodiscard]] static constexpr states next_byte(states const state) noexcept {
    return transcoder::set_byte(state, transcoder::get_byte_no(state) + 1);
  }
  /// \brief Returns a state which references the next byte number.
  /// \returns A state referencing the next byte number.
  [[nodiscard]] constexpr states next_byte() const noexcept { return transcoder::next_byte(state_); }

  /// \brief Adjusts a state so that run mode is selected.
  ///
  /// \param state A valid state machine state.
  /// \returns The modified state.
  [[nodiscard]] static constexpr states set_run_mode(states const state) noexcept {
    assert((static_cast<std::byte>(state) & run_mask) == bom_mode && "Expected a BOM mode state");
    return static_cast<states>((static_cast<std::byte>(state) & ~run_mask) | run_mode);
  }

  /// \brief Returns a byte from the byte order marker table which corresponds to a specific state as denoted by
  ///   \p state_byte and byte count \p byte_number.
  ///
  /// \param state_byte  A valid state machine state.
  /// \param byte_number  The index of the byte within the byte order marker.
  /// \returns  A byte from the byte order marker table.
  [[nodiscard]] static constexpr std::byte bom_value(std::byte const state_byte,
                                                     std::uint_least8_t const byte_number) noexcept {
    assert(((state_byte & (encoding_mask | endian_mask)) >> endian_shift) == (state_byte >> endian_shift));
    auto const encoding_index = static_cast<std::size_t>(state_byte >> endian_shift);
    assert(encoding_index < details::boms.size() && "The index is too large for the BOMs array");
    if (encoding_index >= details::boms.size()) {
      return std::byte{0x00};
    }
    auto const& enc = details::boms[encoding_index];
    assert(byte_number < enc.size() && "Byte number must lie within the BOMs array");
    if (byte_number >= enc.size()) {
      return std::byte{0x00};
    }
    return enc[byte_number];
  }
  /// \brief Returns a byte from the byte order marker table which corresponds to a specific state as denoted by
  ///   the current state and byte count
  ///
  /// \returns  A byte from the byte order marker table.
  [[nodiscard]] constexpr std::byte bom_value() const noexcept {
    return transcoder::bom_value(static_cast<std::byte>(state_), transcoder::get_byte_no(state_));
  }

  /// A short name for the transcoder used when UTF-8 input has been detected.
  using t8_type = transcoder<icubaby::char8, ToEncoding>;
  /// A short name for the transcoder used when UTF-16 input has been detected.
  using t16_type = transcoder<char16_t, ToEncoding>;
  /// A short name for the transcoder used when UTF-32 input has been detected.
  using t32_type = transcoder<char32_t, ToEncoding>;

  /// The current state of the FSM.
  states state_ = states::start;
  /// A buffer into which input bytes are gathered as a complete code unit is being assembled by the state machine.
  std::array<std::byte, 4> buffer_{};
  /// Holds the transcoder used to convert input code units. Holds monostate until the input encoding has been selected.
  std::variant<std::monostate, t8_type, t16_type, t32_type> transcoder_variant_;

  /// \brief A helper for ensuring that a type will not cause variant_ to become valueless by exception.
  ///
  /// We must ensure that the transcoder_variant_ member cannot be in the valueless by exception state. This means that
  /// construction and assignment to the variant must never throw. This type is a helper to verify that a type cannot
  /// throw during default, copy, or move construction as well as copy or move assignment.
  template <typename T>
  struct is_nothrowable
      : std::bool_constant<std::is_nothrow_constructible_v<T> && std::is_nothrow_copy_constructible_v<T> &&
                           std::is_nothrow_move_constructible_v<T> && std::is_nothrow_copy_assignable_v<T> &&
                           std::is_nothrow_move_assignable_v<T>> {};
  static_assert(is_nothrowable<std::monostate>::value);
  static_assert(is_nothrowable<t8_type>::value);
  static_assert(is_nothrowable<t16_type>::value);
  static_assert(is_nothrowable<t32_type>::value);

  /// Handles the initial state of the FSM. Checks the initial input byte against the collection of potential byte order
  /// mark initial bytes and decides on the next action.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param value  The initial input byte.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(ToEncoding) OutputIterator>
  [[nodiscard]] OutputIterator start_state(input_type const value, OutputIterator dest) noexcept {
    static constexpr auto byte_number = 0U;
    buffer_[byte_number] = value;
    if (value == transcoder::bom_value(encoding_utf8 | big_endian, byte_number)) {
      state_ = states::utf8_bom_byte1;
    } else if (value == transcoder::bom_value(encoding_utf16 | big_endian, byte_number)) {
      state_ = states::utf16_be_bom_byte1;
    } else if (value == transcoder::bom_value(encoding_utf16 | little_endian, byte_number)) {
      state_ = states::utf32_or_16_le_bom_byte1;
    } else if (value == transcoder::bom_value(encoding_utf32 | big_endian, byte_number)) {
      state_ = states::utf32_or_16_be_bom_byte1;
    } else {
      // This code unit wasn't recognized as being the first of a BOM in any encoding. Assume UTF-8 and process it
      // immediately.
      dest = this->run8_start(true, dest);
    }
    return dest;
  }

  /// Switches to the run state in which the input has been determined to be UTF-8 encoded.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param copy_buffer  True if the contents of buffer_ should be copied immediately to the output.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(ToEncoding) OutputIterator>
  [[nodiscard]] OutputIterator run8_start(bool const copy_buffer, OutputIterator dest) noexcept {
    assert(!this->is_run_mode() && "The FSM should not be in run mode when run8_start is called");
    assert(std::holds_alternative<std::monostate>(transcoder_variant_) &&
           "The variant should hold monostate until the FSM is in run mode");
    auto& trans = transcoder_variant_.template emplace<t8_type>();
    if (copy_buffer) {
      // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
      auto const first = std::begin(buffer_);
      (void)std::for_each(first, first + this->get_byte_no() + 1,
                          [&trans, &dest](std::byte value) { dest = trans(static_cast<char8>(value), dest); });
    }
    state_ = states::run_8;
    return dest;
  }

  /// Switches to the run state in which the input has been determined to be UTF-16 encoded.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(ToEncoding) OutputIterator>
  [[nodiscard]] OutputIterator run16_start(OutputIterator dest) noexcept {
    assert(!this->is_run_mode() && "The FSM should not be in run mode when run16_start is called");
    assert(std::holds_alternative<std::monostate>(transcoder_variant_) &&
           "The variant should hold monostate until the FSM is in run mode");
    (void)transcoder_variant_.template emplace<t16_type>();
    state_ = static_cast<states>(encoding_utf16 | (static_cast<std::byte>(state_) & endian_mask) | run_mode |
                                 transcoder::byte_no(0U));
    return dest;
  }

  /// \brief Handler for the states::run_16be_byte1 and states::run_16le_byte1 states.
  ///
  /// This function is called once we have received the second byte of a UTF-16 code unit. We build the native-endian
  /// version of the 16-bit value and pass it to the transcoder which will be expecting UTF-16. The FSM is then reset
  /// to expect byte 0 of the next code unit.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param value  The final byte of the 16-bit code unit.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(ToEncoding) OutputIterator>
  [[nodiscard]] OutputIterator run16(input_type const value, OutputIterator dest) noexcept {
    assert(state_ == states::run_16be_byte1 || state_ == states::run_16le_byte1);
    assert(std::holds_alternative<t16_type>(transcoder_variant_));

    if (auto* const utf16_input = std::get_if<t16_type>(&transcoder_variant_)) {
      dest = (*utf16_input)(state_ == states::run_16be_byte1 ? this->char16_from_big_endian_buffer(value)
                                                             : this->char16_from_little_endian_buffer(value),
                            dest);
    }
    state_ = transcoder::set_byte(state_, 0);
    return dest;
  }

  /// \brief Handler for the state::run_32be_byte3 and state::run_32le_byte3 states.
  ///
  /// This function is called once we have received all four bytes of a UTF-32 code unit. We build the native-endian
  /// version of the 32-bit value and pass it to the transcoder which will be expecting UTF-32. The FSM is then reset
  /// to expect byte 0 of the next code unit.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param value  The final byte of the 32-bit code unit.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(ToEncoding) OutputIterator>
  [[nodiscard]] OutputIterator run32(input_type const value, OutputIterator dest) noexcept {
    assert(state_ == states::run_32be_byte3 || state_ == states::run_32le_byte3);
    assert(std::holds_alternative<t32_type>(transcoder_variant_));

    if (auto* const utf32_input = std::get_if<t32_type>(&transcoder_variant_)) {
      dest = (*utf32_input)(state_ == states::run_32be_byte3 ? this->char32_from_big_endian_buffer(value)
                                                             : this->char32_from_little_endian_buffer(value),
                            dest);
    }
    state_ = transcoder::set_byte(state_, 0);
    return dest;
  }

  /// \brief Produces a native-endian 16-bit value from big endian encoded input by combining the first entry in the
  ///        buffer_ array with \p value.
  ///
  /// \param value An input byte
  /// \returns A native-endian 16 bit value.
  [[nodiscard]] constexpr char16_t char16_from_big_endian_buffer(input_type const value) const noexcept {
    return static_cast<char16_t>(static_cast<std::uint_least16_t>(static_cast<std::uint_least16_t>(buffer_[0]) << 8U) |
                                 static_cast<std::uint_least16_t>(value));
  }
  /// \brief Produces a native-endian 16-bit value from little endian encoded input by combining the first entry in the
  ///        buffer_ array with \p value.
  ///
  /// \param value An input byte
  /// \returns A native-endian 16 bit value.
  [[nodiscard]] constexpr char16_t char16_from_little_endian_buffer(input_type const value) const noexcept {
    return static_cast<char16_t>(static_cast<std::uint_least16_t>(static_cast<std::uint_least16_t>(value) << 8U) |
                                 static_cast<std::uint_least16_t>(buffer_[0]));
  }
  /// \brief Produces a native-endian 32-bit value from big endian encoded input by combining the entries in the
  ///        buffer_ array with \p value.
  ///
  /// \param value An input byte
  /// \returns A native-endian 32 bit value.
  [[nodiscard]] constexpr char32_t char32_from_big_endian_buffer(input_type const value) const noexcept {
    return static_cast<char32_t>(static_cast<std::uint_least32_t>(static_cast<std::uint_least32_t>(buffer_[0]) << 24U) |
                                 static_cast<std::uint_least32_t>(static_cast<std::uint_least32_t>(buffer_[1]) << 16U) |
                                 static_cast<std::uint_least32_t>(static_cast<std::uint_least32_t>(buffer_[2]) << 8U) |
                                 static_cast<std::uint_least32_t>(value));
  }
  /// \brief Produces a native-endian 32-bit value from little endian encoded input by combining the entries in the
  ///        buffer_ array with \p value.
  ///
  /// \param value An input byte
  /// \returns A native-endian 32 bit value.
  [[nodiscard]] constexpr char32_t char32_from_little_endian_buffer(input_type const value) const noexcept {
    return static_cast<char32_t>(
        (static_cast<std::uint_least32_t>(value << 24U)) | (static_cast<std::uint_least32_t>(buffer_[2]) << 16U) |
        (static_cast<std::uint_least32_t>(buffer_[1]) << 8U) | (static_cast<std::uint_least32_t>(buffer_[0])));
  }
};

// partial
// ~~~~~~~
template <ICUBABY_CONCEPT_UNICODE_CHAR_TYPE ToEncoding>
constexpr bool transcoder<std::byte, ToEncoding>::partial() const noexcept {
  // We ensure that the variant cannot ever become stateless. This check is belt and braces to guarantee that
  // std::visit() cannot throw.
  if (transcoder_variant_.valueless_by_exception()) {
    return false;
  }
  return std::visit(
      [this](auto const& arg) {
        if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::monostate>) {
          return this->state_ != states::start;
        } else {
          return arg.partial();
        }
      },
      transcoder_variant_);
}

// well formed
// ~~~~~~~~~~~
template <ICUBABY_CONCEPT_UNICODE_CHAR_TYPE ToEncoding>
constexpr bool transcoder<std::byte, ToEncoding>::well_formed() const noexcept {
  if (transcoder_variant_.valueless_by_exception()) {
    return true;
  }
  return std::visit(
      [](auto const& arg) {
        if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::monostate>) {
          return true;
        } else {
          return arg.well_formed();
        }
      },
      transcoder_variant_);
}

// selected encoding
// ~~~~~~~~~~~~~~~~~
template <ICUBABY_CONCEPT_UNICODE_CHAR_TYPE ToEncoding>
constexpr encoding transcoder<std::byte, ToEncoding>::selected_encoding() const noexcept {
  if (!this->is_run_mode()) {
    return encoding::unknown;
  }

  switch (static_cast<std::uint_least8_t>(static_cast<std::byte>(state_) & encoding_mask)) {
  case static_cast<std::uint_least8_t>(encoding_utf8): return encoding::utf8;
  case static_cast<std::uint_least8_t>(encoding_utf16):
    return this->is_little_endian() ? encoding::utf16le : encoding::utf16be;
  case static_cast<std::uint_least8_t>(encoding_utf32):
    return this->is_little_endian() ? encoding::utf32le : encoding::utf32be;
  default: assert(false && "We must know the encoding when in run mode"); return encoding::unknown;
  }
}

namespace details {

/// \brief The maximum number of code units produced as the intermediate output from the triangulator's conversion to
///        UTF-32.
///
/// The maximum number of code units produced from a single input code unit can vary (particularly if the input is
/// malformed).
///
/// \tparam FromEncoding  The source encoding.
/// \tparam ToEncoding  The destination encoding.
template <ICUBABY_CONCEPT_UNICODE_CHAR_TYPE FromEncoding, ICUBABY_CONCEPT_UNICODE_CHAR_TYPE ToEncoding>
struct triangulator_intermediate_code_units : public std::integral_constant<std::size_t, 1> {};
/// \brief The maximum number of code units produced when converting from UTF-16.
/// \tparam ToEncoding  The destination encoding.
template <ICUBABY_CONCEPT_UNICODE_CHAR_TYPE ToEncoding>
struct triangulator_intermediate_code_units<char16_t, ToEncoding> : public std::integral_constant<std::size_t, 2> {};

/// \brief A "triangulator" converts from the \p FromEncoding encoding to the \p ToEncoding encoding via an
///        intermediate UTF-32 encoding.
///
/// \tparam FromEncoding  The source encoding.
/// \tparam ToEncoding  The destination encoding.
template <ICUBABY_CONCEPT_UNICODE_CHAR_TYPE FromEncoding, ICUBABY_CONCEPT_UNICODE_CHAR_TYPE ToEncoding>
class triangulator {
public:
  /// The type of the code units consumed by this transcoder.
  using input_type = FromEncoding;
  /// The type of the code units produced by this transcoder.
  using output_type = ToEncoding;

  /// Accepts a code unit in the source encoding (as given by triangulator::input_type). These are first converted
  /// to UTF-32 and then to the output encoding (double_transcoder::output_type). As output code units are generated,
  /// they are written to the output iterator \p dest.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param code_unit  A code unit in the source encoding.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  OutputIterator operator()(input_type code_unit, OutputIterator dest) {
    // The (intermediate) output from the conversion to UTF-32. It's possible
    // for the transcoder to produce more than a single output code unit if the
    // input is malformed.
    std::array<char32_t, triangulator_intermediate_code_units<FromEncoding, ToEncoding>::value> intermediate{};
    // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
    auto const begin = std::begin(intermediate);
    return copy(begin, intermediate_(code_unit, begin), dest);
  }

  /// Call once the entire input sequence has been fed to operator(). This
  /// function ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator> OutputIterator end_cp(OutputIterator dest) {
    std::array<char32_t, triangulator_intermediate_code_units<FromEncoding, ToEncoding>::value> intermediate{};
    // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
    auto const first = std::begin(intermediate);
    // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
    auto const last = intermediate_.end_cp(first);
    assert(last >= first && last <= std::end(intermediate));
    return output_.end_cp(this->copy(first, last, dest));
  }

  /// Call once the entire input sequence has been fed to operator(). This
  /// function ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr iterator<transcoder<FromEncoding, ToEncoding>, OutputIterator> end_cp(
      iterator<transcoder<FromEncoding, ToEncoding>, OutputIterator> dest) {
    auto const transcoder = dest.transcoder();
    assert(transcoder == this);
    return {transcoder, transcoder->end_cp(dest.base())};
  }

  /// \returns True if the input passed to operator() was valid.
  [[nodiscard]] constexpr bool well_formed() const noexcept {
    return intermediate_.well_formed() && output_.well_formed();
  }

  /// \returns True if a partial code-point has been passed to operator() and
  /// false otherwise.
  [[nodiscard]] constexpr bool partial() const noexcept { return intermediate_.partial(); }

private:
  /// We use the intermediate_ transcoder to convert from the input encoding to UTF-32.
  transcoder<input_type, char32_t> intermediate_;
  /// The output_ transcoder converts from the intermediate (UTF-32) encoding to the selected output encoding.
  transcoder<char32_t, output_type> output_;

  /// Copies the range [first, last) to the output iterator \p dest via the output_ transcoder.
  ///
  /// \param first  The first of the range to copy.
  /// \param last  The last of the range to copy.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <typename InputIterator, typename OutputIterator>
  OutputIterator copy(InputIterator first, InputIterator last, OutputIterator dest) {
    (void)std::for_each(first, last, [this, &dest](char32_t const code_unit) { dest = output_(code_unit, dest); });
    return dest;
  }
};

}  // end namespace details

/// Takes a sequence of UTF-8 code units and converts them to UTF-16.
template <> class transcoder<char8, char16_t> : public details::triangulator<char8, char16_t> {};
/// Takes a sequence of UTF-16 code units and converts them to UTF-8.
template <> class transcoder<char16_t, char8> : public details::triangulator<char16_t, char8> {};
/// Takes a sequence of UTF-8 code units and converts them to UTF-8.
template <> class transcoder<char8, char8> : public details::triangulator<char8, char8> {};
/// Takes a sequence of UTF-16 code units and converts them to UTF-16.
template <> class transcoder<char16_t, char16_t> : public details::triangulator<char16_t, char16_t> {};
/// Takes a sequence of UTF-32 code units and converts them to UTF-32.
template <> class transcoder<char32_t, char32_t> {
public:
  /// The type of the code units consumed by this transcoder.
  using input_type = char32_t;
  /// The type of the code units produced by this transcoder.
  using output_type = char32_t;

  /// Accepts a code unit in the UTF-32 source encoding. As UTF-32 output code units are generated, they are written to
  /// the output iterator \p dest.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of
  ///   output_type can be written.
  /// \param code_unit  A UTF-32 code unit,
  /// \param dest  Iterator to which the output should be written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  OutputIterator operator()(input_type code_unit, OutputIterator dest) {
    // From D90 in Chapter 3 of Unicode 15.0.0
    // <https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf>:
    //
    // "Because surrogate code points are not included in the set of Unicode
    // scalar values, UTF-32 code units in the range 0000D80016..0000DFFF16 are
    // ill-formed. Any UTF-32 code unit greater than 0x0010FFFF is ill-formed."
    if (code_unit > max_code_point || is_surrogate(code_unit)) {
      well_formed_ = false;
      code_unit = replacement_char;
    }
    *(dest++) = code_unit;
    return dest;
  }

  /// Call once the entire input sequence has been fed to operator(). This
  /// function ensures that the sequence did not end with a partial code point.
  ///
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  The output iterator.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr OutputIterator end_cp(OutputIterator dest) const {
    return dest;
  }

  /// Call once the entire input sequence has been fed to operator(). This
  /// function ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <ICUBABY_CONCEPT_OUTPUT_ITERATOR(output_type) OutputIterator>
  constexpr iterator<transcoder, OutputIterator> end_cp(iterator<transcoder, OutputIterator> dest) {
    auto coder = dest.transcoder();
    assert(coder == this);
    return {coder, coder->end_cp(dest.base())};
  }

  /// \returns True if the input represented well formed UTF-32.
  [[nodiscard]] constexpr bool well_formed() const noexcept { return well_formed_; }
  /// \returns True if a partial code-point has been passed to operator() and
  /// false otherwise.
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  [[nodiscard]] constexpr bool partial() const noexcept { return false; }

private:
  /// True if the input consumed is well formed, false otherwise.
  bool well_formed_ = true;
};

/// \brief A shorter name for the UTF-8 to UTF-8 transcoder.
/// This, assuming well-formed input, represents no change.
using t8_8 = transcoder<char8, char8>;
/// A shorter name for the UTF-8 to UTF-16 transcoder.
using t8_16 = transcoder<char8, char16_t>;
/// A shorter name for the UTF-8 to UTF-32 transcoder.
using t8_32 = transcoder<char8, char32_t>;
/// A shorter name for the UTF-16 to UTF-8 transcoder.
using t16_8 = transcoder<char16_t, char8>;
/// \brief A shorter name for the UTF-16 to UTF-16 transcoder.
/// This, assuming well-formed input, represents no change.
using t16_16 = transcoder<char16_t, char16_t>;
/// A shorter name for the UTF-16 to UTF-32 transcoder.
using t16_32 = transcoder<char16_t, char32_t>;

/// A shorter name for the UTF-32 to UTF-8 transcoder.
using t32_8 = transcoder<char32_t, char8>;
/// A shorter name for the UTF-32 to UTF-16 transcoder.
using t32_16 = transcoder<char32_t, char16_t>;
/// \brief A shorter name for the UTF-32 to UTF-32 transcoder.
/// This, assuming well-formed input, represents no change.
using t32_32 = transcoder<char32_t, char32_t>;

/// A shorter name for the UTF-8 "byte transcoder" which consumes bytes in unknown input encoding and produces UTF-8.
using tx_8 = transcoder<std::byte, char8>;
/// A shorter name for the UTF-16 "byte transcoder" which consumes bytes in unknown input encoding and produces UTF-16.
using tx_16 = transcoder<std::byte, char16_t>;
/// A shorter name for the UTF-32 "byte transcoder" which consumes bytes in unknown input encoding and produces UTF-32.
using tx_32 = transcoder<std::byte, char32_t>;

#if ICUBABY_HAVE_RANGES && ICUBABY_HAVE_CONCEPTS

/// \brief icubaby C++ 20 ranges support types.
namespace ranges {

/// \brief A range adaptor for lazily converting between Unicode encodings.
///
/// A range adaptor that represents view of an underlying sequence consisting of Unicode code points in the encoding
/// given by FromEncoding and produces the equivalent code points in the encoding given by ToEncoding.
///
/// \tparam FromEncoding  The encoding used by the underlying sequence.
/// \tparam ToEncoding  The encoding that will be produced by this range adaptor.
/// \tparam View  The type of the underlying view.
template <typename FromEncoding, typename ToEncoding, std::ranges::input_range View>
  requires std::ranges::view<View>
class transcode_view : public std::ranges::view_interface<transcode_view<FromEncoding, ToEncoding, View>> {
public:
  class iterator;
  class sentinel;

  /// \brief Default initializes the base view of a new transcode_view instance.
  transcode_view()
    requires std::default_initializable<View>
  = default;
  /// \brief Initializes the base view of a new transcode_view instance.
  constexpr explicit transcode_view(View base) : base_(std::move(base)) {}

  /// \returns The base view.
  constexpr View base() const&
    requires std::copy_constructible<View>
  {
    return base_;
  }
  /// \returns Moves the base view out of this object.
  constexpr View base() && { return std::move(base_); }

  /// \brief Obtains the beginning iterator of a transcode_view.
  constexpr auto begin() const { return iterator{*this, std::ranges::begin(base_)}; }

  /// \brief Obtains the sentinel denoting the end of transcode_view.
  constexpr auto end() const {
    if constexpr (std::ranges::common_range<View>) {
      return iterator{*this, std::ranges::end(base_)};
    } else {
      return sentinel{*this};
    }
  }

  /// \returns True if the input processed was well formed.
  [[nodiscard]] constexpr bool well_formed() const noexcept { return well_formed_; }

private:
  /// The underlying view from which input is drawn.
  ICUBABY_NO_UNIQUE_ADDRESS View base_ = View();
  /// True if the input consumed is well formed, false otherwise.
  mutable bool well_formed_ = true;
};

/// The maximum number of bytes that can be produced by a single code-unit being passed to a transcoder.
template <typename FromEncoding, typename ToEncoding>
inline constexpr auto max_output_bytes = longest_sequence_v<ToEncoding>;

/// \brief The maximum number of bytes produced by a single code-unit being passed to a transcoder consuming UTF-16.
///
/// The value six comes from the worst-case output which happens when converting a code-units
/// char16_t{0xD902}, char16_t{0xFFFF} (that is a high surrogate followed by the maximum 16-bit value). This will
/// cause the second invocation of the UTF-16 to UTF-8 transcoder to produce the REPLACEMENT CHAR and "Not a Character"
/// code-points which are each 3 bytes when encoded as UTF-8.
template <> inline constexpr auto max_output_bytes<char16_t, icubaby::char8> = std::size_t{6};

/// \brief The maximum number of bytes produced by a single code-unit being passed to a transcoder consuming UTF-32.
template <> inline constexpr auto max_output_bytes<char16_t, char32_t> = std::size_t{2};

/// \brief The iterator type of transcode_view.
template <typename FromEncoding, typename ToEncoding, std::ranges::input_range View>
  requires std::ranges::view<View>
class transcode_view<FromEncoding, ToEncoding, View>::iterator {
public:
  /// Defines this class as fulfilling the requirements of a forward iterator.
  using iterator_category = std::forward_iterator_tag;
  /// Define this class as following the forward iterator concept.
  using iterator_concept = std::forward_iterator_tag;

  /// The type produced by this iterator.
  using value_type = ToEncoding;

  /// A type that can be used to identify distance between iterators.
  using difference_type = std::ranges::range_difference_t<View>;

  iterator()
    requires std::default_initializable<std::ranges::iterator_t<View>>
  = default;
  constexpr iterator(transcode_view const& parent, std::ranges::iterator_t<View const> const& current)
      : current_{current}, parent_{&parent}, state_{current} {
    assert(state_.empty());
    // Prime the input state so that dereferencing the iterator will yield the first of the output code-units.
    if (current != std::ranges::end(parent_->base_)) {
      current_ = state_.fill(parent_);
    }
  }

  /// \brief Returns the underlying view
  constexpr std::ranges::iterator_t<View> const& base() const& noexcept { return current_; }
  /// \brief Returns the underlying view
  constexpr std::ranges::iterator_t<View> base() && { return std::move(current_); }

  constexpr value_type const& operator*() const { return state_.front(); }
  constexpr std::ranges::iterator_t<View> operator->() const { return state_.front(); }

  constexpr iterator& operator++() {
    state_.advance();
    if (state_.empty()) {
      // We've exhausted the stashed output code units. Refill the buffer and reset.
      current_ = state_.fill(parent_);
    }
    return *this;
  }
  constexpr void operator++(int) { ++*this; }
  constexpr iterator operator++(int)
    requires std::ranges::forward_range<View>
  {
    auto result = *this;
    ++*this;
    return result;
  }

  friend constexpr bool operator==(iterator const& lhs, iterator const& rhs)
    requires std::equality_comparable<std::ranges::iterator_t<View>>
  {
    return lhs.current_ == rhs.current_ && lhs.parent_ == rhs.parent_;
  }

private:
  /// \brief The state class is responsible for transforming the input values to output code units.
  ///
  /// It maintains a code point's worth of output code units in its internal buffer. These may be accessed by the
  /// owning iterator using the front() and advance() member functions. Once the buffer is empty, it is refilled using
  /// the fill() function.
  class state {
  public:
    /// \brief Initializes the state and primes the internal buffer with an initial code-point read from the input.
    ///
    /// \param iter  An iterator referencing the next element in the input range to be consumed.
    constexpr explicit state(std::ranges::iterator_t<View const> iter) : next_{std::move(iter)} {}
    constexpr state() = default;

    /// Returns true if the output buffer is empty and false otherwise.
    [[nodiscard]] constexpr bool empty() const noexcept {
      assert(first_ <= last_ && last_ <= out_.size() && "first_ and last_ must be valid indexes in the out_ array");
      return first_ == last_;
    }

    /// Returns the first element from the range of code units forming the current code point.
    [[nodiscard]] constexpr auto& front() const noexcept {
      assert(!this->empty() && "The out_ array must not be empty when front() is called");
      return out_[first_];
    }

    /// Removes the first element from the range of code units forming the current code point.
    constexpr void advance() noexcept {
      assert(!this->empty() && "The out_ array must not be empty when advance() is called");
      ++first_;
    }

    /// \brief Consumes enough code-units from the base iterator to form a single code-point.
    ///
    /// The resulting code-units in the output encoding can be sequentially accessed using the front() and
    /// advance() methods.
    ///
    /// \param parent  The view from which input values are to be consumed.
    /// \returns The updated base iterator.
    constexpr std::ranges::iterator_t<View const> fill(transcode_view const* parent);

  private:
    /// The type of the output buffer. This is sized so that it allows for the largest number of bytes that the
    /// transcoder can produce.
    using out_type = std::array<ToEncoding, max_output_bytes<FromEncoding, ToEncoding>>;
    /// Output buffer iterator type.
    using iterator = typename out_type::iterator;

    /// An iterator referencing the next input code-unit to be consumed.
    std::ranges::iterator_t<View const> next_{};
    /// The container into which the transcoder's output will be written.
    out_type out_{};
    /// The transcoder used to convert a series of code-units in the source encoding to the destination encoding.
    transcoder<FromEncoding, ToEncoding> transcoder_;

    /// The number of bits allocated for the first_ and last_ members.
    /// Must be enough to represent all valid indexes in out_type.
    static constexpr auto valid_range_bits = 4U;
    static_assert(out_type{}.size() < std::size_t{1} << valid_range_bits,
                  "There are not sufficient bits to represent indexes in out_type");

    /// \brief The index of the start of the valid range of code units in the state::out_ container.
    ///
    /// Together with the last_ field, determines the code-units to be produced when the view is dereferenced.
    std::uint_least8_t first_ : valid_range_bits = 0;
    /// \brief The index one beyond the end of the the valid range of code units in the state::out_ container.
    ///
    /// Together with the last_ field, determines the code-units to be produced when the view is dereferenced.
    std::uint_least8_t last_ : valid_range_bits = 0;
  };
  std::ranges::iterator_t<View const> current_{};
  transcode_view const* parent_ = nullptr;
  mutable state state_{};
};

template <typename FromEncoding, typename ToEncoding, std::ranges::input_range View>
  requires std::ranges::view<View>
constexpr std::ranges::iterator_t<View const> transcode_view<FromEncoding, ToEncoding, View>::iterator::state::fill(
    transcode_view const* parent) {
  auto result = next_;
  assert(this->empty() && "out_ was not empty when fill called");

  auto const out_begin = out_.begin();
  auto out_it = out_begin;
  auto const input_end = std::ranges::end(parent->base_);
  // Loop until we've produced a code-point's worth of code-units in the out container, or we've run out of input.
  while (out_it == out_begin && next_ != input_end) {
    out_it = transcoder_(*next_, out_it);
    assert(out_it >= out_begin && out_it <= out_.end() && "out_ buffer overflow!");
    ++next_;
  }
  if (next_ == input_end) {
    // We've consumed the entire input so tell the transcoder and get any final output.
    out_it = transcoder_.end_cp(out_it);
  }
  assert(out_it >= out_begin && out_it <= out_.end() && "out_ buffer overflow!");
  if (!transcoder_.well_formed()) {
    parent->well_formed_ = false;
  }
  first_ = std::uint_least8_t{0};
  last_ = static_cast<std::uint_least8_t>(out_it - out_begin);
  return result;
}

/// \brief The sentinel type of transcode_view when the underlying view is not a common_range.
template <typename FromEncoding, typename ToEncoding, std::ranges::input_range View>
  requires std::ranges::view<View>
class transcode_view<FromEncoding, ToEncoding, View>::sentinel {
public:
  sentinel() = default;
  /// Derives the value of this sentinel instance from the end of the associated view.
  constexpr explicit sentinel(transcode_view const& parent) : end_{std::ranges::end(parent.base_)} {}
  /// \returns The underlying view's end sentinel
  constexpr std::ranges::sentinel_t<View> base() const { return end_; }
  /// \brief Compares an iterator and sentinel for equality.
  friend constexpr bool operator==(iterator const& lhs, sentinel const& rhs) { return lhs.base() == rhs.end_; }

private:
  std::ranges::sentinel_t<View> end_{};  ///< The underlying view's end sentinel
};

namespace views {

/// \tparam FromEncoding  The encoding used by the underlying sequence.
/// \tparam ToEncoding  The encoding that will be produced by this adaptor.
template <typename FromEncoding, typename ToEncoding> class transcode_range_adaptor {
public:
  template <std::ranges::viewable_range Range> constexpr auto operator()(Range&& range) const {
    return transcode_view<FromEncoding, ToEncoding, std::ranges::views::all_t<Range>>{std::forward<Range>(range)};
  }
};

/// \tparam FromEncoding  The encoding used by the underlying sequence.
/// \tparam ToEncoding  The encoding that will be produced.
/// \tparam Range  The type of the range that will be consumed.
template <typename FromEncoding, typename ToEncoding, std::ranges::viewable_range Range>
constexpr auto operator|(Range&& range, transcode_range_adaptor<FromEncoding, ToEncoding> const& adaptor) {
  return adaptor(std::forward<Range>(range));
}

/// \tparam FromEncoding  The encoding used by the underlying sequence.
/// \tparam ToEncoding  The encoding that will be produced.
template <typename FromEncoding, typename ToEncoding>
inline constexpr auto transcode = transcode_range_adaptor<FromEncoding, ToEncoding>{};

}  // end namespace views

}  // end namespace ranges

namespace views = ranges::views;
#endif  // ICUBABY_HAVE_RANGES && ICUBABY_HAVE_CONCEPTS

}  // end namespace icubaby

#ifdef ICUBABY_INSIDE_NS
}  // end namespace ICUBABY_INSIDE_NS
#endif

#endif  // MIDI2_ICUBABY_HPP
