//===-- Dispatcher Concept ----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_TRANSLATOR_HPP
#define MIDI2_TRANSLATOR_HPP

#include <concepts>

/// The translator types provided by the library implement the protocol defined by this concept.
/// Type T must define the following two types:
///
/// - A type `input_type` which is the same as InputType.
/// - A type `output_type` which is the same as OutputType.
///
/// It must also define these functions:
///
/// - `void push(InputType)`: this member function pushes an input message into the translator.
/// - `bool empty() const`: Returns true if output is available, false otherwise.
/// - `OutputType pop()`: Pulls a translated value from the object. `empty()` must return false when this function is
///   called.
/// - `void reset()`: restore the translator to its initial state. Any partially translated messages are dropped.
template <typename InputType, typename OutputType, typename T>
concept translator = requires(T v) {
  /// Type of input messages
  requires std::same_as<typename std::remove_reference_t<T>::input_type, InputType>;
  /// Type of output messages
  requires std::same_as<typename std::remove_reference_t<T>::output_type, OutputType>;

  /// Push an input message into the translator
  { v.push(InputType{}) };
  /// Is output available?
  { std::as_const(v).empty() } -> std::convertible_to<bool>;
  /// Pop an output message from the translator. empty() must be false before this function is called.
  { v.pop() } -> std::convertible_to<OutputType>;
  /// Reset the translator state
  { v.reset() };
};

#endif  // MIDI2_TRANSLATOR_HPP
