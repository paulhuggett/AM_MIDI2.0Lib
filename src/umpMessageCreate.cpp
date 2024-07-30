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
#include "midi2/umpMessageCreate.h"

#include "midi2/utils.h"

using namespace midi2;

namespace {

uint32_t m1Create(uint8_t group, uint8_t status, uint8_t val1, uint8_t val2) {
  return (((static_cast<std::uint32_t>(ump_message_type::system) << 4) | group)
          << 24) |
         (static_cast<std::uint32_t>(status) << 16) |
         (static_cast<std::uint32_t>(val1) << 8) |
         static_cast<std::uint32_t>(val2);
}

uint32_t mt2Create(uint8_t group, uint8_t status, uint8_t channel, uint8_t val1,
                   uint8_t val2) {
  return (((static_cast<std::uint32_t>(ump_message_type::m1cvm) << 4) | group)
          << 24) |
         (static_cast<std::uint32_t>(status | channel) << 16) |
         (static_cast<std::uint32_t>(val1) << 8) | val2;
}

uint32_t mt4CreateFirstWord(uint8_t group, uint8_t status, uint8_t channel,
                            uint8_t val1, uint8_t val2) {
  return (((static_cast<std::uint32_t>(ump_message_type::m2cvm) << 4) | group)
          << 24) |
         ((static_cast<std::uint32_t>(status) | channel) << 16) |
         (static_cast<std::uint32_t>(val1) << 8) | val2;
}

}  // end anonymous namespace

