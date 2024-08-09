//
//  ci_types.h
//  libmidi2
//
//  Created by Paul Bowen-Huggett on 09/08/2024.
//

#ifndef MIDI2_CI_TYPES_H
#define MIDI2_CI_TYPES_H

#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>

namespace midi2::ci {

constexpr auto FUNCTION_BLOCK = std::uint8_t{0x7F};

using reqId = std::tuple<uint32_t, std::byte>;  // muid-requestId

struct MIDICI {
  bool operator==(MIDICI const&) const = default;

  std::uint8_t umpGroup = 0xFF;
  std::uint8_t deviceId = FUNCTION_BLOCK;
  std::uint8_t ciType = 0xFF;
  std::uint8_t ciVer = 1;
  std::uint32_t remoteMUID = 0;
  std::uint32_t localMUID = 0;
  std::optional<reqId> _peReqIdx;

  std::uint8_t totalChunks = 0;
  std::uint8_t numChunk = 0;
  std::uint8_t partialChunkCount = 0;
  std::byte requestId{0xFF};
};

struct __attribute__((packed)) discovery_v1 {
  bool operator==(discovery_v1 const&) const = default;

  std::array<std::byte, 3> manufacturer;  ///< Device manufacturer (system exclusive ID number)
  std::uint16_t family;                   ///< Device family
  std::uint16_t model;                    ///< Device family model number
  std::array<std::byte, 4> version;       ///< Software revision level (format is device specific)
  std::uint8_t capability;                ///< Capability inquiry category supported
  std::uint32_t max_sysex_size;           ///< Receivable maximum sysex message size
};
static_assert(sizeof(discovery_v1) == 16);
static_assert(offsetof(discovery_v1, manufacturer) == 0);
static_assert(offsetof(discovery_v1, family) == 3);
static_assert(offsetof(discovery_v1, model) == 5);
static_assert(offsetof(discovery_v1, version) == 7);
static_assert(offsetof(discovery_v1, capability) == 11);
static_assert(offsetof(discovery_v1, max_sysex_size) == 12);

struct __attribute__((packed)) discovery_v2 : public discovery_v1 {
  bool operator==(discovery_v2 const&) const = default;

  std::uint8_t output_path_id = 0;  ///< Initiator's output path ID
};

static_assert(sizeof(discovery_v2) == 17);
static_assert(offsetof(discovery_v2, manufacturer) == 0);
static_assert(offsetof(discovery_v2, output_path_id) == 16);

using discovery_current = discovery_v2;

struct __attribute__((packed)) discovery_reply_v1 {
  bool operator==(discovery_reply_v1 const&) const = default;

  std::array<std::byte, 3> manufacturer;  ///< Device manufacturer (system exclusive ID number)
  std::uint16_t family;                   ///< Device family
  std::uint16_t model;                    ///< Device family model number
  std::array<std::byte, 4> version;       ///< Software revision level (format is device specific)
  std::uint8_t capability;                ///< Capability inquiry category supported
  std::uint32_t max_sysex_size;           ///< Receivable maximum sysex message size
};
static_assert(sizeof(discovery_reply_v1) == 16);
static_assert(offsetof(discovery_reply_v1, manufacturer) == 0);
static_assert(offsetof(discovery_reply_v1, family) == 3);
static_assert(offsetof(discovery_reply_v1, model) == 5);
static_assert(offsetof(discovery_reply_v1, version) == 7);
static_assert(offsetof(discovery_reply_v1, capability) == 11);
static_assert(offsetof(discovery_reply_v1, max_sysex_size) == 12);

struct __attribute__((packed)) discovery_reply_v2 : public discovery_reply_v1 {
  bool operator==(discovery_reply_v2 const&) const = default;

  std::uint8_t output_path_id = 0;  ///< Initiator's output path ID
  std::uint8_t function_block = 0;  ///< Function block
};

static_assert(sizeof(discovery_reply_v2) == 18);
static_assert(offsetof(discovery_reply_v2, manufacturer) == 0);
static_assert(offsetof(discovery_reply_v2, output_path_id) == 16);
static_assert(offsetof(discovery_reply_v2, function_block) == 17);

using discovery_reply_current = discovery_reply_v2;

}  // end namespace midi2::ci

#endif  // MIDI2_CI_TYPES_H
