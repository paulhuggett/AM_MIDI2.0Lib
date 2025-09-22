//===-- Demo UMP Dispatcher Function Backend ----------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// This demo program shows the UMP dispatcher being used with the provided "function" backend class.
//
// The dispatcher's backend class determines how each UMP message is handled. The "function" backend uses
// std::function<> to make it simple to use callback functions (be they lambdas or other types of callable object) as
// message handlers.
//
// Be aware that std::function<> is powerful and simple to use but may allocate memory or throw exceptions.
// For resource-constrained applications, one of the other backend options may be more suitable.

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>

#include "midi2/ump/ump_dispatcher.hpp"
#include "midi2/ump/ump_types.hpp"

int main() {
  int exit_code = EXIT_SUCCESS;
  try {
    // We must pass a "context" to the dispatcher which will be forwarded to the callbacks as they are invoked.
    // The context enables message handlers to efficiently share state but we don't need that in this simple example so
    // a struct with no members will suffice.
    struct context {};
    // Create the dispatcher with default-initialized context. Here we're using the pre-made "std::function<>" backend
    // for convenience. Production code would probably use a custom dispatcher back-end for maximum efficiency.
    auto dispatcher = midi2::ump::make_ump_function_dispatcher(context{});
    // Ask the dispatcher for its configuration object and install handlers for MIDI2 note on/off channel voice
    // messages.
    dispatcher.config()
        .m2cvm
        .on_note_off([](context, midi2::ump::m2cvm::note_off const& noff) {
          std::cout << "note off: #" << unsigned{noff.note()} << ", velocity " << noff.velocity() << '\n';
        })
        .on_note_on([](context, midi2::ump::m2cvm::note_on const& non) {
          std::cout << "note on: #" << unsigned{non.note()} << ", velocity " << non.velocity() << '\n';
        });
    // Send note-on/off messages to the dispatcher.
    for (std::uint32_t const v : {0x40913C00U, 0x7F100000U, 0x40813C00U, 0x7FFF0000U}) {
      dispatcher.dispatch(v);
    }
  } catch (std::exception const& ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    exit_code = EXIT_FAILURE;
  } catch (...) {
    std::cerr << "An unknown error\n";
    exit_code = EXIT_FAILURE;
  }
  return exit_code;
}
