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
#include <bit>
#include <concepts>
#include <cstdint>
#include <functional>
#include <span>

#include "midi2/ump_types.h"
#include "midi2/utils.h"

namespace midi2 {

struct umpCommon {
  bool operator==(umpCommon const&) const = default;

  uint8_t group = 255;
  ump_message_type messageType = ump_message_type::utility;
  uint8_t status = 0;
};
struct umpCVM {
  bool operator==(umpCVM const&) const = default;

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
  bool operator==(umpGeneric const&) const = default;

  umpCommon common;
  std::uint16_t value = 0;
};

struct umpData {
  umpCommon common;
  uint8_t streamId = 0;
  uint8_t form = 0;
  std::span<std::uint8_t> data;
};

enum class note : std::uint8_t {
  unknown = 0x0,
  A = 0x1,
  B = 0x2,
  C = 0x3,
  D = 0x4,
  E = 0x5,
  F = 0x6,
  G = 0x7,
};

enum class chord_type : std::uint8_t {
  no_chord = 0x00,
  major = 0x01,
  major_6th = 0x02,
  major_7th = 0x03,
  major_9th = 0x04,
  major_11th = 0x05,
  major_13th = 0x06,
  minor = 0x07,
  minor_6th = 0x08,
  minor_7th = 0x09,
  minor_9th = 0x0A,
  minor_11th = 0x0B,
  minor_13th = 0x0C,
  dominant = 0x0D,
  dominant_ninth = 0x0E,
  dominant_11th = 0x0F,
  dominant_13th = 0x10,
  augmented = 0x11,
  augmented_seventh = 0x12,
  diminished = 0x13,
  diminished_seventh = 0x14,
  half_diminished = 0x15,
  major_minor = 0x16,
  pedal = 0x17,
  power = 0x18,
  suspended_2nd = 0x19,
  suspended_4th = 0x1A,
  seven_suspended_4th = 0x1B,
};

// (note that these are mostly four bit fields. The type fields are the
// exception. )
struct chord {
  bool operator==(chord const&) const = default;

  struct alteration {
    bool operator==(alteration const&) const = default;
    uint8_t type : 4;
    uint8_t degree : 4;
  };

  std::uint8_t chShrpFlt;
  note chTonic;
  chord_type chType;
  alteration chAlt1;
  alteration chAlt2;
  alteration chAlt3;
  alteration chAlt4;
  std::uint8_t baShrpFlt;
  note baTonic;
  chord_type baType;
  alteration baAlt1;
  alteration baAlt2;
};

struct function_block_info {
  bool operator==(function_block_info const&) const = default;

  enum class fbdirection : std::uint8_t {
    reserved = 0b00,
    input = 0b01,
    output = 0b10,
    bidirectional = 0b11,
  };

  std::uint8_t fbIdx;
  bool active;
  fbdirection direction;
  std::uint8_t firstGroup;
  std::uint8_t groupLength;
  std::uint8_t midiCIVersion;
  std::uint8_t isMIDI1;
  std::uint8_t maxS8Streams;
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
  { v.functionBlockInfo(function_block_info{}) } -> std::same_as<void>;
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
  virtual void utility_message(umpGeneric const& /*mess*/) { /* nop */ }
  virtual void channel_voice_message(umpCVM const& /*mess*/) { /* nop */ }
  virtual void system_message(umpGeneric const& /*mess*/) { /* nop */ }
  virtual void send_out_sysex(umpData const& /*mess*/) { /* nop */ }

