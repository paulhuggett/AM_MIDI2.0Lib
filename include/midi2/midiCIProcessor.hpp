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

  { v.unknown_midici(MIDICI{}) } -> std::same_as<void>;
  { v.buffer_overflow() } -> std::same_as<void>;
};

template <typename T> concept profile_backend = requires(T && v) {
  { v.inquiry(MIDICI{}) } -> std::same_as<void>;
  { v.inquiry_reply(MIDICI{}, ci::profile_configuration::inquiry_reply{}) } -> std::same_as<void>;
  { v.added(MIDICI{}, ci::profile_configuration::added{}) } -> std::same_as<void>;
  { v.removed(MIDICI{}, ci::profile_configuration::removed{}) } -> std::same_as<void>;
  { v.details(MIDICI{}, ci::profile_configuration::details{}) } -> std::same_as<void>;
  { v.details_reply(MIDICI{}, ci::profile_configuration::details_reply{}) } -> std::same_as<void>;
  { v.on(MIDICI{}, ci::profile_configuration::on{}) } -> std::same_as<void>;
  { v.off(MIDICI{}, ci::profile_configuration::off{}) } -> std::same_as<void>;
  { v.enabled(MIDICI{}, ci::profile_configuration::enabled{}) } -> std::same_as<void>;
  { v.disabled(MIDICI{}, ci::profile_configuration::disabled{}) } -> std::same_as<void>;
  { v.specific_data(MIDICI{}, ci::profile_configuration::specific_data{}) } -> std::same_as<void>;
};

template <typename T> concept property_exchange_backend = requires(T && v) {
  { v.capabilities(MIDICI{}, ci::property_exchange::capabilities{}) } -> std::same_as<void>;
  { v.capabilities_reply(MIDICI{}, ci::property_exchange::capabilities_reply{}) } -> std::same_as<void>;

  { v.get(MIDICI{}, ci::property_exchange::chunk_info{}, ci::property_exchange::property_exchange{}) } -> std::same_as<void>;
  { v.get_reply(MIDICI{}, ci::property_exchange::chunk_info{}, ci::property_exchange::property_exchange{}) } -> std::same_as<void>;
  { v.set(MIDICI{}, ci::property_exchange::chunk_info{}, ci::property_exchange::property_exchange{}) } -> std::same_as<void>;
  { v.set_reply(MIDICI{}, ci::property_exchange::chunk_info{}, ci::property_exchange::property_exchange{}) } -> std::same_as<void>;
};

template <typename T> concept process_inquiry_backend = requires(T && v) {
  { v.capabilities(MIDICI{}) } -> std::same_as<void>;
  { v.capabilities_reply(MIDICI{}, ci::process_inquiry::capabilities_reply{}) } -> std::same_as<void>;
  { v.midi_message_report(MIDICI{}, ci::process_inquiry::midi_message_report{}) } -> std::same_as<void>;
  { v.midi_message_report_reply(MIDICI{}, ci::process_inquiry::midi_message_report_reply{}) } -> std::same_as<void>;
  { v.midi_message_report_end(MIDICI{}) } -> std::same_as<void>;
};

class management_callbacks {
public:
  management_callbacks() = default;
  management_callbacks(management_callbacks const &) = default;
  management_callbacks(management_callbacks &&) noexcept = default;
  virtual ~management_callbacks() noexcept = default;

  virtual bool check_muid(std::uint8_t /*group*/, std::uint32_t /*muid*/) { return false; }

  virtual void discovery(MIDICI const &, ci::discovery const &) { /* do nothing */ }
  virtual void discovery_reply(MIDICI const &, ci::discovery_reply const &) { /* do nothing */ }
  virtual void endpoint_info(MIDICI const &, ci::endpoint_info const &) { /* do nothing*/ }
  virtual void endpoint_info_reply(MIDICI const &, ci::endpoint_info_reply const &) { /* do nothing */ }
  virtual void invalidate_muid(MIDICI const &, ci::invalidate_muid const &) { /* do nothing */ }
  virtual void ack(MIDICI const &, ci::ack const &) { /* do nothing */ }
  virtual void nak(MIDICI const &, ci::nak const &) { /* do nothing */ }

