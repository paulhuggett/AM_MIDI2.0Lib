//===-- UMP to Bytestream -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMP_UMP_TO_BYTESTREAM_HPP
#define MIDI2_UMP_UMP_TO_BYTESTREAM_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

#include "midi2/adt/fifo.hpp"
#include "midi2/bytestream/bytestream_types.hpp"
#include "midi2/translator.hpp"
#include "midi2/ump/ump_dispatcher.hpp"
#include "midi2/ump/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2::bytestream {

/// \brief An integral constant which holds the number of bytes of a specific message type.
template <status> struct message_size {};

/// \brief An integral constant which holds the number of bytes of a note-off message.
template <> struct message_size<status::note_off> : std::integral_constant<unsigned, 3> {};
/// \brief An integral constant which holds the number of bytes of a note-on message.
template <> struct message_size<status::note_on> : std::integral_constant<unsigned, 3> {};
/// \brief An integral constant which holds the number of bytes of a poly-pressure message.
template <> struct message_size<status::poly_pressure> : std::integral_constant<unsigned, 3> {};
/// \brief An integral constant which holds the number of bytes of a continuous-controller message.
template <> struct message_size<status::cc> : std::integral_constant<unsigned, 3> {};
/// \brief An integral constant which holds the number of bytes of a program-change message.
template <> struct message_size<status::program_change> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of bytes of a channel-pressure message.
template <> struct message_size<status::channel_pressure> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of bytes of a pitch-bend message.
template <> struct message_size<status::pitch_bend> : std::integral_constant<unsigned, 3> {};

// System Common Messages
/// \brief An integral constant which holds the number of bytes of a system exclusive start
///   message.
template <> struct message_size<status::sysex_start> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of bytes of a timing-code message.
template <> struct message_size<status::timing_code> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of bytes of a song-position-pointer message.
template <> struct message_size<status::spp> : std::integral_constant<unsigned, 3> {};
/// \brief An integral constant which holds the number of bytes of a song-select message.
template <> struct message_size<status::song_select> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of bytes of a tune-request message.
template <> struct message_size<status::tune_request> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of bytes of a system-exclusive-stop message.
template <> struct message_size<status::sysex_stop> : std::integral_constant<unsigned, 1> {};
// System Realtime Messages
/// \brief An integral constant which holds the number of bytes of a timing-clock message.
template <> struct message_size<status::timing_clock> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of bytes of a sequence-start message.
template <> struct message_size<status::sequence_start> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of bytes of a sequence-continue message.
template <> struct message_size<status::sequence_continue> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of bytes of a sequence-stop message.
template <> struct message_size<status::sequence_stop> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of bytes of an active-sensing message.
template <> struct message_size<status::active_sensing> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of bytes of a system-reset message.
template <> struct message_size<status::system_reset> : std::integral_constant<unsigned, 1> {};

/// \brief Converts UMP messages to a MIDI 1.0 Bytestream
class ump_to_bytestream {
public:
  /// \brief The type of input from a 32-bit UMP stream
  using input_type = std::uint32_t;
  /// \brief The type of output to a bytestream
  using output_type = std::byte;

  constexpr ump_to_bytestream() = default;

  /// Checks if the output has no elements
  [[nodiscard]] constexpr bool empty() const noexcept { return context_.output.empty(); }
  /// \brief Pops and returns the next available byte for the bytestream
  /// \return The next available byte
  /// \pre !empty()
  [[nodiscard]] constexpr output_type pop() noexcept {
    assert(!empty());
    return context_.output.pop_front();
  }

  /// \brief Provides a word of UMP input to the translator
  /// \param ump The byte of input to be translated
  constexpr void push(input_type const ump) { dispatcher_.dispatch(ump); }

  /// \brief Filter the output to only include messages from the specified groups
  /// \param group_bitmap A bitmap indicating which groups should be included in the output
  constexpr void group_filter(std::uint16_t const group_bitmap) noexcept {
    context_.only_groups = group_bitmap == 0 ? std::uint16_t{0xFFFF} : group_bitmap;
  }
  /// \brief Restore the translator to its original state.
  /// Any in-flight messages are lost.
  constexpr void reset() noexcept { context_.reset(); }

private:
  struct context_type {
    constexpr void push_back(std::byte const s) {
      if (!is_status_byte(s) || is_system_real_time_message(s)) {
        // Not a status byte or a system real-time message, so always emit. Don't update running status.
        output.push_back(s);
      } else {
        // If this doesn't match the current running status byte then emit it.
        if (!status || *status != s) {
          output.push_back(s);
        }
        // Update running status.
        status = s;
      }
    }

