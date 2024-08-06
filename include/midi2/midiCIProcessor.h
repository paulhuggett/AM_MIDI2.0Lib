/**********************************************************
 * MIDI 2.0 Library
 * Author: Andrew Mee
 *
 * MIT License
 * Copyright 2022 Andrew Mee
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

#ifndef MIDI2_MIDICIPROCESSOR_H
#define MIDI2_MIDICIPROCESSOR_H

#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <tuple>

#include "midi2/utils.h"

namespace midi2 {

constexpr auto FUNCTION_BLOCK = std::uint8_t{0x7F};

using reqId = std::tuple<uint32_t, std::uint8_t>;  // muid-requestId

struct MIDICI {
  std::uint8_t umpGroup = 255;
  std::uint8_t deviceId = FUNCTION_BLOCK;
  std::uint8_t ciType = 255;
  std::uint8_t ciVer = 1;
  std::uint32_t remoteMUID = 0;
  std::uint32_t localMUID = 0;
  std::optional<reqId> _peReqIdx;

  std::uint8_t totalChunks = 0;
  std::uint8_t numChunk = 0;
  std::uint8_t partialChunkCount = 0;
  std::uint8_t requestId = 255;
};

class ci_callbacks {
public:
  virtual ~ci_callbacks () = default;

  virtual bool checkMUIDFn(std::uint8_t /*group*/, uint32_t /*muid*/) { return false; }
  virtual void recvDiscoveryRequestFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 3> /*manuId*/, std::array<std::uint8_t, 2> /*familyId*/, std::array<std::uint8_t, 2> /*modelId*/, std::array<std::uint8_t, 4> /*version*/, std::uint8_t /*ciSupport*/, std::uint16_t /*maxSysex*/, std::uint8_t /*outputPathId*/) { }
  virtual void recvDiscoveryReplyFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 3> /*manuId*/, std::array<std::uint8_t, 2> /*familyId*/, std::array<std::uint8_t, 2> /*modelId*/, std::array<std::uint8_t, 4> /*version*/, std::uint8_t /*ciSupport*/, std::uint16_t /*maxSysex*/, std::uint8_t /*outputPathId*/, std::uint8_t /*fbIdx*/) {}
  virtual void recvEndPointInfoFn(MIDICI /*ciDetails*/, std::uint8_t /*status*/) {}
  virtual void recvEndPointInfoReplyFn(MIDICI /*ciDetails*/, std::uint8_t /*status*/, std::uint16_t /*infoLength*/, std::uint8_t* /*infoData*/) {}
  virtual void recvNAKFn(MIDICI /*ciDetails*/, std::uint8_t /*origSubID*/, std::uint8_t /*statusCode*/, std::uint8_t /*statusData*/, std::uint8_t* /*ackNakDetails*/, std::uint16_t /*messageLength*/, std::uint8_t* /*ackNakMessage*/) {}
  virtual void recvACKFn(MIDICI /*ciDetails*/, std::uint8_t /*origSubID*/, std::uint8_t /*statusCode*/, std::uint8_t /*statusData*/, std::uint8_t* /*ackNakDetails*/, std::uint16_t /*messageLength*/, std::uint8_t* /*ackNakMessage*/) {}
  virtual void recvInvalidateMUIDFn(MIDICI /*ciDetails*/, uint32_t /*terminateMuid*/) {}
  virtual void recvUnknownMIDICIFn(MIDICI /*ciDetails*/, std::uint8_t /*s7Byte*/) {}

  // Protocol Negotiation
  virtual void recvProtocolAvailableFn(MIDICI /*ciDetails*/, std::uint8_t /*authorityLevel*/, std::uint8_t* /*protocol*/) {}
  virtual void recvSetProtocolFn(MIDICI /*ciDetails*/, std::uint8_t /*authorityLevel*/, std::uint8_t* /*protocol*/) {}
  virtual void recvSetProtocolConfirmFn(MIDICI /*ciDetails*/, std::uint8_t /*authorityLevel*/) {}
  virtual void recvProtocolTestFn(MIDICI /*ciDetails*/, std::uint8_t /*authorityLevel*/, bool /*testDataAccurate*/) {}

  // Profiles
  virtual void recvProfileInquiryFn(MIDICI /*ciDetails*/) {}
  virtual void recvSetProfileEnabledFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 5> /*profile*/, std::uint8_t /*numberOfChannels*/) {}
  virtual void recvSetProfileRemovedFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 5> /*profile*/) {}
  virtual void recvSetProfileDisabledFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 5> /*profile*/, std::uint8_t /*numberOfChannels*/) {}
  virtual void recvSetProfileOnFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 5> /*profile*/, std::uint8_t /*numberOfChannels*/) {}
  virtual void recvSetProfileOffFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 5> /*profile*/) {}
  virtual void recvProfileSpecificDataFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 5> /*profile*/, std::uint16_t /*datalen*/, std::uint8_t* /*data*/, std::uint16_t /*part*/, bool /*lastByteOfSet*/) {}
  virtual void recvSetProfileDetailsInquiryFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 5> /*profile*/, std::uint8_t /*InquiryTarget*/) {}
  virtual void recvSetProfileDetailsReplyFn(MIDICI /*ciDetails*/, std::array<std::uint8_t, 5> /*profile*/, std::uint8_t /*InquiryTarget*/, std::uint16_t /*datalen*/, std::uint8_t* /*data*/) {}

  // Property Exchange
  virtual void recvPECapabilitiesFn(MIDICI /*ciDetails*/, std::uint8_t /*numSimulRequests*/, std::uint8_t /*majVer*/, std::uint8_t /*minVer*/) {}
  virtual void recvPECapabilitiesRepliesFn(MIDICI /*ciDetails*/, std::uint8_t /*numSimulRequests*/, std::uint8_t /*majVer*/, std::uint8_t /*minVer*/) {}
  virtual void recvPEGetInquiryFn(MIDICI /*ciDetails*/, std::string /*requestDetails*/) {}
  virtual void recvPESetReplyFn(MIDICI /*ciDetails*/, std::string /*requestDetails*/) {}
  virtual void recvPESubReplyFn(MIDICI /*ciDetails*/, std::string /*requestDetails*/) {}
  virtual void recvPENotifyFn(MIDICI /*ciDetails*/, std::string /*requestDetails*/) {}
  virtual void recvPEGetReplyFn(MIDICI /*ciDetails*/, std::string /*requestDetails*/, std::uint16_t /*bodyLen*/, std::uint8_t* /*body*/, bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) {}
  virtual void recvPESetInquiryFn(MIDICI /*ciDetails*/, std::string /*requestDetails*/, std::uint16_t /*bodyLen*/, std::uint8_t* /*body*/, bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) {}
  virtual void recvPESubInquiryFn(MIDICI /*ciDetails*/, std::string /*requestDetails*/, std::uint16_t /*bodyLen*/, std::uint8_t* /*body*/, bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) {}

  // Process Inquiry
  virtual void recvPICapabilitiesFn(MIDICI /*ciDetails*/) {}
  virtual void recvPICapabilitiesReplyFn(MIDICI /*ciDetails*/, std::uint8_t /*supportedFeatures*/) {}
  virtual void recvPIMMReportFn(MIDICI /*ciDetails*/, std::uint8_t /*MDC*/, std::uint8_t /*systemBitmap*/, std::uint8_t /*chanContBitmap*/, std::uint8_t /*chanNoteBitmap*/) {}
  virtual void recvPIMMReportReplyFn(MIDICI /*ciDetails*/, std::uint8_t /*systemBitmap*/, std::uint8_t /*chanContBitmap*/, std::uint8_t /*chanNoteBitmap*/) {}
  virtual void recvPIMMReportEndFn(MIDICI /*ciDetails*/) {}
};

