//===-- CI Create Message -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_CI_CREATE_MESSAGE_HPP
#define MIDI2_CI_CREATE_MESSAGE_HPP

#include <algorithm>
#include <bit>
#include <cstddef>
#include <iterator>
#include <span>
#include <type_traits>

#include "midi2/ci/ci_types.hpp"

namespace midi2::ci {

/// \brief Private implementation details of the CI types and functions
namespace details {

template <typename T> struct type_to_packed {};

struct empty {};
struct not_available {};
/// Maps the public "discovery" message structure to the packed types that conform to the MIDI CI standard for sending
/// to other devices.
template <> struct type_to_packed<discovery> {
  static constexpr auto id = message::discovery;
  using v1 = packed::discovery_v1;
  using v2 = packed::discovery_v2;
};
/// Maps the public "discovery reply" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<discovery_reply> {
  static constexpr auto id = message::discovery_reply;
  using v1 = packed::discovery_reply_v1;
  using v2 = packed::discovery_reply_v2;
};
/// Maps the public "endpoint" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<endpoint> {
  static constexpr auto id = message::endpoint;
  using v1 = packed::endpoint_v1;
  using v2 = packed::endpoint_v1;
};
/// Maps the public "endpoint reply" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<endpoint_reply> {
  static constexpr auto id = message::endpoint_reply;
};
/// Maps the public "invalidate MUID" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<invalidate_muid> {
  static constexpr auto id = message::invalidate_muid;
  using v1 = packed::invalidate_muid_v1;
  using v2 = packed::invalidate_muid_v1;
};
/// Maps the public "ACK" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<ack> {
  static constexpr auto id = message::ack;
};
/// Maps the public "NAK" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<nak> {
  static constexpr auto id = message::nak;
  using v1 = empty;
  using v2 = packed::nak_v2;
};

// Profile Configuration Types

/// Maps the public "profile configuration added" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::added> {
  static constexpr auto id = message::profile_added;
  using v1 = profile_configuration::packed::added_v1;
  using v2 = profile_configuration::packed::added_v1;
};
/// Maps the public "profile configuration removed" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::removed> {
  static constexpr auto id = message::profile_removed;
  using v1 = profile_configuration::packed::removed_v1;
  using v2 = profile_configuration::packed::removed_v1;
};
/// Maps the public "profile configuration details" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::details> {
  static constexpr auto id = message::profile_details;
  using v1 = profile_configuration::packed::details_v1;
  using v2 = profile_configuration::packed::details_v1;
};
/// Maps the public "profile configuration details reply" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::details_reply> {
  static constexpr auto id = message::profile_details_reply;
  using v1 = profile_configuration::packed::details_reply_v1;
  using v2 = profile_configuration::packed::details_reply_v1;
};
/// Maps the public "profile configuration Inquiry" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::inquiry> {
  static constexpr auto id = message::profile_inquiry;
  using v1 = empty;
  using v2 = empty;
};
/// Maps the public "profile configuration Inquiry Reply" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::inquiry_reply> {
  static constexpr auto id = message::profile_inquiry_reply;
};
/// Maps the public "profile configuration on" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::on> {
  static constexpr auto id = message::profile_set_on;
  using v1 = profile_configuration::packed::on_v1;
  using v2 = profile_configuration::packed::on_v2;
};
/// Maps the public "profile configuration off" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::off> {
  static constexpr auto id = message::profile_set_off;
  using v1 = profile_configuration::packed::off_v1;
  using v2 = profile_configuration::packed::off_v2;
};
/// Maps the public "profile configuration enabled" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::enabled> {
  static constexpr auto id = message::profile_enabled;
  using v1 = profile_configuration::packed::enabled_v1;
  using v2 = profile_configuration::packed::enabled_v2;
};
/// Maps the public "profile configuration disabled" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::disabled> {
  static constexpr auto id = message::profile_disabled;
  using v1 = profile_configuration::packed::disabled_v1;
  using v2 = profile_configuration::packed::disabled_v2;
};
/// Maps the public "profile configuration specific data" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::specific_data> {
  static constexpr auto id = message::profile_specific_data;
  using v1 = profile_configuration::packed::specific_data_v1;
  using v2 = profile_configuration::packed::specific_data_v1;
};