namespace midi2 {

uint32_t UMPMessage::mt0NOOP() {
  return 0;
}

uint32_t UMPMessage::mt0JRClock(uint16_t clockTime) {
  return ((UTILITY_JRCLOCK + 0L) << 20) + clockTime;
}

uint32_t UMPMessage::mt0JRTimeStamp(uint16_t timestamp) {
  return ((UTILITY_JRTS + 0L) << 20) + timestamp;
}

uint32_t UMPMessage::mt0DeltaClockTick(uint16_t ticksPerQtrNote) {
  return ((UTILITY_DELTACLOCKTICK + 0L) << 20) + ticksPerQtrNote;
}

uint32_t UMPMessage::mt0DeltaTicksSinceLast(uint16_t noTicksSince) {
  return ((UTILITY_DELTACLOCKSINCE + 0L) << 20) + noTicksSince;
}

uint32_t UMPMessage::mt1MTC(uint8_t group, uint8_t timeCode) {
  return m1Create(group, timing_code, timeCode, 0);
}
uint32_t UMPMessage::mt1SPP(uint8_t group, uint16_t position) {
  return m1Create(group, spp, position & 0x7F, (position >> 7) & 0x7F);
}
uint32_t UMPMessage::mt1SongSelect(uint8_t group, uint8_t song) {
  return m1Create(group, song_select, song, 0);
}
uint32_t UMPMessage::mt1TuneRequest(uint8_t group) {
  return m1Create(group, tunerequest, 0, 0);
}
uint32_t UMPMessage::mt1TimingClock(uint8_t group) {
  return m1Create(group, timingclock, 0, 0);
}
uint32_t UMPMessage::mt1SeqStart(uint8_t group) {
  return m1Create(group, seqstart, 0, 0);
}
uint32_t UMPMessage::mt1SeqCont(uint8_t group) {
  return m1Create(group, seqcont, 0, 0);
}
uint32_t UMPMessage::mt1SeqStop(uint8_t group) {
  return m1Create(group, seqstop, 0, 0);
}
uint32_t UMPMessage::mt1ActiveSense(uint8_t group) {
  return m1Create(group, activesense, 0, 0);
}
uint32_t UMPMessage::mt1SystemReset(uint8_t group) {
  return m1Create(group, systemreset, 0, 0);
}

uint32_t UMPMessage::mt2NoteOn(uint8_t group, uint8_t channel,
                               uint8_t noteNumber, uint8_t velocity) {
  return mt2Create(group, status::note_on, channel, noteNumber, velocity);
}
uint32_t UMPMessage::mt2NoteOff(uint8_t group, uint8_t channel,
                                uint8_t noteNumber, uint8_t velocity) {
  return mt2Create(group, status::note_off, channel, noteNumber, velocity);
}

uint32_t UMPMessage::mt2PolyPressure(uint8_t group, uint8_t channel,
                                     uint8_t noteNumber, uint8_t pressure) {
  return mt2Create(group, status::key_pressure, channel, noteNumber, pressure);
}
uint32_t UMPMessage::mt2CC(uint8_t group, uint8_t channel, uint8_t index,
                           uint8_t value) {
  return mt2Create(group, status::cc, channel, index, value);
}
uint32_t UMPMessage::mt2ProgramChange(uint8_t group, uint8_t channel,
                                      uint8_t program) {
  return mt2Create(group, status::program_change, channel, program, 0);
}
uint32_t UMPMessage::mt2ChannelPressure(uint8_t group, uint8_t channel,
                                        uint8_t pressure) {
  return mt2Create(group, status::channel_pressure, channel, pressure, 0);
}
uint32_t UMPMessage::mt2PitchBend(uint8_t group, uint8_t channel,
                                  uint16_t value) {
  return mt2Create(group, status::pitch_bend, channel, value & 0x7F,
                   (value >> 7) & 0x7F);
}

std::array<uint32_t, 2> UMPMessage::mt3Sysex7(uint8_t group, uint8_t status,
                                              uint8_t numBytes,
                                              std::array<uint8_t, 6> sx) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] = (static_cast<std::uint32_t>(0x3) << 28) |
               (static_cast<std::uint32_t>(group) << 24) |
               (static_cast<std::uint32_t>(status) << 20) |
               (static_cast<std::uint32_t>(numBytes) << 16);
  if (numBytes > 0) {
    umpMess[0] |= (static_cast<std::uint32_t>(sx[0]) << 8);
  }
  if (numBytes > 1) {
    umpMess[0] |= sx[1];
  }
  if (numBytes > 2) {
    umpMess[1] |= (static_cast<std::uint32_t>(sx[2]) << 24);
  }
  if (numBytes > 3) {
    umpMess[1] |= (static_cast<std::uint32_t>(sx[3]) << 16);
  }
  if (numBytes > 4) {
    umpMess[1] |= (static_cast<std::uint32_t>(sx[4]) << 8);
  }
  if (numBytes > 5) {
    umpMess[1] |= sx[5];
  }
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4NoteOn(uint8_t group, uint8_t channel,
                                              uint8_t noteNumber,
                                              uint16_t velocity,
                                              uint8_t attributeType,
                                              uint16_t attributeData) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] = mt4CreateFirstWord(group, status::note_on, channel, noteNumber,
                                  attributeType);
  umpMess[1] = static_cast<std::uint32_t>(velocity) << 16;
  umpMess[1] += attributeData;
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4NoteOff(uint8_t group, uint8_t channel,
                                               uint8_t noteNumber,
                                               uint16_t velocity,
                                               uint8_t attributeType,
                                               uint16_t attributeData) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] = mt4CreateFirstWord(group, status::note_off, channel, noteNumber,
                                  attributeType);
  umpMess[1] = static_cast<std::uint32_t>(velocity << 16);
  umpMess[1] += attributeData;
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4CPolyPressure(uint8_t group,
                                                     uint8_t channel,
                                                     uint8_t noteNumber,
                                                     uint32_t pressure) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] =
      mt4CreateFirstWord(group, status::key_pressure, channel, noteNumber, 0);
  umpMess[1] = pressure;
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4PitchBend(uint8_t group, uint8_t channel,
                                                 uint32_t pitch) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] = mt4CreateFirstWord(group, status::pitch_bend, channel, 0, 0);
  umpMess[1] = pitch;
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4CC(uint8_t group, uint8_t channel,
                                          uint8_t index, uint32_t value) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] = mt4CreateFirstWord(group, status::cc, channel, index, 0);
  umpMess[1] = value;
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4RPN(uint8_t group, uint8_t channel,
                                           uint8_t bank, uint8_t index,
                                           uint32_t value) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] =
      mt4CreateFirstWord(group, midi2status::rpn, channel, bank, index);
  umpMess[1] = value;
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4NRPN(uint8_t group, uint8_t channel,
                                            uint8_t bank, uint8_t index,
                                            uint32_t value) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] =
      mt4CreateFirstWord(group, midi2status::nrpn, channel, bank, index);
  umpMess[1] = value;
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4RelativeRPN(uint8_t group,
                                                   uint8_t channel,
                                                   uint8_t bank, uint8_t index,
                                                   int32_t value) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] = mt4CreateFirstWord(group, midi2status::rpn_relative, channel,
                                  bank, index);
  umpMess[1] = static_cast<std::uint32_t>(value);
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4RelativeNRPN(uint8_t group,
                                                    uint8_t channel,
                                                    uint8_t bank, uint8_t index,
                                                    int32_t value) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] = mt4CreateFirstWord(group, midi2status::nrpn_relative, channel,
                                  bank, index);
  umpMess[1] = static_cast<std::uint32_t>(value);
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4ChannelPressure(uint8_t group,
                                                       uint8_t channel,
                                                       uint32_t pressure) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] =
      mt4CreateFirstWord(group, status::channel_pressure, channel, 0, 0);
  umpMess[1] = pressure;
  return umpMess;
}

