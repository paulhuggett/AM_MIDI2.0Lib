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
  { v.system } -> dispatcher_backend::system<decltype(v.context)>;
  { v.management } -> dispatcher_backend::management<decltype(v.context)>;
  { v.profile } -> dispatcher_backend::profile<decltype(v.context)>;
  { v.property_exchange } -> dispatcher_backend::property_exchange<decltype(v.context)>;
  { v.process_inquiry } -> dispatcher_backend::process_inquiry<decltype(v.context)>;
};

template <typename Context> struct function_config {
  constexpr explicit function_config(Context c = Context{}) : context{std::move(c)} {}

  [[no_unique_address]] Context context;
  dispatcher_backend::system_function<Context> system;
  dispatcher_backend::management_function<Context> management;
  dispatcher_backend::profile_function<Context> profile;
  dispatcher_backend::property_exchange_function<Context> property_exchange;
  dispatcher_backend::process_inquiry_function<Context> process_inquiry;
};

template <typename Context> function_config(Context) -> function_config<Context>;

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
  message type_ = static_cast<message>(0x00);
  std::uint8_t group_ = 0;
  consumer_fn consumer_ = &ci_dispatcher::header;

  struct header header_;
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
  using header_type = struct header;
  header_ = header_type{};
  header_.device_id = to_underlying(device_id);

  count_ = header_size;
  pos_ = 0;
  group_ = group;
  type_ = static_cast<message>(0x00);
  consumer_ = &ci_dispatcher::header;
}

