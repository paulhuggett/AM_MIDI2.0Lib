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

using midi2::ci::byte_array_5;
using midi2::ci::from_le7;

class CICreateMessage : public testing::Test {
protected:
  static constexpr auto broadcast_muid_ =
      std::array{std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F}};
  static constexpr auto sender_muid_ = std::array{std::byte{0x7F}, std::byte{0x7E}, std::byte{0x7D}, std::byte{0x7C}};
  static constexpr auto destination_muid_ =
      std::array{std::byte{0x62}, std::byte{0x16}, std::byte{0x63}, std::byte{0x26}};

  template <typename T> struct trivial_sentinel {
    constexpr bool operator==(T) const { return false; }
    friend constexpr bool operator==(T, trivial_sentinel) { return false; }
  };

  template <typename Content>
  static std::vector<std::byte> make_message(struct midi2::ci::params const& params, Content const& content) {
    std::vector<std::byte> message;
    auto out_it = std::back_inserter(message);
    midi2::ci::create_message(out_it, trivial_sentinel<decltype(out_it)>{}, params, content);
    return message;
  }
};

TEST_F(CICreateMessage, DiscoveryV1) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto manufacturer = std::array{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}};
  constexpr auto family = std::array{std::byte{0x67}, std::byte{0x79}};
  constexpr auto model = std::array{std::byte{0x6B}, std::byte{0x5D}};
  constexpr auto version = std::array{std::byte{0x4E}, std::byte{0x3C}, std::byte{0x2A}, std::byte{0x18}};
  constexpr auto capability = std::byte{0x7F};
  constexpr auto max_sysex_size = std::array{std::byte{0x76}, std::byte{0x54}, std::byte{0x32}, std::byte{0x10}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
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
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(device_id), .version = 1, .remote_muid = 0, .local_muid = midi2::ci_broadcast};
  constexpr midi2::ci::discovery discovery{.manufacturer = midi2::ci::from_array(manufacturer),
                                           .family = from_le7(family),
                                           .model = from_le7(model),
                                           .version = midi2::ci::from_array(version),
                                           .capability = midi2::to_underlying(capability),
                                           .max_sysex_size = from_le7(max_sysex_size)};
  EXPECT_THAT(make_message(params, discovery), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, DiscoveryV2) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto manufacturer = std::array{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}};
  constexpr auto family = std::array{std::byte{0x67}, std::byte{0x79}};
  constexpr auto model = std::array{std::byte{0x6B}, std::byte{0x5D}};
  constexpr auto version = std::array{std::byte{0x4E}, std::byte{0x3C}, std::byte{0x2A}, std::byte{0x18}};
  constexpr auto capability = std::byte{0x7F};
  constexpr auto max_sysex_size = std::array{std::byte{0x76}, std::byte{0x54}, std::byte{0x32}, std::byte{0x10}};
  constexpr auto output_path_id = std::byte{0x71};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
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

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(device_id), .version = 2, .remote_muid = 0, .local_muid = midi2::ci_broadcast};
  constexpr midi2::ci::discovery discovery{.manufacturer = midi2::ci::from_array(manufacturer),
                                           .family = from_le7(family),
                                           .model = from_le7(model),
                                           .version = midi2::ci::from_array(version),
                                           .capability = midi2::to_underlying(capability),
                                           .max_sysex_size = from_le7(max_sysex_size),
                                           .output_path_id = midi2::to_underlying(output_path_id)};
  EXPECT_THAT(make_message(params, discovery), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, DiscoveryReplyV2) {
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
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
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

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(device_id), .version = 2, .remote_muid = 0, .local_muid = midi2::ci_broadcast};
  constexpr midi2::ci::discovery_reply reply{.manufacturer = midi2::ci::from_array(manufacturer),
                                             .family = from_le7(family),
                                             .model = from_le7(model),
                                             .version = midi2::ci::from_array(version),
                                             .capability = midi2::to_underlying(capability),
                                             .max_sysex_size = from_le7(max_sysex_size),
                                             .output_path_id = midi2::to_underlying(output_path_id),
                                             .function_block = midi2::to_underlying(function_block)};
  EXPECT_THAT(make_message(params, reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, EndpointInfo) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto status = std::uint8_t{0b0101010};
  constexpr std::array const receiver_muid{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to MIDI Port
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x72}, // Universal System Exclusive Sub-ID#2: Endpoint Information
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    std::byte{status}, // Status
  };
  // clang-format on

  constexpr midi2::ci::params params{.device_id = midi2::to_underlying(device_id),
                                     .version = 1,
                                     .remote_muid = from_le7(sender_muid_),
                                     .local_muid = from_le7(receiver_muid)};
  constexpr midi2::ci::endpoint_info endpoint_info{.status = status};
  EXPECT_THAT(make_message(params, endpoint_info), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, EndpointInfoReply) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto status = std::byte{0b0101010};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};
  constexpr auto length = std::array{std::byte{0x08}, std::byte{0x00}};
  constexpr auto information = std::array{
      std::byte{2},  std::byte{3},  std::byte{5},  std::byte{7},  // Information data
      std::byte{11}, std::byte{13}, std::byte{17}, std::byte{19},
  };
  ASSERT_EQ(from_le7(length), information.size());

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x73}, // Universal System Exclusive Sub-ID#2: Reply to Endpoint Information
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    status, // Status
    length[0], length[1], // Length of following data (LSB first)
    information[0], information[1], information[2], information[3],
    information[4], information[5],  information[6], information[7],
  };
  // clang-format on

  constexpr midi2::ci::params params{.device_id = midi2::to_underlying(device_id),
                                     .version = 1,
                                     .remote_muid = from_le7(sender_muid_),
                                     .local_muid = from_le7(receiver_muid)};

  // Test create_message()
  midi2::ci::endpoint_info_reply const reply{.status = from_le7(status), .information = information};
  EXPECT_THAT(make_message(params, reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, InvalidateMuid) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr std::array const receiver_muid{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};
  constexpr std::array const target_muid{std::byte{0x21}, std::byte{0x43}, std::byte{0x75}, std::byte{0x71}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    midi2::s7_universal_nrt, // Universal System Exclusive Sub-ID#2: Invalidate MUID
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    target_muid[0], target_muid[1], target_muid[2], target_muid[3], // Target MUID (the MUID to invalidate) (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::params params{.device_id = midi2::to_underlying(device_id),
                                     .version = 1,
                                     .remote_muid = from_le7(sender_muid_),
                                     .local_muid = from_le7(receiver_muid)};
  constexpr midi2::ci::invalidate_muid invalidate_muid{.target_muid = from_le7(target_muid)};
  EXPECT_THAT(make_message(params, invalidate_muid), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, Ack) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  constexpr auto original_id = std::byte{0x34};
  constexpr auto ack_status_code = std::byte{0x00};
  constexpr auto ack_status_data = std::byte{0x7F};
  constexpr auto ack_details =
      std::array{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
  constexpr auto text_length = std::array{std::byte{0x05}, std::byte{0x00}};
  static constexpr auto text =
      std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};
  ASSERT_EQ(from_le7(text_length), text.size());

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x7D}, // Universal System Exclusive Sub-ID#2: MIDI-CI ACK
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
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

  constexpr midi2::ci::params params{
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
  EXPECT_THAT(make_message(params, ack), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, NakV1) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x7F}, // Universal System Exclusive Sub-ID#2: MIDI-CI NAK
    std::byte{1}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(device_id),
      .version = 1,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(receiver_muid),
  };
  EXPECT_THAT(make_message(params, midi2::ci::nak{}), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, NakV2) {
  constexpr auto device_id = std::byte{0x7F};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  constexpr auto original_id = std::byte{0x34};
  constexpr auto nak_status_code = std::byte{0x00};
  constexpr auto nak_status_data = std::byte{0x7F};
  constexpr auto nak_details =
      std::array{std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}, std::byte{0x05}};
  constexpr auto text_length = std::array{std::byte{0x05}, std::byte{0x00}};
  static constexpr auto text =
      std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    device_id, // Device ID: 0x7F = to Function Block
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x7F}, // Universal System Exclusive Sub-ID#2: MIDI-CI NAK
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
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

  constexpr midi2::ci::params params{
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
  EXPECT_THAT(make_message(params, nak), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileInquiry) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x20}, // Universal System Exclusive Sub-ID#2: Profile Inquiry
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(receiver_muid),
  };
  EXPECT_THAT(make_message(params, midi2::ci::profile_configuration::inquiry{}), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileInquiryReply) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto receiver_muid = std::array{std::byte{0x12}, std::byte{0x34}, std::byte{0x5E}, std::byte{0x0F}};

  constexpr auto enabled = std::array<byte_array_5, 2>{
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}},
      byte_array_5{std::byte{0x76}, std::byte{0x65}, std::byte{0x54}, std::byte{0x43}, std::byte{0x32}},
  };
  constexpr auto disabled = std::array<byte_array_5, 1>{
      byte_array_5{std::byte{0x71}, std::byte{0x61}, std::byte{0x51}, std::byte{0x41}, std::byte{0x31}},
  };
  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x21}, // Universal System Exclusive Sub-ID#2: Profile Inquiry Reply
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    receiver_muid[0], receiver_muid[1], receiver_muid[2], receiver_muid[3], // Destination MUID (LSB first)
    std::byte{2}, std::byte{0},
    enabled[0][0], enabled[0][1], enabled[0][2], enabled[0][3], enabled[0][4],
    enabled[1][0], enabled[1][1], enabled[1][2], enabled[1][3], enabled[1][4],
    std::byte{1}, std::byte{0},
    disabled[0][0], disabled[0][1], disabled[0][2], disabled[0][3], disabled[0][4],
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(receiver_muid),
  };
  EXPECT_THAT(make_message(params, midi2::ci::profile_configuration::inquiry_reply{enabled, disabled}),
              testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileAdded) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x26}, // Universal System Exclusive Sub-ID#2: Profile Added Report
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid_[0], broadcast_muid_[1], broadcast_muid_[2], broadcast_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile being added
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(broadcast_muid_),
  };
  constexpr midi2::ci::profile_configuration::added added{.pid = pid};

  EXPECT_THAT(make_message(params, added), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileRemoved) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x27}, // Universal System Exclusive Sub-ID#2: Profile Removed Report
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid_[0], broadcast_muid_[1], broadcast_muid_[2], broadcast_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile being removed
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(broadcast_muid_),
  };
  constexpr midi2::ci::profile_configuration::removed removed{.pid = pid};

  EXPECT_THAT(make_message(params, removed), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileDetails) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x28}, // Universal System Exclusive Sub-ID#2: Profile Details Inquiry
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    std::byte{0x23}, // Inquiry target
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  constexpr midi2::ci::profile_configuration::details details{
      .pid = pid,
      .target = 0x23,
  };
  EXPECT_THAT(make_message(params, details), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileDetailsReply) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto target = std::byte{0x23};
  constexpr auto data_length = std::array{std::byte{0x05}, std::byte{0x00}};
  constexpr auto data = std::array{std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x29}, // Universal System Exclusive Sub-ID#2: Profile Details Reply
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    target, // Inquiry target
    data_length[0], data_length[1], // Inquiry target data length (LSB first)
    data[0], data[1], data[2], data[3], data[4],
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  using midi2::ci::profile_configuration::details_reply;

  EXPECT_THAT(make_message(params, details_reply{pid, midi2::to_underlying(target), data}),
              testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileOn) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto channels = std::array{std::byte{0x23}, std::byte{0x00}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x22}, // Universal System Exclusive Sub-ID#2: Set Profile On
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    channels[0], channels[1], // Number of channels
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(params, midi2::ci::profile_configuration::on{pid, from_le7(channels)}),
              testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileOff) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto reserved = std::array{std::byte{0x00}, std::byte{0x00}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x23}, // Universal System Exclusive Sub-ID#2: Set Profile Off
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    reserved[0], reserved[1],
  };
  // clang-format on
  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(params, midi2::ci::profile_configuration::off{pid}), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProfileEnabled) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto pid =
      byte_array_5{std::byte{0x12}, std::byte{0x23}, std::byte{0x34}, std::byte{0x45}, std::byte{0x56}};
  constexpr auto num_channels = std::array{std::byte{0x22}, std::byte{0x11}};

  // clang-format off
  constexpr std::array expected{
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x24}, // Universal System Exclusive Sub-ID#2: Profile Enabled Report
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    broadcast_muid_[0], broadcast_muid_[1], broadcast_muid_[2], broadcast_muid_[3], // Destination MUID (LSB first)
    pid[0], pid[1], pid[2], pid[3], pid[4], // Profile ID of profile
    num_channels[0], num_channels[1], // Number of channels
  };
  // clang-format on
  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(broadcast_muid_),
  };
  constexpr midi2::ci::profile_configuration::enabled enabled{
      .pid = pid,
      .num_channels = from_le7(num_channels),
  };

  EXPECT_THAT(make_message(params, enabled), testing::ElementsAreArray(expected));
}