std::array<uint32_t, 2> UMPMessage::mt4ProgramChange(
    uint8_t group, uint8_t channel, uint8_t program, bool bankValid,
    uint8_t bank, uint8_t index) {
  std::array<uint32_t, 2> umpMess = {0, 0};
  umpMess[0] = mt4CreateFirstWord(group, status::program_change, channel,
                                  program, bankValid ? 1 : 0);
  umpMess[1] = bankValid ? (static_cast<std::uint32_t>(bank) << 8) | index : 0;
  return umpMess;
}

// TODO: mtD*

std::array<uint32_t, 4> UMPMessage::mtFMidiEndpoint(uint8_t filter) {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  umpMess[0] =
      (std::uint32_t{0xF} << 28) | (UMP_VER_MAJOR << 8) | UMP_VER_MINOR;
  umpMess[1] = filter;
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFMidiEndpointInfoNotify(
    uint8_t numOfFuncBlock, bool m2, bool m1, bool rxjr, bool txjr) {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  umpMess[0] = (std::uint32_t{0xF} << 28) |
               (MIDIENDPOINT_INFO_NOTIFICATION << 16) | (UMP_VER_MAJOR << 8) |
               UMP_VER_MINOR;
  umpMess[1] = (static_cast<std::uint32_t>(numOfFuncBlock) << 24) |
               (static_cast<std::uint32_t>(m2) << 9) |
               (static_cast<std::uint32_t>(m1) << 8) |
               (static_cast<std::uint32_t>(rxjr) << 1) |
               static_cast<std::uint32_t>(txjr);
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFMidiEndpointDeviceInfoNotify(
    std::array<uint8_t, 3> manuId, std::array<uint8_t, 2> familyId,
    std::array<uint8_t, 2> modelId, std::array<uint8_t, 4> version) {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  umpMess[0] =
      (std::uint32_t{0xF} << 28) |
      (MIDIENDPOINT_DEVICEINFO_NOTIFICATION << 16) /*+  numOfFuncBlock*/;

  umpMess[1] = (static_cast<std::uint32_t>(manuId[0]) << 16) |
               static_cast<std::uint32_t>((manuId[1]) << 8) |
               static_cast<std::uint32_t>(manuId[2]);
  umpMess[2] = (static_cast<std::uint32_t>(familyId[0]) << 24) |
               (static_cast<std::uint32_t>(familyId[1]) << 16) |
               (static_cast<std::uint32_t>(modelId[0]) << 8) |
               static_cast<std::uint32_t>(modelId[1]);
  umpMess[3] = (static_cast<std::uint32_t>(version[0]) << 24) |
               (static_cast<std::uint32_t>(version[1]) << 16) |
               (static_cast<std::uint32_t>(version[2]) << 8) |
               static_cast<std::uint32_t>(version[3]);
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFMidiEndpointTextNotify(
    uint16_t replyType, uint8_t offset, uint8_t* text, uint8_t textLen) {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  auto form = std::uint32_t{0};
  if (offset == 0) {
    if (textLen > 14)
      form = 1;
  } else {
    if (offset + 13 < textLen) {
      form = 2;
    } else {
      form = 3;
    }
  }
  umpMess[0] = (std::uint32_t{0xF} << 28) | (form << 26) |
               (static_cast<std::uint32_t>(replyType) << 16);
  if (offset < textLen) {
    umpMess[0] += (static_cast<std::uint32_t>(text[offset++]) << 8);
  }
  if (offset < textLen) {
    umpMess[0] += static_cast<std::uint32_t>(text[offset++]);
  }
  for (uint8_t i = 1; i < 4; i++) {
    if (offset < textLen) {
      umpMess[i] += (static_cast<std::uint32_t>(text[offset++]) << 24);
    }
    if (offset < textLen) {
      umpMess[i] += (static_cast<std::uint32_t>(text[offset++]) << 16);
    }
    if (offset < textLen) {
      umpMess[i] += (static_cast<std::uint32_t>(text[offset++]) << 8);
    }
    if (offset < textLen) {
      umpMess[i] += text[offset++];
    }
  }
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFFunctionBlock(uint8_t fbIdx,
                                                     uint8_t filter) {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  umpMess[0] = (static_cast<std::uint32_t>(0xF) << 28) | (FUNCTIONBLOCK << 16) |
               (static_cast<std::uint32_t>(fbIdx) << 8) | filter;
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFFunctionBlockInfoNotify(
    uint8_t fbIdx, bool active, uint8_t direction, bool sender, bool recv,
    uint8_t firstGroup, uint8_t groupLength, uint8_t midiCISupport,
    uint8_t isMIDI1, uint8_t maxS8Streams) {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  umpMess[0] =
      (std::uint32_t{0xF} << 28) | (FUNCTIONBLOCK_INFO_NOTFICATION << 16) |
      ((active ? 1U : 0U) << 15) | (static_cast<std::uint32_t>(fbIdx) << 8) |
      (static_cast<std::uint32_t>(recv) << 5) |
      (static_cast<std::uint32_t>(sender) << 4) |
      (static_cast<std::uint32_t>(isMIDI1) << 2) |
      static_cast<std::uint32_t>(direction);
  umpMess[1] = (static_cast<std::uint32_t>(firstGroup) << 24) |
               (static_cast<std::uint32_t>(groupLength) << 16) |
               (static_cast<std::uint32_t>(midiCISupport) << 8) | maxS8Streams;
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFFunctionBlockNameNotify(
    uint8_t fbIdx, uint8_t offset, uint8_t* text, uint8_t textLen) {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  uint8_t form = 0;
  if (offset == 0) {
    if (textLen > 13) {
      form = 1;
    }
  } else {
    if (offset + 12 < textLen) {
      form = 2;
    } else {
      form = 3;
    }
  }
  umpMess[0] = (std::uint32_t{0xF} << 28) |
               (static_cast<std::uint32_t>(form) << 26) |
               (FUNCTIONBLOCK_NAME_NOTIFICATION << 16) |
               (static_cast<std::uint32_t>(fbIdx) << 8);
  if (offset < textLen) {
    umpMess[0] += static_cast<std::uint32_t>(text[offset++]);
  }
  for (uint8_t i = 1; i < 4; i++) {
    if (offset < textLen) {
      umpMess[i] += (static_cast<std::uint32_t>(text[offset++]) << 24);
    }
    if (offset < textLen) {
      umpMess[i] += (static_cast<std::uint32_t>(text[offset++]) << 16);
    }
    if (offset < textLen) {
      umpMess[i] += (static_cast<std::uint32_t>(text[offset++]) << 8);
    }
    if (offset < textLen) {
      umpMess[i] += text[offset++];
    }
  }
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFStartOfSeq() {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  umpMess[0] = (std::uint32_t{0xF} << 28) | (std::uint32_t{STARTOFSEQ} << 16);
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFEndOfFile() {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  umpMess[0] = (std::uint32_t{0xF} << 28) | (std::uint32_t{ENDOFFILE} << 16);
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFRequestProtocol(uint8_t protocol,
                                                       bool jrrx, bool jrtx) {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  umpMess[0] = (std::uint32_t{0xF} << 28) |
               (MIDIENDPOINT_PROTOCOL_REQUEST << 16) |
               (static_cast<std::uint32_t>(protocol) << 8) |
               (static_cast<std::uint32_t>(jrrx) << 1) |
               static_cast<std::uint32_t>(jrtx);
  return umpMess;
}

std::array<uint32_t, 4> UMPMessage::mtFNotifyProtocol(uint8_t protocol,
                                                      bool jrrx, bool jrtx) {
  std::array<uint32_t, 4> umpMess = {0, 0, 0, 0};
  umpMess[0] = (std::uint32_t{0xF} << 28) |
               (MIDIENDPOINT_PROTOCOL_NOTIFICATION << 16) |
               (static_cast<std::uint32_t>(protocol) << 8) |
               (static_cast<std::uint32_t>(jrrx) << 1) |
               static_cast<std::uint32_t>(jrtx);
  return umpMess;
}

}  // end namespace midi2