template <typename Callbacks = ci_callbacks>
class midiCIProcessor {
public:
  midiCIProcessor (Callbacks callbacks) : callbacks_{std::move(callbacks)} {}

  void startSysex7(std::uint8_t group, std::uint8_t deviceId);
  void endSysex7();

  void processMIDICI(std::uint8_t s7Byte);

private:
  static constexpr auto S7_BUFFERLEN = 36;

  Callbacks callbacks_;
  MIDICI midici;
  std::uint8_t buffer[256];
  /*
   * in Discovery this is
   * [sysexID1,sysexID2,sysexID3,famId1,famid2,modelId1,modelId2,ver1,ver2,ver3,ver4,...product
   * Id] in Profiles this is [pf1, pf1, pf3, pf4, pf5] in Protocols this is
   * [pr1, pr2, pr3, pr4, pr5]
   */

  std::uint16_t intTemp_[4];
  /* in Discovery this is [ciSupport, maxSysex, output path id]
   * in Profile Inquiry Reply, this is [Enabled Profiles Length, Disabled
   * Profile Length] in Profile On/Off/Enabled/Disabled, this is
   * [numOfChannels] in PE this is [header length, Body Length]
   */
  std::uint16_t sysexPos_ = 0;


  void processProtocolSysex(std::uint8_t s7Byte);
  void processProfileSysex(std::uint8_t s7Byte);

