#include <gmock/gmock.h>

#include "midi2/midiCIProcessor.h"

namespace {

template <std::size_t Size> std::ostream &write_bytes(std::ostream &os, std::array<std::byte, Size> const &arr) {
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

std::ostream &operator<<(std::ostream &os, ci::discovery_v1 const &d);
std::ostream &operator<<(std::ostream &os, ci::discovery_v1 const &d) {
  os << "{ manufacturer=";
  write_bytes(os, d.manufacturer);
  os << ", family=" << d.family << ", model=" << d.model << ", version=";
  write_bytes(os, d.version);
  os << ", capability=" << static_cast<unsigned>(d.capability) << ", max_sysex_size=" << d.max_sysex_size << " }";
  return os;
}
std::ostream &operator<<(std::ostream &os, ci::discovery_v2 const &d);
std::ostream &operator<<(std::ostream &os, ci::discovery_v2 const &d) {
  return os << "{ " << static_cast<ci::discovery_v1 const &>(d) << "output_path_id=" << d.output_path_id << " }";
}

std::ostream &operator<<(std::ostream &os, ci::discovery_reply_v1 const &d);
std::ostream &operator<<(std::ostream &os, ci::discovery_reply_v1 const &d) {
  os << "{ manufacturer=";
  write_bytes(os, d.manufacturer);
  os << ", family=" << d.family << ", model=" << d.model << ", version=";
  write_bytes(os, d.version);
  os << ", capability=" << static_cast<unsigned>(d.capability) << ", max_sysex_size=" << d.max_sysex_size << " }";
  return os;
}
std::ostream &operator<<(std::ostream &os, ci::discovery_reply_v2 const &d);
std::ostream &operator<<(std::ostream &os, ci::discovery_reply_v2 const &d) {
  return os << "{ " << static_cast<ci::discovery_reply_v1 const &>(d) << "output_path_id=" << d.output_path_id
            << "function_block=" << static_cast<unsigned>(d.function_block) << " }";
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
  MOCK_METHOD(void, discovery, (MIDICI const &, midi2::ci::discovery_current const &), (override));
  MOCK_METHOD(void, discovery_reply, (MIDICI const &, midi2::ci::discovery_reply_current const &), (override));
  MOCK_METHOD(void, end_point_info, (MIDICI const &, std::byte), (override));
  MOCK_METHOD(void, end_point_info_reply, (MIDICI const &, std::uint8_t, std::uint16_t, std::span<std::byte>),
              (override));

  MOCK_METHOD(void, ack,
              (MIDICI const &, std::uint8_t, std::uint8_t, std::uint8_t, (std::span<std::byte, 5>), std::uint16_t,
               std::span<std::byte>),
              (override));
  MOCK_METHOD(void, nak,
              (MIDICI const &, std::uint8_t, std::uint8_t, std::uint8_t, (std::span<std::byte, 5>), std::uint16_t,
               std::span<std::byte>),
              (override));
  MOCK_METHOD(void, invalidate_muid, (MIDICI const &, uint32_t), (override));

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
  constexpr auto family = swappable<std::uint16_t>{0x6789};
  constexpr auto model = swappable<std::uint16_t>{0xABCD};
  constexpr auto version = std::array{std::byte{0xFE}, std::byte{0xDC}, std::byte{0xBA}, std::byte{0x98}};
  constexpr auto capability = std::byte{0x7F};
  constexpr auto max_sysex_size = swappable<std::uint32_t>{0x76543210};
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
  midici.deviceId = midi2::ci::FUNCTION_BLOCK;
  midici.ciType = midi2::MIDICI_DISCOVERY;
  midici.ciVer = 2;
  midici.remoteMUID = 0;
  midici.localMUID = 0x0FFFFFFF;
  midici._peReqIdx = std::optional<midi2::ci::reqId>{};
  midici.totalChunks = 0;
  midici.numChunk = 0;
  midici.partialChunkCount = 0;
  midici.requestId = std::byte{0xFF};

  midi2::ci::discovery_current discovery;
  discovery.manufacturer = manufacturer;
  discovery.family = family;
  discovery.model = model;
  discovery.version = version;
  discovery.capability = static_cast<std::uint8_t>(capability);
  discovery.max_sysex_size = max_sysex_size;
  discovery.output_path_id = static_cast<std::uint8_t>(output_path_id);

  mock_callbacks mocks;
  EXPECT_CALL(mocks, discovery(midici, discovery)).Times(1);
  midi2::midiCIProcessor ci{std::ref(mocks)};
  std::for_each(std::begin(message), std::end(message), std::bind_front(&decltype(ci)::processMIDICI, &ci));
}

TEST(CIProcessor, DiscoveryReply) {
  constexpr auto manufacturer = std::array{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}};
  constexpr auto family = swappable<std::uint16_t>{0x6789};
  constexpr auto model = swappable<std::uint16_t>{0xABCD};
  constexpr auto version = std::array{std::byte{0xFE}, std::byte{0xDC}, std::byte{0xBA}, std::byte{0x98}};
  constexpr auto capability = std::byte{0x7F};
  constexpr auto max_sysex_size = swappable<std::uint32_t>{0x76543210};
  constexpr auto output_path_id = std::byte{0x71};
  constexpr auto function_block = std::byte{0x82};

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
  midici.deviceId = midi2::ci::FUNCTION_BLOCK;
  midici.ciType = midi2::MIDICI_DISCOVERY_REPLY;
  midici.ciVer = 2;
  midici.remoteMUID = 0;
  midici.localMUID = 0x0FFFFFFF;
  midici._peReqIdx = std::optional<midi2::ci::reqId>{};
  midici.totalChunks = 0;
  midici.numChunk = 0;
  midici.partialChunkCount = 0;
  midici.requestId = std::byte{0xFF};

  midi2::ci::discovery_reply_current reply;
  reply.manufacturer = manufacturer;
  reply.family = family;
  reply.model = model;
  reply.version = version;
  reply.capability = static_cast<std::uint8_t>(capability);
  reply.max_sysex_size = max_sysex_size;
  reply.output_path_id = static_cast<std::uint8_t>(output_path_id);
  reply.function_block = static_cast<std::uint8_t>(function_block);

  mock_callbacks mocks;
  EXPECT_CALL(mocks, discovery_reply(midici, reply)).Times(1);
  midi2::midiCIProcessor ci{std::ref(mocks)};
  std::for_each(std::begin(message), std::end(message), std::bind_front(&decltype(ci)::processMIDICI, &ci));
}

}  // end anonymous namespace
