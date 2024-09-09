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

#include "midi2/midiCIMessageCreate.hpp"

#include <array>
#include <cstdint>

#include "midi2/utils.hpp"

using namespace midi2;

namespace {

constexpr auto S7UNIVERSAL_NRT = 0x7E;
// constexpr auto S7UNIVERSAL_RT = 0x7F;
constexpr auto S7MIDICI = 0x0D;

void setBytesFromNumbers(uint8_t *message, uint32_t number, uint16_t *start,
                         uint8_t amount) {
  for (int amountC = amount; amountC > 0; amountC--) {
    message[(*start)++] = number & 127;
    number = number >> 7;
  }
}

void concatSysexArray(uint8_t *sysex, uint16_t *start, uint8_t const *add,
                      uint16_t len) {
  uint16_t i;
  for (i = 0; i < len; i++) {
    sysex[(*start)++] = add[i];
  }
}

void createCIHeader(uint8_t *sysexHeader, uint8_t deviceId, ci_message ciType, uint8_t ciVer, uint32_t localMUID,
                    uint32_t remoteMUID) {
  sysexHeader[0] = S7UNIVERSAL_NRT;
  sysexHeader[1] = deviceId;
  sysexHeader[2] = S7MIDICI;
  sysexHeader[3] = static_cast<std::uint8_t>(ciType);
  sysexHeader[4] = ciVer;
  uint16_t length = 5;
  setBytesFromNumbers(sysexHeader, localMUID, &length, 4);
  setBytesFromNumbers(sysexHeader, remoteMUID, &length, 4);
}

uint16_t sendPEWithBody(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMUID, uint8_t requestId,
                        uint16_t headerLen, uint8_t *header, uint16_t numberOfChunks, uint16_t numberOfThisChunk,
                        uint16_t bodyLength, uint8_t *body, ci_message ciType) {
  createCIHeader(sysex, 0x7f, ciType, midiCIVer, srcMUID, destMUID);
  sysex[13] = requestId;
  uint16_t length = 14;
  setBytesFromNumbers(sysex, headerLen, &length, 2);
  concatSysexArray(sysex, &length, header, headerLen);
  setBytesFromNumbers(sysex, numberOfChunks, &length, 2);
  setBytesFromNumbers(sysex, numberOfThisChunk, &length, 2);
  setBytesFromNumbers(sysex, bodyLength, &length, 2);
  concatSysexArray(sysex, &length, body, bodyLength);
  return length;
}

uint16_t sendProfileMessage(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMUID, uint8_t destination,
                            std::array<uint8_t, 5> profile, uint8_t numberOfChannels, ci_message ciType) {
  createCIHeader(sysex, destination, ciType, midiCIVer, srcMUID, destMUID);
  uint16_t length = 13;
  concatSysexArray(sysex, &length, profile.data(), 5);
  if (midiCIVer == 1 || ciType == ci_message::profile_added || ciType == ci_message::profile_removed) {
    return length;
  }
  setBytesFromNumbers(sysex, numberOfChannels, &length, 2);
  return length;
}

uint16_t sendPEHeaderOnly(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMUID, uint8_t requestId,
                          uint16_t headerLen, uint8_t *header, ci_message ciType) {
  createCIHeader(sysex, 0x7F, ciType, midiCIVer, srcMUID, destMUID);
  sysex[13] = requestId;
  uint16_t length = 14;
  setBytesFromNumbers(sysex, headerLen, &length, 2);
  concatSysexArray(sysex, &length, header, headerLen);
  setBytesFromNumbers(sysex, 1, &length, 2);
  setBytesFromNumbers(sysex, 1, &length, 2);
  setBytesFromNumbers(sysex, 0, &length, 2);
  return length;
}

}  // end anonymous namespace

