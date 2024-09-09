// DUT
#include "midi2/ci_types.hpp"
#include "midi2/midiCIMessageCreate.hpp"
#include "midi2/midiCIProcessor.hpp"
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

std::ostream &operator<<(std::ostream &os, MIDICI const &ci);
std::ostream &operator<<(std::ostream &os, MIDICI const &ci) {
  return os << "{ umpGroup=" << static_cast<unsigned>(ci.umpGroup)
            << ", deviceId=" << static_cast<unsigned>(ci.deviceId) << ", ciType=" << static_cast<unsigned>(ci.ciType)
            << ", ciVer=" << static_cast<unsigned>(ci.ciVer) << ", remoteMUID=" << ci.remoteMUID
            << ", localMUID=" << ci.localMUID << " }";
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

using midi2::MIDICI;
using midi2::ci::byte_array_5;
using midi2::ci::from_le7;
using midi2::ci::property_exchange::chunk_info;

class mock_management_callbacks final : public midi2::management_callbacks {
public:
  MOCK_METHOD(bool, check_muid, (std::uint8_t group, std::uint32_t muid), (override));

  MOCK_METHOD(void, discovery, (MIDICI const &, midi2::ci::discovery const &), (override));
  MOCK_METHOD(void, discovery_reply, (MIDICI const &, midi2::ci::discovery_reply const &), (override));
  MOCK_METHOD(void, endpoint_info, (MIDICI const &, midi2::ci::endpoint_info const &), (override));
  MOCK_METHOD(void, endpoint_info_reply, (MIDICI const &, midi2::ci::endpoint_info_reply const &), (override));
  MOCK_METHOD(void, invalidate_muid, (MIDICI const &, midi2::ci::invalidate_muid const &), (override));
  MOCK_METHOD(void, ack, (MIDICI const &, midi2::ci::ack const &), (override));
  MOCK_METHOD(void, nak, (MIDICI const &, midi2::ci::nak const &), (override));

  MOCK_METHOD(void, buffer_overflow, (), (override));
  MOCK_METHOD(void, unknown_midici, (MIDICI const &), (override));
};

class mock_profile_callbacks final : public midi2::profile_callbacks {
public:
  MOCK_METHOD(void, inquiry, (MIDICI const &), (override));
  MOCK_METHOD(void, inquiry_reply, (MIDICI const &, midi2::ci::profile_configuration::inquiry_reply const &),
              (override));
  MOCK_METHOD(void, added, (MIDICI const &, midi2::ci::profile_configuration::added const &), (override));
  MOCK_METHOD(void, removed, (MIDICI const &, midi2::ci::profile_configuration::removed const &), (override));
  MOCK_METHOD(void, details, (MIDICI const &, midi2::ci::profile_configuration::details const &), (override));
  MOCK_METHOD(void, details_reply, (MIDICI const &, midi2::ci::profile_configuration::details_reply const &),
              (override));
  MOCK_METHOD(void, on, (MIDICI const &, midi2::ci::profile_configuration::on const &), (override));
  MOCK_METHOD(void, off, (MIDICI const &, midi2::ci::profile_configuration::off const &), (override));
  MOCK_METHOD(void, enabled, (MIDICI const &, midi2::ci::profile_configuration::enabled const &), (override));
  MOCK_METHOD(void, disabled, (MIDICI const &, midi2::ci::profile_configuration::disabled const &), (override));
  MOCK_METHOD(void, specific_data, (MIDICI const &, midi2::ci::profile_configuration::specific_data const &),
              (override));
};

using midi2::ci::property_exchange::property_exchange;

class mock_property_exchange_callbacks final : public midi2::property_exchange_callbacks {
public:
  MOCK_METHOD(void, capabilities, (MIDICI const &, midi2::ci::property_exchange::capabilities const &), (override));
  MOCK_METHOD(void, capabilities_reply, (MIDICI const &, midi2::ci::property_exchange::capabilities_reply const &),
              (override));

  MOCK_METHOD(void, get, (MIDICI const &, chunk_info const &, property_exchange const &), (override));
  MOCK_METHOD(void, get_reply, (MIDICI const &, chunk_info const &, property_exchange const &), (override));
  MOCK_METHOD(void, set, (MIDICI const &, chunk_info const &, property_exchange const &), (override));
  MOCK_METHOD(void, set_reply, (MIDICI const &, chunk_info const &, property_exchange const &), (override));

  MOCK_METHOD(void, subscription, (MIDICI const &, chunk_info const &, property_exchange const &), (override));
  MOCK_METHOD(void, subscription_reply, (MIDICI const &, chunk_info const &, property_exchange const &), (override));
  MOCK_METHOD(void, notify, (MIDICI const &, chunk_info const &, property_exchange const &), (override));
};

class mock_process_inquiry_callbacks final : public midi2::process_inquiry_callbacks {
public:
  MOCK_METHOD(void, capabilities, (MIDICI const &), (override));
  MOCK_METHOD(void, capabilities_reply, (MIDICI const &, midi2::ci::process_inquiry::capabilities_reply const &),
              (override));
  MOCK_METHOD(void, midi_message_report, (MIDICI const &, midi2::ci::process_inquiry::midi_message_report const &),
              (override));
  MOCK_METHOD(void, midi_message_report_end, (MIDICI const &), (override));
};

constexpr auto broadcast_muid = std::array{std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}};

class CIProcessor : public testing::Test {
public:
  CIProcessor()
      : processor_{std::ref(management_mocks_), std::ref(profile_mocks_), std::ref(pe_mocks_), std::ref(pi_mocks_)} {}

protected:
  mock_management_callbacks management_mocks_;
  mock_profile_callbacks profile_mocks_;
  mock_property_exchange_callbacks pe_mocks_;
  mock_process_inquiry_callbacks pi_mocks_;
  midi2::midiCIProcessor<mock_management_callbacks &, mock_profile_callbacks &, mock_property_exchange_callbacks &,
                         mock_process_inquiry_callbacks &>
      processor_;
};

