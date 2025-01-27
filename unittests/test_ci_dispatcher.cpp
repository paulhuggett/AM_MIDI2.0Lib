//===-- CI Dispatcher ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ci_dispatcher.hpp"

// MIDI2
#include "midi2/ci_create_message.hpp"
#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

// Standard library
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <ostream>
#include <ranges>
#include <span>

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace {

template <typename T> struct trivial_sentinel {
  constexpr bool operator==(T) const { return false; }
  friend constexpr bool operator==(T, trivial_sentinel) { return false; }
};

std::ostream &write_bytes(std::ostream &os, std::span<std::uint8_t const> const &arr) {
  os << '[';
  auto const *separator = "";
  for (auto m : arr) {
    os << separator << static_cast<unsigned>(m);
    separator = ",";
  }
  os << ']';
  return os;
}

std::ostream &write_bytes(std::ostream &os, std::span<std::byte const> const &b) {
  os << '[';
  auto const *separator = "";
  for (auto m : b) {
    os << separator << static_cast<unsigned>(m);
    separator = ",";
  }
  os << ']';
  return os;
}

}  // end anonymous namespace

namespace midi2::ci {

std::ostream &operator<<(std::ostream &os, midi_ci const &ci);
std::ostream &operator<<(std::ostream &os, midi_ci const &ci) {
  return os << "{ group=" << static_cast<unsigned>(ci.group)
            << ", device_id=" << static_cast<unsigned>(ci.params.device_id)
            << ", type=" << static_cast<unsigned>(ci.type) << ", version=" << static_cast<unsigned>(ci.params.version)
            << ", remote_muid=" << ci.params.remote_muid << ", local_muid=" << ci.params.local_muid << " }";
}

std::ostream &operator<<(std::ostream &os, property_exchange::property_exchange::chunk_info const &ci);
std::ostream &operator<<(std::ostream &os, property_exchange::property_exchange::chunk_info const &ci) {
  return os << "{ number_of_chunks=" << static_cast<unsigned>(ci.number_of_chunks)
            << ", chunk_number=" << static_cast<unsigned>(ci.chunk_number) << " }";
}

std::ostream &operator<<(std::ostream &os, ci::discovery const &d);
std::ostream &operator<<(std::ostream &os, ci::discovery const &d) {
  os << "{ manufacturer=";
  write_bytes(os, std::span{d.manufacturer});
  os << ", family=" << d.family << ", model=" << d.model << ", version=";
  write_bytes(os, d.version);
  os << ", capability=" << static_cast<unsigned>(d.capability) << ", max_sysex_size=" << d.max_sysex_size << " }";
  return os;
}

std::ostream &operator<<(std::ostream &os, ci::discovery_reply const &d);
std::ostream &operator<<(std::ostream &os, ci::discovery_reply const &d) {
  os << "{ manufacturer=";
  write_bytes(os, d.manufacturer);
  os << ", family=" << d.family << ", model=" << d.model << ", version=";
  write_bytes(os, d.version);
  os << ", capability=" << static_cast<unsigned>(d.capability) << ", max_sysex_size=" << d.max_sysex_size << " }";
  return os;
}

std::ostream &operator<<(std::ostream &os, ci::nak const &nak);
std::ostream &operator<<(std::ostream &os, ci::nak const &nak) {
  os << "{ original_id=" << static_cast<unsigned>(nak.original_id)
     << ", status_code=" << static_cast<unsigned>(nak.status_code)
     << ", status_data=" << static_cast<unsigned>(nak.status_data) << ", details=";
  write_bytes(os, nak.details);
  os << ", message=";
  write_bytes(os, nak.message);
  os << " }";
  return os;
}

std::ostream &operator<<(std::ostream &os, ci::profile_configuration::disabled const &pds);
std::ostream &operator<<(std::ostream &os, ci::profile_configuration::disabled const &pd) {
  os << "{ pid=";
  write_bytes(os, pd.pid);
  os << ", num_channels=" << pd.num_channels << " }";
  return os;
}

}  // end namespace midi2::ci

namespace {

using testing::AllOf;
using testing::ElementsAreArray;
using testing::Eq;
using testing::Field;
using testing::IsEmpty;
using testing::Return;
using testing::StrictMock;

using midi2::ci::byte_array_5;
using midi2::ci::from_le7;
using midi2::ci::midi_ci;

struct context_type {
  constexpr bool operator==(context_type const &) const noexcept = default;
};

class mock_management_callbacks : public midi2::ci::dispatcher_backend::management_pure<context_type> {
public:
  MOCK_METHOD(bool, check_muid, (context_type, std::uint8_t group, std::uint32_t muid), (override));
  MOCK_METHOD(void, buffer_overflow, (context_type), (override));
  MOCK_METHOD(void, unknown_midici, (context_type, midi_ci const &), (override));

