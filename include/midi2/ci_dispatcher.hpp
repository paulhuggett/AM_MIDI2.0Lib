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

#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

using midi_ci = ci::midi_ci;

using profile_span = std::span<std::byte, 5>;
constexpr auto bytes = std::span<std::byte>{};

template <typename T> concept management_backend = requires(T && v) {
  { v.check_muid(std::uint8_t{} /*group*/, std::uint32_t{} /*muid*/) } -> std::convertible_to<bool>;

  { v.discovery(midi_ci{}, ci::discovery{}) } -> std::same_as<void>;
  { v.discovery_reply(midi_ci{}, ci::discovery_reply{}) } -> std::same_as<void>;
  { v.endpoint_info(midi_ci{}, ci::endpoint_info{}) } -> std::same_as<void>;
  { v.endpoint_info_reply(midi_ci{}, ci::endpoint_info_reply{}) } -> std::same_as<void>;
  { v.invalidate_muid(midi_ci{}, ci::invalidate_muid{}) } -> std::same_as<void>;
  { v.ack(midi_ci{}, ci::ack{}) } -> std::same_as<void>;
  { v.nak(midi_ci{}, ci::nak{}) } -> std::same_as<void>;

  { v.unknown_midici(midi_ci{}) } -> std::same_as<void>;
  { v.buffer_overflow() } -> std::same_as<void>;
};

template <typename T>
concept profile_backend = requires(T &&v) {
  { v.inquiry(midi_ci{}) } -> std::same_as<void>;
  { v.inquiry_reply(midi_ci{}, ci::profile_configuration::inquiry_reply{}) } -> std::same_as<void>;
  { v.added(midi_ci{}, ci::profile_configuration::added{}) } -> std::same_as<void>;
  { v.removed(midi_ci{}, ci::profile_configuration::removed{}) } -> std::same_as<void>;
  { v.details(midi_ci{}, ci::profile_configuration::details{}) } -> std::same_as<void>;
  { v.details_reply(midi_ci{}, ci::profile_configuration::details_reply{}) } -> std::same_as<void>;
  { v.on(midi_ci{}, ci::profile_configuration::on{}) } -> std::same_as<void>;
  { v.off(midi_ci{}, ci::profile_configuration::off{}) } -> std::same_as<void>;
  { v.enabled(midi_ci{}, ci::profile_configuration::enabled{}) } -> std::same_as<void>;
  { v.disabled(midi_ci{}, ci::profile_configuration::disabled{}) } -> std::same_as<void>;
  { v.specific_data(midi_ci{}, ci::profile_configuration::specific_data{}) } -> std::same_as<void>;
};

template <typename T>
concept property_exchange_backend = requires(T &&v) {
  { v.capabilities(midi_ci{}, ci::property_exchange::capabilities{}) } -> std::same_as<void>;
  { v.capabilities_reply(midi_ci{}, ci::property_exchange::capabilities_reply{}) } -> std::same_as<void>;

  { v.get(midi_ci{}, ci::property_exchange::get{}) } -> std::same_as<void>;
  { v.get_reply(midi_ci{}, ci::property_exchange::get_reply{}) } -> std::same_as<void>;
  { v.set(midi_ci{}, ci::property_exchange::set{}) } -> std::same_as<void>;
  { v.set_reply(midi_ci{}, ci::property_exchange::set_reply{}) } -> std::same_as<void>;

  { v.subscription(midi_ci{}, ci::property_exchange::subscription{}) } -> std::same_as<void>;
  { v.subscription_reply(midi_ci{}, ci::property_exchange::subscription_reply{}) } -> std::same_as<void>;
  { v.notify(midi_ci{}, ci::property_exchange::notify{}) } -> std::same_as<void>;
};

template <typename T>
concept process_inquiry_backend = requires(T &&v) {
  { v.capabilities(midi_ci{}) } -> std::same_as<void>;
  { v.capabilities_reply(midi_ci{}, ci::process_inquiry::capabilities_reply{}) } -> std::same_as<void>;
  { v.midi_message_report(midi_ci{}, ci::process_inquiry::midi_message_report{}) } -> std::same_as<void>;
  { v.midi_message_report_reply(midi_ci{}, ci::process_inquiry::midi_message_report_reply{}) } -> std::same_as<void>;
  { v.midi_message_report_end(midi_ci{}) } -> std::same_as<void>;
};

