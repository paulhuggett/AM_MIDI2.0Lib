/**********************************************************
 * MIDI 2.0 Library
 * Author: Andrew Mee
 *
 * MIT License
 * Copyright 2022 Andrew Mee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ********************************************************/

#ifndef MIDI2_MIDICIPROCESSOR_HPP
#define MIDI2_MIDICIPROCESSOR_HPP

#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>

#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

using MIDICI = ci::MIDICI;

using profile_span = std::span<std::byte, 5>;
constexpr auto bytes = std::span<std::byte>{};

template <typename T> concept discovery_backend = requires(T && v) {
  { v.check_muid(std::uint8_t{} /*group*/, std::uint32_t{} /*muid*/) } -> std::convertible_to<bool>;

  { v.discovery(MIDICI{}, ci::discovery{}) } -> std::same_as<void>;
  { v.discovery_reply(MIDICI{}, ci::discovery_reply{}) } -> std::same_as<void>;
  { v.endpoint_info(MIDICI{}, ci::endpoint_info{}) } -> std::same_as<void>;
  { v.endpoint_info_reply(MIDICI{}, ci::endpoint_info_reply{}) } -> std::same_as<void>;
  { v.invalidate_muid(MIDICI{}, ci::invalidate_muid{}) } -> std::same_as<void>;
  { v.ack(MIDICI{}, ci::ack{}) } -> std::same_as<void>;
  { v.nak(MIDICI{}, ci::nak{}) } -> std::same_as<void>;

  { v.unknown_midici(MIDICI{}, std::byte{}) } -> std::same_as<void>;
};

template <typename T> concept profile_backend = requires(T && v) {
  { v.inquiry(MIDICI{}) } -> std::same_as<void>;
  { v.inquiry_reply(MIDICI{}, ci::profile_inquiry_reply{}) } -> std::same_as<void>;
  { v.added(MIDICI{}, ci::profile_added{}) } -> std::same_as<void>;
  { v.removed(MIDICI{}, ci::profile_removed{}) } -> std::same_as<void>;
  { v.details_inquiry(MIDICI{}, ci::profile_details_inquiry{}) } -> std::same_as<void>;
  { v.details_reply(MIDICI{}, ci::profile_details_reply{}) } -> std::same_as<void>;
  { v.on(MIDICI{}, ci::profile_on{}) } -> std::same_as<void>;
  { v.off(MIDICI{}, ci::profile_off{}) } -> std::same_as<void>;
  { v.enabled(MIDICI{}, ci::profile_enabled{}) } -> std::same_as<void>;
  { v.disabled(MIDICI{}, ci::profile_disabled{}) } -> std::same_as<void>;
  { v.specific_data(MIDICI{}, ci::profile_specific_data{}) } -> std::same_as<void>;
};

template <typename T> concept property_exchange_backend = requires(T && v) {
  { v.capabilities(MIDICI{}, ci::pe_capabilities{}) } -> std::same_as<void>;
  { v.capabilities_reply(MIDICI{}, ci::pe_capabilities_reply{}) } -> std::same_as<void>;
  { v.get_reply(MIDICI{}, ci::pe_chunk_info{}, std::span<char const>{}, std::span<std::byte const>{}) } -> std::same_as<void>;

#if 0
  { v.recvPEGetInquiry(MIDICI{}, std::string{} /*details*/) } -> std::same_as<void>;
  { v.recvPESetReply(MIDICI{}, std::string{} /*details*/) } -> std::same_as<void>;
  { v.recvPESubReply(MIDICI{}, std::string{} /*details*/) } -> std::same_as<void>;
  { v.recvPENotify(MIDICI{}, std::string{} /*details*/) } -> std::same_as<void>;
  {
    v.recvPEGetReply(MIDICI{}, std::string{} /*requestDetails*/, std::span<std::byte>{} /*body*/,
                     bool{} /*lastByteOfChunk*/, bool{} /*lastByteOfSet*/)
  } -> std::same_as<void>;
  {
    v.recvPESetInquiry(MIDICI{}, std::string{} /*requestDetails*/, std::span<std::byte>{} /*body*/,
                       bool{} /*lastByteOfChunk*/, bool{} /*lastByteOfSet*/)
  } -> std::same_as<void>;
  {
    v.recvPESubInquiry(MIDICI{}, std::string{} /*requestDetails*/, std::span<std::byte>{} /*body*/,
                       bool{} /*lastByteOfChunk*/, bool{} /*lastByteOfSet*/)
  } -> std::same_as<void>;
#endif
};

