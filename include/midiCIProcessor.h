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

#ifndef MIDI2CPP_MIDICIPROCESSOR_H
#define MIDI2CPP_MIDICIPROCESSOR_H

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <tuple>

#include "utils.h"

using reqId = std::tuple<uint32_t, uint8_t>;  // muid-requestId

struct MIDICI {
  std::uint8_t umpGroup = 255;
  std::uint8_t deviceId = FUNCTION_BLOCK;
  std::uint8_t ciType = 255;
  std::uint8_t ciVer = 1;
  std::uint32_t remoteMUID = 0;
  std::uint32_t localMUID = 0;
  bool _reqTupleSet = false;
  reqId _peReqIdx;

  std::uint8_t totalChunks = 0;
  std::uint8_t numChunk = 0;
  std::uint8_t partialChunkCount = 0;
  std::uint8_t requestId = 255;
};

class midiCIProcessor {
public:
  using checkMUIDFn = std::function<bool(uint8_t group, uint32_t muid)>;
  using recvDiscoveryRequestFn = std::function<void(
      MIDICI ciDetails, std::array<uint8_t, 3> manuId,
      std::array<uint8_t, 2> familyId, std::array<uint8_t, 2> modelId,
      std::array<uint8_t, 4> version, uint8_t ciSupport, uint16_t maxSysex,
      uint8_t outputPathId)>;
  using recvDiscoveryReplyFn = std::function<void(
      MIDICI ciDetails, std::array<uint8_t, 3> manuId,
      std::array<uint8_t, 2> familyId, std::array<uint8_t, 2> modelId,
      std::array<uint8_t, 4> version, uint8_t ciSupport, uint16_t maxSysex,
      uint8_t outputPathId, uint8_t fbIdx)>;
  using recvEndPointInfoFn =
      std::function<void(MIDICI ciDetails, uint8_t status)>;
  using recvEndPointInfoReplyFn =
      std::function<void(MIDICI ciDetails, uint8_t status, uint16_t infoLength,
                         uint8_t* infoData)>;
  using recvNAKFn = std::function<void(
      MIDICI ciDetails, uint8_t origSubID, uint8_t statusCode,
      uint8_t statusData, uint8_t* ackNakDetails, uint16_t messageLength,
      uint8_t* ackNakMessage)>;
  using recvACKFn = std::function<void(
      MIDICI ciDetails, uint8_t origSubID, uint8_t statusCode,
      uint8_t statusData, uint8_t* ackNakDetails, uint16_t messageLength,
      uint8_t* ackNakMessage)>;
  using recvInvalidateMUIDFn =
      std::function<void(MIDICI ciDetails, uint32_t terminateMuid)>;
  using recvUnknownMIDICIFn =
      std::function<void(MIDICI ciDetails, uint8_t s7Byte)>;

  using recvProtocolAvailableFn = std::function<void(
      MIDICI ciDetails, uint8_t authorityLevel, uint8_t* protocol)>;
  using recvSetProtocolFn = std::function<void(
      MIDICI ciDetails, uint8_t authorityLevel, uint8_t* protocol)>;
  using recvSetProtocolConfirmFn =
      std::function<void(MIDICI ciDetails, uint8_t authorityLevel)>;
  using recvProtocolTestFn = std::function<void(
      MIDICI ciDetails, uint8_t authorityLevel, bool testDataAccurate)>;

