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

#include "midi2/ci/ci_types.hpp"

namespace midi2::ci::dispatcher_backend {

// clang-format off
/// \brief The system messages concept gathers MIDI CI dispatchers callback functions that relate to the system as a
/// whole rather than to a specific group of MIDI messages.
///
/// The functions that is requires are:
///
/// - check_muid: Checks whether the message is addressed to this receiver. If this function returns true, the message
///   is dispatched otherwise it is dropped.
/// - unknown_midici: Called when an unrecognized message is received.
/// - buffer_overflow: Called when the space allocated to the internal buffer is exceeded.
template <typename T, typename Context>
concept system = requires(T v, Context context) {
  { v.check_muid(context, std::uint8_t{} /*group*/, muid{}) } -> std::convertible_to<bool>;
  { v.unknown_midici(context, header{}) } -> std::same_as<void>;
  { v.buffer_overflow(context) } -> std::same_as<void>;
};

template <typename T, typename Context>
concept management = requires(T v, Context context) {
  { v.discovery(context, header{}, ci::discovery{}) } -> std::same_as<void>;
  { v.discovery_reply(context, header{}, ci::discovery_reply{}) } -> std::same_as<void>;
  { v.endpoint(context, header{}, ci::endpoint{}) } -> std::same_as<void>;
  { v.endpoint_reply(context, header{}, ci::endpoint_reply{}) } -> std::same_as<void>;
  { v.invalidate_muid(context, header{}, ci::invalidate_muid{}) } -> std::same_as<void>;
  { v.ack(context, header{}, ci::ack{}) } -> std::same_as<void>;
  { v.nak(context, header{}, ci::nak{}) } -> std::same_as<void>;
};

template <typename T, typename Context>
concept profile = requires(T v, Context context) {
  { v.inquiry(context, header{}) } -> std::same_as<void>;
  { v.inquiry_reply(context, header{}, ci::profile_configuration::inquiry_reply{}) } -> std::same_as<void>;
  { v.added(context, header{}, ci::profile_configuration::added{}) } -> std::same_as<void>;
  { v.removed(context, header{}, ci::profile_configuration::removed{}) } -> std::same_as<void>;
  { v.details(context, header{}, ci::profile_configuration::details{}) } -> std::same_as<void>;
  { v.details_reply(context, header{}, ci::profile_configuration::details_reply{}) } -> std::same_as<void>;
  { v.on(context, header{}, ci::profile_configuration::on{}) } -> std::same_as<void>;
  { v.off(context, header{}, ci::profile_configuration::off{}) } -> std::same_as<void>;
  { v.enabled(context, header{}, ci::profile_configuration::enabled{}) } -> std::same_as<void>;
  { v.disabled(context, header{}, ci::profile_configuration::disabled{}) } -> std::same_as<void>;
  { v.specific_data(context, header{}, ci::profile_configuration::specific_data{}) } -> std::same_as<void>;
};

template <typename T, typename Context>
concept property_exchange = requires(T v, Context context) {
  { v.capabilities(context, header{}, ci::property_exchange::capabilities{}) } -> std::same_as<void>;
  { v.capabilities_reply(context, header{}, ci::property_exchange::capabilities_reply{}) } -> std::same_as<void>;

  { v.get(context, header{}, ci::property_exchange::get{}) } -> std::same_as<void>;
  { v.get_reply(context, header{}, ci::property_exchange::get_reply{}) } -> std::same_as<void>;
  { v.set(context, header{}, ci::property_exchange::set{}) } -> std::same_as<void>;
  { v.set_reply(context, header{}, ci::property_exchange::set_reply{}) } -> std::same_as<void>;

  { v.subscription(context, header{}, ci::property_exchange::subscription{}) } -> std::same_as<void>;
  { v.subscription_reply(context, header{}, ci::property_exchange::subscription_reply{}) } -> std::same_as<void>;
  { v.notify(context, header{}, ci::property_exchange::notify{}) } -> std::same_as<void>;
};

template <typename T, typename Context>
concept process_inquiry = requires(T v, Context context) {
  { v.capabilities(context, header{}) } -> std::same_as<void>;
  { v.capabilities_reply(context, header{}, ci::process_inquiry::capabilities_reply{}) } -> std::same_as<void>;
  { v.midi_message_report(context, header{}, ci::process_inquiry::midi_message_report{}) } -> std::same_as<void>;
  { v.midi_message_report_reply(context, header{}, ci::process_inquiry::midi_message_report_reply{}) } -> std::same_as<void>;
  { v.midi_message_report_end(context, header{}) } -> std::same_as<void>;
};
// clang-format on

// clang-format off
template <typename Context> struct system_null {
  /// Checks whether the message is addressed to this receiver. If this function returns true, the message is
  /// dispatched otherwise it is dropped.
  ///
  /// \param context  The context object passed to all message handlers.
  /// \param group  The MIDI group to which the message was addressed.
  /// \param id  The MUID to which the message was addressed.
  /// \returns True if the message is to be dispatched, false if it is to be ignored.
  constexpr static bool check_muid(Context context, std::uint8_t const group, muid const id) { (void)context; (void)group; (void)id; return false; }
  /// This function is called when an unrecognized message is received.
  ///
  /// \param context  The context object passed to all message handlers.
  /// \param h  The header that for the unrecognized message.
  constexpr static void unknown_midici(Context context, header const &h) { (void)context; (void)h; /* do nothing */ }
  /// \param context  The context object passed to all message handlers.
  constexpr static void buffer_overflow(Context context) { (void)context; /* do nothing */ }
};
template <typename Context> struct management_null {
  constexpr static void discovery(Context, header const &, ci::discovery const &) { /* do nothing */ }
  constexpr static void discovery_reply(Context, header const &, ci::discovery_reply const &) { /* do nothing */ }
  constexpr static void endpoint(Context, header const &, ci::endpoint const &) { /* do nothing*/ }
  constexpr static void endpoint_reply(Context, header const &, ci::endpoint_reply const &) { /* do nothing */ }
  constexpr static void invalidate_muid(Context, header const &, ci::invalidate_muid const &) { /* do nothing */ }
  constexpr static void ack(Context, header const &, ci::ack const &) { /* do nothing */ }
  constexpr static void nak(Context, header const &, ci::nak const &) { /* do nothing */ }
};
template <typename Context> struct profile_null {
  constexpr static void inquiry(Context, header const &) { /* do nothing */ }
  constexpr static void inquiry_reply(Context, header const &, ci::profile_configuration::inquiry_reply const &) { /* do nothing */ }
  constexpr static void added(Context, header const &, ci::profile_configuration::added const &) { /* do nothing */ }
  constexpr static void removed(Context, header const &, ci::profile_configuration::removed const &) { /* do nothing */ }
  constexpr static void details(Context, header const &, ci::profile_configuration::details const &) { /* do nothing */ }
  constexpr static void details_reply(Context, header const &, ci::profile_configuration::details_reply const &) { /* do nothing */ }
  constexpr static void on(Context, header const &, ci::profile_configuration::on const &) { /* do nothing */ }
  constexpr static void off(Context, header const &, ci::profile_configuration::off const &) { /* do nothing */ }
  constexpr static void enabled(Context, header const &, ci::profile_configuration::enabled const &) { /* do nothing */ }
  constexpr static void disabled(Context, header const &, ci::profile_configuration::disabled const &) { /* do nothing */ }
  constexpr static void specific_data(Context, header const &, ci::profile_configuration::specific_data const &) { /* do nothing */ }
};
template <typename Context> struct property_exchange_null {
  constexpr static void capabilities(Context, header const &, ci::property_exchange::capabilities const &) { /* do nothing */ }
  constexpr static void capabilities_reply(Context, header const &, ci::property_exchange::capabilities_reply const &) { /* do nothing */ }

  constexpr static void get(Context, header const &, ci::property_exchange::get const &) { /* do nothing */ }
  constexpr static void get_reply(Context, header const &, ci::property_exchange::get_reply const &) { /* do nothing */ }
  constexpr static void set(Context, header const &, ci::property_exchange::set const &) { /* do nothing */ }
  constexpr static void set_reply(Context, header const &, ci::property_exchange::set_reply const &) { /* do nothing */ }

  constexpr static void subscription(Context, header const &, ci::property_exchange::subscription const &) { /* do nothing */ }
  constexpr static void subscription_reply(Context, header const &, ci::property_exchange::subscription_reply const &) { /* do nothing */ }
  constexpr static void notify(Context, header const &, ci::property_exchange::notify const &) { /* do nothing */ }
};
template <typename Context> struct process_inquiry_null {
  constexpr static void capabilities(Context, header const &) { /* do nothing */ }
  constexpr static void capabilities_reply(Context, header const &, ci::process_inquiry::capabilities_reply const &) { /* do nothing */ }
  constexpr static void midi_message_report(Context, header const &, ci::process_inquiry::midi_message_report const &) { /* do nothing */ }
  constexpr static void midi_message_report_reply(Context, header const &, ci::process_inquiry::midi_message_report_reply const &) { /* do nothing */ }
  constexpr static void midi_message_report_end(Context, header const &) { /* do nothing */ }
};
// clang-format on

static_assert(system<system_null<int>, int>);
static_assert(management<management_null<int>, int>);
static_assert(profile<profile_null<int>, int>);
static_assert(property_exchange<property_exchange_null<int>, int>);
static_assert(process_inquiry<process_inquiry_null<int>, int>);

template <typename Context> struct system_pure {
  constexpr system_pure() noexcept = default;
  constexpr system_pure(system_pure const &) = default;
  constexpr system_pure(system_pure &&) noexcept = default;
  virtual ~system_pure() noexcept = default;

  constexpr system_pure &operator=(system_pure const &) noexcept = default;
  constexpr system_pure &operator=(system_pure &&) noexcept = default;

  /// Checks whether the message is addressed to this receiver. If this function returns true, the message is
  /// dispatched otherwise it is dropped.
  ///
  /// \param context  The context object passed to all message handlers.
  /// \param group  The MIDI group to which the message was addressed.
  /// \param id  The MUID to which the message was addressed.
  /// \returns True if the message is to be dispatched, false if it is to be ignored.
  virtual bool check_muid(Context context, std::uint8_t group, muid id) = 0;
  /// This function is called when an unrecognized message is received.
  ///
  /// \param context  The context object passed to all message handlers.
  /// \param h  The header that for the unrecognized message.
  virtual void unknown_midici(Context context, header const &h) = 0;
  virtual void buffer_overflow(Context) = 0;
};
template <typename Context> struct management_pure {
  constexpr management_pure() noexcept = default;
  constexpr management_pure(management_pure const &) = default;
  constexpr management_pure(management_pure &&) noexcept = default;
  virtual ~management_pure() noexcept = default;

  constexpr management_pure &operator=(management_pure const &) noexcept = default;
  constexpr management_pure &operator=(management_pure &&) noexcept = default;

  virtual void discovery(Context, header const &, ci::discovery const &) = 0;
  virtual void discovery_reply(Context, header const &, ci::discovery_reply const &) = 0;
  virtual void endpoint(Context, header const &, ci::endpoint const &) = 0;
  virtual void endpoint_reply(Context, header const &, ci::endpoint_reply const &) = 0;
  virtual void invalidate_muid(Context, header const &, ci::invalidate_muid const &) = 0;
  virtual void ack(Context, header const &, ci::ack const &) = 0;
  virtual void nak(Context, header const &, ci::nak const &) = 0;
};
template <typename Context> struct profile_pure {
  constexpr profile_pure() = default;
  constexpr profile_pure(profile_pure const &) = default;
  constexpr profile_pure(profile_pure &&) noexcept = default;
  constexpr virtual ~profile_pure() noexcept = default;

  constexpr profile_pure &operator=(profile_pure const &) = default;
  constexpr profile_pure &operator=(profile_pure &&) noexcept = default;

  virtual void inquiry(Context, header const &) = 0;
  virtual void inquiry_reply(Context, header const &, ci::profile_configuration::inquiry_reply const &) = 0;
  virtual void added(Context, header const &, ci::profile_configuration::added const &) = 0;
  virtual void removed(Context, header const &, ci::profile_configuration::removed const &) = 0;
  virtual void details(Context, header const &, ci::profile_configuration::details const &) = 0;
  virtual void details_reply(Context, header const &, ci::profile_configuration::details_reply const &) = 0;
  virtual void on(Context, header const &, ci::profile_configuration::on const &) = 0;
  virtual void off(Context, header const &, ci::profile_configuration::off const &) = 0;
  virtual void enabled(Context, header const &, ci::profile_configuration::enabled const &) = 0;
  virtual void disabled(Context, header const &, ci::profile_configuration::disabled const &) = 0;
  virtual void specific_data(Context, header const &, ci::profile_configuration::specific_data const &) = 0;
};
template <typename Context> struct property_exchange_pure {
  constexpr property_exchange_pure() = default;
  constexpr property_exchange_pure(property_exchange_pure const &) = default;
  constexpr property_exchange_pure(property_exchange_pure &&) noexcept = default;
  constexpr virtual ~property_exchange_pure() noexcept = default;

  constexpr property_exchange_pure &operator=(property_exchange_pure const &) = default;
  constexpr property_exchange_pure &operator=(property_exchange_pure &&) noexcept = default;

  virtual void capabilities(Context, header const &, ci::property_exchange::capabilities const &) = 0;
  virtual void capabilities_reply(Context, header const &, ci::property_exchange::capabilities_reply const &) = 0;

  virtual void get(Context, header const &, ci::property_exchange::get const &) = 0;
  virtual void get_reply(Context, header const &, ci::property_exchange::get_reply const &) = 0;
  virtual void set(Context, header const &, ci::property_exchange::set const &) = 0;
  virtual void set_reply(Context, header const &, ci::property_exchange::set_reply const &) = 0;

  virtual void subscription(Context, header const &, ci::property_exchange::subscription const &) = 0;
  virtual void subscription_reply(Context, header const &, ci::property_exchange::subscription_reply const &) = 0;
  virtual void notify(Context, header const &, ci::property_exchange::notify const &) = 0;
};
template <typename Context> struct process_inquiry_pure {
  process_inquiry_pure() = default;
  process_inquiry_pure(process_inquiry_pure const &) = default;
  process_inquiry_pure(process_inquiry_pure &&) noexcept = default;
  virtual ~process_inquiry_pure() noexcept = default;

  process_inquiry_pure &operator=(process_inquiry_pure const &) = default;
  process_inquiry_pure &operator=(process_inquiry_pure &&) noexcept = default;

  virtual void capabilities(Context, header const &) = 0;
  virtual void capabilities_reply(Context, header const &, ci::process_inquiry::capabilities_reply const &) = 0;
  virtual void midi_message_report(Context, header const &, ci::process_inquiry::midi_message_report const &) = 0;
  virtual void midi_message_report_reply(Context, header const &,
                                         ci::process_inquiry::midi_message_report_reply const &) = 0;
  virtual void midi_message_report_end(Context, header const &) = 0;
};

static_assert(management<management_pure<int>, int>);
static_assert(profile<profile_pure<int>, int>);
static_assert(property_exchange<property_exchange_pure<int>, int>);
static_assert(process_inquiry<process_inquiry_pure<int>, int>);

// clang-format off
template <typename Context> struct system_base : system_pure<Context> {
  /// Checks whether the message is addressed to this receiver. If this function returns true, the message is
  /// dispatched otherwise it is dropped.
  ///
  /// \param context  The context object passed to all message handlers.
  /// \param group  The MIDI group to which the message was addressed.
  /// \param id  The MUID to which the message was addressed.
  /// \returns True if the message is to be dispatched, false if it is to be ignored.
  bool check_muid(Context context, std::uint8_t const group, muid const id) override { (void)context; (void)group; (void)id; return false; }
  /// This function is called when an unrecognized message is received.
  ///
  /// \param context  The context object passed to all message handlers.
  /// \param h  The header that for the unrecognized message.
  void unknown_midici(Context context, header const &h) override { (void)context; (void)h; /* do nothing */ }
  /// \param context  The context object passed to all message handlers.
  void buffer_overflow(Context context) override { (void)context; }
};
template <typename Context> struct management_base : management_pure<Context> {
  void discovery(Context, header const &, ci::discovery const &) override { /* do nothing */ }
  void discovery_reply(Context, header const &, ci::discovery_reply const &) override { /* do nothing */ }
  void endpoint(Context, header const &, ci::endpoint const &) override { /* do nothing*/ }
  void endpoint_reply(Context, header const &, ci::endpoint_reply const &) override { /* do nothing */ }
  void invalidate_muid(Context, header const &, ci::invalidate_muid const &) override { /* do nothing */ }
  void ack(Context, header const &, ci::ack const &) override { /* do nothing */ }
  void nak(Context, header const &, ci::nak const &) override { /* do nothing */ }
};
template <typename Context> struct profile_base : profile_pure<Context> {
  void inquiry(Context, header const &) override { /* do nothing */ }
  void inquiry_reply(Context, header const &, ci::profile_configuration::inquiry_reply const &) override { /* do nothing */ }
  void added(Context, header const &, ci::profile_configuration::added const &) override { /* do nothing */ }
  void removed(Context, header const &, ci::profile_configuration::removed const &) override { /* do nothing */ }
  void details(Context, header const &, ci::profile_configuration::details const &) override { /* do nothing */ }
  void details_reply(Context, header const &, ci::profile_configuration::details_reply const &) override { /* do nothing */ }
  void on(Context, header const &, ci::profile_configuration::on const &) override { /* do nothing */ }
  void off(Context, header const &, ci::profile_configuration::off const &) override { /* do nothing */ }
  void enabled(Context, header const &, ci::profile_configuration::enabled const &) override { /* do nothing */ }
  void disabled(Context, header const &, ci::profile_configuration::disabled const &) override { /* do nothing */ }
  void specific_data(Context, header const &, ci::profile_configuration::specific_data const &) override { /* do nothing */ }
};
template <typename Context> struct property_exchange_base : property_exchange_pure<Context> {
  void capabilities(Context, header const &, ci::property_exchange::capabilities const &) override { /* do nothing */ }
  void capabilities_reply(Context, header const &, ci::property_exchange::capabilities_reply const &) override { /* do nothing */ }

  void get(Context, header const &, ci::property_exchange::get const &) override { /* do nothing */ }
  void get_reply(Context, header const &, ci::property_exchange::get_reply const &) override { /* do nothing */ }
  void set(Context, header const &, ci::property_exchange::set const &) override { /* do nothing */ }
  void set_reply(Context, header const &, ci::property_exchange::set_reply const &) override { /* do nothing */ }

  void subscription(Context, header const &, ci::property_exchange::subscription const &) override { /* do nothing */ }
  void subscription_reply(Context, header const &, ci::property_exchange::subscription_reply const &) override { /* do nothing */ }
  void notify(Context, header const &, ci::property_exchange::notify const &) override { /* do nothing */ }
};
template <typename Context> struct process_inquiry_base : process_inquiry_pure<Context> {
  void capabilities(Context, header const &) override { /* do nothing */ }
  void capabilities_reply(Context, header const &, ci::process_inquiry::capabilities_reply const &) override { /* do nothing */ }
  void midi_message_report(Context, header const &, ci::process_inquiry::midi_message_report const &) override { /* do nothing */ }
  void midi_message_report_reply(Context, header const &, ci::process_inquiry::midi_message_report_reply const &) override { /* do nothing */ }
  void midi_message_report_end(Context, header const &) override { /* do nothing */ }
};
// clang-format on

template <typename Context> class system_function {
public:
  using check_muid_fn = std::function<bool(Context, std::uint8_t /*group*/, muid)>;
  using unknown_fn = std::function<void(Context, header const &)>;
  using buffer_overflow_fn = std::function<void(Context)>;

  /// Sets the function that is to be called when the library needs to check whether the message is addressed to
  /// this receiver.
  ///
  /// \param check_muid  The function to be called to check whether a message should be handled.
  /// \returns *this
  constexpr system_function &on_check_muid(check_muid_fn check_muid) {
    check_muid_ = std::move(check_muid);
    return *this;
  }
  /// Sets the function that will be called when the library receives an unrecognized message.
  ///
  /// \param unknown The function to be called when an unrecognized message is received.
  /// \returns *this
  constexpr system_function &on_unknown(unknown_fn unknown) {
    unknown_ = std::move(unknown);
    return *this;
  }
  constexpr system_function &on_buffer_overflow(buffer_overflow_fn overflow) {
    overflow_ = std::move(overflow);
    return *this;
  }

  /// Dispatches a call to system::check_muid to the function that was installed by an earlier call to on_check_muid().
  bool check_muid(Context context, std::uint8_t group, muid m) const {
    return check_muid_ ? check_muid_(std::move(context), group, m) : false;
  }
  /// Dispatches a call to system::unknown to the function that was installed previously by a call to on_unknown().
  void unknown_midici(Context context, header const &ci) const { call(unknown_, std::move(context), ci); }
  void buffer_overflow(Context context) const { call(overflow_, std::move(context)); }

private:
  check_muid_fn check_muid_;
  unknown_fn unknown_;
  buffer_overflow_fn overflow_;
};

template <typename Context> class management_function {
public:
  using discovery_fn = std::function<void(Context, header const &, ci::discovery const &)>;
  using discovery_reply_fn = std::function<void(Context, header const &, ci::discovery_reply const &)>;
  using endpoint_fn = std::function<void(Context, header const &, ci::endpoint const &)>;
  using endpoint_reply_fn = std::function<void(Context, header const &, ci::endpoint_reply const &)>;
  using invalidate_muid_fn = std::function<void(Context, header const &, ci::invalidate_muid const &)>;
  using ack_fn = std::function<void(Context, header const &, ci::ack const &)>;
  using nak_fn = std::function<void(Context, header const &, ci::nak const &)>;

  constexpr management_function &on_discovery(discovery_fn discovery) {
    discovery_ = std::move(discovery);
    return *this;
  }
  constexpr management_function &on_discovery_reply(discovery_reply_fn discovery_reply) {
    discovery_reply_ = std::move(discovery_reply);
    return *this;
  }
  constexpr management_function &on_endpoint(endpoint_fn endpoint) {
    endpoint_ = std::move(endpoint);
    return *this;
  }
  constexpr management_function &on_endpoint_reply(endpoint_reply_fn endpoint_reply) {
    endpoint_reply_ = std::move(endpoint_reply);
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

  void discovery(Context context, header const &ci, ci::discovery const &d) const {
    call(discovery_, std::move(context), ci, d);
  }
  void discovery_reply(Context context, header const &ci, ci::discovery_reply const &dr) const {
    call(discovery_reply_, std::move(context), ci, dr);
  }
  void endpoint(Context context, header const &ci, ci::endpoint const &epi) const {
    call(endpoint_, std::move(context), ci, epi);
  }
  void endpoint_reply(Context context, header const &ci, ci::endpoint_reply const &reply) const {
    call(endpoint_reply_, std::move(context), ci, reply);
  }
  void invalidate_muid(Context context, header const &ci, ci::invalidate_muid const &im) const {
    call(invalidate_muid_, std::move(context), ci, im);
  }
  void ack(Context context, header const &ci, ci::ack const &a) const { call(ack_, std::move(context), ci, a); }
  void nak(Context context, header const &ci, ci::nak const &n) const { call(nak_, std::move(context), ci, n); }

private:
  discovery_fn discovery_;
  discovery_reply_fn discovery_reply_;
  endpoint_fn endpoint_;
  endpoint_reply_fn endpoint_reply_;
  invalidate_muid_fn invalidate_muid_;
  ack_fn ack_;
  nak_fn nak_;
};

static_assert(management<management_function<int>, int>);

template <typename Context> class profile_function {
public:
  using inquiry_fn = std::function<void(Context, header const &)>;
  using inquiry_reply_fn =
      std::function<void(Context, header const &, ci::profile_configuration::inquiry_reply const &)>;
  using added_fn = std::function<void(Context, header const &, ci::profile_configuration::added const &)>;
  using removed_fn = std::function<void(Context, header const &, ci::profile_configuration::removed const &)>;
  using details_fn = std::function<void(Context, header const &, ci::profile_configuration::details const &)>;
  using details_reply_fn =
      std::function<void(Context, header const &, ci::profile_configuration::details_reply const &)>;
  using on_fn = std::function<void(Context, header const &, ci::profile_configuration::on const &)>;
  using off_fn = std::function<void(Context, header const &, ci::profile_configuration::off const &)>;
  using enabled_fn = std::function<void(Context, header const &, ci::profile_configuration::enabled const &)>;
  using disabled_fn = std::function<void(Context, header const &, ci::profile_configuration::disabled const &)>;
  using specific_data_fn =
      std::function<void(Context, header const &, ci::profile_configuration::specific_data const &)>;

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

  void inquiry(Context context, header const &ci) const { call(inquiry_, context, ci); }
  void inquiry_reply(Context context, header const &ci, ci::profile_configuration::inquiry_reply const &reply) const {
    call(inquiry_reply_, context, ci, reply);
  }
  void added(Context context, header const &ci, ci::profile_configuration::added const &a) const {
    call(added_, context, ci, a);
  }
  void removed(Context context, header const &ci, ci::profile_configuration::removed const &r) const {
    call(removed_, context, ci, r);
  }
  void details(Context context, header const &ci, ci::profile_configuration::details const &d) const {
    call(details_, context, ci, d);
  }
  void details_reply(Context context, header const &ci, ci::profile_configuration::details_reply const &reply) const {
    call(details_reply_, context, ci, reply);
  }
  void on(Context context, header const &ci, ci::profile_configuration::on const &on) const {
    call(on_, context, ci, on);
  }
  void off(Context context, header const &ci, ci::profile_configuration::off const &off) const {
    call(off_, context, ci, off);
  }
  void enabled(Context context, header const &ci, ci::profile_configuration::enabled const &enabled) const {
    call(enabled_, context, ci, enabled);
  }
  void disabled(Context context, header const &ci, ci::profile_configuration::disabled const &disabled) const {
    call(disabled_, context, ci, disabled);
  }
  void specific_data(Context context, header const &ci, ci::profile_configuration::specific_data const &sd) const {
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
  using capabilities_fn = std::function<void(Context, header const &, ci::property_exchange::capabilities const &)>;
  using capabilities_reply_fn =
      std::function<void(Context, header const &, ci::property_exchange::capabilities_reply const &)>;
  using get_fn = std::function<void(Context, header const &, ci::property_exchange::get const &)>;
  using get_reply_fn = std::function<void(Context, header const &, ci::property_exchange::get_reply const &)>;
  using set_fn = std::function<void(Context, header const &, ci::property_exchange::set const &)>;
  using set_reply_fn = std::function<void(Context, header const &, ci::property_exchange::set_reply const &)>;
  using subscription_fn = std::function<void(Context, header const &, ci::property_exchange::subscription const &)>;
  using subscription_reply_fn =
      std::function<void(Context, header const &, ci::property_exchange::subscription_reply const &)>;
  using notify_fn = std::function<void(Context, header const &, ci::property_exchange::notify const &)>;

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

  void capabilities(Context context, header const &ci, ci::property_exchange::capabilities const &cap) const {
    call(capabilities_, context, ci, cap);
  }
  void capabilities_reply(Context context, header const &ci,
                          ci::property_exchange::capabilities_reply const &reply) const {
    call(capabilities_reply_, context, ci, reply);
  }
  void get(Context context, header const &ci, ci::property_exchange::get const &get) const {
    call(get_, context, ci, get);
  }
  void get_reply(Context context, header const &ci, ci::property_exchange::get_reply const &reply) const {
    call(get_reply_, context, ci, reply);
  }
  void set(Context context, header const &ci, ci::property_exchange::set const &set) const {
    call(set_, context, ci, set);
  }
  void set_reply(Context context, header const &ci, ci::property_exchange::set_reply const &reply) const {
    call(set_reply_, context, ci, reply);
  }
  void subscription(Context context, header const &ci, ci::property_exchange::subscription const &sub) const {
    call(subscription_, context, ci, sub);
  }
  void subscription_reply(Context context, header const &ci,
                          ci::property_exchange::subscription_reply const &reply) const {
    call(subscription_reply_, context, ci, reply);
  }
  void notify(Context context, header const &ci, ci::property_exchange::notify const &notify) const {
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
  using capabilities_fn = std::function<void(Context, header const &)>;
  using capabilities_reply_fn =
      std::function<void(Context, header const &, ci::process_inquiry::capabilities_reply const &)>;
  using midi_message_report_fn =
      std::function<void(Context, header const &, ci::process_inquiry::midi_message_report const &)>;
  using midi_message_report_reply_fn =
      std::function<void(Context, header const &, ci::process_inquiry::midi_message_report_reply const &)>;
  using midi_message_report_end_fn = std::function<void(Context, header const &)>;

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

  void capabilities(Context context, header const &ci) const { call(capabilities_, context, ci); }
  void capabilities_reply(Context context, header const &ci,
                          ci::process_inquiry::capabilities_reply const &reply) const {
    call(capabilities_reply_, context, ci, reply);
  }
  void midi_message_report(Context context, header const &ci,
                           ci::process_inquiry::midi_message_report const &mmr) const {
    call(midi_message_report_, context, ci, mmr);
  }
  void midi_message_report_reply(Context context, header const &ci,
                                 ci::process_inquiry::midi_message_report_reply const &reply) const {
    call(midi_message_report_reply_, context, ci, reply);
  }
  void midi_message_report_end(Context context, header const &ci) const { call(midi_message_report_end_, context, ci); }

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
