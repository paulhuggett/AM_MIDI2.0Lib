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

using MIDICI = ci::MIDICI;

using profile_span = std::span<std::byte, 5>;
constexpr auto bytes = std::span<std::byte>{};

template <typename T> concept discovery_backend = requires(T && v) {
  { v.check_muid(std::uint8_t{} /*group*/, std::uint32_t{} /*muid*/) } -> std::convertible_to<bool>;

  { v.discovery(MIDICI{}, ci::discovery{}) } -> std::same_as<void>;
  { v.discovery_reply(MIDICI{}, ci::discovery_reply{}) } -> std::same_as<void>;
  { v.endpoint_info(MIDICI{}, ci::endpoint_info{}) } -> std::same_as<void>;
  { v.endpoint_info_reply(MIDICI{}, ci::endpoint_info_reply{}) } -> std::same_as<void>;
  { v.invalidate_muid(MIDICI{}, ci::invalidate_muid{}) } -> std::same_as<void>;
  { v.ack(MIDICI{}, ci::ack{}) } -> std::same_as<void>;
  { v.nak(MIDICI{}, ci::nak{}) } -> std::same_as<void>;

  { v.unknown_midici(MIDICI{}, std::byte{}) } -> std::same_as<void>;

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

template <typename T> concept profile_backend = requires(T && v) {
  { v.inquiry(MIDICI{}) } -> std::same_as<void>;
  { v.inquiry_reply(MIDICI{}, ci::profile_inquiry_reply{}) } -> std::same_as<void>;
  { v.profile_added(MIDICI{}, ci::profile_added{}) } -> std::same_as<void>;
  { v.profile_removed(MIDICI{}, ci::profile_removed{}) } -> std::same_as<void>;

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
};

class ci_callbacks {
public:
  ci_callbacks () = default;
  ci_callbacks (ci_callbacks const & ) = default;
  ci_callbacks(ci_callbacks &&) noexcept = default;
  virtual ~ci_callbacks() noexcept = default;

  virtual bool check_muid(std::uint8_t /*group*/, std::uint32_t /*muid*/) { return false; }

  virtual void discovery(MIDICI const &, ci::discovery const &) { /* do nothing */ }
  virtual void discovery_reply(MIDICI const &, ci::discovery_reply const &) { /* do nothing */ }
  virtual void endpoint_info(MIDICI const &, ci::endpoint_info const &) { /* do nothing*/ }
  virtual void endpoint_info_reply(MIDICI const &, ci::endpoint_info_reply const &) { /* do nothing */ }
  virtual void invalidate_muid(MIDICI const &, ci::invalidate_muid const &) { /* do nothing */ }
  virtual void ack(MIDICI const &, ci::ack const &) { /* do nothing */ }
  virtual void nak(MIDICI const &, ci::nak const &) { /* do nothing */ }

  virtual void unknown_midici(MIDICI const &, std::byte s7) { (void)s7; }

  // Property Exchange
  virtual void recvPECapabilities(MIDICI const &, std::uint8_t /*numSimulRequests*/, std::uint8_t /*majVer*/,
                                  std::uint8_t /*minVer*/) { /* do nothing */ }
  virtual void recvPECapabilitiesReply(MIDICI const &, std::uint8_t /*numSimulRequests*/, std::uint8_t /*majVer*/,
                                       std::uint8_t /*minVer*/) { /* do nothing */ }
  virtual void recvPEGetInquiry(MIDICI const &, std::string const & /*requestDetails*/) { /* do nothing */ }
  virtual void recvPESetReply(MIDICI const &, std::string const & /*requestDetails*/) { /* do nothing */ }
  virtual void recvPESubReply(MIDICI const &, std::string const & /*requestDetails*/) { /* do nothing */ }
  virtual void recvPENotify(MIDICI const &, std::string const & /*requestDetails*/) { /* do nothing */ }
  virtual void recvPEGetReply(MIDICI const &, std::string const & /*requestDetails*/, std::span<std::byte> /*body*/,
                              bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) { /* do nothing */ }
  virtual void recvPESetInquiry(MIDICI const &, std::string const & /*requestDetails*/, std::span<std::byte> /*body*/,
                                bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) { /* do nothing */ }
  virtual void recvPESubInquiry(MIDICI const &, std::string const & /*requestDetails*/, std::span<std::byte> /*body*/,
                                bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) { /* do nothing */ }

  // Process Inquiry
  virtual void recvPICapabilities(MIDICI const &) { /* do nothing */ }
  virtual void recvPICapabilitiesReply(MIDICI const &, std::byte /*supportedFeatures*/) { /* do nothing */ }
  virtual void recvPIMMReport(MIDICI const &, std::byte /*MDC*/, std::byte /*systemBitmap*/,
                              std::byte /*chanContBitmap*/, std::byte /*chanNoteBitmap*/) { /* do nothing */ }
  virtual void recvPIMMReportReply(MIDICI const &, std::byte /*systemBitmap*/, std::byte /*chanContBitmap*/,
                                   std::byte /*chanNoteBitmap*/) { /* do nothing */ }
  virtual void recvPIMMReportEnd(MIDICI const &) { /* do nothing */ }
};

class profile_callbacks {
public:
  profile_callbacks() = default;
  profile_callbacks(profile_callbacks const &) = default;
  profile_callbacks(profile_callbacks &&) noexcept = default;
  virtual ~profile_callbacks() noexcept = default;

  virtual void inquiry(MIDICI const &) { /* do nothing */ }
  virtual void inquiry_reply(MIDICI const &, ci::profile_inquiry_reply const &) { /* do nothing */ }
  virtual void profile_added(MIDICI const &, ci::profile_added const &) { /* do nothing */ }
  virtual void profile_removed(MIDICI const &, ci::profile_removed const &) { /* do nothing */ }

  virtual void recvSetProfileEnabled(MIDICI const &, profile_span /*profile*/,
                                     std::uint8_t /*number_of_channels*/) { /* do nothing */ }
  virtual void recvSetProfileRemoved(MIDICI const &, profile_span /*profile*/) { /* do nothing */ }
  virtual void recvSetProfileDisabled(MIDICI const &, profile_span /*profile*/,
                                      std::uint8_t /*number_of_channels*/) { /* do nothing*/ }
  virtual void recvSetProfileOn(MIDICI const &, profile_span /*profile*/,
                                std::uint8_t /*number_of_channels*/) { /* do nothing */ }
  virtual void recvSetProfileOff(MIDICI const &, profile_span /*profile*/) { /* do nothing*/ }
  virtual void profile_specific_data(MIDICI const &, profile_span /*profile*/, std::span<std::byte> /*data*/,
                                     std::uint16_t /*part*/, bool /*lastByteOfSet*/) { /* do nothing*/ }
  virtual void set_profile_details_inquiry(MIDICI const &, profile_span /*profile*/, std::byte /*target*/) {
    /* do nothing */
  }
  virtual void set_profile_details_reply(MIDICI const &, profile_span /*profile*/, std::byte /*target*/,
                                         std::span<std::byte> /*data*/) {
    /* do nothing */
  }
};

template <discovery_backend Callbacks = ci_callbacks, profile_backend ProfileBackend = profile_callbacks>
class midiCIProcessor {
public:
  explicit midiCIProcessor(Callbacks callbacks = Callbacks{}, ProfileBackend profile = ProfileBackend{})
      : callbacks_{callbacks}, profile_backend_{profile} {}

  void startSysex7(std::uint8_t group, std::byte deviceId);
  void endSysex7();

  void processMIDICI(std::byte s7Byte);

private:
  static constexpr auto S7_BUFFERLEN = 36;
  static constexpr auto header_size = 13U;

  [[no_unique_address]] Callbacks callbacks_;
  [[no_unique_address]] ProfileBackend profile_backend_;

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

  void processProfileSysex(std::byte s7Byte);

  // Property Exchange
  std::map<ci::reqId, std::string> peHeaderStr;

  void cleanupRequest(ci::reqId peReqIdx);
  void processPESysex(std::byte s7Byte);
  void processPISysex(std::byte s7Byte);

  // The "discovery" mesages
  void discovery(std::byte s7);
  void discovery_reply(std::byte s7);
  void endpoint_info(std::byte s7);
  void endpoint_info_reply(std::byte s7);
  void invalidate_muid(std::byte s7);
  void ack(std::byte s7);
  void nak(std::byte s7);

  // Profile messages
  void profile_inquiry(std::byte s7);
  void profile_inquiry_reply(std::byte s7);
  void profile_added(std::byte s7);
  void profile_removed(std::byte s7);
};

midiCIProcessor() -> midiCIProcessor<>;
template <discovery_backend C> midiCIProcessor(C) -> midiCIProcessor<C>;
template <discovery_backend C> midiCIProcessor(std::reference_wrapper<C>) -> midiCIProcessor<C &>;
template <discovery_backend C, profile_backend P> midiCIProcessor(C, P) -> midiCIProcessor<C, P>;
template <discovery_backend C, profile_backend P>
midiCIProcessor(std::reference_wrapper<C>, std::reference_wrapper<P>) -> midiCIProcessor<C &, P &>;

template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::endSysex7() {
  if (midici_._peReqIdx) {
    cleanupRequest(midici_._peReqIdx);
  }
}

template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::startSysex7(std::uint8_t group, std::byte deviceId) {
  sysexPos_ = 0;
  buffer_[0] = std::byte{0x00};
  midici_ = MIDICI();
  midici_.deviceId = static_cast<std::uint8_t>(deviceId);
  midici_.umpGroup = group;
}

template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::cleanupRequest(ci::reqId peReqIdx) {
  peHeaderStr.erase(peReqIdx);
}

// discovery
// ~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::discovery(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;

  static_assert(sizeof(ci::packed::discovery_v1) <= sizeof(ci::packed::discovery_v2));
  auto const expected_size = midici_.ciVer == 1 ? sizeof(ci::packed::discovery_v1) : sizeof(ci::packed::discovery_v2);
  if (sysexPos_ < header_size + expected_size - 1) {
    return;
  }

  ci::packed::discovery_v2 packed_discovery{};
  assert(expected_size <= sizeof(packed_discovery));
  std::memcpy(&packed_discovery, buffer_.data(), expected_size);
  callbacks_.discovery(midici_, ci::discovery{packed_discovery});
}

