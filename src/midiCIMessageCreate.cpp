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

void setBytesFromNumbers(uint8_t *message, uint32_t number, uint16_t *start,
                         uint8_t amount) {
  for (int amountC = amount; amountC > 0; amountC--) {
    message[(*start)++] = number & 127;
    number = number >> 7;
  }
}

void createCIHeader(uint8_t *sysexHeader, uint8_t deviceId, ci_message ciType, uint8_t ciVer, uint32_t localMUID,
                    uint32_t remoteMUID) {
  sysexHeader[0] = static_cast<std::uint8_t>(midi2::S7UNIVERSAL_NRT);
  sysexHeader[1] = deviceId;
  sysexHeader[2] = static_cast<std::uint8_t>(midi2::S7MIDICI);
  sysexHeader[3] = static_cast<std::uint8_t>(ciType);
  sysexHeader[4] = ciVer;
  uint16_t length = 5;
  setBytesFromNumbers(sysexHeader, localMUID, &length, 4);
  setBytesFromNumbers(sysexHeader, remoteMUID, &length, 4);
}

}  // end anonymous namespace

namespace midi2 {

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
