//===-- Dispatcher Concept ----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_DISPATCHER_HPP
#define MIDI2_DISPATCHER_HPP

#include <concepts>
#include <utility>

namespace midi2 {

/// The dispatcher types provided by the library implement the protocol defined by this concept.
template <typename Config, typename InputType, typename T>
concept dispatcher = requires(T v) {
  { v.dispatch(InputType{}) };
  { v.config() } -> std::convertible_to<Config>;
  { std::as_const(v).config() } -> std::convertible_to<Config const>;
  { v.reset() };
};

}  // end namespace midi2

#endif  // MIDI2_DISPATCHER_HPP