  virtual void unknown_midici(MIDICI const &) { /* do nothing */ }
  virtual void buffer_overflow() { /* do nothing */ }
};

class profile_callbacks {
public:
  profile_callbacks() = default;
  profile_callbacks(profile_callbacks const &) = default;
  profile_callbacks(profile_callbacks &&) noexcept = default;
  virtual ~profile_callbacks() noexcept = default;

  virtual void inquiry(MIDICI const &) { /* do nothing */ }
  virtual void inquiry_reply(MIDICI const &, ci::profile_configuration::inquiry_reply const &) { /* do nothing */ }
  virtual void added(MIDICI const &, ci::profile_configuration::added const &) { /* do nothing */ }
  virtual void removed(MIDICI const &, ci::profile_configuration::removed const &) { /* do nothing */ }
  virtual void details(MIDICI const &, ci::profile_configuration::details const &) { /* do nothing */ }
  virtual void details_reply(MIDICI const &, ci::profile_configuration::details_reply const &) { /* do nothing */ }
  virtual void on(MIDICI const &, ci::profile_configuration::on const &) { /* do nothing */ }
  virtual void off(MIDICI const &, ci::profile_configuration::off const &) { /* do nothing */ }
  virtual void enabled(MIDICI const &, ci::profile_configuration::enabled const &) { /* do nothing */ }
  virtual void disabled(MIDICI const &, ci::profile_configuration::disabled const &) { /* do nothing */ }
  virtual void specific_data(MIDICI const &, ci::profile_configuration::specific_data const &) { /* do nothing */ }
};

class property_exchange_callbacks {
public:
  property_exchange_callbacks() = default;
  property_exchange_callbacks(property_exchange_callbacks const &) = default;
  property_exchange_callbacks(property_exchange_callbacks &&) noexcept = default;
  virtual ~property_exchange_callbacks() noexcept = default;

  property_exchange_callbacks &operator=(property_exchange_callbacks const &) = default;
  property_exchange_callbacks &operator=(property_exchange_callbacks &&) noexcept = default;

  virtual void capabilities(MIDICI const &, ci::property_exchange::capabilities const &) { /* do nothing */ }
  virtual void capabilities_reply(MIDICI const &, ci::property_exchange::capabilities_reply const &) { /* do nothing */ }

  virtual void get(MIDICI const &, ci::property_exchange::chunk_info const &, ci::property_exchange::property_exchange const &) { /* do nothing */ }
  virtual void get_reply(MIDICI const &, ci::property_exchange::chunk_info const &, ci::property_exchange::property_exchange const &) { /* do nothing */ }
  virtual void set(MIDICI const &, midi2::ci::property_exchange::chunk_info const &, ci::property_exchange::property_exchange const &) { /* do nothing */ }
  virtual void set_reply(MIDICI const &, ci::property_exchange::chunk_info const &, ci::property_exchange::property_exchange const &) { /* do nothing */ }
};

class process_inquiry_callbacks {
public:
  process_inquiry_callbacks() = default;
  process_inquiry_callbacks(process_inquiry_callbacks const &) = default;
  process_inquiry_callbacks(process_inquiry_callbacks &&) noexcept = default;
  virtual ~process_inquiry_callbacks() noexcept = default;

  process_inquiry_callbacks &operator=(process_inquiry_callbacks const &) = default;
  process_inquiry_callbacks &operator=(process_inquiry_callbacks &&) noexcept = default;

  virtual void capabilities(MIDICI const &) { /* do nothing */ }
  virtual void capabilities_reply(MIDICI const &, ci::process_inquiry::capabilities_reply const &) { /* do nothing */ }
  virtual void midi_message_report(MIDICI const &, ci::process_inquiry::midi_message_report const &) { /* do nothing */ }
  virtual void midi_message_report_reply(MIDICI const &, ci::process_inquiry::midi_message_report_reply const &) { /* do nothing */ }
  virtual void midi_message_report_end(MIDICI const &) { /* do nothing */ }
};

