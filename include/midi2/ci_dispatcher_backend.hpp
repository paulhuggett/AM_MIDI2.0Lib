//===-- CI Dispatcher Backend -------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_CI_DISPATCHER_BACKEND_HPP
#define MIDI2_CI_DISPATCHER_BACKEND_HPP

#include <functional>

#include "midi2/ci_types.hpp"

namespace midi2::ci::dispatcher_backend {

template <typename T, typename Context>
concept management = requires(T v, Context context) {
  { v.check_muid(context, std::uint8_t{} /*group*/, std::uint32_t{} /*muid*/) } -> std::convertible_to<bool>;
  { v.unknown_midici(context, midi_ci{}) } -> std::same_as<void>;
  { v.buffer_overflow(context) } -> std::same_as<void>;

  { v.discovery(context, midi_ci{}, ci::discovery{}) } -> std::same_as<void>;
  { v.discovery_reply(context, midi_ci{}, ci::discovery_reply{}) } -> std::same_as<void>;
  { v.endpoint_info(context, midi_ci{}, ci::endpoint_info{}) } -> std::same_as<void>;
  { v.endpoint_info_reply(context, midi_ci{}, ci::endpoint_info_reply{}) } -> std::same_as<void>;
  { v.invalidate_muid(context, midi_ci{}, ci::invalidate_muid{}) } -> std::same_as<void>;
  { v.ack(context, midi_ci{}, ci::ack{}) } -> std::same_as<void>;
  { v.nak(context, midi_ci{}, ci::nak{}) } -> std::same_as<void>;
};

template <typename T, typename Context>
concept profile = requires(T v, Context context) {
  { v.inquiry(context, midi_ci{}) } -> std::same_as<void>;
  { v.inquiry_reply(context, midi_ci{}, ci::profile_configuration::inquiry_reply{}) } -> std::same_as<void>;
  { v.added(context, midi_ci{}, ci::profile_configuration::added{}) } -> std::same_as<void>;
  { v.removed(context, midi_ci{}, ci::profile_configuration::removed{}) } -> std::same_as<void>;
  { v.details(context, midi_ci{}, ci::profile_configuration::details{}) } -> std::same_as<void>;
  { v.details_reply(context, midi_ci{}, ci::profile_configuration::details_reply{}) } -> std::same_as<void>;
  { v.on(context, midi_ci{}, ci::profile_configuration::on{}) } -> std::same_as<void>;
  { v.off(context, midi_ci{}, ci::profile_configuration::off{}) } -> std::same_as<void>;
  { v.enabled(context, midi_ci{}, ci::profile_configuration::enabled{}) } -> std::same_as<void>;
  { v.disabled(context, midi_ci{}, ci::profile_configuration::disabled{}) } -> std::same_as<void>;
  { v.specific_data(context, midi_ci{}, ci::profile_configuration::specific_data{}) } -> std::same_as<void>;
};

template <typename T, typename Context>
concept property_exchange = requires(T v, Context context) {
  { v.capabilities(context, midi_ci{}, ci::property_exchange::capabilities{}) } -> std::same_as<void>;
  { v.capabilities_reply(context, midi_ci{}, ci::property_exchange::capabilities_reply{}) } -> std::same_as<void>;

  { v.get(context, midi_ci{}, ci::property_exchange::get{}) } -> std::same_as<void>;
  { v.get_reply(context, midi_ci{}, ci::property_exchange::get_reply{}) } -> std::same_as<void>;
  { v.set(context, midi_ci{}, ci::property_exchange::set{}) } -> std::same_as<void>;
  { v.set_reply(context, midi_ci{}, ci::property_exchange::set_reply{}) } -> std::same_as<void>;

  { v.subscription(context, midi_ci{}, ci::property_exchange::subscription{}) } -> std::same_as<void>;
  { v.subscription_reply(context, midi_ci{}, ci::property_exchange::subscription_reply{}) } -> std::same_as<void>;
  { v.notify(context, midi_ci{}, ci::property_exchange::notify{}) } -> std::same_as<void>;
};

template <typename T, typename Context>
concept process_inquiry = requires(T v, Context context) {
  { v.capabilities(context, midi_ci{}) } -> std::same_as<void>;
  { v.capabilities_reply(context, midi_ci{}, ci::process_inquiry::capabilities_reply{}) } -> std::same_as<void>;
  { v.midi_message_report(context, midi_ci{}, ci::process_inquiry::midi_message_report{}) } -> std::same_as<void>;
  { v.midi_message_report_reply(context, midi_ci{}, ci::process_inquiry::midi_message_report_reply{}) } -> std::same_as<void>;
  { v.midi_message_report_end(context, midi_ci{}) } -> std::same_as<void>;
};

template <typename Context> struct management_null {
  constexpr bool check_muid(Context, std::uint8_t /*group*/, std::uint32_t /*muid*/) { return false; }
  constexpr void unknown_midici(Context, midi_ci const &) { /* do nothing */ }
  constexpr void buffer_overflow(Context) { /* do nothing */ }

  constexpr void discovery(Context, midi_ci const &, ci::discovery const &) { /* do nothing */ }
  constexpr void discovery_reply(Context, midi_ci const &, ci::discovery_reply const &) { /* do nothing */ }
  constexpr void endpoint_info(Context, midi_ci const &, ci::endpoint_info const &) { /* do nothing*/ }
  constexpr void endpoint_info_reply(Context, midi_ci const &, ci::endpoint_info_reply const &) { /* do nothing */ }
  constexpr void invalidate_muid(Context, midi_ci const &, ci::invalidate_muid const &) { /* do nothing */ }
  constexpr void ack(Context, midi_ci const &, ci::ack const &) { /* do nothing */ }
  constexpr void nak(Context, midi_ci const &, ci::nak const &) { /* do nothing */ }
};
template <typename Context> struct profile_null {
  constexpr void inquiry(Context, midi_ci const &) { /* do nothing */ }
  constexpr void inquiry_reply(Context, midi_ci const &, ci::profile_configuration::inquiry_reply const &) { /* do nothing */ }
  constexpr void added(Context, midi_ci const &, ci::profile_configuration::added const &) { /* do nothing */ }
  constexpr void removed(Context, midi_ci const &, ci::profile_configuration::removed const &) { /* do nothing */ }
  constexpr void details(Context, midi_ci const &, ci::profile_configuration::details const &) { /* do nothing */ }
  constexpr void details_reply(Context, midi_ci const &, ci::profile_configuration::details_reply const &) { /* do nothing */ }
  constexpr void on(Context, midi_ci const &, ci::profile_configuration::on const &) { /* do nothing */ }
  constexpr void off(Context, midi_ci const &, ci::profile_configuration::off const &) { /* do nothing */ }
  constexpr void enabled(Context, midi_ci const &, ci::profile_configuration::enabled const &) { /* do nothing */ }
  constexpr void disabled(Context, midi_ci const &, ci::profile_configuration::disabled const &) { /* do nothing */ }
  constexpr void specific_data(Context, midi_ci const &, ci::profile_configuration::specific_data const &) { /* do nothing */ }
};
template <typename Context> struct property_exchange_null {
  constexpr void capabilities(Context, midi_ci const &, ci::property_exchange::capabilities const &) { /* do nothing */ }
  constexpr void capabilities_reply(Context, midi_ci const &, ci::property_exchange::capabilities_reply const &) { /* do nothing */ }

  constexpr void get(Context, midi_ci const &, ci::property_exchange::get const &) { /* do nothing */ }
  constexpr void get_reply(Context, midi_ci const &, ci::property_exchange::get_reply const &) { /* do nothing */ }
  constexpr void set(Context, midi_ci const &, ci::property_exchange::set const &) { /* do nothing */ }
  constexpr void set_reply(Context, midi_ci const &, ci::property_exchange::set_reply const &) { /* do nothing */ }

  constexpr void subscription(Context, midi_ci const &, ci::property_exchange::subscription const &) { /* do nothing */ }
  constexpr void subscription_reply(Context, midi_ci const &, ci::property_exchange::subscription_reply const &) { /* do nothing */ }
  constexpr void notify(Context, midi_ci const &, ci::property_exchange::notify const &) { /* do nothing */ }
};
template <typename Context> struct process_inquiry_null {
  constexpr void capabilities(Context, midi_ci const &) { /* do nothing */ }
  constexpr void capabilities_reply(Context, midi_ci const &, ci::process_inquiry::capabilities_reply const &) { /* do nothing */ }
  constexpr void midi_message_report(Context, midi_ci const &, ci::process_inquiry::midi_message_report const &) { /* do nothing */ }
  constexpr void midi_message_report_reply(Context, midi_ci const &, ci::process_inquiry::midi_message_report_reply const &) { /* do nothing */ }
  constexpr void midi_message_report_end(Context, midi_ci const &) { /* do nothing */ }
};

static_assert(management<management_null<int>, int>);
static_assert(profile<profile_null<int>, int>);
static_assert(property_exchange<property_exchange_null<int>, int>);
static_assert(process_inquiry<process_inquiry_null<int>, int>);

template <typename Context> struct management_pure {
  constexpr management_pure() noexcept = default;
  constexpr management_pure(management_pure const &) = default;
  constexpr management_pure(management_pure &&) noexcept = default;
  virtual ~management_pure() noexcept = default;

  constexpr management_pure &operator=(management_pure const &) noexcept = default;
  constexpr management_pure &operator=(management_pure &&) noexcept = default;

  virtual bool check_muid(Context, std::uint8_t /*group*/, std::uint32_t /*muid*/) = 0;
  virtual void unknown_midici(Context, midi_ci const &) = 0;
  virtual void buffer_overflow(Context) = 0;

  virtual void discovery(Context, midi_ci const &, ci::discovery const &) = 0;
  virtual void discovery_reply(Context, midi_ci const &, ci::discovery_reply const &) = 0;
  virtual void endpoint_info(Context, midi_ci const &, ci::endpoint_info const &) = 0;
  virtual void endpoint_info_reply(Context, midi_ci const &, ci::endpoint_info_reply const &) = 0;
  virtual void invalidate_muid(Context, midi_ci const &, ci::invalidate_muid const &) = 0;
  virtual void ack(Context, midi_ci const &, ci::ack const &) = 0;
  virtual void nak(Context, midi_ci const &, ci::nak const &) = 0;
};
template <typename Context> struct profile_pure {
  constexpr profile_pure() = default;
  constexpr profile_pure(profile_pure const &) = default;
  constexpr profile_pure(profile_pure &&) noexcept = default;
  constexpr virtual ~profile_pure() noexcept = default;

  constexpr profile_pure &operator=(profile_pure const &) = default;
  constexpr profile_pure &operator=(profile_pure &&) noexcept = default;

  virtual void inquiry(Context, midi_ci const &) = 0;
  virtual void inquiry_reply(Context, midi_ci const &, ci::profile_configuration::inquiry_reply const &) = 0;
  virtual void added(Context, midi_ci const &, ci::profile_configuration::added const &) = 0;
  virtual void removed(Context, midi_ci const &, ci::profile_configuration::removed const &) = 0;
  virtual void details(Context, midi_ci const &, ci::profile_configuration::details const &) = 0;
  virtual void details_reply(Context, midi_ci const &, ci::profile_configuration::details_reply const &) = 0;
  virtual void on(Context, midi_ci const &, ci::profile_configuration::on const &) = 0;
  virtual void off(Context, midi_ci const &, ci::profile_configuration::off const &) = 0;
  virtual void enabled(Context, midi_ci const &, ci::profile_configuration::enabled const &) = 0;
  virtual void disabled(Context, midi_ci const &, ci::profile_configuration::disabled const &) = 0;
  virtual void specific_data(Context, midi_ci const &, ci::profile_configuration::specific_data const &) = 0;
};
template <typename Context> struct property_exchange_pure {
  constexpr property_exchange_pure() = default;
  constexpr property_exchange_pure(property_exchange_pure const &) = default;
  constexpr property_exchange_pure(property_exchange_pure &&) noexcept = default;
  constexpr virtual ~property_exchange_pure() noexcept = default;

  constexpr property_exchange_pure &operator=(property_exchange_pure const &) = default;
  constexpr property_exchange_pure &operator=(property_exchange_pure &&) noexcept = default;

  virtual void capabilities(Context, midi_ci const &, ci::property_exchange::capabilities const &) = 0;
  virtual void capabilities_reply(Context, midi_ci const &, ci::property_exchange::capabilities_reply const &) = 0;

  virtual void get(Context, midi_ci const &, ci::property_exchange::get const &) = 0;
  virtual void get_reply(Context, midi_ci const &, ci::property_exchange::get_reply const &) = 0;
  virtual void set(Context, midi_ci const &, ci::property_exchange::set const &) = 0;
  virtual void set_reply(Context, midi_ci const &, ci::property_exchange::set_reply const &) = 0;

  virtual void subscription(Context, midi_ci const &, ci::property_exchange::subscription const &) = 0;
  virtual void subscription_reply(Context, midi_ci const &, ci::property_exchange::subscription_reply const &) = 0;
  virtual void notify(Context, midi_ci const &, ci::property_exchange::notify const &) = 0;
};
template <typename Context> struct process_inquiry_pure {
  process_inquiry_pure() = default;
  process_inquiry_pure(process_inquiry_pure const &) = default;
  process_inquiry_pure(process_inquiry_pure &&) noexcept = default;
  virtual ~process_inquiry_pure() noexcept = default;

  process_inquiry_pure &operator=(process_inquiry_pure const &) = default;
  process_inquiry_pure &operator=(process_inquiry_pure &&) noexcept = default;

  virtual void capabilities(Context, midi_ci const &) = 0;
  virtual void capabilities_reply(Context, midi_ci const &, ci::process_inquiry::capabilities_reply const &) = 0;
  virtual void midi_message_report(Context, midi_ci const &, ci::process_inquiry::midi_message_report const &) = 0;
  virtual void midi_message_report_reply(Context, midi_ci const &, ci::process_inquiry::midi_message_report_reply const &) = 0;
  virtual void midi_message_report_end(Context, midi_ci const &) = 0;
};

static_assert(management<management_pure<int>, int>);
static_assert(profile<profile_pure<int>, int>);
static_assert(property_exchange<property_exchange_pure<int>, int>);
static_assert(process_inquiry<process_inquiry_pure<int>, int>);

template <typename Context> struct management_base : management_pure<Context> {
  bool check_muid(Context, std::uint8_t /*group*/, std::uint32_t /*muid*/) override { return false; }
  void unknown_midici(Context, midi_ci const &) override { /* do nothing */ }
  void buffer_overflow(Context) override { /* do nothing */ }

  void discovery(Context, midi_ci const &, ci::discovery const &) override { /* do nothing */ }
  void discovery_reply(Context, midi_ci const &, ci::discovery_reply const &) override { /* do nothing */ }
  void endpoint_info(Context, midi_ci const &, ci::endpoint_info const &) override { /* do nothing*/ }
  void endpoint_info_reply(Context, midi_ci const &, ci::endpoint_info_reply const &) override { /* do nothing */ }
  void invalidate_muid(Context, midi_ci const &, ci::invalidate_muid const &) override { /* do nothing */ }
  void ack(Context, midi_ci const &, ci::ack const &) override { /* do nothing */ }
  void nak(Context, midi_ci const &, ci::nak const &) override { /* do nothing */ }
};
template <typename Context> struct profile_base : profile_pure<Context> {
  void inquiry(Context, midi_ci const &) override { /* do nothing */ }
  void inquiry_reply(Context, midi_ci const &, ci::profile_configuration::inquiry_reply const &) override { /* do nothing */ }
  void added(Context, midi_ci const &, ci::profile_configuration::added const &) override { /* do nothing */ }
  void removed(Context, midi_ci const &, ci::profile_configuration::removed const &) override { /* do nothing */ }
  void details(Context, midi_ci const &, ci::profile_configuration::details const &) override { /* do nothing */ }
  void details_reply(Context, midi_ci const &, ci::profile_configuration::details_reply const &) override { /* do nothing */ }
  void on(Context, midi_ci const &, ci::profile_configuration::on const &) override { /* do nothing */ }
  void off(Context, midi_ci const &, ci::profile_configuration::off const &) override { /* do nothing */ }
  void enabled(Context, midi_ci const &, ci::profile_configuration::enabled const &) override { /* do nothing */ }
  void disabled(Context, midi_ci const &, ci::profile_configuration::disabled const &) override { /* do nothing */ }
  void specific_data(Context, midi_ci const &, ci::profile_configuration::specific_data const &) override { /* do nothing */ }
};
template <typename Context> struct property_exchange_base : property_exchange_pure<Context> {
  void capabilities(Context, midi_ci const &, ci::property_exchange::capabilities const &) override { /* do nothing */ }
  void capabilities_reply(Context, midi_ci const &, ci::property_exchange::capabilities_reply const &) override { /* do nothing */ }

  void get(Context, midi_ci const &, ci::property_exchange::get const &) override { /* do nothing */ }
  void get_reply(Context, midi_ci const &, ci::property_exchange::get_reply const &) override { /* do nothing */ }
  void set(Context, midi_ci const &, ci::property_exchange::set const &) override { /* do nothing */ }
  void set_reply(Context, midi_ci const &, ci::property_exchange::set_reply const &) override { /* do nothing */ }

  void subscription(Context, midi_ci const &, ci::property_exchange::subscription const &) override { /* do nothing */ }
  void subscription_reply(Context, midi_ci const &, ci::property_exchange::subscription_reply const &) override { /* do nothing */ }
  void notify(Context, midi_ci const &, ci::property_exchange::notify const &) override { /* do nothing */ }
};
template <typename Context> struct process_inquiry_base : process_inquiry_pure<Context> {
  void capabilities(Context, midi_ci const &) override { /* do nothing */ }
  void capabilities_reply(Context, midi_ci const &, ci::process_inquiry::capabilities_reply const &) override { /* do nothing */ }
  void midi_message_report(Context, midi_ci const &, ci::process_inquiry::midi_message_report const &) override { /* do nothing */ }
  void midi_message_report_reply(Context, midi_ci const &, ci::process_inquiry::midi_message_report_reply const &) override { /* do nothing */ }
  void midi_message_report_end(Context, midi_ci const &) override { /* do nothing */ }
};

template <typename Context> class management_function {
public:
  using check_muid_fn = std::function<bool(Context, std::uint8_t /*group*/, std::uint32_t /*muid*/)>;
  using unknown_fn = std::function<void(Context, midi_ci const &)>;
  using buffer_overflow_fn = std::function<void(Context)>;

  using discovery_fn = std::function<void(Context, midi_ci const &, ci::discovery const &)>;
  using discovery_reply_fn = std::function<void(Context, midi_ci const &, ci::discovery_reply const &)>;
  using endpoint_info_fn = std::function<void(Context, midi_ci const &, ci::endpoint_info const &)>;
  using endpoint_info_reply_fn = std::function<void(Context, midi_ci const &, ci::endpoint_info_reply const &)>;
  using invalidate_muid_fn = std::function<void(Context, midi_ci const &, ci::invalidate_muid const &)>;
  using ack_fn = std::function<void(Context, midi_ci const &, ci::ack const &)>;
  using nak_fn = std::function<void(Context, midi_ci const &, ci::nak const &)>;

  constexpr management_function &on_check_muid(check_muid_fn check_muid) {
    check_muid_ = std::move(check_muid);
    return *this;
  }
  constexpr management_function &on_unknown(unknown_fn unknown) {
    unknown_ = std::move(unknown);
    return *this;
  }
  constexpr management_function &on_buffer_overflow(buffer_overflow_fn overflow) {
    overflow_ = std::move(overflow);
    return *this;
  }

  constexpr management_function &on_discovery(discovery_fn discovery) {
    discovery_ = std::move(discovery);
    return *this;
  }
  constexpr management_function &on_discovery_reply(discovery_reply_fn discovery_reply) {
    discovery_reply_ = std::move(discovery_reply);
    return *this;
  }
  constexpr management_function &on_endpoint_info(endpoint_info_fn endpoint_info) {
    endpoint_info_ = std::move(endpoint_info);
    return *this;
  }
  constexpr management_function &on_endpoint_info_reply(endpoint_info_reply_fn endpoint_info_reply) {
    endpoint_info_reply_ = std::move(endpoint_info_reply);
    return *this;
  }
  constexpr management_function &on_invalidate_muid(invalidate_muid_fn invalidate_muid) {
    invalidate_muid_ = std::move(invalidate_muid);
    return *this;
  }
  constexpr management_function &on_ack(ack_fn ack) {
    ack_ = std::move(ack);
    return *this;
  }
  constexpr management_function &on_nak(nak_fn nak) {
    nak_ = std::move(nak);
    return *this;
  }

  bool check_muid(Context context, std::uint8_t group, std::uint32_t muid) const {
    return check_muid_ ? check_muid_(std::move(context), group, muid) : false;
  }
  void unknown_midici(Context context, midi_ci const &ci) const { call(unknown_, std::move(context), ci); }
  void buffer_overflow(Context context) const { call(overflow_, std::move(context)); }

  void discovery(Context context, midi_ci const &ci, ci::discovery const &d) const {
    call(discovery_, std::move(context), ci, d);
  }
  void discovery_reply(Context context, midi_ci const &ci, ci::discovery_reply const &dr) const {
    call(discovery_reply_, std::move(context), ci, dr);
  }
  void endpoint_info(Context context, midi_ci const &ci, ci::endpoint_info const &epi) const {
    call(endpoint_info_, std::move(context), ci, epi);
  }
  void endpoint_info_reply(Context context, midi_ci const &ci, ci::endpoint_info_reply const &epir) const {
    call(endpoint_info_reply_, std::move(context), ci, epir);
  }
  void invalidate_muid(Context context, midi_ci const &ci, ci::invalidate_muid const &im) const {
    call(invalidate_muid_, std::move(context), ci, im);
  }
  void ack(Context context, midi_ci const &ci, ci::ack const &a) const { call(ack_, std::move(context), ci, a); }
  void nak(Context context, midi_ci const &ci, ci::nak const &n) const { call(nak_, std::move(context), ci, n); }

private:
  check_muid_fn check_muid_;
  unknown_fn unknown_;
  buffer_overflow_fn overflow_;

  discovery_fn discovery_;
  discovery_reply_fn discovery_reply_;
  endpoint_info_fn endpoint_info_;
  endpoint_info_reply_fn endpoint_info_reply_;
  invalidate_muid_fn invalidate_muid_;
  ack_fn ack_;
  nak_fn nak_;
};

static_assert(management<management_function<int>, int>);

template <typename Context> class profile_function {
public:
  using inquiry_fn = std::function<void(Context, midi_ci const &)>;
  using inquiry_reply_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::inquiry_reply const &)>;
  using added_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::added const &)>;
  using removed_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::removed const &)>;
  using details_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::details const &)>;
  using details_reply_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::details_reply const &)>;
  using on_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::on const &)>;
  using off_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::off const &)>;
  using enabled_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::enabled const &)>;
  using disabled_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::disabled const &)>;
  using specific_data_fn = std::function<void(Context, midi_ci const &, ci::profile_configuration::specific_data const &)>;

  constexpr profile_function &on_inquiry(inquiry_fn inquiry) {
    inquiry_ = std::move(inquiry);
    return *this;
  }
  constexpr profile_function &on_inquiry_reply(inquiry_reply_fn inquiry_reply) {
    inquiry_reply_ = std::move(inquiry_reply);
    return *this;
  }
  constexpr profile_function &on_added(added_fn added) {
    added_ = std::move(added);
    return *this;
  }
  constexpr profile_function &on_removed(removed_fn removed) {
    removed_ = std::move(removed);
    return *this;
  }
  constexpr profile_function &on_details(details_fn details) {
    details_ = std::move(details);
    return *this;
  }
  constexpr profile_function &on_details_reply(details_reply_fn details_reply) {
    details_reply_ = std::move(details_reply);
    return *this;
  }
  constexpr profile_function &on_on(on_fn on) {
    on_ = std::move(on);
    return *this;
  }
  constexpr profile_function &on_off(off_fn off) {
    off_ = std::move(off);
    return *this;
  }
  constexpr profile_function &on_enabled(enabled_fn enabled) {
    enabled_ = std::move(enabled);
    return *this;
  }
  constexpr profile_function &on_disabled(disabled_fn disabled) {
    disabled_ = std::move(disabled);
    return *this;
  }
  constexpr profile_function &on_specific_data(specific_data_fn specific_data) {
    specific_data_ = std::move(specific_data);
    return *this;
  }

  void inquiry(Context context, midi_ci const &ci) const { call(inquiry_, context, ci); }
  void inquiry_reply(Context context, midi_ci const &ci, ci::profile_configuration::inquiry_reply const &reply) const {
    call(inquiry_reply_, context, ci, reply);
  }
  void added(Context context, midi_ci const &ci, ci::profile_configuration::added const &a) const {
    call(added_, context, ci, a);
  }
  void removed(Context context, midi_ci const &ci, ci::profile_configuration::removed const &r) const {
    call(removed_, context, ci, r);
  }
  void details(Context context, midi_ci const &ci, ci::profile_configuration::details const &d) const {
    call(details_, context, ci, d);
  }
  void details_reply(Context context, midi_ci const &ci, ci::profile_configuration::details_reply const &reply) const {
    call(details_reply_, context, ci, reply);
  }
  void on(Context context, midi_ci const &ci, ci::profile_configuration::on const &on) const {
    call(on_, context, ci, on);
  }
  void off(Context context, midi_ci const &ci, ci::profile_configuration::off const &off) const {
    call(off_, context, ci, off);
  }
  void enabled(Context context, midi_ci const &ci, ci::profile_configuration::enabled const &enabled) const {
    call(enabled_, context, ci, enabled);
  }
  void disabled(Context context, midi_ci const &ci, ci::profile_configuration::disabled const &disabled) const {
    call(disabled_, context, ci, disabled);
  }
  void specific_data(Context context, midi_ci const &ci, ci::profile_configuration::specific_data const &sd) const {
    call(specific_data_, context, ci, sd);
  }

