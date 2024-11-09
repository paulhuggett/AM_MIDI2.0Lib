//===-- UMP to MIDI 2 ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMP_TO_MIDI2_HPP
#define MIDI2_UMP_TO_MIDI2_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "midi2/adt/fifo.hpp"
#include "midi2/ump_dispatcher.hpp"
#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

template <typename T> struct bits {};

template <> struct bits<std::uint32_t> : std::integral_constant<unsigned, 32> {};
template <> struct bits<std::uint16_t> : std::integral_constant<unsigned, 16> {};
template <> struct bits<std::uint8_t> : std::integral_constant<unsigned, 6> {};

template <typename T> struct bits<T &> : bits<std::remove_cvref_t<T>> {};

template <std::unsigned_integral Container, unsigned Index, unsigned Bits>
struct bits<bitfield<Container, Index, Bits>>
    : std::integral_constant<unsigned, bitfield<Container, Index, Bits>::bits()> {};

template <typename T> static constexpr auto bits_v = bits<T>{}();

class ump_to_midi2 {
public:
  using input_type = std::uint32_t;
  using output_type = std::uint32_t;

  explicit constexpr ump_to_midi2(std::uint8_t const group) {
    assert(group <= 0b1111);
    context_.group = group;
  }

  [[nodiscard]] constexpr bool empty() const { return context_.output.empty(); }
  [[nodiscard]] constexpr output_type pop() {
    assert(!context_.output.empty());
    return context_.output.pop_front();
  }

  void push(input_type const ump) { p_.processUMP(ump); }

private:
  struct context_type {
    template <typename T, unsigned Index = 0>
      requires(std::tuple_size_v<T> >= 0)
    constexpr void push(T const &value) {
      if constexpr (Index >= std::tuple_size_v<T>) {
        return;
      } else {
        output.push_back(std::bit_cast<std::uint32_t>(std::get<Index>(value)));
        push<T, Index + 1>(value);
      }
    }

    struct bank {
      std::uint8_t msb_valid : 1 = false;
      std::uint8_t msb : 7 = 0;  // Set by bank_select control
      std::uint8_t lsb_valid : 1 = false;
      std::uint8_t lsb : 7 = 0;  // Set by bank_select_lsb control

