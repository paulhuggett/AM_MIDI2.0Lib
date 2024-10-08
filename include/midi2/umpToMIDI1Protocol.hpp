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

#include "midi2/fifo.hpp"
#include "midi2/umpProcessor.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

class umpToMIDI1Protocol {
public:
  [[nodiscard]] constexpr bool availableUMP() const { return !context_.output.empty(); }
  [[nodiscard]] std::uint32_t readUMP() { return context_.output.pop_front(); }

  void UMPStreamParse(std::uint32_t const ump) { p_.processUMP(ump); }

private:
  struct context_type {
    void push(std::uint32_t value) { output.push_back(value); }
    template <typename T> void push1(T const &t) { output.push_back(std::bit_cast<std::uint32_t>(t.w0)); }
    template <typename T> void push2(T const &t) {
      output.push_back(std::bit_cast<std::uint32_t>(t.w0));
      output.push_back(std::bit_cast<std::uint32_t>(t.w1));
    }
    template <typename T> void push4(T const &t) {
      output.push_back(std::bit_cast<std::uint32_t>(t.w0));
      output.push_back(std::bit_cast<std::uint32_t>(t.w1));
      output.push_back(std::bit_cast<std::uint32_t>(t.w2));
      output.push_back(std::bit_cast<std::uint32_t>(t.w3));
    }
    fifo<std::uint32_t, 4> output;
  };

  struct to_midi1_config {
    struct utility {
      void noop(context_type *const ctxt) const { ctxt->push(0); }
      void jr_clock(context_type *const ctxt, types::utility::jr_clock const &in) const { ctxt->push1(in); }
      void jr_timestamp(context_type *const ctxt, types::utility::jr_timestamp const &in) const { ctxt->push1(in); }
      void delta_clockstamp_tpqn(context_type *const ctxt, types::utility::delta_clockstamp_tpqn const &in) const {
        ctxt->push1(in);
      }
      void delta_clockstamp(context_type *const ctxt, types::utility::delta_clockstamp const &in) const {
        ctxt->push1(in);
      }

      void unknown(context_type *const ctxt, std::span<std::uint32_t> const &in) const {
        for (auto const value : in) {
          ctxt->push(value);
        }
      }
    };
    struct system {
      void midi_time_code(context_type *const ctxt, types::system::midi_time_code const &in) const { ctxt->push1(in); }
      void song_position_pointer(context_type *const ctxt, types::system::song_position_pointer const &in) const {
        ctxt->push1(in);
      }
      void song_select(context_type *const ctxt, types::system::song_select const &in) const { ctxt->push1(in); }
      void tune_request(context_type *const ctxt, types::system::tune_request const &in) const { ctxt->push1(in); }
      void timing_clock(context_type *const ctxt, types::system::timing_clock const &in) const { ctxt->push1(in); }
      void seq_start(context_type *const ctxt, types::system::seq_start const &in) const { ctxt->push1(in); }
      void seq_continue(context_type *const ctxt, types::system::seq_continue const &in) const { ctxt->push1(in); }
      void seq_stop(context_type *const ctxt, types::system::seq_stop const &in) const { ctxt->push1(in); }
      void active_sensing(context_type *const ctxt, types::system::active_sensing const &in) const { ctxt->push1(in); }
      void reset(context_type *const ctxt, types::system::reset const &in) const { ctxt->push1(in); }
    };
    struct m1cvm {
      void note_off(context_type *const ctxt, types::m1cvm::note_off const &in) const { ctxt->push1(in); }
      void note_on(context_type *const ctxt, types::m1cvm::note_on const &in) const { ctxt->push1(in); }
      void poly_pressure(context_type *const ctxt, types::m1cvm::poly_pressure const &in) const { ctxt->push1(in); }
      void control_change(context_type *const ctxt, types::m1cvm::control_change const &in) const { ctxt->push1(in); }
      void program_change(context_type *const ctxt, types::m1cvm::m1cvm const &in) const { ctxt->push1(in); }
      void channel_pressure(context_type *const ctxt, types::m1cvm::channel_pressure const &in) const {
        ctxt->push1(in);
      }
      void pitch_bend(context_type *const ctxt, types::m1cvm::m1cvm const &in) const { ctxt->push1(in); }
    };
    struct data64 {
      void sysex7_in_1(context_type *const ctxt, types::data64::sysex7 const &in) const { ctxt->push2(in); }
      void sysex7_start(context_type *const ctxt, types::data64::sysex7 const &in) const { ctxt->push2(in); }
      void sysex7_continue(context_type *const ctxt, types::data64::sysex7 const &in) const { ctxt->push2(in); }
      void sysex7_end(context_type *const ctxt, types::data64::sysex7 const &in) const { ctxt->push2(in); }
    };
    struct m2cvm {
      void note_off(context_type *ctxt, types::m2cvm::note_off const &in) const;
      void note_on(context_type *ctxt, types::m2cvm::note_on const &in) const;
      void poly_pressure(context_type *ctxt, types::m2cvm::poly_pressure const &in) const;
      void program_change(context_type *ctxt, types::m2cvm::program_change const &in) const;
      void channel_pressure(context_type *ctxt, types::m2cvm::channel_pressure const &) const;
      void rpn_controller(context_type *ctxt, types::m2cvm::per_note_controller const &) const;
      void nrpn_controller(context_type *ctxt, types::m2cvm::per_note_controller const &) const;
      void per_note_management(context_type *ctxt, types::m2cvm::per_note_management const &) const;
      void control_change(context_type *ctxt, types::m2cvm::control_change const &) const;
      void controller_message(context_type *ctxt, types::m2cvm::controller_message const &) const;
      void pitch_bend(context_type *ctxt, types::m2cvm::pitch_bend const &) const;
      void per_note_pitch_bend(context_type *ctxt, types::m2cvm::per_note_pitch_bend const &) const;
    };
    struct data128 {
      void sysex8_in_1(context_type *const ctxt, types::data128::sysex8 const &in) const { ctxt->push4(in); }
      void sysex8_start(context_type *const ctxt, types::data128::sysex8 const &in) const { ctxt->push4(in); }
      void sysex8_continue(context_type *const ctxt, types::data128::sysex8 const &in) const { ctxt->push4(in); }
      void sysex8_end(context_type *const ctxt, types::data128::sysex8 const &in) const { ctxt->push4(in); }

