//===-- UMP to Bytestream -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMPTOBYTESTREAM_HPP
#define MIDI2_UMPTOBYTESTREAM_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

#include "midi2/adt/fifo.hpp"
#include "midi2/ump_dispatcher.hpp"
#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

template <midi2::status> struct bytestream_message_size {};

template <> struct bytestream_message_size<midi2::status::note_off> : std::integral_constant<unsigned, 3> {};
template <> struct bytestream_message_size<midi2::status::note_on> : std::integral_constant<unsigned, 3> {};
template <> struct bytestream_message_size<midi2::status::poly_pressure> : std::integral_constant<unsigned, 3> {};
template <> struct bytestream_message_size<midi2::status::cc> : std::integral_constant<unsigned, 3> {};
template <> struct bytestream_message_size<midi2::status::program_change> : std::integral_constant<unsigned, 2> {};
template <> struct bytestream_message_size<midi2::status::channel_pressure> : std::integral_constant<unsigned, 2> {};
template <> struct bytestream_message_size<midi2::status::pitch_bend> : std::integral_constant<unsigned, 3> {};

// System Common Messages
template <> struct bytestream_message_size<midi2::status::sysex_start> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::timing_code> : std::integral_constant<unsigned, 2> {};
template <> struct bytestream_message_size<midi2::status::spp> : std::integral_constant<unsigned, 3> {};
template <> struct bytestream_message_size<midi2::status::song_select> : std::integral_constant<unsigned, 2> {};
template <> struct bytestream_message_size<midi2::status::tune_request> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::sysex_stop> : std::integral_constant<unsigned, 1> {};
// System Realtime Messages
template <> struct bytestream_message_size<midi2::status::timing_clock> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::sequence_start> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::sequence_continue> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::sequence_stop> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::active_sensing> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::systemreset> : std::integral_constant<unsigned, 1> {};

class ump_to_bytestream {
public:
  using input_type = std::uint32_t;
  using output_type = std::byte;

  ump_to_bytestream() = default;

  /// Checks if the output has no elements
  [[nodiscard]] constexpr bool empty() const { return context_.output.empty(); }
  [[nodiscard]] constexpr output_type pop() { return context_.output.pop_front(); }

  void push(input_type const ump) { p_.processUMP(ump); }

  void group_filter(std::uint16_t const group_bitmap) {
    context_.only_groups = group_bitmap == 0 ? std::uint16_t{0xFFFF} : group_bitmap;
  }

private:
  struct context_type {
    void push_back(std::byte const s) {
      if (!is_status_byte(s) || is_system_real_time_message(s)) {
        // Not a status byte or a system real-time message, so always emit. Don't update running status.
        output.push_back(s);
      } else {
        if (!running_status_ || status != s) {
          output.push_back(s);
        }
        status = s;
      }
    }

    /// \returns true if the message should be filtered; false if the message should be allowed.
    [[nodiscard]] constexpr bool filter_message(unsigned const group) const {
      assert(group < 16U);
      return (only_groups & (1U << group)) == 0U;
    }
    template <typename T> [[nodiscard]] constexpr bool filter_message(T const &in) const {
      return (only_groups & (1U << in.group())) == 0U;
    }

    bool running_status_ = false;
    std::byte status = std::byte{0xFF};  // TODO: 0xFF is a valid status value.
    std::uint16_t only_groups = 0;       ///< A bitmap indicating which groups should be included in the output
    fifo<std::byte, 8> output;
  };

  struct to_bytestream_config {
    class system {
    public:
      static void midi_time_code(context_type *const ctxt, ump::system::midi_time_code const &in) {
        static_assert(bytestream_message_size<status::timing_code>() == 2);
        system::push(ctxt, in.group(), status::timing_code, std::byte{in.time_code()});
      }
      static void song_position_pointer(context_type *const ctxt, ump::system::song_position_pointer const &in) {
        static_assert(bytestream_message_size<status::spp>() == 3);
        system::push(ctxt, in.group(), status::spp, std::byte{in.position_lsb()}, std::byte{in.position_msb()});
      }
      static void song_select(context_type *const ctxt, ump::system::song_select const &in) {
        static_assert(bytestream_message_size<status::song_select>() == 2);
        system::push(ctxt, in.group(), status::song_select, std::byte{in.song()});
      }
      static void tune_request(context_type *const ctxt, ump::system::tune_request const &in) {
        static_assert(bytestream_message_size<status::tune_request>() == 1);
        system::push(ctxt, in.group(), status::tune_request);
      }
      static void timing_clock(context_type *const ctxt, ump::system::timing_clock const &in) {
        static_assert(bytestream_message_size<status::timing_clock>() == 1);
        system::push(ctxt, in.group(), status::timing_clock);
      }
      static void seq_start(context_type *const ctxt, ump::system::sequence_start const &in) {
        static_assert(bytestream_message_size<status::sequence_start>() == 1);
        system::push(ctxt, in.group(), status::sequence_start);
      }
      static void seq_continue(context_type *const ctxt, ump::system::sequence_continue const &in) {
        static_assert(bytestream_message_size<status::sequence_continue>() == 1);
        system::push(ctxt, in.group(), status::sequence_continue);
      }
      static void seq_stop(context_type *const ctxt, ump::system::sequence_stop const &in) {
        static_assert(bytestream_message_size<status::sequence_stop>() == 1);
        system::push(ctxt, in.group(), status::sequence_stop);
      }
      static void active_sensing(context_type *const ctxt, ump::system::active_sensing const &in) {
        static_assert(bytestream_message_size<status::active_sensing>() == 1);
        system::push(ctxt, in.group(), status::active_sensing);
      }
      static void reset(context_type *const ctxt, ump::system::reset const &in) {
        static_assert(bytestream_message_size<status::systemreset>() == 1);
        system::push(ctxt, in.group(), status::systemreset);
      }