template <typename T> concept unaligned_copyable = alignof(T) == 1 && std::is_trivially_copyable_v<T>;

template <discovery_backend Callbacks = management_callbacks, profile_backend ProfileBackend = profile_callbacks,
          property_exchange_backend PEBackend = property_exchange_callbacks,
          process_inquiry_backend PIBackend = process_inquiry_callbacks>
class midiCIProcessor {
public:
  explicit midiCIProcessor(Callbacks callbacks = Callbacks{}, ProfileBackend profile = ProfileBackend{},
                           PEBackend pe_backend = PEBackend{}, PIBackend pi_backend = PIBackend{})
      : callbacks_{callbacks},
        profile_backend_{profile},
        pe_backend_{pe_backend},
        process_inquiry_backend_{pi_backend} {}

  void startSysex7(std::uint8_t group, std::byte deviceId);
  void endSysex7() {}

  void processMIDICI(std::byte s7Byte);

private:
  static constexpr auto header_size = sizeof(ci::packed::header);

  [[no_unique_address]] Callbacks callbacks_;
  [[no_unique_address]] ProfileBackend profile_backend_;
  [[no_unique_address]] PEBackend pe_backend_;
  [[no_unique_address]] PIBackend process_inquiry_backend_;

  using consumer_fn = void (midiCIProcessor::*)();

  std::size_t count_ = header_size;
  unsigned pos_ = 0;
  consumer_fn consumer_ = &midiCIProcessor::header;

  MIDICI midici_;
  std::array<std::byte, 512> buffer_;

  void discard() { pos_ = 0; }
  void overflow();

  void header();
  // "Management" messages
  void discovery();
  void discovery_reply();
  void endpoint_info();
  void endpoint_info_reply();
  void invalidate_muid();
  void ack();
  void nak();

  // Profile messages
  void profile_inquiry();
  void profile_inquiry_reply();
  void profile_added();
  void profile_removed();
  void profile_details();
  void profile_details_reply();
  void profile_on();
  void profile_off();
  void profile_enabled();
  void profile_disabled();
  void profile_specific_data();

  // Property Exchange messages
  void pe_capabilities();
  void pe_capabilities_reply();
  void property_exchange();

  // Process Inquiry messages
  void process_inquiry_capabilities();
  void process_inquiry_capabilities_reply();
  void process_inquiry_midi_message_report();
  void process_inquiry_midi_message_report_reply();
  void process_inquiry_midi_message_report_end();
};

midiCIProcessor() -> midiCIProcessor<>;
template <discovery_backend C> midiCIProcessor(C) -> midiCIProcessor<C>;
template <discovery_backend C> midiCIProcessor(std::reference_wrapper<C>) -> midiCIProcessor<C &>;
template <discovery_backend C, profile_backend P> midiCIProcessor(C, P) -> midiCIProcessor<C, P>;
template <discovery_backend C, profile_backend P, property_exchange_backend PE>
midiCIProcessor(C, P, PE) -> midiCIProcessor<C, P, PE>;
template <discovery_backend C, profile_backend P, property_exchange_backend PE, process_inquiry_backend PI>
midiCIProcessor(C, P, PE, PI) -> midiCIProcessor<C, P, PE, PI>;

template <discovery_backend C, profile_backend P>
midiCIProcessor(std::reference_wrapper<C>, std::reference_wrapper<P>) -> midiCIProcessor<C &, P &>;
template <discovery_backend C, profile_backend P, property_exchange_backend PE>
midiCIProcessor(std::reference_wrapper<C>, std::reference_wrapper<P>,
                std::reference_wrapper<PE>) -> midiCIProcessor<C &, P &, PE &>;
template <discovery_backend C, profile_backend P, property_exchange_backend PE, process_inquiry_backend PI>
midiCIProcessor(std::reference_wrapper<C>, std::reference_wrapper<P>, std::reference_wrapper<PE>,
                std::reference_wrapper<PI>) -> midiCIProcessor<C &, P &, PE &, PI &>;

