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
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <version>

#include "midi2/ci_types.h"
#include "midi2/utils.h"

namespace midi2 {

// unreachable
// ~~~~~~~~~~~
#if defined(__cpp_lib_unreachable)
/// Executing unreachable() results in undefined behavior.
///
/// An implementation may, for example, optimize impossible code branches away
/// or trap to prevent further execution.
[[noreturn, maybe_unused]] inline void unreachable() {
  assert(false && "unreachable");
  std::unreachable();
}
#elif defined(__GNUC__)  // GCC 4.8+, Clang, Intel and other compilers
[[noreturn]] inline __attribute__((always_inline)) void unreachable() {
  assert(false && "unreachable");
  __builtin_unreachable();
}
#elif defined(_MSC_VER)
[[noreturn, maybe_unused]] __forceinline void unreachable() {
  assert(false && "unreachable");
  __assume(false);
}
#else
// Unknown compiler so no extension is used, Undefined behavior is still raised
// by an empty function body and the noreturn attribute.
[[noreturn, maybe_unused]] inline void unreachable() {
  assert(false && "unreachable");
}
#endif

using MIDICI = ci::MIDICI;

using profile_span = std::span<std::byte, 5>;
using device_manufacturer = std::span<std::byte, 3>;
using device_family = std::uint16_t;
using device_model = std::uint16_t;
using device_version = std::uint32_t;
constexpr auto bytes = std::span<std::byte>{};

template <typename T> concept ci_backend = requires(T && v) {
  { v.check_muid(std::uint8_t{} /*group*/, std::uint32_t{} /*muid*/) } -> std::convertible_to<bool>;

  { v.discovery(MIDICI{}, ci::discovery_current{}) } -> std::same_as<void>;
  { v.discovery_reply(MIDICI{}, ci::discovery_reply_current{}) } -> std::same_as<void>;
  { v.end_point_info(MIDICI{}, std::byte{}) } -> std::same_as<void>;
  {
    v.end_point_info_reply(MIDICI{}, std::uint8_t{}, std::uint16_t{} /*infoLength*/, std::span<std::byte>{})
  } -> std::same_as<void>;
  {
    v.ack(MIDICI{}, std::uint8_t{} /*origSubID*/, std::uint8_t{} /*statusCode*/, std::uint8_t{} /*statusData*/,
          std::span<std::byte, 5>{bytes} /*ackNakDetails*/, std::uint16_t{} /*messageLength*/,
          std::span<std::byte>{} /*ackNakMessage*/)
  } -> std::same_as<void>;
  {
    v.nak(MIDICI{}, std::uint8_t{} /*origSubID*/, std::uint8_t{} /*statusCode*/, std::uint8_t{} /*statusData*/,
          std::span<std::byte, 5>{bytes} /*ackNakDetails*/, std::uint16_t{} /*messageLength*/,
          std::span<std::byte>{} /*ackNakMessage*/)
  } -> std::same_as<void>;
  { v.invalidate_muid(MIDICI{}, std::uint32_t{}) } -> std::same_as<void>;

  { v.unknown_midici(MIDICI{}, std::byte{}) } -> std::same_as<void>;

  // Protocol Negotiation
  {
    v.protocol_available(MIDICI{}, std::uint8_t{} /*authority_level*/, std::span<std::byte, 5>{bytes} /*protocol*/)
  } -> std::same_as<void>;
  {
    v.recvSetProtocol(MIDICI{}, std::uint8_t{} /*authority_level*/, std::span<std::byte, 5>{bytes} /*protocol*/)
  } -> std::same_as<void>;
  { v.recvSetProtocolConfirm(MIDICI{}, std::uint8_t{} /*authorityLevel*/) } -> std::same_as<void>;
  {
    v.recvProtocolTest(MIDICI{}, std::uint8_t{} /*authorityLevel*/, bool{} /*testDataAccurate*/)
  } -> std::same_as<void>;

  // Profiles
  { v.recvProfileInquiry(MIDICI{}) } -> std::same_as<void>;
  { v.recvSetProfileEnabled(MIDICI{}, profile_span{bytes}, std::uint8_t{}) } -> std::same_as<void>;
  { v.recvSetProfileRemoved(MIDICI{}, profile_span{bytes}) } -> std::same_as<void>;
  { v.recvSetProfileDisabled(MIDICI{}, profile_span{bytes}, std::uint8_t{}) } -> std::same_as<void>;
  { v.recvSetProfileOn(MIDICI{}, profile_span{bytes}, std::uint8_t{}) } -> std::same_as<void>;
  { v.recvSetProfileOff(MIDICI{}, profile_span{bytes}) } -> std::same_as<void>;
  {
    v.profile_specific_data(MIDICI{}, profile_span{bytes}, std::span<std::byte>{}, std::uint16_t{} /*part*/,
                            bool{} /*lastByteOfSet*/)
  } -> std::same_as<void>;
  { v.set_profile_details_inquiry(MIDICI{}, profile_span{bytes}, std::byte{} /*target*/) } -> std::same_as<void>;
  {
    v.set_profile_details_reply(MIDICI{}, profile_span{bytes}, std::byte{} /*target*/, std::span<std::byte>{} /*data*/)
  } -> std::same_as<void>;

  // Property Exchange
  {
    v.recvPECapabilities(MIDICI{}, std::uint8_t{} /*numSimulRequests*/, std::uint8_t{} /*majVer*/,
                         std::uint8_t{} /*minVer*/)
  } -> std::same_as<void>;
  {
    v.recvPECapabilitiesReply(MIDICI{}, std::uint8_t{} /*numSimulRequests*/, std::uint8_t{} /*majVer*/,
                              std::uint8_t{} /*minVer*/)
  } -> std::same_as<void>;
  { v.recvPEGetInquiry(MIDICI{}, std::string{} /*details*/) } -> std::same_as<void>;
  { v.recvPESetReply(MIDICI{}, std::string{} /*details*/) } -> std::same_as<void>;
  { v.recvPESubReply(MIDICI{}, std::string{} /*details*/) } -> std::same_as<void>;
  { v.recvPENotify(MIDICI{}, std::string{} /*details*/) } -> std::same_as<void>;
  {
    v.recvPEGetReply(MIDICI{}, std::string{} /*requestDetails*/, std::span<std::byte>{} /*body*/,
                     bool{} /*lastByteOfChunk*/, bool{} /*lastByteOfSet*/)
  } -> std::same_as<void>;
  {
    v.recvPESetInquiry(MIDICI{}, std::string{} /*requestDetails*/, std::span<std::byte>{} /*body*/,
                       bool{} /*lastByteOfChunk*/, bool{} /*lastByteOfSet*/)
  } -> std::same_as<void>;
  {
    v.recvPESubInquiry(MIDICI{}, std::string{} /*requestDetails*/, std::span<std::byte>{} /*body*/,
                       bool{} /*lastByteOfChunk*/, bool{} /*lastByteOfSet*/)
  } -> std::same_as<void>;
};

class ci_callbacks {
public:
  ci_callbacks () = default;
  ci_callbacks (ci_callbacks const & ) = default;
  virtual ~ci_callbacks () = default;