template <ci_dispatcher_config Config> void ci_dispatcher<Config>::header() {
  struct message_dispatch_info {
    message type;
    std::uint8_t v1size;
    std::uint8_t v2size;
    consumer_fn consumer;
  };

  static std::array const messages = {
      message_dispatch_info{message::profile_inquiry, 0, 0, &ci_dispatcher::profile_inquiry},
      message_dispatch_info{message::profile_inquiry_reply,
                            offsetof(ci::profile_configuration::packed::inquiry_reply_v1_pt1, ids),
                            offsetof(ci::profile_configuration::packed::inquiry_reply_v1_pt1, ids),
                            &ci_dispatcher::profile_inquiry_reply},
      message_dispatch_info{message::profile_set_on, sizeof(ci::profile_configuration::packed::on_v1),
                            sizeof(ci::profile_configuration::packed::on_v2), &ci_dispatcher::profile_on},
      message_dispatch_info{message::profile_set_off, sizeof(ci::profile_configuration::packed::off_v1),
                            sizeof(ci::profile_configuration::packed::off_v2), &ci_dispatcher::profile_off},
      message_dispatch_info{message::profile_enabled, sizeof(ci::profile_configuration::packed::enabled_v1),
                            sizeof(ci::profile_configuration::packed::enabled_v2), &ci_dispatcher::profile_enabled},
      message_dispatch_info{message::profile_disabled, sizeof(ci::profile_configuration::packed::disabled_v1),
                            sizeof(ci::profile_configuration::packed::disabled_v2), &ci_dispatcher::profile_disabled},
      message_dispatch_info{message::profile_added, sizeof(ci::profile_configuration::packed::added_v1),
                            sizeof(ci::profile_configuration::packed::added_v1), &ci_dispatcher::profile_added},
      message_dispatch_info{message::profile_removed, sizeof(ci::profile_configuration::packed::removed_v1),
                            sizeof(ci::profile_configuration::packed::removed_v1), &ci_dispatcher::profile_removed},
      message_dispatch_info{message::profile_details, sizeof(ci::profile_configuration::packed::details_v1),
                            sizeof(ci::profile_configuration::packed::details_v1), &ci_dispatcher::profile_details},
      message_dispatch_info{
          message::profile_details_reply, offsetof(ci::profile_configuration::packed::details_reply_v1, data),
          offsetof(ci::profile_configuration::packed::details_reply_v1, data), &ci_dispatcher::profile_details_reply},
      message_dispatch_info{
          message::profile_specific_data, offsetof(ci::profile_configuration::packed::specific_data_v1, data),
          offsetof(ci::profile_configuration::packed::specific_data_v1, data), &ci_dispatcher::profile_specific_data},

      message_dispatch_info{message::pe_capability, sizeof(ci::property_exchange::packed::capabilities_v1),
                            sizeof(ci::property_exchange::packed::capabilities_v2), &ci_dispatcher::pe_capabilities},
      message_dispatch_info{message::pe_capability_reply, sizeof(ci::property_exchange::packed::capabilities_reply_v1),
                            sizeof(ci::property_exchange::packed::capabilities_reply_v2),
                            &ci_dispatcher::pe_capabilities_reply},
      message_dispatch_info{message::pe_get, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &ci_dispatcher::property_exchange},
      message_dispatch_info{
          message::pe_get_reply, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &ci_dispatcher::property_exchange},
      message_dispatch_info{message::pe_set, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &ci_dispatcher::property_exchange},
      message_dispatch_info{
          message::pe_set_reply, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &ci_dispatcher::property_exchange},
      message_dispatch_info{message::pe_sub, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &ci_dispatcher::property_exchange},
      message_dispatch_info{
          message::pe_sub_reply, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &ci_dispatcher::property_exchange},
      message_dispatch_info{message::pe_notify, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &ci_dispatcher::property_exchange},

      message_dispatch_info{message::pi_capability, 0, 0, &ci_dispatcher::process_inquiry_capabilities},
      message_dispatch_info{message::pi_capability_reply, 0, sizeof(ci::process_inquiry::packed::capabilities_reply_v2),
                            &ci_dispatcher::process_inquiry_capabilities_reply},
      message_dispatch_info{message::pi_mm_report, 0, sizeof(ci::process_inquiry::packed::midi_message_report_v2),
                            &ci_dispatcher::process_inquiry_midi_message_report},
      message_dispatch_info{message::pi_mm_report_reply, 0,
                            sizeof(ci::process_inquiry::packed::midi_message_report_reply_v2),
                            &ci_dispatcher::process_inquiry_midi_message_report_reply},
      message_dispatch_info{message::pi_mm_report_end, 0, 0, &ci_dispatcher::process_inquiry_midi_message_report_end},

      message_dispatch_info{message::discovery, sizeof(packed::discovery_v1), sizeof(packed::discovery_v2),
                            &ci_dispatcher::discovery},
      message_dispatch_info{message::discovery_reply, sizeof(packed::discovery_reply_v1),
                            sizeof(packed::discovery_reply_v2), &ci_dispatcher::discovery_reply},
      message_dispatch_info{message::endpoint_info, sizeof(packed::endpoint_info_v1), sizeof(packed::endpoint_info_v1),
                            &ci_dispatcher::endpoint_info},
      message_dispatch_info{message::endpoint_info_reply, offsetof(packed::endpoint_info_reply_v1, data),
                            offsetof(packed::endpoint_info_reply_v1, data), &ci_dispatcher::endpoint_info_reply},
      message_dispatch_info{message::ack, offsetof(packed::ack_v1, message), offsetof(packed::ack_v1, message),
                            &ci_dispatcher::ack},
      message_dispatch_info{message::invalidate_muid, sizeof(packed::invalidate_muid_v1),
                            sizeof(packed::invalidate_muid_v1), &ci_dispatcher::invalidate_muid},
      message_dispatch_info{message::nak, sizeof(packed::nak_v1), offsetof(packed::nak_v2, message),
                            &ci_dispatcher::nak},
  };
  auto const *const h = std::bit_cast<packed::header const *>(buffer_.data());
  type_ = static_cast<message>(h->sub_id_2);
  header_.version = to_underlying(h->version);
  header_.remote_muid = details::from_le7(h->source_muid);
  header_.local_muid = details::from_le7(h->destination_muid);

  auto const first = std::begin(messages);
  auto const last = std::end(messages);
  auto const pred = [](message_dispatch_info const &a, message_dispatch_info const &b) { return a.type < b.type; };
  assert(std::is_sorted(first, last, pred));
  if (auto const pos = std::lower_bound(first, last, message_dispatch_info{type_, 0, 0, nullptr}, pred);
      pos == last || pos->type != type_) {
    // An unknown message type.
    consumer_ = &ci_dispatcher::discard;
    count_ = 0;

    config_.system.unknown_midici(config_.context, header_);
  } else if (header_.local_muid != broadcast_muid &&
             !config_.system.check_muid(config_.context, group_, header_.local_muid)) {
    // The message wasn't intended for us.
    consumer_ = &ci_dispatcher::discard;
    count_ = 0;
  } else {
    assert(pos->consumer != nullptr && "consumer must not be null");
    consumer_ = pos->consumer;
    count_ = header_.version == 1 ? pos->v1size : pos->v2size;
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
    config_.management.discovery(config_.context, header_, discovery::make(*v));
  };
  if (header_.version == 1) {
    handler(std::bit_cast<packed::discovery_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<packed::discovery_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// discovery reply
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::discovery_reply() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.management.discovery_reply(config_.context, header_, discovery_reply::make(*v));
  };
  if (header_.version == 1) {
    handler(std::bit_cast<packed::discovery_reply_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<packed::discovery_reply_v2 const *>(buffer_.data()));
  }
  consumer_ = &ci_dispatcher::discard;
}

// invalidate muid
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::invalidate_muid() {
  using type = packed::invalidate_muid_v1;
  config_.management.invalidate_muid(config_.context, header_,
                                     invalidate_muid::make(*std::bit_cast<type const *>(buffer_.data())));
  consumer_ = &ci_dispatcher::discard;
}

// ack
// ~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::ack() {
  using type = packed::ack_v1;
  auto const *const ptr = std::bit_cast<type const *>(buffer_.data());
  auto const message_length = details::from_le7(ptr->message_length);
  if (pos_ == offsetof(type, message) && message_length > 0) {
    // We've got the fixed-size part of the message. Now wait for the variable-length message buffer.
    count_ = message_length;
    return;
  }
  assert(pos_ == offsetof(type, message) + message_length);
  config_.management.ack(config_.context, header_, ack::make(*ptr));
  consumer_ = &ci_dispatcher::discard;
}

