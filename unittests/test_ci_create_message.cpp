//===-- ci create message------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ci_create_message.hpp"

// MIDI2
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

consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}

using midi2::ci::byte_array;
using midi2::ci::details::from_byte_array;
using midi2::ci::details::from_le7;
using midi2::ci::details::to_le7;

class CICreateMessage : public testing::Test {
protected:
  static constexpr auto broadcast_muid_ = std::array{0x7F_b, 0x7F_b, 0x7F_b, 0x7F_b};
  static constexpr auto sender_muid_ = std::array{0x7F_b, 0x7E_b, 0x7D_b, 0x7C_b};
  static constexpr auto destination_muid_ = std::array{0x62_b, 0x16_b, 0x63_b, 0x26_b};

  template <typename Content>
  static std::vector<std::byte> make_message(struct midi2::ci::header const& hdr, Content const& content) {
    std::vector<std::byte> message;
    midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{}, hdr, content);
    return message;
  }
};

TEST_F(CICreateMessage, DiscoveryV1) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto manufacturer = std::array{0x12_b, 0x23_b, 0x34_b};
  constexpr auto family = std::array{0x67_b, 0x79_b};
  constexpr auto model = std::array{0x6B_b, 0x5D_b};
  constexpr auto version = std::array{0x4E_b, 0x3C_b, 0x2A_b, 0x18_b};
  constexpr auto capability = 0x7F_b;
  constexpr auto max_sysex_size = std::array{0x76_b, 0x54_b, 0x32_b, 0x10_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::discovery)),
    1_b, // 1 byte MIDI-CI Message Version/Format
    0_b, 0_b, 0_b, 0_b, // 4 bytes Source MUID (LSB first)
    0x7F_b, 0x7F_b, 0x7F_b, 0x7F_b, // Destination MUID (LSB first) (to Broadcast MUID)
    manufacturer[0], manufacturer[1], manufacturer[2], // 3 bytes Device Manufacturer (System Exclusive ID Number)
    family[0], family[1],  // 2 bytes Device Family (LSB first)
    model[0], model[1], // 2 bytes Device Family Model Number (LSB first)
    version[0], version[1], version[2], version[3],  // 4 bytes Software Revision Level
    capability, // 1 byte Capability Inquiry Category Supported (bitmap)
    max_sysex_size[0], max_sysex_size[1], max_sysex_size[2], max_sysex_size[3], // Maximum sysex message size (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::header hdr{.device_id = midi2::to_underlying(device_id),
                                  .version = 1,
                                  .remote_muid = 0,
                                  .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery discovery{.manufacturer = from_byte_array(manufacturer),
                                           .family = from_le7(family),
                                           .model = from_le7(model),
                                           .version = from_byte_array(version),
                                           .capability = midi2::to_underlying(capability),
                                           .max_sysex_size = from_le7(max_sysex_size)};
  EXPECT_THAT(make_message(hdr, discovery), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, DiscoveryV2) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto manufacturer = std::array{0x12_b, 0x23_b, 0x34_b};
  constexpr auto family = std::array{0x67_b, 0x79_b};
  constexpr auto model = std::array{0x6B_b, 0x5D_b};
  constexpr auto version = std::array{0x4E_b, 0x3C_b, 0x2A_b, 0x18_b};
  constexpr auto capability = 0x7F_b;
  constexpr auto max_sysex_size = std::array{0x76_b, 0x54_b, 0x32_b, 0x10_b};
  constexpr auto output_path_id = 0x71_b;

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::discovery)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    0_b, 0_b, 0_b, 0_b, // 4 bytes Source MUID (LSB first)
    0x7F_b, 0x7F_b, 0x7F_b, 0x7F_b, // Destination MUID (LSB first) (to Broadcast MUID)
    manufacturer[0], manufacturer[1], manufacturer[2], // 3 bytes Device Manufacturer (System Exclusive ID Number)
    family[0], family[1],  // 2 bytes Device Family (LSB first)
    model[0], model[1], // 2 bytes Device Family Model Number (LSB first)
    version[0], version[1], version[2], version[3],  // 4 bytes Software Revision Level
    capability, // 1 byte Capability Inquiry Category Supported (bitmap)
    max_sysex_size[0], max_sysex_size[1], max_sysex_size[2], max_sysex_size[3], // Maximum sysex message size (LSB first)
    output_path_id, // [1] initiator's output path ID
  };
  // clang-format on

  constexpr midi2::ci::header hdr{.device_id = midi2::to_underlying(device_id),
                                  .version = 2,
                                  .remote_muid = 0,
                                  .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery discovery{.manufacturer = from_byte_array(manufacturer),
                                           .family = from_le7(family),
                                           .model = from_le7(model),
                                           .version = from_byte_array(version),
                                           .capability = midi2::to_underlying(capability),
                                           .max_sysex_size = from_le7(max_sysex_size),
                                           .output_path_id = midi2::to_underlying(output_path_id)};
  EXPECT_THAT(make_message(hdr, discovery), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, DiscoveryReplyV2) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto manufacturer = std::array{0x12_b, 0x23_b, 0x34_b};
  constexpr auto family = std::array{0x67_b, 0x79_b};
  constexpr auto model = std::array{0x5B_b, 0x4D_b};
  constexpr auto version = std::array{0x7E_b, 0x6C_b, 0x5A_b, 0x48_b};
  constexpr auto capability = 0x7F_b;
  constexpr auto max_sysex_size = std::array{0x76_b, 0x54_b, 0x32_b, 0x10_b};
  constexpr auto output_path_id = 0x71_b;
  constexpr auto function_block = 0x32_b;

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::discovery_reply)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    0_b, 0_b, 0_b, 0_b, // 4 bytes Source MUID (LSB first)
    0x7F_b, 0x7F_b, 0x7F_b, 0x7F_b, // Destination MUID (LSB first) (to Broadcast MUID)
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

  constexpr midi2::ci::header hdr{.device_id = midi2::to_underlying(device_id),
                                  .version = 2,
                                  .remote_muid = 0,
                                  .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery_reply reply{.manufacturer = from_byte_array(manufacturer),
                                             .family = from_le7(family),
                                             .model = from_le7(model),
                                             .version = from_byte_array(version),
                                             .capability = midi2::to_underlying(capability),
                                             .max_sysex_size = from_le7(max_sysex_size),
                                             .output_path_id = midi2::to_underlying(output_path_id),
                                             .function_block = midi2::to_underlying(function_block)};
  EXPECT_THAT(make_message(hdr, reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, EndpointInfo) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto status = std::uint8_t{0b0101010};
  constexpr std::array const receiver_muid{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::endpoint_info)),
    1_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    std::byte{status}, // Status
  };
  // clang-format on

  constexpr midi2::ci::header hdr{.device_id = midi2::to_underlying(device_id),
                                  .version = 1,
                                  .remote_muid = from_le7(sender_muid_),
                                  .local_muid = from_le7(receiver_muid)};
  constexpr midi2::ci::endpoint_info endpoint_info{.status = status};
  EXPECT_THAT(make_message(hdr, endpoint_info), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, EndpointInfoReply) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto status = 0b0101010_b;
  constexpr auto receiver_muid = std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};
  constexpr auto length = std::array{0x08_b, 0x00_b};
  constexpr auto information = std::array{
      2_b,  3_b,  5_b,  7_b,  // Information data
      11_b, 13_b, 17_b, 19_b,
  };
  ASSERT_EQ(from_le7(length), information.size());

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::endpoint_info_reply)),
    1_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    status, // Status
    length[0], length[1], // Length of following data (LSB first)
    information[0], information[1], information[2], information[3],
    information[4], information[5],  information[6], information[7],
  };
  // clang-format on

  constexpr midi2::ci::header hdr{.device_id = midi2::to_underlying(device_id),
                                  .version = 1,
                                  .remote_muid = from_le7(sender_muid_),
                                  .local_muid = from_le7(receiver_muid)};

  // Test create_message()
  midi2::ci::endpoint_info_reply const reply{.status = from_le7(status), .information = information};
  EXPECT_THAT(make_message(hdr, reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, InvalidateMuid) {
  constexpr auto device_id = 0x7F_b;
  constexpr std::array const receiver_muid{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};
  constexpr std::array const target_muid{0x21_b, 0x43_b, 0x75_b, 0x71_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::invalidate_muid)),
    1_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    target_muid[0], target_muid[1], target_muid[2], target_muid[3], // Target MUID (the MUID to invalidate) (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::header hdr{.device_id = midi2::to_underlying(device_id),
                                  .version = 1,
                                  .remote_muid = from_le7(sender_muid_),
                                  .local_muid = from_le7(receiver_muid)};
  constexpr midi2::ci::invalidate_muid invalidate_muid{.target_muid = from_le7(target_muid)};
  EXPECT_THAT(make_message(hdr, invalidate_muid), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, Ack) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto receiver_muid = std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};

  constexpr auto original_id = 0x34_b;
  constexpr auto ack_status_code = 0x00_b;
  constexpr auto ack_status_data = 0x7F_b;
  constexpr auto ack_details = std::array{0x01_b, 0x02_b, 0x03_b, 0x04_b, 0x05_b};
  constexpr auto text_length = std::array{0x05_b, 0x00_b};
  static constexpr auto text =
      std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};
  ASSERT_EQ(from_le7(text_length), text.size());

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::ack)),
    1_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    original_id, // Original transaction sub-ID#2 classification
    ack_status_code, // ACK Status Code
    ack_status_data, // ACK Status Data
    ack_details[0], ack_details[1], ack_details[2], ack_details[3], ack_details[4],
    text_length[0], text_length[1],
    text[0], text[1], text[2], text[3], text[4],
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(device_id),
      .version = 1,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(receiver_muid),
  };
  constexpr midi2::ci::ack ack{
      .original_id = midi2::to_underlying(original_id),
      .status_code = midi2::to_underlying(ack_status_code),
      .status_data = midi2::to_underlying(ack_status_data),
      .details = ack_details,
      .message = std::span{text},
  };
  EXPECT_THAT(make_message(hdr, ack), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, NakV1) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto receiver_muid = std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::nak)),
    1_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(device_id),
      .version = 1,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(receiver_muid),
  };
  EXPECT_THAT(make_message(hdr, midi2::ci::nak{}), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, NakV2) {
  constexpr auto device_id = 0x7F_b;
  constexpr auto receiver_muid = std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};

  constexpr auto original_id = 0x34_b;
  constexpr auto nak_status_code = 0x00_b;
  constexpr auto nak_status_data = 0x7F_b;
  constexpr auto nak_details = std::array{0x01_b, 0x02_b, 0x03_b, 0x04_b, 0x05_b};
  constexpr auto text_length = std::array{0x05_b, 0x00_b};
  static constexpr auto text =
      std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::nak)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    original_id, // Originl transaciton sub-ID#2 classification
    nak_status_code, // NAK Status Code
    nak_status_data, // NAK Status Data
    nak_details[0], nak_details[1], nak_details[2], nak_details[3], nak_details[4],
    text_length[0], text_length[1],
    text[0], text[1], text[2], text[3], text[4],
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(device_id),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(receiver_muid),
  };
  // Test create_message()
  constexpr midi2::ci::nak nak{
      .original_id = midi2::to_underlying(original_id),
      .status_code = midi2::to_underlying(nak_status_code),
      .status_data = midi2::to_underlying(nak_status_data),
      .details = nak_details,
      .message = std::span{text},
  };
  EXPECT_THAT(make_message(hdr, nak), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileInquiry) {
  constexpr auto destination = 0x0F_b;
  constexpr auto receiver_muid = std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::profile_inquiry)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(receiver_muid),
  };
  EXPECT_THAT(make_message(hdr, midi2::ci::profile_configuration::inquiry{}), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileInquiryReply) {
  constexpr auto destination = 0x0F_b;
  constexpr auto receiver_muid = std::array{0x12_b, 0x34_b, 0x5E_b, 0x0F_b};

  constexpr auto enabled = std::array<byte_array<5>, 2>{
      byte_array<5>{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b},
      byte_array<5>{0x76_b, 0x65_b, 0x54_b, 0x43_b, 0x32_b},
  };
  constexpr auto disabled = std::array<byte_array<5>, 1>{
      byte_array<5>{0x71_b, 0x61_b, 0x51_b, 0x41_b, 0x31_b},
  };
  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::profile_inquiry_reply)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    2_b, 0_b,
    enabled[0][0], enabled[0][1], enabled[0][2], enabled[0][3], enabled[0][4],
    enabled[1][0], enabled[1][1], enabled[1][2], enabled[1][3], enabled[1][4],
    1_b, 0_b,
    disabled[0][0], disabled[0][1], disabled[0][2], disabled[0][3], disabled[0][4],
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(receiver_muid),
  };
  EXPECT_THAT(make_message(hdr, midi2::ci::profile_configuration::inquiry_reply{enabled, disabled}),
              testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileAdded) {
  constexpr auto destination = 0x0F_b;
  constexpr auto pid = byte_array<5>{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::profile_added)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid_[0], broadcast_muid_[1], broadcast_muid_[2], broadcast_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile being added
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(broadcast_muid_),
  };
  constexpr midi2::ci::profile_configuration::added added{.pid = pid};

  EXPECT_THAT(make_message(hdr, added), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileRemoved) {
  constexpr auto destination = 0x0F_b;
  constexpr auto pid = byte_array<5>{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::profile_removed)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid_[0], broadcast_muid_[1], broadcast_muid_[2], broadcast_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile being removed
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(broadcast_muid_),
  };
  constexpr midi2::ci::profile_configuration::removed removed{.pid = pid};

  EXPECT_THAT(make_message(hdr, removed), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileDetails) {
  constexpr auto destination = 0x0F_b;
  constexpr auto pid = byte_array<5>{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::profile_details)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    0x23_b, // Inquiry target
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::profile_configuration::details details{
      .pid = pid,
      .target = 0x23,
  };
  EXPECT_THAT(make_message(hdr, details), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileDetailsReply) {
  constexpr auto destination = 0x0F_b;
  constexpr auto pid = byte_array<5>{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b};
  constexpr auto target = 0x23_b;
  constexpr auto data_length = std::array{0x05_b, 0x00_b};
  constexpr auto data = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::profile_details_reply)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    target, // Inquiry target
    data_length[0], data_length[1], // Inquiry target data length (LSB first)
    data[0], data[1], data[2], data[3], data[4],
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  using midi2::ci::profile_configuration::details_reply;

  EXPECT_THAT(make_message(hdr, details_reply{pid, midi2::to_underlying(target), data}),
              testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileOn) {
  constexpr auto destination = 0x0F_b;
  constexpr auto pid = byte_array<5>{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b};
  constexpr auto channels = std::array{0x23_b, 0x00_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::profile_set_on)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    channels[0], channels[1], // Number of channels
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(hdr, midi2::ci::profile_configuration::on{pid, from_le7(channels)}),
              testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileOff) {
  constexpr auto destination = 0x0F_b;
  constexpr auto pid = byte_array<5>{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b};
  constexpr auto reserved = std::array{0x00_b, 0x00_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::profile_set_off)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    reserved[0], reserved[1],
  };
  // clang-format on
  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(hdr, midi2::ci::profile_configuration::off{pid}), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileEnabled) {
  constexpr auto destination = 0x0F_b;
  constexpr auto pid = byte_array<5>{0x12_b, 0x23_b, 0x34_b, 0x45_b, 0x56_b};
  constexpr auto num_channels = std::array{0x22_b, 0x11_b};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::profile_enabled)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid_[0], broadcast_muid_[1], broadcast_muid_[2], broadcast_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    num_channels[0], num_channels[1], // Number of channels
  };
  // clang-format on
  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(broadcast_muid_),
  };
  constexpr midi2::ci::profile_configuration::enabled enabled{
      .pid = pid,
      .num_channels = from_le7(num_channels),
  };

  EXPECT_THAT(make_message(hdr, enabled), testing::ElementsAreArray(expected));
}