  virtual bool check_muid(std::uint8_t /*group*/, std::uint32_t /*muid*/) { return false; }

  virtual void discovery(MIDICI const &, ci::discovery_current const &) { /* do nothing */ }
  virtual void discovery_reply(MIDICI const &, ci::discovery_reply_current const &) { /* do nothing */ }
  virtual void end_point_info(MIDICI const &, std::byte status) { (void)status; /* do nothing*/ }
  virtual void end_point_info_reply(MIDICI const &, std::uint8_t status, std::uint16_t infoLength,
                                    std::span<std::byte> infoData) {
    (void)status;
    (void)infoLength;
    (void)infoData; /* do nothing */
  }
  virtual void nak(MIDICI const &, std::uint8_t origSubID, std::uint8_t statusCode, std::uint8_t statusData,
                   std::span<std::byte, 5> ackNakDetails, std::uint16_t messageLength,
                   std::span<std::byte> ackNakMessage) {
    (void)origSubID;
    (void)statusCode;
    (void)statusData;
    (void)ackNakDetails;
    (void)messageLength;
    (void)ackNakMessage;
    /* do nothing */
  }
  virtual void ack(MIDICI const &, std::uint8_t /*origSubID*/, std::uint8_t /*statusCode*/, std::uint8_t /*statusData*/,
                   std::span<std::byte, 5> /*ackNakDetails*/, std::uint16_t /*messageLength*/,
                   std::span<std::byte> /*ackNakMessage*/) {
    /* do nothing */
  }
  virtual void invalidate_muid(MIDICI const &, uint32_t muid) { (void)muid; }

  virtual void unknown_midici(MIDICI const &, std::byte s7) { (void)s7; }

  // Protocol Negotiation
  virtual void protocol_available(MIDICI const &, std::uint8_t authority_level, std::span<std::byte, 5> protocol) {
    (void)authority_level;
    (void)protocol;
  }
  virtual void recvSetProtocol(MIDICI const &, std::uint8_t authority_level, std::span<std::byte, 5> protocol) {
    (void)authority_level;
    (void)protocol;
  }
  virtual void recvSetProtocolConfirm(MIDICI const &, std::uint8_t /*authorityLevel*/) {}
  virtual void recvProtocolTest(MIDICI const &, std::uint8_t /*authorityLevel*/,
                                bool /*testDataAccurate*/) { /* do nothing */ }

  // Profiles
  virtual void recvProfileInquiry(MIDICI const &) { /* do nothing */ }
  virtual void recvSetProfileEnabled(MIDICI const &, profile_span /*profile*/,
                                     std::uint8_t /*number_of_channels*/) { /* do nothing */ }
  virtual void recvSetProfileRemoved(MIDICI const &, profile_span /*profile*/) {}
  virtual void recvSetProfileDisabled(MIDICI const &, profile_span /*profile*/,
                                      std::uint8_t /*number_of_channels*/) { /* do nothing*/ }
  virtual void recvSetProfileOn(MIDICI const &, profile_span /*profile*/,
                                std::uint8_t /*number_of_channels*/) { /* do nothing */ }
  virtual void recvSetProfileOff(MIDICI const &, profile_span /*profile*/) { /* do nothing*/ }
  virtual void profile_specific_data(MIDICI const &, profile_span /*profile*/, std::span<std::byte> /*data*/,
                                     std::uint16_t /*part*/, bool /*lastByteOfSet*/) { /* do nothing*/ }
  virtual void set_profile_details_inquiry(MIDICI const &, profile_span /*profile*/, std::byte /*target*/) {}
  virtual void set_profile_details_reply(MIDICI const &, profile_span /*profile*/, std::byte /*target*/,
                                         std::span<std::byte> /*data*/) {}

