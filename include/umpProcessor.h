/**********************************************************
 * MIDI 2.0 Library
 * Author: Andrew Mee
 *
 * MIT License
 * Copyright 2021 Andrew Mee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ********************************************************/
#ifndef UMP_PROCESSOR_H
#define UMP_PROCESSOR_H

#include <array>
#include <concepts>
#include <cstdint>
#include <functional>

#include "utils.h"

struct umpCommon {
  uint8_t group = 255;
  ump_message_type messageType = ump_message_type::utility;
  uint8_t status = 0;
};
struct umpCVM {
  umpCommon common;
  uint8_t channel = 0xFF;
  uint8_t note = 0xFF;
  uint32_t value = 0;
  uint16_t index = 0;
  uint8_t bank = 0;
  bool flag1 = false;
  bool flag2 = false;
};

struct umpGeneric {
  umpCommon common;
  std::uint16_t value = 0;
};

struct umpData {
  umpCommon common;
  uint8_t streamId = 0;
  uint8_t form = 0;
  uint8_t* data = nullptr;
  uint8_t dataLength = 0;
};

// (note that these are mostly four bit fields. The type fields are the
// exception. )
struct chord {
  uint8_t chShrpFlt;
  uint8_t chTonic;
  uint8_t chType;
  uint8_t chAlt1Type;
  uint8_t chAlt1Deg;
  uint8_t chAlt2Type;
  uint8_t chAlt2Deg;
  uint8_t chAlt3Type;
  uint8_t chAlt3Deg;
  uint8_t chAlt4Type;
  uint8_t chAlt4Deg;
  uint8_t baShrpFlt;
  uint8_t baTonic;
  uint8_t baType;
  uint8_t baAlt1Type;
  uint8_t baAlt1Deg;
  uint8_t baAlt2Type;
  uint8_t baAlt2Deg;
};

using uint32_ptr = std::uint32_t*;
template <typename T, typename IntegerType = std::int64_t>
concept backend = requires(T && v) {
  { v.utility_message(umpGeneric{}) } -> std::same_as<void>;
  { v.channel_voice_message(umpCVM{}) } -> std::same_as<void>;
  { v.system_message(umpGeneric{}) } -> std::same_as<void>;
  { v.send_out_sysex(umpData{}) } -> std::same_as<void>;

  { v.flex_tempo(std::uint8_t{}, std::uint32_t{}) } -> std::same_as<void>;
  {
    v.flex_time_sig(std::uint8_t{}, std::uint8_t{}, std::uint8_t{},
                    std::uint8_t{})
  } -> std::same_as<void>;
  {
    v.flex_metronome(std::uint8_t{}, std::uint8_t{}, std::uint8_t{},
                     std::uint8_t{}, std::uint8_t{}, std::uint8_t{},
                     std::uint8_t{})
  } -> std::same_as<void>;
  {
    v.flex_key_sig(std::uint8_t{}, std::uint8_t{}, std::uint8_t{},
                   std::uint8_t{}, std::uint8_t{})
  } -> std::same_as<void>;
  {
    v.flex_chord(std::uint8_t{}, std::uint8_t{}, std::uint8_t{}, chord{})
  } -> std::same_as<void>;
  {
    v.flex_performance(umpData{}, std::uint8_t{}, std::uint8_t{})
  } -> std::same_as<void>;
  {
    v.flex_lyric(umpData{}, std::uint8_t{}, std::uint8_t{})
  } -> std::same_as<void>;

  {
    v.midiEndpoint(std::uint8_t{}, std::uint8_t{}, std::uint8_t{})
  } -> std::same_as<void>;
  { v.midiEndpointName(umpData{}) } -> std::same_as<void>;
  { v.midiEndpointProcId(umpData{}) } -> std::same_as<void>;
  {
    v.midiEndpointJRProtocolReq(std::uint8_t{}, bool{}, bool{})
  } -> std::same_as<void>;
  {
    v.midiEndpointInfo(std::uint8_t{}, std::uint8_t{}, std::uint8_t{}, bool{},
                       bool{}, bool{}, bool{})
  } -> std::same_as<void>;
  {
    v.midiEndpointDeviceInfo(
        std::array<std::uint8_t, 3>{}, std::array<std::uint8_t, 2>{},
        std::array<std::uint8_t, 2>{}, std::array<std::uint8_t, 4>{})
  } -> std::same_as<void>;
  {
    v.midiEndpointJRProtocolNotify(std::uint8_t{}, bool{}, bool{})
  } -> std::same_as<void>;

  { v.functionBlock(std::uint8_t{}, std::uint8_t{}) } -> std::same_as<void>;
  {
    v.functionBlockInfo(std::uint8_t{}, bool{}, std::uint8_t{}, bool{}, bool{},
                        std::uint8_t{}, std::uint8_t{}, std::uint8_t{},
                        std::uint8_t{}, std::uint8_t{})
  } -> std::same_as<void>;
  { v.functionBlockName(umpData{}, std::uint8_t{}) } -> std::same_as<void>;

  { v.startOfSeq() } -> std::same_as<void>;
  { v.endOfFile() } -> std::same_as<void>;

  { v.unknownUMPMessage(uint32_ptr{}, std::uint8_t{}) } -> std::same_as<void>;
};

