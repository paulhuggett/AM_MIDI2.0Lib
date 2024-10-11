//===-- UMP To MIDI 1 ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/umpToMIDI1Protocol.hpp"

#include <cassert>
#include <cstdint>

#include "midi2/umpMessageCreate.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

// note off
// ~~~~~~~~
void umpToMIDI1Protocol::to_midi1_config::m2cvm::note_off(context_type *const ctxt,
                                                          types::m2cvm::note_off const &in) const {
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
void umpToMIDI1Protocol::to_midi1_config::m2cvm::note_on(context_type *const ctxt,
                                                         types::m2cvm::note_on const &in) const {
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
void umpToMIDI1Protocol::to_midi1_config::m2cvm::poly_pressure(context_type *const ctxt,
                                                               types::m2cvm::poly_pressure const &in) const {
  auto &in0 = get<0>(in.w);
  auto &in1 = get<1>(in.w);
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
void umpToMIDI1Protocol::to_midi1_config::m2cvm::program_change(context_type *const ctxt,
                                                                types::m2cvm::program_change const &in) const {
  auto &in0 = get<0>(in.w);
  auto &in1 = get<1>(in.w);

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
void umpToMIDI1Protocol::to_midi1_config::m2cvm::channel_pressure(context_type *const ctxt,
                                                                  types::m2cvm::channel_pressure const &in) const {
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
void umpToMIDI1Protocol::to_midi1_config::m2cvm::rpn_controller(context_type *const ctxt,
                                                                types::m2cvm::rpn_controller const &in) const {
  // TODO(pbh): the two (n)rpn_controller handlers are identical apart from the controller numbers for the first two
  // messages. this should be merged later.
  auto const &in0 = get<0>(in.w);
  types::m1cvm::control_change cc;
  auto &cc0 = get<0>(cc.w);
  cc0.group = in0.group.value();
  cc0.channel = in0.channel.value();
  cc0.controller = control::rpn_msb;  // 0x65 #101
  cc0.value = in0.bank.value();
  ctxt->push(cc.w);

  cc0.controller = control::rpn_lsb;  // 0x64 #100
  cc0.value = in0.index.value();
  ctxt->push(cc.w);

  auto const scaled_value = static_cast<std::uint16_t>(mcm_scale<32, 14>(get<1>(in.w)));

  cc0.controller = control::data_entry_msb;  // 0x6
  cc0.value = static_cast<std::uint8_t>((scaled_value >> 7) & 0x7F);
  ctxt->push(cc.w);

  cc0.controller = control::data_entry_lsb;  // 0x26 #38
  cc0.value = static_cast<std::uint8_t>(scaled_value & 0x7F);
  ctxt->push(cc.w);
}

// nrpn controller
// ~~~~~~~~~~~~~~~
void umpToMIDI1Protocol::to_midi1_config::m2cvm::nrpn_controller(context_type *const ctxt,
                                                                 types::m2cvm::nrpn_controller const &in) const {
  auto const &in0 = get<0>(in.w);
  types::m1cvm::control_change cc;
  auto &cc0 = get<0>(cc.w);
  cc0.group = in0.group.value();
  cc0.channel = in0.channel.value();
  cc0.controller = control::nrpn_msb;  // 0x63 #99
  cc0.value = in0.bank.value();
  ctxt->push(cc.w);

  cc0.controller = control::nrpn_lsb;  // 0x62 #98
  cc0.value = in0.index.value();
  ctxt->push(cc.w);

  auto const scaled_value = static_cast<std::uint16_t>(mcm_scale<32, 14>(get<1>(in.w)));

  cc0.controller = control::data_entry_msb;  // 0x6
  cc0.value = (scaled_value >> 7) & 0x7F;
  ctxt->push(cc.w);

  cc0.controller = control::data_entry_lsb;  // 0x26 #38
  cc0.value = scaled_value & 0x7F;
  ctxt->push(cc.w);
}
void umpToMIDI1Protocol::to_midi1_config::m2cvm::control_change(context_type *const ctxt,
                                                                types::m2cvm::control_change const &in) const {
  auto const &in0 = get<0>(in.w);
  types::m1cvm::control_change cc;
  auto &cc0 = get<0>(cc.w);
  cc0.group = in0.group.value();
  cc0.channel = in0.channel.value();
  cc0.controller = in0.controller.value();
  cc0.value = static_cast<std::uint8_t>(mcm_scale<32, decltype(cc0.value)::bits()>(get<1>(in.w)));
  ctxt->push(cc.w);
}
void umpToMIDI1Protocol::to_midi1_config::m2cvm::controller_message(context_type *const ctxt,
                                                                    types::m2cvm::controller_message const &) const {
  (void)ctxt;
  // TODO: stuff goes here
}
void umpToMIDI1Protocol::to_midi1_config::m2cvm::pitch_bend(context_type *const ctxt,
                                                            types::m2cvm::pitch_bend const &) const {
  (void)ctxt;
  // TODO: stuff goes here
}

#if 0
void umpToMIDI1Protocol::UMPStreamParse(uint32_t ump) {
  switch (UMPPos) {
  case 0: {  // First UMP Packet
    // First part of a UMP Message
    mType = static_cast<ump_message_type>(UMP >> 28);
    switch (mType) {
    case ump_message_type::utility:        // 32 bits Utility Messages
    case ump_message_type::reserved32_06:  // 32 Reserved
    case ump_message_type::reserved32_07:  // 32 Reserved
      break;
      // 32-bit MIDI 1.0 Channel Voice Messages
    case ump_message_type::m1cvm:
      // 32-bit System Real Time and System Common Messages (except
      // System Exclusive)
    case ump_message_type::system:
      output_.push_back(UMP);
      break;

      // 64-bit Data Messages (including System Exclusive)
    case ump_message_type::data64:
      // MIDI2.0 Channel Voice Messages
    case ump_message_type::m2cvm:
      ump64word1 = UMP;
      UMPPos++;
      break;
    case ump_message_type::data128:
    case ump_message_type::flex_data:
    case ump_message_type::ump_stream:
    case ump_message_type::reserved128_0E:
    case ump_message_type::reserved64_08:
    case ump_message_type::reserved64_09:
    case ump_message_type::reserved64_0A:
    case ump_message_type::reserved96_0B:
    case ump_message_type::reserved96_0C:
    default: UMPPos++; break;
    }
    break;
  }
  case 1: {  // 64Bit+ Messages only
    switch (mType) {
    case ump_message_type::reserved64_08:  // 64 Reserved
    case ump_message_type::reserved64_09:  // 64 Reserved
    case ump_message_type::reserved64_0A:  // 64 Reserved
      UMPPos = 0;
      break;
      // 64 bits Data Messages (including System Exclusive) part 2
    case ump_message_type::data64: {
      UMPPos = 0;
      output_.push_back(ump64word1);
      output_.push_back(UMP);
      break;
    }
    case ump_message_type::m2cvm: {
      UMPPos = 0;
      uint8_t const status = (ump64word1 >> 16) & 0xFF;
      uint8_t const channel = (ump64word1 >> 16) & 0xF;
      uint8_t const val1 = (ump64word1 >> 8) & 0xFF;
      uint8_t const val2 = ump64word1 & 0xFF;
      uint8_t const group = (ump64word1 >> 24) & 0xF;

      switch (status) {
      case note_off:
        output_.push_back(
            UMPMessage::mt2NoteOff(group, channel, val1, static_cast<std::uint8_t>(scaleDown((UMP >> 16), 16, 7))));
        break;
      case note_on: {
        auto velocity = static_cast<std::uint8_t>(scaleDown((UMP >> 16), 16, 7));
        if (velocity == 0) {
          velocity = 1;
        }
        output_.push_back(UMPMessage::mt2NoteOn(group, channel, val1, velocity));
        break;
      }
      case key_pressure:
        output_.push_back(
            UMPMessage::mt2PolyPressure(group, channel, val1, static_cast<std::uint8_t>(scaleDown(UMP, 32, 7))));
        break;
      case cc:
        output_.push_back(UMPMessage::mt2CC(group, channel, val1, static_cast<std::uint8_t>(scaleDown(UMP, 32, 7))));
        break;
      case channel_pressure:
        output_.push_back(
            UMPMessage::mt2ChannelPressure(group, channel, static_cast<std::uint8_t>(scaleDown(UMP, 32, 7))));
        break;
      case nrpn: {
        output_.push_back(UMPMessage::mt2CC(group, channel, 99, val1));
        output_.push_back(UMPMessage::mt2CC(group, channel, 98, val2));
        auto const val14bit = static_cast<std::uint16_t>(scaleDown(UMP, 32, 14));
        output_.push_back(UMPMessage::mt2CC(group, channel, 6, (val14bit >> 7) & 0x7F));
        output_.push_back(UMPMessage::mt2CC(group, channel, 38, val14bit & 0x7F));
        break;
      }
      case rpn: {
        output_.push_back(UMPMessage::mt2CC(group, channel, 101, val1));
        output_.push_back(UMPMessage::mt2CC(group, channel, 100, val2));
        auto const val14bit = static_cast<std::uint16_t>(scaleDown(UMP, 32, 14));
        output_.push_back(UMPMessage::mt2CC(group, channel, 6, (val14bit >> 7) & 0x7F));
        output_.push_back(UMPMessage::mt2CC(group, channel, 38, val14bit & 0x7F));
        break;
      }
      case program_change: {
        if (ump64word1 & 0x1) {
          output_.push_back(UMPMessage::mt2CC(group, channel, 0, (UMP >> 8) & 0x7F));
          output_.push_back(UMPMessage::mt2CC(group, channel, 32, UMP & 0x7F));
        }
        output_.push_back(UMPMessage::mt2ProgramChange(group, channel, (UMP >> 24) & 0x7F));
        break;
      }
      case pitch_bend: output_.push_back(UMPMessage::mt2PitchBend(group, channel, UMP >> 18)); break;
      default:
        // An unknown CVM message
        break;
      }
      break;
    }
    case ump_message_type::data128:
    case ump_message_type::flex_data:
    case ump_message_type::m1cvm:
    case ump_message_type::ump_stream:
    case ump_message_type::reserved128_0E:
    case ump_message_type::reserved32_06:
    case ump_message_type::reserved32_07:
    case ump_message_type::reserved96_0B:
    case ump_message_type::reserved96_0C:
    case ump_message_type::system:
    case ump_message_type::utility:
    default: UMPPos++; break;
    }
    break;
  }
  case 2: {
    switch (mType) {
    case ump_message_type::reserved96_0B:  // 96 Reserved
    case ump_message_type::reserved96_0C:  // 96 Reserved
      UMPPos = 0;
      break;
    case ump_message_type::data128:
    case ump_message_type::flex_data:
    case ump_message_type::m1cvm:
    case ump_message_type::m2cvm:
    case ump_message_type::ump_stream:
    case ump_message_type::reserved128_0E:
    case ump_message_type::reserved32_06:
    case ump_message_type::reserved32_07:
    case ump_message_type::reserved64_08:
    case ump_message_type::reserved64_09:
    case ump_message_type::reserved64_0A:
    case ump_message_type::data64:
    case ump_message_type::system:
    case ump_message_type::utility:
    default: UMPPos++; break;
    }
    break;
  }
  case 3: {
    UMPPos = 0;
    break;
  }
  default: assert(false); break;
  }
}
#endif

}  // end namespace midi2