  MOCK_METHOD(void, discovery, (context_type, midi_ci const &, midi2::ci::discovery const &), (override));
  MOCK_METHOD(void, discovery_reply, (context_type, midi_ci const &, midi2::ci::discovery_reply const &), (override));
  MOCK_METHOD(void, endpoint_info, (context_type, midi_ci const &, midi2::ci::endpoint_info const &), (override));
  MOCK_METHOD(void, endpoint_info_reply, (context_type, midi_ci const &, midi2::ci::endpoint_info_reply const &),
              (override));
  MOCK_METHOD(void, invalidate_muid, (context_type, midi_ci const &, midi2::ci::invalidate_muid const &), (override));
  MOCK_METHOD(void, ack, (context_type, midi_ci const &, midi2::ci::ack const &), (override));
  MOCK_METHOD(void, nak, (context_type, midi_ci const &, midi2::ci::nak const &), (override));
};

class mock_profile_callbacks : public midi2::ci::dispatcher_backend::profile_pure<context_type> {
public:
  MOCK_METHOD(void, inquiry, (context_type, midi_ci const &), (override));
  MOCK_METHOD(void, inquiry_reply,
              (context_type, midi_ci const &, midi2::ci::profile_configuration::inquiry_reply const &), (override));
  MOCK_METHOD(void, added, (context_type, midi_ci const &, midi2::ci::profile_configuration::added const &),
              (override));
  MOCK_METHOD(void, removed, (context_type, midi_ci const &, midi2::ci::profile_configuration::removed const &),
              (override));
  MOCK_METHOD(void, details, (context_type, midi_ci const &, midi2::ci::profile_configuration::details const &),
              (override));
  MOCK_METHOD(void, details_reply,
              (context_type, midi_ci const &, midi2::ci::profile_configuration::details_reply const &), (override));
  MOCK_METHOD(void, on, (context_type, midi_ci const &, midi2::ci::profile_configuration::on const &), (override));
  MOCK_METHOD(void, off, (context_type, midi_ci const &, midi2::ci::profile_configuration::off const &), (override));
  MOCK_METHOD(void, enabled, (context_type, midi_ci const &, midi2::ci::profile_configuration::enabled const &),
              (override));
  MOCK_METHOD(void, disabled, (context_type, midi_ci const &, midi2::ci::profile_configuration::disabled const &),
              (override));
  MOCK_METHOD(void, specific_data,
              (context_type, midi_ci const &, midi2::ci::profile_configuration::specific_data const &), (override));
};

class mock_property_exchange_callbacks : public midi2::ci::dispatcher_backend::property_exchange_pure<context_type> {
public:
  MOCK_METHOD(void, capabilities, (context_type, midi_ci const &, midi2::ci::property_exchange::capabilities const &),
              (override));
  MOCK_METHOD(void, capabilities_reply,
              (context_type, midi_ci const &, midi2::ci::property_exchange::capabilities_reply const &), (override));

  MOCK_METHOD(void, get, (context_type, midi_ci const &, midi2::ci::property_exchange::get const &), (override));
  MOCK_METHOD(void, get_reply, (context_type, midi_ci const &, midi2::ci::property_exchange::get_reply const &),
              (override));
  MOCK_METHOD(void, set, (context_type, midi_ci const &, midi2::ci::property_exchange::set const &), (override));
  MOCK_METHOD(void, set_reply, (context_type, midi_ci const &, midi2::ci::property_exchange::set_reply const &),
              (override));
  MOCK_METHOD(void, subscription, (context_type, midi_ci const &, midi2::ci::property_exchange::subscription const &),
              (override));
  MOCK_METHOD(void, subscription_reply,
              (context_type, midi_ci const &, midi2::ci::property_exchange::subscription_reply const &), (override));
  MOCK_METHOD(void, notify, (context_type, midi_ci const &, midi2::ci::property_exchange::notify const &), (override));
};

class mock_process_inquiry_callbacks : public midi2::ci::dispatcher_backend::process_inquiry_pure<context_type> {
public:
  MOCK_METHOD(void, capabilities, (context_type, midi_ci const &), (override));
  MOCK_METHOD(void, capabilities_reply,
              (context_type, midi_ci const &, midi2::ci::process_inquiry::capabilities_reply const &), (override));
  MOCK_METHOD(void, midi_message_report,
              (context_type, midi_ci const &, midi2::ci::process_inquiry::midi_message_report const &), (override));
  MOCK_METHOD(void, midi_message_report_reply,
              (context_type, midi_ci const &, midi2::ci::process_inquiry::midi_message_report_reply const &),
              (override));
  MOCK_METHOD(void, midi_message_report_end, (context_type, midi_ci const &), (override));
};

constexpr auto broadcast_muid = std::array{std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}};

