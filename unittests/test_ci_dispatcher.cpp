//===-- CI Dispatcher ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ci/ci_dispatcher.hpp"

// MIDI2
#include "midi2/ci/ci_create_message.hpp"
#include "midi2/ci/ci_types.hpp"
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
std::ostream &write_bytes(std::ostream &os, std::span<midi2::ci::b7 const> const &arr) {
  os << '[';
  auto const *separator = "";
  for (auto const v : arr) {
    os << separator << unsigned{v.get()};
    separator = ",";
  }
  os << ']';
  return os;
}
}  // end anonymous namespace

namespace midi2::ci {

std::ostream &operator<<(std::ostream &os, header const &ci);
std::ostream &operator<<(std::ostream &os, header const &ci) {
  return os << "{ device_id=" << static_cast<unsigned>(ci.device_id.get())
            << ", version=" << static_cast<unsigned>(ci.version.get()) << ", remote_muid=" << ci.remote_muid.get()
            << ", local_muid=" << ci.local_muid.get() << " }";
}

std::ostream &operator<<(std::ostream &os, property_exchange::chunk_info const &ci);
std::ostream &operator<<(std::ostream &os, property_exchange::chunk_info const &ci) {
  return os << "{ number_of_chunks=" << ci.number_of_chunks.get() << ", chunk_number=" << ci.chunk_number.get() << " }";
}

std::ostream &operator<<(std::ostream &os, ci::discovery const &d);
std::ostream &operator<<(std::ostream &os, ci::discovery const &d) {
  os << "{ manufacturer=";
  write_bytes(os, std::span{d.manufacturer});
  os << ", family=" << d.family.get() << ", model=" << d.model.get() << ", version=";
  write_bytes(os, d.version);
  os << ", capability=" << static_cast<unsigned>(d.capability.get())
     << ", max_sysex_size=" << std::uint32_t{d.max_sysex_size} << " }";
  return os;
}

std::ostream &operator<<(std::ostream &os, ci::discovery_reply const &d);
std::ostream &operator<<(std::ostream &os, ci::discovery_reply const &d) {
  os << "{ manufacturer=";
  write_bytes(os, d.manufacturer);
  os << ", family=" << d.family.get() << ", model=" << d.model.get() << ", version=";
  write_bytes(os, d.version);
  os << ", capability=" << static_cast<unsigned>(d.capability.get()) << ", max_sysex_size=" << d.max_sysex_size.get()
     << " }";
  return os;
}

std::ostream &operator<<(std::ostream &os, ci::nak const &nak);
std::ostream &operator<<(std::ostream &os, ci::nak const &nak) {
  os << "{ original_id=" << static_cast<unsigned>(nak.original_id.get())
     << ", status_code=" << static_cast<unsigned>(nak.status_code.get())
     << ", status_data=" << static_cast<unsigned>(nak.status_data.get()) << ", details=";
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
  os << ", num_channels=" << pd.num_channels.get() << " }";
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

using midi2::ci::byte_array;
using midi2::ci::header;
using midi2::ci::details::from_byte_array;
using midi2::ci::details::from_le7;
using namespace midi2::ci::literals;

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}
consteval std::uint8_t operator""_u8(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::uint8_t>(arg);
}

struct context_type {
  constexpr bool operator==(context_type const &) const noexcept = default;
};

class mock_system_callbacks : public midi2::ci::dispatcher_backend::system_pure<context_type> {
public:
  MOCK_METHOD(bool, check_muid, (context_type, std::uint8_t group, midi2::ci::muid muid), (override));
  MOCK_METHOD(void, buffer_overflow, (context_type), (override));
  MOCK_METHOD(void, unknown_midici, (context_type, header const &), (override));
};

class mock_management_callbacks : public midi2::ci::dispatcher_backend::management_pure<context_type> {
public:
  MOCK_METHOD(void, discovery, (context_type, header const &, midi2::ci::discovery const &), (override));
  MOCK_METHOD(void, discovery_reply, (context_type, header const &, midi2::ci::discovery_reply const &), (override));
  MOCK_METHOD(void, endpoint, (context_type, header const &, midi2::ci::endpoint const &), (override));
  MOCK_METHOD(void, endpoint_reply, (context_type, header const &, midi2::ci::endpoint_reply const &), (override));
  MOCK_METHOD(void, invalidate_muid, (context_type, header const &, midi2::ci::invalidate_muid const &), (override));
  MOCK_METHOD(void, ack, (context_type, header const &, midi2::ci::ack const &), (override));
  MOCK_METHOD(void, nak, (context_type, header const &, midi2::ci::nak const &), (override));
};

class mock_profile_callbacks : public midi2::ci::dispatcher_backend::profile_pure<context_type> {
public:
  MOCK_METHOD(void, inquiry, (context_type, header const &), (override));
  MOCK_METHOD(void, inquiry_reply,
              (context_type, header const &, midi2::ci::profile_configuration::inquiry_reply const &), (override));
  MOCK_METHOD(void, added, (context_type, header const &, midi2::ci::profile_configuration::added const &), (override));
  MOCK_METHOD(void, removed, (context_type, header const &, midi2::ci::profile_configuration::removed const &),
              (override));
  MOCK_METHOD(void, details, (context_type, header const &, midi2::ci::profile_configuration::details const &),
              (override));
  MOCK_METHOD(void, details_reply,
              (context_type, header const &, midi2::ci::profile_configuration::details_reply const &), (override));
  MOCK_METHOD(void, on, (context_type, header const &, midi2::ci::profile_configuration::on const &), (override));
  MOCK_METHOD(void, off, (context_type, header const &, midi2::ci::profile_configuration::off const &), (override));
  MOCK_METHOD(void, enabled, (context_type, header const &, midi2::ci::profile_configuration::enabled const &),
              (override));
  MOCK_METHOD(void, disabled, (context_type, header const &, midi2::ci::profile_configuration::disabled const &),
              (override));
  MOCK_METHOD(void, specific_data,
              (context_type, header const &, midi2::ci::profile_configuration::specific_data const &), (override));
};

class mock_property_exchange_callbacks : public midi2::ci::dispatcher_backend::property_exchange_pure<context_type> {
public:
  MOCK_METHOD(void, capabilities, (context_type, header const &, midi2::ci::property_exchange::capabilities const &),
              (override));
  MOCK_METHOD(void, capabilities_reply,
              (context_type, header const &, midi2::ci::property_exchange::capabilities_reply const &), (override));