// nak
// ~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::nak() {
  using v1_type = packed::nak_v1;
  using v2_type = packed::nak_v2;

  auto const handler = [this](unaligned_copyable auto const &reply) {
    config_.management.nak(config_.context, header_, nak::make(reply));
    consumer_ = &ci_dispatcher::discard;
  };
  if (header_.version == 1) {
    assert(pos_ == sizeof(v1_type));
    handler(*std::bit_cast<v1_type const *>(buffer_.data()));
    return;
  }

  auto const *const v2ptr = std::bit_cast<v2_type const *>(buffer_.data());
  auto const message_length = details::from_le7(v2ptr->message_length);
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
  config_.management.endpoint_info(config_.context, header_,
                                   ci::endpoint_info::make(*std::bit_cast<type const *>(buffer_.data())));
  consumer_ = &ci_dispatcher::discard;
}

// endpoint info reply
// ~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::endpoint_info_reply() {
  using type = ci::packed::endpoint_info_reply_v1;
  auto const *const ptr = std::bit_cast<type const *>(buffer_.data());
  auto const data_length = details::from_le7(ptr->data_length);
  if (pos_ == offsetof(type, data) && data_length > 0) {
    // We've got the basic structure. Now get the variable length data array.
    count_ = data_length;
    return;
  }
  assert(pos_ == offsetof(type, data) + data_length);
  config_.management.endpoint_info_reply(config_.context, header_, ci::endpoint_info_reply::make(*ptr));
  consumer_ = &ci_dispatcher::discard;
}

// profile inquiry
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_inquiry() {
  config_.profile.inquiry(config_.context, header_);
  consumer_ = &ci_dispatcher::discard;
}