  // Property Exchange
  std::map<reqId, std::string> peHeaderStr;

  void cleanupRequest(reqId peReqIdx);
  void processPESysex(std::uint8_t s7Byte);
  void processPISysex(std::uint8_t s7Byte);

  void midiCI_discovery_request_reply(std::uint8_t s7Byte);
  void midiCI_ack_nak(std::uint8_t s7Byte);
  void midiCI_endpoint_info_reply(std::uint8_t s7Byte);
};


template <typename Callbacks>
void midiCIProcessor<Callbacks>::endSysex7() {
  if (midici._peReqIdx) {
    cleanupRequest(midici._peReqIdx);
  }
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::startSysex7(std::uint8_t group, std::uint8_t deviceId) {
  sysexPos_ = 0;
  buffer[0] = '\0';
  midici = MIDICI();
  midici.deviceId = deviceId;
  midici.umpGroup = group;
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::cleanupRequest(reqId peReqIdx) {
  peHeaderStr.erase(peReqIdx);
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::midiCI_discovery_request_reply(std::uint8_t s7Byte) {
  if (sysexPos_ >= 13 && sysexPos_ <= 23) {
    buffer[sysexPos_ - 13] = s7Byte;
  }
  if (sysexPos_ == 24) {
    intTemp_[0] = s7Byte;  // ciSupport
  }
  if (sysexPos_ >= 25 && sysexPos_ <= 28) {
    intTemp_[1] += s7Byte << (7 * (sysexPos_ - 25));  // maxSysEx
  }

  bool complete = false;
  if (sysexPos_ == 28 && midici.ciVer == 1) {
    complete = true;
  } else if (sysexPos_ == 28) {
    intTemp_[2] = s7Byte;  // output path id
    if (midici.ciType == MIDICI_DISCOVERY) {
      complete = true;
    }
  } else if (sysexPos_ == 29) {
    intTemp_[3] = s7Byte;  // fbIdx id
    if (midici.ciType == MIDICI_DISCOVERYREPLY) {
      complete = true;
    }
  }

  if (complete) {
    if (midici.ciType == MIDICI_DISCOVERY) {
        callbacks_.recvDiscoveryRequest(midici, {buffer[0], buffer[1], buffer[2]},
                             {buffer[3], buffer[4]}, {buffer[5], buffer[6]},
                             {buffer[7], buffer[8], buffer[9], buffer[10]},
                             static_cast<std::uint8_t>(intTemp_[0]), intTemp_[1],
                             static_cast<std::uint8_t>(intTemp_[2]));
    } else {
        callbacks_.recvDiscoveryReply(
          midici,
          {buffer[0], buffer[1], buffer[2]},
          {buffer[3], buffer[4]}, {buffer[5], buffer[6]},
          {buffer[7], buffer[8], buffer[9], buffer[10]},
          static_cast<std::uint8_t>(intTemp_[0]),
          intTemp_[1],
          static_cast<std::uint8_t>(intTemp_[2]),
          static_cast<std::uint8_t>(intTemp_[3])
        );
    }
  }
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::midiCI_ack_nak(std::uint8_t s7Byte) {
  bool complete = false;

  if (sysexPos_ == 13 && midici.ciVer == 1) {
    complete = true;
  } else if (sysexPos_ == 13 && midici.ciVer > 1) {
    intTemp_[0] = s7Byte;  // std::uint8_t origSubID,
  }

  if (sysexPos_ == 14) {
    intTemp_[1] = s7Byte;  // statusCode
  }

  if (sysexPos_ == 15) {
    intTemp_[2] = s7Byte;  // statusData
  }

  if (sysexPos_ >= 16 && sysexPos_ <= 20) {
    buffer[sysexPos_ - 16] = s7Byte;  // ackNakDetails
  }

  if (sysexPos_ == 21 || sysexPos_ == 22) {
    intTemp_[3] += s7Byte << (7 * (sysexPos_ - 21));
    return;
  }

  if (sysexPos_ >= 23 && sysexPos_ <= 23 + intTemp_[3]) {
    buffer[sysexPos_ - 23] = s7Byte;  // product ID
  }
  if (sysexPos_ == 23 + intTemp_[3]) {
    complete = true;
  }

  if (complete) {
    std::uint8_t ackNakDetails[5] = {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]};

    if (midici.ciType == MIDICI_NAK)
      callbacks_.recvNAK(midici, static_cast<std::uint8_t>(intTemp_[0]),
              static_cast<std::uint8_t>(intTemp_[1]),
              static_cast<std::uint8_t>(intTemp_[2]), ackNakDetails, intTemp_[3],
              buffer);

    if (midici.ciType == MIDICI_ACK && midici.ciVer > 1)
      callbacks_.recvACK(midici, static_cast<std::uint8_t>(intTemp_[0]),
              static_cast<std::uint8_t>(intTemp_[1]),
              static_cast<std::uint8_t>(intTemp_[2]), ackNakDetails, intTemp_[3],
              buffer);
  }
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::midiCI_endpoint_info_reply(std::uint8_t s7Byte) {
  bool complete = false;
  if (midici.ciVer < 2) {
    return;
  }
  if (sysexPos_ == 13) {
    intTemp_[0] = s7Byte;
  }
  if (sysexPos_ == 14 || sysexPos_ == 15) {
    intTemp_[1] += s7Byte << (7 * (sysexPos_ - 14));
    return;
  }
  if (sysexPos_ >= 16 && sysexPos_ <= 15 + intTemp_[1]) {
    buffer[sysexPos_ - 16] = s7Byte;  // Info Data
  }
  if (sysexPos_ == 16 + intTemp_[1]) {
    complete = true;
  }

  if (complete) {
    recvEndPointInfoReply(midici, static_cast<std::uint8_t>(intTemp_[0]), intTemp_[1], buffer);
  }
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::processMIDICI(std::uint8_t s7Byte) {
  // printf("s7 Byte %d\n", s7Byte);
  if (sysexPos_ == 3) {
    midici.ciType = s7Byte;
  }
  if (sysexPos_ == 4) {
    midici.ciVer = s7Byte;
  }
  if (sysexPos_ >= 5 && sysexPos_ <= 8) {
    midici.remoteMUID += static_cast<std::uint32_t>(s7Byte)
                         << (7 * (sysexPos_ - 5));
  }
  if (sysexPos_ >= 9 && sysexPos_ <= 12) {
    midici.localMUID += static_cast<std::uint32_t>(s7Byte)
                        << (7 * (sysexPos_ - 9));
  }
  if (sysexPos_ >= 12 && midici.localMUID != M2_CI_BROADCAST &&
      !callbacks_.checkMUID(midici.umpGroup, midici.localMUID)) {
    return;  // Not for this device
  }

  // break up each Process based on ciType
  if (sysexPos_ >= 12) {
    switch (midici.ciType) {
    case MIDICI_DISCOVERYREPLY:
    case MIDICI_DISCOVERY: this->midiCI_discovery_request_reply(s7Byte); break;

    case MIDICI_INVALIDATEMUID:  // MIDI-CI Invalidate MUID Message
      if (sysexPos_ >= 13 && sysexPos_ <= 16) {
        buffer[sysexPos_ - 13] = s7Byte;
      }

      // terminate MUID
      if (sysexPos_ == 16) {
        callbacks_.recvInvalidateMUID(
            midici, buffer[0] | (static_cast<std::uint32_t>(buffer[1]) << 7) |
                        (static_cast<std::uint32_t>(buffer[2]) << 14) |
                        (static_cast<std::uint32_t>(buffer[3]) << 21));
      }
      break;
    case MIDICI_ENDPOINTINFO:
      if (sysexPos_ == 13 && midici.ciVer > 1) {
        callbacks_.recvEndPointInfo(midici, s7Byte);  // std::uint8_t origSubID,
      }
      break;
    case MIDICI_ENDPOINTINFO_REPLY:
      this->midiCI_endpoint_info_reply(s7Byte);
      break;
    case MIDICI_ACK:
    case MIDICI_NAK: this->midiCI_ack_nak(s7Byte); break;

#ifdef M2_ENABLE_PROTOCOL
    case MIDICI_PROTOCOL_NEGOTIATION:
    case MIDICI_PROTOCOL_NEGOTIATION_REPLY:
    case MIDICI_PROTOCOL_SET:
    case MIDICI_PROTOCOL_TEST:
    case MIDICI_PROTOCOL_TEST_RESPONDER:
    case MIDICI_PROTOCOL_CONFIRM: processProtocolSysex(s7Byte); break;
#endif

#ifndef M2_DISABLE_PROFILE
    case MIDICI_PROFILE_INQUIRY:        // Profile Inquiry
    case MIDICI_PROFILE_INQUIRYREPLY:   // Reply to Profile Inquiry
    case MIDICI_PROFILE_SETON:          // Set Profile On Message
    case MIDICI_PROFILE_SETOFF:         // Set Profile Off Message
    case MIDICI_PROFILE_ENABLED:        // Set Profile Enabled Message
    case MIDICI_PROFILE_DISABLED:       // Set Profile Disabled Message
    case MIDICI_PROFILE_SPECIFIC_DATA:  // ProfileSpecific Data
    case MIDICI_PROFILE_DETAILS_INQUIRY:
    case MIDICI_PROFILE_DETAILS_REPLY: processProfileSysex(s7Byte); break;
#endif

#ifndef M2_DISABLE_PE
    case MIDICI_PE_CAPABILITY:       // Inquiry: Property Exchange Capabilities
    case MIDICI_PE_CAPABILITYREPLY:  // Reply to Property Exchange Capabilities
    case MIDICI_PE_GET:              // Inquiry: Get Property Data
    case MIDICI_PE_GETREPLY:         // Reply To Get Property Data - Needs Work!
    case MIDICI_PE_SET:              // Inquiry: Set Property Data
    case MIDICI_PE_SETREPLY:         // Reply To Inquiry: Set Property Data
    case MIDICI_PE_SUB:              // Inquiry: Subscribe Property Data
    case MIDICI_PE_SUBREPLY:         // Reply To Subscribe Property Data
    case MIDICI_PE_NOTIFY:           // Notify
      processPESysex(s7Byte);
      break;
#endif

#ifndef M2_DISABLE_PROCESSINQUIRY
    case MIDICI_PI_CAPABILITY:
    case MIDICI_PI_CAPABILITYREPLY:
    case MIDICI_PI_MM_REPORT:
    case MIDICI_PI_MM_REPORT_REPLY:
    case MIDICI_PI_MM_REPORT_END: processPISysex(s7Byte); break;
#endif
    default:
      callbacks_.recvUnknownMIDICI(midici, s7Byte);
      break;
    }
  }
  sysexPos_++;
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::processProtocolSysex(std::uint8_t s7Byte) {
  switch (midici.ciType) {
  case MIDICI_PROTOCOL_NEGOTIATION:
  case MIDICI_PROTOCOL_NEGOTIATION_REPLY: {
    // Authority Level
    if (sysexPos_ == 13) {
      intTemp_[0] = s7Byte;
    }
    // Number of Supported Protocols (np)
    if (sysexPos_ == 14) {
      intTemp_[1] = s7Byte;
    }

    int const protocolOffset = intTemp_[1] * 5 + 14;

    if (sysexPos_ >= 15 && sysexPos_ < protocolOffset) {
      std::uint8_t const pos = (sysexPos_ - 14) % 5;
      buffer[pos] = s7Byte;
      if (pos == 4) {
        std::uint8_t protocol[5] = {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]};
        callbacks_.recvProtocolAvailable(midici, static_cast<std::uint8_t>(intTemp_[0]), protocol);
      }
    }
    if (midici.ciVer > 1) {
      if (sysexPos_ >= protocolOffset && sysexPos_ <= protocolOffset + 5) {
        buffer[sysexPos_ - protocolOffset] = s7Byte;
      }
      if (sysexPos_ == protocolOffset + 5) {
        callbacks_.recvSetProtocolConfirm(midici, static_cast<std::uint8_t>(intTemp_[0]));
      }
    }
    break;
  }

  case MIDICI_PROTOCOL_SET:  // Set Profile On Message
    // Authority Level
    if (sysexPos_ == 13) {
      intTemp_[0] = s7Byte;
    }
    if (sysexPos_ >= 14 && sysexPos_ <= 18) {
      buffer[sysexPos_ - 13] = s7Byte;
    }
    if (sysexPos_ == 18) {
      std::uint8_t protocol[5] = {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]};
      callbacks_.recvSetProtocol(midici, static_cast<std::uint8_t>(intTemp_[0]), protocol);
    }
    break;

  case MIDICI_PROTOCOL_TEST_RESPONDER:
  case MIDICI_PROTOCOL_TEST:
    // Authority Level
    if (sysexPos_ == 13) {
      intTemp_[0] = s7Byte;
      intTemp_[1] = 1;
    }
    if (sysexPos_ >= 14 && sysexPos_ <= 61) {
      if (s7Byte != sysexPos_ - 14) {
        intTemp_[1] = 0;
      }
    }
    if (sysexPos_ == 61) {
      callbacks_.recvProtocolTest(midici, static_cast<std::uint8_t>(intTemp_[0]), !!(intTemp_[1]));
    }

    break;

  case MIDICI_PROTOCOL_CONFIRM:  // Set Profile Off Message
    // Authority Level
    if (sysexPos_ == 13) {
      intTemp_[0] = s7Byte;
      callbacks_.recvSetProtocolConfirm(midici, static_cast<std::uint8_t>(intTemp_[0]));
    }
    break;

  default: assert(false && "unknown protocol sysex type"); break;
  }
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::processProfileSysex(std::uint8_t s7Byte) {
  switch (midici.ciType) {
  case MIDICI_PROFILE_INQUIRY:  // Profile Inquiry
    if (sysexPos_ == 12) {
      callbacks_.recvProfileInquiry(midici);
    }
    break;
  case MIDICI_PROFILE_INQUIRYREPLY: {  // Reply to Profile Inquiry
    // Enabled Profiles Length
    if (sysexPos_ == 13 || sysexPos_ == 14) {
      intTemp_[0] += s7Byte << (7 * (sysexPos_ - 13));
    }

    // Disabled Profile Length
    int const enabledProfileOffset = intTemp_[0] * 5 + 13;
    if (sysexPos_ == enabledProfileOffset ||
        sysexPos_ == 1 + enabledProfileOffset) {
      intTemp_[1] += s7Byte << (7 * (sysexPos_ - enabledProfileOffset));
    }

    if (sysexPos_ >= 15 && sysexPos_ < enabledProfileOffset) {
      std::uint8_t const pos = (sysexPos_ - 13) % 5;
      buffer[pos] = s7Byte;
      if (pos == 4) {
        callbacks_.recvSetProfileEnabled(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]}, 0);
      }
    }

    if (sysexPos_ >= 2 + enabledProfileOffset &&
        sysexPos_ < enabledProfileOffset + intTemp_[1] * 5) {
      std::uint8_t const pos = (sysexPos_ - 13) % 5;
      buffer[pos] = s7Byte;
      if (pos == 4) {
        callbacks_.recvSetProfileDisabled(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]}, 0);
      }
    }
    break;
  }

  case MIDICI_PROFILE_ADD:
  case MIDICI_PROFILE_REMOVE:
  case MIDICI_PROFILE_ENABLED:
  case MIDICI_PROFILE_DISABLED:
  case MIDICI_PROFILE_SETOFF:
  case MIDICI_PROFILE_SETON: {  // Set Profile On Message
    bool complete = false;
    if (sysexPos_ >= 13 && sysexPos_ <= 17) {
      buffer[sysexPos_ - 13] = s7Byte;
    }
    if (sysexPos_ == 17 &&
        (midici.ciVer == 1 || midici.ciType == MIDICI_PROFILE_ADD ||
         midici.ciType == MIDICI_PROFILE_REMOVE)) {
      complete = true;
    }
    if (midici.ciVer > 1 && (sysexPos_ == 18 || sysexPos_ == 19)) {
      intTemp_[0] += s7Byte << (7 * (sysexPos_ - 18));
    }
    if (sysexPos_ == 19 && midici.ciVer > 1) {
      complete = true;
    }

    if (complete) {
      if (midici.ciType == MIDICI_PROFILE_ADD) {
        callbacks_.recvSetProfileDisabled( midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]}, 0);
      }
      if (midici.ciType == MIDICI_PROFILE_REMOVE) {
        callbacks_.recvSetProfileRemoved(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]});
      }
      if (midici.ciType == MIDICI_PROFILE_SETON) {
        callbacks_.recvSetProfileOn(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]}, static_cast<std::uint8_t>(intTemp_[0]));
      }
      if (midici.ciType == MIDICI_PROFILE_SETOFF) {
        callbacks_.recvSetProfileOff(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]});
      }
      if (midici.ciType == MIDICI_PROFILE_ENABLED) {
        callbacks_.recvSetProfileEnabled(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]}, static_cast<std::uint8_t>(intTemp_[0]));
      }
      if (midici.ciType == MIDICI_PROFILE_DISABLED) {
        callbacks_.recvSetProfileDisabled(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]}, static_cast<std::uint8_t>(intTemp_[0]));
      }
    }
    break;
  }

  case MIDICI_PROFILE_DETAILS_INQUIRY:
    if (sysexPos_ >= 13 && sysexPos_ <= 17) {
      buffer[sysexPos_ - 13] = s7Byte;
    }
    if (sysexPos_ == 18) {  // Inquiry Target
      callbacks_.recvSetProfileDetailsInquiry(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]}, s7Byte);
    }
    break;

  case MIDICI_PROFILE_DETAILS_REPLY: {
    if (sysexPos_ >= 13 && sysexPos_ <= 17) {
      buffer[sysexPos_ - 13] = s7Byte;
    }
    if (sysexPos_ == 18) {  // Inquiry Target
      buffer[5] = s7Byte;
    }

    if (sysexPos_ == 19 || sysexPos_ == 20) {  // Inquiry Target Data length (dl)
      intTemp_[0] += s7Byte << (7 * (sysexPos_ - 19));
    }

    if (sysexPos_ >= 21 && sysexPos_ <= 21 + intTemp_[0]) {
      buffer[sysexPos_ - 22 + 6] = s7Byte;  // product ID
    }

    if (sysexPos_ == 21 + intTemp_[0]) {
      callbacks_.recvSetProfileDetailsReply(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]}, buffer[5], intTemp_[0], &(buffer[6]));
    }

    break;
  }

  case MIDICI_PROFILE_SPECIFIC_DATA: {
    // Profile
    if (sysexPos_ >= 13 && sysexPos_ <= 17) {
      buffer[sysexPos_ - 13] = s7Byte;
      return;
    }
    if (sysexPos_ >= 18 &&
        sysexPos_ <= 21) {  // Length of Following Profile Specific Data
      intTemp_[0] += s7Byte << (7 * (sysexPos_ - 18));
      intTemp_[1] = 1;
      return;
    }

    //******************

    std::uint16_t const charOffset = (sysexPos_ - 22) % S7_BUFFERLEN;
    std::uint16_t const dataLength = intTemp_[0];
    if ((sysexPos_ >= 22 && sysexPos_ <= 21 + dataLength) || (dataLength == 0 && sysexPos_ == 21)) {
      if (dataLength != 0) {
        buffer[charOffset] = s7Byte;
      }

      if (charOffset == S7_BUFFERLEN - 1 || sysexPos_ == 21 + dataLength || dataLength == 0) {
        bool const lastByteOfSet = (sysexPos_ == 21 + dataLength);
        callbacks_.recvProfileSpecificData(midici, {buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]}, charOffset + 1, buffer, intTemp_[1], lastByteOfSet);
        intTemp_[1]++;
      }
    }

    //***********

  } break;

  default: break;
  }
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::processPESysex(std::uint8_t s7Byte) {
  switch (midici.ciType) {
  case MIDICI_PE_CAPABILITY:
  case MIDICI_PE_CAPABILITYREPLY: {
    bool complete = false;

    if (sysexPos_ == 13) {
      buffer[0] = s7Byte;
    }

    if (sysexPos_ == 13 && midici.ciVer == 1) {
      complete = true;
    }

    if (sysexPos_ == 14) {
      buffer[1] = s7Byte;
    }
    if (sysexPos_ == 15) {
      buffer[2] = s7Byte;
      complete = true;
    }

    if (complete) {
      if (midici.ciType == MIDICI_PE_CAPABILITY) {
        callbacks_.recvPECapabilities(midici, buffer[0], buffer[1], buffer[2]);
      }
      if (midici.ciType == MIDICI_PE_CAPABILITYREPLY) {
        callbacks_.recvPECapabilitiesReplies(midici, buffer[0], buffer[1], buffer[2]);
      }
    }

    break;
  }
  default: {
    if (sysexPos_ == 13) {
      midici._peReqIdx = std::make_tuple(midici.remoteMUID, s7Byte);
      midici.requestId = s7Byte;
      intTemp_[0] = 0;
      return;
    }

    if (sysexPos_ == 14 || sysexPos_ == 15) {  // header Length
      intTemp_[0] += s7Byte << (7 * (sysexPos_ - 14));
      return;
    }

    std::uint16_t const headerLength = intTemp_[0];

    if (sysexPos_ == 16 && midici.numChunk == 1) {
      peHeaderStr[midici._peReqIdx] = "";
    }

    if (sysexPos_ >= 16 && sysexPos_ <= 15 + headerLength) {
      std::uint16_t const charOffset = (sysexPos_ - 16);
      buffer[charOffset] = s7Byte;
      peHeaderStr[midici._peReqIdx].push_back(static_cast<char> (s7Byte));

      if (sysexPos_ == 15 + headerLength) {
        switch (midici.ciType) {
        case MIDICI_PE_GET:
          callbacks_.recvPEGetInquiry(midici, peHeaderStr[midici._peReqIdx]);
          cleanupRequest(midici._peReqIdx);
          break;
        case MIDICI_PE_SETREPLY:
          callbacks_.recvPESetReply(midici, peHeaderStr[midici._peReqIdx]);
          cleanupRequest(midici._peReqIdx);
          break;
        case MIDICI_PE_SUBREPLY:
          callbacks_.recvPESubReply(midici, peHeaderStr[midici._peReqIdx]);
          cleanupRequest(midici._peReqIdx);
          break;
        case MIDICI_PE_NOTIFY:
          callbacks_.recvPENotify(midici, peHeaderStr[midici._peReqIdx]);
          cleanupRequest(midici._peReqIdx);
          break;
        default: break;
        }
      }
    }

    if (sysexPos_ == 16 + headerLength || sysexPos_ == 17 + headerLength) {
      midici.totalChunks += s7Byte << (7 * (sysexPos_ - 16 - headerLength));
      return;
    }

    if (sysexPos_ == 18 + headerLength || sysexPos_ == 19 + headerLength) {
      midici.numChunk += s7Byte << (7 * (sysexPos_ - 18 - headerLength));
      return;
    }

    if (sysexPos_ == 20 + headerLength) {  // Body Length
      intTemp_[1] = s7Byte;
      return;
    }
    if (sysexPos_ == 21 + headerLength) {  // Body Length
      intTemp_[1] += s7Byte << 7;
    }

    std::uint16_t const bodyLength = intTemp_[1];
    std::uint16_t const initPos = 22 + headerLength;
    std::uint16_t const charOffset = (sysexPos_ - initPos) % S7_BUFFERLEN;

    if ((sysexPos_ >= initPos && sysexPos_ <= initPos - 1 + bodyLength) ||
        (bodyLength == 0 && sysexPos_ == initPos - 1)) {
      if (bodyLength != 0) {
        buffer[charOffset] = s7Byte;
      }

      bool const lastByteOfSet = (midici.numChunk == midici.totalChunks && sysexPos_ == initPos - 1 + bodyLength);
      bool const lastByteOfChunk = (bodyLength == 0 || sysexPos_ == initPos - 1 + bodyLength);

      if (charOffset == S7_BUFFERLEN - 1 || lastByteOfChunk) {
        if (midici.ciType == MIDICI_PE_GETREPLY) {
          callbacks_.recvPEGetReply(midici, peHeaderStr[midici._peReqIdx], charOffset + 1, buffer, lastByteOfChunk, lastByteOfSet);
        }
        if (midici.ciType == MIDICI_PE_SUB) {
          callbacks_.recvPESubInquiry(midici, peHeaderStr[midici._peReqIdx], charOffset + 1, buffer, lastByteOfChunk, lastByteOfSet);
        }
        if (midici.ciType == MIDICI_PE_SET) {
          callbacks_.recvPESetInquiry(midici, peHeaderStr[midici._peReqIdx], charOffset + 1, buffer, lastByteOfChunk, lastByteOfSet);
        }
        midici.partialChunkCount++;
      }

      if (lastByteOfSet) {
        cleanupRequest(midici._peReqIdx);
      }
    }
    break;
  }
  }
}