    private:
      static void push(context_type *const ctxt, unsigned const group, status const s) {
        if (!ctxt->filter_message(group)) {
          ctxt->push_back(std::byte{to_underlying(s)});
        }
      }
      static void push(context_type *const ctxt, unsigned const group, status const s, std::byte const b1) {
        if (!ctxt->filter_message(group)) {
          ctxt->push_back(std::byte{to_underlying(s)});
          ctxt->push_back(b1);
        }
      }
      static void push(context_type *const ctxt, unsigned const group, status const s, std::byte const b1,
                       std::byte const b2) {
        if (!ctxt->filter_message(group)) {
          ctxt->push_back(std::byte{to_underlying(s)});
          ctxt->push_back(b1);
          ctxt->push_back(b2);
        }
      }
    };
    class m1cvm {
    public:
      static void note_off(context_type *const ctxt, ump::m1cvm::note_off const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(bytestream_message_size<status::note_off>() == 3);
        ctxt->push_back(std::byte{to_underlying(status::note_off)} | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.note()});
        ctxt->push_back(std::byte{in.velocity()});
      }
      static void note_on(context_type *const ctxt, ump::m1cvm::note_on const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(bytestream_message_size<status::note_on>() == 3);
        ctxt->push_back(std::byte{to_underlying(status::note_on)} | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.note()});
        ctxt->push_back(std::byte{in.velocity()});
      }
      static void poly_pressure(context_type *const ctxt, ump::m1cvm::poly_pressure const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(bytestream_message_size<status::poly_pressure>() == 3);
        ctxt->push_back(std::byte{to_underlying(status::poly_pressure)} | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.note()});
        ctxt->push_back(std::byte{in.pressure()});
      }
      static void control_change(context_type *const ctxt, ump::m1cvm::control_change const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(bytestream_message_size<status::cc>() == 3);
        ctxt->push_back(std::byte{to_underlying(status::cc)} | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.controller()});
        ctxt->push_back(std::byte{in.value()});
      }
      static void program_change(context_type *const ctxt, ump::m1cvm::program_change const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(bytestream_message_size<status::program_change>() == 2);
        ctxt->push_back(std::byte{to_underlying(status::program_change)} | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.program()});
      }
      static void channel_pressure(context_type *const ctxt, ump::m1cvm::channel_pressure const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(bytestream_message_size<status::channel_pressure>() == 2);
        ctxt->push_back(std::byte{to_underlying(status::channel_pressure)} | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.data()});
      }
      static void pitch_bend(context_type *const ctxt, ump::m1cvm::pitch_bend const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(bytestream_message_size<status::pitch_bend>() == 3);
        ctxt->push_back(std::byte{to_underlying(status::pitch_bend)} | std::byte{in.channel()});
        ctxt->push_back(std::byte{in.lsb_data()});
        ctxt->push_back(std::byte{in.msb_data()});
      }
    };
    class data64 {
    public:
      static void sysex7_in_1(context_type *const ctxt, ump::data64::sysex7_in_1 const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        if (in.number_of_bytes() > 0) {
          ctxt->push_back(sysex_start);
          data64::write_sysex_bytes(ctxt, in);
          ctxt->push_back(sysex_stop);
        }
      }
      static void sysex7_start(context_type *const ctxt, ump::data64::sysex7_start const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        ctxt->push_back(sysex_start);
        data64::write_sysex_bytes(ctxt, in);
      }
      static void sysex7_continue(context_type *const ctxt, ump::data64::sysex7_continue const &in) {
        // Skip this message if we're filtering the associated group or if we didn't see a preceeding sysex
        // start message.
        if (ctxt->filter_message(in) || ctxt->status != sysex_start) {
          return;
        }
        data64::write_sysex_bytes(ctxt, in);
      }
      static void sysex7_end(context_type *const ctxt, ump::data64::sysex7_end const &in) {
        // Skip this message if we're filtering the associated group or if we didn't see a preceeding sysex
        // start message.
        if (ctxt->filter_message(in) || ctxt->status != sysex_start) {
          return;
        }
        data64::write_sysex_bytes(ctxt, in);
        ctxt->push_back(sysex_stop);
      }

    private:
      static constexpr auto sysex_start = std::byte{to_underlying(status::sysex_start)};
      static constexpr auto sysex_stop = std::byte{to_underlying(status::sysex_stop)};

      template <typename T> static void write_sysex_bytes(context_type *const ctxt, T const &in) {
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
    context_type *context = nullptr;
    [[no_unique_address]] dispatcher_backend::utility_null<decltype(context)> utility{};
    [[no_unique_address]] class system system{};
    [[no_unique_address]] class m1cvm m1cvm{};
    [[no_unique_address]] class data64 data64{};
    [[no_unique_address]] dispatcher_backend::m2cvm_null<decltype(context)> m2cvm{};
    [[no_unique_address]] dispatcher_backend::data128_null<decltype(context)> data128{};
    [[no_unique_address]] dispatcher_backend::ump_stream_null<decltype(context)> ump_stream{};
    [[no_unique_address]] dispatcher_backend::flex_data_null<decltype(context)> flex{};
  };
  context_type context_;
  ump_dispatcher<to_bytestream_config> p_{to_bytestream_config{.context = &context_}};
};

}  // end namespace midi2

#endif  // MIDI2_UMPTOBYTESTREAM_HPP
