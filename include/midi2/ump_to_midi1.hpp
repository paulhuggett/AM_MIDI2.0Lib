//===-- UMP to MIDI 1.0 UMP ---------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMPTOMIDI1_HPP
#define MIDI2_UMPTOMIDI1_HPP

#include <cstdint>
#include <tuple>

#include "midi2/adt/cache.hpp"
#include "midi2/adt/fifo.hpp"
#include "midi2/ump_dispatcher.hpp"
#include "midi2/ump_types.hpp"

namespace midi2 {

class ump_to_midi1 {
public:
  [[nodiscard]] constexpr bool available() const { return !context_.output.empty(); }
  [[nodiscard]] std::uint32_t read() { return context_.output.pop_front(); }

  void UMPStreamParse(std::uint32_t const ump) { p_.processUMP(ump); }

private:
  struct context_type {
    template <typename T, unsigned Index = 0> requires(std::tuple_size_v<T> >= 0) void push(T const &value) {
      if constexpr (Index >= std::tuple_size_v<T>) {
        return;
      } else {
        auto const value32 = std::bit_cast<std::uint32_t>(std::get<Index>(value));
        output.push_back(value32);
        push<T, Index + 1>(value);
      }
    }

    struct pn_cache_key {
      bool operator==(pn_cache_key const &) const noexcept = default;

      std::uint8_t group : 4 = 0;
      std::uint8_t channel : 4 = 0;
      bool is_rpn = false;
    };

    // value is 14 bit MIDI 1 controller number (bank/index).
    cache<pn_cache_key, std::pair<std::uint8_t, std::uint8_t>, 16> pn_cache;
    fifo<std::uint32_t, 4> output;
  };
  friend struct std::hash<midi2::ump_to_midi1::context_type::pn_cache_key>;

  struct to_midi1_config {
    // system messages go straight through.
    struct system {
      static void midi_time_code(context_type *const ctxt, types::system::midi_time_code const &in) {
        ctxt->push(in.w);
      }
      static void song_position_pointer(context_type *const ctxt, types::system::song_position_pointer const &in) {
        ctxt->push(in.w);
      }
      static void song_select(context_type *const ctxt, types::system::song_select const &in) { ctxt->push(in.w); }
      static void tune_request(context_type *const ctxt, types::system::tune_request const &in) { ctxt->push(in.w); }
      static void timing_clock(context_type *const ctxt, types::system::timing_clock const &in) { ctxt->push(in.w); }
      static void seq_start(context_type *const ctxt, types::system::sequence_start const &in) { ctxt->push(in.w); }
      static void seq_continue(context_type *const ctxt, types::system::sequence_continue const &in) { ctxt->push(in.w); }
      static void seq_stop(context_type *const ctxt, types::system::sequence_stop const &in) { ctxt->push(in.w); }
      static void active_sensing(context_type *const ctxt, types::system::active_sensing const &in) {
        ctxt->push(in.w);
      }
      static void reset(context_type *const ctxt, types::system::reset const &in) { ctxt->push(in.w); }
    };
    // m1cvm messages go straight through.
    struct m1cvm {
      static void note_off(context_type *const ctxt, types::m1cvm::note_off const &in) { ctxt->push(in.w); }
      static void note_on(context_type *const ctxt, types::m1cvm::note_on const &in) { ctxt->push(in.w); }
      static void poly_pressure(context_type *const ctxt, types::m1cvm::poly_pressure const &in) { ctxt->push(in.w); }
      static void control_change(context_type *const ctxt, types::m1cvm::control_change const &in) { ctxt->push(in.w); }
      static void program_change(context_type *const ctxt, types::m1cvm::program_change const &in) { ctxt->push(in.w); }
      static void channel_pressure(context_type *const ctxt, types::m1cvm::channel_pressure const &in) {
        ctxt->push(in.w);
      }
      static void pitch_bend(context_type *const ctxt, types::m1cvm::pitch_bend const &in) { ctxt->push(in.w); }
    };
    // data64 messages go straight through.
    struct data64 {
      static void sysex7_in_1(context_type *const ctxt, types::data64::sysex7 const &in) { ctxt->push(in.w); }
      static void sysex7_start(context_type *const ctxt, types::data64::sysex7 const &in) { ctxt->push(in.w); }
      static void sysex7_continue(context_type *const ctxt, types::data64::sysex7 const &in) { ctxt->push(in.w); }
      static void sysex7_end(context_type *const ctxt, types::data64::sysex7 const &in) { ctxt->push(in.w); }
    };
    // m2cvm messages are translated to m1cvm messages.
    class m2cvm {
    public:
      static void note_off(context_type *ctxt, types::m2cvm::note_off const &in);
      static void note_on(context_type *ctxt, types::m2cvm::note_on const &in);
      static void poly_pressure(context_type *ctxt, types::m2cvm::poly_pressure const &in);
      static void program_change(context_type *ctxt, types::m2cvm::program_change const &in);
      static void channel_pressure(context_type *ctxt, types::m2cvm::channel_pressure const &);

