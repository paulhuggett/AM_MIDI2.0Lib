#include <gmock/gmock.h>

#include "midi2/midiCIProcessor.h"

namespace {

template <std::size_t Size> std::ostream &write_bytes(std::ostream &os, std::array<std::uint8_t, Size> const &arr) {
  os << '[';
  auto separator = "";
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

using midi2::device_family;
using midi2::device_manufacturer;
using midi2::device_model;
using midi2::device_version;
using midi2::MIDICI;
using midi2::profile_span;

class mock_callbacks final : public midi2::ci_callbacks {
public:
  MOCK_METHOD(bool, check_muid, (std::uint8_t group, std::uint32_t muid), (override));
  MOCK_METHOD(void, discovery, (MIDICI const &, midi2::ci::discovery const &), (override));
  MOCK_METHOD(void, discovery_reply, (MIDICI const &, midi2::ci::discovery_reply const &), (override));
  MOCK_METHOD(void, endpoint_info, (MIDICI const &, midi2::ci::endpoint_info const &), (override));
  MOCK_METHOD(void, endpoint_info_reply, (MIDICI const &, midi2::ci::endpoint_info_reply const &), (override));
  MOCK_METHOD(void, invalidate_muid, (MIDICI const &, midi2::ci::invalidate_muid const &), (override));

  MOCK_METHOD(void, ack,
              (MIDICI const &, std::uint8_t, std::uint8_t, std::uint8_t, (std::span<std::byte, 5>), std::uint16_t,
               std::span<std::byte>),
              (override));
  MOCK_METHOD(void, nak,
              (MIDICI const &, std::uint8_t, std::uint8_t, std::uint8_t, (std::span<std::byte, 5>), std::uint16_t,
               std::span<std::byte>),
              (override));

  using protocol_span = std::span<std::byte, 5>;
  MOCK_METHOD(void, protocol_available, (MIDICI const &midici, std::uint8_t authority_level, protocol_span protocol),
              (override));
#if 0
  void recvSetProtocol(MIDICI const &midici,
                       std::uint8_t authority_level,
                       std::span<std::byte, 5> protocol) {
      original_.recvSetProtocol(midici, authority_level, protocol);
  }
  void recvSetProtocolConfirm(MIDICI const &midici, std::uint8_t authority_level) {
      original_.recvSetProtocolConfirm(midici, authority_level);
  }
  void recvProtocolTest(MIDICI const &midici, std::uint8_t authority_level,
                        bool test_data_accurate) {
      original_.recvProtocolTest(midici, authority_level, test_data_accurate);
  }

  void recvProfileInquiry(MIDICI const &midici) {
    original_.recvProfileInquiry(midici);
  }
  void recvSetProfileEnabled(MIDICI const &midici,
                             midi2::profile_span profile,
                             std::uint8_t number_of_channels) {
    original_.recvSetProfileEnabled(midici, profile, number_of_channels);
  }
#endif
};

TEST(CIProcessor, Empty) {
  midi2::midiCIProcessor ci;
  ci.processMIDICI(std::byte{0});
}

using testing::ElementsAreArray;
using testing::Return;

template <std::unsigned_integral T> class swappable {
public:
  explicit constexpr swappable(T value) : value_{value} {}
  constexpr auto operator[](std::size_t index) const {
    return static_cast<std::byte>((value_ >> (index * CHAR_BIT)) & 0xFF);
  }
  constexpr T value() const { return value_; }
  constexpr operator T() const { return value_; }

private:
  T value_;
};

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
  };
  // clang-format on
  static_assert(message.size() == 32 - 2);
  MIDICI midici;
  midici.umpGroup = 0xFF;
  midici.deviceId = 0xFF;
  midici.ciType = midi2::MIDICI_DISCOVERY;
  midici.ciVer = 2;
  midici.remoteMUID = 0;
  midici.localMUID = 0x0FFFFFFF;
  midici._peReqIdx = std::optional<midi2::ci::reqId>{};
  midici.totalChunks = 0;
  midici.numChunk = 0;
  midici.partialChunkCount = 0;
  midici.requestId = std::byte{0xFF};

  midi2::ci::discovery discovery;
  discovery.manufacturer = midi2::ci::packed::from_array(manufacturer);
  discovery.family = midi2::ci::packed::from_le7(family);
  discovery.model = midi2::ci::packed::from_le7(model);
  discovery.version = midi2::ci::packed::from_array(version);
  discovery.capability = static_cast<std::uint8_t>(capability);
  discovery.max_sysex_size = midi2::ci::packed::from_le7(max_sysex_size);
  discovery.output_path_id = static_cast<std::uint8_t>(output_path_id);