namespace midi2 {

// Profiles

uint16_t CIMessage::sendProfileListRequest(uint8_t *sysex, uint8_t midiCIVer,
                                           uint32_t srcMUID, uint32_t destMUID,
                                           uint8_t destination) {
  createCIHeader(sysex, destination, ci_message::profile_inquiry, midiCIVer, srcMUID, destMUID);
  return 13;
}

uint16_t CIMessage::sendProfileListResponse(
    uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMUID,
    uint8_t destination, uint8_t profilesEnabledLen, uint8_t *profilesEnabled,
    uint8_t profilesDisabledLen, uint8_t *profilesDisabled) {
  createCIHeader(sysex, destination, ci_message::profile_inquiry_reply, midiCIVer, srcMUID, destMUID);
  uint16_t length = 13;
  setBytesFromNumbers(sysex, profilesEnabledLen, &length, 2);
  concatSysexArray(sysex, &length, profilesEnabled, profilesEnabledLen * 5);
  setBytesFromNumbers(sysex, profilesDisabledLen, &length, 2);
  concatSysexArray(sysex, &length, profilesDisabled, profilesDisabledLen * 5);
  return length;
}

uint16_t CIMessage::sendProfileAdd(uint8_t *sysex, uint8_t midiCIVer,
                                   uint32_t srcMUID, uint32_t destMUID,
                                   uint8_t destination,
                                   std::array<uint8_t, 5> profile) {
  return sendProfileMessage(sysex, midiCIVer, srcMUID, destMUID, destination, profile, 0, ci_message::profile_added);
}

uint16_t CIMessage::sendProfileRemove(uint8_t *sysex, uint8_t midiCIVer,
                                      uint32_t srcMUID, uint32_t destMUID,
                                      uint8_t destination,
                                      std::array<uint8_t, 5> profile) {
  return sendProfileMessage(sysex, midiCIVer, srcMUID, destMUID, destination, profile, 0, ci_message::profile_removed);
}

uint16_t CIMessage::sendProfileOn(uint8_t *sysex, uint8_t midiCIVer,
                                  uint32_t srcMUID, uint32_t destMUID,
                                  uint8_t destination,
                                  std::array<uint8_t, 5> profile,
                                  uint8_t numberOfChannels) {
  return sendProfileMessage(sysex, midiCIVer, srcMUID, destMUID, destination, profile, numberOfChannels,
                            ci_message::profile_set_on);
}

uint16_t CIMessage::sendProfileOff(uint8_t *sysex, uint8_t midiCIVer,
                                   uint32_t srcMUID, uint32_t destMUID,
                                   uint8_t destination,
                                   std::array<uint8_t, 5> profile) {
  return sendProfileMessage(sysex, midiCIVer, srcMUID, destMUID, destination, profile, 0, ci_message::profile_set_off);
}

uint16_t CIMessage::sendProfileEnabled(uint8_t *sysex, uint8_t midiCIVer,
                                       uint32_t srcMUID, uint32_t destMUID,
                                       uint8_t destination,
                                       std::array<uint8_t, 5> profile,
                                       uint8_t numberOfChannels) {
  return sendProfileMessage(sysex, midiCIVer, srcMUID, destMUID, destination, profile, numberOfChannels,
                            ci_message::profile_enabled);
}

uint16_t CIMessage::sendProfileDisabled(uint8_t *sysex, uint8_t midiCIVer,
                                        uint32_t srcMUID, uint32_t destMUID,
                                        uint8_t destination,
                                        std::array<uint8_t, 5> profile,
                                        uint8_t numberOfChannels) {
  return sendProfileMessage(sysex, midiCIVer, srcMUID, destMUID, destination, profile, numberOfChannels,
                            ci_message::profile_disabled);
}

uint16_t CIMessage::sendProfileSpecificData(uint8_t *sysex, uint8_t midiCIVer,
                                            uint32_t srcMUID, uint32_t destMUID,
                                            uint8_t destination,
                                            std::array<uint8_t, 5> profile,
                                            uint16_t datalen, uint8_t *data) {
  createCIHeader(sysex, destination, ci_message::profile_specific_data, midiCIVer, srcMUID, destMUID);
  uint16_t length = 13;
  concatSysexArray(sysex, &length, profile.data(), 5);
  setBytesFromNumbers(sysex, datalen, &length, 4);
  concatSysexArray(sysex, &length, data, datalen);
  return length;
}

uint16_t CIMessage::sendProfileDetailsInquiry(uint8_t *sysex, uint8_t midiCIVer,
                                              uint32_t srcMUID,
                                              uint32_t destMUID,
                                              uint8_t destination,
                                              std::array<uint8_t, 5> profile,
                                              uint8_t InquiryTarget) {
  if (midiCIVer < 2)
    return 0;
  createCIHeader(sysex, destination, ci_message::profile_details_inquiry, midiCIVer, srcMUID, destMUID);
  uint16_t length = 13;
  concatSysexArray(sysex, &length, profile.data(), 5);
  sysex[length++] = InquiryTarget;
  return length;
}

uint16_t CIMessage::sendProfileDetailsReply(uint8_t *sysex, uint8_t midiCIVer,
                                            uint32_t srcMUID, uint32_t destMUID,
                                            uint8_t destination,
                                            std::array<uint8_t, 5> profile,
                                            uint8_t InquiryTarget,
                                            uint16_t datalen, uint8_t *data) {
  if (midiCIVer < 2)
    return 0;
  createCIHeader(sysex, destination, ci_message::profile_details_reply, midiCIVer, srcMUID, destMUID);
  uint16_t length = 13;
  concatSysexArray(sysex, &length, profile.data(), 5);
  sysex[length++] = InquiryTarget;
  setBytesFromNumbers(sysex, datalen, &length, 2);
  concatSysexArray(sysex, &length, data, datalen);
  return length;
}

// Property Exchange

uint16_t CIMessage::sendPECapabilityRequest(uint8_t *sysex, uint8_t midiCIVer,
                                            uint32_t srcMUID, uint32_t destMUID,
                                            uint8_t numSimulRequests,
                                            uint8_t majVer, uint8_t minVer) {
  createCIHeader(sysex, 0x7F, ci_message::pe_capability, midiCIVer, srcMUID, destMUID);
  sysex[13] = numSimulRequests;
  if (midiCIVer == 1) {
    return 14;
  }
  sysex[14] = majVer;
  sysex[15] = minVer;
  return 16;
}

uint16_t CIMessage::sendPECapabilityReply(uint8_t *sysex, uint8_t midiCIVer,
                                          uint32_t srcMUID, uint32_t destMUID,
                                          uint8_t numSimulRequests,
                                          uint8_t majVer, uint8_t minVer) {
  createCIHeader(sysex, 0x7F, ci_message::pe_capability_reply, midiCIVer, srcMUID, destMUID);
  sysex[13] = numSimulRequests;
  if (midiCIVer == 1) {
    return 14;
  }
  sysex[14] = majVer;
  sysex[15] = minVer;
  return 16;
}

uint16_t CIMessage::sendPESub(uint8_t *sysex, uint8_t midiCIVer,
                              uint32_t srcMUID, uint32_t destMUID,
                              uint8_t requestId, uint16_t headerLen,
                              uint8_t *header, uint16_t numberOfChunks,
                              uint16_t numberOfThisChunk, uint16_t bodyLength,
                              uint8_t *body) {
  return sendPEWithBody(sysex, midiCIVer, srcMUID, destMUID, requestId, headerLen, header, numberOfChunks,
                        numberOfThisChunk, bodyLength, body, ci_message::pe_sub);
}

uint16_t CIMessage::sendPESet(uint8_t *sysex, uint8_t midiCIVer,
                              uint32_t srcMUID, uint32_t destMUID,
                              uint8_t requestId, uint16_t headerLen,
                              uint8_t *header, uint16_t numberOfChunks,
                              uint16_t numberOfThisChunk, uint16_t bodyLength,
                              uint8_t *body) {
  return sendPEWithBody(sysex, midiCIVer, srcMUID, destMUID, requestId, headerLen, header, numberOfChunks,
                        numberOfThisChunk, bodyLength, body, ci_message::pe_set);
}

uint16_t CIMessage::sendPEGetReply(uint8_t *sysex, uint8_t midiCIVer,
                                   uint32_t srcMUID, uint32_t destMUID,
                                   uint8_t requestId, uint16_t headerLen,
                                   uint8_t *header, uint16_t numberOfChunks,
                                   uint16_t numberOfThisChunk,
                                   uint16_t bodyLength, uint8_t *body) {
  return sendPEWithBody(sysex, midiCIVer, srcMUID, destMUID, requestId, headerLen, header, numberOfChunks,
                        numberOfThisChunk, bodyLength, body, ci_message::pe_get_reply);
}

uint16_t CIMessage::sendPEGet(uint8_t *sysex, uint8_t midiCIVer,
                              uint32_t srcMUID, uint32_t destMUID,
                              uint8_t requestId, uint16_t headerLen,
                              uint8_t *header) {
  return sendPEHeaderOnly(sysex, midiCIVer, srcMUID, destMUID, requestId, headerLen, header, ci_message::pe_get);
}

uint16_t CIMessage::sendPESubReply(uint8_t *sysex, uint8_t midiCIVer,
                                   uint32_t srcMUID, uint32_t destMUID,
                                   uint8_t requestId, uint16_t headerLen,
                                   uint8_t *header) {
  return sendPEHeaderOnly(sysex, midiCIVer, srcMUID, destMUID, requestId, headerLen, header, ci_message::pe_sub_reply);
}

uint16_t CIMessage::sendPENotify(uint8_t *sysex, uint8_t midiCIVer,
                                 uint32_t srcMUID, uint32_t destMUID,
                                 uint8_t requestId, uint16_t headerLen,
                                 uint8_t *header) {
  return sendPEHeaderOnly(sysex, midiCIVer, srcMUID, destMUID, requestId, headerLen, header, ci_message::pe_notify);
}

uint16_t CIMessage::sendPESetReply(uint8_t *sysex, uint8_t midiCIVer,
                                   uint32_t srcMUID, uint32_t destMUID,
                                   uint8_t requestId, uint16_t headerLen,
                                   uint8_t *header) {
  return sendPEHeaderOnly(sysex, midiCIVer, srcMUID, destMUID, requestId, headerLen, header, ci_message::pe_set_reply);
}

// Process Inquiry
uint16_t CIMessage::sendPICapabilityRequest(uint8_t *sysex, uint8_t midiCIVer,
                                            uint32_t srcMUID,
                                            uint32_t destMUID) {
  if (midiCIVer == 1)
    return 0;
  createCIHeader(sysex, 0x7F, ci_message::pi_capability, midiCIVer, srcMUID, destMUID);
  return 13;
}

uint16_t CIMessage::sendPICapabilityReply(uint8_t *sysex, uint8_t midiCIVer,
                                          uint32_t srcMUID, uint32_t destMUID,
                                          uint8_t supportedFeatures) {
  createCIHeader(sysex, 0x7F, ci_message::pi_capability_reply, midiCIVer, srcMUID, destMUID);
  sysex[13] = supportedFeatures;
  return 14;
}

uint16_t CIMessage::sendPIMMReport(uint8_t *sysex, uint8_t midiCIVer,
                                   uint32_t srcMUID, uint32_t destMUID,
                                   uint8_t destination, uint8_t MDC,
                                   uint8_t systemBitmap, uint8_t chanContBitmap,
                                   uint8_t chanNoteBitmap) {
  createCIHeader(sysex, destination, ci_message::pi_mm_report, midiCIVer, srcMUID, destMUID);
  sysex[13] = MDC;
  sysex[14] = systemBitmap;
  sysex[15] = 0;
  sysex[16] = chanContBitmap;
  sysex[17] = chanNoteBitmap;
  return 18;
}

uint16_t CIMessage::sendPIMMReportReply(uint8_t *sysex, uint8_t midiCIVer,
                                        uint32_t srcMUID, uint32_t destMUID,
                                        uint8_t destination,
                                        uint8_t systemBitmap,
                                        uint8_t chanContBitmap,
                                        uint8_t chanNoteBitmap) {
  createCIHeader(sysex, destination, ci_message::pi_mm_report_reply, midiCIVer, srcMUID, destMUID);
  sysex[13] = systemBitmap;
  sysex[14] = 0;
  sysex[15] = chanContBitmap;
  sysex[16] = chanNoteBitmap;
  return 17;
}

uint16_t CIMessage::sendPIMMReportEnd(uint8_t *sysex, uint8_t midiCIVer,
                                      uint32_t srcMUID, uint32_t destMUID,
                                      uint8_t destination) {
  createCIHeader(sysex, destination, ci_message::pi_mm_report_end, midiCIVer, srcMUID, destMUID);
  return 13;
}

}  // end namespace midi2
