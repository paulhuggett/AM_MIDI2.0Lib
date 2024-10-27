//===-- UMP To MIDI 1 ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/ump_to_midi1.hpp"

#include <cassert>
#include <cstdint>

#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

// note off
// ~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::note_off(context_type *const ctxt, types::m2cvm::note_off const &in) {
  auto const &in0 = get<0>(in.w);
  auto const &in1 = get<1>(in.w);
  types::m1cvm::note_off out;
  auto &out0 = get<0>(out.w);
  out0.group = in0.group.value();
  out0.channel = in0.channel.value();
  out0.note = in0.note.value();
  out0.velocity = static_cast<std::uint8_t>(
      mcm_scale<decltype(in1.velocity)::bits(), decltype(out0.velocity)::bits()>(in1.velocity.value()));
  ctxt->push(out.w);
}

// note on
// ~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::note_on(context_type *const ctxt, types::m2cvm::note_on const &in) {
  auto const &in0 = get<0>(in.w);
  auto const &in1 = get<1>(in.w);
  types::m1cvm::note_on out;
  auto &out0 = get<0>(out.w);
  out0.group = in0.group.value();
  out0.channel = in0.channel.value();
  out0.note = in0.note.value();
  out0.velocity = static_cast<std::uint8_t>(
      mcm_scale<decltype(in1.velocity)::bits(), decltype(out0.velocity)::bits()>(in1.velocity));
  ctxt->push(out.w);
}

// poly pressure
// ~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::poly_pressure(context_type *const ctxt,
                                                         types::m2cvm::poly_pressure const &in) {
  auto const &in0 = get<0>(in.w);
  auto const &in1 = get<1>(in.w);
  types::m1cvm::poly_pressure out;
  auto &out0 = get<0>(out.w);
  out0.group = in0.group.value();
  out0.channel = in0.channel.value();
  out0.note = in0.note.value();
  out0.pressure = static_cast<std::uint8_t>(mcm_scale<32, decltype(out0.pressure)::bits()>(in1));
  ctxt->push(out.w);
}

// program change
// ~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::program_change(context_type *const ctxt,
                                                          types::m2cvm::program_change const &in) {
  auto const &in0 = get<0>(in.w);
  auto const &in1 = get<1>(in.w);

  auto const group = in0.group.value();
  auto const channel = in0.channel.value();
  if (in0.bank_valid) {
    // Control Change numbers 00H and 20H are defined as the Bank Select message. 00H is the MSB and
    // 20H is the LSB for a total of 14 bits. This allows 16,384 banks to be specified.
    types::m1cvm::control_change cc;
    auto &cc0 = get<0>(cc.w);
    cc0.group = group;
    cc0.channel = channel;
    cc0.controller = control::bank_select;
    cc0.value = in1.bank_msb.value();
    ctxt->push(cc.w);

    cc0.controller = control::bank_select_lsb;
    cc0.value = in1.bank_lsb.value();
    ctxt->push(cc.w);
  }
  types::m1cvm::program_change out;
  auto &pc0 = get<0>(out.w);
  pc0.group = group;
  pc0.channel = channel;
  pc0.program = in1.program.value();
  ctxt->push(out.w);
}

// channel pressure
// ~~~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::channel_pressure(context_type *const ctxt,
                                                            types::m2cvm::channel_pressure const &in) {
  types::m1cvm::channel_pressure out;
  types::m1cvm::channel_pressure::word0 &out0 = get<0>(out.w);
  out0.group = get<0>(in.w).group.value();
  out0.channel = get<0>(in.w).channel.value();
  auto const &in1 = get<1>(in.w);
  out0.data = static_cast<std::uint8_t>(mcm_scale<32, decltype(out0.data)::bits()>(in1));
  ctxt->push(out.w);
}