TEST_F(CIProcessor, Empty) {
  mock_management_callbacks mocks;
  midi2::midiCIProcessor ci{std::ref(mocks)};
  ci.processMIDICI(std::byte{0});
}

TEST_F(CIProcessor, DiscoveryV1) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto manufacturer = std::array{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}};
  constexpr auto family = std::array{std::byte{0x67}, std::byte{0x79}};
  constexpr auto model = std::array{std::byte{0x6B}, std::byte{0x5D}};
  constexpr auto version = std::array{std::byte{0x4E}, std::byte{0x3C}, std::byte{0x2A}, std::byte{0x18}};
  constexpr auto capability = std::byte{0x7F};
  constexpr auto max_sysex_size = std::array{std::byte{0x76}, std::byte{0x54}, std::byte{0x32}, std::byte{0x10}};

  // clang-format off
  std::array const message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x70}, // Universal System Exclusive Sub-ID#2: Discovery
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, // 4 bytes Source MUID (LSB first)
    std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, // Destination MUID (LSB first) (to Broadcast MUID)
    manufacturer[0], manufacturer[1], manufacturer[2], // 3 bytes Device Manufacturer (System Exclusive ID Number)
    family[0], family[1],  // 2 bytes Device Family (LSB first)
    model[0], model[1], // 2 bytes Device Family Model Number (LSB first)
    version[0], version[1], version[2], version[3],  // 4 bytes Software Revision Level
    capability, // 1 byte Capability Inquiry Category Supported (bitmap)
    max_sysex_size[0], max_sysex_size[1], max_sysex_size[2], max_sysex_size[3], // Maximum sysex message size (LSB first)

    std::byte{0}, // a stray extra byte.
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = 0xFF;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::ci_message::discovery;
  midici.ciVer = 1;
  midici.remoteMUID = 0;
  midici.localMUID = midi2::M2_CI_BROADCAST;

  midi2::ci::discovery discovery;
  discovery.manufacturer = midi2::ci::from_array(manufacturer);
  discovery.family = from_le7(family);
  discovery.model = from_le7(model);
  discovery.version = midi2::ci::from_array(version);
  discovery.capability = static_cast<std::uint8_t>(capability);
  discovery.max_sysex_size = from_le7(max_sysex_size);

  EXPECT_CALL(management_mocks_, discovery(midici, discovery)).Times(1);
  processor_.startSysex7(0xFF, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));

  // Test create_message()
  std::vector<std::byte> v{std::size_t{256}};
  auto last = midi2::ci::create_message(std::begin(v), std::end(v), midici, discovery);
  auto const dist = std::distance(v.begin(), last);
  ASSERT_GE(dist, 0);
  v.resize(static_cast<std::size_t>(dist));
  EXPECT_THAT(v, testing::ElementsAreArray(std::begin(message), std::end(message) - 1));
}

TEST_F(CIProcessor, DiscoveryV2) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto manufacturer = std::array{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}};
  constexpr auto family = std::array{std::byte{0x67}, std::byte{0x79}};
  constexpr auto model = std::array{std::byte{0x6B}, std::byte{0x5D}};
  constexpr auto version = std::array{std::byte{0x4E}, std::byte{0x3C}, std::byte{0x2A}, std::byte{0x18}};
  constexpr auto capability = std::byte{0x7F};
  constexpr auto max_sysex_size = std::array{std::byte{0x76}, std::byte{0x54}, std::byte{0x32}, std::byte{0x10}};
  constexpr auto output_path_id = std::byte{0x71};

  // clang-format off
  std::array const message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x70}, // Universal System Exclusive Sub-ID#2: Discovery
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, // 4 bytes Source MUID (LSB first)
    std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, // Destination MUID (LSB first) (to Broadcast MUID)
    manufacturer[0], manufacturer[1], manufacturer[2], // 3 bytes Device Manufacturer (System Exclusive ID Number)
    family[0], family[1],  // 2 bytes Device Family (LSB first)
    model[0], model[1], // 2 bytes Device Family Model Number (LSB first)
    version[0], version[1], version[2], version[3],  // 4 bytes Software Revision Level
    capability, // 1 byte Capability Inquiry Category Supported (bitmap)
    max_sysex_size[0], max_sysex_size[1], max_sysex_size[2], max_sysex_size[3], // Maximum sysex message size (LSB first)
    output_path_id, // [1] initiator's output path ID

    std::byte{0}, // a stray extra byte.
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = 0xFF;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::ci_message::discovery;
  midici.ciVer = 2;
  midici.remoteMUID = 0;
  midici.localMUID = midi2::M2_CI_BROADCAST;

  midi2::ci::discovery discovery;
  discovery.manufacturer = midi2::ci::from_array(manufacturer);
  discovery.family = from_le7(family);
  discovery.model = from_le7(model);
  discovery.version = midi2::ci::from_array(version);
  discovery.capability = static_cast<std::uint8_t>(capability);
  discovery.max_sysex_size = from_le7(max_sysex_size);
  discovery.output_path_id = static_cast<std::uint8_t>(output_path_id);

  EXPECT_CALL(management_mocks_, discovery(midici, discovery)).Times(1);
  processor_.startSysex7(0xFF, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));

  // Test create_message()
  std::vector<std::byte> v{std::size_t{256}};
  auto last = midi2::ci::create_message(std::begin(v), std::end(v), midici, discovery);
  auto const dist = std::distance(v.begin(), last);
  ASSERT_GE(dist, 0);
  v.resize(static_cast<std::size_t>(dist));
  EXPECT_THAT(v, testing::ElementsAreArray(std::begin(message), std::end(message) - 1));
}

