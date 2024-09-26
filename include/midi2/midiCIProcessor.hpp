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
#include <span>
#include <string>
#include <type_traits>

#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

using MIDICI = ci::MIDICI;

using profile_span = std::span<std::byte, 5>;
constexpr auto bytes = std::span<std::byte>{};

template <typename T> concept management_backend = requires(T && v) {
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

  { v.get(MIDICI{}, ci::property_exchange::get{}) } -> std::same_as<void>;
  { v.get_reply(MIDICI{}, ci::property_exchange::get_reply{}) } -> std::same_as<void>;
  { v.set(MIDICI{}, ci::property_exchange::set{}) } -> std::same_as<void>;
  { v.set_reply(MIDICI{}, ci::property_exchange::set_reply{}) } -> std::same_as<void>;

  { v.subscription(MIDICI{}, ci::property_exchange::subscription{}) } -> std::same_as<void>;
  { v.subscription_reply(MIDICI{}, ci::property_exchange::subscription_reply{}) } -> std::same_as<void>;
  { v.notify(MIDICI{}, ci::property_exchange::notify{}) } -> std::same_as<void>;
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

  management_callbacks &operator=(management_callbacks const &) = default;
  management_callbacks &operator=(management_callbacks &&) noexcept = default;

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

  profile_callbacks &operator=(profile_callbacks const &) = default;
  profile_callbacks &operator=(profile_callbacks &&) noexcept = default;

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

  virtual void get(MIDICI const &, ci::property_exchange::get const &) { /* do nothing */ }
  virtual void get_reply(MIDICI const &, ci::property_exchange::get_reply const &) { /* do nothing */ }
  virtual void set(MIDICI const &, ci::property_exchange::set const &) { /* do nothing */ }
  virtual void set_reply(MIDICI const &, ci::property_exchange::set_reply const &) { /* do nothing */ }

  virtual void subscription(MIDICI const &, ci::property_exchange::subscription const &) { /* do nothing */ }
  virtual void subscription_reply(MIDICI const &, ci::property_exchange::subscription_reply const &) { /* do nothing */ }
  virtual void notify(MIDICI const &, ci::property_exchange::notify const &) { /* do nothing */ }
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

template <management_backend ManagementBackend = management_callbacks,
          profile_backend ProfileBackend = profile_callbacks,
          property_exchange_backend PEBackend = property_exchange_callbacks,
          process_inquiry_backend PIBackend = process_inquiry_callbacks>
class midiCIProcessor {
public:
  explicit midiCIProcessor(ManagementBackend management = ManagementBackend{},
                           ProfileBackend profile = ProfileBackend{}, PEBackend pe_backend = PEBackend{},
                           PIBackend pi_backend = PIBackend{})
      : management_backend_{management},
        profile_backend_{profile},
        property_exchange_backend_{pe_backend},
        process_inquiry_backend_{pi_backend} {}

  void startSysex7(std::uint8_t group, std::byte deviceId);
  void endSysex7() {}

  void processMIDICI(std::byte s7Byte);

private:
  static constexpr auto header_size = sizeof(ci::packed::header);

  [[no_unique_address]] ManagementBackend management_backend_;
  [[no_unique_address]] ProfileBackend profile_backend_;
  [[no_unique_address]] PEBackend property_exchange_backend_;
  [[no_unique_address]] PIBackend process_inquiry_backend_;

  using consumer_fn = void (midiCIProcessor::*)();

  std::size_t count_ = header_size;
  unsigned pos_ = 0;
  consumer_fn consumer_ = &midiCIProcessor::header;

  MIDICI midici_;
  std::array<std::byte, 512> buffer_{};

  void discard();
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
template <management_backend C> midiCIProcessor(C) -> midiCIProcessor<C>;
template <management_backend C> midiCIProcessor(std::reference_wrapper<C>) -> midiCIProcessor<C &>;
template <management_backend C, profile_backend P> midiCIProcessor(C, P) -> midiCIProcessor<C, P>;
template <management_backend C, profile_backend P, property_exchange_backend PE>
midiCIProcessor(C, P, PE) -> midiCIProcessor<C, P, PE>;
template <management_backend C, profile_backend P, property_exchange_backend PE, process_inquiry_backend PI>
midiCIProcessor(C, P, PE, PI) -> midiCIProcessor<C, P, PE, PI>;

template <management_backend C, profile_backend P>
midiCIProcessor(std::reference_wrapper<C>, std::reference_wrapper<P>) -> midiCIProcessor<C &, P &>;
template <management_backend C, profile_backend P, property_exchange_backend PE>
midiCIProcessor(std::reference_wrapper<C>, std::reference_wrapper<P>, std::reference_wrapper<PE>)
    -> midiCIProcessor<C &, P &, PE &>;
template <management_backend C, profile_backend P, property_exchange_backend PE, process_inquiry_backend PI>
midiCIProcessor(std::reference_wrapper<C>, std::reference_wrapper<P>, std::reference_wrapper<PE>,
                std::reference_wrapper<PI>) -> midiCIProcessor<C &, P &, PE &, PI &>;

template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::startSysex7(std::uint8_t group,
                                                                                           std::byte deviceId) {
  midici_ = MIDICI{};
  midici_.umpGroup = group;
  midici_.params.deviceId = static_cast<std::uint8_t>(deviceId);

  count_ = header_size;
  pos_ = 0;
  consumer_ = &midiCIProcessor::header;
}

template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::header() {
  struct message_dispatch_info {
    ci_message type;
    std::uint8_t v1size;
    std::uint8_t v2size;
    consumer_fn consumer;
  };

  static std::array const messages = {
      message_dispatch_info{ci_message::profile_inquiry, 0, 0, &midiCIProcessor::profile_inquiry},
      message_dispatch_info{ci_message::profile_inquiry_reply,
                            offsetof(ci::profile_configuration::packed::inquiry_reply_v1_pt1, ids),
                            offsetof(ci::profile_configuration::packed::inquiry_reply_v1_pt1, ids),
                            &midiCIProcessor::profile_inquiry_reply},
      message_dispatch_info{ci_message::profile_set_on, sizeof(ci::profile_configuration::packed::on_v1),
                            sizeof(ci::profile_configuration::packed::on_v2), &midiCIProcessor::profile_on},
      message_dispatch_info{ci_message::profile_set_off, sizeof(ci::profile_configuration::packed::off_v1),
                            sizeof(ci::profile_configuration::packed::off_v2), &midiCIProcessor::profile_off},
      message_dispatch_info{ci_message::profile_enabled, sizeof(ci::profile_configuration::packed::enabled_v1),
                            sizeof(ci::profile_configuration::packed::enabled_v2), &midiCIProcessor::profile_enabled},
      message_dispatch_info{ci_message::profile_disabled, sizeof(ci::profile_configuration::packed::disabled_v1),
                            sizeof(ci::profile_configuration::packed::disabled_v2), &midiCIProcessor::profile_disabled},
      message_dispatch_info{ci_message::profile_added, sizeof(ci::profile_configuration::packed::added_v1),
                            sizeof(ci::profile_configuration::packed::added_v1), &midiCIProcessor::profile_added},
      message_dispatch_info{ci_message::profile_removed, sizeof(ci::profile_configuration::packed::removed_v1),
                            sizeof(ci::profile_configuration::packed::removed_v1), &midiCIProcessor::profile_removed},
      message_dispatch_info{ci_message::profile_details, sizeof(ci::profile_configuration::packed::details_v1),
                            sizeof(ci::profile_configuration::packed::details_v1), &midiCIProcessor::profile_details},
      message_dispatch_info{
          ci_message::profile_details_reply, offsetof(ci::profile_configuration::packed::details_reply_v1, data),
          offsetof(ci::profile_configuration::packed::details_reply_v1, data), &midiCIProcessor::profile_details_reply},
      message_dispatch_info{
          ci_message::profile_specific_data, offsetof(ci::profile_configuration::packed::specific_data_v1, data),
          offsetof(ci::profile_configuration::packed::specific_data_v1, data), &midiCIProcessor::profile_specific_data},

      message_dispatch_info{ci_message::pe_capability, sizeof(ci::property_exchange::packed::capabilities_v1),
                            sizeof(ci::property_exchange::packed::capabilities_v2), &midiCIProcessor::pe_capabilities},
      message_dispatch_info{
          ci_message::pe_capability_reply, sizeof(ci::property_exchange::packed::capabilities_reply_v1),
          sizeof(ci::property_exchange::packed::capabilities_reply_v2), &midiCIProcessor::pe_capabilities_reply},
      message_dispatch_info{ci_message::pe_get, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &midiCIProcessor::property_exchange},
      message_dispatch_info{
          ci_message::pe_get_reply, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &midiCIProcessor::property_exchange},
      message_dispatch_info{ci_message::pe_set, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &midiCIProcessor::property_exchange},
      message_dispatch_info{
          ci_message::pe_set_reply, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &midiCIProcessor::property_exchange},
      message_dispatch_info{ci_message::pe_sub, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
                            &midiCIProcessor::property_exchange},
      message_dispatch_info{
          ci_message::pe_sub_reply, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &midiCIProcessor::property_exchange},
      message_dispatch_info{
          ci_message::pe_notify, offsetof(ci::property_exchange::packed::property_exchange_pt1, header),
          offsetof(ci::property_exchange::packed::property_exchange_pt1, header), &midiCIProcessor::property_exchange},

      message_dispatch_info{ci_message::pi_capability, 0, 0, &midiCIProcessor::process_inquiry_capabilities},
      message_dispatch_info{ci_message::pi_capability_reply, 0,
                            sizeof(ci::process_inquiry::packed::capabilities_reply_v2),
                            &midiCIProcessor::process_inquiry_capabilities_reply},
      message_dispatch_info{ci_message::pi_mm_report, 0, sizeof(ci::process_inquiry::packed::midi_message_report_v2),
                            &midiCIProcessor::process_inquiry_midi_message_report},
      message_dispatch_info{ci_message::pi_mm_report_reply, 0,
                            sizeof(ci::process_inquiry::packed::midi_message_report_reply_v2),
                            &midiCIProcessor::process_inquiry_midi_message_report_reply},
      message_dispatch_info{ci_message::pi_mm_report_end, 0, 0,
                            &midiCIProcessor::process_inquiry_midi_message_report_end},

      message_dispatch_info{ci_message::discovery, sizeof(ci::packed::discovery_v1), sizeof(ci::packed::discovery_v2),
                            &midiCIProcessor::discovery},
      message_dispatch_info{ci_message::discovery_reply, sizeof(ci::packed::discovery_reply_v1),
                            sizeof(ci::packed::discovery_reply_v2), &midiCIProcessor::discovery_reply},
      message_dispatch_info{ci_message::endpoint_info, sizeof(ci::packed::endpoint_info_v1),
                            sizeof(ci::packed::endpoint_info_v1), &midiCIProcessor::endpoint_info},
      message_dispatch_info{ci_message::endpoint_info_reply, offsetof(ci::packed::endpoint_info_reply_v1, data),
                            offsetof(ci::packed::endpoint_info_reply_v1, data), &midiCIProcessor::endpoint_info_reply},
      message_dispatch_info{ci_message::ack, offsetof(ci::packed::ack_v1, message),
                            offsetof(ci::packed::ack_v1, message), &midiCIProcessor::ack},
      message_dispatch_info{ci_message::invalidate_muid, sizeof(ci::packed::invalidate_muid_v1),
                            sizeof(ci::packed::invalidate_muid_v1), &midiCIProcessor::invalidate_muid},
      message_dispatch_info{ci_message::nak, sizeof(ci::packed::nak_v1), offsetof(ci::packed::nak_v2, message),
                            &midiCIProcessor::nak},
  };
  auto const *const h = std::bit_cast<ci::packed::header const *>(buffer_.data());
  midici_.ciType = static_cast<ci_message>(h->sub_id_2);
  midici_.params.ciVer = static_cast<std::uint8_t>(h->version);
  midici_.params.remoteMUID = ci::from_le7(h->source_muid);
  midici_.params.localMUID = ci::from_le7(h->destination_muid);

  auto const first = std::begin(messages);
  auto const last = std::end(messages);
  auto const pred = [](message_dispatch_info const &a, message_dispatch_info const &b) { return a.type < b.type; };
  assert(std::is_sorted(first, last, pred));
  if (auto const pos = std::lower_bound(first, last, message_dispatch_info{midici_.ciType, 0, 0, nullptr}, pred);
      pos == last || pos->type != midici_.ciType) {
    // An unknown message type.
    consumer_ = &midiCIProcessor::discard;
    count_ = 0;

    management_backend_.unknown_midici(midici_);
  } else if (midici_.params.localMUID != M2_CI_BROADCAST &&
             !management_backend_.check_muid(midici_.umpGroup, midici_.params.localMUID)) {
    // The message wasn't intended for us.
    consumer_ = &midiCIProcessor::discard;
    count_ = 0;
  } else {
    assert(pos->consumer != nullptr && "consumer must not be null");
    consumer_ = pos->consumer;
    count_ = midici_.params.ciVer == 1 ? pos->v1size : pos->v2size;
    if (count_ == 0) {
      (this->*consumer_)();
    }
  }
  pos_ = 0;
}

// discovery
// ~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::discovery() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    management_backend_.discovery(midici_, ci::discovery{*v});
  };
  using namespace ci::packed;
  if (midici_.params.ciVer == 1) {
    handler(std::bit_cast<discovery_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<discovery_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// discovery reply
// ~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::discovery_reply() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    management_backend_.discovery_reply(midici_, ci::discovery_reply{*v});
  };
  if (midici_.params.ciVer == 1) {
    handler(std::bit_cast<ci::packed::discovery_reply_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::packed::discovery_reply_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// invalidate muid
// ~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::invalidate_muid() {
  using type = ci::packed::invalidate_muid_v1;
  management_backend_.invalidate_muid(midici_, ci::invalidate_muid{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// ack
// ~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::ack() {
  using type = ci::packed::ack_v1;
  auto const *const ptr = std::bit_cast<type const *>(buffer_.data());
  auto const message_length = ci::from_le7(ptr->message_length);
  if (pos_ == offsetof(type, message) && message_length > 0) {
    // We've got the fixed-size part of the message. Now wait for the variable-length message buffer.
    count_ = message_length;
    return;
  }
  assert(pos_ == offsetof(type, message) + message_length);
  management_backend_.ack(midici_, ci::ack{*ptr});
  consumer_ = &midiCIProcessor::discard;
}

// nak
// ~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::nak() {
  using v1_type = ci::packed::nak_v1;
  using v2_type = ci::packed::nak_v2;

  auto const handler = [this](unaligned_copyable auto const &reply) {
    management_backend_.nak(midici_, ci::nak{reply});
    consumer_ = &midiCIProcessor::discard;
  };
  if (midici_.params.ciVer == 1) {
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
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::endpoint_info() {
  using type = ci::packed::endpoint_info_v1;
  assert(pos_ == sizeof(type));
  management_backend_.endpoint_info(midici_, ci::endpoint_info{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// endpoint info reply
// ~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::endpoint_info_reply() {
  using type = ci::packed::endpoint_info_reply_v1;
  auto const *const ptr = std::bit_cast<type const *>(buffer_.data());
  auto const data_length = ci::from_le7(ptr->data_length);
  if (pos_ == offsetof(type, data) && data_length > 0) {
    // We've got the basic structure. Now get the variable length data array.
    count_ = data_length;
    return;
  }
  assert(pos_ == offsetof(type, data) + data_length);
  management_backend_.endpoint_info_reply(midici_, ci::endpoint_info_reply{*ptr});
  consumer_ = &midiCIProcessor::discard;
}

// profile inquiry
// ~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_inquiry() {
  profile_backend_.inquiry(midici_);
  consumer_ = &midiCIProcessor::discard;
}

// profile inquiry reply
// ~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_inquiry_reply() {
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
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_added() {
  using type = ci::profile_configuration::packed::added_v1;
  assert(pos_ == sizeof(type));
  profile_backend_.added(midici_, ci::profile_configuration::added{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// profile removed
// ~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_removed() {
  using type = ci::profile_configuration::packed::removed_v1;
  assert(pos_ == sizeof(type));
  profile_backend_.removed(midici_, ci::profile_configuration::removed{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// profile details inquiry
// ~~~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_details() {
  using type = ci::profile_configuration::packed::details_v1;
  assert(pos_ == sizeof(type));
  profile_backend_.details(midici_, ci::profile_configuration::details{*std::bit_cast<type const *>(buffer_.data())});
  consumer_ = &midiCIProcessor::discard;
}

// profile details reply
// ~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_details_reply() {
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
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_on() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    profile_backend_.on(midici_, ci::profile_configuration::on{*v});
  };
  if (midici_.params.ciVer == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::on_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::on_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// profile off
// ~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_off() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    profile_backend_.off(midici_, ci::profile_configuration::off{*v});
  };
  if (midici_.params.ciVer == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::off_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::off_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// profile enabled
// ~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_enabled() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    profile_backend_.enabled(midici_, ci::profile_configuration::enabled{*v});
  };
  if (midici_.params.ciVer == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::enabled_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::enabled_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// profile disabled
// ~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_disabled() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    profile_backend_.disabled(midici_, ci::profile_configuration::disabled{*v});
  };
  if (midici_.params.ciVer == 1) {
    handler(std::bit_cast<ci::profile_configuration::packed::disabled_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::profile_configuration::packed::disabled_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// profile specific data
// ~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::profile_specific_data() {
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
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::pe_capabilities() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    property_exchange_backend_.capabilities(midici_, ci::property_exchange::capabilities{*v});
  };
  if (midici_.params.ciVer == 1) {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// pe capabilities reply
// ~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::pe_capabilities_reply() {
  auto const handler = [this](unaligned_copyable auto const *const v) {
    assert(pos_ == sizeof(*v));
    property_exchange_backend_.capabilities_reply(midici_, ci::property_exchange::capabilities_reply{*v});
  };
  if (midici_.params.ciVer == 1) {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_reply_v1 const *>(buffer_.data()));
  } else {
    handler(std::bit_cast<ci::property_exchange::packed::capabilities_reply_v2 const *>(buffer_.data()));
  }
  consumer_ = &midiCIProcessor::discard;
}

// property exchange
// ~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::property_exchange() {
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

  using chunk_info = ci::property_exchange::property_exchange::chunk_info;
  auto const chunk = chunk_info{ci::from_le7(pt2->number_of_chunks), ci::from_le7(pt2->chunk_number)};
  auto const request = static_cast<std::uint8_t>(pt1->request_id);
  auto const header = std::span<char const>{std::bit_cast<char const *>(&pt1->header[0]), header_length};
  auto const data = std::span<char const>{std::bit_cast<char const *>(&pt2->data[0]), data_length};

  using enum ci_message;
  switch (midici_.ciType) {
  case pe_get: property_exchange_backend_.get(midici_, ci::property_exchange::get{chunk, request, header}); break;
  case pe_get_reply:
    property_exchange_backend_.get_reply(midici_, ci::property_exchange::get_reply{chunk, request, header, data});
    break;
  case pe_set: property_exchange_backend_.set(midici_, ci::property_exchange::set{chunk, request, header, data}); break;
  case pe_set_reply:
    property_exchange_backend_.set_reply(midici_, ci::property_exchange::set_reply{chunk, request, header, data});
    break;
  case pe_sub:
    property_exchange_backend_.subscription(midici_, ci::property_exchange::subscription{chunk, request, header, data});
    break;
  case pe_sub_reply:
    property_exchange_backend_.subscription_reply(
        midici_, ci::property_exchange::subscription_reply{chunk, request, header, data});
    break;
  case pe_notify:
    property_exchange_backend_.notify(midici_, ci::property_exchange::notify{chunk, request, header, data});
    break;
  default: assert(false); break;
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry capabilities
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::process_inquiry_capabilities() {
  if (midici_.params.ciVer > 1) {
    process_inquiry_backend_.capabilities(midici_);
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry capabilities reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::process_inquiry_capabilities_reply() {
  if (midici_.params.ciVer > 1) {
    process_inquiry_backend_.capabilities_reply(
        midici_, ci::process_inquiry::capabilities_reply{
                     *std::bit_cast<ci::process_inquiry::packed::capabilities_reply_v2 const *>(buffer_.data())});
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry midi message report
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::process_inquiry_midi_message_report() {
  if (midici_.params.ciVer > 1) {
    process_inquiry_backend_.midi_message_report(
        midici_, ci::process_inquiry::midi_message_report{
                     *std::bit_cast<ci::process_inquiry::packed::midi_message_report_v2 const *>(buffer_.data())});
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry midi message report reply
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend,
                     PIBackend>::process_inquiry_midi_message_report_reply() {
  if (midici_.params.ciVer > 1) {
    process_inquiry_backend_.midi_message_report_reply(
        midici_,
        ci::process_inquiry::midi_message_report_reply{
            *std::bit_cast<ci::process_inquiry::packed::midi_message_report_reply_v2 const *>(buffer_.data())});
  }
  consumer_ = &midiCIProcessor::discard;
}

// process inquiry midi message report end
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend,
                     PIBackend>::process_inquiry_midi_message_report_end() {
  if (midici_.params.ciVer > 1) {
    process_inquiry_backend_.midi_message_report_end(midici_);
  }
  consumer_ = &midiCIProcessor::discard;
}

// discard
// ~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::discard() {
  pos_ = 0;
  count_ = buffer_.size();
}

// overflow
// ~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::overflow() {
  count_ = 0;
  pos_ = 0;
  management_backend_.buffer_overflow();
  consumer_ = &midiCIProcessor::discard;
}

// processMIDICI
// ~~~~~~~~~~~~~
template <management_backend ManagementBackend, profile_backend ProfileBackend, property_exchange_backend PEBackend,
          process_inquiry_backend PIBackend>
void midiCIProcessor<ManagementBackend, ProfileBackend, PEBackend, PIBackend>::processMIDICI(std::byte const s7) {
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