class CIDispatcher : public testing::Test {
public:
  CIDispatcher() : processor_{std::ref(config_)} {}

protected:
  struct mocked_config {
    [[no_unique_address]] context_type context;
    StrictMock<mock_management_callbacks> management;
    StrictMock<mock_profile_callbacks> profile;
    StrictMock<mock_property_exchange_callbacks> property_exchange;
    StrictMock<mock_process_inquiry_callbacks> process_inquiry;
  };
  mocked_config config_;
  midi2::ci::ci_dispatcher<mocked_config &> processor_;

  static constexpr auto sender_muid_ =
      from_le7(std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}});
  static constexpr auto destination_muid_ =
      from_le7(std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}});

  template <typename Content>
  static std::vector<std::byte> make_message(midi2::ci::params const &params, Content const &content) {
    std::vector<std::byte> message;
    auto out_it = std::back_inserter(message);
    midi2::ci::create_message(out_it, trivial_sentinel<decltype(out_it)>{}, params, content);
    message.push_back(std::byte{0});  // a stray extra byte
    return message;
  }

  template <typename Content> void dispatch_ci(midi_ci const &midici, Content const &content) {
    processor_.start(midici.group, static_cast<std::byte>(midici.params.device_id));
    std::ranges::for_each(make_message(midici.params, content),
                          [this](std::byte const b) { processor_.processMIDICI(b); });
    processor_.finish();
  }
};
// NOLINTNEXTLINE
TEST_F(CIDispatcher, Empty) {
  processor_.processMIDICI(std::byte{0});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryV1) {
  midi_ci midici;
  midici.group = std::uint8_t{0xFF};
  midici.type = midi2::ci_message::discovery;
  midici.params.device_id = std::uint8_t{0x7F};
  midici.params.version = 1;
  midici.params.remote_muid = 0;
  midici.params.local_muid = midi2::ci_broadcast;

  midi2::ci::discovery discovery;
  discovery.manufacturer = std::array{std::uint8_t{0x12}, std::uint8_t{0x23}, std::uint8_t{0x34}};
  discovery.family = (1U << (7 * 2)) - 1U;
  discovery.model = (1U << (7 * 2)) - 2U;
  discovery.version = std::array{std::uint8_t{0x7F}, std::uint8_t{0x3C}, std::uint8_t{0x2A}, std::uint8_t{0x18}};
  discovery.capability = std::uint8_t{0x7F};
  discovery.max_sysex_size = (1 << (7 * 4)) - 1;

  EXPECT_CALL(config_.management, discovery(config_.context, midici, discovery)).Times(1);
  this->dispatch_ci(midici, discovery);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryV2) {
  midi_ci midici;
  midici.group = std::uint8_t{0xFF};
  midici.type = midi2::ci_message::discovery;
  midici.params.device_id = std::uint8_t{0x7F};
  midici.params.version = 2;
  midici.params.remote_muid = 0;
  midici.params.local_muid = midi2::ci_broadcast;

  midi2::ci::discovery discovery;
  discovery.manufacturer = std::array{std::uint8_t{0x12}, std::uint8_t{0x23}, std::uint8_t{0x34}};
  discovery.family = from_le7(std::array{std::byte{0x67}, std::byte{0x79}});
  discovery.model = from_le7(std::array{std::byte{0x6B}, std::byte{0x5D}});
  discovery.version =
      midi2::ci::from_array(std::array{std::byte{0x4E}, std::byte{0x3C}, std::byte{0x2A}, std::byte{0x18}});
  discovery.capability = std::uint8_t{0x7F};
  discovery.max_sysex_size = from_le7(std::array{std::byte{0x76}, std::byte{0x54}, std::byte{0x32}, std::byte{0x10}});
  discovery.output_path_id = std::uint8_t{0x71};

  EXPECT_CALL(config_.management, discovery(config_.context, midici, discovery)).Times(1);
  this->dispatch_ci(midici, discovery);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryReplyV2) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto manufacturer = std::array{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}};
  constexpr auto family = std::array{std::byte{0x67}, std::byte{0x79}};
  constexpr auto model = std::array{std::byte{0x5B}, std::byte{0x4D}};
  constexpr auto version = std::array{std::byte{0x7E}, std::byte{0x6C}, std::byte{0x5A}, std::byte{0x48}};
  constexpr auto capability = std::byte{0x7F};
  constexpr auto max_sysex_size = std::array{std::byte{0x76}, std::byte{0x54}, std::byte{0x32}, std::byte{0x10}};
  constexpr auto output_path_id = std::byte{0x71};
  constexpr auto function_block = std::byte{0x32};

  midi_ci midici;
  midici.group = 0xFF;
  midici.type = midi2::ci_message::discovery_reply;
  midici.params.device_id = midi2::to_underlying(device_id);
  midici.params.version = 2;
  midici.params.remote_muid = 0;
  midici.params.local_muid = midi2::ci_broadcast;

  midi2::ci::discovery_reply reply;
  reply.manufacturer = midi2::ci::from_array(manufacturer);
  reply.family = from_le7(family);
  reply.model = from_le7(model);
  reply.version = midi2::ci::from_array(version);
  reply.capability = midi2::to_underlying(capability);
  reply.max_sysex_size = from_le7(max_sysex_size);
  reply.output_path_id = midi2::to_underlying(output_path_id);
  reply.function_block = midi2::to_underlying(function_block);

  EXPECT_CALL(config_.management, discovery_reply(config_.context, midici, reply)).Times(1);
  this->dispatch_ci(midici, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, EndpointInfo) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto status = std::uint8_t{0b0101010};
  constexpr std::array const receiver_muid{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::endpoint_info;
  midici.params.device_id = midi2::to_underlying(device_id);
  midici.params.version = 1;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = from_le7(receiver_muid);

  midi2::ci::endpoint_info endpoint_info;
  endpoint_info.status = status;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, from_le7(receiver_muid)))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, endpoint_info(config_.context, midici, endpoint_info)).Times(1);

  this->dispatch_ci(midici, endpoint_info);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, EndpointInfoReply) {
  constexpr auto group = std::uint8_t{0x71};
  constexpr auto receiver_muid =
      from_le7(std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}});
  constexpr auto status = std::byte{0b0101010};
  constexpr auto information = std::array{
      std::byte{2},  std::byte{3},  std::byte{5},  std::byte{7},
      std::byte{11}, std::byte{13}, std::byte{17}, std::byte{19},
  };

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::endpoint_info_reply;
  midici.params.device_id = std::uint8_t{0x7F};
  midici.params.version = 1;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = receiver_muid;

  midi2::ci::endpoint_info_reply reply;
  reply.status = status;
  reply.information = information;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management,
              endpoint_info_reply(config_.context, midici,
                                  AllOf(Field("status", &midi2::ci::endpoint_info_reply::status, Eq(status)),
                                        Field("information", &midi2::ci::endpoint_info_reply::information,
                                              ElementsAreArray(information)))))
      .Times(1);

  this->dispatch_ci(midici, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, InvalidateMuid) {
  constexpr auto group = std::uint8_t{0x71};
  constexpr auto device_id = std::byte{0x7F};
  constexpr std::array const receiver_muid{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};
  constexpr std::array const target_muid{std::byte{0x21}, std::byte{0x43}, std::byte{0x75}, std::byte{0x71}};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::invalidate_muid;
  midici.params.device_id = midi2::to_underlying(device_id);
  midici.params.version = 1;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = from_le7(receiver_muid);

  midi2::ci::invalidate_muid invalidate_muid;
  invalidate_muid.target_muid = from_le7(target_muid);

  EXPECT_CALL(config_.management, check_muid(config_.context, group, from_le7(receiver_muid)))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, invalidate_muid(config_.context, midici, invalidate_muid)).Times(1);

  this->dispatch_ci(midici, invalidate_muid);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, Ack) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto receiver_muid =
      from_le7(std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}});

  constexpr auto original_id = std::uint8_t{0x34};
  constexpr auto ack_status_code = std::uint8_t{0x00};
  constexpr auto ack_status_data = std::uint8_t{0x7F};
  constexpr auto ack_details =
      std::array{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
  constexpr auto text = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::ack;
  midici.params.device_id = std::uint8_t{0x7F};
  midici.params.version = 1;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = receiver_muid;

  midi2::ci::ack ack;
  ack.original_id = original_id;
  ack.status_code = ack_status_code;
  ack.status_data = ack_status_data;
  ack.details = ack_details;
  ack.message = text;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, ack(config_.context, midici,
                                      AllOf(Field("original_id", &midi2::ci::ack::original_id, Eq(original_id)),
                                            Field("status_code", &midi2::ci::ack::status_code, Eq(ack_status_code)),
                                            Field("status_data", &midi2::ci::ack::status_data, Eq(ack_status_data)),
                                            Field("details", &midi2::ci::ack::details, ElementsAreArray(ack_details)),
                                            Field("message", &midi2::ci::ack::message, ElementsAreArray(text)))))
      .Times(1);

  this->dispatch_ci(midici, ack);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, AckMessageTooLong) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto receiver_muid =
      from_le7(std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}});

  constexpr auto text_length = std::size_t{512};
  std::vector<std::byte> text;
  text.resize(text_length);

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::ack;
  midici.params.device_id = std::uint8_t{0x7F};
  midici.params.version = 1;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = receiver_muid;

  midi2::ci::ack ack;
  ack.original_id = std::uint8_t{0x34};
  ack.status_code = std::uint8_t{0x17};
  ack.status_data = std::uint8_t{0x7F};
  ack.details = std::array{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
  ack.message = text;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, buffer_overflow(config_.context)).Times(1);

  this->dispatch_ci(midici, ack);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, NakV1) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto receiver_muid =
      from_le7(std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}});

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::nak;
  midici.params.device_id = midi2::to_underlying(device_id);
  midici.params.version = 1;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = receiver_muid;

  midi2::ci::nak nak;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, nak(config_.context, midici,
                                      AllOf(Field("original_id", &midi2::ci::nak::original_id, Eq(0)),
                                            Field("status_code", &midi2::ci::nak::status_code, Eq(0)),
                                            Field("status_data", &midi2::ci::nak::status_data, Eq(0)),
                                            Field("details", &midi2::ci::nak::details, Eq(byte_array_5{})),
                                            Field("message", &midi2::ci::nak::message, IsEmpty()))))
      .Times(1);

  this->dispatch_ci(midici, nak);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, NakV2) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  constexpr auto original_id = std::uint8_t{0x34};
  constexpr auto nak_status_code = std::uint8_t{0x17};
  constexpr auto nak_status_data = std::uint8_t{0x7F};
  constexpr auto nak_details =
      std::array{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
  constexpr auto text = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::nak;
  midici.params.device_id = midi2::to_underlying(device_id);
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = from_le7(receiver_muid);

  midi2::ci::nak nak;
  nak.original_id = original_id;
  nak.status_code = nak_status_code;
  nak.status_data = nak_status_data;
  nak.details = nak_details;
  nak.message = text;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, from_le7(receiver_muid)))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, nak(config_.context, midici,
                                      AllOf(Field("original_id", &midi2::ci::nak::original_id, Eq(original_id)),
                                            Field("status_code", &midi2::ci::nak::status_code, Eq(nak_status_code)),
                                            Field("status_data", &midi2::ci::nak::status_data, Eq(nak_status_data)),
                                            Field("details", &midi2::ci::nak::details, ElementsAreArray(nak_details)),
                                            Field("message", &midi2::ci::nak::message, ElementsAreArray(text)))))
      .Times(1);

  this->dispatch_ci(midici, nak);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileInquiry) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto receiver_muid =
      from_le7(std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}});

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::profile_inquiry;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = receiver_muid;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, inquiry(config_.context, midici)).Times(1);

  this->dispatch_ci(midici, midi2::ci::profile_configuration::inquiry{});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileInquiryReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto receiver_muid =
      from_le7(std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}});

  constexpr auto enabled = std::array<byte_array_5, 2>{
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}},
      byte_array_5{std::byte{0x76}, std::byte{0x65}, std::byte{0x54}, std::byte{0x43}, std::byte{0x32}},
  };
  constexpr auto disabled = std::array<byte_array_5, 1>{
      byte_array_5{std::byte{0x71}, std::byte{0x61}, std::byte{0x51}, std::byte{0x41}, std::byte{0x31}},
  };

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::profile_inquiry_reply;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = receiver_muid;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  using midi2::ci::profile_configuration::inquiry_reply;
  EXPECT_CALL(config_.profile,
              inquiry_reply(config_.context, midici,
                            AllOf(Field("enabled", &inquiry_reply::enabled, ElementsAreArray(enabled)),
                                  Field("disabled", &inquiry_reply::disabled, ElementsAreArray(disabled)))))
      .Times(1);

  this->dispatch_ci(midici, inquiry_reply{enabled, disabled});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileAdded) {
  midi_ci midici;
  midici.group = std::uint8_t{0x01};
  midici.type = midi2::ci_message::profile_added;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = from_le7(broadcast_muid);

  midi2::ci::profile_configuration::added added;
  added.pid = byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};

  EXPECT_CALL(config_.profile, added(config_.context, midici, added)).Times(1);

  this->dispatch_ci(midici, added);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileRemoved) {
  midi_ci midici;
  midici.group = std::uint8_t{0x01};
  midici.type = midi2::ci_message::profile_removed;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = from_le7(broadcast_muid);

  midi2::ci::profile_configuration::removed removed;
  removed.pid = byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};

  EXPECT_CALL(config_.profile, removed(config_.context, midici, removed)).Times(1);

  this->dispatch_ci(midici, removed);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDetails) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::profile_details;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  midi2::ci::profile_configuration::details details;
  details.pid = byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  details.target = 0x23;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, details(config_.context, midici, details)).Times(1);

  this->dispatch_ci(midici, details);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDetailsReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto target = std::uint8_t{0x23};
  constexpr auto data = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::profile_details_reply;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  using midi2::ci::profile_configuration::details_reply;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, details_reply(config_.context, midici,
                                             AllOf(Field("pid", &details_reply::pid, Eq(pid)),
                                                   Field("target", &details_reply::target, Eq(target)),
                                                   Field("data", &details_reply::data, ElementsAreArray(data)))))
      .Times(1);

  this->dispatch_ci(midici, details_reply{pid, target, data});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileOn) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::profile_set_on;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  constexpr midi2::ci::profile_configuration::on on{
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}},
      std::uint16_t{23}};

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, on(config_.context, midici, on)).Times(1);

  this->dispatch_ci(midici, on);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileOff) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::profile_set_off;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  constexpr midi2::ci::profile_configuration::off off{
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}}};

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, off(config_.context, midici, off)).Times(1);

  this->dispatch_ci(midici, off);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileEnabled) {
  midi_ci midici;
  midici.group = std::uint8_t{0x01};
  midici.type = midi2::ci_message::profile_enabled;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = from_le7(broadcast_muid);

  midi2::ci::profile_configuration::enabled enabled;
  enabled.pid = byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  enabled.num_channels = 0x1122;

  EXPECT_CALL(config_.profile, enabled(config_.context, midici, enabled)).Times(1);

  this->dispatch_ci(midici, enabled);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDisabled) {
  midi_ci midici;
  midici.group = std::uint8_t{0x01};
  midici.type = midi2::ci_message::profile_disabled;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = from_le7(broadcast_muid);

  midi2::ci::profile_configuration::disabled disabled;
  disabled.pid = byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  disabled.num_channels = 0x123;

  EXPECT_CALL(config_.profile, disabled(config_.context, midici, disabled)).Times(1);

  this->dispatch_ci(midici, disabled);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileSpecificData) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto data = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  // clang-format on
  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::profile_specific_data;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = from_le7(broadcast_muid);

  using midi2::ci::profile_configuration::specific_data;
  EXPECT_CALL(config_.profile, specific_data(config_.context, midici,
                                             AllOf(Field("pid", &specific_data::pid, Eq(pid)),
                                                   Field("data", &specific_data::data, ElementsAreArray(data)))))
      .Times(1);

  this->dispatch_ci(midici, specific_data{pid, data});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeCapabilities) {
  constexpr auto group = std::uint8_t{0x01};
  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pe_capability;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  midi2::ci::property_exchange::capabilities caps;
  caps.num_simultaneous = 2;
  caps.major_version = 3;
  caps.minor_version = 4;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, capabilities(config_.context, midici, caps)).Times(1);

  this->dispatch_ci(midici, caps);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeCapabilitiesReply) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pe_capability_reply;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  midi2::ci::property_exchange::capabilities_reply caps;
  caps.num_simultaneous = 2;
  caps.major_version = 3;
  caps.minor_version = 4;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, capabilities_reply(config_.context, midici, caps)).Times(1);

  this->dispatch_ci(midici, caps);
}