  MOCK_METHOD(void, get, (context_type, header const &, midi2::ci::property_exchange::get const &), (override));
  MOCK_METHOD(void, get_reply, (context_type, header const &, midi2::ci::property_exchange::get_reply const &),
              (override));
  MOCK_METHOD(void, set, (context_type, header const &, midi2::ci::property_exchange::set const &), (override));
  MOCK_METHOD(void, set_reply, (context_type, header const &, midi2::ci::property_exchange::set_reply const &),
              (override));
  MOCK_METHOD(void, subscription, (context_type, header const &, midi2::ci::property_exchange::subscription const &),
              (override));
  MOCK_METHOD(void, subscription_reply,
              (context_type, header const &, midi2::ci::property_exchange::subscription_reply const &), (override));
  MOCK_METHOD(void, notify, (context_type, header const &, midi2::ci::property_exchange::notify const &), (override));
};

class mock_process_inquiry_callbacks : public midi2::ci::dispatcher_backend::process_inquiry_pure<context_type> {
public:
  MOCK_METHOD(void, capabilities, (context_type, header const &), (override));
  MOCK_METHOD(void, capabilities_reply,
              (context_type, header const &, midi2::ci::process_inquiry::capabilities_reply const &), (override));
  MOCK_METHOD(void, midi_message_report,
              (context_type, header const &, midi2::ci::process_inquiry::midi_message_report const &), (override));
  MOCK_METHOD(void, midi_message_report_reply,
              (context_type, header const &, midi2::ci::process_inquiry::midi_message_report_reply const &),
              (override));
  MOCK_METHOD(void, midi_message_report_end, (context_type, header const &), (override));
};

constexpr auto broadcast_muid = midi2::ci::broadcast_muid;

class CIDispatcher : public testing::Test {
public:
  CIDispatcher() : processor_{config_} {}

protected:
  struct mocked_config {
    [[no_unique_address]] context_type context;
    static constexpr auto buffer_size = std::size_t{128};
    StrictMock<mock_system_callbacks> system;
    StrictMock<mock_management_callbacks> management;
    StrictMock<mock_profile_callbacks> profile;
    StrictMock<mock_property_exchange_callbacks> property_exchange;
    StrictMock<mock_process_inquiry_callbacks> process_inquiry;
  };
  mocked_config config_;
  midi2::ci::ci_dispatcher<std::reference_wrapper<mocked_config>> processor_;