class ci_callbacks {
public:
  ci_callbacks() = default;
  ci_callbacks(ci_callbacks const &) = default;
  ci_callbacks(ci_callbacks &&) noexcept = default;
  virtual ~ci_callbacks() noexcept = default;

  virtual bool check_muid(std::uint8_t /*group*/, std::uint32_t /*muid*/) { return false; }

  virtual void discovery(MIDICI const &, ci::discovery const &) { /* do nothing */ }
  virtual void discovery_reply(MIDICI const &, ci::discovery_reply const &) { /* do nothing */ }
  virtual void endpoint_info(MIDICI const &, ci::endpoint_info const &) { /* do nothing*/ }
  virtual void endpoint_info_reply(MIDICI const &, ci::endpoint_info_reply const &) { /* do nothing */ }
  virtual void invalidate_muid(MIDICI const &, ci::invalidate_muid const &) { /* do nothing */ }
  virtual void ack(MIDICI const &, ci::ack const &) { /* do nothing */ }
  virtual void nak(MIDICI const &, ci::nak const &) { /* do nothing */ }

  virtual void unknown_midici(MIDICI const &, std::byte s7) { (void)s7; }

  // Property Exchange
  virtual void recvPEGetInquiry(MIDICI const &, std::string const & /*requestDetails*/) { /* do nothing */ }
  virtual void recvPESetReply(MIDICI const &, ci::pe_chunk_info const &, std::string const & /*requestDetails*/) { /* do nothing */ }
  virtual void recvPESubReply(MIDICI const &, std::string const & /*requestDetails*/) { /* do nothing */ }
  virtual void recvPENotify(MIDICI const &, std::string const & /*requestDetails*/) { /* do nothing */ }
  virtual void recvPESetInquiry(MIDICI const &, std::string const & /*requestDetails*/, std::span<std::byte> /*body*/,
                                bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) { /* do nothing */ }
  virtual void recvPESubInquiry(MIDICI const &, std::string const & /*requestDetails*/, std::span<std::byte> /*body*/,
                                bool /*lastByteOfChunk*/, bool /*lastByteOfSet*/) { /* do nothing */ }

  // Process Inquiry
  virtual void recvPICapabilities(MIDICI const &) { /* do nothing */ }
  virtual void recvPICapabilitiesReply(MIDICI const &, std::byte /*supportedFeatures*/) { /* do nothing */ }
  virtual void recvPIMMReport(MIDICI const &, std::byte /*MDC*/, std::byte /*systemBitmap*/,
                              std::byte /*chanContBitmap*/, std::byte /*chanNoteBitmap*/) { /* do nothing */ }
  virtual void recvPIMMReportReply(MIDICI const &, std::byte /*systemBitmap*/, std::byte /*chanContBitmap*/,
                                   std::byte /*chanNoteBitmap*/) { /* do nothing */ }
  virtual void recvPIMMReportEnd(MIDICI const &) { /* do nothing */ }
};

class profile_callbacks {
public:
  profile_callbacks() = default;
  profile_callbacks(profile_callbacks const &) = default;
  profile_callbacks(profile_callbacks &&) noexcept = default;
  virtual ~profile_callbacks() noexcept = default;

  virtual void inquiry(MIDICI const &) { /* do nothing */ }
  virtual void inquiry_reply(MIDICI const &, ci::profile_inquiry_reply const &) { /* do nothing */ }
  virtual void added(MIDICI const &, ci::profile_added const &) { /* do nothing */ }
  virtual void removed(MIDICI const &, ci::profile_removed const &) { /* do nothing */ }
  virtual void details_inquiry(MIDICI const &, ci::profile_details_inquiry const &) { /* do nothing */ }
  virtual void details_reply(MIDICI const &, ci::profile_details_reply const &) { /* do nothing */ }
  virtual void on(MIDICI const &, ci::profile_on const &) { /* do nothing */ }
  virtual void off(MIDICI const &, ci::profile_off const &) { /* do nothing */ }
  virtual void enabled(MIDICI const &, ci::profile_enabled const &) { /* do nothing */ }
  virtual void disabled(MIDICI const &, ci::profile_disabled const &) { /* do nothing */ }
  virtual void specific_data(MIDICI const &, ci::profile_specific_data const &) { /* do nothing */ }
};