using namespace std::string_view_literals;

TEST_F(CICreateMessage, PropertyExchangeGetPropertyData) {
  constexpr auto destination = std::byte{0x0F};

  constexpr auto request = std::byte{1};

  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};

  constexpr auto header_size = std::array{std::byte{25}, std::byte{0}};
  constexpr auto data_size = std::array{std::byte{0}, std::byte{0}};

  constexpr auto header = R"({"resource":"DeviceInfo"})"sv;

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x34}, // Universal System Exclusive Sub-ID#2: Inquiry: Get Property Data
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
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

  constexpr midi2::ci::params params{
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
  };
  EXPECT_THAT(make_message(params, get), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeGetPropertyDataReply) {
  constexpr auto destination = std::byte{0x0F};

  constexpr auto request = std::byte{1};

  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};

  constexpr auto header_size = std::array{std::byte{14}, std::byte{0}};
  constexpr auto data_size = std::array{std::byte{61}, std::byte{0}};
  constexpr auto header = R"({"status":200})"sv;
  constexpr auto data = R"({"manufacturerId":[125,0,0],"manufacturer":"Educational Use"})"sv;

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x35}, // Universal System Exclusive Sub-ID#2: Inquiry: Reply to Get Property Data
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
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

  constexpr midi2::ci::params params{
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
  EXPECT_THAT(make_message(params, get_reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeSetPropertyData) {
  constexpr auto destination = std::byte{0x0F};

  constexpr auto request = std::byte{1};

  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};

  constexpr auto header_size = std::array{std::byte{61}, std::byte{0}};
  constexpr auto data_size = std::array{std::byte{16}, std::byte{0}};

  constexpr auto header = R"({"resource":"X-ProgramEdit","resId":"abcd","setPartial":true})"sv;
  constexpr auto data = R"({"/lfoSpeed":10})"sv;

  // clang-format off
  std::vector expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x36}, // Universal System Exclusive Sub-ID#2: Inquiry: Set Property Data
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
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

  constexpr midi2::ci::params params{
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
  EXPECT_THAT(make_message(params, set), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeSetPropertyDataReply) {
  constexpr auto destination = std::byte{0x0F};

  constexpr auto request = std::byte{1};

  constexpr auto header_size = std::array{std::byte{14}, std::byte{0}};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto property_data_size = std::array{std::byte{0}, std::byte{0}};

  constexpr auto header = R"({"status":200})"sv;

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x37}, // Universal System Exclusive Sub-ID#2: Inquiry: Reply to Set Property Data
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
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

  constexpr midi2::ci::params params{
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
  };
  EXPECT_THAT(make_message(params, set_reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeSubscription) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto request = std::byte{1};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto header_size = std::array{std::byte{46}, std::byte{0}};
  constexpr auto data_size = std::array{std::byte{12}, std::byte{0}};

  constexpr auto header = R"({"command":"full","subscribeId":"sub32847623"})"sv;
  constexpr auto data = "multichannel"sv;

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x38}, // Universal System Exclusive Sub-ID#2: Subscription
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
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

  constexpr midi2::ci::params params{
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
  EXPECT_THAT(make_message(params, subscription), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeSubscriptionReply) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto request = std::byte{1};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto header = R"({"status":200,"subscribeId":"sub138047"})"sv;
  constexpr auto data = ""sv;
  constexpr auto header_size = midi2::ci::to_le7(static_cast<std::uint16_t>(header.length()));
  constexpr auto property_data_size = std::array{std::byte{0}, std::byte{0}};

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x39}, // Universal System Exclusive Sub-ID#2: Subscription Reply
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
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

  constexpr midi2::ci::params params{
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
  EXPECT_THAT(make_message(params, subscription_reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, PropertyExchangeNotify) {
  constexpr auto destination = std::byte{0x0F};
  constexpr auto request = std::byte{1};
  constexpr auto total_chunks = std::array{std::byte{1}, std::byte{0}};
  constexpr auto chunk_number = std::array{std::byte{1}, std::byte{0}};
  constexpr auto header = R"({"status":144})"sv;
  constexpr auto data = "data"sv;
  constexpr auto header_size = midi2::ci::to_le7(static_cast<std::uint16_t>(header.size()));
  constexpr auto data_size = midi2::ci::to_le7(static_cast<std::uint16_t>(data.size()));

  // clang-format off
  std::vector<std::byte> expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x3F}, // Universal System Exclusive Sub-ID#2: Notify
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
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

  constexpr midi2::ci::params params{
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
  EXPECT_THAT(make_message(params, notify), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryCapabilities) {
  constexpr auto destination = std::byte{0x7F};

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x40}, // Universal System Exclusive Sub-ID#2: Inquiry: Process Inquiry Capabilities
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(params, midi2::ci::process_inquiry::capabilities{}), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryCapabilitiesReply) {
  constexpr auto destination = std::byte{0x7F};
  constexpr auto features = std::byte{0b0101010};

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x41}, // Universal System Exclusive Sub-ID#2: Inquiry: Process Inquiry Capabilities Reply
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    features
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(params, midi2::ci::process_inquiry::capabilities_reply{features}),
              testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryMidiMessageReport) {
  constexpr auto destination = std::byte{0x01};

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x42}, // Universal System Exclusive Sub-ID#2: Inquiry: MIDI Message Report
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    std::byte{0x7F}, // message data control
    std::byte{0b00000111}, // requested system messages
    std::byte{0x00}, // reserved
    std::byte{0b00111111}, // requested channel controller messages
    std::byte{0b00011111}, // requested note data messages
  };
  // clang-format on

  constexpr midi2::ci::params params{
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
  EXPECT_THAT(make_message(params, report), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryMidiMessageReportReply) {
  constexpr auto destination = std::byte{0x01};

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x43}, // Universal System Exclusive Sub-ID#2: Inquiry: MIDI Message Report Reply
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)

    std::byte{0b00000101}, // requested system messages
    std::byte{0x00}, // reserved
    std::byte{0b00101010}, // requested channel controller messages
    std::byte{0b00010010}, // requested note data messages
  };
  // clang-format on

  constexpr midi2::ci::params params{
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
  EXPECT_THAT(make_message(params, reply), testing::ElementsAreArray(expected));
}

TEST_F(CICreateMessage, ProcessInquiryMidiMessageReportEnd) {
  constexpr auto destination = std::byte{0x01};

  // clang-format off
  constexpr std::array expected {
    midi2::s7_universal_nrt, // Universal System Exclusive
    destination, // Destination
    midi2::s7_midi_ci, // Universal System Exclusive Sub-ID#1: MIDI-CI
    std::byte{0x44}, // Universal System Exclusive Sub-ID#2: Inquiry: MIDI Message Report End
    std::byte{2}, // 1 byte MIDI-CI Message Version/Format
    sender_muid_[0], sender_muid_[1], sender_muid_[2], sender_muid_[3], // 4 bytes Source MUID (LSB first)
    destination_muid_[0], destination_muid_[1], destination_muid_[2], destination_muid_[3], // Destination MUID (LSB first)
  };
  // clang-format on

  constexpr midi2::ci::params params{
      .device_id = midi2::to_underlying(destination),
      .version = 2,
      .remote_muid = from_le7(sender_muid_),
      .local_muid = from_le7(destination_muid_),
  };
  EXPECT_THAT(make_message(params, midi2::ci::process_inquiry::midi_message_report_end{}),
              testing::ElementsAreArray(expected));
}

}  // end anonymous namespace