  static constexpr auto sender_muid_ = midi2::ci::muid{from_le7(std::array{0x7F_b, 0x7E_b, 0x7D_b, 0x7C_b})};
  static constexpr auto destination_muid_ = midi2::ci::muid{from_le7(std::array{0x62_b, 0x16_b, 0x63_b, 0x26_b})};

  template <typename Content> static std::vector<std::byte> make_message(header const &hdr, Content const &content) {
    std::vector<std::byte> message;
    midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{}, hdr, content);
    message.push_back(0_b);  // a stray extra byte
    return message;
  }

  template <typename Content> void dispatch_ci(std::uint8_t group, header const &hdr, Content const &content) {
    processor_.start(group, hdr.device_id);
    std::ranges::for_each(make_message(hdr, content), [this](std::byte const b) { processor_.dispatch(b); });
    processor_.finish();
  }
};
// NOLINTNEXTLINE
TEST_F(CIDispatcher, Empty) {
  processor_.dispatch(0_b);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryV1) {
  constexpr header hdr{.device_id = 0x7F_b7,
                       .version = 1_b7,
                       .remote_muid = midi2::ci::muid{0U},
                       .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery discovery{.manufacturer = std::array{0x12_b7, 0x23_b7, 0x34_b7},
                                           .family = midi2::ci::b14{(1U << (7 * 2)) - 1U},
                                           .model = midi2::ci::b14{(1U << (7 * 2)) - 2U},
                                           .version = std::array{0x7F_b7, 0x3C_b7, 0x2A_b7, 0x18_b7},
                                           .capability = 0x7F_b7,
                                           .max_sysex_size = midi2::ci::b28{(1U << (7 * 4)) - 1U},
                                           .output_path_id = 0_b7};
  EXPECT_CALL(config_.management, discovery(config_.context, hdr, discovery)).Times(1);
  this->dispatch_ci(0xFF_u8, hdr, discovery);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryV2) {
  constexpr header hdr{.device_id = 0x7F_b7,
                       .version = 2_b7,
                       .remote_muid = midi2::ci::muid{0U},
                       .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery discovery{.manufacturer = std::array{0x12_b7, 0x23_b7, 0x34_b7},
                                           .family = 0x1779_b14,
                                           .model = 0x1B5D_b14,
                                           .version = from_byte_array(std::array{0x4E_b, 0x3C_b, 0x2A_b, 0x18_b}),
                                           .capability = 0x7F_b7,
                                           .max_sysex_size = 0x07654321_b28,
                                           .output_path_id = 0x71_b7};
  EXPECT_CALL(config_.management, discovery(config_.context, hdr, discovery)).Times(1);
  this->dispatch_ci(0xFF_u8, hdr, discovery);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryReplyV2) {
  constexpr auto device_id = 0x7F_b7;
  constexpr auto manufacturer = std::array{0x12_b, 0x23_b, 0x34_b};
  constexpr auto version = std::array{0x7E_b, 0x6C_b, 0x5A_b, 0x48_b};

  constexpr header hdr{.device_id = device_id,
                       .version = 2_b7,
                       .remote_muid = midi2::ci::muid{0U},
                       .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery_reply reply{.manufacturer = from_byte_array(manufacturer),
                                             .family = 0x33F9_b14,
                                             .model = 0x014D_b14,
                                             .version = from_byte_array(version),
                                             .capability = 0x7F_b7,
                                             .max_sysex_size = 0xFF_b28,
                                             .output_path_id = 0x71_b7,
                                             .function_block = 0x32_b7};
  EXPECT_CALL(config_.management, discovery_reply(config_.context, hdr, reply)).Times(1);
  this->dispatch_ci(0xFF_u8, hdr, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, Endpoint) {
  constexpr auto group = 0x01_u8;
  constexpr midi2::ci::muid const receiver_muid{0x012345EFU};

  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::endpoint endpoint{.status = 0b0101010_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, endpoint(config_.context, hdr, endpoint)).Times(1);

  this->dispatch_ci(group, hdr, endpoint);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, EndpointReply) {
  constexpr auto group = 0x71_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr auto information = std::array{'1'_b7, '2'_b7, '3'_b7, '4'_b7};

  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  midi2::ci::endpoint_reply reply{.status = 0_b7, .information = information};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, endpoint_reply(config_.context, hdr, reply)).Times(1);

  this->dispatch_ci(group, hdr, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, InvalidateMuid) {
  constexpr auto group = 0x71_u8;
  constexpr auto device_id = 0x7F_b7;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr auto target_muid = midi2::ci::muid{0x0FE43210U};

  constexpr header hdr{
      .device_id = device_id, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::invalidate_muid invalidate_muid{.target_muid = target_muid};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, invalidate_muid(config_.context, hdr, invalidate_muid)).Times(1);

  this->dispatch_ci(group, hdr, invalidate_muid);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, Ack) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr auto original_id = 0x34_b7;
  constexpr auto ack_status_code = 0x00_b7;
  constexpr auto ack_status_data = 0x7F_b7;
  constexpr auto ack_details = std::array{0x01_b7, 0x02_b7, 0x03_b7, 0x04_b7, 0x05_b7};
  static constexpr auto text = std::array{'H'_b7, 'e'_b7, 'l'_b7, 'l'_b7, 'o'_b7};

  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::ack ack{.original_id = original_id,
                               .status_code = ack_status_code,
                               .status_data = ack_status_data,
                               .details = ack_details,
                               .message = text};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, ack(config_.context, hdr,
                                      AllOf(Field("original_id", &midi2::ci::ack::original_id, Eq(original_id)),
                                            Field("status_code", &midi2::ci::ack::status_code, Eq(ack_status_code)),
                                            Field("status_data", &midi2::ci::ack::status_data, Eq(ack_status_data)),
                                            Field("details", &midi2::ci::ack::details, ElementsAreArray(ack_details)),
                                            Field("message", &midi2::ci::ack::message, ElementsAreArray(text)))))
      .Times(1);

  this->dispatch_ci(group, hdr, ack);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, AckMessageTooLong) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr auto text_length = decltype(config_)::buffer_size;
  std::vector<midi2::ci::b7> text{text_length, 'a'_b7};

  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  midi2::ci::ack const ack{.original_id = 0x34_b7,
                           .status_code = 0x17_b7,
                           .status_data = 0x7F_b7,
                           .details = std::array{0x01_b7, 0x02_b7, 0x03_b7, 0x04_b7, 0x05_b7},
                           .message = text};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.system, buffer_overflow(config_.context)).Times(1);

  this->dispatch_ci(group, hdr, ack);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, NakV1) {
  constexpr auto group = 0x01_u8;
  constexpr auto device_id = 0x7F_b7;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr header hdr{
      .device_id = device_id, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::nak nak;

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, nak(config_.context, hdr, nak)).Times(1);

  this->dispatch_ci(group, hdr, nak);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, NakV2) {
  constexpr auto group = 0x01_u8;
  constexpr auto device_id = 0x7F_b7;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr auto original_id = 0x34_b7;
  constexpr auto nak_status_code = 0x17_b7;
  constexpr auto nak_status_data = 0x7F_b7;
  constexpr auto nak_details = std::array{0x01_b7, 0x02_b7, 0x03_b7, 0x04_b7, 0x05_b7};
  // (The code points a span<> at this object so it must be static in order for the
  // objects to be constexpr.)
  static constexpr auto text = std::array{'H'_b7, 'e'_b7, 'l'_b7, 'l'_b7, 'o'_b7};

  constexpr header hdr{
      .device_id = device_id, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::nak nak{.original_id = original_id,
                               .status_code = nak_status_code,
                               .status_data = nak_status_data,
                               .details = nak_details,
                               .message = text};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, nak(config_.context, hdr, nak)).Times(1);

  this->dispatch_ci(group, hdr, nak);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileInquiry) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr auto hdr =
      header{.device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, inquiry(config_.context, hdr)).Times(1);

  this->dispatch_ci(group, hdr, midi2::ci::profile_configuration::inquiry{});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileInquiryReply) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr auto enabled = std::array{
      profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7},
      profile{0x76_b7, 0x65_b7, 0x54_b7, 0x43_b7, 0x32_b7},
  };
  constexpr auto disabled = std::array<profile, 1>{
      profile{0x71_b7, 0x61_b7, 0x51_b7, 0x41_b7, 0x31_b7},
  };

  constexpr header hdr{.device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  using midi2::ci::profile_configuration::inquiry_reply;
  EXPECT_CALL(config_.profile,
              inquiry_reply(config_.context, hdr,
                            AllOf(Field("enabled", &inquiry_reply::enabled, ElementsAreArray(enabled)),
                                  Field("disabled", &inquiry_reply::disabled, ElementsAreArray(disabled)))))
      .Times(1);

  this->dispatch_ci(group, hdr, inquiry_reply{enabled, disabled});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileAdded) {
  using midi2::ci::profile_configuration::added;
  using midi2::ci::profile_configuration::profile;
  constexpr header hdr{
      .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = broadcast_muid};
  constexpr added added_profile{.pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7}};
  EXPECT_CALL(config_.profile, added(config_.context, hdr, added_profile)).Times(1);

  this->dispatch_ci(0x01_u8, hdr, added_profile);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileRemoved) {
  using midi2::ci::profile_configuration::profile;
  using midi2::ci::profile_configuration::removed;
  constexpr header hdr{
      .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = broadcast_muid};
  constexpr removed removed_profile{.pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7}};
  EXPECT_CALL(config_.profile, removed(config_.context, hdr, removed_profile)).Times(1);

  this->dispatch_ci(0x01_u8, hdr, removed_profile);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDetails) {
  using midi2::ci::profile_configuration::details;
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = destination_muid_};
  constexpr details config_details{.pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7}, .target = 0x23_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, details(config_.context, hdr, config_details)).Times(1);

  this->dispatch_ci(group, hdr, config_details);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDetailsReply) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr auto pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7};
  constexpr auto target = 0x23_b7;
  constexpr auto data = std::array{'H'_b7, 'e'_b7, 'l'_b7, 'l'_b7, 'o'_b7};

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  using midi2::ci::profile_configuration::details_reply;

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, details_reply(config_.context, hdr,
                                             AllOf(Field("pid", &details_reply::pid, Eq(pid)),
                                                   Field("target", &details_reply::target, Eq(target)),
                                                   Field("data", &details_reply::data, ElementsAreArray(data)))))
      .Times(1);

  this->dispatch_ci(group, hdr, details_reply{pid, target, data});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileOn) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::profile_configuration::on on{
      .pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7},
      .num_channels = 23_b14,
  };

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, on(config_.context, hdr, on)).Times(1);

  this->dispatch_ci(group, hdr, on);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileOff) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr auto off =
      midi2::ci::profile_configuration::off{.pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7}};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, off(config_.context, hdr, off)).Times(1);

  this->dispatch_ci(group, hdr, off);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileEnabled) {
  using midi2::ci::profile_configuration::profile;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = broadcast_muid,
  };
  constexpr midi2::ci::profile_configuration::enabled enabled{
      .pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7},
      .num_channels = 0x1122_b14,
  };

  EXPECT_CALL(config_.profile, enabled(config_.context, hdr, enabled)).Times(1);

  this->dispatch_ci(0x01_u8, hdr, enabled);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDisabled) {
  using midi2::ci::profile_configuration::profile;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = broadcast_muid,
  };
  constexpr midi2::ci::profile_configuration::disabled disabled{
      .pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7},
      .num_channels = 0x123_b14,
  };
  EXPECT_CALL(config_.profile, disabled(config_.context, hdr, disabled)).Times(1);

  this->dispatch_ci(0x01_u8, hdr, disabled);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileSpecificData) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr auto pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7};
  constexpr auto data = std::array{'H'_b7, 'e'_b7, 'l'_b7, 'l'_b7, 'o'_b7};

  // clang-format on
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = broadcast_muid,
  };
  using midi2::ci::profile_configuration::specific_data;
  EXPECT_CALL(config_.profile, specific_data(config_.context, hdr,
                                             AllOf(Field("pid", &specific_data::pid, Eq(pid)),
                                                   Field("data", &specific_data::data, ElementsAreArray(data)))))
      .Times(1);

  this->dispatch_ci(group, hdr, specific_data{pid, data});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeCapabilities) {
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::property_exchange::capabilities caps{
      .num_simultaneous = 2_b7,
      .major_version = 3_b7,
      .minor_version = 4_b7,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, capabilities(config_.context, hdr, caps)).Times(1);

  this->dispatch_ci(group, hdr, caps);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeCapabilitiesReply) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::property_exchange::capabilities_reply caps{
      .num_simultaneous = 2_b7,
      .major_version = 3_b7,
      .minor_version = 4_b7,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, capabilities_reply(config_.context, hdr, caps)).Times(1);

  this->dispatch_ci(group, hdr, caps);
}