TEST_F(CIProcessor, DiscoveryReplyV2) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto manufacturer = std::array{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}};
  constexpr auto family = std::array{std::byte{0x67}, std::byte{0x79}};
  constexpr auto model = std::array{std::byte{0x5B}, std::byte{0x4D}};
  constexpr auto version = std::array{std::byte{0x7E}, std::byte{0x6C}, std::byte{0x5A}, std::byte{0x48}};
  constexpr auto capability = std::byte{0x7F};
  constexpr auto max_sysex_size = std::array{std::byte{0x76}, std::byte{0x54}, std::byte{0x32}, std::byte{0x10}};
  constexpr auto output_path_id = std::byte{0x71};
  constexpr auto function_block = std::byte{0x32};

  // clang-format off
  std::array const message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x71}, // Universal System Exclusive Sub-ID#2: Reply to Discovery
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}, // 4 bytes Source MUID (LSB first)
    std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, // Destination MUID (LSB first) (to Broadcast MUID)
    manufacturer[0], manufacturer[1], manufacturer[2], // 3 bytes Device Manufacturer (System Exclusive ID Number)
    family[0], family[1],  // 2 bytes Device Family (LSB first)
    model[0], model[1], // 2 bytes Device Family Model Number (LSB first)
    version[0], version[1], version[2], version[3],  // 4 bytes Software Revision Level
    capability, // 1 byte Capability Inquiry Category Supported (bitmap)
    max_sysex_size[0], max_sysex_size[1], max_sysex_size[2], max_sysex_size[3], // Maximum sysex message size (LSB first)
    output_path_id, // [1] initiator's output path ID
    function_block, // [1] function block

    std::byte{0}, // a stray extra byte.
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = 0xFF;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::ci_message::discovery_reply;
  midici.ciVer = 2;
  midici.remoteMUID = 0;
  midici.localMUID = midi2::M2_CI_BROADCAST;

  midi2::ci::discovery_reply reply;
  reply.manufacturer = midi2::ci::from_array(manufacturer);
  reply.family = from_le7(family);
  reply.model = from_le7(model);
  reply.version = midi2::ci::from_array(version);
  reply.capability = static_cast<std::uint8_t>(capability);
  reply.max_sysex_size = from_le7(max_sysex_size);
  reply.output_path_id = static_cast<std::uint8_t>(output_path_id);
  reply.function_block = static_cast<std::uint8_t>(function_block);

  EXPECT_CALL(management_mocks_, discovery_reply(midici, reply)).Times(1);
  processor_.startSysex7(0xFF, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));

  // Test create_message()
  std::vector<std::byte> v{std::size_t{256}};
  auto last = midi2::ci::create_message(std::begin(v), std::end(v), midici, reply);
  auto const dist = std::distance(v.begin(), last);
  ASSERT_GE(dist, 0);
  v.resize(static_cast<std::size_t>(dist));
  EXPECT_THAT(v, testing::ElementsAreArray(std::begin(message), std::end(message) - 1));
}

TEST_F(CIProcessor, EndpointInfo) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto status = std::uint8_t{0b0101010};
  constexpr std::array const sender_muid{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr std::array const receiver_muid{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  // clang-format off
  std::array const message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x72}, // Universal System Exclusive Sub-ID#2: Endpoint Information
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    std::byte{status}, // Status

    std::byte{0}, // a stray extra byte.
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::ci_message::endpoint_info;
  midici.ciVer = 1;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  midi2::ci::endpoint_info endpoint_info;
  endpoint_info.status = status;

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(management_mocks_, endpoint_info(midici, endpoint_info)).Times(1);
  processor_.startSysex7(group, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));

  // Test create_message()
  std::vector<std::byte> v{std::size_t{256}};
  auto last = midi2::ci::create_message(std::begin(v), std::end(v), midici, endpoint_info);
  auto const dist = std::distance(v.begin(), last);
  ASSERT_GE(dist, 0);
  v.resize(static_cast<std::size_t>(dist));
  EXPECT_THAT(v, testing::ElementsAreArray(std::begin(message), std::end(message) - 1));
}

TEST_F(CIProcessor, EndpointInfoReply) {
  constexpr auto group = std::uint8_t{0x71};
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto status = std::byte{0b0101010};
  constexpr auto length = std::array{std::byte{0x08}, std::byte{0x00}};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};
  constexpr auto information = std::array{
      std::byte{2},  std::byte{3},  std::byte{5},  std::byte{7},  // Information data
      std::byte{11}, std::byte{13}, std::byte{17}, std::byte{19},
  };
  ASSERT_EQ(from_le7(length), information.size());
  // clang-format off
  std::array const message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x73}, // Universal System Exclusive Sub-ID#2: Reply to Endpoint Information
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    status, // Status
    length[0], length[1], // Length of following data (LSB first)
    information[0], information[1], information[2], information[3],
    information[4], information[5],  information[6], information[7],

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::ci_message::endpoint_info_reply;
  midici.ciVer = 1;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(management_mocks_,
              endpoint_info_reply(midici, AllOf(Field("status", &midi2::ci::endpoint_info_reply::status, Eq(status)),
                                                Field("information", &midi2::ci::endpoint_info_reply::information,
                                                      ElementsAreArray(information)))))
      .Times(1);

  processor_.startSysex7(group, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));

  // Test create_message()
  constexpr auto expected_size = std::size_t{message.size() - 1};
  std::vector<std::byte> v{expected_size + 1};
  midi2::ci::endpoint_info_reply reply;
  reply.status = status;
  reply.information = std::span{information};
  auto last = midi2::ci::create_message(std::begin(v), std::end(v), midici, reply);
  EXPECT_EQ(std::distance(v.begin(), last), expected_size);
  v.resize(expected_size);
  EXPECT_THAT(v, testing::ElementsAreArray(std::begin(message), std::end(message) - 1));
}