      constexpr void set_msb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        msb = value;
        msb_valid = true;
      }
      constexpr void set_lsb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        lsb = value;
        lsb_valid = true;
      }
      constexpr bool is_valid() const noexcept { return lsb_valid && msb_valid; }
    };

    /// Represents the status of registered (RPN) or non-registered/assignable (NRPN) parameters
    struct parameter_number {
      bool pn_is_rpn = false;  ///< Is this nrpn or rpn?
      std::uint8_t pn_msb_valid : 1 = false;
      std::uint8_t pn_msb : 7 = 0;  ///< Set by nrpn_msb/rpn_msb
      std::uint8_t pn_lsb_valid : 1 = false;
      std::uint8_t pn_lsb : 7 = 0;  ///< Set by nrpn_lsb/rpn_lsb

      std::uint8_t value_msb_valid : 1 = false;
      std::uint8_t value_msb : 7 = 0;

      constexpr void set_number_msb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        pn_msb = value;
        pn_msb_valid = true;
      }
      constexpr void set_number_lsb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        pn_lsb = value;
        pn_lsb_valid = true;
      }
      constexpr void reset_number() noexcept {
        pn_msb_valid = false;
        pn_msb = 0;
        pn_lsb_valid = false;
        pn_lsb = 0;
      }
      constexpr void set_value_msb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        value_msb = value;
        value_msb_valid = true;
      }
    };

    std::uint8_t group = 0;
    std::array<bank, 16> bank{};
    std::array<parameter_number, 16> parameter_number{};
    fifo<std::uint32_t, 4> output;
  };

  struct to_midi2_config {
    // utility messages go straight through.
    struct utility {
      static constexpr void noop(context_type *const ) { /* TODO: pass this on? */ }
      static constexpr void jr_clock(context_type *const ctxt, types::utility::jr_clock const &in) { ctxt->push(in.w); }
      static constexpr void jr_timestamp(context_type *const ctxt, types::utility::jr_timestamp const &in) { ctxt->push(in.w); }
      static constexpr void delta_clockstamp_tpqn(context_type *const ctxt, types::utility::delta_clockstamp_tpqn const &in) { ctxt->push(in.w); }
      static constexpr void delta_clockstamp(context_type *const ctxt, types::utility::delta_clockstamp const &in) { ctxt->push(in.w); }
      static constexpr void unknown(context_type *const , std::span<std::uint32_t> ) { /* TODO: pass this on? */ }
    };
    // system messages go straight through.
    struct system {
      static constexpr void midi_time_code(context_type *const ctxt, types::system::midi_time_code const &in) { ctxt->push(in.w); }
      static constexpr void song_position_pointer(context_type *const ctxt, types::system::song_position_pointer const &in) { ctxt->push(in.w); }
      static constexpr void song_select(context_type *const ctxt, types::system::song_select const &in) { ctxt->push(in.w); }
      static constexpr void tune_request(context_type *const ctxt, types::system::tune_request const &in) { ctxt->push(in.w); }
      static constexpr void timing_clock(context_type *const ctxt, types::system::timing_clock const &in) { ctxt->push(in.w); }
      static constexpr void seq_start(context_type *const ctxt, types::system::sequence_start const &in) { ctxt->push(in.w); }
      static constexpr void seq_continue(context_type *const ctxt, types::system::sequence_continue const &in) { ctxt->push(in.w); }
      static constexpr void seq_stop(context_type *const ctxt, types::system::sequence_stop const &in) { ctxt->push(in.w); }
      static constexpr void active_sensing(context_type *const ctxt, types::system::active_sensing const &in) { ctxt->push(in.w); }
      static constexpr void reset(context_type *const ctxt, types::system::reset const &in) { ctxt->push(in.w); }
    };
    // m1cvm messages are converted to m2cvm messages.
    class m1cvm {
    public:
      static void note_off(context_type *const ctxt, types::m1cvm::note_off const &in) {
        auto const & noff_in = get<0>(in.w);

        types::m2cvm::note_off noff;
        auto & w0 = get<0>(noff.w);
        auto & w1 = get<1>(noff.w);
        w0.group = noff_in.group.value();
        w0.channel = noff_in.channel.value();
        w0.note = noff_in.note.value();
        w0.attribute = 0;
        constexpr auto m1bits = bits_v<decltype(noff_in.velocity)>;
        constexpr auto m2bits = bits_v<decltype(w1.velocity)>;
        w1.velocity = static_cast<decltype(w1.velocity)::small_type>(mcm_scale<m1bits, m2bits>(noff_in.velocity));
        w1.attribute = 0;
        ctxt->push(noff.w);
      }
      static void note_on(context_type *const ctxt, types::m1cvm::note_on const &in) {
        auto const &non_in = get<0>(in.w);

        types::m2cvm::note_on non;
        auto &w0 = get<0>(non.w);
        auto &w1 = get<1>(non.w);
        w0.group = non_in.group.value();
        w0.channel = non_in.channel.value();
        w0.note = non_in.note.value();
        w0.attribute = 0;
        constexpr auto m1bits = bits_v<decltype(non_in.velocity)>;
        constexpr auto m2bits = bits_v<decltype(w1.velocity)>;
        w1.velocity = static_cast<decltype(w1.velocity)::small_type>(mcm_scale<m1bits, m2bits>(non_in.velocity));
        w1.attribute = 0;
        ctxt->push(non.w);
      }
      static void poly_pressure(context_type *const ctxt, types::m1cvm::poly_pressure const &in) {
        auto const &pp_in = get<0>(in.w);

        types::m2cvm::poly_pressure out;
        auto &w0 = get<0>(out.w);
        auto &w1 = get<1>(out.w);
        w0.group = pp_in.group.value();
        w0.channel = pp_in.channel.value();
        w0.note = pp_in.note.value();
        constexpr auto m1bits = bits_v<decltype(pp_in.pressure)>;
        constexpr auto m2bits = bits_v<decltype(w1)>;
        w1 = mcm_scale<m1bits, m2bits>(pp_in.pressure);
        ctxt->push(out.w);
      }
      static void control_change(context_type *const ctxt, types::m1cvm::control_change const &in) {
        auto const &cc_in0 = get<0>(in.w);

        auto const group = cc_in0.group.value();
        auto const channel = cc_in0.channel.value();
        auto const controller = cc_in0.controller.value();
        auto const value = cc_in0.value.value();

        auto &c = ctxt->parameter_number[channel];
        switch (controller) {
        case control::bank_select: ctxt->bank[channel].set_msb(value); break;
        case control::bank_select_lsb: ctxt->bank[channel].set_lsb(value); break;

        case control::nrpn_msb:
          c.pn_is_rpn = false;
          c.set_number_msb(value);
          break;
        case control::nrpn_lsb:
          c.pn_is_rpn = false;
          c.pn_lsb_valid = true;
          c.pn_lsb = value;
          break;

        case control::rpn_msb:
          c.pn_is_rpn = true;
          c.set_number_msb(value);
          break;
        case control::rpn_lsb:
          // Setting RPN to 7FH,7FH will disable the data entry, data increment, and data decrement controllers
          // until a new RPN or NRPN is selected. (MIDI 1.0 Approved Protocol JMSC-0011)
          if (c.pn_is_rpn && c.pn_msb_valid && c.pn_msb == 0x7F && value == 0x7F) {
            c.reset_number();
          } else {
            c.pn_is_rpn = true;
            c.set_number_lsb(value);
          }
          break;

        case control::data_entry_msb: c.set_value_msb(value); break;

        case control::data_entry_lsb:
          if (c.pn_msb_valid && c.pn_lsb_valid && c.value_msb_valid) {
            if (c.pn_is_rpn) {
              pn_control_message<types::m2cvm::rpn_controller>(ctxt, c, group, channel, value);
            } else {
              pn_control_message<types::m2cvm::nrpn_controller>(ctxt, c, group, channel, value);
            }
          }
          break;

        case control::reset_all_controllers: c.reset_number(); [[fallthrough]];

        default: {
          types::m2cvm::control_change out;
          auto &out0 = get<0>(out.w);
          out0.group = group;
          out0.channel = channel;
          out0.controller = controller;
          auto &out1 = get<1>(out.w);
          out1 = midi2::mcm_scale<bits_v<decltype(cc_in0.value)>, bits_v<decltype(out1)>>(value);
          ctxt->push(out.w);
          break;
        }
        }
      }

      static void program_change(context_type *const ctxt, types::m1cvm::program_change const &in) {
        auto const &in0 = get<0>(in.w);
        auto const group = in0.group.value();
        // TODO: filter group
        auto const channel = in0.channel.value();

        types::m2cvm::program_change out;
        auto &out0 = get<0>(out.w);
        auto &out1 = get<1>(out.w);
        out0.group = group;
        out0.channel = channel;
        out1.program = in0.program.value();

        if (auto const &b = ctxt->bank[channel]; b.is_valid()) {
          out0.bank_valid = 1;
          out1.bank_msb = b.msb;
          out1.bank_lsb = b.lsb;
        }
        ctxt->push(out.w);
      }
      static constexpr void channel_pressure(context_type *const /*ctxt*/,
                                             types::m1cvm::channel_pressure const & /*in*/) {
        /* TODO: implement! */
      }
      static void pitch_bend(context_type *const ctxt, types::m1cvm::pitch_bend const &in) {
        auto const &pb_in = get<0>(in.w);

        types::m2cvm::pitch_bend out;
        auto &w0 = get<0>(out.w);
        auto &w1 = get<1>(out.w);
        w0.group = pb_in.group.value();
        w0.channel = pb_in.channel.value();
        constexpr auto lsb_bits = bits_v<decltype(pb_in.lsb_data)>;
        constexpr auto msb_bits = bits_v<decltype(pb_in.msb_data)>;
        static_assert(lsb_bits + msb_bits <= 16);
        w1 = mcm_scale<lsb_bits + msb_bits, bits_v<decltype(w1)>>(
            static_cast<std::uint16_t>((pb_in.msb_data << lsb_bits) | pb_in.lsb_data));
        ctxt->push(out.w);
      }

    private:
      template <typename T>
      static void pn_control_message(context_type *const ctxt, struct context_type::parameter_number const &c,
                                     std::uint8_t const group, std::uint8_t const channel, std::uint8_t const value) {
        T out;
        auto &out0 = get<0>(out.w);
        out0.group = group;
        out0.channel = channel;
        out0.bank = c.pn_msb;
        out0.index = c.pn_lsb;
        get<1>(out.w) =
            mcm_scale<14, 32>((static_cast<std::uint32_t>(c.value_msb) << 7) | static_cast<std::uint32_t>(value));
        ctxt->push(out.w);
      }
    };
    // data64 messages go straight through.
    struct data64 {
      static constexpr void sysex7_in_1(context_type *const ctxt, types::data64::sysex7_in_1 const &in) { ctxt->push(in.w); }
      static constexpr void sysex7_start(context_type *const ctxt, types::data64::sysex7_start const &in) { ctxt->push(in.w); }
      static constexpr void sysex7_continue(context_type *const ctxt, types::data64::sysex7_continue const &in) { ctxt->push(in.w); }
      static constexpr void sysex7_end(context_type *const ctxt, types::data64::sysex7_end const &in) { ctxt->push(in.w); }
    };
    // m2cvm messages go straight through.
    struct m2cvm {
    public:
      static constexpr void note_off(context_type *const ctxt, types::m2cvm::note_off const &in) { ctxt->push(in.w); }
      static constexpr void note_on(context_type *const ctxt, types::m2cvm::note_on const &in) { ctxt->push(in.w); }
      static constexpr void poly_pressure(context_type *const ctxt, types::m2cvm::poly_pressure const &in) { ctxt->push(in.w); }
      static constexpr void program_change(context_type *const ctxt, types::m2cvm::program_change const &in) { ctxt->push(in.w); }
      static constexpr void channel_pressure(context_type *const ctxt, types::m2cvm::channel_pressure const &in) { ctxt->push(in.w); }
      static constexpr void rpn_controller(context_type *const ctxt, types::m2cvm::rpn_controller const &in) { ctxt->push(in.w); }
      static constexpr void nrpn_controller(context_type *const ctxt, types::m2cvm::nrpn_controller const &in){ ctxt->push(in.w); }
      static constexpr void rpn_per_note_controller(context_type * const ctxt, midi2::types::m2cvm::rpn_per_note_controller const &in) { ctxt->push(in.w); }
      static constexpr void nrpn_per_note_controller(context_type * const ctxt, midi2::types::m2cvm::nrpn_per_note_controller const &in) { ctxt->push(in.w); }
      static constexpr void rpn_relative_controller(context_type * const ctxt, midi2::types::m2cvm::rpn_relative_controller const &in) { ctxt->push(in.w); }
      static constexpr void nrpn_relative_controller(context_type * const ctxt, midi2::types::m2cvm::nrpn_relative_controller const &in) { ctxt->push(in.w); }
      static constexpr void per_note_management(context_type * const ctxt, midi2::types::m2cvm::per_note_management const &in) { ctxt->push(in.w); }
      static constexpr void control_change(context_type * const ctxt, midi2::types::m2cvm::control_change const &in) { ctxt->push(in.w); }
      static constexpr void pitch_bend(context_type * const ctxt, midi2::types::m2cvm::pitch_bend const &in) { ctxt->push(in.w); }
      static constexpr void per_note_pitch_bend(context_type * const ctxt, midi2::types::m2cvm::per_note_pitch_bend const &in) { ctxt->push(in.w); }
    };
    struct data128 {
      constexpr static void sysex8_in_1(context_type *const ctxt, types::data128::sysex8 const &in) { ctxt->push(in.w); }
      constexpr static void sysex8_start(context_type *const ctxt, types::data128::sysex8 const &in) { ctxt->push(in.w); }
      constexpr static void sysex8_continue(context_type *const ctxt, types::data128::sysex8 const &in) { ctxt->push(in.w); }
      constexpr static void sysex8_end(context_type *const ctxt, types::data128::sysex8 const &in) { ctxt->push(in.w); }
      constexpr static void mds_header(context_type *const ctxt, types::data128::mds_header const &in) { ctxt->push(in.w); }
      constexpr static void mds_payload(context_type *const ctxt, types::data128::mds_payload const &in) { ctxt->push(in.w); }
    };
    struct ump_stream {
      constexpr static void endpoint_discovery(context_type *const ctxt, types::ump_stream::endpoint_discovery const &in) { ctxt->push(in.w); }
      constexpr static void endpoint_info_notification(context_type *const ctxt, types::ump_stream::endpoint_info_notification const &in) { ctxt->push(in.w); }
      constexpr static void device_identity_notification(context_type *const ctxt, types::ump_stream::device_identity_notification const &in) { ctxt->push(in.w); }
      constexpr static void endpoint_name_notification(context_type *const ctxt, types::ump_stream::endpoint_name_notification const &in) { ctxt->push(in.w); }
      constexpr static void product_instance_id_notification(context_type *const ctxt, types::ump_stream::product_instance_id_notification const &in) { ctxt->push(in.w); }
      constexpr static void jr_configuration_request(context_type *const ctxt, types::ump_stream::jr_configuration_request const &in) { ctxt->push(in.w); }
      constexpr static void jr_configuration_notification( context_type *const ctxt, types::ump_stream::jr_configuration_notification const &in) { ctxt->push(in.w); }
      constexpr static void function_block_discovery(context_type *const ctxt, types::ump_stream::function_block_discovery const &in) { ctxt->push(in.w); }
      constexpr static void function_block_info_notification(context_type *const ctxt, types::ump_stream::function_block_info_notification const &in) { ctxt->push(in.w); }
      constexpr static void function_block_name_notification(context_type *const ctxt, types::ump_stream::function_block_name_notification const &in) { ctxt->push(in.w); }
      constexpr static void start_of_clip(context_type *const ctxt, types::ump_stream::start_of_clip const &in) { ctxt->push(in.w); }
      constexpr static void end_of_clip(context_type *const ctxt, types::ump_stream::end_of_clip const &in) { ctxt->push(in.w); }
    };
    struct flex_data {
      constexpr static void set_tempo(context_type *const ctxt, types::flex_data::set_tempo const &in) { ctxt->push(in.w); }
      constexpr static void set_time_signature(context_type *const ctxt, types::flex_data::set_time_signature const &in) { ctxt->push(in.w); }
      constexpr static void set_metronome(context_type *const ctxt, types::flex_data::set_metronome const &in) { ctxt->push(in.w); }
      constexpr static void set_key_signature(context_type *const ctxt, types::flex_data::set_key_signature const &in) { ctxt->push(in.w); }
      constexpr static void set_chord_name(context_type *const ctxt, types::flex_data::set_chord_name const &in) { ctxt->push(in.w); }
      constexpr static void text(context_type *const ctxt, types::flex_data::text_common const &in) { ctxt->push(in.w); }
    };
    context_type *context = nullptr;
    [[no_unique_address]] struct utility utility{};
    [[no_unique_address]] struct system system{};
    [[no_unique_address]] class m1cvm m1cvm{};
    [[no_unique_address]] struct data64 data64{};
    [[no_unique_address]] struct m2cvm m2cvm{};
    [[no_unique_address]] struct data128 data128{};
    [[no_unique_address]] struct ump_stream ump_stream{};
    [[no_unique_address]] struct flex_data flex{};
  };

  context_type context_;
  ump_dispatcher<to_midi2_config> p_{to_midi2_config{&context_}};
};

}  // end namespace midi2

#endif  // MIDI2_UMP_TO_MIDI2_HPP
