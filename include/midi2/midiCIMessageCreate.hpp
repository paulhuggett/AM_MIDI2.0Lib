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

template <typename T> struct type_to_packed {
  // static constexpr ci_message id = ;
  //  using v1 = ;
  //  using v2 = ;
};

struct empty {};
template <> struct type_to_packed<discovery> {
  static constexpr auto id = ci_message::discovery;
  using v1 = packed::discovery_v1;
  using v2 = packed::discovery_v2;
};
template <> struct type_to_packed<discovery_reply> {
  static constexpr auto id = ci_message::discovery_reply;
  using v1 = packed::discovery_reply_v1;
  using v2 = packed::discovery_reply_v2;
};
template <> struct type_to_packed<endpoint_info> {
  static constexpr auto id = ci_message::endpoint_info;
  using v1 = packed::endpoint_info_v1;
  using v2 = packed::endpoint_info_v1;
};
template <> struct type_to_packed<endpoint_info_reply> {
  static constexpr auto id = ci_message::endpoint_info_reply;
  // using v1 = packed::endpoint_info_v1;
  // using v2 = packed::endpoint_info_v1;
};
template <> struct type_to_packed<invalidate_muid> {
  static constexpr auto id = ci_message::invalidate_muid;
  using v1 = packed::invalidate_muid_v1;
  using v2 = packed::invalidate_muid_v1;
};
template <> struct type_to_packed<ack> {
  static constexpr auto id = ci_message::ack;
  // using v1 =
  // using v2 =
};
template <> struct type_to_packed<nak> {
  static constexpr auto id = ci_message::nak;
  using v1 = empty;
  using v2 = packed::nak_v2;
};
template <> struct type_to_packed<profile_configuration::added> {
  static constexpr auto id = ci_message::profile_added;
  using v1 = profile_configuration::packed::added_v1;
  using v2 = profile_configuration::packed::added_v1;
};
template <> struct type_to_packed<profile_configuration::removed> {
  static constexpr auto id = ci_message::profile_removed;
  using v1 = profile_configuration::packed::removed_v1;
  using v2 = profile_configuration::packed::removed_v1;
};
template <> struct type_to_packed<profile_configuration::details> {
  static constexpr auto id = ci_message::profile_details;
  using v1 = profile_configuration::packed::details_v1;
  using v2 = profile_configuration::packed::details_v1;
};
template <> struct type_to_packed<profile_configuration::details_reply> {
  static constexpr auto id = ci_message::profile_details_reply;
  using v1 = profile_configuration::packed::details_reply_v1;
  using v2 = profile_configuration::packed::details_reply_v1;
};
template <> struct type_to_packed<profile_configuration::inquiry_reply> {
  static constexpr auto id = ci_message::profile_inquiry_reply;
  // using v1 = profile_configuration::packed::details_reply_v1;
  // using v2 = profile_configuration::packed::details_reply_v1;
};
template <> struct type_to_packed<profile_configuration::on> {
  static constexpr auto id = ci_message::profile_set_on;
  using v1 = profile_configuration::packed::on_v1;
  using v2 = profile_configuration::packed::on_v2;
};
template <> struct type_to_packed<profile_configuration::off> {
  static constexpr auto id = ci_message::profile_set_off;
  using v1 = profile_configuration::packed::off_v1;
  using v2 = profile_configuration::packed::off_v2;
};

template <> struct type_to_packed<profile_configuration::enabled> {
  static constexpr auto id = ci_message::profile_enabled;
  using v1 = profile_configuration::packed::enabled_v1;
  using v2 = profile_configuration::packed::enabled_v2;
};
template <> struct type_to_packed<profile_configuration::disabled> {
  static constexpr auto id = ci_message::profile_disabled;
  using v1 = profile_configuration::packed::disabled_v1;
  using v2 = profile_configuration::packed::disabled_v2;
};

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

template <typename ElementType, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
  requires(std::is_trivially_copyable_v<ElementType> && alignof(ElementType) == 1)