class property_exchange_callbacks {
public:
  property_exchange_callbacks() = default;
  property_exchange_callbacks(property_exchange_callbacks const &) = default;
  property_exchange_callbacks(property_exchange_callbacks &&) noexcept = default;
  virtual ~property_exchange_callbacks() noexcept = default;

  property_exchange_callbacks &operator=(property_exchange_callbacks const &) = default;
  property_exchange_callbacks &operator=(property_exchange_callbacks &&) noexcept = default;

  virtual void capabilities(MIDICI const &, ci::pe_capabilities const &) { /* do nothing */ }
  virtual void capabilities_reply(MIDICI const &, midi2::ci::pe_capabilities_reply const &) { /* do nothing */ }

  virtual void get_reply(MIDICI const &, midi2::ci::pe_chunk_info const &, std::span<char const>,
                         std::span<std::byte const>) { /* do nothing */ }
};

template <typename T> concept unaligned_copyable = alignof(T) == 1 && std::is_trivially_copyable_v<T>;

template <discovery_backend Callbacks = ci_callbacks, profile_backend ProfileBackend = profile_callbacks,
          property_exchange_backend PEBackend = property_exchange_callbacks>
class midiCIProcessor {
public:
  explicit midiCIProcessor(Callbacks callbacks = Callbacks{}, ProfileBackend profile = ProfileBackend{},
                           PEBackend pe_backend = PEBackend{})
      : callbacks_{callbacks}, profile_backend_{profile}, pe_backend_{pe_backend} {}

  void startSysex7(std::uint8_t group, std::byte deviceId);
  void endSysex7();

  void processMIDICI(std::byte s7Byte);

private:
  static constexpr auto S7_BUFFERLEN = 36;
  static constexpr auto header_size = 13U;

  [[no_unique_address]] Callbacks callbacks_;
  [[no_unique_address]] ProfileBackend profile_backend_;
  [[no_unique_address]] PEBackend pe_backend_;

  MIDICI midici_;
  ci::pe_chunk_info chunk_;
  std::uint16_t sysexPos_ = 0;

  using reqId = std::tuple<std::uint32_t, std::byte>;  // muid-requestId
  std::optional<reqId> _peReqIdx;

  // in Discovery this is [sysexID1,sysexID2,sysexID3,famId1,famid2,modelId1,modelId2,ver1,ver2,ver3,ver4,...product Id]
  // in Profiles this is  [pf1, pf1, pf3, pf4, pf5]
  // in Protocols this is [pr1, pr2, pr3, pr4, pr5]
  std::array<std::byte, 256> buffer_;

  // in Discovery this is [ciSupport, maxSysex, output path id]
  // in Profile Inquiry Reply, this is [Enabled Profiles Length, Disabled Profile Length]
  // in Profile On/Off/Enabled/Disabled, this is [numOfChannels]
  // in PE this is [header length, Body Length]
  std::uint32_t intTemp_[4]{};

  // Property Exchange
  std::map<reqId, std::string> peHeaderStr;

  void cleanupRequest(reqId peReqIdx);
  void processPESysex(std::byte s7Byte);
  void processPISysex(std::byte s7Byte);

  template <unaligned_copyable PackedType, std::invocable<PackedType> Handler>
  void fixed_size(std::byte s7, Handler handler);

  template <unaligned_copyable PackedType, std::size_t FixedSize, std::invocable<PackedType> GetDataSize,
            std::invocable<PackedType> Handler>
  requires(FixedSize <= sizeof(PackedType)) void trailing_data(std::byte s7, GetDataSize get_data_size,
                                                               Handler handler);

  bool gather(std::byte s7, std::size_t size);