// profile inquiry reply
// ~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_inquiry_reply() {
  using pt1_type = ci::profile_configuration::packed::inquiry_reply_v1_pt1;
  using pt2_type = ci::profile_configuration::packed::inquiry_reply_v1_pt2;
  auto const *const pt1 = std::bit_cast<pt1_type const *>(buffer_.data());
  auto const num_enabled = details::from_le7(pt1->num_enabled);
  auto const num_enabled_size = num_enabled * sizeof(pt1->ids[0]);
  if (num_enabled > 0 && pos_ == offsetof(pt1_type, ids)) {
    // Wait for the variable length data following the first part and the fixed size portion of part 2.
    count_ = num_enabled_size + offsetof(pt2_type, ids);
    return;
  }

  auto const *const pt2 = std::bit_cast<pt2_type const *>(buffer_.data() + offsetof(pt1_type, ids) + num_enabled_size);
  if (auto const num_disabled = details::from_le7(pt2->num_disabled);
      num_disabled > 0 && pos_ == offsetof(pt1_type, ids) + num_enabled_size + offsetof(pt2_type, ids)) {
    // Get the variable length "disabled" array.
    count_ = num_disabled * sizeof(pt2->ids[0]);
    return;
  }
  config_.profile.inquiry_reply(config_.context, header_, ci::profile_configuration::inquiry_reply::make(*pt1, *pt2));
  consumer_ = &ci_dispatcher::discard;
}

// profile added
// ~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_added() {
  using type = ci::profile_configuration::packed::added_v1;
  assert(pos_ == sizeof(type));
  config_.profile.added(config_.context, header_,
                        ci::profile_configuration::added::make(*std::bit_cast<type const *>(buffer_.data())));
  consumer_ = &ci_dispatcher::discard;
}

// profile removed
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_removed() {
  using type = ci::profile_configuration::packed::removed_v1;
  assert(pos_ == sizeof(type));
  config_.profile.removed(config_.context, header_,
                          ci::profile_configuration::removed::make(*std::bit_cast<type const *>(buffer_.data())));
  consumer_ = &ci_dispatcher::discard;
}

// profile details inquiry
// ~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_details() {
  using type = ci::profile_configuration::packed::details_v1;
  assert(pos_ == sizeof(type));
  config_.profile.details(config_.context, header_,
                          ci::profile_configuration::details::make(*std::bit_cast<type const *>(buffer_.data())));
  consumer_ = &ci_dispatcher::discard;
}

// profile details reply
// ~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_details_reply() {
  using type = ci::profile_configuration::packed::details_reply_v1;
  auto const *const reply = std::bit_cast<type const *>(buffer_.data());
  if (auto const data_length = details::from_le7(reply->data_length); pos_ == offsetof(type, data) && data_length > 0) {
    count_ = data_length * sizeof(type::data[0]);
    return;
  }
  config_.profile.details_reply(config_.context, header_, ci::profile_configuration::details_reply::make(*reply));
  consumer_ = &ci_dispatcher::discard;
}

// profile on
// ~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_on() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.profile.on(config_.context, header_, ci::profile_configuration::on::make(*v));
  };
  if (header_.version == 1) {
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
    config_.profile.off(config_.context, header_, ci::profile_configuration::off::make(*v));
  };
  if (header_.version == 1) {
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
    config_.profile.enabled(config_.context, header_, ci::profile_configuration::enabled::make(*v));
  };
  if (header_.version == 1) {
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
    config_.profile.disabled(config_.context, header_, ci::profile_configuration::disabled::make(*v));
  };
  if (header_.version == 1) {
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
  if (auto const data_length = details::from_le7(reply->data_length); pos_ == offsetof(type, data) && data_length > 0) {
    count_ = data_length * sizeof(type::data[0]);
    return;
  }
  config_.profile.specific_data(config_.context, header_, ci::profile_configuration::specific_data::make(*reply));
  consumer_ = &ci_dispatcher::discard;
}

