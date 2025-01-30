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

std::ostream &operator<<(std::ostream &os, property_exchange::chunk_info const &ci);
std::ostream &operator<<(std::ostream &os, property_exchange::chunk_info const &ci) {
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

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}
consteval std::byte operator""_b(char arg) noexcept {
  assert(arg < 256);
  return operator""_b(static_cast<unsigned long long>(arg));
}
consteval std::uint8_t operator""_u8(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::uint8_t>(arg);
}

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

constexpr auto broadcast_muid = std::array{0x7F_b, 0x7F_b, 0x7F_b, 0x7F_b};

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

  static constexpr auto sender_muid_ = from_le7(std::array{0x7F_b, 0x7E_b, 0x7D_b, 0x7C_b});
  static constexpr auto destination_muid_ = from_le7(std::array{0x62_b, 0x16_b, 0x63_b, 0x26_b});

  template <typename Content>
  static std::vector<std::byte> make_message(midi2::ci::params const &params, Content const &content) {
    std::vector<std::byte> message;
    auto out_it = std::back_inserter(message);
    midi2::ci::create_message(out_it, trivial_sentinel<decltype(out_it)>{}, params, content);
    message.push_back(0_b);  // a stray extra byte
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
  processor_.processMIDICI(0_b);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryV1) {
  constexpr midi_ci midici{
      .group = 0xFF_u8,
      .type = midi2::ci_message::discovery,
      .params =
          midi2::ci::params{.device_id = 0x7F_u8, .version = 1, .remote_muid = 0, .local_muid = midi2::ci_broadcast}};
  constexpr midi2::ci::discovery discovery{.manufacturer = std::array{0x12_u8, 0x23_u8, 0x34_u8},
                                           .family = (1U << (7 * 2)) - 1U,
                                           .model = (1U << (7 * 2)) - 2U,
                                           .version = std::array{0x7F_u8, 0x3C_u8, 0x2A_u8, 0x18_u8},
                                           .capability = 0x7F_u8,
                                           .max_sysex_size = (1 << (7 * 4)) - 1};
  EXPECT_CALL(config_.management, discovery(config_.context, midici, discovery)).Times(1);
  this->dispatch_ci(midici, discovery);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryV2) {
  constexpr midi_ci midici{
      .group = 0xFF_u8,
      .type = midi2::ci_message::discovery,
      .params =
          midi2::ci::params{.device_id = 0x7F_u8, .version = 2, .remote_muid = 0, .local_muid = midi2::ci_broadcast}};
  constexpr midi2::ci::discovery discovery{.manufacturer = std::array{0x12_u8, 0x23_u8, 0x34_u8},
                                           .family = from_le7(std::array{0x67_b, 0x79_b}),
                                           .model = from_le7(std::array{0x6B_b, 0x5D_b}),
                                           .version = midi2::ci::from_array(std::array{0x4E_b, 0x3C_b, 0x2A_b, 0x18_b}),
                                           .capability = 0x7F_u8,
                                           .max_sysex_size = from_le7(std::array{0x76_b, 0x54_b, 0x32_b, 0x10_b}),
                                           .output_path_id = 0x71_u8};
  EXPECT_CALL(config_.management, discovery(config_.context, midici, discovery)).Times(1);
  this->dispatch_ci(midici, discovery);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryReplyV2) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto manufacturer = std::array{0x12_b, 0x23_b, 0x34_b};
  constexpr auto family = std::array{0x67_b, 0x79_b};
  constexpr auto model = std::array{0x5B_b, 0x4D_b};
  constexpr auto version = std::array{0x7E_b, 0x6C_b, 0x5A_b, 0x48_b};
  constexpr auto capability = 0x7F_b;
  constexpr auto max_sysex_size = std::array{0x76_b, 0x54_b, 0x32_b, 0x10_b};
  constexpr auto output_path_id = 0x71_b;
  constexpr auto function_block = 0x32_b;

  constexpr midi_ci midici{.group = 0xFF,
                           .type = midi2::ci_message::discovery_reply,
                           .params = midi2::ci::params{.device_id = midi2::to_underlying(device_id),
                                                       .version = 2,
                                                       .remote_muid = 0,
                                                       .local_muid = midi2::ci_broadcast}};
  constexpr midi2::ci::discovery_reply reply{.manufacturer = midi2::ci::from_array(manufacturer),
                                             .family = from_le7(family),
                                             .model = from_le7(model),
                                             .version = midi2::ci::from_array(version),
                                             .capability = midi2::to_underlying(capability),
                                             .max_sysex_size = from_le7(max_sysex_size),
                                             .output_path_id = midi2::to_underlying(output_path_id),
                                             .function_block = midi2::to_underlying(function_block)};
  EXPECT_CALL(config_.management, discovery_reply(config_.context, midici, reply)).Times(1);
  this->dispatch_ci(midici, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, EndpointInfo) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto group = 0x01_u8;
  constexpr auto status = 0b0101010_u8;
  constexpr std::array const receiver_muid{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::endpoint_info,
                           .params = midi2::ci::params{.device_id = midi2::to_underlying(device_id),
                                                       .version = 1,
                                                       .remote_muid = sender_muid_,
                                                       .local_muid = from_le7(receiver_muid)}};
  constexpr midi2::ci::endpoint_info endpoint_info{.status = status};

  EXPECT_CALL(config_.management, check_muid(config_.context, group, from_le7(receiver_muid)))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, endpoint_info(config_.context, midici, endpoint_info)).Times(1);

  this->dispatch_ci(midici, endpoint_info);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, EndpointInfoReply) {
  constexpr auto group = 0x71_u8;
  constexpr auto receiver_muid = from_le7(std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b});
  constexpr auto status = 0b0101010_b;
  static constexpr auto information = std::array{
      2_b, 3_b, 5_b, 7_b, 11_b, 13_b, 17_b, 19_b,
  };

  constexpr midi_ci midici{
      .group = group,
      .type = midi2::ci_message::endpoint_info_reply,
      .params = midi2::ci::params{
          .device_id = 0x7F_u8, .version = 1, .remote_muid = sender_muid_, .local_muid = receiver_muid}};
  constexpr midi2::ci::endpoint_info_reply reply{.status = from_le7(status), .information = information};
  EXPECT_CALL(config_.management, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, endpoint_info_reply(config_.context, midici, reply)).Times(1);

  this->dispatch_ci(midici, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, InvalidateMuid) {
  constexpr auto group = 0x71_u8;
  constexpr auto device_id = 0x7F_b;
  constexpr std::array const receiver_muid{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};
  constexpr std::array const target_muid{0x21_b, 0x43_b, 0x75_b, 0x71_b};

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::invalidate_muid,
                           .params = midi2::ci::params{.device_id = midi2::to_underlying(device_id),
                                                       .version = 1,
                                                       .remote_muid = sender_muid_,
                                                       .local_muid = from_le7(receiver_muid)}};
  constexpr midi2::ci::invalidate_muid invalidate_muid{.target_muid = from_le7(target_muid)};
  EXPECT_CALL(config_.management, check_muid(config_.context, group, from_le7(receiver_muid)))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, invalidate_muid(config_.context, midici, invalidate_muid)).Times(1);

  this->dispatch_ci(midici, invalidate_muid);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, Ack) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = from_le7(std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b});

  constexpr auto original_id = 0x34_u8;
  constexpr auto ack_status_code = 0x00_u8;
  constexpr auto ack_status_data = 0x7F_u8;
  constexpr auto ack_details = std::array{0x01_b, 0x02_b, 0x03_b, 0x04_b, 0x05_b};
  static constexpr auto text = std::array{'H'_b, 'e'_b, 'l'_b, 'l'_b, 'o'_b};

  constexpr midi_ci midici{
      .group = group,
      .type = midi2::ci_message::ack,
      .params = midi2::ci::params{
          .device_id = 0x7F_u8, .version = 1, .remote_muid = sender_muid_, .local_muid = receiver_muid}};
  constexpr midi2::ci::ack ack{.original_id = original_id,
                               .status_code = ack_status_code,
                               .status_data = ack_status_data,
                               .details = ack_details,
                               .message = text};
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
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = from_le7(std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b});

  constexpr auto text_length = std::size_t{512};
  std::vector<std::byte> text;
  text.resize(text_length);

  constexpr midi_ci midici{
      .group = group,
      .type = midi2::ci_message::ack,
      .params = midi2::ci::params{
          .device_id = 0x7F_u8, .version = 1, .remote_muid = sender_muid_, .local_muid = receiver_muid}};
  midi2::ci::ack const ack{.original_id = 0x34_u8,
                           .status_code = 0x17_u8,
                           .status_data = 0x7F_u8,
                           .details = std::array{0x01_b, 0x02_b, 0x03_b, 0x04_b, 0x05_b},
                           .message = text};

  EXPECT_CALL(config_.management, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, buffer_overflow(config_.context)).Times(1);

  this->dispatch_ci(midici, ack);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, NakV1) {
  constexpr auto group = 0x01_u8;
  constexpr auto device_id = 0x7F_b;
  constexpr auto receiver_muid = from_le7(std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b});

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::nak,
                           .params = midi2::ci::params{.device_id = midi2::to_underlying(device_id),
                                                       .version = 1,
                                                       .remote_muid = sender_muid_,
                                                       .local_muid = receiver_muid}};
  midi2::ci::nak const nak;

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
  constexpr auto group = 0x01_u8;
  constexpr auto device_id = 0x7F_b;
  constexpr auto receiver_muid = std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};

  constexpr auto original_id = 0x34_u8;
  constexpr auto nak_status_code = 0x17_u8;
  constexpr auto nak_status_data = 0x7F_u8;
  constexpr auto nak_details = std::array{0x01_b, 0x02_b, 0x03_b, 0x04_b, 0x05_b};
  static constexpr auto text = std::array{'H'_b, 'e'_b, 'l'_b, 'l'_b, 'o'_b};

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::nak,
                           .params = midi2::ci::params{.device_id = midi2::to_underlying(device_id),
                                                       .version = 2,
                                                       .remote_muid = sender_muid_,
                                                       .local_muid = from_le7(receiver_muid)}};
  constexpr midi2::ci::nak nak{.original_id = original_id,
                               .status_code = nak_status_code,
                               .status_data = nak_status_data,
                               .details = nak_details,
                               .message = text};
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
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = from_le7(std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b});
  constexpr midi_ci midici{
      .group = group,
      .type = midi2::ci_message::profile_inquiry,
      .params = midi2::ci::params{
          .device_id = 0x0F_u8, .version = 2, .remote_muid = sender_muid_, .local_muid = receiver_muid}};
  EXPECT_CALL(config_.management, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, inquiry(config_.context, midici)).Times(1);

  this->dispatch_ci(midici, midi2::ci::profile_configuration::inquiry{});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileInquiryReply) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = from_le7(std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b});

  constexpr auto enabled = std::array<byte_array_5, 2>{
      byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b},
      byte_array_5{0x76_b, 0x65_b, 0x54_b, 0x43_b, 0x32_b},
  };
  constexpr auto disabled = std::array<byte_array_5, 1>{
      byte_array_5{0x71_b, 0x61_b, 0x51_b, 0x41_b, 0x31_b},
  };

  constexpr midi_ci midici{
      .group = group,
      .type = midi2::ci_message::profile_inquiry_reply,
      .params = midi2::ci::params{
          .device_id = 0x0F_u8, .version = 2, .remote_muid = sender_muid_, .local_muid = receiver_muid}};
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
  constexpr midi_ci midici{
      .group = 0x01_u8,
      .type = midi2::ci_message::profile_added,
      .params = midi2::ci::params{
          .device_id = 0x0F_u8, .version = 2, .remote_muid = sender_muid_, .local_muid = from_le7(broadcast_muid)}};
  constexpr midi2::ci::profile_configuration::added added{.pid = byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b}};
  EXPECT_CALL(config_.profile, added(config_.context, midici, added)).Times(1);

  this->dispatch_ci(midici, added);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileRemoved) {
  constexpr midi_ci midici{
      .group = 0x01_u8,
      .type = midi2::ci_message::profile_removed,
      .params = midi2::ci::params{
          .device_id = 0x0F_u8, .version = 2, .remote_muid = sender_muid_, .local_muid = from_le7(broadcast_muid)}};
  constexpr midi2::ci::profile_configuration::removed removed{.pid =
                                                                  byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b}};
  EXPECT_CALL(config_.profile, removed(config_.context, midici, removed)).Times(1);

  this->dispatch_ci(midici, removed);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDetails) {
  constexpr auto group = 0x01_u8;

  constexpr midi_ci midici{
      .group = group,
      .type = midi2::ci_message::profile_details,
      .params = midi2::ci::params{
          .device_id = 0x0F_u8, .version = 2, .remote_muid = sender_muid_, .local_muid = destination_muid_}};
  constexpr midi2::ci::profile_configuration::details details{
      .pid = byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b}, .target = 0x23};

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, details(config_.context, midici, details)).Times(1);

  this->dispatch_ci(midici, details);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDetailsReply) {
  constexpr auto group = 0x01_u8;
  constexpr auto pid = byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b};
  constexpr auto target = 0x23_u8;
  constexpr auto data = std::array{'H'_b, 'e'_b, 'l'_b, 'l'_b, 'o'_b};

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::profile_details_reply,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
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
  constexpr auto group = 0x01_u8;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::profile_set_on,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  constexpr midi2::ci::profile_configuration::on on{
      .pid = byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b},
      .num_channels = std::uint16_t{23},
  };

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, on(config_.context, midici, on)).Times(1);

  this->dispatch_ci(midici, on);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileOff) {
  constexpr auto group = 0x01_u8;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::profile_set_off,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  constexpr midi2::ci::profile_configuration::off off{.pid = byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b}};

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, off(config_.context, midici, off)).Times(1);

  this->dispatch_ci(midici, off);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileEnabled) {
  constexpr midi_ci midici{.group = 0x01_u8,
                           .type = midi2::ci_message::profile_enabled,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = from_le7(broadcast_muid),
                           }};
  constexpr midi2::ci::profile_configuration::enabled enabled{
      .pid = byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b},
      .num_channels = 0x1122,
  };

  EXPECT_CALL(config_.profile, enabled(config_.context, midici, enabled)).Times(1);

  this->dispatch_ci(midici, enabled);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDisabled) {
  constexpr midi_ci midici{.group = 0x01_u8,
                           .type = midi2::ci_message::profile_disabled,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = from_le7(broadcast_muid),
                           }};
  constexpr midi2::ci::profile_configuration::disabled disabled{
      .pid = byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b},
      .num_channels = 0x123,
  };
  EXPECT_CALL(config_.profile, disabled(config_.context, midici, disabled)).Times(1);

  this->dispatch_ci(midici, disabled);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileSpecificData) {
  constexpr auto group = 0x01_u8;
  constexpr auto pid = byte_array_5{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b};
  constexpr auto data = std::array{'H'_b, 'e'_b, 'l'_b, 'l'_b, 'o'_b};

  // clang-format on
  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::profile_specific_data,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = from_le7(broadcast_muid),
                           }};
  using midi2::ci::profile_configuration::specific_data;
  EXPECT_CALL(config_.profile, specific_data(config_.context, midici,
                                             AllOf(Field("pid", &specific_data::pid, Eq(pid)),
                                                   Field("data", &specific_data::data, ElementsAreArray(data)))))
      .Times(1);

  this->dispatch_ci(midici, specific_data{pid, data});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeCapabilities) {
  constexpr auto group = 0x01_u8;
  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pe_capability,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  constexpr midi2::ci::property_exchange::capabilities caps{
      .num_simultaneous = 2,
      .major_version = 3,
      .minor_version = 4,
  };
  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, capabilities(config_.context, midici, caps)).Times(1);

  this->dispatch_ci(midici, caps);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeCapabilitiesReply) {
  constexpr auto group = 0x01_u8;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pe_capability_reply,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  constexpr midi2::ci::property_exchange::capabilities_reply caps{
      .num_simultaneous = 2,
      .major_version = 3,
      .minor_version = 4,
  };
  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, capabilities_reply(config_.context, midici, caps)).Times(1);

  this->dispatch_ci(midici, caps);
}

