// DUT
#include "midi2/ci_types.h"
#include "midi2/midiCIProcessor.h"

// Standard library
#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <ostream>

// 3rd party
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

template <std::size_t Size> std::ostream &write_bytes(std::ostream &os, std::array<std::uint8_t, Size> const &arr) {
  os << '[';
  auto const *separator = "";
  for (auto m : arr) {
    os << separator << static_cast<unsigned>(m);
    separator = ",";
  }
  os << ']';
  return os;
}

}  // end anonymous namespace

namespace midi2 {

std::ostream &operator<<(std::ostream &os, MIDICI const &ci);
std::ostream &operator<<(std::ostream &os, MIDICI const &ci) {
  return os << "{ umpGroup=" << static_cast<unsigned>(ci.umpGroup)
            << ", deviceId=" << static_cast<unsigned>(ci.deviceId) << ", ciType=" << static_cast<unsigned>(ci.ciType)
            << ", ciVer=" << static_cast<unsigned>(ci.ciVer) << ", remoteMUID=" << ci.remoteMUID << ", localMUID="
            << ci.localMUID
            //  std::optional<reqId> _peReqIdx;

            << ", totalChunks=" << static_cast<unsigned>(ci.totalChunks)
            << ", numChunk=" << static_cast<unsigned>(ci.numChunk)
            << ", partialChunkCount=" << static_cast<unsigned>(ci.partialChunkCount)
            << ", requestId=" << static_cast<unsigned>(ci.requestId) << " }";
}

std::ostream &operator<<(std::ostream &os, ci::discovery const &d);
std::ostream &operator<<(std::ostream &os, ci::discovery const &d) {
  os << "{ manufacturer=";
  write_bytes(os, d.manufacturer);
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

}  // end namespace midi2

namespace {

using testing::AllOf;
using testing::ElementsAreArray;
using testing::Eq;
using testing::Field;
using testing::Return;

using midi2::MIDICI;
using midi2::profile_span;
using midi2::ci::byte_array_5;
using midi2::ci::packed::from_le7;

class mock_discovery_callbacks final : public midi2::ci_callbacks {
public:
  MOCK_METHOD(bool, check_muid, (std::uint8_t group, std::uint32_t muid), (override));
  MOCK_METHOD(void, discovery, (MIDICI const &, midi2::ci::discovery const &), (override));
  MOCK_METHOD(void, discovery_reply, (MIDICI const &, midi2::ci::discovery_reply const &), (override));
  MOCK_METHOD(void, endpoint_info, (MIDICI const &, midi2::ci::endpoint_info const &), (override));
  MOCK_METHOD(void, endpoint_info_reply, (MIDICI const &, midi2::ci::endpoint_info_reply const &), (override));
  MOCK_METHOD(void, invalidate_muid, (MIDICI const &, midi2::ci::invalidate_muid const &), (override));
  MOCK_METHOD(void, ack, (MIDICI const &, midi2::ci::ack const &), (override));
  MOCK_METHOD(void, nak, (MIDICI const &, midi2::ci::nak const &), (override));
};

class mock_profile_callbacks final : public midi2::profile_callbacks {
public:
  MOCK_METHOD(void, inquiry, (MIDICI const &), (override));
  MOCK_METHOD(void, inquiry_reply, (MIDICI const &, midi2::ci::profile_inquiry_reply const &), (override));
  MOCK_METHOD(void, added, (MIDICI const &, midi2::ci::profile_added const &), (override));
  MOCK_METHOD(void, removed, (MIDICI const &, midi2::ci::profile_removed const &), (override));
  MOCK_METHOD(void, details_inquiry, (MIDICI const &, midi2::ci::profile_details_inquiry const &), (override));
  MOCK_METHOD(void, details_reply, (MIDICI const &, midi2::ci::profile_details_reply const &), (override));
  MOCK_METHOD(void, on, (MIDICI const &, midi2::ci::profile_on const &), (override));
  MOCK_METHOD(void, off, (MIDICI const &, midi2::ci::profile_off const &), (override));
  MOCK_METHOD(void, enabled, (MIDICI const &, midi2::ci::profile_enabled const &), (override));
  MOCK_METHOD(void, disabled, (MIDICI const &, midi2::ci::profile_disabled const &), (override));
  MOCK_METHOD(void, specific_data, (MIDICI const &, midi2::ci::profile_specific_data const &), (override));
};

constexpr auto broadcast_muid = std::array{std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}};

TEST(CIProcessor, Empty) {
  mock_discovery_callbacks mocks;
  midi2::midiCIProcessor ci{std::ref(mocks)};
  ci.processMIDICI(std::byte{0});
}

TEST(CIProcessor, Discovery) {
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
    std::byte{0x7F}, // Device ID: 0x7F = to MIDI Port
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
  midici.deviceId = 0xFF;
  midici.ciType = midi2::MIDICI_DISCOVERY;
  midici.ciVer = 2;
  midici.remoteMUID = 0;
  midici.localMUID = 0x0FFFFFFF;

  midi2::ci::discovery discovery;
  discovery.manufacturer = midi2::ci::packed::from_array(manufacturer);
  discovery.family = from_le7(family);
  discovery.model = from_le7(model);
  discovery.version = midi2::ci::packed::from_array(version);
  discovery.capability = static_cast<std::uint8_t>(capability);
  discovery.max_sysex_size = from_le7(max_sysex_size);
  discovery.output_path_id = static_cast<std::uint8_t>(output_path_id);

  mock_discovery_callbacks mocks;
  EXPECT_CALL(mocks, discovery(midici, discovery)).Times(1);
  midi2::midiCIProcessor ci{std::ref(mocks)};
  std::for_each(std::begin(message), std::end(message), std::bind_front(&decltype(ci)::processMIDICI, &ci));
}

TEST(CIProcessor, DiscoveryReply) {
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
    std::byte{0x7F}, // Device ID: 0x7F = to MIDI Port
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
  midici.deviceId = 0xFF;
  midici.ciType = midi2::MIDICI_DISCOVERY_REPLY;
  midici.ciVer = 2;
  midici.remoteMUID = 0;
  midici.localMUID = 0x0FFFFFFF;

  midi2::ci::discovery_reply reply;
  reply.manufacturer = midi2::ci::packed::from_array(manufacturer);
  reply.family = from_le7(family);
  reply.model = from_le7(model);
  reply.version = midi2::ci::packed::from_array(version);
  reply.capability = static_cast<std::uint8_t>(capability);
  reply.max_sysex_size = from_le7(max_sysex_size);
  reply.output_path_id = static_cast<std::uint8_t>(output_path_id);
  reply.function_block = static_cast<std::uint8_t>(function_block);

  mock_discovery_callbacks mocks;
  EXPECT_CALL(mocks, discovery_reply(midici, reply)).Times(1);
  midi2::midiCIProcessor ci{std::ref(mocks)};
  std::for_each(std::begin(message), std::end(message), std::bind_front(&decltype(ci)::processMIDICI, &ci));
}

TEST(CIProcessor, EndpointInfo) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto status = std::uint8_t{0b0101010};
  constexpr std::array const sender_muid{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr std::array const receiver_muid{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  // clang-format off
  std::array const message{
    std::byte{0x7E}, // Universal System Exclusive
    std::byte{0x7F}, // Device ID: 0x7F = to MIDI Port
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
  midici.deviceId = 0x7F;
  midici.ciType = midi2::MIDICI_ENDPOINTINFO;
  midici.ciVer = 1;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  midi2::ci::endpoint_info endpoint_info;
  endpoint_info.status = status;

  mock_discovery_callbacks mocks;
  EXPECT_CALL(mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(mocks, endpoint_info(midici, endpoint_info)).Times(1);
  midi2::midiCIProcessor processor{std::ref(mocks)};
  processor.startSysex7(group, message[1]);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

auto EndpointInfoReplyMatches(std::byte status, std::span<std::byte const> info) {
  return AllOf(Field("status", &midi2::ci::endpoint_info_reply::status, Eq(status)),
               Field("information", &midi2::ci::endpoint_info_reply::information, ElementsAreArray(info)));
};

TEST(CIProcessor, EndpointInfoReply) {
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
  midici.ciType = midi2::MIDICI_ENDPOINTINFO_REPLY;
  midici.ciVer = 1;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  mock_discovery_callbacks mocks;
  EXPECT_CALL(mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(mocks, endpoint_info_reply(
                         midici, EndpointInfoReplyMatches(
                                     status, std::span<std::byte const>{information.begin(), information.size()})))
      .Times(1);
  midi2::midiCIProcessor processor{std::ref(mocks)};
  processor.startSysex7(group, device_id);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, InvalidateMuid) {
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
  midici.ciType = midi2::MIDICI_INVALIDATEMUID;
  midici.ciVer = 1;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  mock_discovery_callbacks mocks;
  EXPECT_CALL(mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  midi2::ci::invalidate_muid invalidate_muid;
  invalidate_muid.target_muid = from_le7(target_muid);
  EXPECT_CALL(mocks, invalidate_muid(midici, invalidate_muid)).Times(1);
  midi2::midiCIProcessor processor{std::ref(mocks)};
  processor.startSysex7(group, device_id);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, Ack) {
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
    original_id, // Originl transaciton sub-ID#2 classification
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
  midici.ciType = midi2::MIDICI_ACK;
  midici.ciVer = 1;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  mock_discovery_callbacks mocks;
  EXPECT_CALL(mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(
      mocks,
      ack(midici,
          AllOf(Field("original_id", &midi2::ci::ack::original_id, Eq(static_cast<std::uint8_t>(original_id))),
                Field("status_code", &midi2::ci::ack::status_code, Eq(static_cast<std::uint8_t>(ack_status_code))),
                Field("status_data", &midi2::ci::ack::status_data, Eq(static_cast<std::uint8_t>(ack_status_data))),
                Field("details", &midi2::ci::ack::details, ElementsAreArray(ack_details)),
                Field("message", &midi2::ci::ack::message, ElementsAreArray(text)))))
      .Times(1);

  midi2::midiCIProcessor processor{std::ref(mocks)};
  processor.startSysex7(group, device_id);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, NakV2) {
  constexpr auto group = std::uint8_t{0x01};
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto sender_muid = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  constexpr auto original_id = std::byte{0x34};
  constexpr auto nak_status_code = std::byte{0x00};
  constexpr auto nak_status_data = std::byte{0x7F};
  constexpr auto nakk_details =
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
    nakk_details[0], nakk_details[1], nakk_details[2], nakk_details[3], nakk_details[4],
    text_length[0], text_length[1],
    text[0], text[1], text[2], text[3], text[4],

              std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::MIDICI_NAK;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  mock_discovery_callbacks mocks;
  EXPECT_CALL(mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(
      mocks,
      nak(midici,
          AllOf(Field("original_id", &midi2::ci::nak::original_id, Eq(static_cast<std::uint8_t>(original_id))),
                Field("status_code", &midi2::ci::nak::status_code, Eq(static_cast<std::uint8_t>(nak_status_code))),
                Field("status_data", &midi2::ci::nak::status_data, Eq(static_cast<std::uint8_t>(nak_status_data))),
                Field("details", &midi2::ci::nak::details, ElementsAreArray(nakk_details)),
                Field("message", &midi2::ci::nak::message, ElementsAreArray(text)))))
      .Times(1);

  midi2::midiCIProcessor processor{std::ref(mocks)};
  processor.startSysex7(group, device_id);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileInquiry) {
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

            std::byte{0}, // stray extra byte
  };
  // clang-format on
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(destination);
  midici.ciType = midi2::MIDICI_PROFILE_INQUIRY;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  EXPECT_CALL(discovery_mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks, inquiry(midici)).Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileInquiryReply) {
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
  midici.ciType = midi2::MIDICI_PROFILE_INQUIRYREPLY;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(receiver_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  EXPECT_CALL(discovery_mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  using midi2::ci::profile_inquiry_reply;
  EXPECT_CALL(
      profile_mocks,
      inquiry_reply(midici, AllOf(Field("enabled", &profile_inquiry_reply::enabled, ElementsAreArray(enabled)),
                                  Field("disabled", &profile_inquiry_reply::disabled, ElementsAreArray(disabled)))))
      .Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileAdded) {
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
  midici.ciType = midi2::MIDICI_PROFILE_ADDED;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(broadcast_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  midi2::ci::profile_added added;
  added.pid = pid;
  EXPECT_CALL(profile_mocks, added(midici, added)).Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileRemoved) {
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
  midici.ciType = midi2::MIDICI_PROFILE_REMOVED;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(broadcast_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  midi2::ci::profile_removed removed;
  removed.pid = pid;
  EXPECT_CALL(profile_mocks, removed(midici, removed)).Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileDetailsInquiry) {
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
  midici.ciType = midi2::MIDICI_PROFILE_DETAILS_INQUIRY;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  midi2::ci::profile_details_inquiry inquiry;
  inquiry.pid = pid;
  inquiry.target = 0x23;
  EXPECT_CALL(discovery_mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks, details_inquiry(midici, inquiry)).Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileDetailsReply) {
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
  midici.ciType = midi2::MIDICI_PROFILE_DETAILS_REPLY;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  using midi2::ci::profile_details_reply;
  
  EXPECT_CALL(discovery_mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks, details_reply(midici,
                                           AllOf(Field("pid", &profile_details_reply::pid, Eq(pid)),
                                                 Field("target", &profile_details_reply::target, Eq(0x23)),
                                                 Field("data", &profile_details_reply::data, ElementsAreArray(data))
                                                )
                                          )
             ).Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileOn) {
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
  midici.ciType = midi2::MIDICI_PROFILE_SETON;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  midi2::ci::profile_on on;
  on.pid = pid;
  on.num_channels = from_le7(channels);
  EXPECT_CALL(discovery_mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks, on(midici, on)).Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileOff) {
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
  midici.ciType = midi2::MIDICI_PROFILE_SETOFF;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(destination_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  midi2::ci::profile_off off;
  off.pid = pid;
  EXPECT_CALL(discovery_mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(profile_mocks, off(midici, off)).Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileEnabled) {
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
  midici.ciType = midi2::MIDICI_PROFILE_ENABLED;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(broadcast_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  midi2::ci::profile_enabled enabled;
  enabled.pid = pid;
  enabled.num_channels = from_le7(num_channels);
  EXPECT_CALL(profile_mocks, enabled(midici, enabled)).Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

TEST(CIProcessor, ProfileSpecificData) {
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
  midici.ciType = midi2::MIDICI_PROFILE_SPECIFIC_DATA;
  midici.ciVer = 2;
  midici.remoteMUID = from_le7(sender_muid);
  midici.localMUID = from_le7(broadcast_muid);

  mock_discovery_callbacks discovery_mocks;
  mock_profile_callbacks profile_mocks;

  EXPECT_CALL(
      profile_mocks,
      specific_data(midici, AllOf(Field("pid", &midi2::ci::profile_specific_data::pid, Eq(pid)),
                                  Field("data", &midi2::ci::profile_specific_data::data, ElementsAreArray(data)))))
      .Times(1);

  midi2::midiCIProcessor processor{std::ref(discovery_mocks), std::ref(profile_mocks)};
  processor.startSysex7(group, destination);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

}  // end anonymous namespace