    /// \returns true if the message should be filtered; false if the message should be allowed.
    [[nodiscard]] constexpr bool filter_message(unsigned const group) const {
      assert(group < 16U);
      return (only_groups & (1U << group)) == 0U;
    }
    template <typename T> [[nodiscard]] constexpr bool filter_message(T const& in) const {
      return (only_groups & (1U << in.group())) == 0U;
    }
    constexpr void reset() noexcept {
      only_groups = std::uint16_t{0xFFFF};
      status.reset();
      output.clear();
    }
    std::uint16_t only_groups =
        std::uint16_t{0xFFFF};        ///< A bitmap indicating which groups should be included in the output
    std::optional<std::byte> status;  ///< Last status emitted.
    adt::fifo<std::byte, 8> output;
  };

  struct to_bytestream_config {
    ///\brief Handlers for the SYSTEM COMMON and SYSTEM REAL TIME groups of messages.
    class system {
    public:
      static constexpr void midi_time_code(context_type* const ctxt, ump::system::midi_time_code const& in) {
        static_assert(message_size<status::timing_code>() == 2);
        system::push(ctxt, in.group(), status::timing_code, std::byte{in.time_code()});
      }
      static constexpr void song_position_pointer(context_type* const ctxt,
                                                  ump::system::song_position_pointer const& in) {
        static_assert(message_size<status::spp>() == 3);
        system::push(ctxt, in.group(), status::spp, std::byte{in.position_lsb()}, std::byte{in.position_msb()});
      }
      static constexpr void song_select(context_type* const ctxt, ump::system::song_select const& in) {
        static_assert(message_size<status::song_select>() == 2);
        system::push(ctxt, in.group(), status::song_select, std::byte{in.song()});
      }
      static constexpr void tune_request(context_type* const ctxt, ump::system::tune_request const& in) {
        static_assert(message_size<status::tune_request>() == 1);
        system::push(ctxt, in.group(), status::tune_request);
      }
      static constexpr void timing_clock(context_type* const ctxt, ump::system::timing_clock const& in) {
        static_assert(message_size<status::timing_clock>() == 1);
        system::push(ctxt, in.group(), status::timing_clock);
      }
      static constexpr void seq_start(context_type* const ctxt, ump::system::sequence_start const& in) {
        static_assert(message_size<status::sequence_start>() == 1);
        system::push(ctxt, in.group(), status::sequence_start);
      }
      static constexpr void seq_continue(context_type* const ctxt, ump::system::sequence_continue const& in) {
        static_assert(message_size<status::sequence_continue>() == 1);
        system::push(ctxt, in.group(), status::sequence_continue);
      }
      static constexpr void seq_stop(context_type* const ctxt, ump::system::sequence_stop const& in) {
        static_assert(message_size<status::sequence_stop>() == 1);
        system::push(ctxt, in.group(), status::sequence_stop);
      }
      static constexpr void active_sensing(context_type* const ctxt, ump::system::active_sensing const& in) {
        static_assert(message_size<status::active_sensing>() == 1);
        system::push(ctxt, in.group(), status::active_sensing);
      }
      static constexpr void reset(context_type* const ctxt, ump::system::reset const& in) {
        static_assert(message_size<status::system_reset>() == 1);
        system::push(ctxt, in.group(), status::system_reset);
      }