constexpr O write_packed_with_tail(O first, S const last, std::byte const *ptr, std::size_t const size,
                                   std::span<ElementType const> const span) {
  auto first2 = first;
  std::ranges::advance(first2, static_cast<std::iter_difference_t<decltype(first)>>(size + span.size_bytes()), last);
  if (first2 == last) {
    return first2;
  }

  first = std::ranges::copy(ptr, ptr + size, first).out;
  return std::ranges::copy(std::span<std::byte const>{std::bit_cast<std::byte const *>(span.data()), span.size_bytes()},
                           first)
      .out;
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O write_header(O first, S const last, struct params const &params, ci_message const id) {
  auto header = static_cast<packed::header>(params);
  header.sub_id_2 = static_cast<std::byte>(id);
  return details::safe_copy(first, last, header);
}

}  // end namespace details

template <typename T, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, struct params const &params, T const &t) {
  first = details::write_header(first, last, params, type_to_packed<T>::id);
  using v1_type = type_to_packed<T>::v1;
  using v2_type = type_to_packed<T>::v2;
  if (params.ciVer == 1) {
    if constexpr (!std::is_same_v<v1_type, empty>) {
      first = details::safe_copy(first, last, static_cast<v1_type>(t));
    }
    return first;
  }
  if constexpr (!std::is_same_v<v2_type, empty>) {
    first = details::safe_copy(first, last, static_cast<v2_type>(t));
  }
  return first;
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, struct params const &params, endpoint_info_reply const &reply) {
  first = details::write_header(first, last, params, type_to_packed<endpoint_info_reply>::id);
  auto const v1 = static_cast<packed::endpoint_info_reply_v1>(reply);
  static_assert(std::is_trivially_copyable_v<decltype(v1)> && alignof(decltype(v1)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v1),
                                         offsetof(packed::endpoint_info_reply_v1, data), reply.information);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, struct params const &params, struct ack const &ack) {
  first = details::write_header(first, last, params, type_to_packed<struct ack>::id);
  auto const v1 = static_cast<packed::ack_v1>(ack);
  static_assert(std::is_trivially_copyable_v<decltype(v1)> && alignof(decltype(v1)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v1),
                                         offsetof(packed::ack_v1, message), ack.message);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S last, struct params const &params, struct nak const &nak) {
  first = details::write_header(first, last, params, type_to_packed<struct nak>::id);
  if (params.ciVer == 1) {
    return first;
  }
  auto const v2 = static_cast<packed::nak_v2>(nak);
  static_assert(std::is_trivially_copyable_v<decltype(v2)> && alignof(decltype(v2)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v2),
                                         offsetof(packed::nak_v2, message), nak.message);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, struct params const &params,
                           profile_configuration::details_reply const &reply) {
  using profile_configuration::packed::details_reply_v1;
  first = details::write_header(first, last, params, type_to_packed<profile_configuration::details_reply>::id);
  auto const v1 = static_cast<details_reply_v1>(reply);
  static_assert(std::is_trivially_copyable_v<decltype(v1)> && alignof(decltype(v1)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v1),
                                         offsetof(details_reply_v1, data), reply.data);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, struct params const &params,
                           profile_configuration::inquiry_reply const &reply) {
  using profile_configuration::packed::inquiry_reply_v1_pt1;
  using profile_configuration::packed::inquiry_reply_v1_pt2;

  first = details::write_header(first, last, params, type_to_packed<profile_configuration::inquiry_reply>::id);

  auto const part1 = static_cast<inquiry_reply_v1_pt1>(reply);
  static_assert(std::is_trivially_copyable_v<decltype(part1)> && alignof(decltype(part1)) == 1);
  first = details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&part1),
                                          offsetof(inquiry_reply_v1_pt1, ids), reply.enabled);

  auto const part2 = static_cast<inquiry_reply_v1_pt2>(reply);
  static_assert(std::is_trivially_copyable_v<decltype(part2)> && alignof(decltype(part2)) == 1);
  first = details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&part2),
                                          offsetof(inquiry_reply_v1_pt2, ids), reply.disabled);
  return first;
}

}  // end namespace midi2::ci

namespace midi2::CIMessage {

uint16_t sendProfileListRequest(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                                uint8_t destination);

uint16_t sendProfileListResponse(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                                 uint8_t destination, uint8_t profilesEnabledLen, uint8_t *profilesEnabled,
                                 uint8_t profilesDisabledLen, uint8_t *profilesDisabled);

uint16_t sendProfileSpecificData(uint8_t *sysex, uint8_t midiCIVer, uint32_t srcMUID, uint32_t destMuid,
                                 uint8_t destination, std::array<uint8_t, 5> profile, uint16_t datalen, uint8_t *data);

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