TEST_F(CIProcessor, InvalidateMuid) {
  constexpr auto group = std::uint8_t{0x71};
  constexpr auto device_id = std::byte{0x7F};
  constexpr std::array const sender_muid{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr std::array const receiver_muid{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};
  constexpr std::array const target_muid{std::byte{0x21}, std::byte{0x43}, std::byte{0x75}, std::byte{0x71}};

  // clang-format off
  std::array const message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x7E}, // Universal System Exclusive Sub-ID#2: Invalidate MUID
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    target_muid[0], target_muid[1], target_muid[2], target_muid[3], // Target MUID (the MUID to invalidate) (LSB first)

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::ci_message::invalidate_muid;
  midici.ciVer = 1;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  midi2::ci::invalidate_muid invalidate_muid;
  invalidate_muid.target_muid = from_le7(target_muid);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(management_mocks_, invalidate_muid(midici, invalidate_muid)).Times(1);

  processor_.startSysex7(group, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));

  // Test create_message()
  std::vector<std::byte> v{std::size_t{256}};
  auto last = midi2::ci::create_message(std::begin(v), std::end(v), midici, invalidate_muid);
  auto const dist = std::distance(v.begin(), last);
  ASSERT_GE(dist, 0);
  v.resize(static_cast<std::size_t>(dist));
  EXPECT_THAT(v, testing::ElementsAreArray(std::begin(message), std::end(message) - 1));
}

TEST_F(CIProcessor, Ack) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  constexpr auto original_id = std::byte{0x34};
  constexpr auto ack_status_code = std::byte{0x00};
  constexpr auto ack_status_data = std::byte{0x7F};
  constexpr auto ack_details =
      std::array{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
  constexpr auto text_length = std::array{std::byte{0x05}, std::byte{0x00}};
  constexpr auto text = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x7D}, // Universal System Exclusive Sub-ID#2: MIDI-CI ACK
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    original_id, // Original transaction sub-ID#2 classification
    ack_status_code, // ACK Status Code
    ack_status_data, // ACK Status Data
    ack_details[0], ack_details[1], ack_details[2], ack_details[3], ack_details[4],
    text_length[0], text_length[1],
    text[0], text[1], text[2], text[3], text[4],

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::ci_message::ack;
  midici.ciVer = 1;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(
      management_mocks_,
      ack(midici,
          AllOf(Field("original_id", &midi2::ci::ack::original_id, Eq(static_cast<std::uint8_t>(original_id))),
                Field("status_code", &midi2::ci::ack::status_code, Eq(static_cast<std::uint8_t>(ack_status_code))),
                Field("status_data", &midi2::ci::ack::status_data, Eq(static_cast<std::uint8_t>(ack_status_data))),
                Field("details", &midi2::ci::ack::details, ElementsAreArray(ack_details)),
                Field("message", &midi2::ci::ack::message, ElementsAreArray(text)))))
      .Times(1);

  processor_.startSysex7(group, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));

  // Test create_message()
  constexpr auto expected_size = std::size_t{message.size() - 1};
  std::vector<std::byte> v{expected_size + 1};
  midi2::ci::ack ack;
  ack.original_id = static_cast<std::uint8_t>(original_id);
  ack.status_code = static_cast<std::uint8_t>(ack_status_code);
  ack.status_data = static_cast<std::uint8_t>(ack_status_data);
  ack.details = ack_details;
  ack.message = std::span{text};

  auto last = midi2::ci::create_message(std::begin(v), std::end(v), midici, ack);
  EXPECT_EQ(std::distance(v.begin(), last), expected_size);
  v.resize(expected_size);
  EXPECT_THAT(v, testing::ElementsAreArray(std::begin(message), std::end(message) - 1));
}

TEST_F(CIProcessor, AckMessageTooLong) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  constexpr auto original_id = std::byte{0x34};
  constexpr auto ack_status_code = std::byte{0x00};
  constexpr auto ack_status_data = std::byte{0x7F};
  constexpr auto ack_details =
      std::array{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
  constexpr auto text_length = std::array{std::byte{0x02}, std::byte{0x7F}};

  // clang-format off
  std::vector<std::byte> message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x7D}, // Universal System Exclusive Sub-ID#2: MIDI-CI ACK
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    original_id, // Originl transaciton sub-ID#2 classification
    ack_status_code, // ACK Status Code
    ack_status_data, // ACK Status Data
    ack_details[0], ack_details[1], ack_details[2], ack_details[3], ack_details[4],
    text_length[0], text_length[1],
  };
  message.insert(std::end(message), from_le7(text_length), std::byte{0});
  // clang-format on

  EXPECT_CALL(management_mocks_, check_muid(group, from_le7(receiver_muid))).WillRepeatedly(Return(true));
  EXPECT_CALL(management_mocks_, buffer_overflow()).Times(1);

  processor_.startSysex7(group, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, NakV1) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x7F}, // Universal System Exclusive Sub-ID#2: MIDI-CI NAK
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::ci_message::nak;
  midici.ciVer = 1;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(management_mocks_, nak(midici, AllOf(Field("original_id", &midi2::ci::nak::original_id, Eq(0)),
                                                   Field("status_code", &midi2::ci::nak::status_code, Eq(0)),
                                                   Field("status_data", &midi2::ci::nak::status_data, Eq(0)),
                                                   Field("details", &midi2::ci::nak::details, Eq(byte_array_5{})),
                                                   Field("message", &midi2::ci::nak::message, IsEmpty()))))
      .Times(1);

  processor_.startSysex7(group, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));

  // Test create_message()
  constexpr auto expected_size = std::size_t{message.size() - 1};
  std::vector<std::byte> v{expected_size + 1};
  midi2::ci::nak nak;
  auto last = midi2::ci::create_message(std::begin(v), std::end(v), midici, nak);
  EXPECT_EQ(std::distance(v.begin(), last), expected_size);
  v.resize(expected_size);
  EXPECT_THAT(v, testing::ElementsAreArray(std::begin(message), std::end(message) - 1));
}

