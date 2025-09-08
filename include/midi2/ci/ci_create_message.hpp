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
template <> struct type_to_packed<discovery> {
  static constexpr auto id = message::discovery;
  using v1 = packed::discovery_v1;
  using v2 = packed::discovery_v2;
};
template <> struct type_to_packed<discovery_reply> {
  static constexpr auto id = message::discovery_reply;
  using v1 = packed::discovery_reply_v1;
  using v2 = packed::discovery_reply_v2;
};
template <> struct type_to_packed<endpoint> {
  static constexpr auto id = message::endpoint;
  using v1 = packed::endpoint_v1;
  using v2 = packed::endpoint_v1;
};
template <> struct type_to_packed<endpoint_reply> {
  static constexpr auto id = message::endpoint_reply;
};
template <> struct type_to_packed<invalidate_muid> {
  static constexpr auto id = message::invalidate_muid;
  using v1 = packed::invalidate_muid_v1;
  using v2 = packed::invalidate_muid_v1;
};
template <> struct type_to_packed<ack> {
  static constexpr auto id = message::ack;
};
template <> struct type_to_packed<nak> {
  static constexpr auto id = message::nak;
  using v1 = empty;
  using v2 = packed::nak_v2;
};
template <> struct type_to_packed<profile_configuration::added> {
  static constexpr auto id = message::profile_added;
  using v1 = profile_configuration::packed::added_v1;
  using v2 = profile_configuration::packed::added_v1;
};
template <> struct type_to_packed<profile_configuration::removed> {
  static constexpr auto id = message::profile_removed;
  using v1 = profile_configuration::packed::removed_v1;
  using v2 = profile_configuration::packed::removed_v1;
};
template <> struct type_to_packed<profile_configuration::details> {
  static constexpr auto id = message::profile_details;
  using v1 = profile_configuration::packed::details_v1;
  using v2 = profile_configuration::packed::details_v1;
};
template <> struct type_to_packed<profile_configuration::details_reply> {
  static constexpr auto id = message::profile_details_reply;
  using v1 = profile_configuration::packed::details_reply_v1;
  using v2 = profile_configuration::packed::details_reply_v1;
};
template <> struct type_to_packed<profile_configuration::inquiry> {
  static constexpr auto id = message::profile_inquiry;
  using v1 = empty;
  using v2 = empty;
};
template <> struct type_to_packed<profile_configuration::inquiry_reply> {
  static constexpr auto id = message::profile_inquiry_reply;
};
template <> struct type_to_packed<profile_configuration::on> {
  static constexpr auto id = message::profile_set_on;
  using v1 = profile_configuration::packed::on_v1;
  using v2 = profile_configuration::packed::on_v2;
};
template <> struct type_to_packed<profile_configuration::off> {
  static constexpr auto id = message::profile_set_off;
  using v1 = profile_configuration::packed::off_v1;
  using v2 = profile_configuration::packed::off_v2;
};
template <> struct type_to_packed<profile_configuration::enabled> {
  static constexpr auto id = message::profile_enabled;
  using v1 = profile_configuration::packed::enabled_v1;
  using v2 = profile_configuration::packed::enabled_v2;
};
template <> struct type_to_packed<profile_configuration::disabled> {
  static constexpr auto id = message::profile_disabled;
  using v1 = profile_configuration::packed::disabled_v1;
  using v2 = profile_configuration::packed::disabled_v2;
};
template <> struct type_to_packed<profile_configuration::specific_data> {
  static constexpr auto id = message::profile_specific_data;
  using v1 = profile_configuration::packed::specific_data_v1;
  using v2 = profile_configuration::packed::specific_data_v1;
};
template <> struct type_to_packed<property_exchange::capabilities> {
  static constexpr auto id = message::pe_capability;
  using v1 = property_exchange::packed::capabilities_v1;
  using v2 = property_exchange::packed::capabilities_v2;
};
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

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S, property_exchange::property_exchange_type Pet>
constexpr O write_pe(O first, S const last, header const &hdr, property_exchange::property_exchange<Pet> const &pe,
                     message const id) {
  first = details::write_header(first, last, hdr, id);

  using property_exchange::packed::property_exchange_pt1;
  using property_exchange::packed::property_exchange_pt2;
  auto const part1 = static_cast<property_exchange_pt1>(pe);
  static_assert(std::is_trivially_copyable_v<decltype(part1)> && alignof(decltype(part1)) == 1);
  first = details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&part1),
                                          offsetof(property_exchange_pt1, header), pe.header);

  auto const part2 = static_cast<property_exchange_pt2>(pe);
  static_assert(std::is_trivially_copyable_v<decltype(part2)> && alignof(decltype(part2)) == 1);
  first = details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&part2),
                                          offsetof(property_exchange_pt2, data), pe.data);
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
  auto const v1 = static_cast<packed::endpoint_reply_v1>(reply);
  static_assert(std::is_trivially_copyable_v<decltype(v1)> && alignof(decltype(v1)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v1),
                                         offsetof(packed::endpoint_reply_v1, data), reply.information);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, struct ack const &ack) {
  first = details::write_header(first, last, hdr, details::type_to_packed<struct ack>::id);
  auto const v1 = static_cast<packed::ack_v1>(ack);
  static_assert(std::is_trivially_copyable_v<decltype(v1)> && alignof(decltype(v1)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v1),
                                         offsetof(packed::ack_v1, message), ack.message);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S last, header const &hdr, struct nak const &nak) {
  first = details::write_header(first, last, hdr, details::type_to_packed<struct nak>::id);
  if (hdr.version == b7{1U}) {
    return first;
  }
  auto const v2 = static_cast<packed::nak_v2>(nak);
  static_assert(std::is_trivially_copyable_v<decltype(v2)> && alignof(decltype(v2)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v2),
                                         offsetof(packed::nak_v2, message), nak.message);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr,
                           profile_configuration::details_reply const &reply) {
  using profile_configuration::packed::details_reply_v1;
  first = details::write_header(first, last, hdr, details::type_to_packed<profile_configuration::details_reply>::id);
  auto const v1 = static_cast<details_reply_v1>(reply);
  static_assert(std::is_trivially_copyable_v<decltype(v1)> && alignof(decltype(v1)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v1),
                                         offsetof(details_reply_v1, data), reply.data);
}

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr,
                           profile_configuration::inquiry_reply const &reply) {
  using profile_configuration::packed::inquiry_reply_v1_pt1;
  using profile_configuration::packed::inquiry_reply_v1_pt2;

  first = details::write_header(first, last, hdr, details::type_to_packed<profile_configuration::inquiry_reply>::id);

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

template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const &hdr, profile_configuration::specific_data const &sd) {
  using profile_configuration::packed::specific_data_v1;
  first = details::write_header(first, last, hdr, details::type_to_packed<profile_configuration::specific_data>::id);
  auto const v1 = static_cast<specific_data_v1>(sd);
  static_assert(std::is_trivially_copyable_v<decltype(v1)> && alignof(decltype(v1)) == 1);
  return details::write_packed_with_tail(first, last, std::bit_cast<std::byte const *>(&v1),
                                         offsetof(specific_data_v1, data), sd.data);
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
