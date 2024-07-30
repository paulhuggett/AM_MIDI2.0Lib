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
#ifndef MIDI2_UMP_PROCESSOR_H
#define MIDI2_UMP_PROCESSOR_H

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <functional>
#include <span>

#include "midi2/utils.h"

namespace midi2 {

struct umpCommon {
  bool operator==(umpCommon const&) const = default;
  bool operator!=(umpCommon const&) const = default;

  uint8_t group = 255;
  ump_message_type messageType = ump_message_type::utility;
  uint8_t status = 0;
};
struct umpCVM {
  bool operator==(umpCVM const&) const = default;
  bool operator!=(umpCVM const&) const = default;

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
  std::span<std::uint8_t> data;
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
  { v.midiEndpointProdId(umpData{}) } -> std::same_as<void>;
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

  { v.unknownUMPMessage(std::span<std::uint32_t>{}) } -> std::same_as<void>;
};

class callbacks_base {
public:
  callbacks_base() = default;
  callbacks_base(callbacks_base const&) = default;
  virtual ~callbacks_base() = default;

  callbacks_base& operator=(callbacks_base const&) = default;

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
  virtual void midiEndpointProdId(umpData const& /*mess*/) {}
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

  virtual void unknownUMPMessage(std::span<std::uint32_t>) {}
};

template <backend Callbacks = callbacks_base> class umpProcessor {
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

  enum data_message_status : std::uint8_t {
    sysex8_in_1_ump = 0b0000,
    sysex8_start = 0b0001,
    sysex8_continue = 0b0010,
    sysex8_end = 0b0011,
    mixed_data_set_header = 0b1000,
    mixed_data_set_payload = 0b1001,
  };

  template <std::output_iterator<std::uint8_t> OutputIterator>
  static constexpr OutputIterator sysex8_payload(
      std::array<std::uint32_t, 4> const& message, std::size_t index,
      std::size_t limit, OutputIterator out);

  std::array<std::uint32_t, 4> message_{};
  std::uint8_t pos_ = 0;

  Callbacks callbacks_;
};

umpProcessor() -> umpProcessor<callbacks_base>;
template <typename T> umpProcessor(T) -> umpProcessor<T>;

template <backend Callbacks> void umpProcessor<Callbacks>::clearUMP() {
  pos_ = 0;
  std::fill(std::begin(message_), std::end(message_), std::uint8_t{0});
}

template <backend Callbacks>
void umpProcessor<Callbacks>::utility_message(ump_message_type const mt) {
  // 32 bit utility messages
  umpGeneric mess;
  mess.common.messageType = mt;
  mess.common.status = static_cast<std::uint8_t>((message_[0] >> 20) & 0x0F);
  mess.value = static_cast<std::uint16_t>((message_[0] >> 16) & 0xFFFF);
  callbacks_.utility_message(mess);
}

template <backend Callbacks>
void umpProcessor<Callbacks>::system_message(ump_message_type const mt,
                                             std::uint8_t const group) {
  // 32 bit System Real Time and System Common Messages (except System
  // Exclusive)
  umpGeneric mess;
  mess.common.messageType = mt;
  mess.common.group = group;
  mess.common.status = static_cast<std::uint8_t>((message_[0] >> 16) & 0xFF);
  switch (mess.common.status) {
  case status::timing_code:
  case status::song_select:
    mess.value = (message_[0] >> 8) & 0x7F;
    callbacks_.system_message(mess);
    break;
  case status::spp:
    mess.value = static_cast<std::uint16_t>(((message_[0] >> 8) & 0x7F) |
                                            ((message_[0] & 0x7F) << 7));
    callbacks_.system_message(mess);
    break;
  default: callbacks_.system_message(mess); break;
  }
}

template <backend Callbacks>
void umpProcessor<Callbacks>::m1cvm_message(ump_message_type const mt,
                                            std::uint8_t const group) {
  // 32 Bit MIDI 1.0 Channel Voice Messages
  umpCVM mess;
  mess.common.group = group;
  mess.common.messageType = mt;
  mess.common.status = message_[0] >> 16 & 0xF0;
  mess.channel = (message_[0] >> 16) & 0xF;

  std::uint8_t const val1 = (message_[0] >> 8) & 0x7F;
  std::uint8_t const val2 = message_[0] & 0x7F;

  switch (mess.common.status) {
  case status::note_off:
  case status::note_on:
  case status::key_pressure:
    mess.note = val1;
    mess.value = scaleUp(val2, 7, 16);
    callbacks_.channel_voice_message(mess);
    break;
  case status::channel_pressure:
    mess.value = scaleUp(val2, 7, 32);
    callbacks_.channel_voice_message(mess);
    break;
  case status::cc:
    mess.index = val1;
    mess.value = scaleUp(val2, 7, 32);
    callbacks_.channel_voice_message(mess);
    break;
  case status::program_change:
    mess.value = val1;
    callbacks_.channel_voice_message(mess);
    break;
  case status::pitch_bend:
    mess.value = scaleUp((std::uint32_t{val2} << 7) | val1, 14, 32);
    callbacks_.channel_voice_message(mess);
    break;
  default: callbacks_.unknownUMPMessage(std::span{message_.data(), 2}); break;
  }
}

template <backend Callbacks>
void umpProcessor<Callbacks>::sysex7_message(ump_message_type const mt,
                                             std::uint8_t const group) {
  // 64 bit Data Messages (including System Exclusive)
  std::array<std::uint8_t, 7> sysex{};
  auto const data_length = (message_[0] >> 16) & 0x7;
  if (data_length > 0) {
    sysex[0] = (message_[0] >> 8) & 0x7F;
  }
  if (data_length > 1) {
    sysex[1] = message_[0] & 0x7F;
  }
  if (data_length > 2) {
    sysex[2] = (message_[1] >> 24) & 0x7F;
  }
  if (data_length > 3) {
    sysex[3] = (message_[1] >> 16) & 0x7F;
  }
  if (data_length > 4) {
    sysex[4] = (message_[1] >> 8) & 0x7F;
  }
  if (data_length > 5) {
    sysex[5] = message_[1] & 0x7F;
  }
  umpData mess;
  mess.common.group = group;
  mess.common.messageType = mt;
  mess.form = (message_[0] >> 20) & 0xF;
  mess.data = std::span{sysex.data(), data_length};
  assert(mess.data.size() <= sysex.size());
  callbacks_.send_out_sysex(mess);
}

template <backend Callbacks>
void umpProcessor<Callbacks>::m2cvm_message(ump_message_type const mt,
                                            std::uint8_t const group) {
  // 64 bits MIDI 2.0 Channel Voice Messages
  umpCVM mess;
  mess.common.group = group;
  mess.common.messageType = mt;
  mess.common.status = (message_[0] >> 16) & 0xF0;
  mess.channel = (message_[0] >> 16) & 0xF;
  uint8_t val1 = (message_[0] >> 8) & 0xFF;
  uint8_t val2 = message_[0] & 0xFF;

  switch (mess.common.status) {
  case status::note_off:  // Note Off
  case status::note_on:   // Note On
    mess.note = val1;
    mess.value = message_[1] >> 16;
    mess.bank = val2;
    mess.index = message_[1] & 65535;
    callbacks_.channel_voice_message(mess);
    break;
  case midi2status::pitch_bend_pernote:
  case status::key_pressure:  // Poly Pressure
    mess.note = val1;
    mess.value = message_[1];
    callbacks_.channel_voice_message(mess);
    break;
  case status::channel_pressure:  // Channel Pressure
    mess.value = message_[1];
    callbacks_.channel_voice_message(mess);
    break;
  case status::cc:
    mess.index = val1;
    mess.value = message_[1];
    callbacks_.channel_voice_message(mess);
    break;

  case midi2status::rpn:            // RPN
  case midi2status::nrpn:           // NRPN
  case midi2status::rpn_relative:   // Relative RPN
  case midi2status::nrpn_relative:  // Relative NRPN
    mess.bank = val1;
    mess.index = val2;
    mess.value = message_[1];
    callbacks_.channel_voice_message(mess);
    break;

  case status::program_change:  // Program Change Message
    mess.value = message_[1] >> 24;
    mess.flag1 = message_[0] & 1;
    mess.bank = (message_[1] >> 8) & 0x7f;
    mess.index = message_[1] & 0x7f;
    callbacks_.channel_voice_message(mess);
    break;

  case status::pitch_bend:  // PitchBend
    mess.value = message_[1];
    callbacks_.channel_voice_message(mess);
    break;

  case midi2status::nrpn_pernote:  // Assignable Per-Note Controller 1
  case midi2status::rpn_pernote:   // Registered Per-Note Controller 0
    mess.note = val1;
    mess.index = val2;
    mess.value = message_[1];
    callbacks_.channel_voice_message(mess);
    break;
  case midi2status::pernote_manage:  // Per-Note Management Message
    mess.note = val1;
    mess.flag1 = (val2 & 2) != 0;
    mess.flag2 = (val2 & 1) != 0;
    callbacks_.channel_voice_message(mess);
    break;
  default: callbacks_.unknownUMPMessage(std::span{message_.data(), 2}); break;
  }
}

template <backend Callbacks>
void umpProcessor<Callbacks>::midi_endpoint_message(ump_message_type const mt) {
  // 128 bits UMP Stream Messages
  uint16_t status = (message_[0] >> 16) & 0x3FF;
  switch (status) {
  case MIDIENDPOINT:
    callbacks_.midiEndpoint((message_[0] >> 8) & 0xFF,  // Maj Ver
                            message_[0] & 0xFF,         // Min Ver
                            message_[1] & 0xFF);        // Filter
    break;

  case MIDIENDPOINT_INFO_NOTIFICATION:
    callbacks_.midiEndpointInfo(
        (message_[0] >> 8) & 0xFF,   // Maj Ver
        message_[0] & 0xFF,          // Min Ver
        (message_[1] >> 24) & 0xFF,  // Num Of Func Block
        ((message_[1] >> 9) & 0x1),  // M2 Support
        ((message_[1] >> 8) & 0x1),  // M1 Support
        ((message_[1] >> 1) & 0x1),  // rxjr Support
        (message_[1] & 0x1)          // txjr Support
    );
    break;

  case MIDIENDPOINT_DEVICEINFO_NOTIFICATION:
    callbacks_.midiEndpointDeviceInfo(
        {static_cast<std::uint8_t>((message_[1] >> 16) & 0x7F),
         static_cast<std::uint8_t>((message_[1] >> 8) & 0x7F),
         static_cast<std::uint8_t>(message_[1] & 0x7F)},
        {static_cast<std::uint8_t>((message_[2] >> 24) & 0x7F),
         static_cast<std::uint8_t>((message_[2] >> 16) & 0x7F)},
        {static_cast<std::uint8_t>((message_[2] >> 8) & 0x7F),
         static_cast<std::uint8_t>(message_[2] & 0x7F)},
        {static_cast<std::uint8_t>((message_[3] >> 24) & 0x7F),
         static_cast<std::uint8_t>((message_[3] >> 16) & 0x7F),
         static_cast<std::uint8_t>((message_[3] >> 8) & 0x7F),
         static_cast<std::uint8_t>(message_[3] & 0x7F)});
    break;
  case MIDIENDPOINT_NAME_NOTIFICATION:
  case MIDIENDPOINT_PRODID_NOTIFICATION: {
    std::array<std::uint8_t, 14> text;
    auto text_length = 0U;

    if ((message_[0] >> 8) & 0xFF) {
      text[text_length++] = (message_[0] >> 8) & 0xFF;
    }
    if (message_[0] & 0xFF) {
      text[text_length++] = message_[0] & 0xFF;
    }
    for (uint8_t i = 1; i <= 3; i++) {
      for (int j = 24; j >= 0; j -= 8) {
        if (uint8_t c = (message_[i] >> j) & 0xFF) {
          text[text_length++] = c;
        }
      }
    }
    assert(text_length <= text.size());
    umpData mess;
    mess.common.messageType = mt;
    mess.common.status = static_cast<std::uint8_t>(status);
    mess.form = message_[0] >> 24 & 0x3;
    mess.data = std::span{text.data(), text_length};
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
        static_cast<std::uint8_t>(message_[0] >> 8), (message_[0] >> 1) & 1,
        message_[0] & 1);
    break;
  case MIDIENDPOINT_PROTOCOL_NOTIFICATION:  // JR Protocol Req
    callbacks_.midiEndpointJRProtocolNotify(
        static_cast<std::uint8_t>(message_[0] >> 8), (message_[0] >> 1) & 1,
        message_[0] & 1);
    break;

  case FUNCTIONBLOCK:
    callbacks_.functionBlock((message_[0] >> 8) & 0xFF,  // fbIdx
                             message_[0] & 0xFF          // filter
    );
    break;

  case FUNCTIONBLOCK_INFO_NOTFICATION:
    callbacks_.functionBlockInfo((message_[0] >> 8) & 0x7F,     // fbIdx
                                 (message_[0] >> 15) & 0x1,     // active
                                 message_[0] & 0x3,             // dir
                                 (message_[0] >> 7) & 0x1,      // Sender
                                 (message_[0] >> 6) & 0x1,      // Receiver
                                 ((message_[1] >> 24) & 0x1F),  // first group
                                 ((message_[1] >> 16) & 0x1F),  // group length
                                 ((message_[1] >> 8) & 0x7F),   // midiCIVersion
                                 ((message_[0] >> 2) & 0x3),    // isMIDI 1
                                 (message_[1] & 0xFF)           // max Streams
    );
    break;
  case FUNCTIONBLOCK_NAME_NOTIFICATION: {
    uint8_t fbIdx = (message_[0] >> 8) & 0x7F;
    std::array<std::uint8_t, 13> text;
    auto text_length = 0U;

    if (message_[0] & 0xFF) {
      text[text_length++] = message_[0] & 0xFF;
    }
    for (uint8_t i = 1; i <= 3; i++) {
      for (int j = 24; j >= 0; j -= 8) {
        if (uint8_t c = (message_[i] >> j) & 0xFF) {
          text[text_length++] = c;
        }
      }
    }
    assert(text_length <= text.size());
    umpData mess;
    mess.common.messageType = mt;
    mess.common.status = static_cast<std::uint8_t>(status);
    mess.form = message_[0] >> 24 & 0x3;
    mess.data = std::span{text.data(), text_length};
    callbacks_.functionBlockName(mess, fbIdx);
    break;
  }
  case STARTOFSEQ: callbacks_.startOfSeq(); break;
  case ENDOFFILE: callbacks_.endOfFile(); break;
  default: callbacks_.unknownUMPMessage(std::span{message_.data(), 4}); break;
  }
}

template <backend Callbacks>
template <std::output_iterator<std::uint8_t> OutputIterator>
constexpr OutputIterator umpProcessor<Callbacks>::sysex8_payload(
    std::array<std::uint32_t, 4> const& message, std::size_t index,
    std::size_t limit, OutputIterator out) {
  assert(limit < message.size() * sizeof(std::uint32_t) && index <= limit);
  if (index >= limit) {
    return out;
  }
  // There are 4 bytes per packet and we start at packet #1.
  auto const packet_num = (index >> 2) + 1U;
  auto const rem4 = index & 0b11U;  // rem4 = index % 4
  auto const shift = 24U - 8U * rem4;
  *(out++) = (message[packet_num] >> shift) & 0xFF;
  return sysex8_payload(message, index + 1U, limit, out);
}

template <backend Callbacks>
void umpProcessor<Callbacks>::data_message(ump_message_type const mt,
                                           std::uint8_t const group) {
  // 128 bit Data Messages (including System Exclusive 8)
  uint8_t const status = (message_[0] >> 20) & 0xF;
  switch (status) {
  case data_message_status::sysex8_in_1_ump:
  case data_message_status::sysex8_start:
  case data_message_status::sysex8_continue:
  case data_message_status::sysex8_end: {
    std::array<std::uint8_t, 13> sysex{};
    auto const data_length =
        std::min(std::size_t{(message_[0] >> 16) & 0x0F}, sysex.size());
    if (data_length >= 1) {
      sysex[0] = message_[0] & 0xFF;
      sysex8_payload(message_, 0, data_length - 1, std::begin(sysex) + 1);
    }
    umpData mess;
    mess.common.group = group;
    mess.common.messageType = mt;
    mess.streamId = (message_[0] >> 8) & 0xFF;
    mess.form = status;
    assert(data_length <= sysex.size());
    mess.data = std::span{sysex.data(), data_length};
    callbacks_.send_out_sysex(mess);
  } break;
  case data_message_status::mixed_data_set_header:
  case data_message_status::mixed_data_set_payload: {
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
    callbacks_.unknownUMPMessage(std::span{message_.data(), 4});
  } break;
  default: callbacks_.unknownUMPMessage(std::span{message_.data(), 4}); break;
  }
}

template <backend Callbacks>
void umpProcessor<Callbacks>::flexdata_message(ump_message_type const mt,
                                               std::uint8_t const group) {
  // 128 bit Data Messages (including System Exclusive 8)
  uint8_t statusBank = (message_[0] >> 8) & 0xFF;
  uint8_t status = message_[0] & 0xFF;
  uint8_t channel = (message_[0] >> 16) & 0xF;
  uint8_t addrs = (message_[0] >> 18) & 3;
  uint8_t form = (message_[0] >> 20) & 3;
  // SysEx 8
  switch (statusBank) {
  case FLEXDATA_COMMON: {  // Common/Configuration for MIDI File, Project,
                           // and Track
    switch (status) {
    case FLEXDATA_COMMON_TEMPO: {  // Set Tempo Message
      callbacks_.flex_tempo(group, message_[1]);
      break;
    }
    case FLEXDATA_COMMON_TIMESIG: {  // Set Time Signature Message
      callbacks_.flex_time_sig(group, (message_[1] >> 24) & 0xFF,
                               (message_[1] >> 16) & 0xFF,
                               (message_[1] >> 8) & 0xFF);
      break;
    }
    case FLEXDATA_COMMON_METRONOME: {  // Set Metronome Message
      callbacks_.flex_metronome(
          group, (message_[1] >> 24) & 0xFF, (message_[1] >> 16) & 0xFF,
          (message_[1] >> 8) & 0xFF, message_[1] & 0xFF,
          (message_[2] >> 24) & 0xFF, (message_[2] >> 16) & 0xFF);
      break;
    }
    case FLEXDATA_COMMON_KEYSIG: {  // Set Key Signature Message
      callbacks_.flex_key_sig(group, addrs, channel, (message_[1] >> 24) & 0xFF,
                              (message_[1] >> 16) & 0xFF);
      break;
    }
    case FLEXDATA_COMMON_CHORD: {  // Set Chord Message
      chord c;
      c.chShrpFlt = (message_[1] >> 28) & 0xF;
      c.chTonic = (message_[1] >> 24) & 0x0F;
      c.chType = (message_[1] >> 16) & 0xFF;
      c.chAlt1Type = (message_[1] >> 12) & 0x0F;
      c.chAlt1Deg = (message_[1] >> 8) & 0x0F;
      c.chAlt2Type = (message_[1] >> 4) & 0x0F;
      c.chAlt2Deg = message_[1] & 0x0F;
      c.chAlt3Type = (message_[2] >> 28) & 0x0F;
      c.chAlt3Deg = (message_[2] >> 24) & 0x0F;
      c.chAlt4Type = (message_[2] >> 20) & 0x0F;
      c.chAlt4Deg = (message_[2] >> 16) & 0x0F;
      c.baShrpFlt = (message_[3] >> 28) & 0x0F;
      c.baTonic = (message_[3] >> 24) & 0x0F;
      c.baType = (message_[3] >> 16) & 0xFF;
      c.baAlt1Type = (message_[3] >> 12) & 0x0F;
      c.baAlt1Deg = (message_[3] >> 8) & 0x0F;
      c.baAlt2Type = (message_[3] >> 4) & 0x0F;
      c.baAlt2Deg = message_[1] & 0xF;
      callbacks_.flex_chord(group, addrs, channel, c);
      break;
    }
    default: callbacks_.unknownUMPMessage(std::span{message_.data(), 4}); break;
    }
    break;
  }
  case FLEXDATA_PERFORMANCE:  // Performance Events
  case FLEXDATA_LYRIC: {      // Lyric Events
    std::array<std::uint8_t, 12> text;
    auto text_length = 0U;
    for (uint8_t i = 1; i <= 3; i++) {
      for (int j = 24; j >= 0; j -= 8) {
        if (uint8_t const c = (message_[i] >> j) & 0xFF) {
          text[text_length++] = c;
        }
      }
    }
    assert(text_length <= text.size());

    umpData mess;
    mess.common.group = group;
    mess.common.messageType = mt;
    mess.common.status = status;
    mess.form = form;
    mess.data = std::span{text.data(), text_length};
    if (statusBank == FLEXDATA_LYRIC) {
      callbacks_.flex_lyric(mess, addrs, channel);
    } else {
      assert(statusBank == FLEXDATA_PERFORMANCE);
      callbacks_.flex_performance(mess, addrs, channel);
    }
    break;
  }
  default: callbacks_.unknownUMPMessage(std::span{message_.data(), 4}); break;
  }
}

template <backend Callbacks>
void umpProcessor<Callbacks>::processUMP(uint32_t UMP) {
  message_[pos_] = UMP;

  auto mt = static_cast<ump_message_type>((message_[0] >> 28) & 0xF);
  uint8_t group = (message_[0] >> 24) & 0xF;

  if (pos_ == 0 &&
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

  } else if (pos_ == 1 &&
             (mt == ump_message_type::sysex7 || mt == ump_message_type::m2cvm ||
              mt == ump_message_type::reserved64_08 ||
              mt == ump_message_type::reserved64_09 ||
              mt == ump_message_type::reserved64_0A)) {  // 64bit Messages
    if (mt == ump_message_type::sysex7) {
      this->sysex7_message(mt, group);
    } else if (mt == ump_message_type::m2cvm) {
      this->m2cvm_message(mt, group);
    }
    pos_ = 0;
  } else if (pos_ == 2 &&
             (mt == ump_message_type::reserved96_0B ||
              mt == ump_message_type::reserved96_0C)) {  // 96bit Messages
    pos_ = 0;
    callbacks_.unknownUMPMessage(std::span{message_.data(), 3});
  } else if (pos_ == 3 &&
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
      callbacks_.unknownUMPMessage(std::span{message_.data(), 4});
    }
    pos_ = 0;
  } else {
    pos_++;
  }
}

}  // end namespace midi2

#endif  // MIDI2_UMP_PROCESSOR_H