  mock_callbacks mocks;
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
  };
  // clang-format on
  static_assert(message.size() == 33 - 2);
  MIDICI midici;
  midici.umpGroup = 0xFF;
  midici.deviceId = 0xFF;
  midici.ciType = midi2::MIDICI_DISCOVERY_REPLY;
  midici.ciVer = 2;
  midici.remoteMUID = 0;
  midici.localMUID = 0x0FFFFFFF;
  midici._peReqIdx = std::optional<midi2::ci::reqId>{};
  midici.totalChunks = 0;
  midici.numChunk = 0;
  midici.partialChunkCount = 0;
  midici.requestId = std::byte{0xFF};

  midi2::ci::discovery_reply reply;
  reply.manufacturer = midi2::ci::packed::from_array(manufacturer);
  reply.family = midi2::ci::packed::from_le7(family);
  reply.model = midi2::ci::packed::from_le7(model);
  reply.version = midi2::ci::packed::from_array(version);
  reply.capability = static_cast<std::uint8_t>(capability);
  reply.max_sysex_size = midi2::ci::packed::from_le7(max_sysex_size);
  reply.output_path_id = static_cast<std::uint8_t>(output_path_id);
  reply.function_block = static_cast<std::uint8_t>(function_block);

  mock_callbacks mocks;
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
  };
  // clang-format on
  static_assert(message.size() == 13 + sizeof(midi2::ci::packed::endpoint_info_v1));
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = 0x7F;
  midici.ciType = midi2::MIDICI_ENDPOINTINFO;
  midici.ciVer = 1;
  midici.remoteMUID = midi2::ci::packed::from_le7(sender_muid);
  midici.localMUID = midi2::ci::packed::from_le7(receiver_muid);
  midici._peReqIdx = std::optional<midi2::ci::reqId>{};
  midici.totalChunks = 0;
  midici.numChunk = 0;
  midici.partialChunkCount = 0;
  midici.requestId = std::byte{0xFF};

  midi2::ci::endpoint_info endpoint_info;
  endpoint_info.status = status;

  mock_callbacks mocks;
  EXPECT_CALL(mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  EXPECT_CALL(mocks, endpoint_info(midici, endpoint_info)).Times(1);
  midi2::midiCIProcessor processor{std::ref(mocks)};
  processor.startSysex7(group, message[1]);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

using testing::AllOf;
using testing::ElementsAreArray;
using testing::Eq;
using testing::Field;

auto EndpointInfoReplyMatches(std::byte status, std::span<std::byte const> info) {
  return AllOf(Field("status", &midi2::ci::endpoint_info_reply::status, Eq(status)),
               Field("information", &midi2::ci::endpoint_info_reply::information, ElementsAreArray(info)));
};

TEST(CIProcessor, EndpointInfoReply) {
  constexpr auto group = std::uint8_t{0x71};
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto status = std::byte{0b0101010};
  constexpr auto length = swappable<std::uint16_t>{8};
  constexpr std::array const sender_muid{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  constexpr std::array const receiver_muid{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};
  constexpr std::array<std::byte, length.value()> information{
      std::byte{2},  std::byte{3},  std::byte{5},  std::byte{7},  // Information data
      std::byte{11}, std::byte{13}, std::byte{17}, std::byte{19},
  };

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
  };
  // clang-format on
  static_assert(message.size() == 16 + length);
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::MIDICI_ENDPOINTINFO_REPLY;
  midici.ciVer = 1;
  midici.remoteMUID = midi2::ci::packed::from_le7(sender_muid);
  midici.localMUID = midi2::ci::packed::from_le7(receiver_muid);
  midici._peReqIdx = std::optional<midi2::ci::reqId>{};
  midici.totalChunks = 0;
  midici.numChunk = 0;
  midici.partialChunkCount = 0;
  midici.requestId = std::byte{0xFF};

  mock_callbacks mocks;
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
    target_muid[0], target_muid[1], target_muid[2], target_muid[3] // Target MUID (the MUID to invalidate) (LSB first)
  };
  // clang-format on
  static_assert(message.size() == 17);
  MIDICI midici;
  midici.umpGroup = group;
  midici.deviceId = static_cast<std::uint8_t>(device_id);
  midici.ciType = midi2::MIDICI_INVALIDATEMUID;
  midici.ciVer = 1;
  midici.remoteMUID = midi2::ci::packed::from_le7(sender_muid);
  midici.localMUID = midi2::ci::packed::from_le7(receiver_muid);
  midici._peReqIdx = std::optional<midi2::ci::reqId>{};
  midici.totalChunks = 0;
  midici.numChunk = 0;
  midici.partialChunkCount = 0;
  midici.requestId = std::byte{0xFF};

  mock_callbacks mocks;
  EXPECT_CALL(mocks, check_muid(group, midici.localMUID)).WillRepeatedly(Return(true));
  midi2::ci::invalidate_muid invalidate_muid;
  invalidate_muid.target_muid = midi2::ci::packed::from_le7(target_muid);
  EXPECT_CALL(mocks, invalidate_muid(midici, invalidate_muid)).Times(1);
  midi2::midiCIProcessor processor{std::ref(mocks)};
  processor.startSysex7(group, device_id);

  std::for_each(std::begin(message), std::end(message),
                std::bind_front(&decltype(processor)::processMIDICI, &processor));
}

}  // end anonymous namespace