template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::startSysex7(std::uint8_t group,
                                                                                   std::byte deviceId) {
  midici_ = MIDICI{};
  midici_.deviceId = static_cast<std::uint8_t>(deviceId);
  midici_.umpGroup = group;

  count_ = header_size;
  pos_ = 0;
  consumer_ = &midiCIProcessor::header;
}

template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::header() {
  struct message_dispatch_info {
    std::uint8_t type;
    std::uint8_t v1size;
    std::uint8_t v2size;
    consumer_fn consumer;
  };

  static std::array const messages = {
    message_dispatch_info{MIDICI_PROFILE_INQUIRY, 0, 0, &midiCIProcessor::profile_inquiry},
    message_dispatch_info{MIDICI_PROFILE_INQUIRYREPLY,
                          offsetof(ci::profile_configuration::packed::inquiry_reply_v1_pt1, ids),
                          offsetof(ci::profile_configuration::packed::inquiry_reply_v1_pt1, ids),
                          &midiCIProcessor::profile_inquiry_reply},
    message_dispatch_info{MIDICI_PROFILE_SETON, sizeof(ci::profile_configuration::packed::on_v1),
                          sizeof(ci::profile_configuration::packed::on_v2), &midiCIProcessor::profile_on},
    message_dispatch_info{MIDICI_PROFILE_SETOFF, sizeof(ci::profile_configuration::packed::off_v1),
                          sizeof(ci::profile_configuration::packed::off_v2), &midiCIProcessor::profile_off},
    message_dispatch_info{MIDICI_PROFILE_ENABLED, sizeof(ci::profile_configuration::packed::enabled_v1),
                          sizeof(ci::profile_configuration::packed::enabled_v2), &midiCIProcessor::profile_enabled},
    message_dispatch_info{MIDICI_PROFILE_DISABLED, sizeof(ci::profile_configuration::packed::disabled_v1),
                          sizeof(ci::profile_configuration::packed::disabled_v2), &midiCIProcessor::profile_disabled},
    message_dispatch_info{MIDICI_PROFILE_ADDED, sizeof(ci::profile_configuration::packed::added_v1),
                          sizeof(ci::profile_configuration::packed::added_v1), &midiCIProcessor::profile_added},
    message_dispatch_info{MIDICI_PROFILE_REMOVED, sizeof(ci::profile_configuration::packed::removed_v1),
                          sizeof(ci::profile_configuration::packed::removed_v1), &midiCIProcessor::profile_removed},
    message_dispatch_info{MIDICI_PROFILE_DETAILS_INQUIRY, sizeof(ci::profile_configuration::packed::details_v1),
                          sizeof(ci::profile_configuration::packed::details_v1), &midiCIProcessor::profile_details},
    message_dispatch_info{MIDICI_PROFILE_DETAILS_REPLY, offsetof(ci::profile_configuration::packed::details_reply_v1, data),
                          offsetof(ci::profile_configuration::packed::details_reply_v1, data),
                          &midiCIProcessor::profile_details_reply},
    message_dispatch_info{MIDICI_PROFILE_SPECIFIC_DATA,
                          offsetof(ci::profile_configuration::packed::specific_data_v1, data),
                          offsetof(ci::profile_configuration::packed::specific_data_v1, data),
                          &midiCIProcessor::profile_specific_data},

    message_dispatch_info{MIDICI_PE_CAPABILITY, sizeof(ci::property_exchange::packed::capabilities_v1),
                          sizeof(ci::property_exchange::packed::capabilities_v2), &midiCIProcessor::pe_capabilities},
    message_dispatch_info{MIDICI_PE_CAPABILITYREPLY, sizeof(ci::property_exchange::packed::capabilities_reply_v1),
                          sizeof(ci::property_exchange::packed::capabilities_reply_v2), &midiCIProcessor::pe_capabilities_reply},
    message_dispatch_info{MIDICI_PE_GET, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &midiCIProcessor::property_exchange},
    message_dispatch_info{MIDICI_PE_GETREPLY, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &midiCIProcessor::property_exchange},
    message_dispatch_info{MIDICI_PE_SET, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &midiCIProcessor::property_exchange},
    message_dispatch_info{MIDICI_PE_SETREPLY, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &midiCIProcessor::property_exchange},
    // message_dispatch_info{ MIDICI_PE_SUB,  },
    // message_dispatch_info{ MIDICI_PE_SUBREPLY, },
    // message_dispatch_info{ MIDICI_PE_NOTIFY, },

    message_dispatch_info{MIDICI_PI_CAPABILITY, 0, 0, &midiCIProcessor::process_inquiry_capabilities},
    message_dispatch_info{MIDICI_PI_CAPABILITYREPLY, 0, sizeof(ci::process_inquiry::packed::capabilities_reply_v2),
                          &midiCIProcessor::process_inquiry_capabilities_reply},
    message_dispatch_info{MIDICI_PI_MM_REPORT, 0, sizeof(ci::process_inquiry::packed::midi_message_report_v2),
                          &midiCIProcessor::process_inquiry_midi_message_report},
    message_dispatch_info{MIDICI_PI_MM_REPORT_REPLY, 0,
                          sizeof(ci::process_inquiry::packed::midi_message_report_reply_v2),
                          &midiCIProcessor::process_inquiry_midi_message_report_reply},
    message_dispatch_info{MIDICI_PI_MM_REPORT_END, 0, 0, &midiCIProcessor::process_inquiry_midi_message_report_end},

    message_dispatch_info{MIDICI_DISCOVERY, sizeof(ci::packed::discovery_v1), sizeof(ci::packed::discovery_v2),
                          &midiCIProcessor::discovery},
    message_dispatch_info{MIDICI_DISCOVERY_REPLY, sizeof(ci::packed::discovery_reply_v1),
                          sizeof(ci::packed::discovery_reply_v2), &midiCIProcessor::discovery_reply},
    message_dispatch_info{MIDICI_ENDPOINTINFO, sizeof(ci::packed::endpoint_info_v1),
                          sizeof(ci::packed::endpoint_info_v1), &midiCIProcessor::endpoint_info},
    message_dispatch_info{MIDICI_ENDPOINTINFO_REPLY, offsetof(ci::packed::endpoint_info_reply_v1, data),
                          offsetof(ci::packed::endpoint_info_reply_v1, data), &midiCIProcessor::endpoint_info_reply},
    message_dispatch_info{MIDICI_ACK, offsetof(ci::packed::ack_v1, message), offsetof(ci::packed::ack_v1, message),
                          &midiCIProcessor::ack},
    message_dispatch_info{MIDICI_INVALIDATEMUID, sizeof(ci::packed::invalidate_muid_v1),
                          sizeof(ci::packed::invalidate_muid_v1), &midiCIProcessor::invalidate_muid},
    message_dispatch_info{MIDICI_NAK, sizeof(ci::packed::nak_v1), offsetof(ci::packed::nak_v2, message),
                          &midiCIProcessor::nak},
  };
  auto const *const h = std::bit_cast<ci::packed::header const *>(buffer_.data());
  midici_.ciType = static_cast<std::uint8_t>(h->sub_id_2);
  midici_.ciVer = static_cast<std::uint8_t>(h->version);
  midici_.remoteMUID = ci::from_le7(h->source_muid);
  midici_.localMUID = ci::from_le7(h->destination_muid);

  auto const first = std::begin(messages);
  auto const last = std::end(messages);
  auto const pred = [](message_dispatch_info const &a, message_dispatch_info const &b) { return a.type < b.type; };
  assert(std::is_sorted(first, last, pred));
  auto const pos = std::lower_bound(first, last, message_dispatch_info{midici_.ciType, 0, 0, nullptr}, pred);
  if (pos == last || pos->type != midici_.ciType) {
    // An unknown message type.
    consumer_ = &midiCIProcessor::discard;
    count_ = 0;

    callbacks_.unknown_midici(midici_);
  } else if (midici_.localMUID != M2_CI_BROADCAST && !callbacks_.check_muid(midici_.umpGroup, midici_.localMUID)) {
    // The message wasn't intended for us.
    consumer_ = &midiCIProcessor::discard;
    count_ = 0;
  } else {
    assert(pos->consumer != nullptr && "consumer must not be null");
    consumer_ = pos->consumer;
    count_ = midici_.ciVer == 1 ? pos->v1size : pos->v2size;
    if (count_ == 0) {
      (this->*consumer_)();
    }
  }
  pos_ = 0;
}