template <typename T>
concept ci_dispatcher_config = requires(T v) {
  { v.management } -> management_backend;
  { v.profile } -> profile_backend;
  { v.property_exchange } -> property_exchange_backend;
  { v.process_inquiry } -> process_inquiry_backend;
};

struct management_null {
  constexpr bool check_muid(std::uint8_t /*group*/, std::uint32_t /*muid*/) { return false; }

  constexpr void discovery(midi_ci const &, ci::discovery const &) { /* do nothing */ }
  constexpr void discovery_reply(midi_ci const &, ci::discovery_reply const &) { /* do nothing */ }
  constexpr void endpoint_info(midi_ci const &, ci::endpoint_info const &) { /* do nothing*/ }
  constexpr void endpoint_info_reply(midi_ci const &, ci::endpoint_info_reply const &) { /* do nothing */ }
  constexpr void invalidate_muid(midi_ci const &, ci::invalidate_muid const &) { /* do nothing */ }
  constexpr void ack(midi_ci const &, ci::ack const &) { /* do nothing */ }
  constexpr void nak(midi_ci const &, ci::nak const &) { /* do nothing */ }

  constexpr void unknown_midici(midi_ci const &) { /* do nothing */ }
  constexpr void buffer_overflow() { /* do nothing */ }
};

static_assert(management_backend<management_null>);

struct profile_null {
  constexpr void inquiry(midi_ci const &) { /* do nothing */ }
  constexpr void inquiry_reply(midi_ci const &, ci::profile_configuration::inquiry_reply const &) { /* do nothing */ }
  constexpr void added(midi_ci const &, ci::profile_configuration::added const &) { /* do nothing */ }
  constexpr void removed(midi_ci const &, ci::profile_configuration::removed const &) { /* do nothing */ }
  constexpr void details(midi_ci const &, ci::profile_configuration::details const &) { /* do nothing */ }
  constexpr void details_reply(midi_ci const &, ci::profile_configuration::details_reply const &) { /* do nothing */ }
  constexpr void on(midi_ci const &, ci::profile_configuration::on const &) { /* do nothing */ }
  constexpr void off(midi_ci const &, ci::profile_configuration::off const &) { /* do nothing */ }
  constexpr void enabled(midi_ci const &, ci::profile_configuration::enabled const &) { /* do nothing */ }
  constexpr void disabled(midi_ci const &, ci::profile_configuration::disabled const &) { /* do nothing */ }
  constexpr void specific_data(midi_ci const &, ci::profile_configuration::specific_data const &) { /* do nothing */ }
};

static_assert(profile_backend<profile_null>);

struct property_exchange_null {
  constexpr void capabilities(midi_ci const &, ci::property_exchange::capabilities const &) { /* do nothing */ }
  constexpr void capabilities_reply(midi_ci const &, ci::property_exchange::capabilities_reply const &) { /* do nothing */ }

  constexpr void get(midi_ci const &, ci::property_exchange::get const &) { /* do nothing */ }
  constexpr void get_reply(midi_ci const &, ci::property_exchange::get_reply const &) { /* do nothing */ }
  constexpr void set(midi_ci const &, ci::property_exchange::set const &) { /* do nothing */ }
  constexpr void set_reply(midi_ci const &, ci::property_exchange::set_reply const &) { /* do nothing */ }

  constexpr void subscription(midi_ci const &, ci::property_exchange::subscription const &) { /* do nothing */ }
  constexpr void subscription_reply(midi_ci const &, ci::property_exchange::subscription_reply const &) { /* do nothing */ }
  constexpr void notify(midi_ci const &, ci::property_exchange::notify const &) { /* do nothing */ }
};

static_assert(property_exchange_backend<property_exchange_null>);

