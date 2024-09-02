//
//  ci7text.hpp
//  midi2
//
//  Created by Paul Bowen-Huggett on 01/09/2024.
//

#include "icubaby/icubaby.hpp"

template <std::output_iterator<char> OutputIterator>
OutputIterator convert_from_32 (char32_t code_unit, OutputIterator dest) {
  if (code_unit < (1U << 7)) {
    if (code_unit == '\\') {
      *(dest++) = '\\';
    }
    *(dest++) = static_cast<char> (code_unit);
    return dest;
  }

  std::array<char16_t, 2> out16;
  auto const first = out16.begin();
  auto const last = icubaby::t32_16{}(code_unit, first);
  auto const to_hex = [] (unsigned const v) {
    return static_cast<char>(v + ((v < 10) ? '0' : 'A' - 10));
  };
  std::for_each (first, last, [&dest,&to_hex] (char16_t const c) {
    *(dest++) = '\\';
    *(dest++) = 'u';
    *(dest++) = to_hex((static_cast<std::uint16_t> (c) >> 12) & 0x0F);
    *(dest++) = to_hex((static_cast<std::uint16_t> (c) >> 8) & 0x0F);
    *(dest++) = to_hex((static_cast<std::uint16_t> (c) >> 4) & 0x0F);
    *(dest++) = to_hex(static_cast<std::uint16_t> (c) & 0x0F);
  });

  return dest;
}


template <icubaby::unicode_char_type InputEncoding>
class transcoder {
public:
  /// The type of the code units consumed by this transcoder.
  using input_type = InputEncoding;
  /// The type of the code units produced by this transcoder.
  using output_type = char;

  /// This member function is the heart of the transcoder. It accepts a single byte or code unit in the input encoding
  /// and, once an entire code point has been consumed, produces the equivalent code point expressed in the output
  /// encoding. Malformed input is detected and replaced with the Unicode replacement character (U+FFFD REPLACEMENT
  /// CHARACTER).
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type transcoder::output_type can be written.
  /// \param code_unit  A code unit in the source encoding.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <std::output_iterator<output_type> OutputIterator>
  OutputIterator operator() (input_type code_unit, OutputIterator dest) noexcept {
    static_assert (icubaby::longest_sequence_v<char32_t> == 1);
    char32_t out32 = 0;
    auto * const out_pos = src_to_32_(code_unit, &out32);
    if (out_pos != &out32)  {
      dest = convert_from_32 (out32, dest);
    }
    return dest;
  }

  /// Call once the entire input sequence has been fed to \ref transcoder-call-operator "operator()". This function
  /// ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type transcoder::output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <std::output_iterator<output_type> OutputIterator>
  constexpr OutputIterator end_cp (OutputIterator dest) {
    char32_t out32 = 0;
    auto * const out_pos = src_to_32_.end_cp(&out32);
    if (out_pos != &out32)  {
      dest = convert_from_32 (out32, dest);
    }
    return dest;
  }

  /// Indicates whether the input was well formed
  /// \returns True if the input was well formed.
  [[nodiscard]] constexpr bool well_formed () const noexcept { return src_to_32_.well_formed(); }

  /// \brief Indicates whether a "partial" code point has been passed to \ref transcoder-call-operator "operator()".
  ///
  /// If true, one or more code units are required to build the complete code point.
  ///
  /// \returns True if a partial code-point has been passed to \ref transcoder-call-operator "operator()" and
  ///   false otherwise.
  [[nodiscard]] constexpr bool partial () const noexcept { return src_to_32_.partial(); }

private:
  icubaby::transcoder<input_type, char32_t> src_to_32_;
};