// discovery
// ~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::discovery() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    callbacks_.discovery(midici_, ci::discovery{*v});
  };
  using namespace ci::packed;
  if (midici_.ciVer == 1) {
    handler(std::bit_cast<discovery_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<discovery_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// discovery reply
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::discovery_reply() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    callbacks_.discovery_reply(midici_, ci::discovery_reply{*v});
  };
  if (midici_.ciVer == 1) {
    handler(std::bit_cast<ci::packed::discovery_reply_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::packed::discovery_reply_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// invalidate muid
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::invalidate_muid() {
  using type = ci::packed::invalidate_muid_v1;
  callbacks_.invalidate_muid(midici_, ci::invalidate_muid{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// ack
// ~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::ack() {
  using type = ci::packed::ack_v1;
  auto const *const ptr = std::bit_cast<type const *>(buffer_.data());
  auto const message_length = ci::from_le7(ptr->message_length);
  if (pos_ == offsetof(type, message) && message_length > 0) {
    // We've got the fixed-size part of the message. Now wait for the variable-length message buffer.
    count_ = message_length;
    return;
  }
  assert(pos_ == offsetof(type, message) + message_length);
  callbacks_.ack(midici_, ci::ack{*ptr});
  consumer_ = &midiCIProcessor::discard;
}

// nak
// ~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::nak() {
  using v1_type = ci::packed::nak_v1;
  using v2_type = ci::packed::nak_v2;

  auto const handler = [this](unaligned_copyable auto const &reply) {
    assert(pos_ >= sizeof(reply));
    callbacks_.nak(midici_, ci::nak{reply});
    consumer_ = &midiCIProcessor::discard;
  };
  if (midici_.ciVer == 1) {
    assert(pos_ == sizeof(v1_type));
    handler(*std::bit_cast<v1_type const *>(buffer_.data()));
    return;
  }

  auto const *const v2ptr = std::bit_cast<v2_type const *>(buffer_.data());
  auto const message_length = ci::from_le7(v2ptr->message_length);
  if (pos_ == offsetof(ci::packed::nak_v2, message) && message_length > 0) {
    count_ = message_length;
    return;
  }
  assert(pos_ == offsetof(ci::packed::nak_v2, message) + message_length);
  handler(*v2ptr);
}

// endpoint info
// ~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::endpoint_info() {
  using type = ci::packed::endpoint_info_v1;
  assert(pos_ == sizeof(type));
  callbacks_.endpoint_info(midici_, ci::endpoint_info{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// endpoint info reply
// ~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::endpoint_info_reply() {
  using type = ci::packed::endpoint_info_reply_v1;
  auto const *const ptr = std::bit_cast<type const *>(buffer_.data());
  auto const data_length = ci::from_le7(ptr->data_length);
  if (pos_ == offsetof(type, data) && data_length > 0) {
    // We've got the basic structure. Now get the variable length data array.
    count_ = data_length;
    return;
  }
  assert(pos_ == offsetof(type, data) + data_length);
  callbacks_.endpoint_info_reply(midici_, ci::endpoint_info_reply{*ptr});
  consumer_ = &midiCIProcessor::discard;
}

// profile inquiry
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_inquiry() {
  profile_backend_.inquiry(midici_);
  consumer_ = &midiCIProcessor::discard;
}

// profile inquiry reply
// ~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_inquiry_reply() {
  using pt1_type = ci::profile_configuration::packed::inquiry_reply_v1_pt1;
  using pt2_type = ci::profile_configuration::packed::inquiry_reply_v1_pt2;
  auto const *const pt1 = std::bit_cast<pt1_type const *>(buffer_.data());
  auto const num_enabled = ci::from_le7(pt1->num_enabled);
  auto const num_enabled_size = num_enabled * sizeof(pt1->ids[0]);
  if (num_enabled > 0 && pos_ == offsetof(pt1_type, ids)) {
    // Wait for the variable length data following the first part and the fixed size portion of part 2.
    count_ = num_enabled_size + offsetof(pt2_type, ids);
    return;
  }

  auto const *const pt2 = std::bit_cast<pt2_type const *>(buffer_.data() + offsetof(pt1_type, ids) + num_enabled_size);
  auto const num_disabled = ci::from_le7(pt2->num_disabled);
  if (num_disabled > 0 && pos_ == offsetof(pt1_type, ids) + num_enabled_size + offsetof(pt2_type, ids)) {
    // Get the variable length "disabled" array.
    count_ = num_disabled * sizeof(pt2->ids[0]);
    return;
  }
  profile_backend_.inquiry_reply(midici_, ci::profile_configuration::inquiry_reply{*pt1, *pt2});
  consumer_ = &midiCIProcessor::discard;
}

// profile added
// ~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_added() {
  using type = ci::profile_configuration::packed::added_v1;
  assert(pos_ == sizeof(type));
  profile_backend_.added(midici_, ci::profile_configuration::added{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// profile removed
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_removed() {
  using type = ci::profile_configuration::packed::removed_v1;
  assert(pos_ == sizeof(type));
  profile_backend_.removed(midici_, ci::profile_configuration::removed{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// profile details inquiry
// ~~~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_details() {
  using type = ci::profile_configuration::packed::details_v1;
  assert(pos_ == sizeof(type));
  profile_backend_.details(midici_, ci::profile_configuration::details{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// profile details reply
// ~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_details_reply() {
  using type = ci::profile_configuration::packed::details_reply_v1;
  auto const *const reply = std::bit_cast<type const *>(buffer_.data());
  auto const data_length = ci::from_le7(reply->data_length);
  if (pos_ == offsetof(type, data) && data_length > 0) {
    count_ = data_length * sizeof(type::data[0]);
    return;
  }
  profile_backend_.details_reply(midici_, ci::profile_configuration::details_reply{*reply});
  consumer_ = &midiCIProcessor::discard;
}

// profile on
// ~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_on() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    profile_backend_.on(midici_, ci::profile_configuration::on{*v});
  };
  if (midici_.ciVer == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::on_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::on_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// profile off
// ~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_off() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    profile_backend_.off(midici_, ci::profile_configuration::off{*v});
  };
  if (midici_.ciVer == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::off_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::off_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// profile enabled
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_enabled() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    profile_backend_.enabled(midici_, ci::profile_configuration::enabled{*v});
  };
  if (midici_.ciVer == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::enabled_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::enabled_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// profile disabled
// ~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_disabled() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    profile_backend_.disabled(midici_, ci::profile_configuration::disabled{*v});
  };
  if (midici_.ciVer == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::disabled_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::disabled_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// profile specific data
// ~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::profile_specific_data() {
  using type = ci::profile_configuration::packed::specific_data_v1;
  auto const *const reply = std::bit_cast<type const *>(buffer_.data());
  if (auto const data_length = ci::from_le7(reply->data_length); pos_ == offsetof(type, data) && data_length > 0) {
    count_ = data_length * sizeof(type::data[0]);
    return;
  }
  profile_backend_.specific_data(midici_, ci::profile_configuration::specific_data{*reply});
  consumer_ = &midiCIProcessor::discard;
}

// pe capabilities
// ~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::pe_capabilities() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    pe_backend_.capabilities(midici_, ci::property_exchange::capabilities{*v});
  };
  if (midici_.ciVer == 1) {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// pe capabilities reply
// ~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::pe_capabilities_reply() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    pe_backend_.capabilities_reply(midici_, ci::property_exchange::capabilities_reply{*v});
  };
  if (midici_.ciVer == 1) {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_reply_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_reply_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// property exchange
// ~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::property_exchange() {
  using ci::property_exchange::packed::property_exchange_pt1;
  using ci::property_exchange::packed::property_exchange_pt2;
  auto size = offsetof(property_exchange_pt1, header);
  auto const *const pt1 = std::bit_cast<property_exchange_pt1 const *>(buffer_.data());
  auto const header_length = ci::from_le7(pt1->header_length);
  if (pos_ == size && header_length > 0) {
    count_ = header_length * sizeof(pt1->header[0]);
    return;
  }
  size += header_length;
  constexpr auto pt2_size = offsetof(property_exchange_pt2, data);
  if (pos_ == size) {
    count_ = pt2_size;
    return;
  }

  auto const *const pt2 = std::bit_cast<property_exchange_pt2 const *>(buffer_.data() + size);
  size += pt2_size;
  auto const data_length = ci::from_le7(pt2->data_length);
  if (pos_ == size && data_length > 0) {
    count_ = data_length * sizeof(pt2->data[0]);
    return;
  }

  ci::property_exchange::chunk_info chunk;
  chunk.number_of_chunks = ci::from_le7(pt2->number_of_chunks);
  chunk.chunk_number = ci::from_le7(pt2->chunk_number);

  ci::property_exchange::property_exchange pe;
  pe.request_id = static_cast<std::uint8_t>(pt1->request_id);
  pe.header = std::span<char const>{std::bit_cast<char const *>(&pt1->header[0]), header_length};
  pe.data = std::span<char const>{std::bit_cast<char const *>(&pt2->data[0]), data_length};

  switch (midici_.ciType) {
  case MIDICI_PE_GET: pe_backend_.get(midici_, chunk, pe); break;
  case MIDICI_PE_GETREPLY: pe_backend_.get_reply(midici_, chunk, pe); break;
  case MIDICI_PE_SET: pe_backend_.set(midici_, chunk, pe); break;
  case MIDICI_PE_SETREPLY: pe_backend_.set_reply(midici_, chunk, pe); break;
  default: assert(false); break;
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry capabilities
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::process_inquiry_capabilities() {
  if (midici_.ciVer > 1) {
    process_inquiry_backend_.capabilities(midici_);
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry capabilities reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::process_inquiry_capabilities_reply() {
  if (midici_.ciVer > 1) {
    process_inquiry_backend_.capabilities_reply(
        midici_, ci::process_inquiry::capabilities_reply{
                     *std::bit_cast<ci::process_inquiry::packed::capabilities_reply_v2 const *>(buffer_.data())});
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry midi message report
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::process_inquiry_midi_message_report() {
  if (midici_.ciVer > 1) {
    process_inquiry_backend_.midi_message_report(
        midici_, ci::process_inquiry::midi_message_report{
                     *std::bit_cast<ci::process_inquiry::packed::midi_message_report_v2 const *>(buffer_.data())});
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry midi message report reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::process_inquiry_midi_message_report_reply() {
  if (midici_.ciVer > 1) {
    process_inquiry_backend_.midi_message_report_reply(
        midici_,
        ci::process_inquiry::midi_message_report_reply{
            *std::bit_cast<ci::process_inquiry::packed::midi_message_report_reply_v2 const *>(buffer_.data())});
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry midi message report end
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::process_inquiry_midi_message_report_end() {
  if (midici_.ciVer > 1) {
    process_inquiry_backend_.midi_message_report_end(midici_);
  }
  consumer_ = &midiCIProcessor::discard;
}

template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::overflow() {
  count_ = 0;
  pos_ = 0;
  callbacks_.buffer_overflow();
  consumer_ = &midiCIProcessor::discard;
}

// processMIDICI
// ~~~~~~~~~~~~~
template <discovery_backend Callbacks, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<Callbacks, ProfileBackend, PEBackend, PIBackend>::processMIDICI(std::byte const s7) {
  assert((s7 & std::byte{0b10000000}) == std::byte{0});
  if (count_ > 0) {
    if (pos_ >= buffer_.size()) {
      this->overflow();
      return;
    }
    buffer_[pos_++] = s7;
    --count_;
  }
  if (count_ == 0) {
    (this->*consumer_)();
  }
}

}  // end namespace midi2

#endif  // MIDI2_MIDICIPROCESSOR_HPP