  // Property Exchange
  virtual void recvPECapabilities(MIDICI const &, std::uint8_t /*numSimulRequests*/, std::uint8_t /*majVer*/,
                                  std::uint8_t /*minVer*/) {}
  virtual void recvPECapabilitiesReply(MIDICI const &, std::uint8_t /*numSimulRequests*/, std::uint8_t /*majVer*/,
                                       std::uint8_t /*minVer*/) {}
  virtual void recvPEGetInquiry(MIDICI const &, std::string const & /*requestDetails*/) {}
  virtual void recvPESetReply(MIDICI const &, std::string const & /*requestDetails*/) {}
  virtual void recvPESubReply(MIDICI const &, std::string const & /*requestDetails*/) {}
  virtual void recvPENotify(MIDICI const &, std::string const & /*requestDetails*/) {}
  virtual void recvPEGetReply(MIDICI const &, std::string const & /*requestDetails*/, std::span<std::byte> /*body*/,
                              bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) {}
  virtual void recvPESetInquiry(MIDICI const &, std::string const & /*requestDetails*/, std::span<std::byte> /*body*/,
                                bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) {}
  virtual void recvPESubInquiry(MIDICI const &, std::string const & /*requestDetails*/, std::span<std::byte> /*body*/,
                                bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) {}