struct process_inquiry_null {
  constexpr void capabilities(midi_ci const &) { /* do nothing */ }
  constexpr void capabilities_reply(midi_ci const &, ci::process_inquiry::capabilities_reply const &) { /* do nothing */ }
  constexpr void midi_message_report(midi_ci const &, ci::process_inquiry::midi_message_report const &) { /* do nothing */ }
  constexpr void midi_message_report_reply(midi_ci const &, ci::process_inquiry::midi_message_report_reply const &) { /* do nothing */ }
  constexpr void midi_message_report_end(midi_ci const &) { /* do nothing */ }
};

static_assert(process_inquiry_backend<process_inquiry_null>);

class management_callbacks {
public:
  management_callbacks() = default;
  management_callbacks(management_callbacks const &) = default;
  management_callbacks(management_callbacks &&) noexcept = default;
  virtual ~management_callbacks() noexcept = default;

  management_callbacks &operator=(management_callbacks const &) = default;
  management_callbacks &operator=(management_callbacks &&) noexcept = default;

  virtual bool check_muid(std::uint8_t /*group*/, std::uint32_t /*muid*/) { return false; }

  virtual void discovery(midi_ci const &, ci::discovery const &) { /* do nothing */ }
  virtual void discovery_reply(midi_ci const &, ci::discovery_reply const &) { /* do nothing */ }
  virtual void endpoint_info(midi_ci const &, ci::endpoint_info const &) { /* do nothing*/ }
  virtual void endpoint_info_reply(midi_ci const &, ci::endpoint_info_reply const &) { /* do nothing */ }
  virtual void invalidate_muid(midi_ci const &, ci::invalidate_muid const &) { /* do nothing */ }
  virtual void ack(midi_ci const &, ci::ack const &) { /* do nothing */ }
  virtual void nak(midi_ci const &, ci::nak const &) { /* do nothing */ }

  virtual void unknown_midici(midi_ci const &) { /* do nothing */ }
  virtual void buffer_overflow() { /* do nothing */ }
};

static_assert(management_backend<management_callbacks>);

class profile_callbacks {
public:
  profile_callbacks() = default;
  profile_callbacks(profile_callbacks const &) = default;
  profile_callbacks(profile_callbacks &&) noexcept = default;
  virtual ~profile_callbacks() noexcept = default;

  profile_callbacks &operator=(profile_callbacks const &) = default;
  profile_callbacks &operator=(profile_callbacks &&) noexcept = default;

  virtual void inquiry(midi_ci const &) { /* do nothing */ }
  virtual void inquiry_reply(midi_ci const &, ci::profile_configuration::inquiry_reply const &) { /* do nothing */ }
  virtual void added(midi_ci const &, ci::profile_configuration::added const &) { /* do nothing */ }
  virtual void removed(midi_ci const &, ci::profile_configuration::removed const &) { /* do nothing */ }
  virtual void details(midi_ci const &, ci::profile_configuration::details const &) { /* do nothing */ }
  virtual void details_reply(midi_ci const &, ci::profile_configuration::details_reply const &) { /* do nothing */ }
  virtual void on(midi_ci const &, ci::profile_configuration::on const &) { /* do nothing */ }
  virtual void off(midi_ci const &, ci::profile_configuration::off const &) { /* do nothing */ }
  virtual void enabled(midi_ci const &, ci::profile_configuration::enabled const &) { /* do nothing */ }
  virtual void disabled(midi_ci const &, ci::profile_configuration::disabled const &) { /* do nothing */ }
  virtual void specific_data(midi_ci const &, ci::profile_configuration::specific_data const &) { /* do nothing */ }
};

static_assert(profile_backend<profile_callbacks>);

class property_exchange_callbacks {
public:
  property_exchange_callbacks() = default;
  property_exchange_callbacks(property_exchange_callbacks const &) = default;
  property_exchange_callbacks(property_exchange_callbacks &&) noexcept = default;
  virtual ~property_exchange_callbacks() noexcept = default;

  property_exchange_callbacks &operator=(property_exchange_callbacks const &) = default;
  property_exchange_callbacks &operator=(property_exchange_callbacks &&) noexcept = default;

  virtual void capabilities(midi_ci const &, ci::property_exchange::capabilities const &) { /* do nothing */ }
  virtual void capabilities_reply(midi_ci const &, ci::property_exchange::capabilities_reply const &) { /* do nothing */ }

