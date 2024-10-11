//===-- UMP to MIDI 1.0 UMP ---------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMPTOMIDI1PROTOCOL_HPP
#define MIDI2_UMPTOMIDI1PROTOCOL_HPP

#include <algorithm>
#include <cstdint>
#include <tuple>

#include "midi2/fifo.hpp"
#include "midi2/ump_dispatcher.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

class umpToMIDI1Protocol {
public:
  [[nodiscard]] constexpr bool available() const { return !context_.output.empty(); }
  [[nodiscard]] std::uint32_t readUMP() { return context_.output.pop_front(); }

  void UMPStreamParse(std::uint32_t const ump) { p_.processUMP(ump); }

private:
  struct context_type {
    template <typename T, unsigned Index = 0> requires(std::tuple_size_v<T> >= 0) void push(T const &value) {
      if constexpr (Index >= std::tuple_size_v<T>) {
        return;
      } else {
        output.push_back(std::bit_cast<std::uint32_t>(std::get<Index>(value)));
        push<T, Index + 1>(value);
      }
    }

    fifo<std::uint32_t, 4> output;
  };

  struct to_midi1_config {
    // system messages go straight through.
    struct system {
      void midi_time_code(context_type *const ctxt, types::system::midi_time_code const &in) const { ctxt->push(in.w); }
      void song_position_pointer(context_type *const ctxt, types::system::song_position_pointer const &in) const {
        ctxt->push(in.w);
      }
      void song_select(context_type *const ctxt, types::system::song_select const &in) const { ctxt->push(in.w); }
      void tune_request(context_type *const ctxt, types::system::tune_request const &in) const { ctxt->push(in.w); }
      void timing_clock(context_type *const ctxt, types::system::timing_clock const &in) const { ctxt->push(in.w); }
      void seq_start(context_type *const ctxt, types::system::seq_start const &in) const { ctxt->push(in.w); }
      void seq_continue(context_type *const ctxt, types::system::seq_continue const &in) const { ctxt->push(in.w); }
      void seq_stop(context_type *const ctxt, types::system::seq_stop const &in) const { ctxt->push(in.w); }
      void active_sensing(context_type *const ctxt, types::system::active_sensing const &in) const { ctxt->push(in.w); }
      void reset(context_type *const ctxt, types::system::reset const &in) const { ctxt->push(in.w); }
    };
    // m1cvm messages go straight through.
    struct m1cvm {
      void note_off(context_type *const ctxt, types::m1cvm::note_off const &in) const { ctxt->push(in.w); }
      void note_on(context_type *const ctxt, types::m1cvm::note_on const &in) const { ctxt->push(in.w); }
      void poly_pressure(context_type *const ctxt, types::m1cvm::poly_pressure const &in) const { ctxt->push(in.w); }
      void control_change(context_type *const ctxt, types::m1cvm::control_change const &in) const { ctxt->push(in.w); }
      void program_change(context_type *const ctxt, types::m1cvm::program_change const &in) const { ctxt->push(in.w); }
      void channel_pressure(context_type *const ctxt, types::m1cvm::channel_pressure const &in) const { ctxt->push(in.w); }
      void pitch_bend(context_type *const ctxt, types::m1cvm::pitch_bend const &in) const { ctxt->push(in.w); }
    };
    // data64 messages go straight through.
    struct data64 {
      void sysex7_in_1(context_type *const ctxt, types::data64::sysex7 const &in) const { ctxt->push(in.w); }
      void sysex7_start(context_type *const ctxt, types::data64::sysex7 const &in) const { ctxt->push(in.w); }
      void sysex7_continue(context_type *const ctxt, types::data64::sysex7 const &in) const { ctxt->push(in.w); }
      void sysex7_end(context_type *const ctxt, types::data64::sysex7 const &in) const { ctxt->push(in.w); }
    };
    // m2cvm messages are translated to m1cvm messages.
    struct m2cvm {
      void note_off(context_type *ctxt, types::m2cvm::note_off const &in) const;
      void note_on(context_type *ctxt, types::m2cvm::note_on const &in) const;
      void poly_pressure(context_type *ctxt, types::m2cvm::poly_pressure const &in) const;
      void program_change(context_type *ctxt, types::m2cvm::program_change const &in) const;
      void channel_pressure(context_type *ctxt, types::m2cvm::channel_pressure const &) const;

      void rpn_per_note_controller(context_type *ctxt, midi2::types::m2cvm::rpn_per_note_controller const &) const;
      void nrpn_per_note_controller(context_type *ctxt, midi2::types::m2cvm::nrpn_per_note_controller const &) const;
      void rpn_controller(context_type *ctxt, types::m2cvm::rpn_controller const &) const;
      void nrpn_controller(context_type *ctxt, types::m2cvm::nrpn_controller const &) const;

      void per_note_management(context_type *ctxt, types::m2cvm::per_note_management const &) const;
      void control_change(context_type *ctxt, types::m2cvm::control_change const &) const;
      void controller_message(context_type *ctxt, types::m2cvm::controller_message const &) const;
      void pitch_bend(context_type *ctxt, types::m2cvm::pitch_bend const &) const;
      void per_note_pitch_bend(context_type *ctxt, types::m2cvm::per_note_pitch_bend const &) const;
    };
    context_type *context = nullptr;
    [[no_unique_address]] utility_null<decltype(context)> utility{};
    [[no_unique_address]] struct system system{};
    [[no_unique_address]] struct m1cvm m1cvm{};
    [[no_unique_address]] struct data64 data64{};
    [[no_unique_address]] struct m2cvm m2cvm{};
    [[no_unique_address]] data128_null<decltype(context)> data128{};
    [[no_unique_address]] ump_stream_null<decltype(context)> ump_stream{};
    [[no_unique_address]] flex_data_null<decltype(context)> flex{};
  };
  context_type context_;
  ump_dispatcher<to_midi1_config> p_{to_midi1_config{&context_}};
};

}  // end namespace midi2

#endif  // MIDI2_UMPTOMIDI1PROTOCOL_HPP