  using recvProfileInquiryFn = std::function<void(MIDICI ciDetails)>;
  using recvSetProfileEnabledFn =
      std::function<void(MIDICI ciDetails, std::array<uint8_t, 5> profile,
                         uint8_t numberOfChannels)>;
  using recvSetProfileRemovedFn =
      std::function<void(MIDICI ciDetails, std::array<uint8_t, 5>)>;
  using recvSetProfileDisabledFn = std::function<void(
      MIDICI ciDetails, std::array<uint8_t, 5>, uint8_t numberOfChannels)>;
  using recvSetProfileOnFn =
      std::function<void(MIDICI ciDetails, std::array<uint8_t, 5> profile,
                         uint8_t numberOfChannels)>;
  using recvSetProfileOffFn =
      std::function<void(MIDICI ciDetails, std::array<uint8_t, 5> profile)>;
  using recvProfileSpecificDataFn = std::function<void(
      MIDICI ciDetails, std::array<uint8_t, 5> profile, uint16_t datalen,
      uint8_t* data, uint16_t part, bool lastByteOfSet)>;
  using recvSetProfileDetailsInquiryFn = std::function<void(
      MIDICI ciDetails, std::array<uint8_t, 5> profile, uint8_t InquiryTarget)>;
  using recvSetProfileDetailsReplyFn = std::function<void(
      MIDICI ciDetails, std::array<uint8_t, 5> profile, uint8_t InquiryTarget,
      uint16_t datalen, uint8_t* data)>;

  // Property Exchange
  using recvPECapabilitiesFn =
      std::function<void(MIDICI ciDetails, uint8_t numSimulRequests,
                         uint8_t majVer, uint8_t minVer)>;
  using recvPECapabilitiesRepliesFn =
      std::function<void(MIDICI ciDetails, uint8_t numSimulRequests,
                         uint8_t majVer, uint8_t minVer)>;
  using recvPEGetInquiryFn =
      std::function<void(MIDICI ciDetails, std::string requestDetails)>;
  using recvPESetReplyFn =
      std::function<void(MIDICI ciDetails, std::string requestDetails)>;
  using recvPESubReplyFn =
      std::function<void(MIDICI ciDetails, std::string requestDetails)>;
  using recvPENotifyFn =
      std::function<void(MIDICI ciDetails, std::string requestDetails)>;
  using recvPEGetReplyFn = std::function<void(
      MIDICI ciDetails, std::string requestDetails, uint16_t bodyLen,
      uint8_t* body, bool lastByteOfChunk, bool lastByteOfSet)>;
  using recvPESetInquiryFn = std::function<void(
      MIDICI ciDetails, std::string requestDetails, uint16_t bodyLen,
      uint8_t* body, bool lastByteOfChunk, bool lastByteOfSet)>;
  using recvPESubInquiryFn = std::function<void(
      MIDICI ciDetails, std::string requestDetails, uint16_t bodyLen,
      uint8_t* body, bool lastByteOfChunk, bool lastByteOfSet)>;

  // Process Inquiry
  using recvPICapabilitiesFn = std::function<void(MIDICI ciDetails)>;
  using recvPICapabilitiesReplyFn =
      std::function<void(MIDICI ciDetails, uint8_t supportedFeatures)>;
  using recvPIMMReportFn =
      std::function<void(MIDICI ciDetails, uint8_t MDC, uint8_t systemBitmap,
                         uint8_t chanContBitmap, uint8_t chanNoteBitmap)>;
  using recvPIMMReportReplyFn =
      std::function<void(MIDICI ciDetails, uint8_t systemBitmap,
                         uint8_t chanContBitmap, uint8_t chanNoteBitmap)>;
  using recvPIMMReportEndFn = std::function<void(MIDICI ciDetails)>;

  // EB: update callbacks step2 - update setCallback functions:
  // void setCallback(std::function<void(params)> fptr){ pointerName = fptr; }
  //
  // Calling these functions from within a member class looks like:
  // MIDICIHandler->setCheckMUID(std::bind(&YourClass::checkMUID, this,
  // std::placeholders::_1, std::placeholders::_2));

  void setCheckMUID(checkMUIDFn fptr) { checkMUID = fptr; }
  void endSysex7();
  void startSysex7(uint8_t group, uint8_t deviceId);
  void processMIDICI(uint8_t s7Byte);

  void setRecvDiscovery(recvDiscoveryRequestFn fptr) {
    recvDiscoveryRequest = fptr;
  }
  void setRecvDiscoveryReply(recvDiscoveryReplyFn fptr) {
    recvDiscoveryReply = fptr;
  }
  void setRecvNAK(recvNAKFn fptr) { recvNAK = fptr; }
  void setRecvACK(recvACKFn fptr) { recvACK = fptr; }
  void setRecvInvalidateMUID(recvInvalidateMUIDFn fptr) {
    recvInvalidateMUID = fptr;
  }
  void setRecvUnknownMIDICI(recvUnknownMIDICIFn fptr) {
    recvUnknownMIDICI = fptr;
  }