  virtual void get(midi_ci const &, ci::property_exchange::get const &) { /* do nothing */ }
  virtual void get_reply(midi_ci const &, ci::property_exchange::get_reply const &) { /* do nothing */ }
  virtual void set(midi_ci const &, ci::property_exchange::set const &) { /* do nothing */ }
  virtual void set_reply(midi_ci const &, ci::property_exchange::set_reply const &) { /* do nothing */ }

  virtual void subscription(midi_ci const &, ci::property_exchange::subscription const &) { /* do nothing */ }
  virtual void subscription_reply(midi_ci const &, ci::property_exchange::subscription_reply const &) { /* do nothing */ }
  virtual void notify(midi_ci const &, ci::property_exchange::notify const &) { /* do nothing */ }
};

static_assert(property_exchange_backend<property_exchange_callbacks>);

class process_inquiry_callbacks {
public:
  process_inquiry_callbacks() = default;
  process_inquiry_callbacks(process_inquiry_callbacks const &) = default;
  process_inquiry_callbacks(process_inquiry_callbacks &&) noexcept = default;
  virtual ~process_inquiry_callbacks() noexcept = default;

  process_inquiry_callbacks &operator=(process_inquiry_callbacks const &) = default;
  process_inquiry_callbacks &operator=(process_inquiry_callbacks &&) noexcept = default;

  virtual void capabilities(midi_ci const &) { /* do nothing */ }
  virtual void capabilities_reply(midi_ci const &, ci::process_inquiry::capabilities_reply const &) { /* do nothing */ }
  virtual void midi_message_report(midi_ci const &, ci::process_inquiry::midi_message_report const &) { /* do nothing */ }
  virtual void midi_message_report_reply(midi_ci const &, ci::process_inquiry::midi_message_report_reply const &) { /* do nothing */ }
  virtual void midi_message_report_end(midi_ci const &) { /* do nothing */ }
};

static_assert(process_inquiry_backend<process_inquiry_callbacks>);

template <typename T> concept unaligned_copyable = alignof(T) == 1 && std::is_trivially_copyable_v<T>;