template <typename Callbacks>
void midiCIProcessor<Callbacks>::processPISysex(std::uint8_t s7Byte) {
  if (midici.ciVer == 1) {
    return;
  }

  switch (midici.ciType) {
  case MIDICI_PI_CAPABILITY:
    if (sysexPos_ == 12) {
      callbacks_.recvPICapabilities(midici);
    }
    break;
  case MIDICI_PI_CAPABILITYREPLY:
    if (sysexPos_ == 13) {
      callbacks_.recvPICapabilitiesReply(midici, s7Byte);
    }
    break;
  case MIDICI_PI_MM_REPORT_END:
    if (sysexPos_ == 12) {
      callbacks_.recvPIMMReportEnd(midici);
    }
    break;
  case MIDICI_PI_MM_REPORT: {
    if (sysexPos_ == 13) {  // MDC
      buffer[0] = s7Byte;
    }
    if (sysexPos_ == 14) {  // Bitmap of requested System Message Types
      buffer[1] = s7Byte;
    }
    if (sysexPos_ == 16) {  // Bitmap of requested Channel Controller Message Types
      buffer[2] = s7Byte;
    }
    if (sysexPos_ == 17) {
      callbacks_.recvPIMMReport(midici, buffer[0], buffer[1], buffer[2], s7Byte);
    }
    break;
  }
  case MIDICI_PI_MM_REPORT_REPLY: {
    if (sysexPos_ == 13) {  // Bitmap of requested System Message Types
      buffer[0] = s7Byte;
    }
    if (sysexPos_ == 15) {  // Bitmap of requested Channel Controller Message Types
      buffer[1] = s7Byte;
    }
    if (sysexPos_ == 16) {
      callbacks_.recvPIMMReportReply(midici, buffer[0], buffer[1], s7Byte);
    }
    break;
  }
  default:
    break;
  }
}



}  // end namespace midi2

#endif  // MIDI2_MIDICIPROCESSOR_H