using namespace std::string_view_literals;
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeGetPropertyData) {
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::property_exchange::get g{
      .chunk = midi2::ci::property_exchange::chunk_info{2_b14, 1_b14},
      .request = 1_b7,
      .header = R"({"status":200})"sv,
      .data = {},
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, get(config_.context, hdr, g));

  this->dispatch_ci(group, hdr, g);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeGetPropertyDataReply) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  using get_reply = midi2::ci::property_exchange::get_reply;
  constexpr get_reply gr{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 1_b7,
      .header = R"({"status":200})"sv,
      .data = R"([{"resource":"DeviceInfo"},{"resource":"ChannelList"},{"resource":"CMList"}])"sv,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              get_reply(config_.context, hdr,
                        AllOf(Field("chunk", &get_reply::chunk, Eq(gr.chunk)),
                              Field("request", &get_reply::request, Eq(gr.request)),
                              Field("header", &get_reply::header, ElementsAreArray(gr.header)),
                              Field("data", &get_reply::data, ElementsAreArray(gr.data)))));

  this->dispatch_ci(group, hdr, gr);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSetPropertyData) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::set;
  constexpr set spd{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 1_b7,
      .header = R"({"resource":"X-ProgramEdit","resId":"abcd"})"sv,
      .data = R"({"name":"Violin 2","lfoSpeed":10,"lfoWaveform":"sine"})"sv,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              set(config_.context, hdr,
                  AllOf(Field("chunk", &set::chunk, Eq(spd.chunk)), Field("request", &set::request, Eq(spd.request)),
                        Field("header", &set::header, ElementsAreArray(spd.header)),
                        Field("data", &set::data, ElementsAreArray(spd.data)))));

  this->dispatch_ci(group, hdr, spd);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSetPropertyDataReply) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::set_reply;
  constexpr set_reply spd_reply{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 2_b7,
      .header = R"({"status":200})"sv,
      .data = {},
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              set_reply(config_.context, hdr,
                        AllOf(Field("chunk", &set_reply::chunk, Eq(spd_reply.chunk)),
                              Field("request", &set_reply::request, Eq(spd_reply.request)),
                              Field("header", &set_reply::header, ElementsAreArray(spd_reply.header)))));

  this->dispatch_ci(group, hdr, spd_reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSubscription) {
  constexpr auto group = 0x01_u8;
  constexpr auto destination = 0x0F_b7;

  constexpr auto header = R"({"command":"full","subscribeId":"sub32847623"})"sv;
  constexpr auto data = "multichannel"sv;

  constexpr struct header hdr {
    .device_id = destination, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::subscription;
  constexpr subscription sub{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 17_b7,
      .header = header,
      .data = data,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              subscription(config_.context, hdr,
                           AllOf(Field("chunk", &subscription::chunk, Eq(sub.chunk)),
                                 Field("request", &subscription::request, Eq(sub.request)),
                                 Field("header", &subscription::header, ElementsAreArray(header)),
                                 Field("data", &subscription::data, ElementsAreArray(data)))));

  this->dispatch_ci(group, hdr, sub);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSubscriptionReply) {
  constexpr auto group = 0x01_u8;

  constexpr auto header = R"({"status":200})"sv;
  constexpr auto data = "data"sv;

  constexpr struct header hdr {
    .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::subscription_reply;
  constexpr subscription_reply sub_reply{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 17_b7,
      .header = header,
      .data = data,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              subscription_reply(config_.context, hdr,
                                 AllOf(Field("chunk", &subscription_reply::chunk, Eq(sub_reply.chunk)),
                                       Field("request", &subscription_reply::request, Eq(sub_reply.request)),
                                       Field("header", &subscription_reply::header, ElementsAreArray(header)),
                                       Field("data", &subscription_reply::data, ElementsAreArray(data)))));

  this->dispatch_ci(group, hdr, sub_reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeNotify) {
  constexpr auto group = 0x01_u8;
  constexpr auto header = R"({"status":144})"sv;
  constexpr auto data = "data"sv;

  constexpr struct header hdr {
    .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::notify;
  constexpr notify note{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 1_b7,
      .header = header,
      .data = data,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, notify(config_.context, hdr,
                                                AllOf(Field("chunk", &notify::chunk, Eq(note.chunk)),
                                                      Field("request", &notify::request, Eq(note.request)),
                                                      Field("header", &notify::header, ElementsAreArray(header)),
                                                      Field("data", &notify::data, ElementsAreArray(data)))));

  this->dispatch_ci(group, hdr, note);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryCapabilities) {
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x7F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, capabilities(config_.context, hdr)).Times(1);

  this->dispatch_ci(group, hdr, midi2::ci::process_inquiry::capabilities{});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryCapabilitiesReply) {
  constexpr auto group = 0x01_u8;
  constexpr auto destination = 0x7F_b7;

  constexpr header hdr{
      .device_id = destination,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::process_inquiry::capabilities_reply reply{.features = 0b0101010_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, capabilities_reply(config_.context, hdr, reply)).Times(1);

  this->dispatch_ci(group, hdr, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReport) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x01_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
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
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report(config_.context, hdr, report)).Times(1);

  this->dispatch_ci(group, hdr, report);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReportReply) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x01_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
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
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report_reply(config_.context, hdr, reply)).Times(1);

  this->dispatch_ci(group, hdr, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReportEnd) {
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x01_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report_end(config_.context, hdr)).Times(1);

  this->dispatch_ci(group, hdr, midi2::ci::process_inquiry::midi_message_report_end{});
}

// This test simply gets ci_dispatcher to consume a random buffer.
void NeverCrashes(std::vector<std::byte> const &message) {
  // Ensure the top bit is clear.
  std::vector<std::byte> message2;
  message2.reserve(message.size());
  std::ranges::transform(message, std::back_inserter(message2), [](std::byte v) { return v & 0x7F_b; });
  struct empty {};
  static constexpr auto buffer_size = std::size_t{64};
  auto dispatcher = midi2::ci::make_function_dispatcher<empty, buffer_size>();
  dispatcher.config().system.on_check_muid([](empty, std::uint8_t, midi2::ci::muid) { return true; });
  std::ranges::for_each(message2, std::bind_front(&decltype(dispatcher)::dispatch, &dispatcher));
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