TEST_F(CIProcessor, NakV2) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  constexpr auto original_id = std::byte{0x34};
  constexpr auto nak_status_code = std::byte{0x00};
  constexpr auto nak_status_data = std::byte{0x7F};
  constexpr auto nak_details =
      std::array{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
  constexpr auto text_length = std::array{std::byte{0x05}, std::byte{0x00}};
  constexpr auto text = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x7F}, // Universal System Exclusive Sub-ID#2: MIDI-CI NAK
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    original_id, // Originl transaciton sub-ID#2 classification
    nak_status_code, // NAK Status Code
    nak_status_data, // NAK Status Data
    nak_details[0], nak_details[1], nak_details[2], nak_details[3], nak_details[4],
    text_length[0], text_length[1],
    text[0], text[1], text[2], text[3], text[4],

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::ci_message::nak;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(
      management_mocks_,
      nak(midici,
          AllOf(Field("original_id", &midi2::ci::nak::original_id, Eq(static_cast<std::uint8_t>(original_id))),
                Field("status_code", &midi2::ci::nak::status_code, Eq(static_cast<std::uint8_t>(nak_status_code))),
                Field("status_data", &midi2::ci::nak::status_data, Eq(static_cast<std::uint8_t>(nak_status_data))),
                Field("details", &midi2::ci::nak::details, ElementsAreArray(nak_details)),
                Field("message", &midi2::ci::nak::message, ElementsAreArray(text)))))
      .Times(1);

  processor_.startSysex7(group, device_id);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));

  // Test create_message()
  constexpr auto expected_size = std::size_t{message.size() - 1};
  std::vector<std::byte> v{expected_size + 1};
  midi2::ci::nak nak;
  nak.original_id = static_cast<std::uint8_t>(original_id);
  nak.status_code = static_cast<std::uint8_t>(nak_status_code);
  nak.status_data = static_cast<std::uint8_t>(nak_status_data);
  nak.details = nak_details;
  nak.message = std::span{text};

  auto last = midi2::ci::create_message(std::begin(v), std::end(v), midici, nak);
  EXPECT_EQ(std::distance(v.begin(), last), expected_size);
  v.resize(expected_size);
  EXPECT_THAT(v, testing::ElementsAreArray(std::begin(message), std::end(message) - 1));
}

