//===-- Demo Send UMP ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <system_error>

// The midi2 library umbrella include file
#include <midi2/midi2.hpp>

namespace {

constexpr std::error_code transmit(std::uint32_t word) {
  // ... transmit the 32 bit word ...
  // Here we just print the value but this code could transmit it or record it in
  // a container (e.g. a FIFO) for later use.
  std::cout << "0x" << std::setw(8) << std::setfill('0') << std::hex << word << ' ';
  // For this demo, always return success. When transmitting over a real interface,
  // return an appropriate error code on failure.
  return {};
}

constexpr std::error_code send_note_on(std::uint8_t group, std::uint8_t channel, std::uint8_t note,
                                       std::uint16_t velocity) {
  // Create an instance of the type that represents the UMP message to be sent. Set
  // the fields of the message as desired.
  auto const message = midi2::ump::m2cvm::note_on{}.group(group).channel(channel).note(note).velocity(velocity);

  // Invoke a function for each of the words that make up the complete message. There's
  // no need for this code to understand the layout or size of the message.
  return midi2::ump::apply(message, transmit);
}

template <std::ranges::input_range R>
  requires(std::same_as<std::ranges::range_value_t<R>, std::uint8_t>)
constexpr std::error_code notes_off(R const& range, std::uint8_t group, std::uint8_t channel, std::uint16_t velocity) {
  auto noff = midi2::ump::m2cvm::note_off{}.group(group).channel(channel).velocity(velocity);
  for (auto const note : range) {
    // Adjust the value of the note field.
    noff.note(note);
    // Call transmit() for each word of the note-off message.
    if (auto const err = midi2::ump::apply(noff, transmit)) {
      return err;
    }
    // Print a dash to separate the individual UMP messages.
    std::cout << "- ";
  }
  return {};
}

}  // end anonymous namespace

int main() {
  constexpr auto group = std::uint8_t{0};
  constexpr auto channel = std::uint8_t{1};

  midi2::ump::apply(midi2::ump::m1cvm::program_change{}.group(group).channel(channel).program(42), transmit);
  std::cout << "- \n";

  constexpr auto velocity = std::uint16_t{10000};
  constexpr std::array notes = {std::uint8_t{60}, std::uint8_t{64}, std::uint8_t{67}};
  for (auto const note : notes) {
    if (auto const err = send_note_on(group, channel, note, velocity)) {
      // In this example, send_note_on() will never fail because transmit() always returns
      // success.
    }
    // Print a dash to separate the individual UMP messages.
    std::cout << "- ";
  }
  std::cout << '\n';

  notes_off(notes, group, channel, velocity);
  std::cout << '\n';
}
