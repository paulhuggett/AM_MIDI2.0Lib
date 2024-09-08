//
//  ci7text.hpp
//  midi2
//
//  Created by Paul Bowen-Huggett on 01/09/2024.
//

#include "icubaby/icubaby.hpp"

template <icubaby::unicode_char_type InputEncoding> class icubaby::transcoder<InputEncoding, char> {
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
    std::array<char32_t, 2> out32{};
    // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
    auto const first = std::begin (out32);
    // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
    auto const last = src_to_32_(code_unit, first);
    std::for_each(first, last, [this, &dest](char32_t c) { dest = this->convert_from_32(c, dest); });
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
    if (auto* const out_pos = src_to_32_.end_cp(&out32); out_pos != &out32) {
      dest = this->convert_from_32(out32, dest);
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

  template <std::output_iterator<char> OutputIterator>
  OutputIterator convert_from_32(char32_t code_unit, OutputIterator dest) {
    if (code_unit < (1U << 7)) {
      if (code_unit == '\\') {
        *(dest++) = '\\';
      }
      *(dest++) = static_cast<char>(code_unit);
      return dest;
    }

    std::array<char16_t, 2> out16;
    auto const first = out16.begin();
    auto const last = icubaby::t32_16{}(code_unit, first);
    auto const to_hex = [](unsigned const v) { return static_cast<char>(v + ((v < 10) ? '0' : 'A' - 10)); };
    std::for_each(first, last, [&dest, &to_hex](char16_t const c) {
      *(dest++) = '\\';
      *(dest++) = 'u';
      *(dest++) = to_hex((static_cast<std::uint16_t>(c) >> 12) & 0x0F);
      *(dest++) = to_hex((static_cast<std::uint16_t>(c) >> 8) & 0x0F);
      *(dest++) = to_hex((static_cast<std::uint16_t>(c) >> 4) & 0x0F);
      *(dest++) = to_hex(static_cast<std::uint16_t>(c) & 0x0F);
    });
    return dest;
  }
};

template <icubaby::unicode_char_type OutputEncoding> class icubaby::transcoder<char, OutputEncoding> {
public:
  /// The type of the code units consumed by this transcoder.
  using input_type = char;
  /// The type of the code units produced by this transcoder.
  using output_type = OutputEncoding;

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
  OutputIterator operator()(input_type code_unit, OutputIterator dest) noexcept {
    //    using enum state;
    if ((static_cast<std::byte>(code_unit) & std::byte{1U << 7}) != std::byte{0}) {
      well_formed_ = false;
      return dest;
    }
    switch (state_) {
    case state::normal:
      if (code_unit == '\\') {
        state_ = state::escape;
      } else {
        dest = hex_to_32_.end_cp(dest);
        *(dest++) = static_cast<output_type>(code_unit);
      }
      break;

    case state::hex1:
    case state::hex2:
    case state::hex3:
      if (this->hex_char(code_unit)) {
        state_ = static_cast<state>(static_cast<std::underlying_type_t<state>>(state_) + 1);
      } else {
        state_ = state::normal;
        hex_ = 0;
      }
      break;
    case state::hex4:
      if (this->hex_char(code_unit)) {
        dest = hex_to_32_(hex_, dest);
      }
      state_ = state::normal;
      hex_ = 0;
      break;
    case state::escape: dest = this->escape_char(code_unit, dest); break;
    }
    return dest;
  }

  /// Call once the entire input sequence has been fed to \ref transcoder-call-operator "operator()". This function
  /// ensures that the sequence did not end with a partial code point.
  ///
  /// \tparam OutputIterator  An output iterator type to which values of type transcoder::output_type can be written.
  /// \param dest  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <std::output_iterator<output_type> OutputIterator> constexpr OutputIterator end_cp(OutputIterator dest) {
    dest = hex_to_32_.end_cp(dest);
    if (!hex_to_32_.well_formed() || state_ != state::normal) {
      well_formed_ = false;
    }
    return dest;
  }

  /// Indicates whether the input was well formed
  /// \returns True if the input was well formed.
  [[nodiscard]] constexpr bool well_formed() const noexcept { return well_formed_; }  // src_to_32_.well_formed(); }

  /// \brief Indicates whether a "partial" code point has been passed to \ref transcoder-call-operator "operator()".
  ///
  /// If true, one or more code units are required to build the complete code point.
  ///
  /// \returns True if a partial code-point has been passed to \ref transcoder-call-operator "operator()" and
  ///   false otherwise.
  [[nodiscard]] constexpr bool partial() const noexcept { return false; }  // src_to_32_.partial(); }

private:
  enum class state : std::uint8_t {
    normal,
    escape,
    hex1,
    hex2,
    hex3,
    hex4,
  };
  bool well_formed_ = true;
  state state_ = state::normal;
  std::uint_least16_t hex_ = 0;
  icubaby::transcoder<char16_t, output_type> hex_to_32_;

  template <std::output_iterator<output_type> OutputIterator>
  OutputIterator escape_char(input_type code_unit, OutputIterator dest) {
    if (code_unit == 'u') {
      state_ = state::hex1;
      hex_ = 0U;
    } else {
      switch (code_unit) {
      case 'b': code_unit = static_cast<output_type>(0x0008); break;  // backspace
      case 'f': code_unit = static_cast<output_type>(0x000C); break;  // form feed
      case 'n': code_unit = static_cast<output_type>(0x000A); break;  // line feed
      case 'r': code_unit = static_cast<output_type>(0x000D); break;  // carriage return
      case 't': code_unit = static_cast<output_type>(0x0009); break;  // tab
      case '"':
      case '\\':
      case '/': break;
      default: well_formed_ = false; break;
      }
      dest = hex_to_32_.end_cp(dest);
      *(dest++) = static_cast<output_type>(code_unit);
      state_ = state::normal;
    }
    return dest;
  }

  bool hex_char(char const code_unit) {
    auto delta = std::uint_least16_t{0};
    if (code_unit >= '0' && code_unit <= '9') {
      delta = '0';
    } else if (code_unit >= 'a' && code_unit <= 'f') {
      delta = 'a' - 10;
    } else if (code_unit >= 'A' && code_unit <= 'F') {
      delta = 'A' - 10;
    } else {
      well_formed_ = false;
      return false;
    }
    assert(static_cast<std::uint_least16_t>(code_unit) >= delta);
    hex_ <<= 4;
    hex_ |= static_cast<std::uint_least16_t>(code_unit) - delta;
    return true;
  }
};

namespace midi2 {
using icubaby::transcoder;
}