template <ci_dispatcher_config Config> class ci_dispatcher {
public:
  explicit ci_dispatcher(Config config) : config_{config} {}

  void startSysex7(std::uint8_t group, std::byte deviceId);
  void endSysex7() {}

  void processMIDICI(std::byte s7Byte);

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

template <ci_dispatcher_config Config>
void ci_dispatcher<Config>::startSysex7(std::uint8_t group, std::byte device_id) {
  midici_ = midi_ci{};
  midici_.group = group;
  midici_.params.device_id = static_cast<std::uint8_t>(device_id);

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
  midici_.params.version = static_cast<std::uint8_t>(h->version);
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

    config_.management.unknown_midici(midici_);
  } else if (midici_.params.local_muid != M2_CI_BROADCAST &&
             !config_.management.check_muid(midici_.group, midici_.params.local_muid)) {
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
    config_.management.discovery(midici_, ci::discovery{*v});
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
    config_.management.discovery_reply(midici_, ci::discovery_reply{*v});
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
  config_.management.invalidate_muid(midici_, ci::invalidate_muid{*std::bit_cast<type const *>(buffer_.data())});
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
  config_.management.ack(midici_, ci::ack{*ptr});
  consumer_ = &ci_dispatcher::discard;
}

// nak
// ~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::nak() {
  using v1_type = ci::packed::nak_v1;
  using v2_type = ci::packed::nak_v2;

  auto const handler = [this](unaligned_copyable auto const &reply) {
    config_.management.nak(midici_, ci::nak{reply});
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
  config_.management.endpoint_info(midici_, ci::endpoint_info{*std::bit_cast<type const *>(buffer_.data())});
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
  config_.management.endpoint_info_reply(midici_, ci::endpoint_info_reply{*ptr});
  consumer_ = &ci_dispatcher::discard;
}

// profile inquiry
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_inquiry() {
  config_.profile.inquiry(midici_);
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
  config_.profile.inquiry_reply(midici_, ci::profile_configuration::inquiry_reply{*pt1, *pt2});
  consumer_ = &ci_dispatcher::discard;
}

// profile added
// ~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_added() {
  using type = ci::profile_configuration::packed::added_v1;
  assert(pos_ == sizeof(type));
  config_.profile.added(midici_, ci::profile_configuration::added{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &ci_dispatcher::discard;
}

// profile removed
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_removed() {
  using type = ci::profile_configuration::packed::removed_v1;
  assert(pos_ == sizeof(type));
  config_.profile.removed(midici_, ci::profile_configuration::removed{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &ci_dispatcher::discard;
}

// profile details inquiry
// ~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_details() {
  using type = ci::profile_configuration::packed::details_v1;
  assert(pos_ == sizeof(type));
  config_.profile.details(midici_, ci::profile_configuration::details{*std::bit_cast<type const *>(buffer_.data())});
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
  config_.profile.details_reply(midici_, ci::profile_configuration::details_reply{*reply});
  consumer_ = &ci_dispatcher::discard;
}

// profile on
// ~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::profile_on() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.profile.on(midici_, ci::profile_configuration::on{*v});
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
    config_.profile.off(midici_, ci::profile_configuration::off{*v});
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
    config_.profile.enabled(midici_, ci::profile_configuration::enabled{*v});
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
    config_.profile.disabled(midici_, ci::profile_configuration::disabled{*v});
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
  config_.profile.specific_data(midici_, ci::profile_configuration::specific_data{*reply});
  consumer_ = &ci_dispatcher::discard;
}

// pe capabilities
// ~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::pe_capabilities() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    config_.property_exchange.capabilities(midici_, ci::property_exchange::capabilities{*v});
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
    config_.property_exchange.capabilities_reply(midici_, ci::property_exchange::capabilities_reply{*v});
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
  auto const request = static_cast<std::uint8_t>(pt1->request_id);
  auto const header = std::span<char const>{std::bit_cast<char const *>(&pt1->header[0]), header_length};
  auto const data = std::span<char const>{std::bit_cast<char const *>(&pt2->data[0]), data_length};

  using enum ci_message;
  switch (midici_.type) {
  case pe_get: config_.property_exchange.get(midici_, ci::property_exchange::get{chunk, request, header}); break;
  case pe_get_reply: config_.property_exchange.get_reply(midici_, ci::property_exchange::get_reply{chunk, request, header, data}); break;
  case pe_set: config_.property_exchange.set(midici_, ci::property_exchange::set{chunk, request, header, data}); break;
  case pe_set_reply: config_.property_exchange.set_reply(midici_, ci::property_exchange::set_reply{chunk, request, header, data}); break;
  case pe_sub: config_.property_exchange.subscription(midici_, ci::property_exchange::subscription{chunk, request, header, data}); break;
  case pe_sub_reply: config_.property_exchange.subscription_reply(midici_, ci::property_exchange::subscription_reply{chunk, request, header, data}); break;
  case pe_notify: config_.property_exchange.notify(midici_, ci::property_exchange::notify{chunk, request, header, data}); break;
  default: assert(false); break;
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry capabilities
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_capabilities() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.capabilities(midici_);
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry capabilities reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_capabilities_reply() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.capabilities_reply(
        midici_, ci::process_inquiry::capabilities_reply{
                     *std::bit_cast<ci::process_inquiry::packed::capabilities_reply_v2 const *>(buffer_.data())});
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry midi message report
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_midi_message_report() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.midi_message_report(
        midici_, ci::process_inquiry::midi_message_report{
                     *std::bit_cast<ci::process_inquiry::packed::midi_message_report_v2 const *>(buffer_.data())});
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry midi message report reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_midi_message_report_reply() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.midi_message_report_reply(
        midici_,
        ci::process_inquiry::midi_message_report_reply{
            *std::bit_cast<ci::process_inquiry::packed::midi_message_report_reply_v2 const *>(buffer_.data())});
  }
  consumer_ = &ci_dispatcher::discard;
}

// process inquiry midi message report end
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <ci_dispatcher_config Config> void ci_dispatcher<Config>::process_inquiry_midi_message_report_end() {
  if (midici_.params.version > 1) {
    config_.process_inquiry.midi_message_report_end(midici_);
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
  config_.management.buffer_overflow();
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

}  // end namespace midi2

#endif  // MIDI2_CI_DISPATCHER_HPP