  //---------- Flex Data
  virtual void flex_tempo(uint8_t /*group*/,
                          uint32_t /*num10nsPQN*/) { /* nop */ }
  virtual void flex_time_sig(uint8_t /*group*/, uint8_t /*numerator*/,
                             uint8_t /*denominator*/,
                             uint8_t /*num32Notes*/) { /* nop */ }
  virtual void flex_metronome(uint8_t /*group*/, uint8_t /*numClkpPriCli*/,
                              uint8_t /*bAccP1*/, uint8_t /*bAccP2*/,
                              uint8_t /*bAccP3*/, uint8_t /*numSubDivCli1*/,
                              uint8_t /*numSubDivCli2*/) { /* nop */ }
  virtual void flex_key_sig(uint8_t /*group*/, uint8_t /*addrs*/,
                            uint8_t /*channel*/, uint8_t /*sharpFlats*/,
                            uint8_t /*tonic*/) { /* nop */ }
  virtual void flex_chord(uint8_t /*group*/, uint8_t /*addrs*/,
                          uint8_t /*channel*/,
                          chord const& /*chord*/) { /* nop */ }
  virtual void flex_performance(umpData const& /*mess*/, uint8_t /*addrs*/,
                                uint8_t /*channel*/) { /* nop */ }
  virtual void flex_lyric(umpData const& /*mess*/, uint8_t /*addrs*/,
                          uint8_t /*channel*/) { /* nop */ }

  //---------- UMP Stream
  virtual void midiEndpoint(uint8_t /*majVer*/, uint8_t /*minVer*/,
                            uint8_t /*filter*/) { /* nop */ }
  virtual void midiEndpointName(umpData const& /*mess*/) { /* nop */ }
  virtual void midiEndpointProdId(umpData const& /*mess*/) { /* nop */ }
  virtual void midiEndpointJRProtocolReq(uint8_t /*protocol*/, bool /*jrrx*/,
                                         bool /*jrtx*/) { /* nop */ }
  virtual void midiEndpointInfo(uint8_t /*majVer*/, uint8_t /*minVer*/,
                                uint8_t /*numOfFuncBlocks*/, bool /*m2*/,
                                bool /*m1*/, bool /*rxjr*/,
                                bool /*txjr*/) { /* nop */ }
  virtual void midiEndpointDeviceInfo(
      std::array<uint8_t, 3> const& /*manuId*/,
      std::array<uint8_t, 2> const& /*familyId*/,
      std::array<uint8_t, 2> const& /*modelId*/,
      std::array<uint8_t, 4> const& /*version*/) { /* nop */ }
  virtual void midiEndpointJRProtocolNotify(uint8_t /*protocol*/, bool /*jrrx*/,
                                            bool /*jrtx*/) { /* nop */ }

  virtual void functionBlock(uint8_t /*fbIdx*/, uint8_t /*filter*/) { /* nop */
  }
  virtual void functionBlockInfo(function_block_info const& fbi) { (void)fbi; }
  virtual void functionBlockName(umpData const& /*mess*/,
                                 uint8_t /*fbIdx*/) { /* nop */ }

  virtual void startOfSeq() { /* nop */ }
  virtual void endOfFile() { /* nop */ }