  // The "management" messages
  void discovery(std::byte s7);
  void discovery_reply(std::byte s7);
  void endpoint_info(std::byte s7);
  void endpoint_info_reply(std::byte s7);
  void invalidate_muid(std::byte s7);
  void ack(std::byte s7);
  void nak(std::byte s7);

  // Profile messages
  void profile_inquiry(std::byte s7);
  void profile_inquiry_reply(std::byte s7);
  void profile_added(std::byte s7);
  void profile_removed(std::byte s7);
  void profile_details_inquiry(std::byte s7);
  void profile_details_reply(std::byte s7);
  void profile_on(std::byte s7);
  void profile_off(std::byte s7);
  void profile_enabled(std::byte s7);
  void profile_disabled(std::byte s7);
  void profile_specific_data(std::byte s7);

  // Property Exchange messages
  void pe_capabilities(std::byte s7);
  void pe_capabilities_reply(std::byte s7);
};

midiCIProcessor() -> midiCIProcessor<>;
template <discovery_backend C> midiCIProcessor(C) -> midiCIProcessor<C>;
template <discovery_backend C> midiCIProcessor(std::reference_wrapper<C>) -> midiCIProcessor<C &>;
template <discovery_backend C, profile_backend P> midiCIProcessor(C, P) -> midiCIProcessor<C, P>;
template <discovery_backend C, profile_backend P, property_exchange_backend PE>
midiCIProcessor(C, P, PE) -> midiCIProcessor<C, P, PE>;

template <discovery_backend C, profile_backend P>
midiCIProcessor(std::reference_wrapper<C>, std::reference_wrapper<P>) -> midiCIProcessor<C &, P &>;
template <discovery_backend C, profile_backend P, property_exchange_backend PE>
midiCIProcessor(std::reference_wrapper<C>, std::reference_wrapper<P>,
                std::reference_wrapper<PE>) -> midiCIProcessor<C &, P &, PE &>;

template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::endSysex7() {
  if (_peReqIdx) {
    cleanupRequest(_peReqIdx);
  }
}

template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::startSysex7(std::uint8_t group, std::byte deviceId) {
  sysexPos_ = 0;
  buffer_[0] = std::byte{0x00};
  midici_ = MIDICI();
  midici_.deviceId = static_cast<std::uint8_t>(deviceId);
  midici_.umpGroup = group;
}

template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::cleanupRequest(reqId peReqIdx) {
  peHeaderStr.erase(peReqIdx);
}

// gather
// ~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
bool midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::gather(std::byte const s7, std::size_t const size) {
  assert(size < buffer_.size() - header_size);
  if (sysexPos_ < header_size || sysexPos_ > buffer_.size()) {
    return false;
  }
  buffer_[sysexPos_ - header_size] = s7;
  return sysexPos_ == header_size + size - 1;
}

// fixed size
// ~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
template <unaligned_copyable PackedType, std::invocable<PackedType> Handler>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::fixed_size(std::byte const s7, Handler const handler) {
  if (this->gather(s7, sizeof(PackedType))) {
    return handler(*std::bit_cast<PackedType const *>(buffer_.data()));
  }
}

// trailing data
// ~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
template <unaligned_copyable PackedType, std::size_t FixedSize, std::invocable<PackedType> GetDataSize,
          std::invocable<PackedType> Handler>
requires(FixedSize <= sizeof(PackedType)) void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::trailing_data(
    std::byte const s7, GetDataSize const get_data_size, Handler const handler) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;
  // Wait for the basic structure to arrive.
  constexpr auto fixed_size_index = header_size + FixedSize - 1;
  if (sysexPos_ < fixed_size_index) {
    return;
  }

  // Wait for the variable-length data.
  auto const *const packed_reply = std::bit_cast<PackedType const *>(buffer_.data());
  if (sysexPos_ == fixed_size_index + get_data_size(*packed_reply)) {
    return handler(*std::bit_cast<PackedType const *>(buffer_.data()));
  }
}

// discovery
// ~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::discovery(std::byte const s7) {
  auto const handler = [this](unaligned_copyable auto const &v) { callbacks_.discovery(midici_, ci::discovery{v}); };
  if (midici_.ciVer == 1) {
    return fixed_size<ci::packed::discovery_v1>(s7, handler);
  }
  return fixed_size<ci::packed::discovery_v2>(s7, handler);
}

