//===-- UMP to Bytestream -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMPTOBYTESTREAM_HPP
#define MIDI2_UMPTOBYTESTREAM_HPP

#include <cstddef>
#include <cstdint>
#include <optional>

#include "midi2/adt/cache.hpp"
#include "midi2/adt/fifo.hpp"
#include "midi2/ump_dispatcher.hpp"
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

template <> struct bytestream_message_size<midi2::status::timing_clock> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::sequence_start> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::sequence_continue> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::sequence_stop> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::activesense> : std::integral_constant<unsigned, 1> {};
template <> struct bytestream_message_size<midi2::status::systemreset> : std::integral_constant<unsigned, 1> {};

class ump_to_bytestream {
public:
  ump_to_bytestream() = default;

  [[nodiscard]] constexpr bool available() const { return !context_.output.empty(); }
  [[nodiscard]] std::byte read() { return context_.output.pop_front(); }

  void UMPStreamParse(std::uint32_t const ump) { p_.processUMP(ump); }

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
    template <typename T> constexpr bool filter_message(T const &in) const {
      return (only_groups & (1U << get<0>(in.w).group)) == 0U;
    }

    bool running_status_ = false;
    std::byte status = std::byte{0xFF};  // TODO: 0xFF is a valid status value.
    std::uint16_t only_groups = 0;       ///< A bitmap indicating which groups should be included in the output
    fifo<std::byte, 8> output;
  };

  struct to_bytestream_config {
    struct system {
      static void midi_time_code(context_type *const ctxt, types::system::midi_time_code const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(std::tuple_size_v<decltype(types::system::midi_time_code::w)> == 1);
        static_assert(bytestream_message_size<status::timing_code>() == 2);
        ctxt->push_back(std::byte{to_underlying(status::timing_code)});
        ctxt->push_back(std::byte{get<0>(in.w).time_code.value()});
      }
      static void song_position_pointer(context_type *const ctxt, types::system::song_position_pointer const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(std::tuple_size_v<decltype(types::system::song_position_pointer::w)> == 1);
        static_assert(bytestream_message_size<status::spp>() == 3);
        auto const &w0 = get<0>(in.w);
        ctxt->push_back(std::byte{to_underlying(status::spp)});
        ctxt->push_back(std::byte{w0.position_lsb.value()});
        ctxt->push_back(std::byte{w0.position_msb.value()});
      }
      static void song_select(context_type *const ctxt, types::system::song_select const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(std::tuple_size_v<decltype(types::system::song_select::w)> == 1);
        static_assert(bytestream_message_size<status::song_select>() == 2);
        auto const &w0 = get<0>(in.w);
        ctxt->push_back(std::byte{to_underlying(status::song_select)});
        ctxt->push_back(std::byte{w0.song.value()});
      }
      static void tune_request(context_type *const ctxt, types::system::tune_request const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(std::tuple_size_v<decltype(types::system::tune_request::w)> == 1);
        static_assert(bytestream_message_size<status::tune_request>() == 1);
        ctxt->push_back(std::byte{to_underlying(status::tune_request)});
      }
      static void timing_clock(context_type *const ctxt, types::system::timing_clock const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(std::tuple_size_v<decltype(types::system::timing_clock::w)> == 1);
        static_assert(bytestream_message_size<status::timing_clock>() == 1);
        ctxt->push_back(std::byte{to_underlying(status::timing_clock)});
      }
      static void seq_start(context_type *const ctxt, types::system::seq_start const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        // TODO
      }
      static void seq_continue(context_type *const ctxt, types::system::seq_continue const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        // TODO
      }
      static void seq_stop(context_type *const ctxt, types::system::seq_stop const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        // TODO
      }
      static void active_sensing(context_type *const ctxt, types::system::active_sensing const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        // TODO
      }
      static void reset(context_type *const ctxt, types::system::reset const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        // TODO
      }
    };
    struct m1cvm {
      static void note_off(context_type *const ctxt, types::m1cvm::note_off const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(std::tuple_size_v<decltype(types::m1cvm::note_off::w)> == 1);
        static_assert(bytestream_message_size<status::note_off>() == 3);
        auto const &w0 = get<0>(in.w);
        ctxt->push_back(std::byte{to_underlying(status::note_off)} | std::byte{w0.channel.value()});
        ctxt->push_back(std::byte{w0.note.value()});
        ctxt->push_back(std::byte{w0.velocity.value()});
      }
      static void note_on(context_type *const ctxt, types::m1cvm::note_on const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(std::tuple_size_v<decltype(types::m1cvm::note_on::w)> == 1);
        static_assert(bytestream_message_size<status::note_on>() == 3);
        auto const &w0 = get<0>(in.w);
        ctxt->push_back(std::byte{to_underlying(status::note_on)} | std::byte{w0.channel.value()});
        ctxt->push_back(std::byte{w0.note.value()});
        ctxt->push_back(std::byte{w0.velocity.value()});
      }
      static void poly_pressure(context_type *const ctxt, types::m1cvm::poly_pressure const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        // TODO: implement!
      }
      static void control_change(context_type *const ctxt, types::m1cvm::control_change const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(std::tuple_size_v<decltype(types::m1cvm::control_change::w)> == 1);
        static_assert(bytestream_message_size<status::cc>() == 3);
        auto const &w0 = get<0>(in.w);
        ctxt->push_back(std::byte{to_underlying(status::cc)} | std::byte{w0.channel.value()});
        ctxt->push_back(std::byte{w0.controller.value()});
        ctxt->push_back(std::byte{w0.value.value()});
      }
      static void program_change(context_type *const ctxt, types::m1cvm::program_change const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        static_assert(std::tuple_size_v<decltype(types::m1cvm::program_change::w)> == 1);
        auto const &w0 = get<0>(in.w);
        ctxt->push_back((std::byte{w0.status.value()} << 4) | std::byte{w0.channel.value()});
        ctxt->push_back(std::byte{w0.program.value()});
      }
      static void channel_pressure(context_type *const ctxt, types::m1cvm::channel_pressure const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        // TODO: implement!
      }
      static void pitch_bend(context_type *const ctxt, types::m1cvm::pitch_bend const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        // TODO: implement!
      }
    };
    struct data64 {
    public:
      static void sysex7_in_1(context_type *const ctxt, types::data64::sysex7 const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        if (get<0>(in.w).number_of_bytes > 0) {
          ctxt->push_back(sysex_start);
          data64::write_sysex_bytes(ctxt, in);
          ctxt->push_back(sysex_stop);
        }
      }
      static void sysex7_start(context_type *const ctxt, types::data64::sysex7 const &in) {
        if (ctxt->filter_message(in)) {
          return;
        }
        // TODO: check for messages interleaved on different channels?
        ctxt->push_back(sysex_start);
        data64::write_sysex_bytes(ctxt, in);
      }
      static void sysex7_continue(context_type *const ctxt, types::data64::sysex7 const &in) {
        // Skip this message if we're filtering the associated group or if we didn't see a preceeding sysex
        // start message.
        if (ctxt->filter_message(in) || ctxt->status != sysex_start) {
          return;
        }
        // TODO: check for messages interleaved on different channels?
        data64::write_sysex_bytes(ctxt, in);
      }
      static void sysex7_end(context_type *const ctxt, types::data64::sysex7 const &in) {
        // Skip this message if we're filtering the associated group or if we didn't see a preceeding sysex
        // start message.
        if (ctxt->filter_message(in) || ctxt->status != sysex_start) {
          return;
        }
        // TODO: check for messages interleaves on different channels.
        data64::write_sysex_bytes(ctxt, in);
        ctxt->push_back(sysex_stop);
      }

    private:
      static constexpr auto sysex_start = std::byte{to_underlying(status::sysex_start)};
      static constexpr auto sysex_stop = std::byte{to_underlying(status::sysex_stop)};

      template <typename T> static void write_sysex_bytes(context_type *const ctxt, T const &in) {
        static_assert(std::tuple_size_v<decltype(T::w)> == 2);
        auto const &w0 = get<0>(in.w);
        auto const &w1 = get<1>(in.w);
        auto const number_of_bytes = w0.number_of_bytes.value();
        if (number_of_bytes > 0) {
          ctxt->push_back(static_cast<std::byte>(w0.data0.value()));
        }
        if (number_of_bytes > 1) {
          ctxt->push_back(static_cast<std::byte>(w0.data1.value()));
        }
        if (number_of_bytes > 2) {
          ctxt->push_back(static_cast<std::byte>(w1.data2.value()));
        }
        if (number_of_bytes > 3) {
          ctxt->push_back(static_cast<std::byte>(w1.data3.value()));
        }
        if (number_of_bytes > 4) {
          ctxt->push_back(static_cast<std::byte>(w1.data4.value()));
        }
        if (number_of_bytes > 5) {
          ctxt->push_back(static_cast<std::byte>(w1.data5.value()));
        }
      }
    };
    context_type *context = nullptr;
    [[no_unique_address]] utility_null<decltype(context)> utility{};
    [[no_unique_address]] struct system system{};
    [[no_unique_address]] struct m1cvm m1cvm{};
    [[no_unique_address]] struct data64 data64{};
    [[no_unique_address]] m2cvm_null<decltype(context)> m2cvm{};
    [[no_unique_address]] data128_null<decltype(context)> data128{};
    [[no_unique_address]] ump_stream_null<decltype(context)> ump_stream{};
    [[no_unique_address]] flex_data_null<decltype(context)> flex{};
  };
  context_type context_;
  ump_dispatcher<to_bytestream_config> p_{to_bytestream_config{.context = &context_}};
};

}  // end namespace midi2

#endif  // MIDI2_UMPTOBYTESTREAM_HPP