using namespace std::string_view_literals;
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeGetPropertyData) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pe_get;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  using midi2::ci::property_exchange::get;
  using chunk_info = midi2::ci::property_exchange::property_exchange::chunk_info;

  get g;
  g.chunk = chunk_info{2, 1};
  g.request = std::uint8_t{1};
  g.header = R"({"status":200})"sv;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              get(config_.context, midici,
                  AllOf(Field("chunk", &get::chunk, Eq(g.chunk)), Field("request", &get::request, Eq(g.request)),
                        Field("header", &get::header, ElementsAreArray(g.header)))));

  this->dispatch_ci(midici, g);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeGetPropertyDataReply) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pe_get_reply;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  using get_reply = midi2::ci::property_exchange::get_reply;
  get_reply gr;
  gr.chunk.number_of_chunks = 1;
  gr.chunk.chunk_number = 1;
  gr.request = std::uint8_t{1};
  gr.header = R"({"status":200})"sv;
  gr.data = R"([{"resource":"DeviceInfo"},{"resource":"ChannelList"},{"resource":"CMList"}])"sv;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              get_reply(config_.context, midici,
                        AllOf(Field("chunk", &get_reply::chunk, Eq(gr.chunk)),
                              Field("request", &get_reply::request, Eq(gr.request)),
                              Field("header", &get_reply::header, ElementsAreArray(gr.header)),
                              Field("data", &get_reply::data, ElementsAreArray(gr.data)))));

  this->dispatch_ci(midici, gr);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSetPropertyData) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pe_set;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  using midi2::ci::property_exchange::set;
  set spd;
  spd.chunk.number_of_chunks = 1;
  spd.chunk.chunk_number = 1;
  spd.request = std::uint8_t{1};
  spd.header = R"({"resource":"X-ProgramEdit","resId":"abcd"})"sv;
  spd.data = R"({"name":"Violin 2","lfoSpeed":10,"lfoWaveform":"sine"})"sv;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              set(config_.context, midici,
                  AllOf(Field("chunk", &set::chunk, Eq(spd.chunk)), Field("request", &set::request, Eq(spd.request)),
                        Field("header", &set::header, ElementsAreArray(spd.header)),
                        Field("data", &set::data, ElementsAreArray(spd.data)))));

  this->dispatch_ci(midici, spd);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSetPropertyDataReply) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pe_set_reply;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  using midi2::ci::property_exchange::set_reply;
  set_reply spd_reply;
  spd_reply.chunk.number_of_chunks = 1;
  spd_reply.chunk.chunk_number = 1;
  spd_reply.request = 2;
  spd_reply.header = R"({"status":200})"sv;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              set_reply(config_.context, midici,
                        AllOf(Field("chunk", &set_reply::chunk, Eq(spd_reply.chunk)),
                              Field("request", &set_reply::request, Eq(spd_reply.request)),
                              Field("header", &set_reply::header, ElementsAreArray(spd_reply.header)))));

  this->dispatch_ci(midici, spd_reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSubscription) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};

  auto const header = R"({"command":"full","subscribeId":"sub32847623"})"sv;
  auto const data = "multichannel"sv;

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pe_sub;
  midici.params.device_id = midi2::to_underlying(destination);
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  using midi2::ci::property_exchange::subscription;
  subscription sub;
  sub.chunk.number_of_chunks = 1;
  sub.chunk.chunk_number = 1;
  sub.request = std::uint8_t{17};
  sub.header = header;
  sub.data = data;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              subscription(config_.context, midici,
                           AllOf(Field("chunk", &subscription::chunk, Eq(sub.chunk)),
                                 Field("request", &subscription::request, Eq(sub.request)),
                                 Field("header", &subscription::header, ElementsAreArray(header)),
                                 Field("data", &subscription::data, ElementsAreArray(data)))));

  this->dispatch_ci(midici, sub);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSubscriptionReply) {
  constexpr auto group = std::uint8_t{0x01};

  constexpr auto header = R"({"status":200})"sv;
  constexpr auto data = "data"sv;

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pe_sub_reply;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  using midi2::ci::property_exchange::subscription_reply;
  subscription_reply sub_reply;
  sub_reply.chunk.number_of_chunks = 1;
  sub_reply.chunk.chunk_number = 1;
  sub_reply.request = std::uint8_t{17};
  sub_reply.header = header;
  sub_reply.data = data;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              subscription_reply(config_.context, midici,
                                 AllOf(Field("chunk", &subscription_reply::chunk, Eq(sub_reply.chunk)),
                                       Field("request", &subscription_reply::request, Eq(sub_reply.request)),
                                       Field("header", &subscription_reply::header, ElementsAreArray(header)),
                                       Field("data", &subscription_reply::data, ElementsAreArray(data)))));

  this->dispatch_ci(midici, sub_reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeNotify) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto request = std::byte{1};
  constexpr auto header = R"({"status":144})"sv;
  constexpr auto data = "data"sv;

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pe_notify;
  midici.params.device_id = std::uint8_t{0x0F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  using midi2::ci::property_exchange::notify;
  notify note;
  note.chunk.number_of_chunks = 1;
  note.chunk.chunk_number = 1;
  note.request = midi2::to_underlying(request);
  note.header = header;
  note.data = data;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, notify(config_.context, midici,
                                                AllOf(Field("chunk", &notify::chunk, Eq(note.chunk)),
                                                      Field("request", &notify::request, Eq(note.request)),
                                                      Field("header", &notify::header, ElementsAreArray(header)),
                                                      Field("data", &notify::data, ElementsAreArray(data)))));

  this->dispatch_ci(midici, note);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryCapabilities) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pi_capability;
  midici.params.device_id = std::uint8_t{0x7F};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, capabilities(config_.context, midici)).Times(1);

  this->dispatch_ci(midici, midi2::ci::process_inquiry::capabilities{});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryCapabilitiesReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x7F};
  constexpr auto features = std::byte{0b0101010};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pi_capability_reply;
  midici.params.device_id = midi2::to_underlying(destination);
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  midi2::ci::process_inquiry::capabilities_reply reply;
  reply.features = features;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, capabilities_reply(config_.context, midici, reply)).Times(1);

  this->dispatch_ci(midici, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReport) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pi_mm_report;
  midici.params.device_id = std::uint8_t{0x01};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  midi2::ci::process_inquiry::midi_message_report report;
  report.message_data_control = decltype(report)::control::full;
  // system messages
  report.mtc_quarter_frame = 1;
  report.song_position = 0;
  report.song_select = 1;
  // channel controller messages
  report.pitchbend = 1;
  report.control_change = 0;
  report.rpn_registered_controller = 1;
  report.nrpn_assignable_controller = 0;
  report.program_change = 1;
  report.channel_pressure = 0;
  // note data messages
  report.notes = 1;
  report.poly_pressure = 0;
  report.per_note_pitchbend = 1;
  report.registered_per_note_controller = 0;
  report.assignable_per_note_controller = 1;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report(config_.context, midici, report)).Times(1);

  this->dispatch_ci(midici, report);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReportReply) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pi_mm_report_reply;
  midici.params.device_id = std::uint8_t{0x01};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  midi2::ci::process_inquiry::midi_message_report_reply reply;
  // system messages
  reply.mtc_quarter_frame = 1;
  reply.song_position = 0;
  reply.song_select = 1;
  // channel controller messages
  reply.pitchbend = 1;
  reply.control_change = 0;
  reply.rpn_registered_controller = 1;
  reply.nrpn_assignable_controller = 0;
  reply.program_change = 1;
  reply.channel_pressure = 0;
  // note data messages
  reply.notes = 1;
  reply.poly_pressure = 0;
  reply.per_note_pitchbend = 1;
  reply.registered_per_note_controller = 0;
  reply.assignable_per_note_controller = 1;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report_reply(config_.context, midici, reply)).Times(1);

  this->dispatch_ci(midici, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReportEnd) {
  constexpr auto group = std::uint8_t{0x01};

  midi_ci midici;
  midici.group = group;
  midici.type = midi2::ci_message::pi_mm_report_end;
  midici.params.device_id = std::uint8_t{0x01};
  midici.params.version = 2;
  midici.params.remote_muid = sender_muid_;
  midici.params.local_muid = destination_muid_;

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report_end(config_.context, midici)).Times(1);

  this->dispatch_ci(midici, midi2::ci::process_inquiry::midi_message_report_end{});
}

