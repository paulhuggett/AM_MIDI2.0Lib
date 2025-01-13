//===-- CI Dispatcher Backends ------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//
#include "midi2/ci_dispatcher_backend.hpp"

// Standard library
#include <array>
#include <span>

// Google Test/Mock
#include <gmock/gmock.h>

namespace {

struct context_type {
  constexpr bool operator==(context_type const &) const noexcept = default;
  int value = 23;
};

using testing::MockFunction;
using testing::StrictMock;

class CIDispatcherBackendManagement : public testing::Test {
protected:
  context_type context_;
  midi2::ci::dispatcher_backend::management_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendManagement, Discovery) {
  StrictMock<MockFunction<decltype(be_)::discovery_fn>> fn;
  midi2::ci::midi_ci const mci{};
  midi2::ci::discovery const d{};
  // The first call should do nothing since no handler has been installed.
  be_.discovery(context_, mci, d);
  // Install a handler for the discovery message.
  be_.on_discovery(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  EXPECT_CALL(fn, Call(context_, mci, d)).Times(1);
  be_.discovery(context_, mci, d);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendManagement, DiscoveryReply) {
  StrictMock<MockFunction<decltype(be_)::discovery_reply_fn>> fn;
  midi2::ci::midi_ci const mci{};
  midi2::ci::discovery_reply const d{};
  be_.discovery_reply(context_, mci, d);
  be_.on_discovery_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, d)).Times(1);
  be_.discovery_reply(context_, mci, d);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendManagement, EndpointInfo) {
  StrictMock<MockFunction<decltype(be_)::endpoint_info_fn>> fn;
  midi2::ci::midi_ci const mci{};
  midi2::ci::endpoint_info const ei{};
  be_.endpoint_info(context_, mci, ei);
  be_.on_endpoint_info(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, ei)).Times(1);
  be_.endpoint_info(context_, mci, ei);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendManagement, EndpointInfoReply) {
  StrictMock<MockFunction<decltype(be_)::endpoint_info_reply_fn>> fn;
  midi2::ci::midi_ci const mci{};
  midi2::ci::endpoint_info_reply const ei{};
  be_.endpoint_info_reply(context_, mci, ei);
  be_.on_endpoint_info_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, ei)).Times(1);
  be_.endpoint_info_reply(context_, mci, ei);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendManagement, InvalidateMUID) {
  StrictMock<MockFunction<decltype(be_)::invalidate_muid_fn>> fn;
  midi2::ci::midi_ci const mci{};
  midi2::ci::invalidate_muid const im{};
  be_.invalidate_muid(context_, mci, im);
  be_.on_invalidate_muid(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, im)).Times(1);
  be_.invalidate_muid(context_, mci, im);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendManagement, Ack) {
  StrictMock<MockFunction<decltype(be_)::ack_fn>> fn;
  midi2::ci::midi_ci const mdi{};
  midi2::ci::ack const a{};
  be_.ack(context_, mdi, a);
  be_.on_ack(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mdi, a)).Times(1);
  be_.ack(context_, mdi, a);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendManagement, Nak) {
  StrictMock<MockFunction<decltype(be_)::nak_fn>> fn;
  midi2::ci::midi_ci const mdi{};
  midi2::ci::nak const n{};
  be_.nak(context_, mdi, n);
  be_.on_nak(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mdi, n)).Times(1);
  be_.nak(context_, mdi, n);
}