    private:
      static constexpr void push(context_type* const ctxt, unsigned const group, status const s) {
        if (!ctxt->filter_message(group)) {
          ctxt->push_back(to_byte(s));
        }
      }
      static constexpr void push(context_type* const ctxt, unsigned const group, status const s, std::byte const b1) {
        if (!ctxt->filter_message(group)) {
          ctxt->push_back(to_byte(s));
          ctxt->push_back(b1);
        }
      }
      static constexpr void push(context_type* const ctxt, unsigned const group, status const s, std::byte const b1,
                                 std::byte const b2) {
        if (!ctxt->filter_message(group)) {
          ctxt->push_back(to_byte(s));
          ctxt->push_back(b1);
          ctxt->push_back(b2);
        }
      }
    };
    /// \brief Handlers for the MIDI 1.0 CHANNEL VOICE group of messages.
    class m1cvm {
    public:
      static constexpr void note_off(context_type* const ctxt, ump::m1cvm::note_off const& in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(message_size<status::note_off>() == 3);
        ctxt->push_back(to_byte(status::note_off) | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.note()});
        ctxt->push_back(std::byte{in.velocity()});
      }
      static constexpr void note_on(context_type* const ctxt, ump::m1cvm::note_on const& in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(message_size<status::note_on>() == 3);
        ctxt->push_back(to_byte(status::note_on) | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.note()});
        ctxt->push_back(std::byte{in.velocity()});
      }
      static constexpr void poly_pressure(context_type* const ctxt, ump::m1cvm::poly_pressure const& in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(message_size<status::poly_pressure>() == 3);
        ctxt->push_back(to_byte(status::poly_pressure) | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.note()});
        ctxt->push_back(std::byte{in.pressure()});
      }
      static constexpr void control_change(context_type* const ctxt, ump::m1cvm::control_change const& in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(message_size<status::cc>() == 3);
        ctxt->push_back(to_byte(status::cc) | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.controller()});
        ctxt->push_back(std::byte{in.value()});
      }
      static constexpr void program_change(context_type* const ctxt, ump::m1cvm::program_change const& in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(message_size<status::program_change>() == 2);
        ctxt->push_back(to_byte(status::program_change) | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.program()});
      }
      static constexpr void channel_pressure(context_type* const ctxt, ump::m1cvm::channel_pressure const& in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(message_size<status::channel_pressure>() == 2);
        ctxt->push_back(to_byte(status::channel_pressure) | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.data()});
      }
      static constexpr void pitch_bend(context_type* const ctxt, ump::m1cvm::pitch_bend const& in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(message_size<status::pitch_bend>() == 3);
        ctxt->push_back(to_byte(status::pitch_bend) | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.lsb_data()});
        ctxt->push_back(std::byte{in.msb_data()});
      }
    };
    /// \brief Handlers for the DATA 64 BIT group of messages.
    class data64 {
    public:
      static void sysex7_in_1(context_type* const ctxt, ump::data64::sysex7_in_1 const& in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        if (in.number_of_bytes() > 0) {
          ctxt->push_back(sysex_start);
          data64::write_sysex_bytes(ctxt, in);
          ctxt->push_back(sysex_stop);
        }
      }
      static void sysex7_start(context_type* const ctxt, ump::data64::sysex7_start const& in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        ctxt->push_back(sysex_start);
        data64::write_sysex_bytes(ctxt, in);
      }
      static void sysex7_continue(context_type* const ctxt, ump::data64::sysex7_continue const& in) {
        // Skip this message if we're filtering the associated group or if we didn't see a preceding sysex
        // start message.
        if (ctxt->filter_message(in) || ctxt->status != sysex_start) {
          return;
        }
        data64::write_sysex_bytes(ctxt, in);
      }
      static void sysex7_end(context_type* const ctxt, ump::data64::sysex7_end const& in) {
        // Skip this message if we're filtering the associated group or if we didn't see a preceding sysex
        // start message.
        if (ctxt->filter_message(in) || ctxt->status != sysex_start) {
          return;
        }
        data64::write_sysex_bytes(ctxt, in);
        ctxt->push_back(sysex_stop);
      }

    private:
      static constexpr auto sysex_start = to_byte(status::sysex_start);
      static constexpr auto sysex_stop = to_byte(status::sysex_stop);

      template <typename T> static void write_sysex_bytes(context_type* const ctxt, T const& in) {
        auto const number_of_bytes = in.number_of_bytes();
        if (number_of_bytes > 0) {
          ctxt->push_back(std::byte{in.data0()});
        }
        if (number_of_bytes > 1) {
          ctxt->push_back(std::byte{in.data1()});
        }
        if (number_of_bytes > 2) {
          ctxt->push_back(std::byte{in.data2()});
        }
        if (number_of_bytes > 3) {
          ctxt->push_back(std::byte{in.data3()});
        }
        if (number_of_bytes > 4) {
          ctxt->push_back(std::byte{in.data4()});
        }
        if (number_of_bytes > 5) {
          ctxt->push_back(std::byte{in.data5()});
        }
      }
    };
    context_type* context = nullptr;
    [[no_unique_address]] ump::dispatcher_backend::utility_null<decltype(context)> utility{};
    [[no_unique_address]] class system system {};
    [[no_unique_address]] class m1cvm m1cvm {};
    [[no_unique_address]] class data64 data64 {};
    [[no_unique_address]] ump::dispatcher_backend::m2cvm_null<decltype(context)> m2cvm{};
    [[no_unique_address]] ump::dispatcher_backend::data128_null<decltype(context)> data128{};
    [[no_unique_address]] ump::dispatcher_backend::stream_null<decltype(context)> stream{};
    [[no_unique_address]] ump::dispatcher_backend::flex_data_null<decltype(context)> flex{};
  };
  context_type context_;
  ump::ump_dispatcher<to_bytestream_config> dispatcher_{to_bytestream_config{.context = &context_}};
};

static_assert(translator<std::uint32_t, std::byte, ump_to_bytestream>);

}  // end namespace midi2::bytestream

#endif  // MIDI2_UMP_UMP_TO_BYTESTREAM_HPP