// discovery reply
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::discovery_reply(std::byte const s7) {
  auto const handler = [this](unaligned_copyable auto const &v) {
    callbacks_.discovery_reply(midici_, ci::discovery_reply{v});
  };
  if (midici_.ciVer == 1) {
    return this->fixed_size<ci::packed::discovery_reply_v1>(s7, handler);
  }
  return this->fixed_size<ci::packed::discovery_reply_v2>(s7, handler);
}

// invalidate muid
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::invalidate_muid(std::byte const s7) {
  using type = ci::packed::invalidate_muid_v1;
  return this->fixed_size<type>(
      s7, [this](type const &v1) { callbacks_.invalidate_muid(midici_, ci::invalidate_muid{v1}); });
}

// ack
// ~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::ack(std::byte const s7) {
  using type = ci::packed::ack_v1;
  auto const get_data_size = [](type const &reply) { return ci::packed::from_le7(reply.message_length); };
  auto const handler = [this](type const &reply) { return callbacks_.ack(midici_, ci::ack{reply}); };
  return this->trailing_data<type, offsetof(type, message)>(s7, get_data_size, handler);
}

// nak
// ~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::nak(std::byte const s7) {
  using v1_type = ci::packed::nak_v1;
  using v2_type = ci::packed::nak_v2;

  auto const handler = [this](unaligned_copyable auto const &reply) { return callbacks_.nak(midici_, ci::nak{reply}); };
  if (midici_.ciVer == 1) {
    return this->fixed_size<v1_type>(s7, handler);
  }
  auto const get_data_size = [](v2_type const &reply) { return ci::packed::from_le7(reply.message_length); };
  return this->trailing_data<v2_type, offsetof(v2_type, message)>(s7, get_data_size, handler);
}

// endpoint info
// ~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::endpoint_info(std::byte const s7) {
  using type = ci::packed::endpoint_info_v1;
  return this->fixed_size<type>(s7,
                                [this](type const &v1) { callbacks_.endpoint_info(midici_, ci::endpoint_info{v1}); });
}

// endpoint info reply
// ~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::endpoint_info_reply(std::byte const s7) {
  using type = ci::packed::endpoint_info_reply_v1;
  auto const get_data_size = [](type const &reply) { return ci::packed::from_le7(reply.data_length); };
  auto const handler = [this](type const &reply) {
    return callbacks_.endpoint_info_reply(midici_, ci::endpoint_info_reply{reply});
  };
  return this->trailing_data<type, offsetof(type, data)>(s7, get_data_size, handler);
}

// profile inquiry
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_inquiry(std::byte) {
  if (sysexPos_ == header_size - 1) {
    return profile_backend_.inquiry(midici_);
  }
}

// profile inquiry reply
// ~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_inquiry_reply(std::byte const s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;
  // Wait for the first part to arrive.
  constexpr auto pt1_size = offsetof(ci::packed::profile_inquiry_reply_v1_pt1, ids);
  auto end = header_size + pt1_size - 1;
  if (sysexPos_ < end) {
    return;
  }
  // Wait for the variable length data following the first part.
  auto const *const pt1 = std::bit_cast<ci::packed::profile_inquiry_reply_v1_pt1 const *>(buffer_.data());
  end += ci::packed::from_le7(pt1->num_enabled) * sizeof(pt1->ids[0]);
  if (sysexPos_ < end) {
    return;
  }
  // Wait for the second part to arrive.
  end += offsetof(ci::packed::profile_inquiry_reply_v1_pt2, ids);
  if (sysexPos_ < end) {
    return;
  }
  // Now the second block of variable-length data.
  auto p2_pos = end - (header_size + pt1_size - 1);
  auto const *const pt2 = std::bit_cast<ci::packed::profile_inquiry_reply_v1_pt2 const *>(buffer_.data() + p2_pos);
  end += ci::packed::from_le7(pt2->num_disabled) * sizeof(pt2->ids[0]);
  if (sysexPos_ == end) {
    return profile_backend_.inquiry_reply(midici_, ci::profile_inquiry_reply{*pt1, *pt2});
  }
}