// discovery reply
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::discovery_reply(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  assert(sysexPos_ >= header_size);
  buffer_[sysexPos_ - header_size] = s7;

  static_assert(sizeof(ci::packed::discovery_reply_v1) <= sizeof(ci::packed::discovery_reply_v2));
  auto const expected_size =
      midici_.ciVer == 1 ? sizeof(ci::packed::discovery_reply_v1) : sizeof(ci::packed::discovery_reply_v2);
  if (sysexPos_ < header_size + expected_size - 1) {
    return;
  }

  ci::packed::discovery_reply_v2 packed{};
  assert(expected_size <= sizeof(packed));
  std::memcpy(&packed, buffer_.data(), expected_size);
  callbacks_.discovery_reply(midici_, ci::discovery_reply{packed});
}

// ack
// ~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::ack(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  assert(sysexPos_ >= header_size);
  buffer_[sysexPos_ - header_size] = s7;

  // Wait for the basic structure to arrive.
  auto const expected_size = offsetof(ci::packed::ack_v1, message);
  if (sysexPos_ < header_size + expected_size - 1) {
    return;
  }
  // Wait for the variable-length data.
  auto const *const packed_reply = std::bit_cast<ci::packed::ack_v1 const *>(buffer_.data());
  if (sysexPos_ < header_size + expected_size + ci::packed::from_le7(packed_reply->message_length) - 1) {
    return;
  }
  callbacks_.ack(midici_, ci::ack{*packed_reply});
}

