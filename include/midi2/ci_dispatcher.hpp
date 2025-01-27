//===-- UMP Types -------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_CI_DISPATCHER_HPP
#define MIDI2_CI_DISPATCHER_HPP

#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <span>
#include <string>
#include <type_traits>

#include "midi2/ci_dispatcher_backend.hpp"
#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

namespace midi2::ci {

template <typename T>
concept ci_dispatcher_config = requires(T v) {
  { v.context };
  { v.management } -> dispatcher_backend::management<decltype(v.context)>;
  { v.profile } -> dispatcher_backend::profile<decltype(v.context)>;
  { v.property_exchange } -> dispatcher_backend::property_exchange<decltype(v.context)>;
  { v.process_inquiry } -> dispatcher_backend::process_inquiry<decltype(v.context)>;
};

template <typename T>
concept unaligned_copyable = alignof(T) == 1 && std::is_trivially_copyable_v<T>;

template <ci_dispatcher_config Config> class ci_dispatcher {
public:
  explicit ci_dispatcher(Config config) : config_{config} {}

  void start(std::uint8_t group, std::byte deviceId);
  void finish() { /* here for symmetry with start */
  }

  void processMIDICI(std::byte s7Byte);

  constexpr Config const &config() const noexcept { return config_; }
  constexpr Config &config() noexcept { return config_; }

private:
  static constexpr auto header_size = sizeof(ci::packed::header);

  [[no_unique_address]] Config config_;

  using consumer_fn = void (ci_dispatcher::*)();

  std::size_t count_ = header_size;
  unsigned pos_ = 0;
  consumer_fn consumer_ = &ci_dispatcher::header;

  midi_ci midici_;
  std::array<std::byte, 512> buffer_{};

  void discard();
  void overflow();

  void header();
  // "Management" messages
  void discovery();
  void discovery_reply();
  void endpoint_info();
  void endpoint_info_reply();
  void invalidate_muid();
  void ack();
  void nak();

  // Profile messages
  void profile_inquiry();
  void profile_inquiry_reply();
  void profile_added();
  void profile_removed();
  void profile_details();
  void profile_details_reply();
  void profile_on();
  void profile_off();
  void profile_enabled();
  void profile_disabled();
  void profile_specific_data();

  // Property Exchange messages
  void pe_capabilities();
  void pe_capabilities_reply();
  void property_exchange();

  // Process Inquiry messages
  void process_inquiry_capabilities();
  void process_inquiry_capabilities_reply();
  void process_inquiry_midi_message_report();
  void process_inquiry_midi_message_report_reply();
  void process_inquiry_midi_message_report_end();
};

template <ci_dispatcher_config Config> void ci_dispatcher<Config>::start(std::uint8_t group, std::byte device_id) {
  midici_ = midi_ci{};
  midici_.group = group;
  midici_.params.device_id = to_underlying(device_id);

  count_ = header_size;
  pos_ = 0;
  consumer_ = &ci_dispatcher::header;
}

template <ci_dispatcher_config Config> void ci_dispatcher<Config>::header() {
  struct message_dispatch_info {
    ci_message type;
    std::uint8_t v1size;
    std::uint8_t v2size;
    consumer_fn consumer;
  };

  static std::array const messages = {
      message_dispatch_info{ci_message::profile_inquiry, 0, 0, &ci_dispatcher::profile_inquiry},
      message_dispatch_info{ci_message::profile_inquiry_reply,
                            offsetof(ci::profile_configuration::packed::inquiry_reply_v1_pt1, ids),
                            offsetof(ci::profile_configuration::packed::inquiry_reply_v1_pt1, ids),
                            &ci_dispatcher::profile_inquiry_reply},
      message_dispatch_info{ci_message::profile_set_on, sizeof(ci::profile_configuration::packed::on_v1),
                            sizeof(ci::profile_configuration::packed::on_v2), &ci_dispatcher::profile_on},
      message_dispatch_info{ci_message::profile_set_off, sizeof(ci::profile_configuration::packed::off_v1),
                            sizeof(ci::profile_configuration::packed::off_v2), &ci_dispatcher::profile_off},
      message_dispatch_info{ci_message::profile_enabled, sizeof(ci::profile_configuration::packed::enabled_v1),
                            sizeof(ci::profile_configuration::packed::enabled_v2), &ci_dispatcher::profile_enabled},
      message_dispatch_info{ci_message::profile_disabled, sizeof(ci::profile_configuration::packed::disabled_v1),
                            sizeof(ci::profile_configuration::packed::disabled_v2), &ci_dispatcher::profile_disabled},
      message_dispatch_info{ci_message::profile_added, sizeof(ci::profile_configuration::packed::added_v1),
                            sizeof(ci::profile_configuration::packed::added_v1), &ci_dispatcher::profile_added},
      message_dispatch_info{ci_message::profile_removed, sizeof(ci::profile_configuration::packed::removed_v1),
                            sizeof(ci::profile_configuration::packed::removed_v1), &ci_dispatcher::profile_removed},
      message_dispatch_info{ci_message::profile_details, sizeof(ci::profile_configuration::packed::details_v1),
                            sizeof(ci::profile_configuration::packed::details_v1), &ci_dispatcher::profile_details},
      message_dispatch_info{
          ci_message::profile_details_reply, offsetof(ci::profile_configuration::packed::details_reply_v1, data),
          offsetof(ci::profile_configuration::packed::details_reply_v1, data), &ci_dispatcher::profile_details_reply},
      message_dispatch_info{
          ci_message::profile_specific_data, offsetof(ci::profile_configuration::packed::specific_data_v1, data),
          offsetof(ci::profile_configuration::packed::specific_data_v1, data), &ci_dispatcher::profile_specific_data},

      message_dispatch_info{ci_message::pe_capability, sizeof(ci::property_exchange::packed::capabilities_v1),
                            sizeof(ci::property_exchange::packed::capabilities_v2), &ci_dispatcher::pe_capabilities},
      message_dispatch_info{
          ci_message::pe_capability_reply, sizeof(ci::property_exchange::packed::capabilities_reply_v1),
          sizeof(ci::property_exchange::packed::capabilities_reply_v2), &ci_dispatcher::pe_capabilities_reply},
      message_dispatch_info{ci_message::pe_get, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &ci_dispatcher::property_exchange},
      message_dispatch_info{
          ci_message::pe_get_reply, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &ci_dispatcher::property_exchange},
      message_dispatch_info{ci_message::pe_set, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &ci_dispatcher::property_exchange},
      message_dispatch_info{
          ci_message::pe_set_reply, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &ci_dispatcher::property_exchange},
      message_dispatch_info{ci_message::pe_sub, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &ci_dispatcher::property_exchange},
      message_dispatch_info{
          ci_message::pe_sub_reply, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &ci_dispatcher::property_exchange},
      message_dispatch_info{
          ci_message::pe_notify, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &ci_dispatcher::property_exchange},

      message_dispatch_info{ci_message::pi_capability, 0, 0, &ci_dispatcher::process_inquiry_capabilities},
      message_dispatch_info{ci_message::pi_capability_reply, 0,
                            sizeof(ci::process_inquiry::packed::capabilities_reply_v2),
                            &ci_dispatcher::process_inquiry_capabilities_reply},
      message_dispatch_info{ci_message::pi_mm_report, 0, sizeof(ci::process_inquiry::packed::midi_message_report_v2),
                            &ci_dispatcher::process_inquiry_midi_message_report},
      message_dispatch_info{ci_message::pi_mm_report_reply, 0,
                            sizeof(ci::process_inquiry::packed::midi_message_report_reply_v2),
                            &ci_dispatcher::process_inquiry_midi_message_report_reply},
      message_dispatch_info{ci_message::pi_mm_report_end, 0, 0,
                            &ci_dispatcher::process_inquiry_midi_message_report_end},

      message_dispatch_info{ci_message::discovery, sizeof(ci::packed::discovery_v1), sizeof(ci::packed::discovery_v2),
                            &ci_dispatcher::discovery},
      message_dispatch_info{ci_message::discovery_reply, sizeof(ci::packed::discovery_reply_v1),
                            sizeof(ci::packed::discovery_reply_v2), &ci_dispatcher::discovery_reply},
      message_dispatch_info{ci_message::endpoint_info, sizeof(ci::packed::endpoint_info_v1),
                            sizeof(ci::packed::endpoint_info_v1), &ci_dispatcher::endpoint_info},
      message_dispatch_info{ci_message::endpoint_info_reply, offsetof(ci::packed::endpoint_info_reply_v1, data),
                            offsetof(ci::packed::endpoint_info_reply_v1, data), &ci_dispatcher::endpoint_info_reply},
      message_dispatch_info{ci_message::ack, offsetof(ci::packed::ack_v1, message),
                            offsetof(ci::packed::ack_v1, message), &ci_dispatcher::ack},
      message_dispatch_info{ci_message::invalidate_muid, sizeof(ci::packed::invalidate_muid_v1),
                            sizeof(ci::packed::invalidate_muid_v1), &ci_dispatcher::invalidate_muid},
      message_dispatch_info{ci_message::nak, sizeof(ci::packed::nak_v1), offsetof(ci::packed::nak_v2, message),
                            &ci_dispatcher::nak},
  };
  auto const *const h = std::bit_cast<ci::packed::header const *>(buffer_.data());
  midici_.type = static_cast<ci_message>(h->sub_id_2);
  midici_.params.version = to_underlying(h->version);
  midici_.params.remote_muid = ci::from_le7(h->source_muid);
  midici_.params.local_muid = ci::from_le7(h->destination_muid);

  auto const first = std::begin(messages);
  auto const last = std::end(messages);
  auto const pred = [](message_dispatch_info const &a, message_dispatch_info const &b) { return a.type < b.type; };
  assert(std::is_sorted(first, last, pred));
  if (auto const pos = std::lower_bound(first, last, message_dispatch_info{midici_.type, 0, 0, nullptr}, pred);
      pos == last || pos->type != midici_.type) {
    // An unknown message type.
    consumer_ = &ci_dispatcher::discard;
    count_ = 0;

    config_.management.unknown_midici(config_.context, midici_);
  } else if (midici_.params.local_muid != ci_broadcast &&
             !config_.management.check_muid(config_.context, midici_.group, midici_.params.local_muid)) {
    // The message wasn't intended for us.
    consumer_ = &ci_dispatcher::discard;
    count_ = 0;
  } else {
    assert(pos->consumer != nullptr && "consumer must not be null");
    consumer_ = pos->consumer;
    count_ = midici_.params.version == 1 ? pos->v1size : pos->v2size;
    if (count_ == 0) {
      (this->*consumer_)();
    }
  }
  pos_ = 0;
}

// discovery
// ~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::discovery() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.management.discovery(config_.context, midici_, ci::discovery::make(*v));
  };
  if (midici_.params.version == 1) {
    handler(std::bit_cast<ci::packed::discovery_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::packed::discovery_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// discovery reply
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::discovery_reply() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.management.discovery_reply(config_.context, midici_, ci::discovery_reply{*v});
  };
  if (midici_.params.version == 1) {
    handler(std::bit_cast<ci::packed::discovery_reply_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::packed::discovery_reply_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// invalidate muid
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::invalidate_muid() {
  using type = ci::packed::invalidate_muid_v1;
  config_.management.invalidate_muid(config_.context, midici_,
                                     ci::invalidate_muid{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &ci_dispatcher::discard;
}

// ack
// ~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::ack() {
  using type = ci::packed::ack_v1;
  auto const *const ptr = std::bit_cast<type const *>(buffer_.data());
  auto const message_length = ci::from_le7(ptr->message_length);
  if (pos_ == offsetof(type, message) && message_length > 0) {
    // We've got the fixed-size part of the message. Now wait for the variable-length message buffer.
    count_ = message_length;
    return;
  }
  assert(pos_ == offsetof(type, message) + message_length);
  config_.management.ack(config_.context, midici_, ci::ack{*ptr});
  consumer_ = &ci_dispatcher::discard;
}

// nak
// ~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::nak() {
  using v1_type = ci::packed::nak_v1;
  using v2_type = ci::packed::nak_v2;

  auto const handler = [this](unaligned_copyable auto const &reply) {
    config_.management.nak(config_.context, midici_, ci::nak{reply});
    consumer_ = &ci_dispatcher::discard;
  };
  if (midici_.params.version == 1) {
    assert(pos_ == sizeof(v1_type));
    handler(*std::bit_cast<v1_type const *>(buffer_.data()));
    return;
  }

  auto const *const v2ptr = std::bit_cast<v2_type const *>(buffer_.data());
  auto const message_length = ci::from_le7(v2ptr->message_length);
  if (pos_ == offsetof(ci::packed::nak_v2, message) && message_length > 0) {
    count_ = message_length;
    return;
  }
  assert(pos_ == offsetof(ci::packed::nak_v2, message) + message_length);
  handler(*v2ptr);
}

// endpoint info
// ~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::endpoint_info() {
  using type = ci::packed::endpoint_info_v1;
  assert(pos_ == sizeof(type));
  config_.management.endpoint_info(config_.context, midici_,
                                   ci::endpoint_info{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &ci_dispatcher::discard;
}

// endpoint info reply
// ~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::endpoint_info_reply() {
  using type = ci::packed::endpoint_info_reply_v1;
  auto const *const ptr = std::bit_cast<type const *>(buffer_.data());
  auto const data_length = ci::from_le7(ptr->data_length);
  if (pos_ == offsetof(type, data) && data_length > 0) {
    // We've got the basic structure. Now get the variable length data array.
    count_ = data_length;
    return;
  }
  assert(pos_ == offsetof(type, data) + data_length);
  config_.management.endpoint_info_reply(config_.context, midici_, ci::endpoint_info_reply{*ptr});
  consumer_ = &ci_dispatcher::discard;
}

// profile inquiry
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_inquiry() {
  config_.profile.inquiry(config_.context, midici_);
  consumer_ = &ci_dispatcher::discard;
}

// profile inquiry reply
// ~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_inquiry_reply() {
  using pt1_type = ci::profile_configuration::packed::inquiry_reply_v1_pt1;
  using pt2_type = ci::profile_configuration::packed::inquiry_reply_v1_pt2;
  auto const *const pt1 = std::bit_cast<pt1_type const *>(buffer_.data());
  auto const num_enabled = ci::from_le7(pt1->num_enabled);
  auto const num_enabled_size = num_enabled * sizeof(pt1->ids[0]);
  if (num_enabled > 0 && pos_ == offsetof(pt1_type, ids)) {
    // Wait for the variable length data following the first part and the fixed size portion of part 2.
    count_ = num_enabled_size + offsetof(pt2_type, ids);
    return;
  }

  auto const *const pt2 = std::bit_cast<pt2_type const *>(buffer_.data() + offsetof(pt1_type, ids) + num_enabled_size);
  if (auto const num_disabled = ci::from_le7(pt2->num_disabled);
      num_disabled > 0 && pos_ == offsetof(pt1_type, ids) + num_enabled_size + offsetof(pt2_type, ids)) {
    // Get the variable length "disabled" array.
    count_ = num_disabled * sizeof(pt2->ids[0]);
    return;
  }
  config_.profile.inquiry_reply(config_.context, midici_, ci::profile_configuration::inquiry_reply{*pt1, *pt2});
  consumer_ = &ci_dispatcher::discard;
}

// profile added
// ~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_added() {
  using type = ci::profile_configuration::packed::added_v1;
  assert(pos_ == sizeof(type));
  config_.profile.added(config_.context, midici_,
                        ci::profile_configuration::added{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &ci_dispatcher::discard;
}

// profile removed
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_removed() {
  using type = ci::profile_configuration::packed::removed_v1;
  assert(pos_ == sizeof(type));
  config_.profile.removed(config_.context, midici_,
                          ci::profile_configuration::removed{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &ci_dispatcher::discard;
}

// profile details inquiry
// ~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_details() {
  using type = ci::profile_configuration::packed::details_v1;
  assert(pos_ == sizeof(type));
  config_.profile.details(config_.context, midici_,
                          ci::profile_configuration::details{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &ci_dispatcher::discard;
}

// profile details reply
// ~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_details_reply() {
  using type = ci::profile_configuration::packed::details_reply_v1;
  auto const *const reply = std::bit_cast<type const *>(buffer_.data());
  if (auto const data_length = ci::from_le7(reply->data_length); pos_ == offsetof(type, data) && data_length > 0) {
    count_ = data_length * sizeof(type::data[0]);
    return;
  }
  config_.profile.details_reply(config_.context, midici_, ci::profile_configuration::details_reply{*reply});
  consumer_ = &ci_dispatcher::discard;
}

// profile on
// ~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_on() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.profile.on(config_.context, midici_, ci::profile_configuration::on{*v});
  };
  if (midici_.params.version == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::on_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::on_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// profile off
// ~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_off() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.profile.off(config_.context, midici_, ci::profile_configuration::off{*v});
  };
  if (midici_.params.version == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::off_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::off_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// profile enabled
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_enabled() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.profile.enabled(config_.context, midici_, ci::profile_configuration::enabled{*v});
  };
  if (midici_.params.version == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::enabled_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::enabled_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// profile disabled
// ~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_disabled() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.profile.disabled(config_.context, midici_, ci::profile_configuration::disabled{*v});
  };
  if (midici_.params.version == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::disabled_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::disabled_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// profile specific data
// ~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_specific_data() {
  using type = ci::profile_configuration::packed::specific_data_v1;
  auto const *const reply = std::bit_cast<type const *>(buffer_.data());
  if (auto const data_length = ci::from_le7(reply->data_length); pos_ == offsetof(type, data) && data_length > 0) {
    count_ = data_length * sizeof(type::data[0]);
    return;
  }
  config_.profile.specific_data(config_.context, midici_, ci::profile_configuration::specific_data{*reply});
  consumer_ = &ci_dispatcher::discard;
}

// pe capabilities
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::pe_capabilities() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.property_exchange.capabilities(config_.context, midici_, ci::property_exchange::capabilities{*v});
  };
  if (midici_.params.version == 1) {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// pe capabilities reply
// ~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::pe_capabilities_reply() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.property_exchange.capabilities_reply(config_.context, midici_,
                                                 ci::property_exchange::capabilities_reply{*v});
  };
  if (midici_.params.version == 1) {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_reply_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_reply_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// property exchange
// ~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::property_exchange() {
  using ci::property_exchange::packed::property_exchange_pt1;
  using ci::property_exchange::packed::property_exchange_pt2;
  auto size = offsetof(property_exchange_pt1, header);
  auto const *const pt1 = std::bit_cast<property_exchange_pt1 const *>(buffer_.data());
  auto const header_length = ci::from_le7(pt1->header_length);
  if (pos_ == size && header_length > 0) {
    count_ = header_length * sizeof(pt1->header[0]);
    return;
  }
  size += header_length;
  constexpr auto pt2_size = offsetof(property_exchange_pt2, data);
  if (pos_ == size) {
    count_ = pt2_size;
    return;
  }

  auto const *const pt2 = std::bit_cast<property_exchange_pt2 const *>(buffer_.data() + size);
  size += pt2_size;
  auto const data_length = ci::from_le7(pt2->data_length);
  if (pos_ == size && data_length > 0) {
    count_ = data_length * sizeof(pt2->data[0]);
    return;
  }

  using chunk_info = ci::property_exchange::property_exchange::chunk_info;
  auto const chunk = chunk_info{ci::from_le7(pt2->number_of_chunks), ci::from_le7(pt2->chunk_number)};
  auto const request = to_underlying(pt1->request_id);
  auto const header = std::span<char const>{std::bit_cast<char const *>(&pt1->header[0]), header_length};
  auto const data = std::span<char const>{std::bit_cast<char const *>(&pt2->data[0]), data_length};

  using enum ci_message;
  switch (midici_.type) {
  case pe_get:
    config_.property_exchange.get(config_.context, midici_, ci::property_exchange::get{chunk, request, header});
    break;
  case pe_get_reply:
    config_.property_exchange.get_reply(config_.context, midici_,
                                        ci::property_exchange::get_reply{chunk, request, header, data});
    break;
  case pe_set:
    config_.property_exchange.set(config_.context, midici_, ci::property_exchange::set{chunk, request, header, data});
    break;
  case pe_set_reply:
    config_.property_exchange.set_reply(config_.context, midici_,
                                        ci::property_exchange::set_reply{chunk, request, header, data});
    break;
  case pe_sub:
    config_.property_exchange.subscription(config_.context, midici_,
                                           ci::property_exchange::subscription{chunk, request, header, data});
    break;
  case pe_sub_reply:
    config_.property_exchange.subscription_reply(
        config_.context, midici_, ci::property_exchange::subscription_reply{chunk, request, header, data});
    break;
  case pe_notify:
    config_.property_exchange.notify(config_.context, midici_,
                                     ci::property_exchange::notify{chunk, request, header, data});
    break;
  default: assert(false); break;
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry capabilities
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_capabilities() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.capabilities(config_.context, midici_);
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry capabilities reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_capabilities_reply() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.capabilities_reply(
        config_.context, midici_,
        ci::process_inquiry::capabilities_reply{
            *std::bit_cast<ci::process_inquiry::packed::capabilities_reply_v2 const *>(buffer_.data())});
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry midi message report
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_midi_message_report() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.midi_message_report(
        config_.context, midici_,
        ci::process_inquiry::midi_message_report{
            *std::bit_cast<ci::process_inquiry::packed::midi_message_report_v2 const *>(buffer_.data())});
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry midi message report reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_midi_message_report_reply() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.midi_message_report_reply(
        config_.context, midici_,
        ci::process_inquiry::midi_message_report_reply{
            *std::bit_cast<ci::process_inquiry::packed::midi_message_report_reply_v2 const *>(buffer_.data())});
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry midi message report end
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_midi_message_report_end() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.midi_message_report_end(config_.context, midici_);
  }
  consumer_ = &ci_dispatcher::discard;
}

// discard
// ~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::discard() {
  pos_ = 0;
  count_ = buffer_.size();
}

// overflow
// ~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::overflow() {
  count_ = 0;
  pos_ = 0;
  config_.management.buffer_overflow(config_.context);
  consumer_ = &ci_dispatcher::discard;
}

// processMIDICI
// ~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::processMIDICI(std::byte const s7) {
  if (count_ > 0) {
    if (pos_ >= buffer_.size()) {
      this->overflow();
      return;
    }
    buffer_[pos_] = s7;
    ++pos_;
    --count_;
  }
  if (count_ == 0) {
    (this->*consumer_)();
  }
}

}  // end namespace midi2::ci

#endif  // MIDI2_CI_DISPATCHER_HPP