  void setRecvEndpointInfo(recvEndPointInfoFn fptr) { recvEndPointInfo = fptr; }
  void setRecvEndpointInfoReply(recvEndPointInfoReplyFn fptr) {
    recvEndPointInfoReply = fptr;
  }

  // Protocol Negotiation
  void setRecvProtocolAvailable(recvProtocolAvailableFn fptr) {
    recvProtocolAvailable = fptr;
  }
  void setRecvSetProtocol(recvSetProtocolFn fptr) { recvSetProtocol = fptr; }
  void setRecvSetProtocolConfirm(recvSetProtocolConfirmFn fptr) {
    recvSetProtocolConfirm = fptr;
  }
  void setRecvSetProtocolTest(recvProtocolTestFn fptr) {
    recvProtocolTest = fptr;
  }

  // Profiles
  void setRecvProfileInquiry(recvProfileInquiryFn fptr) {
    recvProfileInquiry = fptr;
  }
  void setRecvProfileEnabled(recvSetProfileEnabledFn fptr) {
    recvSetProfileEnabled = fptr;
  }
  void setRecvSetProfileRemoved(recvSetProfileRemovedFn fptr) {
    recvSetProfileRemoved = fptr;
  }
  void setRecvProfileDisabled(recvSetProfileDisabledFn fptr) {
    recvSetProfileDisabled = fptr;
  }
  void setRecvProfileOn(recvSetProfileOnFn fptr) { recvSetProfileOn = fptr; }
  void setRecvProfileOff(recvSetProfileOffFn fptr) { recvSetProfileOff = fptr; }
  void setRecvProfileSpecificData(recvProfileSpecificDataFn fptr) {
    recvProfileSpecificData = fptr;
  }
  void setRecvProfileDetailsInquiry(recvSetProfileDetailsInquiryFn fptr) {
    recvSetProfileDetailsInquiry = fptr;
  }
  void setRecvProfileDetailsReply(recvSetProfileDetailsReplyFn fptr) {
    recvSetProfileDetailsReply = fptr;
  }

  // Property Exchange
  void setPECapabilities(recvPECapabilitiesFn fptr) {
    recvPECapabilities = fptr;
  }
  void setPECapabilitiesReply(recvPECapabilitiesRepliesFn fptr) {
    recvPECapabilitiesReplies = fptr;
  }
  void setRecvPEGetInquiry(recvPEGetInquiryFn fptr) { recvPEGetInquiry = fptr; }
  void setRecvPESetReply(recvPESetReplyFn fptr) { recvPESetReply = fptr; }
  void setRecvPESubReply(recvPESubReplyFn fptr) { recvPESubReply = fptr; }
  void setRecvPENotify(recvPENotifyFn fptr) { recvPENotify = fptr; }
  void setRecvPEGetReply(recvPEGetReplyFn fptr) { recvPEGetReply = fptr; }
  void setRecvPESetInquiry(recvPESetInquiryFn fptr) { recvPESetInquiry = fptr; }
  void setRecvPESubInquiry(recvPESubInquiryFn fptr) { recvPESubInquiry = fptr; }

  // Process Inquiry
  void setRecvPICapabilities(recvPICapabilitiesFn fptr) {
    recvPICapabilities = fptr;
  }
  void setRecvPICapabilitiesReply(recvPICapabilitiesReplyFn fptr) {
    recvPICapabilitiesReply = fptr;
  }
  void setRecvPIMMReport(recvPIMMReportFn fptr) { recvPIMMReport = fptr; }
  void setRecvPIMMReportReply(recvPIMMReportReplyFn fptr) {
    recvPIMMReportReply = fptr;
  }
  void setRecvPIMMEnd(recvPIMMReportEndFn fptr) { recvPIMMReportEnd = fptr; }

private:
  MIDICI midici;
  uint8_t buffer[256];
  /*
   * in Discovery this is
   * [sysexID1,sysexID2,sysexID3,famId1,famid2,modelId1,modelId2,ver1,ver2,ver3,ver4,...product
   * Id] in Profiles this is [pf1, pf1, pf3, pf4, pf5] in Protocols this is
   * [pr1, pr2, pr3, pr4, pr5]
   */

