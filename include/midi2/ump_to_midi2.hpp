//===-- UMP to MIDI 2 ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMP_TO_MIDI2_HPP
#define MIDI2_UMP_TO_MIDI2_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>

#include "midi2/adt/fifo.hpp"
#include "midi2/ump_dispatcher.hpp"
#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

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
  struct context {
    template <typename T>
      requires(std::tuple_size_v<T> >= 0)
    constexpr void push(T const &value) {
      types::apply(value, [this](auto const v) {
        output.push_back(v.word());
        return false;
      });
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
      [[nodiscard]] constexpr bool is_valid() const noexcept { return lsb_valid && msb_valid; }
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

    /// An alias template for a two-dimensional std::array
    template <typename T, std::size_t Row, std::size_t Col> using array2d = std::array<std::array<T, Col>, Row>;
    array2d<bank, 16, 16> bank{};
    array2d<parameter_number, 16, 16> parameter_number{};
    fifo<std::uint32_t, 4> output;
  };

  struct to_midi2_config {
    // utility messages go straight through.
    struct utility {
      static constexpr void noop(context const *const) { /* Just consume NOOP messages */ }
      static constexpr void jr_clock(context *const ctxt, types::utility::jr_clock const &in) { ctxt->push(in); }
      static constexpr void jr_timestamp(context *const ctxt, types::utility::jr_timestamp const &in) { ctxt->push(in); }
      static constexpr void delta_clockstamp_tpqn(context *const ctxt, types::utility::delta_clockstamp_tpqn const &in) { ctxt->push(in); }
      static constexpr void delta_clockstamp(context *const ctxt, types::utility::delta_clockstamp const &in) { ctxt->push(in); }
      static constexpr void unknown(context const *, std::span<std::uint32_t>) { /* Just drop bad messages */ }
    };
    // system messages go straight through.
    struct system {
      static constexpr void midi_time_code(context *const ctxt, types::system::midi_time_code const &in) { ctxt->push(in); }
      static constexpr void song_position_pointer(context *const ctxt, types::system::song_position_pointer const &in) { ctxt->push(in); }
      static constexpr void song_select(context *const ctxt, types::system::song_select const &in) { ctxt->push(in); }
      static constexpr void tune_request(context *const ctxt, types::system::tune_request const &in) { ctxt->push(in); }
      static constexpr void timing_clock(context *const ctxt, types::system::timing_clock const &in) { ctxt->push(in); }
      static constexpr void seq_start(context *const ctxt, types::system::sequence_start const &in) { ctxt->push(in); }
      static constexpr void seq_continue(context *const ctxt, types::system::sequence_continue const &in) { ctxt->push(in); }
      static constexpr void seq_stop(context *const ctxt, types::system::sequence_stop const &in) { ctxt->push(in); }
      static constexpr void active_sensing(context *const ctxt, types::system::active_sensing const &in) { ctxt->push(in); }
      static constexpr void reset(context *const ctxt, types::system::reset const &in) { ctxt->push(in); }
    };
    // m1cvm messages are converted to m2cvm messages.
    class m1cvm {
    public:
      static void note_off(context *ctxt, types::m1cvm::note_off const &in);
      static void note_on(context *ctxt, types::m1cvm::note_on const &in);
      static void poly_pressure(context *ctxt, types::m1cvm::poly_pressure const &in);
      static void control_change(context *ctxt, types::m1cvm::control_change const &in);
      static void program_change(context *ctxt, types::m1cvm::program_change const &in);
      static void channel_pressure(context *ctxt, types::m1cvm::channel_pressure const &in);
      static void pitch_bend(context *ctxt, types::m1cvm::pitch_bend const &in);

    private:
      template <typename T>
      static void pn_control_message(struct context *const ctxt, struct context::parameter_number const &c,
                                     std::uint8_t const group, std::uint8_t const channel, std::uint8_t const value) {
        auto const out = T{}.group(group).channel(channel).bank(c.pn_msb).index(c.pn_lsb).value(
            mcm_scale<14, 32>(static_cast<std::uint16_t>((static_cast<std::uint16_t>(c.value_msb) << 7) | value)));
        ctxt->push(out);
      }
    };
    // data64 messages go straight through.
    struct data64 {
      static constexpr void sysex7_in_1(context *const ctxt, types::data64::sysex7_in_1 const &in) { ctxt->push(in); }
      static constexpr void sysex7_start(context *const ctxt, types::data64::sysex7_start const &in) { ctxt->push(in); }
      static constexpr void sysex7_continue(context *const ctxt, types::data64::sysex7_continue const &in) {
        ctxt->push(in);
      }
      static constexpr void sysex7_end(context *const ctxt, types::data64::sysex7_end const &in) { ctxt->push(in); }
    };
    // m2cvm messages go straight through.
    struct m2cvm {
    public:
      static constexpr void note_off(context *const ctxt, types::m2cvm::note_off const &in) { ctxt->push(in); }
      static constexpr void note_on(context *const ctxt, types::m2cvm::note_on const &in) { ctxt->push(in); }
      static constexpr void poly_pressure(context *const ctxt, types::m2cvm::poly_pressure const &in) {
        ctxt->push(in);
      }
      static constexpr void program_change(context *const ctxt, types::m2cvm::program_change const &in) {
        ctxt->push(in);
      }
      static constexpr void channel_pressure(context *const ctxt, types::m2cvm::channel_pressure const &in) {
        ctxt->push(in);
      }
      static constexpr void rpn_controller(context *const ctxt, types::m2cvm::rpn_controller const &in) {
        ctxt->push(in);
      }
      static constexpr void nrpn_controller(context *const ctxt, types::m2cvm::nrpn_controller const &in) {
        ctxt->push(in);
      }
      static constexpr void rpn_per_note_controller(context *const ctxt,
                                                    types::m2cvm::rpn_per_note_controller const &in) {
        ctxt->push(in);
      }
      static constexpr void nrpn_per_note_controller(context *const ctxt,
                                                     types::m2cvm::nrpn_per_note_controller const &in) {
        ctxt->push(in);
      }
      static constexpr void rpn_relative_controller(context *const ctxt,
                                                    types::m2cvm::rpn_relative_controller const &in) {
        ctxt->push(in);
      }
      static constexpr void nrpn_relative_controller(context *const ctxt,
                                                     types::m2cvm::nrpn_relative_controller const &in) {
        ctxt->push(in);
      }
      static constexpr void per_note_management(context *const ctxt, types::m2cvm::per_note_management const &in) {
        ctxt->push(in);
      }
      static constexpr void control_change(context *const ctxt, types::m2cvm::control_change const &in) {
        ctxt->push(in);
      }
      static constexpr void pitch_bend(context *const ctxt, types::m2cvm::pitch_bend const &in) { ctxt->push(in); }
      static constexpr void per_note_pitch_bend(context *const ctxt, types::m2cvm::per_note_pitch_bend const &in) {
        ctxt->push(in);
      }
    };
    struct data128 {
      constexpr static void sysex8_in_1(context *const ctxt, types::data128::sysex8_in_1 const &in) { ctxt->push(in); }
      constexpr static void sysex8_start(context *const ctxt, types::data128::sysex8_start const &in) { ctxt->push(in); }
      constexpr static void sysex8_continue(context *const ctxt, types::data128::sysex8_continue const &in) { ctxt->push(in); }
      constexpr static void sysex8_end(context *const ctxt, types::data128::sysex8_end const &in) { ctxt->push(in); }
      constexpr static void mds_header(context *const ctxt, types::data128::mds_header const &in) { ctxt->push(in); }
      constexpr static void mds_payload(context *const ctxt, types::data128::mds_payload const &in) { ctxt->push(in); }
    };
    struct ump_stream {
      constexpr static void endpoint_discovery(context *const ctxt, types::ump_stream::endpoint_discovery const &in) { ctxt->push(in); }
      constexpr static void endpoint_info_notification(context *const ctxt, types::ump_stream::endpoint_info_notification const &in) { ctxt->push(in); }
      constexpr static void device_identity_notification(context *const ctxt, types::ump_stream::device_identity_notification const &in) { ctxt->push(in); }
      constexpr static void endpoint_name_notification(context *const ctxt, types::ump_stream::endpoint_name_notification const &in) { ctxt->push(in); }
      constexpr static void product_instance_id_notification(context *const ctxt, types::ump_stream::product_instance_id_notification const &in) { ctxt->push(in); }
      constexpr static void jr_configuration_request(context *const ctxt, types::ump_stream::jr_configuration_request const &in) { ctxt->push(in); }
      constexpr static void jr_configuration_notification(context *const ctxt, types::ump_stream::jr_configuration_notification const &in) { ctxt->push(in); }
      constexpr static void function_block_discovery(context *const ctxt, types::ump_stream::function_block_discovery const &in) { ctxt->push(in); }
      constexpr static void function_block_info_notification(context *const ctxt, types::ump_stream::function_block_info_notification const &in) { ctxt->push(in); }
      constexpr static void function_block_name_notification(context *const ctxt, types::ump_stream::function_block_name_notification const &in) { ctxt->push(in); }
      constexpr static void start_of_clip(context *const ctxt, types::ump_stream::start_of_clip const &in) { ctxt->push(in); }
      constexpr static void end_of_clip(context *const ctxt, types::ump_stream::end_of_clip const &in) { ctxt->push(in); }
    };
    struct flex_data {
      constexpr static void set_tempo(context *const ctxt, types::flex_data::set_tempo const &in) { ctxt->push(in); }
      constexpr static void set_time_signature(context *const ctxt, types::flex_data::set_time_signature const &in) { ctxt->push(in); }
      constexpr static void set_metronome(context *const ctxt, types::flex_data::set_metronome const &in) { ctxt->push(in); }
      constexpr static void set_key_signature(context *const ctxt, types::flex_data::set_key_signature const &in) { ctxt->push(in); }
      constexpr static void set_chord_name(context *const ctxt, types::flex_data::set_chord_name const &in) { ctxt->push(in); }
      constexpr static void text(context *const ctxt, types::flex_data::text_common const &in) { ctxt->push(in); }
    };
    struct context *context = nullptr;
    [[no_unique_address]] struct utility utility{};
    [[no_unique_address]] struct system system{};
    [[no_unique_address]] class m1cvm m1cvm{};
    [[no_unique_address]] struct data64 data64{};
    [[no_unique_address]] struct m2cvm m2cvm{};
    [[no_unique_address]] struct data128 data128{};
    [[no_unique_address]] struct ump_stream ump_stream{};
    [[no_unique_address]] struct flex_data flex{};
  };

  context context_;
  ump_dispatcher<to_midi2_config> p_{to_midi2_config{&context_}};
};

}  // end namespace midi2

#endif  // MIDI2_UMP_TO_MIDI2_HPP