class callbacks_base {
public:
  virtual ~callbacks_base() = default;

  //-----------------------Handlers ---------------------------
  virtual void utility_message(umpGeneric const& /*mess*/) {}
  virtual void channel_voice_message(umpCVM const& /*mess*/) {}
  virtual void system_message(umpGeneric const& /*mess*/) {}
  virtual void send_out_sysex(umpData const& /*mess*/) {}

  //---------- Flex Data
  virtual void flex_tempo(uint8_t /*group*/, uint32_t /*num10nsPQN*/) {}
  virtual void flex_time_sig(uint8_t /*group*/, uint8_t /*numerator*/,
                             uint8_t /*denominator*/, uint8_t /*num32Notes*/) {}
  virtual void flex_metronome(uint8_t /*group*/, uint8_t /*numClkpPriCli*/,
                              uint8_t /*bAccP1*/, uint8_t /*bAccP2*/,
                              uint8_t /*bAccP3*/, uint8_t /*numSubDivCli1*/,
                              uint8_t /*numSubDivCli2*/) {}
  virtual void flex_key_sig(uint8_t /*group*/, uint8_t /*addrs*/,
                            uint8_t /*channel*/, uint8_t /*sharpFlats*/,
                            uint8_t /*tonic*/) {}
  virtual void flex_chord(uint8_t /*group*/, uint8_t /*addrs*/,
                          uint8_t /*channel*/, chord const& /*chord*/) {}
  virtual void flex_performance(umpData const& /*mess*/, uint8_t /*addrs*/,
                                uint8_t /*channel*/) {}
  virtual void flex_lyric(umpData const& /*mess*/, uint8_t /*addrs*/,
                          uint8_t /*channel*/) {}

  //---------- UMP Stream
  virtual void midiEndpoint(uint8_t /*majVer*/, uint8_t /*minVer*/,
                            uint8_t /*filter*/) {}
  virtual void midiEndpointName(umpData const& /*mess*/) {}
  virtual void midiEndpointProcId(umpData const& /*mess*/) {}
  virtual void midiEndpointJRProtocolReq(uint8_t /*protocol*/, bool /*jrrx*/,
                                         bool /*jrtx*/) {}
  virtual void midiEndpointInfo(uint8_t /*majVer*/, uint8_t /*minVer*/,
                                uint8_t /*numOfFuncBlocks*/, bool /*m2*/,
                                bool /*m1*/, bool /*rxjr*/, bool /*txjr*/) {}
  virtual void midiEndpointDeviceInfo(
      std::array<uint8_t, 3> const& /*manuId*/,
      std::array<uint8_t, 2> const& /*familyId*/,
      std::array<uint8_t, 2> const& /*modelId*/,
      std::array<uint8_t, 4> const& /*version*/) {}
  virtual void midiEndpointJRProtocolNotify(uint8_t /*protocol*/, bool /*jrrx*/,
                                            bool /*jrtx*/) {}

