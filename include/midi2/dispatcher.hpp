//===-- Dispatcher Concept ----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file dispatcher.hpp
/// \brief The dispatcher concept for MIDI message dispatchers.

#ifndef MIDI2_DISPATCHER_HPP
#define MIDI2_DISPATCHER_HPP

#include <concepts>
#include <type_traits>
#include <utility>

namespace midi2 {

/// The dispatcher types provided by the library implement the protocol defined by this concept.
///
/// - `void reset()`: restore the dispatcher to its initial state. Any partially processed messages are dropped.

template <typename Config, typename InputType, typename T>
concept dispatcher = requires(T v) {
  /// Type of input messages
  requires std::same_as<typename std::remove_reference_t<T>::input_type, InputType>;
  /// The configuration type.
  requires std::same_as<typename std::remove_reference_t<T>::config_type,
                        std::remove_reference_t<std::unwrap_reference_t<Config>>>;

  { v.dispatch(InputType{}) };
  { v.config() } -> std::convertible_to<std::unwrap_reference_t<Config>&>;
  { std::as_const(v).config() } -> std::convertible_to<std::remove_reference_t<std::unwrap_reference_t<Config>> const&>;
  { v.reset() };
};

}  // end namespace midi2

#endif  // MIDI2_DISPATCHER_HPP