using namespace std::string_view_literals;

TEST_F(CICreateMessage, PropertyExchangeGetPropertyData) {
  constexpr auto destination = 0x0F_b;

  constexpr auto request = 1_b;

  constexpr auto total_chunks = std::array{1_b, 0_b};
  constexpr auto chunk_number = std::array{1_b, 0_b};

  constexpr auto header_size = std::array{25_b, 0_b};
  constexpr auto data_size = std::array{0_b, 0_b};

  constexpr auto header = R"({"resource":"DeviceInfo"})"sv;

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pe_get)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
    request,
  };
  // clang-format on

  auto const as_byte = [](char const c) { return static_cast<std::byte>(c); };
  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  auto out = std::ranges::copy(header_size, std::back_inserter(expected)).out;
  // Header Body
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U) << "the CI spec mandates that total_chunks==1";
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U) << "the CI spec mandates that chunk_number==1";
  out = std::ranges::copy(chunk_number, out).out;
  // Property Length
  ASSERT_EQ(from_le7(data_size), 0) << "data size must be 0";
  out = std::ranges::copy(data_size, out).out;

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::property_exchange::get get{
      .chunk =
          midi2::ci::property_exchange::chunk_info{
              .number_of_chunks = from_le7(total_chunks),
              .chunk_number = from_le7(chunk_number),
          },
      .request = midi2::to_underlying(request),
      .header = header,
      .data = {},
  };
  EXPECT_THAT(make_message(hdr, get), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeGetPropertyDataReply) {
  constexpr auto destination = 0x0F_b;

  constexpr auto request = 1_b;

  constexpr auto total_chunks = std::array{1_b, 0_b};
  constexpr auto chunk_number = std::array{1_b, 0_b};

  constexpr auto header_size = std::array{14_b, 0_b};
  constexpr auto data_size = std::array{61_b, 0_b};
  constexpr auto header = R"({"status":200})"sv;
  constexpr auto data = R"({"manufacturerId":[125,0,0],"manufacturer":"Educational Use"})"sv;

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pe_get_reply)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    request,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  auto out = std::ranges::copy(header_size, std::back_inserter(expected)).out;
  // Header Body
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  out = std::ranges::copy(total_chunks, out).out;
  out = std::ranges::copy(chunk_number, out).out;
  // Property Length
  ASSERT_EQ(from_le7(data_size), data.length());
  out = std::ranges::copy(data_size, out).out;
  // Property Data
  std::ranges::copy(std::views::transform(data, as_byte), out);

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::property_exchange::get_reply get_reply{
      .chunk =
          midi2::ci::property_exchange::chunk_info{
              .number_of_chunks = from_le7(total_chunks),
              .chunk_number = from_le7(chunk_number),
          },
      .request = midi2::to_underlying(request),
      .header = header,
      .data = data,
  };
  EXPECT_THAT(make_message(hdr, get_reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeSetPropertyData) {
  constexpr auto destination = 0x0F_b;

  constexpr auto request = 1_b;

  constexpr auto total_chunks = std::array{1_b, 0_b};
  constexpr auto chunk_number = std::array{1_b, 0_b};

  constexpr auto header_size = std::array{61_b, 0_b};
  constexpr auto data_size = std::array{16_b, 0_b};

  constexpr auto header = R"({"resource":"X-ProgramEdit","resId":"abcd","setPartial":true})"sv;
  constexpr auto data = R"({"/lfoSpeed":10})"sv;

  // clang-format off
  std::vector expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pe_set)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    request,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  // Header Size/Body
  ASSERT_EQ(from_le7(header_size), header.length());
  auto out = std::ranges::copy(header_size, std::back_inserter(expected)).out;
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Property Data Size/Body
  ASSERT_EQ(from_le7(data_size), data.length());
  out = std::ranges::copy(data_size, out).out;
  out = std::ranges::copy(std::views::transform(data, as_byte), out).out;

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::property_exchange::set set{
      .chunk =
          midi2::ci::property_exchange::chunk_info{
              .number_of_chunks = from_le7(total_chunks),
              .chunk_number = from_le7(chunk_number),
          },
      .request = midi2::to_underlying(request),
      .header = header,
      .data = data,
  };
  EXPECT_THAT(make_message(hdr, set), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeSetPropertyDataReply) {
  constexpr auto destination = 0x0F_b;

  constexpr auto request = 1_b;

  constexpr auto header_size = std::array{14_b, 0_b};
  constexpr auto total_chunks = std::array{1_b, 0_b};
  constexpr auto chunk_number = std::array{1_b, 0_b};
  constexpr auto property_data_size = std::array{0_b, 0_b};

  constexpr auto header = R"({"status":200})"sv;

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pe_set_reply)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    request,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };

  // Header Size
  ASSERT_EQ(from_le7(header_size), header.length());
  auto out = std::ranges::copy(header_size, std::back_inserter(expected)).out;
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

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::property_exchange::set_reply set_reply{
      .chunk =
          midi2::ci::property_exchange::chunk_info{
              .number_of_chunks = from_le7(total_chunks),
              .chunk_number = from_le7(chunk_number),
          },
      .request = midi2::to_underlying(request),
      .header = header,
      .data = {},
  };
  EXPECT_THAT(make_message(hdr, set_reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeSubscription) {
  constexpr auto destination = 0x0F_b;
  constexpr auto request = 1_b;
  constexpr auto total_chunks = std::array{1_b, 0_b};
  constexpr auto chunk_number = std::array{1_b, 0_b};
  constexpr auto header_size = std::array{46_b, 0_b};
  constexpr auto data_size = std::array{12_b, 0_b};

  constexpr auto header = R"({"command":"full","subscribeId":"sub32847623"})"sv;
  constexpr auto data = "multichannel"sv;

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pe_sub)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    request,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };
  // Header length/body
  ASSERT_EQ(from_le7(header_size), header.length());
  auto out = std::ranges::copy(header_size, std::back_inserter(expected)).out;
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Data length/body
  ASSERT_EQ(from_le7(data_size), data.length());
  out = std::ranges::copy(data_size, out).out;
  out = std::ranges::copy(std::views::transform(data, as_byte), out).out;

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::property_exchange::subscription subscription{
      .chunk =
          midi2::ci::property_exchange::chunk_info{
              .number_of_chunks = from_le7(total_chunks),
              .chunk_number = from_le7(chunk_number),
          },
      .request = midi2::to_underlying(request),
      .header = header,
      .data = data,
  };
  EXPECT_THAT(make_message(hdr, subscription), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeSubscriptionReply) {
  constexpr auto destination = 0x0F_b;
  constexpr auto request = 1_b;
  constexpr auto total_chunks = std::array{1_b, 0_b};
  constexpr auto chunk_number = std::array{1_b, 0_b};
  constexpr auto header = R"({"status":200,"subscribeId":"sub138047"})"sv;
  constexpr auto data = ""sv;
  constexpr auto header_size = to_le7(static_cast<std::uint16_t>(header.length()));
  constexpr auto property_data_size = std::array{0_b, 0_b};

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pe_sub_reply)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    request,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };

  // Header size/body
  ASSERT_EQ(from_le7(header_size), header.length());
  auto out = std::ranges::copy(header_size, std::back_inserter(expected)).out;
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Data length/body
  ASSERT_EQ(from_le7(property_data_size), data.length());
  out = std::ranges::copy(property_data_size, out).out;
  out = std::ranges::copy(std::views::transform(data, as_byte), out).out;

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::property_exchange::subscription_reply subscription_reply{
      .chunk =
          midi2::ci::property_exchange::chunk_info{
              .number_of_chunks = from_le7(total_chunks),
              .chunk_number = from_le7(chunk_number),
          },
      .request = midi2::to_underlying(request),
      .header = header,
      .data = data,
  };
  EXPECT_THAT(make_message(hdr, subscription_reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeNotify) {
  constexpr auto destination = 0x0F_b;
  constexpr auto request = 1_b;
  constexpr auto total_chunks = std::array{1_b, 0_b};
  constexpr auto chunk_number = std::array{1_b, 0_b};
  constexpr auto header = R"({"status":144})"sv;
  constexpr auto data = "data"sv;
  constexpr auto header_size = to_le7(static_cast<std::uint16_t>(header.size()));
  constexpr auto data_size = to_le7(static_cast<std::uint16_t>(data.size()));

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pe_notify)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    request,
  };
  // clang-format on

  auto const as_byte = [](char c) { return static_cast<std::byte>(c); };

  // Header size/body
  auto out = std::ranges::copy(header_size, std::back_inserter(expected)).out;
  out = std::ranges::copy(std::views::transform(header, as_byte), out).out;
  // Total/current chunk numbers
  ASSERT_EQ(from_le7(total_chunks), 1U);
  out = std::ranges::copy(total_chunks, out).out;
  ASSERT_EQ(from_le7(chunk_number), 1U);
  out = std::ranges::copy(chunk_number, out).out;
  // Data length/body
  out = std::ranges::copy(data_size, out).out;
  out = std::ranges::copy(std::views::transform(data, as_byte), out).out;

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::property_exchange::notify notify{
      .chunk =
          midi2::ci::property_exchange::chunk_info{
              .number_of_chunks = from_le7(total_chunks),
              .chunk_number = from_le7(chunk_number),
          },
      .request = midi2::to_underlying(request),
      .header = header,
      .data = data,
  };
  EXPECT_THAT(make_message(hdr, notify), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryCapabilities) {
  constexpr auto destination = 0x7F_b;

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pi_capability)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(hdr, midi2::ci::process_inquiry::capabilities{}), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryCapabilitiesReply) {
  constexpr auto destination = 0x7F_b;
  constexpr auto features = 0b0101010_b;

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pi_capability_reply)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    features
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(hdr, midi2::ci::process_inquiry::capabilities_reply{features}),
              testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryMidiMessageReport) {
  constexpr auto destination = 0x01_b;

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pi_mm_report)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    0x7F_b, // message data control
    0b00000111_b, // requested system messages
    0x00_b, // reserved
    0b00111111_b, // requested channel controller messages
    0b00011111_b, // requested note data messages
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::process_inquiry::midi_message_report report{
      .message_data_control = decltype(report)::control::full,
      // system messages
      .mtc_quarter_frame = 1,
      .song_position = 1,
      .song_select = 1,
      // channel controller messages
      .pitchbend = 1,
      .control_change = 1,
      .rpn_registered_controller = 1,
      .nrpn_assignable_controller = 1,
      .program_change = 1,
      .channel_pressure = 1,
      // note data messages
      .notes = 1,
      .poly_pressure = 1,
      .per_note_pitchbend = 1,
      .registered_per_note_controller = 1,
      .assignable_per_note_controller = 1,
  };
  EXPECT_THAT(make_message(hdr, report), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryMidiMessageReportReply) {
  constexpr auto destination = 0x01_b;

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pi_mm_report_reply)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    0b00000101_b, // requested system messages
    0x00_b, // reserved
    0b00101010_b, // requested channel controller messages
    0b00010010_b, // requested note data messages
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::process_inquiry::midi_message_report_reply reply{
      // system messages
      .mtc_quarter_frame = 1,
      .song_position = 0,
      .song_select = 1,
      // channel controller messages
      .pitchbend = 0,
      .control_change = 1,
      .rpn_registered_controller = 0,
      .nrpn_assignable_controller = 1,
      .program_change = 0,
      .channel_pressure = 1,
      // note data messages
      .notes = 0,
      .poly_pressure = 1,
      .per_note_pitchbend = 0,
      .registered_per_note_controller = 0,
      .assignable_per_note_controller = 1,
  };
  EXPECT_THAT(make_message(hdr, reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryMidiMessageReportEnd) {
  constexpr auto destination = 0x01_b;

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    static_cast<std::byte>(midi2::to_underlying(midi2::ci::message::pi_mm_report_end)),
    2_b, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::header hdr{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(hdr, midi2::ci::process_inquiry::midi_message_report_end{}),
              testing::ElementsAreArray(expected));
}

}  // end anonymous namespace