  // Process Inquiry
  virtual void recvPICapabilities(MIDICI const &) {}
  virtual void recvPICapabilitiesReply(MIDICI const &, std::byte /*supportedFeatures*/) {}
  virtual void recvPIMMReport(MIDICI const &, std::byte /*MDC*/, std::byte /*systemBitmap*/,
                              std::byte /*chanContBitmap*/, std::byte /*chanNoteBitmap*/) {}
  virtual void recvPIMMReportReply(MIDICI const &, std::byte /*systemBitmap*/, std::byte /*chanContBitmap*/,
                                   std::byte /*chanNoteBitmap*/) {}
  virtual void recvPIMMReportEnd(MIDICI const &) {}
};

template <ci_backend Callbacks = ci_callbacks> class midiCIProcessor {
public:
  explicit midiCIProcessor(Callbacks callbacks = Callbacks{}) : callbacks_{callbacks} {}

  void startSysex7(std::uint8_t group, std::uint8_t deviceId);
  void endSysex7();

  void processMIDICI(std::byte s7Byte);

private:
  static constexpr auto S7_BUFFERLEN = 36;

  Callbacks callbacks_;
  MIDICI midici_;
  std::uint16_t sysexPos_ = 0;

  // in Discovery this is [sysexID1,sysexID2,sysexID3,famId1,famid2,modelId1,modelId2,ver1,ver2,ver3,ver4,...product Id]
  // in Profiles this is  [pf1, pf1, pf3, pf4, pf5]
  // in Protocols this is [pr1, pr2, pr3, pr4, pr5]
  std::array<std::byte, 256> buffer_;

  // in Discovery this is [ciSupport, maxSysex, output path id]
  // in Profile Inquiry Reply, this is [Enabled Profiles Length, Disabled Profile Length]
  // in Profile On/Off/Enabled/Disabled, this is [numOfChannels]
  // in PE this is [header length, Body Length]
  std::uint32_t intTemp_[4]{};

  void processProtocolSysex(std::byte s7Byte);
  void processProfileSysex(std::byte s7Byte);

  // Property Exchange
  std::map<ci::reqId, std::string> peHeaderStr;

  void cleanupRequest(ci::reqId peReqIdx);
  void processPESysex(std::byte s7Byte);
  void processPISysex(std::byte s7Byte);

  void discovery_request_reply(std::byte s7Byte);
  void midiCI_ack_nak(std::byte s7Byte);
  void midiCI_endpoint_info_reply(std::byte s7Byte);
};

midiCIProcessor() -> midiCIProcessor<ci_callbacks>;
template <ci_backend T> midiCIProcessor(T) -> midiCIProcessor<T>;
template <ci_backend T> midiCIProcessor(std::reference_wrapper<T>) -> midiCIProcessor<T &>;

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::endSysex7() {
  if (midici_._peReqIdx) {
    cleanupRequest(midici_._peReqIdx);
  }
}

template <ci_backend Callbacks>
void midiCIProcessor<Callbacks>::startSysex7(std::uint8_t group, std::uint8_t deviceId) {
  sysexPos_ = 0;
  buffer_[0] = std::byte{0x00};
  midici_ = MIDICI();
  midici_.deviceId = deviceId;
  midici_.umpGroup = group;
}

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::cleanupRequest(ci::reqId peReqIdx) {
  peHeaderStr.erase(peReqIdx);
}

template <std::unsigned_integral T> constexpr T little_to_native(T v);

template <> constexpr std::uint32_t little_to_native<std::uint32_t>(std::uint32_t v) {
  if constexpr (std::endian::native == std::endian::little) {
    return v;
  } else if constexpr (std::endian::native == std::endian::big) {
    return ((v >> 24) & 0xFF) | ((v >> 16) & 0x0000FF00) | ((v << 8) & 0x00FF0000) | ((v << 24) & 0xFF000000);
  } else {
    assert(false && "Can't byte swap if endian is mixed");
  }
}
template <> constexpr std::uint16_t little_to_native<std::uint16_t>(std::uint16_t v) {
  if constexpr (std::endian::native == std::endian::little) {
    return v;
  } else if constexpr (std::endian::native == std::endian::big) {
    return ((v >> 8) & 0x00FF) | ((v << 8) & 0xFF00);
  } else {
    assert(false && "Can't byte swap if endian is mixed");
  }
}

template <typename T> void swaps(T &);

template <> void swaps<ci::discovery_v1>(ci::discovery_v1 &d1) {
  d1.family = little_to_native(d1.family);
  d1.model = little_to_native(d1.model);
  d1.max_sysex_size = little_to_native(d1.max_sysex_size);
}

template <> void swaps<ci::discovery_v2>(ci::discovery_v2 &(d2)) {
  swaps<ci::discovery_v1>(d2.v1);
}

template <> void swaps<ci::discovery_reply_v1>(ci::discovery_reply_v1 &dr1) {
  dr1.family = little_to_native(dr1.family);
  dr1.model = little_to_native(dr1.model);
  dr1.max_sysex_size = little_to_native(dr1.max_sysex_size);
}
template <> void swaps<ci::discovery_reply_v2>(ci::discovery_reply_v2 &dr2) {
  swaps<ci::discovery_reply_v1>(dr2.v1);
}

constexpr std::size_t expected_size(unsigned version, unsigned citype) {
  if (citype == MIDICI_DISCOVERY) {
    return version == 1 ? sizeof(ci::discovery_v1) : sizeof(ci::discovery_v2);
  } else if (citype == MIDICI_DISCOVERY_REPLY) {
    return version == 1 ? sizeof(ci::discovery_reply_v1) : sizeof(ci::discovery_reply_v2);
  } else {
    unreachable();
  }
}

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::discovery_request_reply(std::byte s7) {
  static constexpr auto header_size = 13;
  if (sysexPos_ < 13) {
    return;
  }
  assert(sysexPos_ >= header_size);
  buffer_[sysexPos_ - header_size] = s7;
  if (sysexPos_ < header_size + expected_size(midici_.ciVer, midici_.ciType) - 1) {
    return;
  }

  if (midici_.ciType == MIDICI_DISCOVERY) {
    ci::discovery_current dv;
    static_assert(sizeof(ci::discovery_v1) <= sizeof(ci::discovery_v2));
    static_assert(std::is_same_v<ci::discovery_v2, ci::discovery_current>);
    std::memcpy(&dv, buffer_.data(), midici_.ciVer == 1 ? sizeof(ci::discovery_v1) : sizeof(ci::discovery_v2));
    swaps(dv);
    callbacks_.discovery(midici_, dv);
  } else if (midici_.ciType == MIDICI_DISCOVERY_REPLY) {
    ci::discovery_reply_current reply;
    static_assert(sizeof(ci::discovery_reply_v1) <= sizeof(ci::discovery_reply_v2));
    static_assert(std::is_same_v<ci::discovery_reply_v2, ci::discovery_reply_current>);

    std::memcpy(&reply, buffer_.data(),
                midici_.ciVer == 1 ? sizeof(ci::discovery_reply_v1) : sizeof(ci::discovery_reply_v2));
    swaps(reply);
    callbacks_.discovery_reply(midici_, reply);
  } else {
    assert(false);
  }
}

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::midiCI_ack_nak(std::byte s7Byte) {
  bool complete = false;
  if (sysexPos_ == 13) {
    if (midici_.ciVer == 1) {
      complete = true;
    } else if (midici_.ciVer > 1) {
      intTemp_[0] = static_cast<std::uint16_t>(s7Byte);  // std::uint8_t origSubID,
    }
  }
  if (sysexPos_ == 14) {
    intTemp_[1] = static_cast<std::uint32_t>(s7Byte);  // statusCode
  }
  if (sysexPos_ == 15) {
    intTemp_[2] = static_cast<std::uint32_t>(s7Byte);  // statusData
  }
  if (sysexPos_ >= 16 && sysexPos_ <= 20) {
    buffer_[sysexPos_ - 16] = s7Byte;  // ackNakDetails
  }
  if (sysexPos_ == 21 || sysexPos_ == 22) {
    intTemp_[3] += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 21));
    return;
  }

  if (sysexPos_ >= 23 && sysexPos_ <= 23 + intTemp_[3]) {
    buffer_[sysexPos_ - 23] = s7Byte;  // product ID
  }
  if (sysexPos_ == 23 + intTemp_[3]) {
    complete = true;
  }

  if (complete) {
    std::span<std::byte, 5> ackNakDetails{std::begin(buffer_), 5};

    if (midici_.ciType == MIDICI_NAK)
      callbacks_.nak(midici_, static_cast<std::uint8_t>(intTemp_[0]), static_cast<std::uint8_t>(intTemp_[1]),
                     static_cast<std::uint8_t>(intTemp_[2]), ackNakDetails, static_cast<std::uint16_t>(intTemp_[3]),
                     buffer_);

    if (midici_.ciType == MIDICI_ACK && midici_.ciVer > 1)
      callbacks_.ack(midici_, static_cast<std::uint8_t>(intTemp_[0]), static_cast<std::uint8_t>(intTemp_[1]),
                     static_cast<std::uint8_t>(intTemp_[2]), ackNakDetails, static_cast<std::uint16_t>(intTemp_[3]),
                     buffer_);
  }
}

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::midiCI_endpoint_info_reply(std::byte s7Byte) {
  bool complete = false;
  if (midici_.ciVer < 2) {
    return;
  }
  if (sysexPos_ == 13) {
    intTemp_[0] = static_cast<std::uint32_t>(s7Byte);
  }
  if (sysexPos_ == 14 || sysexPos_ == 15) {
    intTemp_[1] += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 14));
    return;
  }
  if (sysexPos_ >= 16 && sysexPos_ <= 15 + intTemp_[1]) {
    buffer_[sysexPos_ - 16] = s7Byte;  // Info Data
  }
  if (sysexPos_ == 16 + intTemp_[1]) {
    complete = true;
  }

  if (complete) {
    callbacks_.end_point_info_reply(midici_, static_cast<std::uint8_t>(intTemp_[0]),
                                    static_cast<std::uint16_t>(intTemp_[1]), buffer_);
  }
}

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::processMIDICI(std::byte s7Byte) {
  // printf("s7 Byte %d\n", s7Byte);
  if (sysexPos_ == 3) {
    midici_.ciType = static_cast<std::uint8_t>(s7Byte);
  }
  if (sysexPos_ == 4) {
    midici_.ciVer = static_cast<std::uint8_t>(s7Byte);
  }
  if (sysexPos_ >= 5 && sysexPos_ <= 8) {
    midici_.remoteMUID += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 5));
  }
  if (sysexPos_ >= 9 && sysexPos_ <= 12) {
    midici_.localMUID += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 9));
  }
  if (sysexPos_ >= 12 && midici_.localMUID != M2_CI_BROADCAST &&
      !callbacks_.check_muid(midici_.umpGroup, midici_.localMUID)) {
    return;  // Not for this device
  }

  // break up each Process based on ciType
  if (sysexPos_ >= 12) {
    switch (midici_.ciType) {
    case MIDICI_DISCOVERY:
    case MIDICI_DISCOVERY_REPLY: this->discovery_request_reply(s7Byte); break;

    case MIDICI_INVALIDATEMUID:  // MIDI-CI Invalidate MUID Message
      if (sysexPos_ >= 13 && sysexPos_ <= 16) {
        buffer_[sysexPos_ - 13] = s7Byte;
      }

      // terminate MUID
      if (sysexPos_ == 16) {
        std::uint32_t const muid =
            static_cast<std::uint32_t>(buffer_[0]) | (static_cast<std::uint32_t>(buffer_[1]) << 7) |
            (static_cast<std::uint32_t>(buffer_[2]) << 14) | (static_cast<std::uint32_t>(buffer_[3]) << 21);
        callbacks_.invalidate_muid(midici_, muid);
      }
      break;
    case MIDICI_ENDPOINTINFO:
      if (sysexPos_ == 13 && midici_.ciVer > 1) {
        callbacks_.end_point_info(midici_, s7Byte);
      }
      break;
    case MIDICI_ENDPOINTINFO_REPLY:
      this->midiCI_endpoint_info_reply(s7Byte);
      break;
    case MIDICI_ACK:
    case MIDICI_NAK:
      this->midiCI_ack_nak(s7Byte);
      break;

      // #ifdef M2_ENABLE_PROTOCOL
    case MIDICI_PROTOCOL_NEGOTIATION:
    case MIDICI_PROTOCOL_NEGOTIATION_REPLY:
    case MIDICI_PROTOCOL_SET:
    case MIDICI_PROTOCOL_TEST:
    case MIDICI_PROTOCOL_TEST_RESPONDER:
    case MIDICI_PROTOCOL_CONFIRM:
      this->processProtocolSysex(s7Byte);
      break;
      // #endif

      // #ifndef M2_DISABLE_PROFILE
    case MIDICI_PROFILE_INQUIRY:        // Profile Inquiry
    case MIDICI_PROFILE_INQUIRYREPLY:   // Reply to Profile Inquiry
    case MIDICI_PROFILE_SETON:          // Set Profile On Message
    case MIDICI_PROFILE_SETOFF:         // Set Profile Off Message
    case MIDICI_PROFILE_ENABLED:        // Set Profile Enabled Message
    case MIDICI_PROFILE_DISABLED:       // Set Profile Disabled Message
    case MIDICI_PROFILE_SPECIFIC_DATA:  // ProfileSpecific Data
    case MIDICI_PROFILE_DETAILS_INQUIRY:
    case MIDICI_PROFILE_DETAILS_REPLY:
      this->processProfileSysex(s7Byte);
      break;
      // #endif

      // #ifndef M2_DISABLE_PE
    case MIDICI_PE_CAPABILITY:       // Inquiry: Property Exchange Capabilities
    case MIDICI_PE_CAPABILITYREPLY:  // Reply to Property Exchange Capabilities
    case MIDICI_PE_GET:              // Inquiry: Get Property Data
    case MIDICI_PE_GETREPLY:         // Reply To Get Property Data - Needs Work!
    case MIDICI_PE_SET:              // Inquiry: Set Property Data
    case MIDICI_PE_SETREPLY:         // Reply To Inquiry: Set Property Data
    case MIDICI_PE_SUB:              // Inquiry: Subscribe Property Data
    case MIDICI_PE_SUBREPLY:         // Reply To Subscribe Property Data
    case MIDICI_PE_NOTIFY:           // Notify
      this->processPESysex(s7Byte);
      break;
      // #endif

      // #ifndef M2_DISABLE_PROCESSINQUIRY
    case MIDICI_PI_CAPABILITY:
    case MIDICI_PI_CAPABILITYREPLY:
    case MIDICI_PI_MM_REPORT:
    case MIDICI_PI_MM_REPORT_REPLY:
    case MIDICI_PI_MM_REPORT_END:
      this->processPISysex(s7Byte);
      break;
      // #endif
    default: callbacks_.unknown_midici(midici_, s7Byte); break;
    }
  }
  sysexPos_++;
}

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::processProtocolSysex(std::byte s7Byte) {
  switch (midici_.ciType) {
  case MIDICI_PROTOCOL_NEGOTIATION:
  case MIDICI_PROTOCOL_NEGOTIATION_REPLY: {
    // Authority Level
    if (sysexPos_ == 13) {
      intTemp_[0] = static_cast<std::uint16_t>(s7Byte);
    }
    // Number of Supported Protocols (np)
    if (sysexPos_ == 14) {
      intTemp_[1] = static_cast<std::uint16_t>(s7Byte);
    }

    auto const protocolOffset = intTemp_[1] * 5 + 14;

    if (sysexPos_ >= 15 && sysexPos_ < protocolOffset) {
      std::uint8_t const pos = (sysexPos_ - 14) % 5;
      buffer_[pos] = s7Byte;
      if (pos == 4) {
        std::span<std::byte, 5> protocol{std::begin(buffer_), 5};
        callbacks_.protocol_available(midici_, static_cast<std::uint8_t>(intTemp_[0]), protocol);
      }
    }
    if (midici_.ciVer > 1) {
      if (sysexPos_ >= protocolOffset && sysexPos_ <= protocolOffset + 5) {
        buffer_[sysexPos_ - protocolOffset] = s7Byte;
      }
      if (sysexPos_ == protocolOffset + 5) {
        callbacks_.recvSetProtocolConfirm(midici_, static_cast<std::uint8_t>(intTemp_[0]));
      }
    }
    break;
  }

  case MIDICI_PROTOCOL_SET:  // Set Profile On Message
    // Authority Level
    if (sysexPos_ == 13) {
      intTemp_[0] = static_cast<std::uint16_t>(s7Byte);
    }
    if (sysexPos_ >= 14 && sysexPos_ <= 18) {
      buffer_[sysexPos_ - 13] = s7Byte;
    }
    if (sysexPos_ == 18) {
      std::span<std::byte, 5> protocol{std::begin(buffer_), 5};
      callbacks_.recvSetProtocol(midici_, static_cast<std::uint8_t>(intTemp_[0]), protocol);
    }
    break;

  case MIDICI_PROTOCOL_TEST_RESPONDER:
  case MIDICI_PROTOCOL_TEST:
    // Authority Level
    if (sysexPos_ == 13) {
      intTemp_[0] = static_cast<std::uint16_t>(s7Byte);
      intTemp_[1] = 1;
    }
    if (sysexPos_ >= 14 && sysexPos_ <= 61) {
      if (static_cast<std::uint8_t>(s7Byte) != sysexPos_ - 14) {
        intTemp_[1] = 0;
      }
    }
    if (sysexPos_ == 61) {
      callbacks_.recvProtocolTest(midici_, static_cast<std::uint8_t>(intTemp_[0]), !!(intTemp_[1]));
    }
    break;

  case MIDICI_PROTOCOL_CONFIRM:  // Set Profile Off Message
    // Authority Level
    if (sysexPos_ == 13) {
      intTemp_[0] = static_cast<std::uint16_t>(s7Byte);
      callbacks_.recvSetProtocolConfirm(midici_, static_cast<std::uint8_t>(intTemp_[0]));
    }
    break;

  default: assert(false && "unknown protocol sysex type"); break;
  }
}

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::processProfileSysex(std::byte s7Byte) {
  switch (midici_.ciType) {
  case MIDICI_PROFILE_INQUIRY:  // Profile Inquiry
    if (sysexPos_ == 12) {
      callbacks_.recvProfileInquiry(midici_);
    }
    break;
  case MIDICI_PROFILE_INQUIRYREPLY: {  // Reply to Profile Inquiry
    // Enabled Profiles Length
    if (sysexPos_ == 13 || sysexPos_ == 14) {
      intTemp_[0] += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 13));
    }

    // Disabled Profile Length
    auto const enabledProfileOffset = intTemp_[0] * 5 + 13;
    if (sysexPos_ == enabledProfileOffset || sysexPos_ == 1 + enabledProfileOffset) {
      intTemp_[1] += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - enabledProfileOffset));
    }

    if (sysexPos_ >= 15 && sysexPos_ < enabledProfileOffset) {
      std::uint8_t const pos = (sysexPos_ - 13) % 5;
      buffer_[pos] = s7Byte;
      if (pos == 4) {
        callbacks_.recvSetProfileEnabled(midici_, std::span<std::byte, 5>{std::begin(buffer_), 5}, 0);
      }
    }

    if (sysexPos_ >= 2 + enabledProfileOffset &&
        sysexPos_ < enabledProfileOffset + intTemp_[1] * 5) {
      std::uint8_t const pos = (sysexPos_ - 13) % 5;
      buffer_[pos] = s7Byte;
      if (pos == 4) {
        callbacks_.recvSetProfileDisabled(midici_, std::span<std::byte, 5>{std::begin(buffer_), 5}, 0);
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
      buffer_[sysexPos_ - 13] = s7Byte;
    }
    if (sysexPos_ == 17 &&
        (midici_.ciVer == 1 || midici_.ciType == MIDICI_PROFILE_ADD || midici_.ciType == MIDICI_PROFILE_REMOVE)) {
      complete = true;
    }
    if (midici_.ciVer > 1 && (sysexPos_ == 18 || sysexPos_ == 19)) {
      intTemp_[0] += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 18));
    }
    if (sysexPos_ == 19 && midici_.ciVer > 1) {
      complete = true;
    }

    if (complete) {
      auto const profile = std::span<std::byte, 5>{std::begin(buffer_), 5};
      if (midici_.ciType == MIDICI_PROFILE_ADD) {
        callbacks_.recvSetProfileDisabled(midici_, profile, 0);
      }
      if (midici_.ciType == MIDICI_PROFILE_REMOVE) {
        callbacks_.recvSetProfileRemoved(midici_, profile);
      }
      if (midici_.ciType == MIDICI_PROFILE_SETON) {
        callbacks_.recvSetProfileOn(midici_, profile, static_cast<std::uint8_t>(intTemp_[0]));
      }
      if (midici_.ciType == MIDICI_PROFILE_SETOFF) {
        callbacks_.recvSetProfileOff(midici_, profile);
      }
      if (midici_.ciType == MIDICI_PROFILE_ENABLED) {
        callbacks_.recvSetProfileEnabled(midici_, profile, static_cast<std::uint8_t>(intTemp_[0]));
      }
      if (midici_.ciType == MIDICI_PROFILE_DISABLED) {
        callbacks_.recvSetProfileDisabled(midici_, profile, static_cast<std::uint8_t>(intTemp_[0]));
      }
    }
    break;
  }

  case MIDICI_PROFILE_DETAILS_INQUIRY:
    if (sysexPos_ >= 13 && sysexPos_ <= 17) {
      buffer_[sysexPos_ - 13] = s7Byte;
    }
    if (sysexPos_ == 18) {  // Inquiry Target
      auto const profile = std::span<std::byte, 5>{std::begin(buffer_), 5};
      callbacks_.set_profile_details_inquiry(midici_, profile, s7Byte);
    }
    break;

  case MIDICI_PROFILE_DETAILS_REPLY: {
    if (sysexPos_ >= 13 && sysexPos_ <= 17) {
      buffer_[sysexPos_ - 13] = s7Byte;
    }
    if (sysexPos_ == 18) {  // Inquiry Target
      buffer_[5] = s7Byte;
    }

    if (sysexPos_ == 19 || sysexPos_ == 20) {  // Inquiry Target Data length (dl)
      intTemp_[0] += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 19));
    }

    if (sysexPos_ >= 21 && sysexPos_ <= 21 + intTemp_[0]) {
      buffer_[sysexPos_ - 22 + 6] = s7Byte;  // product ID
    }

    if (sysexPos_ == 21 + intTemp_[0]) {
      auto const profile = std::span<std::byte, 5>{std::begin(buffer_), 5};
      callbacks_.set_profile_details_reply(midici_, profile, buffer_[5],
                                           std::span<std::byte>(&buffer_[6], intTemp_[0]));
    }

    break;
  }

  case MIDICI_PROFILE_SPECIFIC_DATA: {
    // Profile
    if (sysexPos_ >= 13 && sysexPos_ <= 17) {
      buffer_[sysexPos_ - 13] = s7Byte;
      return;
    }
    if (sysexPos_ >= 18 && sysexPos_ <= 21) {
      // Length of Following Profile Specific Data
      intTemp_[0] += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 18));
      intTemp_[1] = 1;
      return;
    }

    std::uint16_t const charOffset = (sysexPos_ - 22) % S7_BUFFERLEN;
    auto const dataLength = intTemp_[0];
    if ((sysexPos_ >= 22 && sysexPos_ <= 21 + dataLength) || (dataLength == 0 && sysexPos_ == 21)) {
      if (dataLength != 0) {
        buffer_[charOffset] = s7Byte;
      }

      if (charOffset == S7_BUFFERLEN - 1 || sysexPos_ == 21 + dataLength || dataLength == 0) {
        bool const lastByteOfSet = (sysexPos_ == 21 + dataLength);
        auto const profile = profile_span{std::begin(buffer_), 5};
        auto data_length = charOffset + 1U;
        callbacks_.profile_specific_data(midici_, profile, std::span<std::byte>{std::begin(buffer_), data_length},
                                         static_cast<std::uint16_t> (intTemp_[1]), lastByteOfSet);
        intTemp_[1]++;
      }
    }
  } break;

  default: break;
  }
}

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::processPESysex(std::byte s7Byte) {
  switch (midici_.ciType) {
  case MIDICI_PE_CAPABILITY:
  case MIDICI_PE_CAPABILITYREPLY: {
    bool complete = false;

    if (sysexPos_ == 13) {
      buffer_[0] = s7Byte;
    }

    if (sysexPos_ == 13 && midici_.ciVer == 1) {
      complete = true;
    }

    if (sysexPos_ == 14) {
      buffer_[1] = s7Byte;
    }
    if (sysexPos_ == 15) {
      buffer_[2] = s7Byte;
      complete = true;
    }

    if (complete) {
      if (midici_.ciType == MIDICI_PE_CAPABILITY) {
        callbacks_.recvPECapabilities(midici_, static_cast<std::uint8_t>(buffer_[0]),
                                      static_cast<std::uint8_t>(buffer_[1]), static_cast<std::uint8_t>(buffer_[2]));
      }
      if (midici_.ciType == MIDICI_PE_CAPABILITYREPLY) {
        callbacks_.recvPECapabilitiesReply(midici_, static_cast<std::uint8_t>(buffer_[0]),
                                           static_cast<std::uint8_t>(buffer_[1]),
                                           static_cast<std::uint8_t>(buffer_[2]));
      }
    }

    break;
  }
  default: {
    if (sysexPos_ == 13) {
      midici_._peReqIdx = std::make_tuple(midici_.remoteMUID, s7Byte);
      midici_.requestId = s7Byte;
      intTemp_[0] = 0;
      return;
    }

    if (sysexPos_ == 14 || sysexPos_ == 15) {  // header Length
      intTemp_[0] += static_cast<std::uint32_t>(s7Byte) << (7U * (sysexPos_ - 14U));
      return;
    }

    auto const headerLength = static_cast<std::uint16_t> (intTemp_[0]);

    if (sysexPos_ == 16 && midici_.numChunk == 1) {
      peHeaderStr[*midici_._peReqIdx] = "";
    }

    if (sysexPos_ >= 16 && sysexPos_ <= 15 + headerLength) {
      std::uint16_t const charOffset = (sysexPos_ - 16);
      buffer_[charOffset] = s7Byte;
      peHeaderStr[*midici_._peReqIdx].push_back(static_cast<char>(s7Byte));

      if (sysexPos_ == 15 + headerLength) {
        switch (midici_.ciType) {
        case MIDICI_PE_GET:
          callbacks_.recvPEGetInquiry(midici_, peHeaderStr[*midici_._peReqIdx]);
          cleanupRequest(*midici_._peReqIdx);
          break;
        case MIDICI_PE_SETREPLY:
          callbacks_.recvPESetReply(midici_, peHeaderStr[*midici_._peReqIdx]);
          cleanupRequest(*midici_._peReqIdx);
          break;
        case MIDICI_PE_SUBREPLY:
          callbacks_.recvPESubReply(midici_, peHeaderStr[*midici_._peReqIdx]);
          cleanupRequest(*midici_._peReqIdx);
          break;
        case MIDICI_PE_NOTIFY:
          callbacks_.recvPENotify(midici_, peHeaderStr[*midici_._peReqIdx]);
          cleanupRequest(*midici_._peReqIdx);
          break;
        default: break;
        }
      }
    }

    if (sysexPos_ == 16 + headerLength || sysexPos_ == 17 + headerLength) {
      midici_.totalChunks += static_cast<std::uint8_t>(s7Byte) << (7 * (sysexPos_ - 16 - headerLength));
      return;
    }

    if (sysexPos_ == 18 + headerLength || sysexPos_ == 19 + headerLength) {
      midici_.numChunk += static_cast<std::uint8_t>(s7Byte) << (7 * (sysexPos_ - 18 - headerLength));
      return;
    }

    if (sysexPos_ == 20 + headerLength) {  // Body Length
      intTemp_[1] = static_cast<std::uint16_t>(s7Byte);
      return;
    }
    if (sysexPos_ == 21 + headerLength) {  // Body Length
      intTemp_[1] += static_cast<std::uint32_t>(s7Byte) << 7;
    }

    auto const bodyLength = intTemp_[1];
    std::uint16_t const initPos = 22 + headerLength;
    std::uint16_t const charOffset = (sysexPos_ - initPos) % S7_BUFFERLEN;

    if ((sysexPos_ >= initPos && sysexPos_ <= initPos - 1 + bodyLength) ||
        (bodyLength == 0 && sysexPos_ == initPos - 1)) {
      if (bodyLength != 0) {
        buffer_[charOffset] = s7Byte;
      }

      bool const lastByteOfSet = midici_.numChunk == midici_.totalChunks && sysexPos_ == initPos - 1 + bodyLength;
      bool const lastByteOfChunk = bodyLength == 0 || sysexPos_ == initPos - 1 + bodyLength;

      if (charOffset == S7_BUFFERLEN - 1 || lastByteOfChunk) {
        if (midici_.ciType == MIDICI_PE_GETREPLY) {
          callbacks_.recvPEGetReply(midici_, peHeaderStr[*midici_._peReqIdx],
                                    std::span<std::byte>{buffer_.data(), charOffset + 1U}, lastByteOfChunk,
                                    lastByteOfSet);
        }
        if (midici_.ciType == MIDICI_PE_SUB) {
          callbacks_.recvPESubInquiry(midici_, peHeaderStr[*midici_._peReqIdx],
                                      std::span<std::byte>{buffer_.data(), charOffset + 1U}, lastByteOfChunk,
                                      lastByteOfSet);
        }
        if (midici_.ciType == MIDICI_PE_SET) {
          callbacks_.recvPESetInquiry(midici_, peHeaderStr[*midici_._peReqIdx],
                                      std::span<std::byte>{buffer_.data(), charOffset + 1U}, lastByteOfChunk,
                                      lastByteOfSet);
        }
        midici_.partialChunkCount++;
      }

      if (lastByteOfSet) {
        cleanupRequest(*midici_._peReqIdx);
      }
    }
    break;
  }
  }
}