  uint16_t intTemp[4];
  /* in Discovery this is [ciSupport, maxSysex, output path id]
   * in Profile Inquiry Reply, this is [Enabled Profiles Length, Disabled
   * Profile Length] in Profile On/Off/Enabled/Disabled, this is
   * [numOfChannels] in PE this is [header length, Body Length]
   */
  uint16_t sysexPos;

  // MIDI-CI  callbacks

  // EB: update callbacks step1 - update pointer definitions to:
  // std::function<void(..params..)> name = nullptr;

  checkMUIDFn checkMUID = nullptr;
  recvDiscoveryRequestFn recvDiscoveryRequest = nullptr;
  recvDiscoveryReplyFn recvDiscoveryReply = nullptr;
  recvEndPointInfoFn recvEndPointInfo = nullptr;
  recvEndPointInfoReplyFn recvEndPointInfoReply = nullptr;
  recvNAKFn recvNAK = nullptr;
  recvACKFn recvACK = nullptr;
  recvInvalidateMUIDFn recvInvalidateMUID = nullptr;
  recvUnknownMIDICIFn recvUnknownMIDICI = nullptr;

  // Protocol Negotiation
  recvProtocolAvailableFn recvProtocolAvailable = nullptr;
  recvSetProtocolFn recvSetProtocol = nullptr;
  recvSetProtocolConfirmFn recvSetProtocolConfirm = nullptr;
  recvProtocolTestFn recvProtocolTest = nullptr;

  void processProtocolSysex(uint8_t s7Byte);

  // Profiles
  recvProfileInquiryFn recvProfileInquiry = nullptr;
  recvSetProfileEnabledFn recvSetProfileEnabled = nullptr;
  recvSetProfileRemovedFn recvSetProfileRemoved = nullptr;
  recvSetProfileDisabledFn recvSetProfileDisabled = nullptr;
  recvSetProfileOnFn recvSetProfileOn = nullptr;
  recvSetProfileOffFn recvSetProfileOff = nullptr;
  recvProfileSpecificDataFn recvProfileSpecificData = nullptr;
  recvSetProfileDetailsInquiryFn recvSetProfileDetailsInquiry = nullptr;
  recvSetProfileDetailsReplyFn recvSetProfileDetailsReply = nullptr;

  void processProfileSysex(uint8_t s7Byte);

  // Property Exchange
  std::map<reqId, std::string> peHeaderStr;

  recvPECapabilitiesFn recvPECapabilities = nullptr;
  recvPECapabilitiesRepliesFn recvPECapabilitiesReplies = nullptr;
  recvPEGetInquiryFn recvPEGetInquiry = nullptr;
  recvPESetReplyFn recvPESetReply = nullptr;
  recvPESubReplyFn recvPESubReply = nullptr;
  recvPENotifyFn recvPENotify = nullptr;
  recvPEGetReplyFn recvPEGetReply = nullptr;
  recvPESetInquiryFn recvPESetInquiry = nullptr;
  recvPESubInquiryFn recvPESubInquiry = nullptr;

  void cleanupRequest(reqId peReqIdx);

  void processPESysex(uint8_t s7Byte);

  // Process Inquiry
  recvPICapabilitiesFn recvPICapabilities = nullptr;
  recvPICapabilitiesReplyFn recvPICapabilitiesReply = nullptr;
  recvPIMMReportFn recvPIMMReport = nullptr;
  recvPIMMReportReplyFn recvPIMMReportReply = nullptr;
  recvPIMMReportEndFn recvPIMMReportEnd = nullptr;

  void processPISysex(uint8_t s7Byte);
};

#endif  // MIDI2CPP_MIDICIPROCESSOR_H