      void mds_header(context_type *const ctxt, types::data128::mds_header const &in) const { ctxt->push4(in); }
      void mds_payload(context_type *const ctxt, types::data128::mds_payload const &in) const { ctxt->push4(in); }
    };
    struct ump_stream {
      void endpoint_discovery(context_type *const ctxt, types::ump_stream::endpoint_discovery const &in) const {
        ctxt->push4(in);
      }
      void endpoint_info_notification(context_type *const ctxt,
                                      types::ump_stream::endpoint_info_notification const &in) const {
        ctxt->push4(in);
      }
      void device_identity_notification(context_type *const ctxt,
                                        types::ump_stream::device_identity_notification const &in) const {
        ctxt->push4(in);
      }
      void endpoint_name_notification(context_type *const ctxt,
                                      types::ump_stream::endpoint_name_notification const &in) const {
        ctxt->push4(in);
      }
      void product_instance_id_notification(context_type *const ctxt,
                                            types::ump_stream::product_instance_id_notification const &in) const {
        ctxt->push4(in);
      }
      void jr_configuration_request(context_type *const ctxt,
                                    types::ump_stream::jr_configuration_request const &in) const {
        ctxt->push4(in);
      }
      void jr_configuration_notification(context_type *const ctxt,
                                         types::ump_stream::jr_configuration_notification const &in) const {
        ctxt->push4(in);
      }

      void function_block_discovery(context_type *const ctxt,
                                    types::ump_stream::function_block_discovery const &in) const {
        ctxt->push4(in);
      }
      void function_block_info_notification(context_type *const ctxt,
                                            types::ump_stream::function_block_info_notification const &in) const {
        ctxt->push4(in);
      }
      void function_block_name_notification(context_type *const ctxt,
                                            types::ump_stream::function_block_name_notification const &in) const {
        ctxt->push4(in);
      }

      void start_of_clip(context_type *const ctxt, types::ump_stream::start_of_clip const &in) const {
        ctxt->push4(in);
      }
      void end_of_clip(context_type *const ctxt, types::ump_stream::end_of_clip const &in) const { ctxt->push4(in); }
    };
    struct flex_data {
      void set_tempo(context_type *const ctxt, types::flex_data::set_tempo const &in) const { ctxt->push4(in); }
      void set_time_signature(context_type *const ctxt, types::flex_data::set_time_signature const &in) const {
        ctxt->push4(in);
      }
      void set_metronome(context_type *const ctxt, types::flex_data::set_metronome const &in) const { ctxt->push4(in); }
      void set_key_signature(context_type *const ctxt, types::flex_data::set_key_signature const &in) const {
        ctxt->push4(in);
      }
      void set_chord_name(context_type *const ctxt, types::flex_data::set_chord_name const &in) const {
        ctxt->push4(in);
      }
      void text(context_type *const ctxt, types::flex_data::text_common const &in) const { ctxt->push4(in); }
    };
    context_type *context = nullptr;
    [[no_unique_address]] struct utility utility{};
    [[no_unique_address]] struct system system{};
    [[no_unique_address]] struct m1cvm m1cvm{};
    [[no_unique_address]] struct data64 data64{};
    [[no_unique_address]] struct m2cvm m2cvm{};
    [[no_unique_address]] struct data128 data128{};
    [[no_unique_address]] struct ump_stream ump_stream{};
    [[no_unique_address]] struct flex_data flex{};
  };
  context_type context_;
  umpProcessor<to_midi1_config> p_{to_midi1_config{&context_}};
};

}  // end namespace midi2

#endif  // MIDI2_UMPTOMIDI1PROTOCOL_HPP