template <ci_backend Callbacks> void midiCIProcessor<Callbacks>::processPISysex(std::byte s7Byte) {
  if (midici_.ciVer == 1) {
    return;
  }

  switch (midici_.ciType) {
  case MIDICI_PI_CAPABILITY:
    if (sysexPos_ == 12) {
      callbacks_.recvPICapabilities(midici_);
    }
    break;
  case MIDICI_PI_CAPABILITYREPLY:
    if (sysexPos_ == 13) {
      callbacks_.recvPICapabilitiesReply(midici_, s7Byte);
    }
    break;
  case MIDICI_PI_MM_REPORT_END:
    if (sysexPos_ == 12) {
      callbacks_.recvPIMMReportEnd(midici_);
    }
    break;
  case MIDICI_PI_MM_REPORT:
    if (sysexPos_ == 13) {  // MDC
      buffer_[0] = s7Byte;
    }
    if (sysexPos_ == 14) {  // Bitmap of requested System Message Types
      buffer_[1] = s7Byte;
    }
    if (sysexPos_ == 16) {  // Bitmap of requested Channel Controller Message Types
      buffer_[2] = s7Byte;
    }
    if (sysexPos_ == 17) {
      callbacks_.recvPIMMReport(midici_, buffer_[0], buffer_[1], buffer_[2], s7Byte);
    }
    break;

  case MIDICI_PI_MM_REPORT_REPLY:
    if (sysexPos_ == 13) {  // Bitmap of requested System Message Types
      buffer_[0] = s7Byte;
    }
    if (sysexPos_ == 15) {  // Bitmap of requested Channel Controller Message Types
      buffer_[1] = s7Byte;
    }
    if (sysexPos_ == 16) {
      callbacks_.recvPIMMReportReply(midici_, buffer_[0], buffer_[1], s7Byte);
    }
    break;
  default:
    break;
  }
}

}  // end namespace midi2

#endif  // MIDI2_MIDICIPROCESSOR_H