template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::nak(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  assert(sysexPos_ >= header_size);
  buffer_[sysexPos_ - header_size] = s7;

  // Wait for the basic structure to arrive.
  auto const expected_size = midici_.ciVer == 1 ? sizeof(ci::packed::nak_v1) : offsetof(ci::packed::nak_v2, message);
  if (sysexPos_ < header_size + expected_size - 1) {
    return;
  }
  if (midici_.ciVer == 1) {
    auto const *const v1 = std::bit_cast<ci::packed::nak_v1 const *>(buffer_.data());
    return callbacks_.nak(midici_, ci::nak{*v1});
  }
  // Wait for the variable-length data.
  auto const *const v2 = std::bit_cast<ci::packed::nak_v2 const *>(buffer_.data());
  if (sysexPos_ < header_size + expected_size + ci::packed::from_le7(v2->message_length) - 1) {
    return;
  }
  callbacks_.nak(midici_, ci::nak{*v2});
}

// endpoint info
// ~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::endpoint_info(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;
  if (sysexPos_ < header_size + sizeof(ci::packed::endpoint_info_v1) - 1) {
    return;
  }
  callbacks_.endpoint_info(midici_,
                           ci::endpoint_info{*std::bit_cast<ci::packed::endpoint_info_v1 const *>(buffer_.data())});
}