class CIDispatcherBackendProfile : public testing::Test {
protected:
  context_type context_;
  midi2::ci::dispatcher_backend::profile_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, Inquiry) {
  StrictMock<MockFunction<decltype(be_)::inquiry_fn>> fn;
  midi2::ci::midi_ci const mci{};
  // The first call should do nothing since no handler has been installed.
  be_.inquiry(context_, mci);
  // Install a handler for the inquiry message.
  be_.on_inquiry(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  EXPECT_CALL(fn, Call(context_, mci)).Times(1);
  be_.inquiry(context_, mci);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, InquiryReply) {
  StrictMock<MockFunction<decltype(be_)::inquiry_reply_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::inquiry_reply const reply;
  be_.inquiry_reply(context_, mci, reply);
  be_.on_inquiry_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, reply)).Times(1);
  be_.inquiry_reply(context_, mci, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, Added) {
  StrictMock<MockFunction<decltype(be_)::added_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::added const a;
  be_.added(context_, mci, a);
  be_.on_added(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, a)).Times(1);
  be_.added(context_, mci, a);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, Removed) {
  StrictMock<MockFunction<decltype(be_)::removed_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::removed const a;
  be_.removed(context_, mci, a);
  be_.on_removed(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, a)).Times(1);
  be_.removed(context_, mci, a);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, Details) {
  StrictMock<MockFunction<decltype(be_)::details_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::details const a;
  be_.details(context_, mci, a);
  be_.on_details(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, a)).Times(1);
  be_.details(context_, mci, a);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, DetailsReply) {
  StrictMock<MockFunction<decltype(be_)::details_reply_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::details_reply const a;
  be_.details_reply(context_, mci, a);
  be_.on_details_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, a)).Times(1);
  be_.details_reply(context_, mci, a);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, On) {
  StrictMock<MockFunction<decltype(be_)::on_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::on const a;
  be_.on(context_, mci, a);
  be_.on_on(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, a)).Times(1);
  be_.on(context_, mci, a);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, Off) {
  StrictMock<MockFunction<decltype(be_)::off_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::off const a;
  be_.off(context_, mci, a);
  be_.on_off(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, a)).Times(1);
  be_.off(context_, mci, a);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, Enabled) {
  StrictMock<MockFunction<decltype(be_)::enabled_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::enabled const a;
  be_.enabled(context_, mci, a);
  be_.on_enabled(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, a)).Times(1);
  be_.enabled(context_, mci, a);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, Disabled) {
  StrictMock<MockFunction<decltype(be_)::disabled_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::disabled const a;
  be_.disabled(context_, mci, a);
  be_.on_disabled(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, a)).Times(1);
  be_.disabled(context_, mci, a);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProfile, SpecficData) {
  StrictMock<MockFunction<decltype(be_)::specific_data_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::profile_configuration::specific_data const a;
  be_.specific_data(context_, mci, a);
  be_.on_specific_data(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, a)).Times(1);
  be_.specific_data(context_, mci, a);
}