TEST_F(CIProcessor, ProfileInquiry) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x20}, // Universal System Exclusive Sub-ID#2: Profile Inquiry
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)

    //std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_inquiry;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks_, inquiry(midici)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileInquiryReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  constexpr auto enabled = std::array<byte_array_5, 2>{
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}},
      byte_array_5{std::byte{0x76}, std::byte{0x65}, std::byte{0x54}, std::byte{0x43}, std::byte{0x32}},
  };
  constexpr auto disabled = std::array<byte_array_5, 1>{
      byte_array_5{std::byte{0x71}, std::byte{0x61}, std::byte{0x51}, std::byte{0x41}, std::byte{0x31}},
  };
  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x21}, // Universal System Exclusive Sub-ID#2: Profile Inquiry Reply
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    std::byte{2}, std::byte{0},
    enabled[0][0], enabled[0][1], enabled[0][2], enabled[0][3], enabled[0][4],
    enabled[1][0], enabled[1][1], enabled[1][2], enabled[1][3], enabled[1][4],
    std::byte{1}, std::byte{0},
    disabled[0][0], disabled[0][1], disabled[0][2], disabled[0][3], disabled[0][4],

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_inquiry_reply;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  using midi2::ci::profile_configuration::inquiry_reply;
  EXPECT_CALL(profile_mocks_,
              inquiry_reply(midici, AllOf(Field("enabled", &inquiry_reply::enabled, ElementsAreArray(enabled)),
                                          Field("disabled", &inquiry_reply::disabled, ElementsAreArray(disabled)))))
      .Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileAdded) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x26}, // Universal System Exclusive Sub-ID#2: Profile Added Report
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid[0], broadcast_muid[1], broadcast_muid[2], broadcast_muid[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile being added

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_added;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(broadcast_muid);

  midi2::ci::profile_configuration::added added;
  added.pid = pid;
  EXPECT_CALL(profile_mocks_, added(midici, added)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileRemoved) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x27}, // Universal System Exclusive Sub-ID#2: Profile Removed Report
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid[0], broadcast_muid[1], broadcast_muid[2], broadcast_muid[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile being removed

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_removed;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(broadcast_muid);

  midi2::ci::profile_configuration::removed removed;
  removed.pid = pid;

  EXPECT_CALL(profile_mocks_, removed(midici, removed)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileDetailsInquiry) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x28}, // Universal System Exclusive Sub-ID#2: Profile Details Inquiry
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    std::byte{0x23}, // Inquiry target

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_details_inquiry;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::profile_configuration::details inquiry;
  inquiry.pid = pid;
  inquiry.target = 0x23;

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks_, details(midici, inquiry)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileDetailsReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto data_length = std::array{std::byte{0x05}, std::byte{0x00}};
  constexpr auto data = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x29}, // Universal System Exclusive Sub-ID#2: Profile Details Reply
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    std::byte{0x23}, // Inquiry target
    data_length[0], data_length[1], // Inquiry target data length (LSB first)
    data[0], data[1], data[2], data[3], data[4],

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_details_reply;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  using midi2::ci::profile_configuration::details_reply;

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks_, details_reply(midici, AllOf(Field("pid", &details_reply::pid, Eq(pid)),
                                                          Field("target", &details_reply::target, Eq(0x23)),
                                                          Field("data", &details_reply::data, ElementsAreArray(data)))))
      .Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileOn) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto channels = std::array{std::byte{0x23}, std::byte{0x00}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x22}, // Universal System Exclusive Sub-ID#2: Set Profile On
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    channels[0], channels[1], // Number of channels

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_set_on;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::profile_configuration::on on;
  on.pid = pid;
  on.num_channels = from_le7(channels);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks_, on(midici, on)).Times(1);
  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileOff) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto reserved = std::array{std::byte{0x00}, std::byte{0x00}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x23}, // Universal System Exclusive Sub-ID#2: Set Profile Off
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    reserved[0], reserved[1], // Number of channels

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_set_off;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::profile_configuration::off off;
  off.pid = pid;

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks_, off(midici, off)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileEnabled) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto num_channels = std::array{std::byte{0x22}, std::byte{0x11}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x24}, // Universal System Exclusive Sub-ID#2: Profile Enabled Report
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid[0], broadcast_muid[1], broadcast_muid[2], broadcast_muid[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    num_channels[0], num_channels[1], // Number of channels

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_enabled;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(broadcast_muid);

  midi2::ci::profile_configuration::enabled enabled;
  enabled.pid = pid;
  enabled.num_channels = from_le7(num_channels);

  EXPECT_CALL(profile_mocks_, enabled(midici, enabled)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileDisabled) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto num_channels = std::array{std::byte{0x22}, std::byte{0x11}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x25}, // Universal System Exclusive Sub-ID#2: Profile Disabled Report
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid[0], broadcast_muid[1], broadcast_muid[2], broadcast_muid[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    num_channels[0], num_channels[1], // Number of channels

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_disabled;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(broadcast_muid);

  midi2::ci::profile_configuration::disabled disabled;
  disabled.pid = pid;
  disabled.num_channels = from_le7(num_channels);

  EXPECT_CALL(profile_mocks_, disabled(midici, disabled)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProfileSpecificData) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto length = std::array{std::byte{0x05}, std::byte{0x00}};
  constexpr auto data = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x2F}, // Universal System Exclusive Sub-ID#2: Profile Specific Data
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid[0], broadcast_muid[1], broadcast_muid[2], broadcast_muid[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    length[0], length[1], // Length of Following Profile Specific Data (LSB first)
    data[0], data[1], data[2], data[3], data[4],

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::profile_specific_data;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(broadcast_muid);

  using midi2::ci::profile_configuration::specific_data;
  EXPECT_CALL(profile_mocks_, specific_data(midici, AllOf(Field("pid", &specific_data::pid, Eq(pid)),
                                                          Field("data", &specific_data::data, ElementsAreArray(data)))))
      .Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, PropertyExchangeCapabilities) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x30}, // Universal System Exclusive Sub-ID#2: Inquiry: Property Data Exchange Capabilities
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    std::byte{0x02}, // Number of Simultaneous Property Exchange Requests Supported
    std::byte{0x03}, // Property Exchange Major Version
    std::byte{0x04}, // Property Exchange Minor Version

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pe_capability;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::property_exchange::capabilities caps;
  caps.num_simultaneous = 2;
  caps.major_version = 3;
  caps.minor_version = 4;

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(pe_mocks_, capabilities(midici, caps)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, PropertyExchangeCapabilitiesReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  // clang-format off
  constexpr std::array message{
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x31}, // Universal System Exclusive Sub-ID#2: Inquiry: Property Data Exchange Capabilities
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    std::byte{0x02}, // Number of Simultaneous Property Exchange Requests Supported
    std::byte{0x03}, // Property Exchange Major Version
    std::byte{0x04}, // Property Exchange Minor Version

    std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pe_capability_reply;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::property_exchange::capabilities_reply caps;
  caps.num_simultaneous = 2;
  caps.major_version = 3;
  caps.minor_version = 4;

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(pe_mocks_, capabilities_reply(midici, caps)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

using namespace std::string_view_literals;

TEST_F(CIProcessor, PropertyExchangeGetPropertyData) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  constexpr auto request_id = std::byte{1};

  constexpr auto header_size = std::array{std::byte{14}, std::byte{0}};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto property_data_size = std::array{std::byte{0}, std::byte{0}};

  auto const header = R"({"status":200})"sv;

  // clang-format off
  constexpr std::array ci_prolog {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x34}, // Universal System Exclusive Sub-ID#2: Inquiry: Get Property Data
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    request_id,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  std::vector<std::byte> message;
  // The standard MIDI CI header
  auto out = std::ranges::copy(ci_prolog, std::back_inserter(message)).out;
  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  out = std::ranges::copy(header_size, out).out;
  // Header Body
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Property Length
  ASSERT_EQ(from_le7(property_data_size), 0);
  out = std::ranges::copy(property_data_size, out).out;

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pe_get;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::property_exchange::chunk_info chunk_info;
  chunk_info.number_of_chunks = from_le7(total_chunks);
  chunk_info.chunk_number = from_le7(chunk_number);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(pe_mocks_,
              get(midici, chunk_info,
                  AllOf(Field("request_id", &property_exchange::request_id, Eq(static_cast<std::uint8_t>(request_id))),
                        Field("header", &property_exchange::header, ElementsAreArray(header)),
                        Field("data", &property_exchange::data, IsEmpty()))));

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, PropertyExchangeGetPropertyDataReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  constexpr auto request_id = std::byte{1};

  constexpr auto header_size = std::array{std::byte{14}, std::byte{0}};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto property_data_size = std::array{std::byte{76}, std::byte{0}};

  auto const header = R"({"status":200})"sv;
  auto const data = R"([{"resource":"DeviceInfo"},{"resource":"ChannelList"},{"resource":"CMList"}])"sv;

  // clang-format off
  constexpr std::array ci_prolog {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x35}, // Universal System Exclusive Sub-ID#2: Inquiry: Reply to Get Property Data
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    request_id,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  std::vector<std::byte> message;
  // The standard MIDI CI header
  auto out = std::ranges::copy(ci_prolog, std::back_inserter(message)).out;
  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  out = std::ranges::copy(header_size, out).out;
  // Header Body
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Property Length
  ASSERT_EQ(from_le7(property_data_size), data.length());
  out = std::ranges::copy(property_data_size, out).out;
  // Property Data
  std::ranges::copy(std::views::transform(data, as_byte), out);

  message.push_back(std::byte{0});  // Stray extra byte.

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pe_get_reply;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::property_exchange::chunk_info chunk_info;
  chunk_info.number_of_chunks = from_le7(total_chunks);
  chunk_info.chunk_number = from_le7(chunk_number);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));

  EXPECT_CALL(
      pe_mocks_,
      get_reply(midici, chunk_info,
                AllOf(Field("request_id", &property_exchange::request_id, Eq(static_cast<std::uint8_t>(request_id))),
                      Field("header", &property_exchange::header, ElementsAreArray(header)),
                      Field("data", &property_exchange::data, ElementsAreArray(data)))));

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, PropertyExchangeSetPropertyData) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  constexpr auto request_id = std::byte{1};

  constexpr auto header_size = std::array{std::byte{14}, std::byte{0}};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto property_data_size = std::array{std::byte{0}, std::byte{0}};

  auto const header = R"({"status":200})"sv;

  // clang-format off
  constexpr std::array ci_prolog {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x36}, // Universal System Exclusive Sub-ID#2: Inquiry: Set Property Data
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    request_id,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  std::vector<std::byte> message;
  // The standard MIDI CI header
  auto out = std::ranges::copy(ci_prolog, std::back_inserter(message)).out;
  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  out = std::ranges::copy(header_size, out).out;
  // Header Body
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Property Length
  ASSERT_EQ(from_le7(property_data_size), 0);
  out = std::ranges::copy(property_data_size, out).out;

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pe_set;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::property_exchange::chunk_info chunk_info;
  chunk_info.number_of_chunks = from_le7(total_chunks);
  chunk_info.chunk_number = from_le7(chunk_number);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(pe_mocks_,
              set(midici, chunk_info,
                  AllOf(Field("request_id", &property_exchange::request_id, Eq(static_cast<std::uint8_t>(request_id))),
                        Field("header", &property_exchange::header, ElementsAreArray(header)),
                        Field("data", &property_exchange::data, IsEmpty()))));

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, PropertyExchangeSetPropertyDataReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  constexpr auto request_id = std::byte{1};

  constexpr auto header_size = std::array{std::byte{14}, std::byte{0}};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto property_data_size = std::array{std::byte{0}, std::byte{0}};

  auto const header = R"({"status":200})"sv;

  // clang-format off
  constexpr std::array ci_prolog {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x37}, // Universal System Exclusive Sub-ID#2: Inquiry: Reply to Set Property Data
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    request_id,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  std::vector<std::byte> message;
  // The standard MIDI CI header
  auto out = std::ranges::copy(ci_prolog, std::back_inserter(message)).out;
  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  out = std::ranges::copy(header_size, out).out;
  // Header Body
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Property Length
  ASSERT_EQ(from_le7(property_data_size), 0);
  out = std::ranges::copy(property_data_size, out).out;

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pe_set_reply;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::property_exchange::chunk_info chunk_info;
  chunk_info.number_of_chunks = from_le7(total_chunks);
  chunk_info.chunk_number = from_le7(chunk_number);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));

  EXPECT_CALL(
      pe_mocks_,
      set_reply(midici, chunk_info,
                AllOf(Field("request_id", &property_exchange::request_id, Eq(static_cast<std::uint8_t>(request_id))),
                      Field("header", &property_exchange::header, ElementsAreArray(header)),
                      Field("data", &property_exchange::data, IsEmpty()))));

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, PropertyExchangeSubscription) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  constexpr auto request_id = std::byte{1};

  constexpr auto header_size = std::array{std::byte{14}, std::byte{0}};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto property_data_size = std::array{std::byte{0}, std::byte{0}};

  auto const header = R"({"status":200})"sv;

  // clang-format off
  constexpr std::array ci_prolog {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x38}, // Universal System Exclusive Sub-ID#2: Subscription
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    request_id,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  std::vector<std::byte> message;
  // The standard MIDI CI header
  auto out = std::ranges::copy(ci_prolog, std::back_inserter(message)).out;
  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  out = std::ranges::copy(header_size, out).out;
  // Header Body
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Property Length
  ASSERT_EQ(from_le7(property_data_size), 0);
  out = std::ranges::copy(property_data_size, out).out;

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pe_sub;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::property_exchange::chunk_info chunk_info;
  chunk_info.number_of_chunks = from_le7(total_chunks);
  chunk_info.chunk_number = from_le7(chunk_number);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(
      pe_mocks_,
      subscription(midici, chunk_info,
                   AllOf(Field("request_id", &property_exchange::request_id, Eq(static_cast<std::uint8_t>(request_id))),
                         Field("header", &property_exchange::header, ElementsAreArray(header)),
                         Field("data", &property_exchange::data, IsEmpty()))));

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, PropertyExchangeSubscriptionReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  constexpr auto request_id = std::byte{1};

  constexpr auto header_size = std::array{std::byte{14}, std::byte{0}};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto property_data_size = std::array{std::byte{0}, std::byte{0}};

  auto const header = R"({"status":200})"sv;

  // clang-format off
  constexpr std::array ci_prolog {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x39}, // Universal System Exclusive Sub-ID#2: Subscription Reply
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    request_id,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  std::vector<std::byte> message;
  // The standard MIDI CI header
  auto out = std::ranges::copy(ci_prolog, std::back_inserter(message)).out;
  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  out = std::ranges::copy(header_size, out).out;
  // Header Body
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Property Length
  ASSERT_EQ(from_le7(property_data_size), 0);
  out = std::ranges::copy(property_data_size, out).out;

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pe_sub_reply;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::property_exchange::chunk_info chunk_info;
  chunk_info.number_of_chunks = from_le7(total_chunks);
  chunk_info.chunk_number = from_le7(chunk_number);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(pe_mocks_, subscription_reply(midici, chunk_info,
                                            AllOf(Field("request_id", &property_exchange::request_id,
                                                        Eq(static_cast<std::uint8_t>(request_id))),
                                                  Field("header", &property_exchange::header, ElementsAreArray(header)),
                                                  Field("data", &property_exchange::data, IsEmpty()))));

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, PropertyExchangeNotify) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x0F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  constexpr auto request_id = std::byte{1};

  constexpr auto header_size = std::array{std::byte{14}, std::byte{0}};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto property_data_size = std::array{std::byte{0}, std::byte{0}};

  auto const header = R"({"status":200})"sv;

  // clang-format off
  constexpr std::array ci_prolog {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x3F}, // Universal System Exclusive Sub-ID#2: Notify
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    request_id,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  std::vector<std::byte> message;
  // The standard MIDI CI header
  auto out = std::ranges::copy(ci_prolog, std::back_inserter(message)).out;
  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  out = std::ranges::copy(header_size, out).out;
  // Header Body
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Property Length
  ASSERT_EQ(from_le7(property_data_size), 0);
  out = std::ranges::copy(property_data_size, out).out;

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pe_notify;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::property_exchange::chunk_info chunk_info;
  chunk_info.number_of_chunks = from_le7(total_chunks);
  chunk_info.chunk_number = from_le7(chunk_number);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(
      pe_mocks_,
      notify(midici, chunk_info,
             AllOf(Field("request_id", &property_exchange::request_id, Eq(static_cast<std::uint8_t>(request_id))),
                   Field("header", &property_exchange::header, ElementsAreArray(header)),
                   Field("data", &property_exchange::data, IsEmpty()))));

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProcessInquiryCapabilities) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x7F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  // clang-format off
  constexpr std::array message {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x40}, // Universal System Exclusive Sub-ID#2: Inquiry: Process Inquiry Capabilities
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)
  };
  // clang-format on

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pi_capability;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(pi_mocks_, capabilities(midici)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProcessInquiryCapabilitiesReply) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x7F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};
  constexpr auto features = std::byte{0b0101010};

  // clang-format off
  constexpr std::array message {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x41}, // Universal System Exclusive Sub-ID#2: Inquiry: Process Inquiry Capabilities Reply
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    features
  };
  // clang-format on

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pi_capability_reply;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::process_inquiry::capabilities_reply reply;
  reply.features = features;

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(pi_mocks_, capabilities_reply(midici, reply)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProcessInquiryMidiMessageReport) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x01};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  // clang-format off
  constexpr std::array message {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x42}, // Universal System Exclusive Sub-ID#2: Inquiry: MIDI Message Report
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)

    std::byte{0x7F}, // message data control
    std::byte{0b00000111}, // requested system messages
    std::byte{0x00}, // reserved
    std::byte{0b00111111}, // requested channel controller messages
    std::byte{0b00011111}, // requested note data messages
  };
  // clang-format on

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pi_mm_report;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  midi2::ci::process_inquiry::midi_message_report reply;
  reply.message_data_control = decltype(reply)::control::full;
  // system messages
  reply.mtc_quarter_frame = 1;
  reply.song_position = 1;
  reply.song_select = 1;
  // channel controller messages
  reply.pitchbend = 1;
  reply.control_change = 1;
  reply.rpn_registered_controller = 1;
  reply.nrpn_assignable_controller = 1;
  reply.program_change = 1;
  reply.channel_pressure = 1;
  // note data messages
  reply.notes = 1;
  reply.poly_pressure = 1;
  reply.per_note_pitchbend = 1;
  reply.registered_per_note_controller = 1;
  reply.assignable_per_note_controller = 1;

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(pi_mocks_, midi_message_report(midici, reply)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

TEST_F(CIProcessor, ProcessInquiryMidiMessageReportEnd) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto destination = std::byte{0x01};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto destination_muid = std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  // clang-format off
  constexpr std::array message {
    std::byte{0x7E}, // Universal System Exclusive
    destination, // Destination
    std::byte{0x0D}, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x44}, // Universal System Exclusive Sub-ID#2: Inquiry: MIDI Message Report End
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid[0], sender_muid[1], sender_muid[2], sender_muid[3], // 4 bytes Source MUID (LSB first)
    destination_muid[0], destination_muid[1], destination_muid[2], destination_muid[3], // Destination MUID (LSB first)
  };
  // clang-format on

  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::ci_message::pi_mm_report_end;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  EXPECT_CALL(management_mocks_, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(pi_mocks_, midi_message_report_end(midici)).Times(1);

  processor_.startSysex7(group, destination);
  std::ranges::for_each(message, std::bind_front(&decltype(processor_)::processMIDICI, &processor_));
}

// This test simply gets midiCIProcessor to consume a random buffer.
void NeverCrashes(std::vector<std::byte> const &message) {
  // Ensure the top bit is clear.
  std::vector<std::byte> message2;
  message2.reserve(message.size());
  std::ranges::transform(message, std::back_inserter(message2), [](std::byte v) { return v & std::byte{0x7F}; });
  midi2::midiCIProcessor processor;
  std::ranges::for_each(message2, std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(CIProcessorFuzz, NeverCrashes);
#endif
TEST(CIProcessorFuzz, Empty) {
  NeverCrashes({});
}

}  // end anonymous namespace
