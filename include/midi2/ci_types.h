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

using byte_array_3 = std::array<std::byte, 3>;
using byte_array_4 = std::array<std::byte, 4>;

#define MIDI2_DISCOVERY_V1                                       \
  STRUCT_START(discovery_v1)                                     \
  STRUCT_MEMBER(discovery_v1, 0, byte_array_3, manufacturer)     \
  STRUCT_MEMBER(discovery_v1, 3, std::uint16_t, family)          \
  STRUCT_MEMBER(discovery_v1, 5, std::uint16_t, model)           \
  STRUCT_MEMBER(discovery_v1, 7, byte_array_4, version)          \
  STRUCT_MEMBER(discovery_v1, 11, std::uint8_t, capability)      \
  STRUCT_MEMBER(discovery_v1, 12, std::uint32_t, max_sysex_size) \
  STRUCT_END(discovery_v1, 16)

#define MIDI2_DISCOVERY_V2                                      \
  STRUCT_START(discovery_v2)                                    \
  STRUCT_MEMBER(discovery_v2, 0, discovery_v1, v1)              \
  STRUCT_MEMBER(discovery_v2, 16, std::uint8_t, output_path_id) \
  STRUCT_END(discovery_v2, 17)

#define MIDI2_DISCOVERY_REPLY_V1                                       \
  STRUCT_START(discovery_reply_v1)                                     \
  STRUCT_MEMBER(discovery_reply_v1, 0, byte_array_3, manufacturer)     \
  STRUCT_MEMBER(discovery_reply_v1, 3, std::uint16_t, family)          \
  STRUCT_MEMBER(discovery_reply_v1, 5, std::uint16_t, model)           \
  STRUCT_MEMBER(discovery_reply_v1, 7, byte_array_4, version)          \
  STRUCT_MEMBER(discovery_reply_v1, 11, std::uint8_t, capability)      \
  STRUCT_MEMBER(discovery_reply_v1, 12, std::uint32_t, max_sysex_size) \
  STRUCT_END(discovery_reply_v1, 16)

#define MIDI2_DISCOVERY_REPLY_V2                                      \
  STRUCT_START(discovery_reply_v2)                                    \
  STRUCT_MEMBER(discovery_reply_v2, 0, discovery_reply_v1, v1)        \
  STRUCT_MEMBER(discovery_reply_v2, 16, std::uint8_t, output_path_id) \
  STRUCT_MEMBER(discovery_reply_v2, 17, std::uint8_t, function_block) \
  STRUCT_END(discovery_reply_v2, 18)

#define MIDI2_ALL_CI_TYPES \
  MIDI2_DISCOVERY_V1       \
  MIDI2_DISCOVERY_V2       \
  MIDI2_DISCOVERY_REPLY_V1 \
  MIDI2_DISCOVERY_REPLY_V2

// ****
// Write the packed structures
// ****
#define STRUCT_START(structname) \
  _Pragma("pack(push, 1)")       \
  struct structname {
#define STRUCT_MEMBER(structname, offset, type, name) type name{};
#define STRUCT_END(structname, size) \
  };                                 \
  _Pragma("pack(pop)")

namespace packed {
MIDI2_ALL_CI_TYPES
}

#undef STRUCT_END
#undef STRUCT_MEMBER
#undef STRUCT_START

// ****
// Write static assertions to check the layout of the packed structures
// ****
#define STRUCT_START(structname)
#define STRUCT_MEMBER(structname, offset, type, name)                              \
  static_assert(offsetof(structname, name) == (offset),                            \
                "member " #name " of " #structname " was not at offset " #offset); \
  static_assert(std::is_trivially_copyable_v<type>, "member " #name " of " #structname " was not TriviallyCopyable");
#define STRUCT_END(structname, size) \
  static_assert(sizeof(structname) == (size), "size of struct " #structname " was not " #size);

namespace packed {
MIDI2_ALL_CI_TYPES
}

#undef STRUCT_END
#undef STRUCT_MEMBER
#undef STRUCT_START

// ****
// Write the un-packed structure
// ****
#define MIDI2_PACKED_NAME(structname) packed::structname

#define STRUCT_START(structname)                                               \
  struct structname {                                                          \
    constexpr structname() = default;                                          \
    constexpr structname(structname const &) = default;                        \
    constexpr structname(structname &&) noexcept = default;                    \
    constexpr explicit structname(MIDI2_PACKED_NAME(structname) const &);      \
    constexpr ~structname() noexcept = default;                                \
    constexpr structname &operator=(structname const &) = default;             \
    constexpr structname &operator=(structname &&) noexcept = default;         \
    constexpr structname &operator=(MIDI2_PACKED_NAME(structname) const &rhs); \
    constexpr bool operator==(structname const &) const = default;
#define STRUCT_MEMBER(structname, offset, type, name) type name;
#define STRUCT_END(structname, size) };

MIDI2_ALL_CI_TYPES

#undef STRUCT_END
#undef STRUCT_MEMBER
#undef STRUCT_START

// ****
// Write a constructor which will initialize from the packed structure
// ****
#define STRUCT_START(structname) constexpr structname::structname(MIDI2_PACKED_NAME(structname) const &other) {
#define STRUCT_MEMBER(structname, offset, type, name) name = other.name;
#define STRUCT_END(structname, size) }

MIDI2_ALL_CI_TYPES

#undef STRUCT_END
#undef STRUCT_MEMBER
#undef STRUCT_START

// ****
// Write operator= to assign from the packed structure
// ****
#define STRUCT_START(structname) constexpr structname &structname::operator=(MIDI2_PACKED_NAME(structname) const &rhs) {
#define STRUCT_MEMBER(structname, offset, type, name) name = rhs.name;
#define STRUCT_END(structname, size) return *this; }

MIDI2_ALL_CI_TYPES

#undef STRUCT_END
#undef STRUCT_MEMBER
#undef STRUCT_START

using discovery = discovery_v2;
using discovery_reply = discovery_reply_v2;

namespace packed {
using discovery_current = discovery_v2;
using discovery_reply_current = discovery_reply_v2;
}  // namespace packed

}  // end namespace midi2::ci

#endif  // MIDI2_CI_TYPES_H