// profile added
// ~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_added(std::byte const s7) {
  using type = ci::packed::profile_added_v1;
  return this->fixed_size<type>(s7, [this](type const &v) { profile_backend_.added(midici_, ci::profile_added{v}); });
}

// profile removed
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_removed(std::byte const s7) {
  using type = ci::packed::profile_removed_v1;
  return this->fixed_size<type>(s7,
                                [this](type const &v) { profile_backend_.removed(midici_, ci::profile_removed{v}); });
}

// profile details inquiry
// ~~~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_details_inquiry(std::byte const s7) {
  using type = ci::packed::profile_details_inquiry_v1;
  return this->fixed_size<type>(
      s7, [this](type const &v) { profile_backend_.details_inquiry(midici_, ci::profile_details_inquiry{v}); });
}

// profile details reply
// ~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_details_reply(std::byte const s7) {
  using type = ci::packed::profile_details_reply_v1;
  auto const get_data_size = [](type const &reply) { return ci::packed::from_le7(reply.data_length); };
  auto const handler = [this](type const &reply) {
    return profile_backend_.details_reply(midici_, ci::profile_details_reply{reply});
  };
  return this->trailing_data<type, offsetof(type, data)>(s7, get_data_size, handler);
}

// profile on
// ~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_on(std::byte const s7) {
  auto const handler = [this](auto const &v) { profile_backend_.on(midici_, ci::profile_on{v}); };
  if (midici_.ciVer == 1) {
    return this->fixed_size<ci::packed::profile_on_v1>(s7, handler);
  }
  return this->fixed_size<ci::packed::profile_on_v2>(s7, handler);
}

// profile off
// ~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_off(std::byte const s7) {
  auto const handler = [this](unaligned_copyable auto const &v) { profile_backend_.off(midici_, ci::profile_off{v}); };
  if (midici_.ciVer == 1) {
    return this->fixed_size<ci::packed::profile_off_v1>(s7, handler);
  }
  return this->fixed_size<ci::packed::profile_off_v2>(s7, handler);
}

// profile enabled
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_enabled(std::byte const s7) {
  auto const handler = [this](auto const &v) { profile_backend_.enabled(midici_, ci::profile_enabled{v}); };
  if (midici_.ciVer == 1) {
    return this->fixed_size<ci::packed::profile_enabled_v1>(s7, handler);
  }
  return this->fixed_size<ci::packed::profile_enabled_v2>(s7, handler);
}

// profile disabled
// ~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_disabled(std::byte const s7) {
  auto const handler = [this](unaligned_copyable auto const &v) {
    profile_backend_.disabled(midici_, ci::profile_disabled{v});
  };
  if (midici_.ciVer == 1) {
    return this->fixed_size<ci::packed::profile_disabled_v1>(s7, handler);
  }
  return this->fixed_size<ci::packed::profile_disabled_v2>(s7, handler);
}

// profile specific data
// ~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::profile_specific_data(std::byte const s7) {
  using type = ci::packed::profile_specific_data_v1;
  auto const get_data_size = [](type const &reply) { return ci::packed::from_le7(reply.data_length); };
  auto const handler = [this](type const &reply) {
    return profile_backend_.specific_data(midici_, ci::profile_specific_data{reply});
  };
  return this->trailing_data<type, offsetof(type, data)>(s7, get_data_size, handler);
}

// pe capabilities
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::pe_capabilities(std::byte const s7) {
  auto const handler = [this](unaligned_copyable auto const &v) {
    pe_backend_.capabilities(midici_, ci::pe_capabilities{v});
  };
  if (midici_.ciVer == 1) {
    return this->fixed_size<ci::packed::pe_capabilities_v1>(s7, handler);
  }
  return this->fixed_size<ci::packed::pe_capabilities_v2>(s7, handler);
}

// pe capabilities reply
// ~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::pe_capabilities_reply(std::byte const s7) {
  auto const handler = [this](unaligned_copyable auto const &v) {
    pe_backend_.capabilities_reply(midici_, ci::pe_capabilities_reply{v});
  };
  if (midici_.ciVer == 1) {
    return this->fixed_size<ci::packed::pe_capabilities_reply_v1>(s7, handler);
  }
  return this->fixed_size<ci::packed::pe_capabilities_reply_v2>(s7, handler);
}