private:
  inquiry_fn inquiry_;
  inquiry_reply_fn inquiry_reply_;
  added_fn added_;
  removed_fn removed_;
  details_fn details_;
  details_reply_fn details_reply_;
  on_fn on_;
  off_fn off_;
  enabled_fn enabled_;
  disabled_fn disabled_;
  specific_data_fn specific_data_;
};

static_assert(profile<profile_function<int>, int>);

template <typename Context> class property_exchange_function {
public:
  using capabilities_fn = std::function<void(Context, midi_ci const &, ci::property_exchange::capabilities const &)>;
  using capabilities_reply_fn = std::function<void(Context, midi_ci const &, ci::property_exchange::capabilities_reply const &)>;
  using get_fn = std::function<void(Context, midi_ci const &, ci::property_exchange::get const &)>;
  using get_reply_fn = std::function<void(Context, midi_ci const &, ci::property_exchange::get_reply const &)>;
  using set_fn = std::function<void(Context, midi_ci const &, ci::property_exchange::set const &)>;
  using set_reply_fn = std::function<void(Context, midi_ci const &, ci::property_exchange::set_reply const &)>;
  using subscription_fn = std::function<void(Context, midi_ci const &, ci::property_exchange::subscription const &)>;
  using subscription_reply_fn = std::function<void(Context, midi_ci const &, ci::property_exchange::subscription_reply const &)>;
  using notify_fn = std::function<void(Context, midi_ci const &, ci::property_exchange::notify const &)>;

  constexpr property_exchange_function &on_capabilities(capabilities_fn capabilities) {
    capabilities_ = std::move(capabilities);
    return *this;
  }
  constexpr property_exchange_function &on_capabilities_reply(capabilities_reply_fn capabilities_reply) {
    capabilities_reply_ = std::move(capabilities_reply);
    return *this;
  }
  constexpr property_exchange_function &on_get(get_fn get) {
    get_ = std::move(get);
    return *this;
  }
  constexpr property_exchange_function &on_get_reply(get_reply_fn get_reply) {
    get_reply_ = std::move(get_reply);
    return *this;
  }
  constexpr property_exchange_function &on_set(set_fn set) {
    set_ = std::move(set);
    return *this;
  }
  constexpr property_exchange_function &on_set_reply(set_reply_fn set_reply) {
    set_reply_ = std::move(set_reply);
    return *this;
  }
  constexpr property_exchange_function &on_subscription(subscription_fn subscription) {
    subscription_ = std::move(subscription);
    return *this;
  }
  constexpr property_exchange_function &on_subscription_reply(subscription_reply_fn subscription_reply) {
    subscription_reply_ = std::move(subscription_reply);
    return *this;
  }
  constexpr property_exchange_function &on_notify(notify_fn notify) {
    notify_ = std::move(notify);
    return *this;
  }

  void capabilities(Context context, midi_ci const &ci, ci::property_exchange::capabilities const &cap) const {
    call(capabilities_, context, ci, cap);
  }
  void capabilities_reply(Context context, midi_ci const &ci,
                          ci::property_exchange::capabilities_reply const &reply) const {
    call(capabilities_reply_, context, ci, reply);
  }
  void get(Context context, midi_ci const &ci, ci::property_exchange::get const &get) const {
    call(get_, context, ci, get);
  }
  void get_reply(Context context, midi_ci const &ci, ci::property_exchange::get_reply const &reply) const {
    call(get_reply_, context, ci, reply);
  }
  void set(Context context, midi_ci const &ci, ci::property_exchange::set const &set) const {
    call(set_, context, ci, set);
  }
  void set_reply(Context context, midi_ci const &ci, ci::property_exchange::set_reply const &reply) const {
    call(set_reply_, context, ci, reply);
  }
  void subscription(Context context, midi_ci const &ci, ci::property_exchange::subscription const &sub) const {
    call(subscription_, context, ci, sub);
  }
  void subscription_reply(Context context, midi_ci const &ci,
                          ci::property_exchange::subscription_reply const &reply) const {
    call(subscription_reply_, context, ci, reply);
  }
  void notify(Context context, midi_ci const &ci, ci::property_exchange::notify const &notify) const {
    call(notify_, context, ci, notify);
  }

private:
  capabilities_fn capabilities_;
  capabilities_reply_fn capabilities_reply_;
  get_fn get_;
  get_reply_fn get_reply_;
  set_fn set_;
  set_reply_fn set_reply_;
  subscription_fn subscription_;
  subscription_reply_fn subscription_reply_;
  notify_fn notify_;
};