// pe capabilities
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::pe_capabilities() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.property_exchange.capabilities(config_.context, header_, ci::property_exchange::capabilities::make(*v));
  };
  if (header_.version == 1) {
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
    config_.property_exchange.capabilities_reply(config_.context, header_,
                                                 ci::property_exchange::capabilities_reply::make(*v));
  };
  if (header_.version == 1) {
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
  auto const header_length = details::from_le7(pt1->header_length);
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
  auto const data_length = details::from_le7(pt2->data_length);
  if (pos_ == size && data_length > 0) {
    count_ = data_length * sizeof(pt2->data[0]);
    return;
  }

  using chunk_info = ci::property_exchange::chunk_info;
  auto const chunk = chunk_info{details::from_le7(pt2->number_of_chunks), details::from_le7(pt2->chunk_number)};
  auto const request = to_underlying(pt1->request_id);
  auto const header = std::span<char const>{std::bit_cast<char const *>(&pt1->header[0]), header_length};
  auto const data = std::span<char const>{std::bit_cast<char const *>(&pt2->data[0]), data_length};

  using enum message;
  switch (type_) {
  case pe_get:
    config_.property_exchange.get(config_.context, header_, ci::property_exchange::get::make(chunk, request, header));
    break;
  case pe_get_reply:
    config_.property_exchange.get_reply(config_.context, header_,
                                        ci::property_exchange::get_reply::make(chunk, request, header, data));
    break;
  case pe_set:
    config_.property_exchange.set(config_.context, header_,
                                  ci::property_exchange::set::make(chunk, request, header, data));
    break;
  case pe_set_reply:
    config_.property_exchange.set_reply(config_.context, header_,
                                        ci::property_exchange::set_reply::make(chunk, request, header, data));
    break;
  case pe_sub:
    config_.property_exchange.subscription(config_.context, header_,
                                           ci::property_exchange::subscription::make(chunk, request, header, data));
    break;
  case pe_sub_reply:
    config_.property_exchange.subscription_reply(
        config_.context, header_, ci::property_exchange::subscription_reply::make(chunk, request, header, data));
    break;
  case pe_notify:
    config_.property_exchange.notify(config_.context, header_,
                                     ci::property_exchange::notify::make(chunk, request, header, data));
    break;
  default: assert(false); break;
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry capabilities
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_capabilities() {
  if (header_.version > 1) {
    config_.process_inquiry.capabilities(config_.context, header_);
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry capabilities reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_capabilities_reply() {
  if (header_.version > 1) {
    config_.process_inquiry.capabilities_reply(
        config_.context, header_,
        ci::process_inquiry::capabilities_reply::make(
            *std::bit_cast<ci::process_inquiry::packed::capabilities_reply_v2 const *>(buffer_.data())));
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry midi message report
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_midi_message_report() {
  if (header_.version > 1) {
    config_.process_inquiry.midi_message_report(
        config_.context, header_,
        ci::process_inquiry::midi_message_report::make(
            *std::bit_cast<ci::process_inquiry::packed::midi_message_report_v2 const *>(buffer_.data())));
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry midi message report reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_midi_message_report_reply() {
  if (header_.version > 1) {
    config_.process_inquiry.midi_message_report_reply(
        config_.context, header_,
        ci::process_inquiry::midi_message_report_reply::make(
            *std::bit_cast<ci::process_inquiry::packed::midi_message_report_reply_v2 const *>(buffer_.data())));
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry midi message report end
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_midi_message_report_end() {
  if (header_.version > 1) {
    config_.process_inquiry.midi_message_report_end(config_.context, header_);
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
  config_.system.buffer_overflow(config_.context);
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

template <typename Context>
ci_dispatcher<function_config<Context>> make_function_dispatcher(Context context = Context{}) {
  return midi2::ci::ci_dispatcher{function_config<Context>{std::move(context)}};
}

}  // end namespace midi2::ci

#endif  // MIDI2_CI_DISPATCHER_HPP