// Property Exchange Types

/// Maps the public "property exchange capabilities" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<property_exchange::capabilities> {
  static constexpr auto id = message::pe_capability;
  using v1 = property_exchange::packed::capabilities_v1;
  using v2 = property_exchange::packed::capabilities_v2;
};
/// Maps the public "property exchange capabilities reply" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<property_exchange::capabilities_reply> {
  static constexpr auto id = message::pe_capability_reply;
  using v1 = property_exchange::packed::capabilities_reply_v1;
  using v2 = property_exchange::packed::capabilities_reply_v2;
};
template <> struct type_to_packed<property_exchange::get> {
  static constexpr auto id = message::pe_get;
};
template <> struct type_to_packed<property_exchange::get_reply> {
  static constexpr auto id = message::pe_get_reply;
};
template <> struct type_to_packed<property_exchange::set> {
  static constexpr auto id = message::pe_set;
};
template <> struct type_to_packed<property_exchange::set_reply> {
  static constexpr auto id = message::pe_set_reply;
};
template <> struct type_to_packed<property_exchange::subscription> {
  static constexpr auto id = message::pe_sub;
};
template <> struct type_to_packed<property_exchange::subscription_reply> {
  static constexpr auto id = message::pe_sub_reply;
};
template <> struct type_to_packed<property_exchange::notify> {
  static constexpr auto id = message::pe_notify;
};

// Process Inquiry

template <> struct type_to_packed<process_inquiry::capabilities> {
  static constexpr auto id = message::pi_capability;
  using v1 = empty;
  using v2 = empty;
};
template <> struct type_to_packed<process_inquiry::capabilities_reply> {
  static constexpr auto id = message::pi_capability_reply;
  using v1 = not_available;
  using v2 = process_inquiry::packed::capabilities_reply_v2;
};
template <> struct type_to_packed<process_inquiry::midi_message_report> {
  static constexpr auto id = message::pi_mm_report;
  using v1 = not_available;
  using v2 = process_inquiry::packed::midi_message_report_v2;
};
template <> struct type_to_packed<process_inquiry::midi_message_report_reply> {
  static constexpr auto id = message::pi_mm_report_reply;
  using v1 = not_available;
  using v2 = process_inquiry::packed::midi_message_report_reply_v2;
};
template <> struct type_to_packed<process_inquiry::midi_message_report_end> {
  static constexpr auto id = message::pi_mm_report_end;
  using v1 = not_available;
  using v2 = empty;
};

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
constexpr O write_header(O first, S const last, struct header const &h, message const id) {
  auto hdr = static_cast<packed::header>(h);
  hdr.sub_id_2 = static_cast<std::byte>(id);
  return details::safe_copy(first, last, hdr);
}

template <typename ExternalType, typename InternalType, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O write_header_body(O first, S const last, header const &hdr, InternalType const &t) {
  first = write_header(first, last, hdr, details::type_to_packed<InternalType>::id);
  if constexpr (!std::is_same_v<ExternalType, details::empty>) {
    first = safe_copy(first, last, static_cast<ExternalType>(t));
  }
  return first;
}

/// Wraps an array of bytes that is large enough to hold an instance of type \p T. \p T is a
/// trivially copyable type with alignment of 1 so can be safely copied into the byte array.
/// A data() member enables access to that array.
template <typename T>
  requires(std::is_trivially_copyable_v<T> && alignof(T) == 1)
class byte_array_wrapper {
public:
  explicit constexpr byte_array_wrapper(T const& other) { std::memcpy(arr_.data(), &other, sizeof(T)); }
  constexpr std::byte const* data() const noexcept { return arr_.data(); }

private:
  byte_array<sizeof(T)> arr_;
};

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S, property_exchange::property_exchange_type Pet>
constexpr O write_pe(O first, S const last, header const &hdr, property_exchange::property_exchange<Pet> const &pe,
                     message const id) {
  first = details::write_header(first, last, hdr, id);

  using property_exchange::packed::property_exchange_pt1;
  using property_exchange::packed::property_exchange_pt2;

  byte_array_wrapper const part1{static_cast<property_exchange_pt1>(pe)};
  first =
      details::write_packed_with_tail(first, last, part1.data(), offsetof(property_exchange_pt1, header), pe.header);
  byte_array_wrapper const part2{static_cast<property_exchange_pt2>(pe)};
  first = details::write_packed_with_tail(first, last, part2.data(), offsetof(property_exchange_pt2, data), pe.data);
  return first;
}

}  // end namespace details