static_assert(property_exchange<property_exchange_function<int>, int>);

template <typename Context> class process_inquiry_function {
public:
  using capabilities_fn = std::function<void(Context, midi_ci const &)>;
  using capabilities_reply_fn = std::function<void(Context, midi_ci const &, ci::process_inquiry::capabilities_reply const &)>;
  using midi_message_report_fn = std::function<void(Context, midi_ci const &, ci::process_inquiry::midi_message_report const &)>;
  using midi_message_report_reply_fn = std::function<void(Context, midi_ci const &, ci::process_inquiry::midi_message_report_reply const &)>;
  using midi_message_report_end_fn = std::function<void(Context, midi_ci const &)>;

  constexpr process_inquiry_function &on_capabilities(capabilities_fn capabilities) {
    capabilities_ = std::move(capabilities);
    return *this;
  }
  constexpr process_inquiry_function &on_capabilities_reply(capabilities_reply_fn capabilities_reply) {
    capabilities_reply_ = std::move(capabilities_reply);
    return *this;
  }
  constexpr process_inquiry_function &on_midi_message_report(midi_message_report_fn mmr) {
    midi_message_report_ = std::move(mmr);
    return *this;
  }
  constexpr process_inquiry_function &on_midi_message_report_reply(midi_message_report_reply_fn mmrr) {
    midi_message_report_reply_ = std::move(mmrr);
    return *this;
  }
  constexpr process_inquiry_function &on_midi_message_report_end(midi_message_report_end_fn mmre) {
    midi_message_report_end_ = std::move(mmre);
    return *this;
  }

  void capabilities(Context context, midi_ci const &ci) const { call(capabilities_, context, ci); }
  void capabilities_reply(Context context, midi_ci const &ci,
                          ci::process_inquiry::capabilities_reply const &reply) const {
    call(capabilities_reply_, context, ci, reply);
  }
  void midi_message_report(Context context, midi_ci const &ci,
                           ci::process_inquiry::midi_message_report const &mmr) const {
    call(midi_message_report_, context, ci, mmr);
  }
  void midi_message_report_reply(Context context, midi_ci const &ci,
                                 ci::process_inquiry::midi_message_report_reply const &reply) const {
    call(midi_message_report_reply_, context, ci, reply);
  }
  void midi_message_report_end(Context context, midi_ci const &ci) const {
    call(midi_message_report_end_, context, ci);
  }

private:
  capabilities_fn capabilities_;
  capabilities_reply_fn capabilities_reply_;
  midi_message_report_fn midi_message_report_;
  midi_message_report_reply_fn midi_message_report_reply_;
  midi_message_report_end_fn midi_message_report_end_;
};

static_assert(process_inquiry<process_inquiry_function<int>, int>);

}  // end namespace midi2::ci::dispatcher_backend

#endif  // MIDI2_CI_DISPATCHER_BACKEND_HPP