// endpoint info reply
// ~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::endpoint_info_reply(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;

  // Wait for the basic structure to arrive.
  auto const expected_size = offsetof(ci::packed::endpoint_info_reply_v1, data);
  if (sysexPos_ < header_size + expected_size - 1) {
    return;
  }

  // Wait for the variable-length data.
  auto const *const packed_reply = std::bit_cast<ci::packed::endpoint_info_reply_v1 const *>(buffer_.data());
  if (sysexPos_ < header_size + expected_size + ci::packed::from_le7(packed_reply->data_length) - 1) {
    return;
  }
  callbacks_.endpoint_info_reply(midici_, ci::endpoint_info_reply{*packed_reply});
}

// invalidate muid
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::invalidate_muid(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;
  if (sysexPos_ < header_size + sizeof(ci::packed::invalidate_muid_v1) - 1) {
    return;
  }
  auto const *const p = std::bit_cast<ci::packed::invalidate_muid_v1 const *>(buffer_.data());
  callbacks_.invalidate_muid(midici_, ci::invalidate_muid{*p});
}

// profile inquiry
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::profile_inquiry(std::byte) {
  static constexpr auto header_size = 13;
  if (sysexPos_ < header_size - 1) {
    return;
  }
  profile_backend_.inquiry(midici_);
}

// profile inquiry reply
// ~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::profile_inquiry_reply(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;
  // Wait for the first part to arrive.
  constexpr auto pt1_size = offsetof(ci::packed::profile_inquiry_reply_v1_pt1, ids);
  auto end = header_size + pt1_size - 1;
  if (sysexPos_ < end) {
    return;
  }
  auto const *const pt1 = std::bit_cast<ci::packed::profile_inquiry_reply_v1_pt1 const *>(buffer_.data());
  end += ci::packed::from_le7(pt1->num_enabled) * sizeof(pt1->ids[0]);
  if (sysexPos_ < end) {
    return;
  }
  constexpr auto pt2_size = offsetof(ci::packed::profile_inquiry_reply_v1_pt2, ids);
  end += pt2_size;
  if (sysexPos_ < end) {
    return;
  }
  auto p2_pos = end - (header_size + pt1_size - 1);
  auto const *const pt2 = std::bit_cast<ci::packed::profile_inquiry_reply_v1_pt2 const *>(buffer_.data() + p2_pos);
  end += ci::packed::from_le7(pt2->num_disabled) * sizeof(pt2->ids[0]);
  if (sysexPos_ < end) {
    return;
  }
  profile_backend_.inquiry_reply(midici_, ci::profile_inquiry_reply{*pt1, *pt2});
}

// profile added
// ~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::profile_added(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;
  if (sysexPos_ < header_size + sizeof(ci::packed::profile_added_v1) - 1) {
    return;
  }
  auto const *const p = std::bit_cast<ci::packed::profile_added_v1 const *>(buffer_.data());
  profile_backend_.profile_added(midici_, ci::profile_added{*p});
}