  virtual void functionBlock(uint8_t /*fbIdx*/, uint8_t /*filter*/) {}
  virtual void functionBlockInfo(uint8_t /*fbIdx*/, bool /*active*/,
                                 uint8_t /*direction*/, bool /*sender*/,
                                 bool /*recv*/, uint8_t /*firstGroup*/,
                                 uint8_t /*groupLength*/,
                                 uint8_t /*midiCIVersion*/, uint8_t /*isMIDI1*/,
                                 uint8_t /*maxS8Streams*/) {}
  virtual void functionBlockName(umpData /*mess*/, uint8_t /*fbIdx*/) {}

  virtual void startOfSeq() {}
  virtual void endOfFile() {}

  virtual void unknownUMPMessage(uint32_t* /*ump*/, uint8_t /*length*/) {}
};

template <typename Callbacks = callbacks_base>
requires backend<Callbacks> class umpProcessor {
public:
  umpProcessor(Callbacks cb = Callbacks{}) : callbacks_{std::move(cb)} {}

  void clearUMP();
  void processUMP(uint32_t UMP);

private:
  void utility_message(ump_message_type mt);
  void system_message(ump_message_type mt, std::uint8_t group);
  void m1cvm_message(ump_message_type mt, std::uint8_t group);
  void sysex7_message(ump_message_type mt, std::uint8_t group);
  void m2cvm_message(ump_message_type mt, std::uint8_t group);
  void midi_endpoint_message(ump_message_type mt);
  void data_message(ump_message_type mt, std::uint8_t group);
  void flexdata_message(ump_message_type mt, std::uint8_t group);

  std::array<std::uint32_t, 4> umpMess;
  std::uint8_t messPos = 0;

  Callbacks callbacks_;
};

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::clearUMP() {
  messPos = 0;
  umpMess[0] = 0;
  umpMess[1] = 0;
  umpMess[2] = 0;
  umpMess[3] = 0;
}

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::utility_message(
    ump_message_type const mt) {
  // 32 bit utility messages
  umpGeneric mess;
  mess.common.messageType = mt;
  mess.common.status = static_cast<std::uint8_t>((umpMess[0] >> 20) & 0x0F);
  mess.value = static_cast<std::uint16_t>((umpMess[0] >> 16) & 0xFFFF);
  callbacks_.utilityMessage(mess);
}

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::system_message(
    ump_message_type const mt, std::uint8_t const group) {
  // 32 bit System Real Time and System Common Messages (except System
  // Exclusive)
  umpGeneric mess;
  mess.common.messageType = mt;
  mess.common.group = group;
  mess.common.status = static_cast<std::uint8_t>((umpMess[0] >> 16) & 0xFF);
  switch (mess.common.status) {
  case status::timing_code:
  case status::song_select:
    mess.value = (umpMess[0] >> 8) & 0x7F;
    callbacks_.systemMessage(mess);
    break;
  case status::spp:
    mess.value = static_cast<std::uint16_t>(((umpMess[0] >> 8) & 0x7F) |
                                            ((umpMess[0] & 0x7F) << 7));
    callbacks_.systemMessage(mess);
    break;
  default: callbacks_.systemMessage(mess); break;
  }
}

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::m1cvm_message(
    ump_message_type const mt, std::uint8_t const group) {
  // 32 Bit MIDI 1.0 Channel Voice Messages
  umpCVM mess;
  mess.common.group = group;
  mess.common.messageType = mt;
  mess.common.status = umpMess[0] >> 16 & 0xF0;
  mess.channel = (umpMess[0] >> 16) & 0xF;

  std::uint8_t const val1 = (umpMess[0] >> 8) & 0x7F;
  std::uint8_t const val2 = umpMess[0] & 0x7F;

  switch (mess.common.status) {
  case status::note_off:
  case status::note_on:
  case status::key_pressure:
    mess.note = val1;
    mess.value = M2Utils::scaleUp(val2, 7, 16);
    callbacks_.channelVoiceMessage(mess);
    break;
  case status::channel_pressure:
    mess.value = M2Utils::scaleUp(val2, 7, 32);
    callbacks_.channelVoiceMessage(mess);
    break;
  case status::cc:
    mess.index = val1;
    mess.value = M2Utils::scaleUp(val2, 7, 32);
    callbacks_.channelVoiceMessage(mess);
    break;
  case status::program_change:
    mess.value = val1;
    callbacks_.channelVoiceMessage(mess);
    break;
  case status::pitch_bend:
    mess.value = M2Utils::scaleUp((std::uint32_t{val2} << 7) | val1, 14, 32);
    callbacks_.channelVoiceMessage(mess);
    break;
  default: callbacks_.unknownUMPMessage(umpMess, 2); break;
  }
}

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::sysex7_message(
    ump_message_type const mt, std::uint8_t const group) {
  // 64 bits Data Messages (including System Exclusive)
  std::array<std::uint8_t, 6> sysex{};
  umpData mess;
  mess.common.group = group;
  mess.common.messageType = mt;
  mess.form = (umpMess[0] >> 20) & 0xF;
  mess.data = sysex.data();
  mess.dataLength = (umpMess[0] >> 16) & 0xF;
  assert(mess.dataLength <= sysex.size());

  if (mess.dataLength > 0) {
    sysex[0] = (umpMess[0] >> 8) & 0x7F;
  }
  if (mess.dataLength > 1) {
    sysex[1] = umpMess[0] & 0x7F;
  }
  if (mess.dataLength > 2) {
    sysex[2] = (umpMess[1] >> 24) & 0x7F;
  }
  if (mess.dataLength > 3) {
    sysex[3] = (umpMess[1] >> 16) & 0x7F;
  }
  if (mess.dataLength > 4) {
    sysex[4] = (umpMess[1] >> 8) & 0x7F;
  }
  if (mess.dataLength > 5) {
    sysex[5] = umpMess[1] & 0x7F;
  }

  callbacks_.sendOutSysex(mess);
}

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::m2cvm_message(
    ump_message_type const mt, std::uint8_t const group) {
  // 64 bits MIDI 2.0 Channel Voice Messages
  umpCVM mess;
  mess.common.group = group;
  mess.common.messageType = mt;
  mess.common.status = (umpMess[0] >> 16) & 0xF0;
  mess.channel = (umpMess[0] >> 16) & 0xF;
  uint8_t val1 = (umpMess[0] >> 8) & 0xFF;
  uint8_t val2 = umpMess[0] & 0xFF;

  switch (mess.common.status) {
  case status::note_off:  // Note Off
  case status::note_on:   // Note On
    mess.note = val1;
    mess.value = umpMess[1] >> 16;
    mess.bank = val2;
    mess.index = umpMess[1] & 65535;
    callbacks_.channelVoiceMessage(mess);
    break;
  case midi2status::pitch_bend_pernote:
  case status::key_pressure:  // Poly Pressure
    mess.note = val1;
    mess.value = umpMess[1];
    callbacks_.channelVoiceMessage(mess);
    break;
  case status::channel_pressure:  // Channel Pressure
    mess.value = umpMess[1];
    callbacks_.channelVoiceMessage(mess);
    break;
  case status::cc:
    mess.index = val1;
    mess.value = umpMess[1];
    callbacks_.channelVoiceMessage(mess);
    break;

  case midi2status::rpn:            // RPN
  case midi2status::nrpn:           // NRPN
  case midi2status::rpn_relative:   // Relative RPN
  case midi2status::nrpn_relative:  // Relative NRPN
    mess.bank = val1;
    mess.index = val2;
    mess.value = umpMess[1];
    callbacks_.channelVoiceMessage(mess);
    break;

  case status::program_change:  // Program Change Message
    mess.value = umpMess[1] >> 24;
    mess.flag1 = umpMess[0] & 1;
    mess.bank = (umpMess[1] >> 8) & 0x7f;
    mess.index = umpMess[1] & 0x7f;
    callbacks_.channelVoiceMessage(mess);
    break;

  case status::pitch_bend:  // PitchBend
    mess.value = umpMess[1];
    callbacks_.channelVoiceMessage(mess);
    break;

  case midi2status::nrpn_pernote:  // Assignable Per-Note Controller 1
  case midi2status::rpn_pernote:   // Registered Per-Note Controller 0
    mess.note = val1;
    mess.index = val2;
    mess.value = umpMess[1];
    callbacks_.channelVoiceMessage(mess);
    break;
  case midi2status::pernote_manage:  // Per-Note Management Message
    mess.note = val1;
    mess.flag1 = (val2 & 2) != 0;
    mess.flag2 = (val2 & 1) != 0;
    callbacks_.channelVoiceMessage(mess);
    break;
  default: callbacks_.unknownUMPMessage(umpMess, 2); break;
  }
}

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::midi_endpoint_message(
    ump_message_type const mt) {
  // 128 bits UMP Stream Messages
  uint16_t status = (umpMess[0] >> 16) & 0x3FF;
  switch (status) {
  case MIDIENDPOINT:
    callbacks_.midiEndpoint((umpMess[0] >> 8) & 0xFF,  // Maj Ver
                            umpMess[0] & 0xFF,         // Min Ver
                            umpMess[1] & 0xFF);        // Filter
    break;

  case MIDIENDPOINT_INFO_NOTIFICATION:
    callbacks_.midiEndpointInfo((umpMess[0] >> 8) & 0xFF,   // Maj Ver
                                umpMess[0] & 0xFF,          // Min Ver
                                (umpMess[1] >> 24) & 0xFF,  // Num Of Func Block
                                ((umpMess[1] >> 9) & 0x1),  // M2 Support
                                ((umpMess[1] >> 8) & 0x1),  // M1 Support
                                ((umpMess[1] >> 1) & 0x1),  // rxjr Support
                                (umpMess[1] & 0x1)          // txjr Support
    );
    break;

  case MIDIENDPOINT_DEVICEINFO_NOTIFICATION:
    callbacks_.midiEndpointDeviceInfo(
        {static_cast<std::uint8_t>((umpMess[1] >> 16) & 0x7F),
         static_cast<std::uint8_t>((umpMess[1] >> 8) & 0x7F),
         static_cast<std::uint8_t>(umpMess[1] & 0x7F)},
        {static_cast<std::uint8_t>((umpMess[2] >> 24) & 0x7F),
         static_cast<std::uint8_t>((umpMess[2] >> 16) & 0x7F)},
        {static_cast<std::uint8_t>((umpMess[2] >> 8) & 0x7F),
         static_cast<std::uint8_t>(umpMess[2] & 0x7F)},
        {static_cast<std::uint8_t>((umpMess[3] >> 24) & 0x7F),
         static_cast<std::uint8_t>((umpMess[3] >> 16) & 0x7F),
         static_cast<std::uint8_t>((umpMess[3] >> 8) & 0x7F),
         static_cast<std::uint8_t>(umpMess[3] & 0x7F)});
    break;
  case MIDIENDPOINT_NAME_NOTIFICATION:
  case MIDIENDPOINT_PRODID_NOTIFICATION: {
    std::array<std::uint8_t, 14> text;
    umpData mess;
    mess.common.messageType = mt;
    mess.common.status = static_cast<std::uint8_t>(status);
    mess.form = umpMess[0] >> 24 & 0x3;
    mess.data = text.data();
    mess.dataLength = 0;

    if ((umpMess[0] >> 8) & 0xFF) {
      text[mess.dataLength++] = (umpMess[0] >> 8) & 0xFF;
    }
    if (umpMess[0] & 0xFF) {
      text[mess.dataLength++] = umpMess[0] & 0xFF;
    }
    for (uint8_t i = 1; i <= 3; i++) {
      for (int j = 24; j >= 0; j -= 8) {
        if (uint8_t c = (umpMess[i] >> j) & 0xFF) {
          text[mess.dataLength++] = c;
        }
      }
    }
    assert(mess.dataLength <= text.size());
    if (status == MIDIENDPOINT_NAME_NOTIFICATION) {
      callbacks_.midiEndpointName(mess);
    } else {
      assert(status == MIDIENDPOINT_PRODID_NOTIFICATION);
      callbacks_.midiEndpointProdId(mess);
    }
    break;
  }

  case MIDIENDPOINT_PROTOCOL_REQUEST:  // JR Protocol Req
    callbacks_.midiEndpointJRProtocolReq(
        static_cast<std::uint8_t>(umpMess[0] >> 8), (umpMess[0] >> 1) & 1,
        umpMess[0] & 1);
    break;
  case MIDIENDPOINT_PROTOCOL_NOTIFICATION:  // JR Protocol Req
    callbacks_.midiEndpointJRProtocolNotify(
        static_cast<std::uint8_t>(umpMess[0] >> 8), (umpMess[0] >> 1) & 1,
        umpMess[0] & 1);
    break;

  case FUNCTIONBLOCK:
    callbacks_.functionBlock((umpMess[0] >> 8) & 0xFF,  // fbIdx
                             umpMess[0] & 0xFF          // filter
    );
    break;

  case FUNCTIONBLOCK_INFO_NOTFICATION:
    callbacks_.functionBlockInfo((umpMess[0] >> 8) & 0x7F,     // fbIdx
                                 (umpMess[0] >> 15) & 0x1,     // active
                                 umpMess[0] & 0x3,             // dir
                                 (umpMess[0] >> 7) & 0x1,      // Sender
                                 (umpMess[0] >> 6) & 0x1,      // Receiver
                                 ((umpMess[1] >> 24) & 0x1F),  // first group
                                 ((umpMess[1] >> 16) & 0x1F),  // group length
                                 ((umpMess[1] >> 8) & 0x7F),   // midiCIVersion
                                 ((umpMess[0] >> 2) & 0x3),    // isMIDI 1
                                 (umpMess[1] & 0xFF)           // max Streams
    );
    break;
  case FUNCTIONBLOCK_NAME_NOTIFICATION: {
    uint8_t fbIdx = (umpMess[0] >> 8) & 0x7F;
    std::array<std::uint8_t, 13> text;
    umpData mess;
    mess.common.messageType = mt;
    mess.common.status = static_cast<std::uint8_t>(status);
    mess.form = umpMess[0] >> 24 & 0x3;
    mess.data = text.data();
    mess.dataLength = 0;

    if (umpMess[0] & 0xFF) {
      text[mess.dataLength++] = umpMess[0] & 0xFF;
    }
    for (uint8_t i = 1; i <= 3; i++) {
      for (int j = 24; j >= 0; j -= 8) {
        if (uint8_t c = (umpMess[i] >> j) & 0xFF) {
          text[mess.dataLength++] = c;
        }
      }
    }
    assert(mess.dataLength <= text.size());
    callbacks_.functionBlockName(mess, fbIdx);
    break;
  }
  case STARTOFSEQ: callbacks_.startOfSeq(); break;
  case ENDOFFILE: callbacks_.endOfFile(); break;
  default: callbacks_.unknownUMPMessage(umpMess, 4); break;
  }
}

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::data_message(
    ump_message_type const mt, std::uint8_t const group) {
  // 128 bits Data Messages (including System Exclusive 8)
  uint8_t status = (umpMess[0] >> 20) & 0xF;
  if (status <= 3) {
    std::array<std::uint8_t, 13> sysex;
    umpData mess;
    mess.common.group = group;
    mess.common.messageType = mt;
    mess.streamId = (umpMess[0] >> 8) & 0xFF;
    mess.form = status;
    mess.data = sysex.data();
    mess.dataLength = (umpMess[0] >> 16) & 0xF;
    assert(mess.dataLength <= sysex.size());

    if (mess.dataLength > 1) {
      sysex[0] = umpMess[0] & 0x7F;
    }
    if (mess.dataLength > 2) {
      sysex[1] = (umpMess[1] >> 24) & 0x7F;
    }
    if (mess.dataLength > 3) {
      sysex[2] = (umpMess[1] >> 16) & 0x7F;
    }
    if (mess.dataLength > 4) {
      sysex[3] = (umpMess[1] >> 8) & 0x7F;
    }
    if (mess.dataLength > 5) {
      sysex[4] = umpMess[1] & 0x7F;
    }
    if (mess.dataLength > 6) {
      sysex[5] = (umpMess[2] >> 24) & 0x7F;
    }
    if (mess.dataLength > 7) {
      sysex[6] = (umpMess[2] >> 16) & 0x7F;
    }
    if (mess.dataLength > 8) {
      sysex[7] = (umpMess[2] >> 8) & 0x7F;
    }
    if (mess.dataLength > 9) {
      sysex[8] = umpMess[2] & 0x7F;
    }
    if (mess.dataLength > 10) {
      sysex[9] = (umpMess[3] >> 24) & 0x7F;
    }
    if (mess.dataLength > 11) {
      sysex[10] = (umpMess[3] >> 16) & 0x7F;
    }
    if (mess.dataLength > 12) {
      sysex[11] = (umpMess[3] >> 8) & 0x7F;
    }
    if (mess.dataLength > 13) {
      sysex[12] = umpMess[3] & 0x7F;
    }
    callbacks_.sendOutSysex(mess);
  } else if (status == 8 || status == 9) {
    // Beginning of Mixed Data Set
    // uint8_t mdsId  = (umpMess[0] >> 16) & 0xF;

    if (status == 8) {
      /*uint16_t numValidBytes  = umpMess[0] & 0xFFFF;
      uint16_t numChunk  = (umpMess[1] >> 16) & 0xFFFF;
      uint16_t numOfChunk  = umpMess[1] & 0xFFFF;
      uint16_t manuId  = (umpMess[2] >> 16) & 0xFFFF;
      uint16_t deviceId  = umpMess[2] & 0xFFFF;
      uint16_t subId1  = (umpMess[3] >> 16) & 0xFFFF;
      uint16_t subId2  = umpMess[3] & 0xFFFF;*/
    } else {
      // MDS bytes?
    }
    callbacks_.unknownUMPMessage(umpMess, 4);
  }
}

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::flexdata_message(
    ump_message_type const mt, std::uint8_t const group) {
  // 128 bit Data Messages (including System Exclusive 8)
  uint8_t statusBank = (umpMess[0] >> 8) & 0xFF;
  uint8_t status = umpMess[0] & 0xFF;
  uint8_t channel = (umpMess[0] >> 16) & 0xF;
  uint8_t addrs = (umpMess[0] >> 18) & 3;
  uint8_t form = (umpMess[0] >> 20) & 3;
  // SysEx 8
  switch (statusBank) {
  case FLEXDATA_COMMON: {  // Common/Configuration for MIDI File, Project,
                           // and Track
    switch (status) {
    case FLEXDATA_COMMON_TEMPO: {  // Set Tempo Message
      callbacks_.flexTempo(group, umpMess[1]);
      break;
    }
    case FLEXDATA_COMMON_TIMESIG: {  // Set Time Signature Message
      callbacks_.flexTimeSig(group, (umpMess[1] >> 24) & 0xFF,
                             (umpMess[1] >> 16) & 0xFF,
                             (umpMess[1] >> 8) & 0xFF);
      break;
    }
    case FLEXDATA_COMMON_METRONOME: {  // Set Metronome Message
      callbacks_.flexMetronome(
          group, (umpMess[1] >> 24) & 0xFF, (umpMess[1] >> 16) & 0xFF,
          (umpMess[1] >> 8) & 0xFF, umpMess[1] & 0xFF,
          (umpMess[2] >> 24) & 0xFF, (umpMess[2] >> 16) & 0xFF);
      break;
    }
    case FLEXDATA_COMMON_KEYSIG: {  // Set Key Signature Message
      callbacks_.flexKeySig(group, addrs, channel, (umpMess[1] >> 24) & 0xFF,
                            (umpMess[1] >> 16) & 0xFF);
      break;
    }
    case FLEXDATA_COMMON_CHORD: {  // Set Chord Message
      chord c;
      c.chShrpFlt = (umpMess[1] >> 28) & 0xF;
      c.chTonic = (umpMess[1] >> 24) & 0x0F;
      c.chType = (umpMess[1] >> 16) & 0xFF;
      c.chAlt1Type = (umpMess[1] >> 12) & 0x0F;
      c.chAlt1Deg = (umpMess[1] >> 8) & 0x0F;
      c.chAlt2Type = (umpMess[1] >> 4) & 0x0F;
      c.chAlt2Deg = umpMess[1] & 0x0F;
      c.chAlt3Type = (umpMess[2] >> 28) & 0x0F;
      c.chAlt3Deg = (umpMess[2] >> 24) & 0x0F;
      c.chAlt4Type = (umpMess[2] >> 20) & 0x0F;
      c.chAlt4Deg = (umpMess[2] >> 16) & 0x0F;
      c.baShrpFlt = (umpMess[3] >> 28) & 0x0F;
      c.baTonic = (umpMess[3] >> 24) & 0x0F;
      c.baType = (umpMess[3] >> 16) & 0xFF;
      c.baAlt1Type = (umpMess[3] >> 12) & 0x0F;
      c.baAlt1Deg = (umpMess[3] >> 8) & 0x0F;
      c.baAlt2Type = (umpMess[3] >> 4) & 0x0F;
      c.baAlt2Deg = umpMess[1] & 0xF;
      callbacks_.flexChord(group, addrs, channel, c);
      break;
    }
    default: callbacks_.unknownUMPMessage(umpMess, 4); break;
    }
    break;
  }
  case FLEXDATA_PERFORMANCE:  // Performance Events
  case FLEXDATA_LYRIC: {      // Lyric Events
    std::array<std::uint8_t, 12> text;
    umpData mess;
    mess.common.group = group;
    mess.common.messageType = mt;
    mess.common.status = status;
    mess.form = form;
    mess.data = text.data();
    mess.dataLength = 0;
    for (uint8_t i = 1; i <= 3; i++) {
      for (int j = 24; j >= 0; j -= 8) {
        if (uint8_t const c = (umpMess[i] >> j) & 0xFF) {
          text[mess.dataLength++] = c;
        }
      }
    }
    assert(mess.dataLength <= text.size());
    if (statusBank == FLEXDATA_LYRIC) {
      callbacks_.flexLyric(mess, addrs, channel);
    } else {
      assert(statusBank == FLEXDATA_PERFORMANCE);
      callbacks_.flexPerformance(mess, addrs, channel);
    }
    break;
  }
  default: callbacks_.unknownUMPMessage(umpMess, 4); break;
  }
}