template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::processMIDICI(std::byte s7Byte) {
  assert((s7Byte & std::byte{0b10000000}) == std::byte{0});
  if (sysexPos_ == 3) {
    midici_.ciType = static_cast<std::uint8_t>(s7Byte);
  }
  if (sysexPos_ == 4) {
    midici_.ciVer = static_cast<std::uint8_t>(s7Byte);
  }
  if (sysexPos_ >= 5 && sysexPos_ <= 8) {
    midici_.remoteMUID += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 5));
  }
  if (sysexPos_ >= 9 && sysexPos_ <= 12) {
    midici_.localMUID += static_cast<std::uint32_t>(s7Byte) << (7 * (sysexPos_ - 9));
  }
  if (sysexPos_ >= 12 && midici_.localMUID != M2_CI_BROADCAST &&
      !callbacks_.check_muid(midici_.umpGroup, midici_.localMUID)) {
    return;  // Not for this device
  }

  // break up each Process based on ciType
  if (sysexPos_ >= 12) {
    switch (midici_.ciType) {
    case MIDICI_DISCOVERY: this->discovery(s7Byte); break;
    case MIDICI_DISCOVERY_REPLY: this->discovery_reply(s7Byte); break;
    case MIDICI_ENDPOINTINFO: this->endpoint_info(s7Byte); break;
    case MIDICI_ENDPOINTINFO_REPLY: this->endpoint_info_reply(s7Byte); break;
    case MIDICI_INVALIDATEMUID: this->invalidate_muid(s7Byte); break;
    case MIDICI_ACK: this->ack(s7Byte); break;
    case MIDICI_NAK: this->nak(s7Byte); break;

    case MIDICI_PROFILE_INQUIRY: this->profile_inquiry(s7Byte); break;
    case MIDICI_PROFILE_INQUIRYREPLY: this->profile_inquiry_reply(s7Byte); break;
    case MIDICI_PROFILE_ADDED: this->profile_added(s7Byte); break;
    case MIDICI_PROFILE_REMOVED: this->profile_removed(s7Byte); break;
    case MIDICI_PROFILE_DETAILS_INQUIRY: this->profile_details_inquiry(s7Byte); break;
    case MIDICI_PROFILE_DETAILS_REPLY: this->profile_details_reply(s7Byte); break;
    case MIDICI_PROFILE_SETOFF: this->profile_off(s7Byte); break;
    case MIDICI_PROFILE_SETON: this->profile_on(s7Byte); break;
    case MIDICI_PROFILE_ENABLED: this->profile_enabled(s7Byte); break;
    case MIDICI_PROFILE_DISABLED: this->profile_disabled(s7Byte); break;
    case MIDICI_PROFILE_SPECIFIC_DATA:
      this->profile_specific_data(s7Byte);
      break;

    case MIDICI_PE_CAPABILITY: this->pe_capabilities(s7Byte); break;
    case MIDICI_PE_CAPABILITYREPLY: this->pe_capabilities_reply(s7Byte); break;
      // #ifndef M2_DISABLE_PE
    case MIDICI_PE_GET:              // Inquiry: Get Property Data
    case MIDICI_PE_GETREPLY:         // Reply To Get Property Data - Needs Work!
    case MIDICI_PE_SET:              // Inquiry: Set Property Data
    case MIDICI_PE_SETREPLY:         // Reply To Inquiry: Set Property Data
    case MIDICI_PE_SUB:              // Inquiry: Subscribe Property Data
    case MIDICI_PE_SUBREPLY:         // Reply To Subscribe Property Data
    case MIDICI_PE_NOTIFY:           // Notify
      this->processPESysex(s7Byte);
      break;
      // #endif

      // #ifndef M2_DISABLE_PROCESSINQUIRY
    case MIDICI_PI_CAPABILITY:
    case MIDICI_PI_CAPABILITYREPLY:
    case MIDICI_PI_MM_REPORT:
    case MIDICI_PI_MM_REPORT_REPLY:
    case MIDICI_PI_MM_REPORT_END:
      this->processPISysex(s7Byte);
      break;
      // #endif
    default: callbacks_.unknown_midici(midici_, s7Byte); break;
    }
  }
  sysexPos_++;
}

