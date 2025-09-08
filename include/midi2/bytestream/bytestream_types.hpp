#ifndef MIDI2_BYTESTREAM_BYTESTREAM_TYPES_HPP
#define MIDI2_BYTESTREAM_BYTESTREAM_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace midi2::bytestream {

/// Bytestream status message codes
enum class status : std::uint8_t {
  // Channel voice messages
  note_off = 0x80,
  note_on = 0x90,
  poly_pressure = 0xA0,  ///< Polyphonic Key Pressure (Aftertouch).
  cc = 0xB0,             ///< Continuous Controller
  program_change = 0xC0,
  channel_pressure = 0xD0,  ///< Channel Pressure (Aftertouch).
  pitch_bend = 0xE0,

  // System Common/Real Time/Exclusive Messages
  sysex_start = 0xF0,  ///< Begin system exclusive data
  timing_code = 0xF1,
  spp = 0xF2,  ///< Song Position Pointer
  song_select = 0xF3,
  reserved1 = 0xF4,
  reserved2 = 0xF5,
  tune_request = 0xF6,
  sysex_stop = 0xF7,  ///< End of system exclusive
  timing_clock = 0xF8,
  reserved3 = 0xF9,
  sequence_start = 0xFA,     ///< Start the current sequence playing
  sequence_continue = 0xFB,  ///< Continue at the point the sequence was stopped
  sequence_stop = 0xFC,      ///< Stop the current sequence
  reserved4 = 0xFD,
  active_sensing = 0xFE,
  systemreset = 0xFF,
};

[[nodiscard]] constexpr bool is_system_real_time_message(std::byte const midi1_byte) noexcept {
  using enum status;
  switch (static_cast<status>(midi1_byte)) {
  case timing_clock:
  case sequence_start:
  case sequence_continue:
  case sequence_stop:
  case active_sensing:
  case systemreset: return true;
  default: return false;
  }
}

[[nodiscard]] constexpr bool is_status_byte(std::byte const midi1_byte) noexcept {
  return (midi1_byte & std::byte{0x80}) != std::byte{0x00};
}

constexpr auto s7_universal_nrt = std::byte{0x7E};
constexpr auto s7_midi_ci = std::byte{0x0D};

}  // end namespace midi2::bytestream

#endif  // MIDI2_BYTESTREAM_BYTESTREAM_TYPES_HPP
