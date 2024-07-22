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

#include "umpProcessor.h"

void umpProcessor::clearUMP() {
  messPos = 0;
  umpMess[0] = 0;
  umpMess[1] = 0;
  umpMess[2] = 0;
  umpMess[3] = 0;
}

void umpProcessor::processUMP(uint32_t UMP) {
  umpMess[messPos] = UMP;

  auto mt = static_cast<ump_message_type>((umpMess[0] >> 28) & 0xF);
  uint8_t group = (umpMess[0] >> 24) & 0xF;

  if (messPos == 0 &&
      (mt == ump_message_type::utility || mt == ump_message_type::system ||
       mt == ump_message_type::m1cvm || mt == ump_message_type::reserved32_06 ||
       mt == ump_message_type::reserved32_07)) {  // 32bit Messages
    if (mt == ump_message_type::utility &&
        utilityMessage != nullptr) {  // 32 bits Utility Messages
      umpGeneric mess;
      mess.common.messageType = mt;
      mess.common.status = static_cast<std::uint8_t>((umpMess[0] >> 20) & 0x0F);
      mess.value = static_cast<std::uint16_t>((umpMess[0] >> 16) & 0xFFFF);
      utilityMessage(mess);
    } else if (mt == ump_message_type::system && systemMessage) {
      // 32 bits System Real Time and System Common Messages (except System
      // Exclusive)
      umpGeneric mess;
      mess.common.messageType = mt;
      mess.common.group = group;
      mess.common.status = static_cast<std::uint8_t>((umpMess[0] >> 16) & 0xFF);
      switch (mess.common.status) {
      case status::timing_code:
      case status::song_select:
        mess.value = (umpMess[0] >> 8) & 0x7F;
        systemMessage(mess);
        break;
      case status::spp:
        mess.value = static_cast<std::uint16_t>(((umpMess[0] >> 8) & 0x7F) |
                                                ((umpMess[0] & 0x7F) << 7));
        systemMessage(mess);
        break;
      default: systemMessage(mess); break;
      }
    } else if (mt == ump_message_type::m1cvm &&
               channelVoiceMessage !=
                   nullptr) {  // 32 Bits MIDI 1.0 Channel Voice Messages
      umpCVM mess;
      mess.common.group = group;
      mess.common.messageType = mt;
      mess.common.status = umpMess[0] >> 16 & 0xF0;
      mess.channel = (umpMess[0] >> 16) & 0xF;
      uint8_t val1 = (umpMess[0] >> 8) & 0x7F;
      uint8_t val2 = umpMess[0] & 0x7F;

      switch (mess.common.status) {
      case status::note_off:      // Note Off
      case status::note_on:       // Note On
      case status::key_pressure:  // Poly Pressure
        mess.note = val1;
        mess.value = M2Utils::scaleUp(val2, 7, 16);
        channelVoiceMessage(mess);
        break;
      case status::channel_pressure:  // Channel Pressure
        mess.value = M2Utils::scaleUp(val2, 7, 32);
        channelVoiceMessage(mess);
        break;
      case status::cc:  // CC
        mess.index = val1;
        mess.value = M2Utils::scaleUp(val2, 7, 32);
        channelVoiceMessage(mess);
        break;
      case status::program_change:  // Program Change Message
        mess.value = val1;
        channelVoiceMessage(mess);
        break;
      case status::pitch_bend:  // PitchBend
        mess.value = M2Utils::scaleUp(
            (static_cast<std::uint32_t>(val2) << 7) + val1, 14, 32);
        channelVoiceMessage(mess);
        break;
      default:
        if (unknownUMPMessage) {
          unknownUMPMessage(umpMess, 2);
        }
        break;
      }
    }
    return;

  } else if (messPos == 1 &&
             (mt == ump_message_type::sysex7 || mt == ump_message_type::m2cvm ||
              mt == ump_message_type::reserved64_08 ||
              mt == ump_message_type::reserved64_09 ||
              mt == ump_message_type::reserved64_0A)) {  // 64bit Messages
    if (mt == ump_message_type::sysex7 &&
        sendOutSysex != nullptr) {  // 64 bits Data Messages (including
                                    // System Exclusive)
      umpData mess;
      mess.common.group = group;
      mess.common.messageType = mt;
      mess.form = (umpMess[0] >> 20) & 0xF;
      mess.dataLength = (umpMess[0] >> 16) & 0xF;
      uint8_t sysex[6];

      if (mess.dataLength > 0)
        sysex[0] = (umpMess[0] >> 8) & 0x7F;
      if (mess.dataLength > 1)
        sysex[1] = umpMess[0] & 0x7F;
      if (mess.dataLength > 2)
        sysex[2] = (umpMess[1] >> 24) & 0x7F;
      if (mess.dataLength > 3)
        sysex[3] = (umpMess[1] >> 16) & 0x7F;
      if (mess.dataLength > 4)
        sysex[4] = (umpMess[1] >> 8) & 0x7F;
      if (mess.dataLength > 5)
        sysex[5] = umpMess[1] & 0x7F;

      mess.data = sysex;
      sendOutSysex(mess);

    } else if (mt == ump_message_type::m2cvm &&
               channelVoiceMessage !=
                   nullptr) {  // 64 bits MIDI 2.0 Channel Voice Messages
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
        channelVoiceMessage(mess);
        break;
      case status::pitch_bend_pernote:
      case status::key_pressure:  // Poly Pressure
        mess.note = val1;
        mess.value = umpMess[1];
        channelVoiceMessage(mess);
        break;
      case status::channel_pressure:  // Channel Pressure
        mess.value = umpMess[1];
        channelVoiceMessage(mess);
        break;
      case status::cc:
        mess.index = val1;
        mess.value = umpMess[1];
        if (channelVoiceMessage != nullptr)
          channelVoiceMessage(mess);
        break;

      case status::rpn:            // RPN
      case status::nrpn:           // NRPN
      case status::rpn_relative:   // Relative RPN
      case status::nrpn_relative:  // Relative NRPN
        mess.bank = val1;
        mess.index = val2;
        mess.value = umpMess[1];
        if (channelVoiceMessage != nullptr)
          channelVoiceMessage(mess);
        break;

      case status::program_change:  // Program Change Message
        mess.value = umpMess[1] >> 24;
        mess.flag1 = umpMess[0] & 1;
        mess.bank = (umpMess[1] >> 8) & 0x7f;
        mess.index = umpMess[1] & 0x7f;
        if (channelVoiceMessage != nullptr)
          channelVoiceMessage(mess);
        break;

      case status::pitch_bend:  // PitchBend
        mess.value = umpMess[1];
        if (channelVoiceMessage != nullptr)
          channelVoiceMessage(mess);
        break;

      case status::nrpn_pernote:  // Assignable Per-Note Controller 1
      case status::rpn_pernote:   // Registered Per-Note Controller 0
        mess.note = val1;
        mess.index = val2;
        mess.value = umpMess[1];
        if (channelVoiceMessage != nullptr) {
          channelVoiceMessage(mess);
        }
        break;
      case status::pernote_manage:  // Per-Note Management Message
        mess.note = val1;
        mess.flag1 = (val2 & 2) != 0;
        mess.flag2 = (val2 & 1) != 0;
        if (channelVoiceMessage != nullptr) {
          channelVoiceMessage(mess);
        }
        break;
      default:
        if (unknownUMPMessage) {
          unknownUMPMessage(umpMess, 2);
        }
        break;
      }
    }
    messPos = 0;
  } else if (messPos == 2 &&
             (mt == ump_message_type::reserved96_0B ||
              mt == ump_message_type::reserved96_0C)) {  // 96bit Messages
    messPos = 0;
    if (unknownUMPMessage) {
      unknownUMPMessage(umpMess, 3);
    }
  } else if (messPos == 3 &&
             (mt == ump_message_type::data ||
              mt == ump_message_type::flex_data ||
              mt == ump_message_type::reserved128_0E ||
              mt == ump_message_type::midi_endpoint)) {  // 128bit Messages

    if (mt == ump_message_type::midi_endpoint) {  // 128 bits UMP Stream
                                                  // Messages
      uint16_t status = (umpMess[0] >> 16) & 0x3FF;

      switch (status) {
      case MIDIENDPOINT: {
        if (midiEndpoint != nullptr) {
          midiEndpoint((umpMess[0] >> 8) & 0xFF,  // Maj Ver
                       umpMess[0] & 0xFF,         // Min Ver
                       umpMess[1] & 0xFF);        // Filter
        }
        break;
      }
      case MIDIENDPOINT_INFO_NOTIFICATION: {
        if (midiEndpointInfo != nullptr) {
          midiEndpointInfo((umpMess[0] >> 8) & 0xFF,   // Maj Ver
                           umpMess[0] & 0xFF,          // Min Ver
                           (umpMess[1] >> 24) & 0xFF,  // Num Of Func Block
                           ((umpMess[1] >> 9) & 0x1),  // M2 Support
                           ((umpMess[1] >> 8) & 0x1),  // M1 Support
                           ((umpMess[1] >> 1) & 0x1),  // rxjr Support
                           (umpMess[1] & 0x1)          // txjr Support
          );
        }
        break;
      }

      case MIDIENDPOINT_DEVICEINFO_NOTIFICATION:
        if (midiEndpointDeviceInfo != nullptr) {
          midiEndpointDeviceInfo(
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
        }
        break;
      case MIDIENDPOINT_NAME_NOTIFICATION:
      case MIDIENDPOINT_PRODID_NOTIFICATION: {
        umpData mess = umpData();
        mess.common.messageType = mt;
        mess.common.status = static_cast<std::uint8_t>(status);
        mess.form = umpMess[0] >> 24 & 0x3;
        mess.dataLength = 0;
        uint8_t text[14];

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
        mess.data = text;
        if (status == MIDIENDPOINT_NAME_NOTIFICATION &&
            midiEndpointName != nullptr) {
          midiEndpointName(mess);
        }
        if (status == MIDIENDPOINT_PRODID_NOTIFICATION &&
            midiEndpointProdId != nullptr) {
          midiEndpointProdId(mess);
        }
        break;
      }

      case MIDIENDPOINT_PROTOCOL_REQUEST:  // JR Protocol Req
        if (midiEndpointJRProtocolReq != nullptr)
          midiEndpointJRProtocolReq(static_cast<std::uint8_t>(umpMess[0] >> 8),
                                    (umpMess[0] >> 1) & 1, umpMess[0] & 1);
        break;
      case MIDIENDPOINT_PROTOCOL_NOTIFICATION:  // JR Protocol Req
        if (midiEndpointJRProtocolNotify != nullptr)
          midiEndpointJRProtocolNotify(
              static_cast<std::uint8_t>(umpMess[0] >> 8), (umpMess[0] >> 1) & 1,
              umpMess[0] & 1);
        break;

      case FUNCTIONBLOCK: {
        uint8_t filter = umpMess[0] & 0xFF;
        uint8_t fbIdx = (umpMess[0] >> 8) & 0xFF;
        if (functionBlock != nullptr) {
          functionBlock(fbIdx, filter);
        }
        break;
      }

      case FUNCTIONBLOCK_INFO_NOTFICATION:
        if (functionBlockInfo != nullptr) {
          uint8_t fbIdx = (umpMess[0] >> 8) & 0x7F;
          functionBlockInfo(fbIdx,                        // fbIdx
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
        }
        break;
      case FUNCTIONBLOCK_NAME_NOTIFICATION: {
        uint8_t fbIdx = (umpMess[0] >> 8) & 0x7F;
        umpData mess = umpData();
        mess.common.messageType = mt;
        mess.common.status = static_cast<std::uint8_t>(status);
        mess.form = umpMess[0] >> 24 & 0x3;
        mess.dataLength = 0;
        uint8_t text[13];

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
        mess.data = text;

        if (functionBlockName != nullptr) {
          functionBlockName(mess, fbIdx);
        }
        break;
      }
      case STARTOFSEQ: {
        if (startOfSeq != nullptr) {
          startOfSeq();
        }
        break;
      }
      case ENDOFFILE: {
        if (endOfFile != nullptr) {
          endOfFile();
        }
        break;
      }
      default:
        if (unknownUMPMessage) {
          unknownUMPMessage(umpMess, 4);
        }
        break;
      }

    } else if (mt == ump_message_type::data) {  // 128 bits Data Messages
                                                // (including System
                                                // Exclusive 8)
      uint8_t status = (umpMess[0] >> 20) & 0xF;

      if (status <= 3) {
        umpData mess = umpData();
        mess.common.group = group;
        mess.common.messageType = mt;
        mess.streamId = (umpMess[0] >> 8) & 0xFF;
        mess.form = status;
        mess.dataLength = (umpMess[0] >> 16) & 0xF;

        uint8_t sysex[13];

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

        mess.data = sysex;
        sendOutSysex(mess);

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
        if (unknownUMPMessage)
          unknownUMPMessage(umpMess, 4);
      }

    } else if (mt == ump_message_type::flex_data) {  // 128 bits Data Messages
                                                     // (including System
                                                     // Exclusive 8)
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
          if (flexTempo != nullptr)
            flexTempo(group, umpMess[1]);
          break;
        }
        case FLEXDATA_COMMON_TIMESIG: {  // Set Time Signature Message
          if (flexTimeSig != nullptr)
            flexTimeSig(group, (umpMess[1] >> 24) & 0xFF,
                        (umpMess[1] >> 16) & 0xFF, (umpMess[1] >> 8) & 0xFF);
          break;
        }
        case FLEXDATA_COMMON_METRONOME: {  // Set Metronome Message
          if (flexMetronome != nullptr)
            flexMetronome(group, (umpMess[1] >> 24) & 0xFF,
                          (umpMess[1] >> 16) & 0xFF, (umpMess[1] >> 8) & 0xFF,
                          umpMess[1] & 0xFF, (umpMess[2] >> 24) & 0xFF,
                          (umpMess[2] >> 16) & 0xFF);
          break;
        }
        case FLEXDATA_COMMON_KEYSIG: {  // Set Key Signature Message
          if (flexKeySig != nullptr)
            flexKeySig(group, addrs, channel, (umpMess[1] >> 24) & 0xFF,
                       (umpMess[1] >> 16) & 0xFF);
          break;
        }
        case FLEXDATA_COMMON_CHORD: {  // Set Chord Message
          if (flexChord != nullptr)
            flexChord(group, addrs, channel,
                      (umpMess[1] >> 28) & 0xF,   // chShrpFlt
                      (umpMess[1] >> 24) & 0xF,   // chTonic
                      (umpMess[1] >> 16) & 0xFF,  // chType
                      (umpMess[1] >> 12) & 0xF,   // chAlt1Type
                      (umpMess[1] >> 8) & 0xF,    // chAlt1Deg
                      (umpMess[1] >> 4) & 0xF,    // chAlt2Type
                      umpMess[1] & 0xF,           // chAlt2Deg
                      (umpMess[2] >> 28) & 0xF,   // chAlt3Type
                      (umpMess[2] >> 24) & 0xF,   // chAlt3Deg
                      (umpMess[2] >> 20) & 0xF,   // chAlt4Type
                      (umpMess[2] >> 16) & 0xF,   // chAlt4Deg
                      (umpMess[3] >> 28) & 0xF,   // baShrpFlt
                      (umpMess[3] >> 24) & 0xF,   // baTonic
                      (umpMess[3] >> 16) & 0xFF,  // baType
                      (umpMess[3] >> 12) & 0xF,   // baAlt1Type
                      (umpMess[3] >> 8) & 0xF,    // baAlt1Deg
                      (umpMess[3] >> 4) & 0xF,    // baAlt2Type
                      umpMess[1] & 0xF            // baAlt2Deg
            );
          break;
        }
        default:
          if (unknownUMPMessage)
            unknownUMPMessage(umpMess, 4);
          break;
        }
        break;
      }
      case FLEXDATA_PERFORMANCE:  // Performance Events
      case FLEXDATA_LYRIC: {      // Lyric Events
        umpData mess;
        mess.common.group = group;
        mess.common.messageType = mt;
        mess.common.status = status;
        mess.form = form;
        mess.dataLength = 0;
        uint8_t text[12];

        for (uint8_t i = 1; i <= 3; i++) {
          for (int j = 24; j >= 0; j -= 8) {
            uint8_t c = (umpMess[i] >> j) & 0xFF;
            if (c) {
              text[mess.dataLength++] = c;
            }
          }
        }
        mess.data = text;
        if (statusBank == FLEXDATA_LYRIC && flexLyric != nullptr)
          flexLyric(mess, addrs, channel);
        if (statusBank == FLEXDATA_PERFORMANCE && flexPerformance != nullptr)
          flexPerformance(mess, addrs, channel);
        break;
      }
      default:
        if (unknownUMPMessage)
          unknownUMPMessage(umpMess, 4);
        break;
      }
    } else {
      if (unknownUMPMessage)
        unknownUMPMessage(umpMess, 4);
    }
    messPos = 0;
  } else {
    messPos++;
  }
}