struct trivial_sentinel {
  /// Trivial sentinels are never equal to an object of a different type.
  template <typename T> friend constexpr bool operator==(trivial_sentinel, T) noexcept { return false; }
  /// Trivial sentinels always compare equal.
  friend constexpr bool operator==(trivial_sentinel, trivial_sentinel) noexcept { return true; }
};

template <typename T, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, T const &t) {
  if (hdr.version == b7{1U}) {
    using v1_type = details::type_to_packed<T>::v1;
    if constexpr (!std::is_same_v<v1_type, details::not_available>) {
      first = details::write_header_body<v1_type>(first, last, hdr, t);
    }
    return first;
  }
  return details::write_header_body<typename details::type_to_packed<T>::v2>(first, last, hdr, t);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, endpoint_reply const &reply) {
  first = details::write_header(first, last, hdr, details::type_to_packed<endpoint_reply>::id);
  details::byte_array_wrapper const v1{static_cast<packed::endpoint_reply_v1>(reply)};
  return details::write_packed_with_tail(first, last, v1.data(), offsetof(packed::endpoint_reply_v1, data),
                                         reply.information);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, struct ack const &ack) {
  first = details::write_header(first, last, hdr, details::type_to_packed<struct ack>::id);
  details::byte_array_wrapper const v1{static_cast<packed::ack_v1>(ack)};
  return details::write_packed_with_tail(first, last, v1.data(), offsetof(packed::ack_v1, message), ack.message);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S last, header const &hdr, struct nak const &nak) {
  first = details::write_header(first, last, hdr, details::type_to_packed<struct nak>::id);
  if (hdr.version == b7{1U}) {
    return first;
  }
  details::byte_array_wrapper const v2{static_cast<packed::nak_v2>(nak)};
  return details::write_packed_with_tail(first, last, v2.data(), offsetof(packed::nak_v2, message), nak.message);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr,
                           profile_configuration::details_reply const &reply) {
  using profile_configuration::packed::details_reply_v1;
  first = details::write_header(first, last, hdr, details::type_to_packed<profile_configuration::details_reply>::id);
  details::byte_array_wrapper const v1{static_cast<details_reply_v1>(reply)};
  return details::write_packed_with_tail(first, last, v1.data(), offsetof(details_reply_v1, data), reply.data);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr,
                           profile_configuration::inquiry_reply const &reply) {
  using profile_configuration::packed::inquiry_reply_v1_pt1;
  using profile_configuration::packed::inquiry_reply_v1_pt2;

  first = details::write_header(first, last, hdr, details::type_to_packed<profile_configuration::inquiry_reply>::id);
  details::byte_array_wrapper const part1{static_cast<inquiry_reply_v1_pt1>(reply)};
  first =
      details::write_packed_with_tail(first, last, part1.data(), offsetof(inquiry_reply_v1_pt1, ids), reply.enabled);
  details::byte_array_wrapper const part2{static_cast<inquiry_reply_v1_pt2>(reply)};
  first =
      details::write_packed_with_tail(first, last, part2.data(), offsetof(inquiry_reply_v1_pt2, ids), reply.disabled);
  return first;
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, profile_configuration::specific_data const &sd) {
  using profile_configuration::packed::specific_data_v1;
  first = details::write_header(first, last, hdr, details::type_to_packed<profile_configuration::specific_data>::id);
  details::byte_array_wrapper const v1{static_cast<specific_data_v1>(sd)};
  return details::write_packed_with_tail(first, last, v1.data(), offsetof(specific_data_v1, data), sd.data);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, property_exchange::get const &pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, property_exchange::get_reply const &pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, property_exchange::set const &pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, property_exchange::set_reply const &pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, property_exchange::subscription const &pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, property_exchange::subscription_reply const &pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, property_exchange::notify const &pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}

}  // end namespace midi2::ci

#endif  // MIDI2_CI_CREATE_MESSAGE_HPP