using namespace std::string_view_literals;
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeGetPropertyData) {
  constexpr auto group = 0x01_u8;
  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pe_get,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  constexpr midi2::ci::property_exchange::get g{
      .chunk = midi2::ci::property_exchange::chunk_info{2, 1},
      .request = 1_u8,
      .header = R"({"status":200})"sv,
  };
  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, get(config_.context, midici, g));

  this->dispatch_ci(midici, g);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeGetPropertyDataReply) {
  constexpr auto group = 0x01_u8;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pe_get_reply,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  using get_reply = midi2::ci::property_exchange::get_reply;
  constexpr get_reply gr{
      .chunk.number_of_chunks = 1,
      .chunk.chunk_number = 1,
      .request = 1_u8,
      .header = R"({"status":200})"sv,
      .data = R"([{"resource":"DeviceInfo"},{"resource":"ChannelList"},{"resource":"CMList"}])"sv,
  };
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
  constexpr auto group = 0x01_u8;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pe_set,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  using midi2::ci::property_exchange::set;
  constexpr set spd{
      .chunk.number_of_chunks = 1,
      .chunk.chunk_number = 1,
      .request = 1_u8,
      .header = R"({"resource":"X-ProgramEdit","resId":"abcd"})"sv,
      .data = R"({"name":"Violin 2","lfoSpeed":10,"lfoWaveform":"sine"})"sv,
  };
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
  constexpr auto group = 0x01_u8;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pe_set_reply,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  using midi2::ci::property_exchange::set_reply;
  constexpr set_reply spd_reply{
      .chunk.number_of_chunks = 1,
      .chunk.chunk_number = 1,
      .request = 2,
      .header = R"({"status":200})"sv,
  };
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
  constexpr auto group = 0x01_u8;
  constexpr auto destination = 0x0F_b;

  constexpr auto header = R"({"command":"full","subscribeId":"sub32847623"})"sv;
  constexpr auto data = "multichannel"sv;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pe_sub,
                           .params = midi2::ci::params{
                               .device_id = midi2::to_underlying(destination),
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  using midi2::ci::property_exchange::subscription;
  constexpr subscription sub{
      .chunk.number_of_chunks = 1,
      .chunk.chunk_number = 1,
      .request = 17_u8,
      .header = header,
      .data = data,
  };
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
  constexpr auto group = 0x01_u8;

  constexpr auto header = R"({"status":200})"sv;
  constexpr auto data = "data"sv;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pe_sub_reply,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  using midi2::ci::property_exchange::subscription_reply;
  constexpr subscription_reply sub_reply{
      .chunk.number_of_chunks = 1,
      .chunk.chunk_number = 1,
      .request = 17_u8,
      .header = header,
      .data = data,
  };
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
  constexpr auto group = 0x01_u8;
  constexpr auto request = 1_b;
  constexpr auto header = R"({"status":144})"sv;
  constexpr auto data = "data"sv;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pe_notify,
                           .params = midi2::ci::params{
                               .device_id = 0x0F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  using midi2::ci::property_exchange::notify;
  constexpr notify note{
      .chunk.number_of_chunks = 1,
      .chunk.chunk_number = 1,
      .request = midi2::to_underlying(request),
      .header = header,
      .data = data,
  };
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
  constexpr auto group = 0x01_u8;
  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pi_capability,
                           .params = midi2::ci::params{
                               .device_id = 0x7F_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, capabilities(config_.context, midici)).Times(1);

  this->dispatch_ci(midici, midi2::ci::process_inquiry::capabilities{});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryCapabilitiesReply) {
  constexpr auto group = 0x01_u8;
  constexpr auto destination = 0x7F_b;
  constexpr auto features = 0b0101010_b;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pi_capability_reply,
                           .params = midi2::ci::params{
                               .device_id = midi2::to_underlying(destination),
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  constexpr midi2::ci::process_inquiry::capabilities_reply reply{.features = features};

  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, capabilities_reply(config_.context, midici, reply)).Times(1);

  this->dispatch_ci(midici, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReport) {
  constexpr auto group = 0x01_u8;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pi_mm_report,
                           .params = midi2::ci::params{
                               .device_id = 0x01_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  constexpr midi2::ci::process_inquiry::midi_message_report report{
      .message_data_control = decltype(report)::control::full,
      // system messages
      .mtc_quarter_frame = 1,
      .song_position = 0,
      .song_select = 1,
      // channel controller messages
      .pitchbend = 1,
      .control_change = 0,
      .rpn_registered_controller = 1,
      .nrpn_assignable_controller = 0,
      .program_change = 1,
      .channel_pressure = 0,
      // note data messages
      .notes = 1,
      .poly_pressure = 0,
      .per_note_pitchbend = 1,
      .registered_per_note_controller = 0,
      .assignable_per_note_controller = 1,
  };
  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report(config_.context, midici, report)).Times(1);

  this->dispatch_ci(midici, report);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReportReply) {
  constexpr auto group = 0x01_u8;

  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pi_mm_report_reply,
                           .params = midi2::ci::params{
                               .device_id = 0x01_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  constexpr midi2::ci::process_inquiry::midi_message_report_reply reply{
      // system messages
      .mtc_quarter_frame = 1,
      .song_position = 0,
      .song_select = 1,
      // channel controller messages
      .pitchbend = 1,
      .control_change = 0,
      .rpn_registered_controller = 1,
      .nrpn_assignable_controller = 0,
      .program_change = 1,
      .channel_pressure = 0,
      // note data messages
      .notes = 1,
      .poly_pressure = 0,
      .per_note_pitchbend = 1,
      .registered_per_note_controller = 0,
      .assignable_per_note_controller = 1,
  };
  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report_reply(config_.context, midici, reply)).Times(1);

  this->dispatch_ci(midici, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReportEnd) {
  constexpr auto group = 0x01_u8;
  constexpr midi_ci midici{.group = group,
                           .type = midi2::ci_message::pi_mm_report_end,
                           .params = midi2::ci::params{
                               .device_id = 0x01_u8,
                               .version = 2,
                               .remote_muid = sender_muid_,
                               .local_muid = destination_muid_,
                           }};
  EXPECT_CALL(config_.management, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report_end(config_.context, midici)).Times(1);

  this->dispatch_ci(midici, midi2::ci::process_inquiry::midi_message_report_end{});
}

// This test simply gets ci_dispatcher to consume a random buffer.
void NeverCrashes(std::vector<std::byte> const &message) {
  // Ensure the top bit is clear.
  std::vector<std::byte> message2;
  message2.reserve(message.size());
  std::ranges::transform(message, std::back_inserter(message2), [](std::byte v) { return v & 0x7F_b; });
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
