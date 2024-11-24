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
  constexpr auto m2v = types::m2cvm::note_off::word1::velocity::bits();
  constexpr auto m1v = types::m1cvm::note_off::word0::velocity::bits();
  auto const out = types::m1cvm::note_off{}
                       .group(in.group())
                       .channel(in.channel())
                       .note(in.note())
                       .velocity(mcm_scale<m2v, m1v>(in.velocity()));
  ctxt->push(out);
}

// note on
// ~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::note_on(context_type *const ctxt, types::m2cvm::note_on const &in) {
  constexpr auto m2v = types::m2cvm::note_on::word1::velocity::bits();
  constexpr auto m1v = types::m1cvm::note_on::word0::velocity::bits();
  auto const out = types::m1cvm::note_on{}
                       .group(in.group())
                       .channel(in.channel())
                       .note(in.note())
                       .velocity(mcm_scale<m2v, m1v>(in.velocity()));
  ctxt->push(out);
}

// poly pressure
// ~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::poly_pressure(context_type *const ctxt,
                                                         types::m2cvm::poly_pressure const &in) {
  constexpr auto m2v = types::m2cvm::poly_pressure::word1::pressure::bits();
  constexpr auto m1v = types::m1cvm::poly_pressure::word0::pressure::bits();

  auto const out = types::m1cvm::poly_pressure{}
                       .group(in.group())
                       .channel(in.channel())
                       .note(in.note())
                       .pressure(mcm_scale<m2v, m1v>(in.pressure()));
  ctxt->push(out);
}

// program change
// ~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::program_change(context_type *const ctxt,
                                                          types::m2cvm::program_change const &in) {
  auto const group = in.group();
  auto const channel = in.channel();
  if (in.bank_valid()) {
    // Control Change numbers 00H and 20H are defined as the Bank Select message. 00H is the MSB and
    // 20H is the LSB for a total of 14 bits. This allows 16,384 banks to be specified.
    ctxt->push(types::m1cvm::control_change{}
                   .group(group)
                   .channel(channel)
                   .controller(control::bank_select)
                   .value(in.bank_msb()));

    ctxt->push(types::m1cvm::control_change{}
                   .group(group)
                   .channel(channel)
                   .controller(control::bank_select_lsb)
                   .value(in.bank_lsb()));
  }
  ctxt->push(types::m1cvm::program_change{}.group(group).channel(channel).program(in.program()));
}

// channel pressure
// ~~~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::channel_pressure(context_type *const ctxt,
                                                            types::m2cvm::channel_pressure const &in) {
  constexpr auto m2p = types::m2cvm::channel_pressure::word1::value::bits();
  constexpr auto m1p = types::m1cvm::channel_pressure::word0::data::bits();

  const auto out =
      types::m1cvm::channel_pressure{}.group(in.group()).channel(in.channel()).data(mcm_scale<m2p, m1p>(in.value()));
  ctxt->push(out);
}

// rpn controller
// ~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::rpn_controller(context_type *const ctxt,
                                                          types::m2cvm::rpn_controller const &in) {
  pn_message(ctxt, context_type::pn_cache_key{in.group(), in.channel(), true}, std::make_pair(in.bank(), in.index()),
             in.value());
}

// nrpn controller
// ~~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::nrpn_controller(context_type *const ctxt,
                                                           types::m2cvm::nrpn_controller const &in) {
  pn_message(ctxt, context_type::pn_cache_key{in.group(), in.channel(), false}, std::make_pair(in.bank(), in.index()),
             in.value());
}

void ump_to_midi1::to_midi1_config::m2cvm::pn_message(context_type *const ctxt, context_type::pn_cache_key const &key,
                                                      std::pair<std::uint8_t, std::uint8_t> const &controller_number,
                                                      std::uint32_t const value) {
  types::m1cvm::control_change cc;
  cc.group(key.group).channel(key.channel);

  // The basic procedure for altering a parameter value is to first send the Registered or Non-Registered Parameter
  // Number corresponding to the parameter to be modified, followed by the Data Entry value to be applied to the
  // parameter.
  if (!ctxt->pn_cache.set(key, controller_number)) {
    // Controller number MSB
    cc.controller(key.is_rpn ? control::rpn_msb : control::nrpn_msb);  // 0x65/0x63
    cc.value(controller_number.first);
    ctxt->push(cc);
    // Controller number LSB
    cc.controller(key.is_rpn ? control::rpn_lsb : control::nrpn_lsb);  // 0x64/0x62
    cc.value(controller_number.second);
    ctxt->push(cc);
  }

  auto const scaled_value = mcm_scale<32, 14>(value);

  // Data Entry MSB
  cc.controller(control::data_entry_msb);  // 0x6
  cc.value(static_cast<std::uint8_t>((scaled_value >> 7) & 0x7F));
  ctxt->push(cc);

  // Data Entry LSB
  cc.controller(control::data_entry_lsb);  // 0x26
  cc.value(static_cast<std::uint8_t>(scaled_value & 0x7F));
  ctxt->push(cc);
}

// control change
// ~~~~~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::control_change(context_type *const ctxt,
                                                          types::m2cvm::control_change const &in) {
  constexpr auto m1v = types::m1cvm::control_change::word0::value::bits();
  constexpr auto m2v = types::m2cvm::control_change::word1::value::bits();
  auto const cc = types::m1cvm::control_change{}
                      .group(in.group())
                      .channel(in.channel())
                      .controller(in.controller())
                      .value(mcm_scale<m2v, m1v>(in.value()));
  ctxt->push(cc);
}

// pitch bend
// ~~~~~~~~~~
void ump_to_midi1::to_midi1_config::m2cvm::pitch_bend(context_type *const ctxt, types::m2cvm::pitch_bend const &in) {
  auto const scaled_value = in.value() >> (32 - 14);
  auto const out = types::m1cvm::pitch_bend{}
                       .group(in.group())
                       .channel(in.channel())
                       .lsb_data(scaled_value & 0x7F)
                       .msb_data((scaled_value >> 7) & 0x7F);
  ctxt->push(out);
}

}  // end namespace midi2