      static void rpn_controller(context_type *ctxt, types::m2cvm::rpn_controller const &in);
      static void nrpn_controller(context_type *ctxt, types::m2cvm::nrpn_controller const &in);

      static constexpr void rpn_per_note_controller(context_type const *,
                                                    midi2::types::m2cvm::rpn_per_note_controller const &) {
        // do nothing: cannot be translated to MIDI 1
      }
      static constexpr void nrpn_per_note_controller(context_type const *,
                                                     midi2::types::m2cvm::nrpn_per_note_controller const &) {
        // do nothing: cannot be translated to MIDI 1
      }
      static constexpr void rpn_relative_controller(context_type const *,
                                                    types::m2cvm::rpn_relative_controller const &) {
        // do nothing: cannot be translated to MIDI 1
      }
      static constexpr void nrpn_relative_controller(context_type const *,
                                                     types::m2cvm::nrpn_relative_controller const &) {
        // do nothing: cannot be translated to MIDI 1
      }

      static constexpr void per_note_management(context_type const *, types::m2cvm::per_note_management const &) {
        // do nothing: cannot be translated to MIDI 1
      }
      static void control_change(context_type *ctxt, types::m2cvm::control_change const &);
      static void pitch_bend(context_type *ctxt, types::m2cvm::pitch_bend const &);

      static constexpr void per_note_pitch_bend(context_type const *, types::m2cvm::per_note_pitch_bend const &) {
        // do nothing: cannot be translated to MIDI 1
      }

    private:
      static void pn_message(context_type *ctxt, context_type::pn_cache_key const &key,
                             std::pair<std::uint8_t, std::uint8_t> const &controller_number, std::uint32_t const value);
    };
    context_type *context = nullptr;
    [[no_unique_address]] utility_null<decltype(context)> utility{};
    [[no_unique_address]] struct system system{};
    [[no_unique_address]] struct m1cvm m1cvm{};
    [[no_unique_address]] struct data64 data64{};
    [[no_unique_address]] class m2cvm m2cvm{};
    [[no_unique_address]] data128_null<decltype(context)> data128{};
    [[no_unique_address]] ump_stream_null<decltype(context)> ump_stream{};
    [[no_unique_address]] flex_data_null<decltype(context)> flex{};
  };
  context_type context_;
  ump_dispatcher<to_midi1_config> p_{to_midi1_config{&context_}};
};

}  // end namespace midi2

template <> struct std::hash<midi2::ump_to_midi1::context_type::pn_cache_key> {
  std::size_t operator()(midi2::ump_to_midi1::context_type::pn_cache_key const &key) const noexcept {
    return std::hash<unsigned>{}(static_cast<unsigned>(key.group << 5) | static_cast<unsigned>(key.channel << 1) |
                                 static_cast<unsigned>(key.is_rpn));
  }
};

#endif  // MIDI2_UMPTOMIDI1_HPP
