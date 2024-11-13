//===-- UMP to MIDI 2 ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/ump_to_midi2.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

// note off
// ~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::note_off(ump_to_midi2::context *const ctxt,
                                                    types::m1cvm::note_off const &in) {
  auto const &noff_in = get<0>(in.w);

  types::m2cvm::note_off noff;
  auto &w0 = get<0>(noff.w);
  auto &w1 = get<1>(noff.w);
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

// note on
// ~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::note_on(ump_to_midi2::context *const ctxt, types::m1cvm::note_on const &in) {
  auto const &non_in = get<0>(in.w);

  types::m2cvm::note_on non{};
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

// poly pressure
// ~~~~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::poly_pressure(ump_to_midi2::context *const ctxt,
                                                         types::m1cvm::poly_pressure const &in) {
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

// control change
// ~~~~~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::control_change(ump_to_midi2::context *const ctxt,
                                                          types::m1cvm::control_change const &in) {
  auto const &cc_in0 = get<0>(in.w);

  auto const group = cc_in0.group.value();
  auto const channel = cc_in0.channel.value();
  auto const controller = cc_in0.controller.value();
  auto const value = cc_in0.value.value();

  auto &c = ctxt->parameter_number[group][channel];
  switch (controller) {
  case control::bank_select: ctxt->bank[group][channel].set_msb(value); break;
  case control::bank_select_lsb: ctxt->bank[group][channel].set_lsb(value); break;

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

// program change
// ~~~~~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::program_change(ump_to_midi2::context *const ctxt,
                                                          types::m1cvm::program_change const &in) {
  auto const &in0 = get<0>(in.w);
  auto const group = in0.group.value();
  auto const channel = in0.channel.value();

  types::m2cvm::program_change out;
  auto &out0 = get<0>(out.w);
  auto &out1 = get<1>(out.w);
  out0.group = group;
  out0.channel = channel;
  out1.program = in0.program.value();

  if (auto const &b = ctxt->bank[group][channel]; b.is_valid()) {
    out0.bank_valid = 1;
    out1.bank_msb = b.msb;
    out1.bank_lsb = b.lsb;
  }
  ctxt->push(out.w);
}

// channel pressure
// ~~~~~~~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::channel_pressure(ump_to_midi2::context *const ctxt,
                                                            types::m1cvm::channel_pressure const &in) {
  auto const &cp_in = get<0>(in.w);

  types::m2cvm::channel_pressure out;
  auto &w0 = get<0>(out.w);
  auto &w1 = get<1>(out.w);
  w0.group = cp_in.group.value();
  w0.channel = cp_in.channel.value();
  constexpr auto m1bits = bits_v<decltype(cp_in.data)>;
  constexpr auto m2bits = bits_v<decltype(w1)>;
  w1 = mcm_scale<m1bits, m2bits>(cp_in.data);
  ctxt->push(out.w);
}

// pitch bend
// ~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::pitch_bend(ump_to_midi2::context *const ctxt,
                                                      types::m1cvm::pitch_bend const &in) {
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

}  // end namespace midi2