template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::processPESysex(std::byte s7) {
  if (sysexPos_ < header_size) {
    return;
  }
  buffer_[sysexPos_ - header_size] = s7;
  // Wait for the first part to arrive.
  constexpr auto pt1_size = offsetof(ci::packed::property_exchange_pt1, header);
  auto end = header_size + pt1_size - 1;
  if (sysexPos_ < end) {
    return;
  }
  // Wait for the variable-length header data following the first part.
  auto const *const pt1 = std::bit_cast<ci::packed::property_exchange_pt1 const *>(buffer_.data());
  auto const hlength = ci::packed::from_le7(pt1->header_length);
  end += hlength * sizeof(pt1->header[0]);
  if (sysexPos_ < end) {
    return;
  }
  // Wait for the second part to arrive.
  end += offsetof(ci::packed::property_exchange_pt2, data);
  if (sysexPos_ < end) {
    return;
  }

  // Now the second block of variable-length data.
  auto const p2_pos = pt1_size + hlength;
  auto const *const pt2 = std::bit_cast<ci::packed::property_exchange_pt2 const *>(buffer_.data() + p2_pos);

  auto const data_length = ci::packed::from_le7(pt2->data_length);
  end += data_length * sizeof(pt2->data[0]);
  if (sysexPos_ == end) {
    ci::pe_chunk_info chunk;
    chunk.requestId = static_cast<std::uint8_t>(pt1->request_id);
    chunk.totalChunks = ci::packed::from_le7(pt2->number_of_chunks);
    chunk.numChunk = ci::packed::from_le7(pt2->chunk_number);

    auto const header = std::span<char const>{std::bit_cast<char const *>(&pt1->header[0]), hlength};
    auto const data = std::span<std::byte const>{&pt2->data[0], data_length};
    switch (midici_.ciType) {
    case MIDICI_PE_GETREPLY: pe_backend_.get_reply(midici_, chunk, header, data); break;
    default: break;
    }
    return;
  }
}

template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend>::processPISysex(std::byte s7Byte) {
  if (midici_.ciVer == 1) {
    return;
  }

  switch (midici_.ciType) {
  case MIDICI_PI_CAPABILITY:
    if (sysexPos_ == 12) {
      callbacks_.recvPICapabilities(midici_);
    }
    break;
  case MIDICI_PI_CAPABILITYREPLY:
    if (sysexPos_ == 13) {
      callbacks_.recvPICapabilitiesReply(midici_, s7Byte);
    }
    break;
  case MIDICI_PI_MM_REPORT_END:
    if (sysexPos_ == 12) {
      callbacks_.recvPIMMReportEnd(midici_);
    }
    break;
  case MIDICI_PI_MM_REPORT:
    if (sysexPos_ == 13) {  // MDC
      buffer_[0] = s7Byte;
    }
    if (sysexPos_ == 14) {  // Bitmap of requested System Message Types
      buffer_[1] = s7Byte;
    }
    if (sysexPos_ == 16) {  // Bitmap of requested Channel Controller Message Types
      buffer_[2] = s7Byte;
    }
    if (sysexPos_ == 17) {
      callbacks_.recvPIMMReport(midici_, buffer_[0], buffer_[1], buffer_[2], s7Byte);
    }
    break;

  case MIDICI_PI_MM_REPORT_REPLY:
    if (sysexPos_ == 13) {  // Bitmap of requested System Message Types
      buffer_[0] = s7Byte;
    }
    if (sysexPos_ == 15) {  // Bitmap of requested Channel Controller Message Types
      buffer_[1] = s7Byte;
    }
    if (sysexPos_ == 16) {
      callbacks_.recvPIMMReportReply(midici_, buffer_[0], buffer_[1], s7Byte);
    }
    break;
  default: break;
  }
}

}  // end namespace midi2

#endif  // MIDI2_MIDICIPROCESSOR_HPP