  virtual void unknownUMPMessage(std::span<std::uint32_t>) { /* nop */ }
};

template <backend Callbacks = callbacks_base> class umpProcessor {
public:
  explicit umpProcessor(Callbacks cb = Callbacks{})
      : callbacks_{std::move(cb)} {}

  void clearUMP();
  void processUMP(uint32_t UMP);

private:
  void utility_message(ump_message_type mt);
  void system_message(ump_message_type mt, std::uint8_t group);
  void m1cvm_message();
  void sysex7_message(ump_message_type mt, std::uint8_t group);
  void m2cvm_message(ump_message_type mt, std::uint8_t group);
  void midi_endpoint_message(ump_message_type mt);
  void data_message();
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
  static constexpr OutputIterator payload(
      std::array<std::uint32_t, 4> const& message, std::size_t index,
      std::size_t limit, OutputIterator out);

  void midiendpoint_name_or_prodid(ump_message_type mt);
  void functionblock_name();
  void functionblock_info();
  void set_chord_name();
  void flexdata_performance_or_lyric(ump_message_type mt, std::uint8_t group);

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

template <backend Callbacks> void umpProcessor<Callbacks>::m1cvm_message() {
  // 32 Bit MIDI 1.0 Channel Voice Messages

  auto const w1 = std::bit_cast<types::m1cvm_w1>(message_[0]);

  umpCVM mess;
  mess.common.group = w1.group;
  mess.common.messageType = static_cast<ump_message_type>(w1.mt.value());
  mess.common.status = w1.status;
  mess.channel = w1.channel;
  auto const val1 = w1.byte_a;
  auto const val2 = w1.byte_b;

  switch (mess.common.status << 4) {
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
void umpProcessor<Callbacks>::midiendpoint_name_or_prodid(
    ump_message_type const mt) {
  std::uint16_t status = (message_[0] >> 16) & 0x3FF;
  assert(status == MIDIENDPOINT_NAME_NOTIFICATION ||
         status == MIDIENDPOINT_PRODID_NOTIFICATION);

  std::array<std::uint8_t, 14> text;
  auto text_length = 0U;
  if ((message_[0] >> 8) & 0xFF) {
    text[text_length++] = (message_[0] >> 8) & 0xFF;
  }
  if (message_[0] & 0xFF) {
    text[text_length++] = message_[0] & 0xFF;
  }
  for (auto i = 1U; i <= 3U; ++i) {
    for (auto j = 24; j >= 0; j -= 8) {
      if (std::uint8_t c = (message_[i] >> j) & 0xFF) {
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
    callbacks_.midiEndpointProdId(mess);
  }
}

template <backend Callbacks>
template <std::output_iterator<std::uint8_t> OutputIterator>
constexpr OutputIterator umpProcessor<Callbacks>::payload(
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
  return umpProcessor::payload(message, index + 1U, limit, out);
}

template <backend Callbacks>
void umpProcessor<Callbacks>::functionblock_name() {
  auto w1 = std::bit_cast<types::function_block_name_w1>(message_[0]);

  std::uint8_t const fbIdx = w1.block_number;
  std::array<std::uint8_t, 13> text;
  text[0] = w1.name;
  umpProcessor::payload(message_, 0, text.size() - 1, std::begin(text) + 1);
  auto const text_length =
      std::distance(find_if_not(std::rbegin(text), std::rend(text),
                                [](std::uint8_t v) { return v == 0; }),
                    std::rend(text));
  assert(text_length >= 0 &&
         static_cast<std::size_t>(text_length) <= text.size());

  umpData mess;
  mess.common.messageType = static_cast<ump_message_type>(w1.mt.value());
  mess.common.status = static_cast<std::uint8_t>(w1.status);
  mess.form = w1.format;
  mess.data = std::span{text.data(), static_cast<std::size_t>(text_length)};
  callbacks_.functionBlockName(mess, fbIdx);
}

template <backend Callbacks>
void umpProcessor<Callbacks>::functionblock_info() {
  function_block_info info;
  auto const w1 = std::bit_cast<types::function_block_info_w1>(message_[0]);
  auto const w2 = std::bit_cast<types::function_block_info_w2>(message_[1]);

  info.fbIdx = w1.block_number;
  info.active = w1.a;
  info.direction =
      static_cast<function_block_info::fbdirection>(w1.dir.value());
  info.firstGroup = w2.first_group;
  info.groupLength = w2.groups_spanned;
  info.midiCIVersion = w2.message_version;
  info.isMIDI1 = w1.m1;
  info.maxS8Streams = w2.num_sysex8_streams;
  callbacks_.functionBlockInfo(info);
}

template <backend Callbacks>
void umpProcessor<Callbacks>::midi_endpoint_message(ump_message_type const mt) {
  // 128 bits UMP Stream Messages
  std::uint16_t status = (message_[0] >> 16) & 0x3FF;
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
  case MIDIENDPOINT_PRODID_NOTIFICATION:
    this->midiendpoint_name_or_prodid(mt);
    break;
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

  case FUNCTIONBLOCK_INFO_NOTFICATION: this->functionblock_info(); break;
  case FUNCTIONBLOCK_NAME_NOTIFICATION: this->functionblock_name(); break;
  case STARTOFSEQ: callbacks_.startOfSeq(); break;
  case ENDOFFILE: callbacks_.endOfFile(); break;
  default: callbacks_.unknownUMPMessage(std::span{message_.data(), 4}); break;
  }
}

template <backend Callbacks> void umpProcessor<Callbacks>::data_message() {
  // 128 bit Data Messages (including System Exclusive 8)
  uint8_t const status = (message_[0] >> 20) & 0xF;
  switch (status) {
  case data_message_status::sysex8_in_1_ump:
  case data_message_status::sysex8_start:
  case data_message_status::sysex8_continue:
  case data_message_status::sysex8_end: {
    std::array<std::uint8_t, 13> sysex{};
    auto const w1 = std::bit_cast<types::sysex8_w1>(message_[0]);
    auto const data_length =
        std::min(std::size_t{w1.number_of_bytes}, sysex.size());
    if (data_length >= 1) {
      sysex[0] = w1.data;
      umpProcessor::payload(message_, 0, data_length - 1,
                            std::begin(sysex) + 1);
    }
    umpData mess;
    mess.common.group = w1.group;
    mess.common.messageType = static_cast<ump_message_type>(w1.mt.value());
    mess.streamId = w1.stream_id;
    mess.form = w1.status;
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

template <backend Callbacks> void umpProcessor<Callbacks>::set_chord_name() {
  auto const w1 = std::bit_cast<types::set_chord_name_w1>(message_[0]);
  auto const w2 = std::bit_cast<types::set_chord_name_w2>(message_[1]);
  auto const w3 = std::bit_cast<types::set_chord_name_w3>(message_[2]);
  auto const w4 = std::bit_cast<types::set_chord_name_w4>(message_[3]);

  auto const valid_note = [](std::uint8_t n) {
    return n <= static_cast<std::uint8_t>(note::G) ? static_cast<note>(n)
                                                   : note::unknown;
  };

  auto const valid_chord_type = [](std::uint8_t ct) {
    return ct <= static_cast<std::uint8_t>(chord_type::seven_suspended_4th)
               ? static_cast<chord_type>(ct)
               : chord_type::no_chord;
  };

  chord c;
  c.chShrpFlt = w2.tonic_sharps_flats;
  c.chTonic = valid_note(w2.chord_tonic);
  c.chType = valid_chord_type(w2.chord_type);
  c.chAlt1.type = w2.alter_1_type;
  c.chAlt1.degree = w2.alter_1_degree;
  c.chAlt2.type = w2.alter_2_type;
  c.chAlt2.degree = w2.alter_2_degree;
  c.chAlt3.type = w3.alter_3_type;
  c.chAlt3.degree = w3.alter_3_degree;
  c.chAlt4.type = w3.alter_4_type;
  c.chAlt4.degree = w3.alter_4_degree;
  c.baShrpFlt = w4.bass_sharps_flats;
  c.baTonic = valid_note(w4.bass_note);
  c.baType = valid_chord_type(w4.bass_chord_type);
  c.baAlt1.type = w4.alter_1_type;
  c.baAlt1.degree = w4.alter_1_degree;
  c.baAlt2.type = w4.alter_2_type;
  c.baAlt2.degree = w4.alter_2_degree;
  callbacks_.flex_chord(w1.group, w1.addrs, w1.channel, c);
}

template <backend Callbacks>
void umpProcessor<Callbacks>::flexdata_performance_or_lyric(
    ump_message_type const mt, std::uint8_t const group) {
  std::uint8_t const statusBank = (message_[0] >> 8) & 0xFF;
  std::uint8_t const status = message_[0] & 0xFF;
  std::uint8_t const channel = (message_[0] >> 16) & 0xF;
  std::uint8_t const addrs = (message_[0] >> 18) & 3;
  std::uint8_t const form = (message_[0] >> 20) & 3;

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
}

template <backend Callbacks>
void umpProcessor<Callbacks>::flexdata_message(ump_message_type const mt,
                                               std::uint8_t const group) {
  // 128 bit Data Messages (including System Exclusive 8)
  uint8_t statusBank = (message_[0] >> 8) & 0xFF;
  uint8_t status = message_[0] & 0xFF;
  uint8_t channel = (message_[0] >> 16) & 0xF;
  uint8_t addrs = (message_[0] >> 18) & 3;
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
    case FLEXDATA_COMMON_CHORD: this->set_chord_name(); break;
    default: callbacks_.unknownUMPMessage(std::span{message_.data(), 4}); break;
    }
    break;
  }
  case FLEXDATA_PERFORMANCE:
  case FLEXDATA_LYRIC: this->flexdata_performance_or_lyric(mt, group); break;

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
      this->m1cvm_message();
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
      this->data_message();
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