// This test simply gets ci_dispatcher to consume a random buffer.
void NeverCrashes(std::vector<std::byte> const &message) {
  // Ensure the top bit is clear.
  std::vector<std::byte> message2;
  message2.reserve(message.size());
  std::ranges::transform(message, std::back_inserter(message2), [](std::byte v) { return v & std::byte{0x7F}; });
  struct empty {};
  struct config {
    [[no_unique_address]] empty context;
    midi2::ci::dispatcher_backend::management_function<decltype(config::context)> management;
    midi2::ci::dispatcher_backend::profile_function<decltype(config::context)> profile;
    midi2::ci::dispatcher_backend::property_exchange_function<decltype(config::context)> property_exchange;
    midi2::ci::dispatcher_backend::process_inquiry_function<decltype(config::context)> process_inquiry;
  };
  midi2::ci::ci_dispatcher dispatcher{config{}};
  dispatcher.config().management.on_check_muid([](empty, std::uint8_t, std::uint32_t) { return true; });
  std::ranges::for_each(message2, std::bind_front(&decltype(dispatcher)::processMIDICI, &dispatcher));
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(CIProcessorFuzz, NeverCrashes);
#endif
// NOLINTNEXTLINE
TEST(CIProcessorFuzz, Empty) {
  NeverCrashes({});
}

}  // end anonymous namespace