// rpn controller
// ~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::rpn_controller(context_type *const ctxt,
                                                          types::m2cvm::rpn_controller const &in) {
  auto const &in0 = get<0>(in.w);
  pn_message(ctxt, true, in0.group.value(), in0.channel.value(),
             context_type::pn_value::pair14{in0.bank.value(), in0.index.value()}, get<1>(in.w));
}

// nrpn controller
// ~~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::nrpn_controller(context_type *const ctxt,
                                                           types::m2cvm::nrpn_controller const &in) {
  auto const &in0 = get<0>(in.w);
  pn_message(ctxt, false, in0.group.value(), in0.channel.value(),
             context_type::pn_value::pair14{in0.bank.value(), in0.index.value()}, get<1>(in.w));
}

void ump_to_midi1::to_midi1_config::m2cvm::pn_message(context_type *const ctxt, bool const is_rpn,
                                                      std::uint8_t const group, std::uint8_t const channel,
                                                      context_type::pn_value::pair14 const &new_value,
                                                      std::uint32_t const value) {
  static_assert(sizeof(context_type::pn_value) == 5);
  static_assert(sizeof(decltype(context_type::last_pn)) == 5 * 256);
  types::m1cvm::control_change cc;
  auto &cc0 = get<0>(cc.w);
  // The basic procedure for altering a parameter value is to first send the Registered or Non-Registered Parameter
  // Number corresponding to the parameter to be modified, followed by the Data Entry value to be applied to the
  // parameter.
  auto &last = ctxt->last_pn[group][channel];
  auto &last_value = is_rpn ? last.rpn : last.nrpn;
  if (last_value != new_value || !last.valid) {
    last.valid = true;
    last_value = new_value;

    auto &cc0 = get<0>(cc.w);
    cc0.group = group;
    cc0.channel = channel;
    // Controller number MSB
    cc0.controller = is_rpn ? control::rpn_msb : control::nrpn_msb;  // 0x65/0x63
    cc0.value = new_value.first;
    ctxt->push(cc.w);
    // Controller number LSB
    cc0.controller = is_rpn ? control::rpn_lsb : control::nrpn_lsb;  // 0x64/0x62
    cc0.value = new_value.second;
    ctxt->push(cc.w);
  }

  auto const scaled_value = static_cast<std::uint16_t>(mcm_scale<32, 14>(value));

  cc0.group = group;
  cc0.channel = channel;
  // Data Entry MSB
  cc0.controller = control::data_entry_msb;  // 0x6
  cc0.value = static_cast<std::uint8_t>((scaled_value >> 7) & 0x7F);
  ctxt->push(cc.w);

  // Data Entry LSB
  cc0.controller = control::data_entry_lsb;  // 0x26
  cc0.value = static_cast<std::uint8_t>(scaled_value & 0x7F);
  ctxt->push(cc.w);
}

// control change
// ~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::control_change(context_type *const ctxt,
                                                          types::m2cvm::control_change const &in) {
  auto const &in0 = get<0>(in.w);
  types::m1cvm::control_change cc;
  auto &cc0 = get<0>(cc.w);
  // TODO: invalidate the cache if an MSB/LSB is seen.
  cc0.group = in0.group.value();
  cc0.channel = in0.channel.value();
  cc0.controller = in0.controller.value();
  cc0.value = static_cast<std::uint8_t>(mcm_scale<32, decltype(cc0.value)::bits()>(get<1>(in.w)));
  ctxt->push(cc.w);
}

// pitch bend
// ~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::pitch_bend(context_type *const ctxt, types::m2cvm::pitch_bend const &in) {
  auto const &in0 = get<0>(in.w);
  types::m1cvm::pitch_bend pb;
  auto &pb0 = get<0>(pb.w);
  pb0.group = in0.group.value();
  pb0.channel = in0.channel.value();
  auto const scaled_value = get<1>(in.w) >> (32 - 14);
  pb0.lsb_data = scaled_value & 0x7F;
  pb0.msb_data = (scaled_value >> 7) & 0x7F;
  ctxt->push(pb.w);
}

}  // end namespace midi2