template <typename Callbacks>
requires backend<Callbacks> void umpProcessor<Callbacks>::processUMP(
    uint32_t UMP) {
  umpMess[messPos] = UMP;

  auto mt = static_cast<ump_message_type>((umpMess[0] >> 28) & 0xF);
  uint8_t group = (umpMess[0] >> 24) & 0xF;

  if (messPos == 0 &&
      (mt == ump_message_type::utility || mt == ump_message_type::system ||
       mt == ump_message_type::m1cvm || mt == ump_message_type::reserved32_06 ||
       mt == ump_message_type::reserved32_07)) {
    // 32bit Messages
    if (mt == ump_message_type::utility) {
      this->utility_message(mt);
    } else if (mt == ump_message_type::system) {
      this->system_message(mt, group);
    } else if (mt == ump_message_type::m1cvm) {
      this->m1cvm_message(mt, group);
    }
    return;

  } else if (messPos == 1 &&
             (mt == ump_message_type::sysex7 || mt == ump_message_type::m2cvm ||
              mt == ump_message_type::reserved64_08 ||
              mt == ump_message_type::reserved64_09 ||
              mt == ump_message_type::reserved64_0A)) {  // 64bit Messages
    if (mt == ump_message_type::sysex7) {
      this->sysex7_message(mt, group);
    } else if (mt == ump_message_type::m2cvm) {
      this->m2cvm_message(mt, group);
    }
    messPos = 0;
  } else if (messPos == 2 &&
             (mt == ump_message_type::reserved96_0B ||
              mt == ump_message_type::reserved96_0C)) {  // 96bit Messages
    messPos = 0;
    callbacks_.unknownUMPMessage(umpMess, 3);
  } else if (messPos == 3 &&
             (mt == ump_message_type::data ||
              mt == ump_message_type::flex_data ||
              mt == ump_message_type::reserved128_0E ||
              mt == ump_message_type::midi_endpoint)) {  // 128bit Messages

    if (mt == ump_message_type::midi_endpoint) {
      this->midi_endpoint_message(mt);
    } else if (mt == ump_message_type::data) {
      this->data_message(mt, group);
    } else if (mt == ump_message_type::flex_data) {
      this->flexdata_message(mt, group);
    } else {
      callbacks_.unknownUMPMessage(umpMess, 4);
    }
    messPos = 0;
  } else {
    messPos++;
  }
}

#endif  // UMP_PROCESSOR_H
