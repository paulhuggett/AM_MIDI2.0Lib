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

#ifndef MIDI2_MIDICIMESSAGECREATE_HPP
#define MIDI2_MIDICIMESSAGECREATE_HPP

#include <array>
#include <cstdint>

#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

namespace midi2::ci {

namespace details {

template <typename T, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
  requires(std::is_trivially_copyable_v<T> && alignof(T) == 1)
constexpr O safe_copy(O first, S last, T const &t) {
  auto first2 = first;
  std::ranges::advance(first2, sizeof(T), last);
  if (first2 == last) {
    return first2;
  }
  return std::ranges::copy(std::bit_cast<std::byte const *>(&t), std::bit_cast<std::byte const *>(&t + 1), first).out;
}

template <typename TailElement, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
  requires(std::is_trivially_copyable_v<TailElement> && alignof(TailElement) == 1)
constexpr O write_packed_with_tail(O first, S last, std::byte const *ptr, std::size_t const size,
                                   std::span<TailElement> const span) {
  auto first2 = first;
  std::ranges::advance(first2, static_cast<std::iter_difference_t<decltype(first)>>(size + span.size_bytes()), last);
  if (first2 == last) {
    return first2;
  }

  first = std::ranges::copy(ptr, ptr + size, first).out;
  return std::ranges::copy(span, first).out;
}

}  // end namespace details

template <typename T> struct type_to_packed {
  // using v1 = ;
  // using v2 = ;
};

template <> struct type_to_packed<discovery> {
  using v1 = packed::discovery_v1;
  using v2 = packed::discovery_v2;
};
template <> struct type_to_packed<discovery_reply> {
  using v1 = packed::discovery_reply_v1;
  using v2 = packed::discovery_reply_v2;
};
template <> struct type_to_packed<endpoint_info> {
  using v1 = packed::endpoint_info_v1;
  using v2 = packed::endpoint_info_v1;
};
template <> struct type_to_packed<invalidate_muid> {
  using v1 = packed::invalidate_muid_v1;
  using v2 = packed::invalidate_muid_v1;
};
template <> struct type_to_packed<nak> {
  using v1 = packed::nak_v1;
  using v2 = packed::nak_v2;
};

template <typename T, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S last, MIDICI const &header, T const &t) {
  first = details::safe_copy(first, last, static_cast<packed::header>(header));
  if (header.ciVer == 1) {
    return details::safe_copy(first, last, static_cast<type_to_packed<T>::v1>(t));
  }
  return details::safe_copy(first, last, static_cast<type_to_packed<T>::v2>(t));
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S last, MIDICI const &header, endpoint_info_reply const &reply) {
  first = details::safe_copy(first, last, static_cast<packed::header>(header));
  auto const v1 = static_cast<packed::endpoint_info_reply_v1>(reply);
  static_assert(std::is_trivially_copyable_v<decltype(v1)> && alignof(decltype(v1)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v1),
                                         offsetof(packed::endpoint_info_reply_v1, data), reply.information);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S last, MIDICI const &header, struct ack const &ack) {
  first = details::safe_copy(first, last, static_cast<packed::header>(header));
  auto const v1 = static_cast<packed::ack_v1>(ack);
  static_assert(std::is_trivially_copyable_v<decltype(v1)> && alignof(decltype(v1)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v1),
                                         offsetof(packed::ack_v1, message), ack.message);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S last, MIDICI const &header, struct nak const &nak) {
  first = details::safe_copy(first, last, static_cast<packed::header>(header));
  if (header.ciVer == 1) {
    return first;
  }
  auto const v2 = static_cast<packed::nak_v2>(nak);
  static_assert(std::is_trivially_copyable_v<decltype(v2)> && alignof(decltype(v2)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v2),
                                         offsetof(packed::nak_v2, message), nak.message);
}

}  // end namespace midi2::ci

namespace midi2::CIMessage {

uint16_t sendProfileListRequest(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                                uint8_t destination);

uint16_t sendProfileListResponse(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                                 uint8_t destination, uint8_t profilesEnabledLen, uint8_t *profilesEnabled,
                                 uint8_t profilesDisabledLen, uint8_t *profilesDisabled);

uint16_t sendProfileAdd(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t destination,
                        std::array<uint8_t, 5> profile);

uint16_t sendProfileRemove(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t destination,
                           std::array<uint8_t, 5> profile);

uint16_t sendProfileOn(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t destination,
                       std::array<uint8_t, 5> profile, uint8_t numberOfChannels);

uint16_t sendProfileOff(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t destination,
                        std::array<uint8_t, 5> profile);

uint16_t sendProfileEnabled(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t destination,
                            std::array<uint8_t, 5> profile, uint8_t numberOfChannels);

uint16_t sendProfileDisabled(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                             uint8_t destination, std::array<uint8_t, 5> profile, uint8_t numberOfChannels);

uint16_t sendProfileSpecificData(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                                 uint8_t destination, std::array<uint8_t, 5> profile, uint16_t datalen, uint8_t *data);

uint16_t sendProfileDetailsInquiry(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                                   uint8_t destination, std::array<uint8_t, 5> profile, uint8_t InquiryTarget);

uint16_t sendProfileDetailsReply(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                                 uint8_t destination, std::array<uint8_t, 5> profile, uint8_t InquiryTarget,
                                 uint16_t datalen, uint8_t *data);

uint16_t sendPECapabilityRequest(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                                 uint8_t numSimulRequests, uint8_t majVer, uint8_t minVer);

uint16_t sendPECapabilityReply(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                               uint8_t numSimulRequests, uint8_t majVer, uint8_t minVer);

uint16_t sendPEGet(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t requestId,
                   uint16_t headerLen, uint8_t *header);

uint16_t sendPESet(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t requestId,
                   uint16_t headerLen, uint8_t *header, uint16_t numberOfChunks, uint16_t numberOfThisChunk,
                   uint16_t bodyLength, uint8_t *body);

uint16_t sendPESub(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t requestId,
                   uint16_t headerLen, uint8_t *header, uint16_t numberOfChunks, uint16_t numberOfThisChunk,
                   uint16_t bodyLength, uint8_t *body);

uint16_t sendPEGetReply(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t requestId,
                        uint16_t headerLen, uint8_t *header, uint16_t numberOfChunks, uint16_t numberOfThisChunk,
                        uint16_t bodyLength, uint8_t *body);

uint16_t sendPESubReply(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t requestId,
                        uint16_t headerLen, uint8_t *header);

uint16_t sendPENotify(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t requestId,
                      uint16_t headerLen, uint8_t *header);

uint16_t sendPESetReply(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t requestId,
                        uint16_t headerLen, uint8_t *header);

uint16_t sendPICapabilityRequest(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid);

uint16_t sendPICapabilityReply(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                               uint8_t supportedFeatures);

uint16_t sendPIMMReport(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t destination,
                        uint8_t MDC, uint8_t systemBitmap, uint8_t chanContBitmap, uint8_t chanNoteBitmap);

uint16_t sendPIMMReportReply(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                             uint8_t destination, uint8_t systemBitmap, uint8_t chanContBitmap, uint8_t chanNoteBitmap);

uint16_t sendPIMMReportEnd(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid, uint8_t destination);

}  // end namespace midi2::CIMessage
#endif  // MIDI2_MIDICIMESSAGECREATE_HPP
