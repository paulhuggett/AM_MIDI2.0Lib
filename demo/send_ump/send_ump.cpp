//===-- Demo Send UMP ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <chrono>
#include <cstdlib>
#include <format>
#include <iostream>
#include <ranges>
#include <thread>

// The midi2 library umbrella include file
#include <midi2/midi2.hpp>

using namespace std::chrono_literals;

static auto transmit(std::uint32_t word) -> std::error_code {
  // ... transmit the 32 bit word ...
  std::cout << std::format("0x{:02X} ", word);
  // For this demo, always return success. When transmitting over a real interface,
  // return an appropriate error code on failure.
  return {};
}

static auto send_note_on(std::uint8_t channel, std::uint8_t note, std::uint16_t velocity) -> std::error_code {
  // Create an instance of the type that represents the UMP message to be sent. Set
  // the fields of the message as desired.
  auto const message = midi2::ump::m2cvm::note_on{}.group(0).channel(channel).note(note).velocity(velocity);

  // Invoke a function for each of the words that make up the complete message. There's
  // no need for this code to understand the layout or size of the message.
  return midi2::ump::apply(message, transmit);
}

template <std::ranges::input_range R>
  requires(std::same_as<std::ranges::range_value_t<R>, std::uint8_t>)
static auto notes_off(R const& range, std::uint8_t channel, std::uint16_t velocity) -> std::error_code {
  auto noff = midi2::ump::m2cvm::note_off{}.group(0).channel(channel).velocity(velocity);
  for (auto const note : range) {
    // Adjust the value of the note field.
    noff.note(note);
    if (auto const err = midi2::ump::apply(noff, transmit)) {
      return err;
    }
  }
  return {};
}

int main() {
  int exit_code = EXIT_SUCCESS;
  try {
    constexpr auto channel = std::uint8_t{1};
    constexpr auto velocity = std::uint16_t{10000};
    constexpr std::array notes = {std::uint8_t{60}, std::uint8_t{64}, std::uint8_t{67}};
    for (auto const note : notes) {
      if (auto const err = send_note_on(channel, note, velocity)) {
        // The library itself does not throw or catch exceptions, but throwing is the simplest
        // way to handle errors in this demo.
        throw std::system_error(err);
      }
    }
    std::cout << std::endl;
    std::this_thread::sleep_for(500ms);

    notes_off(notes, channel, velocity);
    std::cout << std::endl;
  } catch (std::exception const& ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    exit_code = EXIT_FAILURE;
  } catch (...) {
    std::cerr << "An unknown error\n";
    exit_code = EXIT_FAILURE;
  }
  return exit_code;
}