class CIDispatcherBackendPropertyExchange : public testing::Test {
protected:
  context_type context_;
  midi2::ci::dispatcher_backend::property_exchange_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendPropertyExchange, Capabilities) {
  StrictMock<MockFunction<decltype(be_)::capabilities_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::property_exchange::capabilities const cap;
  // The first call should do nothing since no handler has been installed.
  be_.capabilities(context_, mci, cap);
  // Install a handler for the capabilities message.
  be_.on_capabilities(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  EXPECT_CALL(fn, Call(context_, mci, cap)).Times(1);
  be_.capabilities(context_, mci, cap);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendPropertyExchange, CapabilitiesReply) {
  StrictMock<MockFunction<decltype(be_)::capabilities_reply_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::property_exchange::capabilities_reply const cap;
  be_.capabilities_reply(context_, mci, cap);
  be_.on_capabilities_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, cap)).Times(1);
  be_.capabilities_reply(context_, mci, cap);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendPropertyExchange, Get) {
  StrictMock<MockFunction<decltype(be_)::get_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::property_exchange::get const g;
  be_.get(context_, mci, g);
  be_.on_get(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, g)).Times(1);
  be_.get(context_, mci, g);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendPropertyExchange, GetReply) {
  StrictMock<MockFunction<decltype(be_)::get_reply_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::property_exchange::get_reply const g;
  be_.get_reply(context_, mci, g);
  be_.on_get_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, g)).Times(1);
  be_.get_reply(context_, mci, g);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendPropertyExchange, Set) {
  StrictMock<MockFunction<decltype(be_)::set_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::property_exchange::set const s;
  be_.set(context_, mci, s);
  be_.on_set(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, s)).Times(1);
  be_.set(context_, mci, s);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendPropertyExchange, SetReply) {
  StrictMock<MockFunction<decltype(be_)::set_reply_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::property_exchange::set_reply const sr;
  be_.set_reply(context_, mci, sr);
  be_.on_set_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, sr)).Times(1);
  be_.set_reply(context_, mci, sr);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendPropertyExchange, Subscription) {
  StrictMock<MockFunction<decltype(be_)::subscription_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::property_exchange::subscription const sub;
  be_.subscription(context_, mci, sub);
  be_.on_subscription(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, sub)).Times(1);
  be_.subscription(context_, mci, sub);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendPropertyExchange, SubscriptionReply) {
  StrictMock<MockFunction<decltype(be_)::subscription_reply_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::property_exchange::subscription_reply const sub;
  be_.subscription_reply(context_, mci, sub);
  be_.on_subscription_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, sub)).Times(1);
  be_.subscription_reply(context_, mci, sub);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendPropertyExchange, Notify) {
  StrictMock<MockFunction<decltype(be_)::notify_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::property_exchange::notify const notify;
  be_.notify(context_, mci, notify);
  be_.on_notify(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, notify)).Times(1);
  be_.notify(context_, mci, notify);
}

class CIDispatcherBackendProcessInquiry : public testing::Test {
protected:
  context_type context_;
  midi2::ci::dispatcher_backend::process_inquiry_function<context_type> be_;
};
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProcessInquiry, Capabilities) {
  StrictMock<MockFunction<decltype(be_)::capabilities_fn>> fn;
  midi2::ci::midi_ci const mci;
  // The first call should do nothing since no handler has been installed.
  be_.capabilities(context_, mci);
  // Install a handler for the capabilities message.
  be_.on_capabilities(fn.AsStdFunction());
  // Expect that our handler is called with the correct arguments.
  EXPECT_CALL(fn, Call(context_, mci)).Times(1);
  be_.capabilities(context_, mci);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProcessInquiry, CapabilitiesReply) {
  StrictMock<MockFunction<decltype(be_)::capabilities_reply_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::process_inquiry::capabilities_reply const cap;
  be_.capabilities_reply(context_, mci, cap);
  be_.on_capabilities_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, cap)).Times(1);
  be_.capabilities_reply(context_, mci, cap);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProcessInquiry, MidiMessageReport) {
  StrictMock<MockFunction<decltype(be_)::midi_message_report_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::process_inquiry::midi_message_report const mmr;
  be_.midi_message_report(context_, mci, mmr);
  be_.on_midi_message_report(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, mmr)).Times(1);
  be_.midi_message_report(context_, mci, mmr);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProcessInquiry, MidiMessageReportReply) {
  StrictMock<MockFunction<decltype(be_)::midi_message_report_reply_fn>> fn;
  midi2::ci::midi_ci const mci;
  midi2::ci::process_inquiry::midi_message_report_reply const mmr;
  be_.midi_message_report_reply(context_, mci, mmr);
  be_.on_midi_message_report_reply(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci, mmr)).Times(1);
  be_.midi_message_report_reply(context_, mci, mmr);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherBackendProcessInquiry, MidiMessageReportEnd) {
  StrictMock<MockFunction<decltype(be_)::midi_message_report_end_fn>> fn;
  midi2::ci::midi_ci const mci;
  be_.midi_message_report_end(context_, mci);
  be_.on_midi_message_report_end(fn.AsStdFunction());
  EXPECT_CALL(fn, Call(context_, mci)).Times(1);
  be_.midi_message_report_end(context_, mci);
}

}  // end anonymous namespace