// profile removed
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::profile_removed(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;
  if (sysexPos_ < header_size + sizeof(ci::packed::profile_removed_v1) - 1) {
    return;
  }
  auto const *const p = std::bit_cast<ci::packed::profile_removed_v1 const *>(buffer_.data());
  profile_backend_.profile_removed(midici_, ci::profile_removed{*p});
}

template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::processMIDICI(std::byte s7Byte) {
  assert((s7Byte & std::byte{0b10000000}) == std::byte{0});
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
    case MIDICI_DISCOVERY: this->discovery(s7Byte); break;
    case MIDICI_DISCOVERY_REPLY: this->discovery_reply(s7Byte); break;
    case MIDICI_ENDPOINTINFO: this->endpoint_info(s7Byte); break;
    case MIDICI_ENDPOINTINFO_REPLY: this->endpoint_info_reply(s7Byte); break;
    case MIDICI_INVALIDATEMUID: this->invalidate_muid(s7Byte); break;
    case MIDICI_ACK: this->ack(s7Byte); break;
    case MIDICI_NAK: this->nak(s7Byte); break;

    case MIDICI_PROFILE_INQUIRY: this->profile_inquiry(s7Byte); break;
    case MIDICI_PROFILE_INQUIRYREPLY: this->profile_inquiry_reply(s7Byte); break;
    case MIDICI_PROFILE_ADDED: this->profile_added(s7Byte); break;
    case MIDICI_PROFILE_REMOVED: this->profile_removed(s7Byte); break;

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

template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::processProfileSysex(std::byte s7Byte) {
  switch (midici_.ciType) {
  case MIDICI_PROFILE_ADDED:
  case MIDICI_PROFILE_REMOVED:
  case MIDICI_PROFILE_ENABLED:
  case MIDICI_PROFILE_DISABLED:
  case MIDICI_PROFILE_SETOFF:
  case MIDICI_PROFILE_SETON: {  // Set Profile On Message
    bool complete = false;
    if (sysexPos_ >= 13 && sysexPos_ <= 17) {
      buffer_[sysexPos_ - 13] = s7Byte;
    }
    if (sysexPos_ == 17 &&
        (midici_.ciVer == 1 || midici_.ciType == MIDICI_PROFILE_ADDED || midici_.ciType == MIDICI_PROFILE_REMOVED)) {
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
      if (midici_.ciType == MIDICI_PROFILE_ADDED) {
        profile_backend_.recvSetProfileDisabled(midici_, profile, 0);
      }
      if (midici_.ciType == MIDICI_PROFILE_REMOVED) {
        profile_backend_.recvSetProfileRemoved(midici_, profile);
      }
      if (midici_.ciType == MIDICI_PROFILE_SETON) {
        profile_backend_.recvSetProfileOn(midici_, profile, static_cast<std::uint8_t>(intTemp_[0]));
      }
      if (midici_.ciType == MIDICI_PROFILE_SETOFF) {
        profile_backend_.recvSetProfileOff(midici_, profile);
      }
      if (midici_.ciType == MIDICI_PROFILE_ENABLED) {
        profile_backend_.recvSetProfileEnabled(midici_, profile, static_cast<std::uint8_t>(intTemp_[0]));
      }
      if (midici_.ciType == MIDICI_PROFILE_DISABLED) {
        profile_backend_.recvSetProfileDisabled(midici_, profile, static_cast<std::uint8_t>(intTemp_[0]));
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
      profile_backend_.set_profile_details_inquiry(midici_, profile, s7Byte);
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
      profile_backend_.set_profile_details_reply(midici_, profile, buffer_[5],
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
        profile_backend_.profile_specific_data(midici_, profile, std::span<std::byte>{std::begin(buffer_), data_length},
                                               static_cast<std::uint16_t>(intTemp_[1]), lastByteOfSet);
        intTemp_[1]++;
      }
    }
  } break;

  default: break;
  }
}

template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::processPESysex(std::byte s7Byte) {
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

template <discovery_backend Callbacks, profile_backend ProfileBackend>
void midiCIProcessor<Callbacks, ProfileBackend>::processPISysex(std::byte s7Byte) {
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
