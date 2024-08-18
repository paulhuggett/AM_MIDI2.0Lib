#include <algorithm>
#include <bit>
#include <functional>
#include <numeric>

#include "midi2/bitfield.hpp"
#include "midi2/umpProcessor.hpp"
#include "midi2/ump_types.hpp"

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace midi2 {

std::ostream& operator<<(std::ostream& os, umpCommon const& common);
std::ostream& operator<<(std::ostream& os, umpCommon const& common) {
  return os << "{ group=" << static_cast<unsigned>(common.group)
            << ", messageType=" << static_cast<unsigned>(common.messageType)
            << ", status=" << static_cast<unsigned>(common.status) << " }";
};

std::ostream& operator<<(std::ostream& os, midi2::umpGeneric const& generic);
std::ostream& operator<<(std::ostream& os, midi2::umpGeneric const& generic) {
  return os << "{ common:" << generic.common << ", value=" << generic.value
            << " }";
}

std::ostream& operator<<(std::ostream& os, umpCVM const& cvm);
std::ostream& operator<<(std::ostream& os, umpCVM const& cvm) {
  return os << "{ common:" << cvm.common
            << ", channel=" << static_cast<unsigned>(cvm.channel)
            << ", note=" << static_cast<unsigned>(cvm.note)
            << ", value=" << cvm.value << ", index=" << cvm.index
            << ", bank=" << static_cast<unsigned>(cvm.bank)
            << ", flag1=" << cvm.flag1 << ", flag2=" << cvm.flag2 << " }";
};

std::ostream& operator<<(std::ostream& os, umpData const& data);
std::ostream& operator<<(std::ostream& os, umpData const& data) {
  os << "{ common:" << data.common
     << ", streamId=" << static_cast<unsigned>(data.streamId)
     << ", form=" << static_cast<unsigned>(data.form) << ", data=[";
  std::copy(std::begin(data.data), std::end(data.data),
            std::ostream_iterator<unsigned>(os, ","));
  os << "] }";
  return os;
}

std::ostream& operator<<(std::ostream& os,
                         function_block_info::fbdirection const direction);
std::ostream& operator<<(std::ostream& os,
                         function_block_info::fbdirection const direction) {
  using enum function_block_info::fbdirection;
  char const* str = "";
  switch (direction) {
  case reserved: str = "reserved"; break;
  case input: str = "input"; break;
  case output: str = "output"; break;
  case bidirectional: str = "bidirectional"; break;
  default: str = "unknown"; break;
  }
  return os << str;
}

std::ostream& operator<<(std::ostream& os, function_block_info const& fbi);
std::ostream& operator<<(std::ostream& os, function_block_info const& fbi) {
  return os << "{ fbIdx=" << static_cast<unsigned>(fbi.fbIdx)
            << ", active=" << fbi.active << ", direction=" << fbi.direction
            << ", firstGroup=" << static_cast<unsigned>(fbi.firstGroup)
            << ", groupLength=" << static_cast<unsigned>(fbi.groupLength)
            << ", midiCIVersion=" << static_cast<unsigned>(fbi.midiCIVersion)
            << ", isMIDI1=" << static_cast<unsigned>(fbi.isMIDI1)
            << ", maxS8Streams=" << static_cast<unsigned>(fbi.maxS8Streams)
            << " }";
}

std::ostream& operator<<(std::ostream& os, chord::alteration const& alt);
std::ostream& operator<<(std::ostream& os, chord::alteration const& alt) {
  return os << "{ type=" << static_cast<unsigned>(alt.type)
            << ", degree=" << static_cast<unsigned>(alt.degree) << " }";
}

std::ostream& operator<<(std::ostream& os, chord const& c);
std::ostream& operator<<(std::ostream& os, chord const& c) {
  return os << "{ chShrpFlt=" << static_cast<unsigned>(c.chShrpFlt)
            << ", chTonic=" << static_cast<unsigned>(c.chTonic)
            << ", chType=" << static_cast<unsigned>(c.chType)
            << ", chAlt1=" << c.chAlt1 << ", chAlt2=" << c.chAlt2
            << ", chAlt3=" << c.chAlt3 << ", chAlt4=" << c.chAlt4
            << ", baShrpFlt=" << static_cast<unsigned>(c.baShrpFlt)
            << ", baTonic=" << static_cast<unsigned>(c.baTonic)
            << ", baType=" << static_cast<unsigned>(c.baType)
            << ", baAlt1=" << c.baAlt1 << ", baAlt2=" << c.baAlt2 << " }";
}

}  // end namespace midi2

namespace {

using midi2::pack;

class MockCallbacks final : public midi2::callbacks_base {
public:
  MOCK_METHOD(void, utility_message, (midi2::umpGeneric const&), (override));
  MOCK_METHOD(void, channel_voice_message, (midi2::umpCVM const&), (override));
  MOCK_METHOD(void, system_message, (midi2::umpGeneric const&), (override));
  MOCK_METHOD(void, send_out_sysex, (midi2::umpData const&), (override));

  MOCK_METHOD(void, flex_tempo, (std::uint8_t group, std::uint32_t num10nsPQN),
              (override));
  MOCK_METHOD(void, flex_time_sig,
              (std::uint8_t group, std::uint8_t numerator,
               std::uint8_t denominator, std::uint8_t num32Notes),
              (override));
  MOCK_METHOD(void, flex_metronome,
              (std::uint8_t group, std::uint8_t numClkpPriCli,
               std::uint8_t bAccP1, std::uint8_t bAccP2, std::uint8_t bAccP3,
               std::uint8_t numSubDivCli1, std::uint8_t numSubDivCli2),
              (override));
  MOCK_METHOD(void, flex_key_sig,
              (std::uint8_t group, std::uint8_t addrs, std::uint8_t channel,
               std::uint8_t sharpFlats, std::uint8_t tonic),
              (override));
  MOCK_METHOD(void, flex_chord,
              (std::uint8_t, std::uint8_t, std::uint8_t, midi2::chord const&),
              (override));
  MOCK_METHOD(void, flex_performance,
              (midi2::umpData const& mess, std::uint8_t addrs,
               std::uint8_t channel),
              (override));
  MOCK_METHOD(void, flex_lyric,
              (midi2::umpData const& mess, std::uint8_t addrs,
               std::uint8_t channel),
              (override));

  MOCK_METHOD(void, midiEndpoint, (std::uint8_t, std::uint8_t, std::uint8_t),
              (override));
  MOCK_METHOD(void, midiEndpointName, (midi2::umpData const&), (override));
  MOCK_METHOD(void, midiEndpointProdId, (midi2::umpData const&), (override));
  MOCK_METHOD(void, midiEndpointJRProtocolReq, (std::uint8_t, bool, bool),
              (override));
  MOCK_METHOD(void, midiEndpointInfo,
              (std::uint8_t, std::uint8_t, std::uint8_t, bool, bool, bool,
               bool),
              (override));
  MOCK_METHOD(void, midiEndpointDeviceInfo,
              ((std::array<std::uint8_t, 3> const&),
               (std::array<std::uint8_t, 2> const&),
               (std::array<std::uint8_t, 2> const&),
               (std::array<std::uint8_t, 4> const&)),
              (override));
  MOCK_METHOD(void, midiEndpointJRProtocolNotify,
              (std::uint8_t protocol, bool jrrx, bool jrtx), (override));

  MOCK_METHOD(void, functionBlock, (std::uint8_t fbIdx, std::uint8_t filter),
              (override));
  MOCK_METHOD(void, functionBlockInfo, (midi2::function_block_info const&),
              (override));
  MOCK_METHOD(void, functionBlockName, (midi2::umpData const&, std::uint8_t),
              (override));

  MOCK_METHOD(void, startOfSeq, (), (override));
  MOCK_METHOD(void, endOfFile, (), (override));

  MOCK_METHOD(void, unknownUMPMessage, (std::span<std::uint32_t>), (override));
};

}  // end anonymous namespace

template class midi2::umpProcessor<MockCallbacks&>;

namespace {

TEST(UMPProcessor, Noop) {
  midi2::umpGeneric message;
  message.common.group = 255;
  message.common.messageType = midi2::ump_message_type::utility;
  message.common.status = 0;
  message.value = 0;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, utility_message(message)).Times(1);

  midi2::umpProcessor p{std::ref(callbacks)};
  midi2::types::noop w1{};
  w1.mt = static_cast<std::uint8_t>(midi2::ump_message_type::utility);
  w1.reserved = 0;
  w1.status = 0b0000;
  w1.data = 0;
  p.processUMP(std::bit_cast<std::uint32_t>(w1));
}

constexpr std::uint32_t ump_cvm(midi2::status s) {
  static_assert(
      std::is_same_v<std::underlying_type_t<midi2::status>, std::uint8_t>,
      "status type must be a std::uint8_t");
  assert((s & 0x0F) == 0 &&
         "Bottom 4 bits of a channel voice message status enum must be 0");
  return std::uint32_t{s} >> 4;
}

constexpr auto ump_note_on = ump_cvm(midi2::status::note_on);

TEST(UMPProcessor, Midi1NoteOn) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint16_t{0x43};
  constexpr auto group = std::uint8_t{0};

  midi2::umpCVM message;
  message.common.group = group;
  message.common.messageType = midi2::ump_message_type::m1cvm;
  message.common.status = midi2::status::note_on >> 4;
  message.channel = channel;
  message.note = note_number;
  message.value = midi2::scaleUp(velocity, 7, 16);
  message.index = 0;
  message.bank = 0;
  message.flag1 = false;
  message.flag2 = false;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, channel_voice_message(message)).Times(1);

  midi2::umpProcessor p{std::ref(callbacks)};
  midi2::types::m1cvm_w1 w1{};
  w1.mt = static_cast<std::uint8_t>(midi2::ump_message_type::m1cvm);
  w1.group = group;
  w1.status = midi2::status::note_on >> 4;
  w1.channel = channel;
  w1.byte_a = note_number;
  w1.byte_b = velocity;
  p.processUMP(std::bit_cast<std::uint32_t>(w1));
}

TEST(UMPProcessor, Midi2NoteOn) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint16_t{0x432};
  constexpr auto group = std::uint8_t{0};

  midi2::umpCVM message;
  message.common.group = group;
  message.common.messageType = midi2::ump_message_type::m2cvm;
  message.common.status = midi2::status::note_on;
  message.channel = channel;
  message.note = note_number;
  message.value = velocity;
  message.index = 0;
  message.bank = 0;
  message.flag1 = false;
  message.flag2 = false;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, channel_voice_message(message)).Times(1);

  midi2::umpProcessor p{std::ref(callbacks)};
  p.processUMP(std::uint32_t{
      (static_cast<std::uint32_t>(midi2::ump_message_type::m2cvm) << 28) |
      (std::uint32_t{group} << 24) | (ump_note_on << 20) |
      (std::uint32_t{channel} << 16) | (std::uint32_t{note_number} << 8)});
  p.processUMP(std::uint32_t{velocity << 16});
}

using testing::AllOf;
using testing::ElementsAreArray;
using testing::Eq;
using testing::Field;
using testing::InSequence;

// The umpData type contains a std::span{} so we can't just lean on the default
// operator==() for comparing instances.
template <std::input_iterator Iterator>
auto UMPDataMatches(midi2::umpCommon const& common, std::uint8_t stream_id,
                    std::uint8_t form, Iterator first, Iterator last) {
  return AllOf(
      Field("common", &midi2::umpData::common, Eq(common)),
      Field("streamId", &midi2::umpData::streamId, Eq(stream_id)),
      Field("form", &midi2::umpData::form, Eq(form)),
      Field("data", &midi2::umpData::data, ElementsAreArray(first, last)));
};

TEST(UMPProcessor, Sysex8_16ByteMessage) {
  constexpr auto group = std::uint8_t{0};
  constexpr auto stream_id = std::uint8_t{0};
  constexpr auto start_form = std::uint8_t{0b0001};
  constexpr auto end_form = std::uint8_t{0b0011};

  std::array<std::uint8_t, 16> payload;
  std::iota(std::begin(payload), std::end(payload), 1);

  // The start_form packet can hold 13 data bytes.
  auto split_point = std::begin(payload);
  std::advance(split_point, 13);

  MockCallbacks callbacks;
  {
    InSequence _;
    midi2::umpCommon const common{group, midi2::ump_message_type::data, 0};
    EXPECT_CALL(callbacks, send_out_sysex(UMPDataMatches(
                               common, stream_id, start_form,
                               std::begin(payload), split_point)));
    EXPECT_CALL(callbacks,
                send_out_sysex(UMPDataMatches(common, stream_id, end_form,
                                              split_point, std::end(payload))));
  }
  midi2::umpProcessor p{std::ref(callbacks)};
  // Send 13 bytes
  p.processUMP(pack(
      (static_cast<std::uint8_t>(midi2::ump_message_type::data) << 4) | group,
      (start_form << 4) | 13U, stream_id, payload[0]));
  p.processUMP(pack(payload[1], payload[2], payload[3], payload[4]));
  p.processUMP(pack(payload[5], payload[6], payload[7], payload[8]));
  p.processUMP(pack(payload[9], payload[10], payload[11], payload[12]));
  // Send the final 3 bytes.
  p.processUMP(pack(
      (static_cast<std::uint8_t>(midi2::ump_message_type::data) << 4) | group,
      (end_form << 4) | 3U, stream_id, payload[13]));
  p.processUMP(pack(payload[14], payload[15], 0, 0));
  p.processUMP(0);
  p.processUMP(0);
}

TEST(UMPProcessor, PartialMessageThenClear) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint16_t{0x43};  // 7 bits
  constexpr auto group = std::uint8_t{0};

  midi2::umpCVM message;
  message.common.group = group;
  message.common.messageType = midi2::ump_message_type::m1cvm;
  message.common.status = midi2::status::note_on >> 4;
  message.channel = channel;
  message.note = note_number;
  message.value = midi2::scaleUp(velocity, 7, 16);
  message.index = 0;
  message.bank = 0;
  message.flag1 = false;
  message.flag2 = false;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, channel_voice_message(message)).Times(1);

  midi2::umpProcessor p{std::ref(callbacks)};
  // The first half of a 64-bit MIDI 2 note-on message.
  p.processUMP(pack(
      (static_cast<std::uint8_t>(midi2::ump_message_type::m2cvm) << 4) | group,
      (ump_note_on << 4) | channel, note_number, 0));
  p.clearUMP();

  // An entire 32-bit MIDI 1 note-on message.
  p.processUMP(pack(
      (static_cast<std::uint8_t>(midi2::ump_message_type::m1cvm) << 4) | group,
      (ump_note_on << 4) | channel, note_number, velocity));
}

TEST(UMPProcessor, FunctionBlockInfo) {
  constexpr auto active = std::uint32_t{0b1};  // 1 bit
  constexpr auto first_group = std::uint8_t{0};
  constexpr auto function_block_num = std::uint32_t{0b0101010};  // 7 bits
  constexpr auto groups_spanned = std::uint8_t{1};
  constexpr auto midi1 = std::uint32_t{0x00};  // 2 bits
  constexpr auto num_sysex8_streams = std::uint8_t{0x17U};
  constexpr auto ui_hint = std::uint32_t{0b10};  // 2 bits
  constexpr auto version = std::uint8_t{0x01};

  midi2::function_block_info fbi;
  fbi.fbIdx = function_block_num;
  fbi.active = active;
  fbi.direction = midi2::function_block_info::fbdirection::output;
  fbi.firstGroup = first_group;
  fbi.groupLength = groups_spanned;
  fbi.midiCIVersion = version;
  fbi.isMIDI1 = false;
  fbi.maxS8Streams = num_sysex8_streams;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, functionBlockInfo(fbi)).Times(1);

  midi2::types::function_block_info_w1 word1{};
  word1.mt = static_cast<std::uint32_t>(midi2::ump_message_type::midi_endpoint);
  word1.format = 0U;
  word1.status =
      static_cast<std::uint32_t>(midi2::MIDICI_PROTOCOL_NEGOTIATION_REPLY);
  word1.a = active;
  word1.block_number = function_block_num;
  word1.reserv = 0U;
  word1.ui_hint = ui_hint;
  word1.m1 = midi1;
  word1.dir = static_cast<std::uint32_t>(
      midi2::function_block_info::fbdirection::output);

  midi2::types::function_block_info_w2 word2{};
  word2.first_group = first_group;
  word2.groups_spanned = groups_spanned;
  word2.message_version = version;
  word2.num_sysex8_streams = num_sysex8_streams;

  midi2::umpProcessor p{std::ref(callbacks)};
  p.processUMP(std::bit_cast<std::uint32_t>(word1));
  p.processUMP(std::bit_cast<std::uint32_t>(word2));
  p.processUMP(0);
  p.processUMP(0);
}

TEST(UMPProcessor, FunctionBlockName) {
  constexpr auto function_block_num = std::uint8_t{0b0101010};  // 8 bits
  constexpr auto group = std::uint8_t{0xFF};
  constexpr auto stream_id = 0U;
  constexpr auto format = 0U;

  std::array<std::uint8_t, 4> const payload{'n', 'a', 'm', 'e'};

  MockCallbacks callbacks;
  EXPECT_CALL(
      callbacks,
      functionBlockName(
          UMPDataMatches(
              midi2::umpCommon{
                  group, midi2::ump_message_type::midi_endpoint,
                  static_cast<std::uint8_t>(midi2::MIDICI_PROTOCOL_SET)},
              stream_id, format, std::begin(payload), std::end(payload)),
          function_block_num));

  midi2::types::function_block_name_w1 word1{};
  word1.mt = static_cast<std::uint32_t>(midi2::ump_message_type::midi_endpoint);
  word1.format = 0U;  // "complete UMP"
  word1.status = static_cast<std::uint32_t>(midi2::MIDICI_PROTOCOL_SET);
  word1.block_number = function_block_num;
  word1.name = 'n';

  midi2::umpProcessor p{std::ref(callbacks)};
  p.processUMP(std::bit_cast<std::uint32_t>(word1));
  p.processUMP(pack('a', 'm', 'e', 0));
  p.processUMP(0);
  p.processUMP(0);
}

TEST(UMPProcessor, SetChordName) {
  constexpr auto group = std::uint8_t{0xF};
  constexpr auto addrs = std::uint8_t{0x3};
  constexpr auto channel = std::uint8_t{3};

  constexpr auto chord_tonic = midi2::chord::note::E;
  constexpr auto chord_type = midi2::chord::chord_type::augmented;
  constexpr auto bass_note = midi2::chord::note::unknown;
  constexpr auto bass_chord_type = midi2::chord::chord_type::diminished;

  midi2::chord chord{};
  chord.chShrpFlt = midi2::chord::sharps_flats::sharp;
  chord.chTonic = chord_tonic;
  chord.chType = chord_type;
  chord.chAlt1 = midi2::chord::alteration{1, 5};
  chord.chAlt2 = midi2::chord::alteration{2, 6};
  chord.chAlt3 = midi2::chord::alteration{3, 7};
  chord.chAlt4 = midi2::chord::alteration{4, 8};
  chord.baShrpFlt = midi2::chord::sharps_flats::double_flat;  // Double Flat
  chord.baTonic = bass_note;
  chord.baType = bass_chord_type;
  chord.baAlt1 = midi2::chord::alteration{1, 3};
  chord.baAlt2 = midi2::chord::alteration{2, 4};

  midi2::types::set_chord_name_w1 word1{};
  word1.mt = static_cast<std::uint8_t>(midi2::ump_message_type::flex_data);
  word1.group = group;
  word1.format = 0x0;
  word1.addrs = addrs;
  word1.channel = channel;
  word1.status_bank = 0x00;
  word1.status = 0x06;

  midi2::types::set_chord_name_w2 word2{};
  word2.tonic_sharps_flats = 0x1;
  word2.chord_tonic = static_cast<std::uint8_t>(chord_tonic);
  word2.chord_type = static_cast<std::uint8_t>(chord_type);
  word2.alter_1_type = 1;
  word2.alter_1_degree = 5;
  word2.alter_2_type = 2;
  word2.alter_2_degree = 6;

  midi2::types::set_chord_name_w3 word3{};
  word3.alter_3_type = 3;
  word3.alter_3_degree = 7;
  word3.alter_4_type = 4;
  word3.alter_4_degree = 8;
  word3.reserved = 0x0000;

  midi2::types::set_chord_name_w4 word4{};
  word4.bass_sharps_flats = 0xE;
  word4.bass_note = static_cast<std::uint8_t>(bass_note);
  word4.bass_chord_type = static_cast<std::uint8_t>(bass_chord_type);
  word4.alter_1_type = 1;
  word4.alter_1_degree = 3;
  word4.alter_2_type = 2;
  word4.alter_2_degree = 4;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, flex_chord(group, addrs, channel, chord)).Times(1);

  midi2::umpProcessor p{std::ref(callbacks)};
  p.processUMP(std::bit_cast<std::uint32_t>(word1));
  p.processUMP(std::bit_cast<std::uint32_t>(word2));
  p.processUMP(std::bit_cast<std::uint32_t>(word3));
  p.processUMP(std::bit_cast<std::uint32_t>(word4));
}

void UMPProcessorNeverCrashes(std::vector<std::uint32_t> const& in) {
  midi2::umpProcessor p;
  std::for_each(std::begin(in), std::end(in),
                std::bind_front(&decltype(p)::processUMP, &p));
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, UMPProcessorNeverCrashes);
#endif
// NOLINTNEXTLINE
TEST(UMPProcessor, Empty) {
  UMPProcessorNeverCrashes({});
}

// See M2-104-UM (UMP Format & MIDI 2.0 Protocol v.1.1.2 2023-10-27)
//    Table 4 Message Type (MT) Allocation
template <midi2::ump_message_type> struct message_size {};
template <>
struct message_size<midi2::ump_message_type::utility>
    : std::integral_constant<unsigned, 1> {};
template <>
struct message_size<midi2::ump_message_type::system>
    : std::integral_constant<unsigned, 1> {};
template <>
struct message_size<midi2::ump_message_type::m1cvm>
    : std::integral_constant<unsigned, 1> {};
template <>
struct message_size<midi2::ump_message_type::sysex7>
    : std::integral_constant<unsigned, 2> {};
template <>
struct message_size<midi2::ump_message_type::m2cvm>
    : std::integral_constant<unsigned, 2> {};
template <>
struct message_size<midi2::ump_message_type::data>
    : std::integral_constant<unsigned, 4> {};
template <>
struct message_size<midi2::ump_message_type::reserved32_06>
    : std::integral_constant<unsigned, 1> {};
template <>
struct message_size<midi2::ump_message_type::reserved32_07>
    : std::integral_constant<unsigned, 1> {};
template <>
struct message_size<midi2::ump_message_type::reserved64_08>
    : std::integral_constant<unsigned, 2> {};
template <>
struct message_size<midi2::ump_message_type::reserved64_09>
    : std::integral_constant<unsigned, 2> {};
template <>
struct message_size<midi2::ump_message_type::reserved64_0A>
    : std::integral_constant<unsigned, 2> {};
template <>
struct message_size<midi2::ump_message_type::reserved96_0B>
    : std::integral_constant<unsigned, 3> {};
template <>
struct message_size<midi2::ump_message_type::reserved96_0C>
    : std::integral_constant<unsigned, 3> {};
template <>
struct message_size<midi2::ump_message_type::flex_data>
    : std::integral_constant<unsigned, 4> {};
template <>
struct message_size<midi2::ump_message_type::reserved128_0E>
    : std::integral_constant<unsigned, 4> {};
template <>
struct message_size<midi2::ump_message_type::midi_endpoint>
    : std::integral_constant<unsigned, 4> {};

template <midi2::ump_message_type MessageType>
void process_message(std::span<std::uint32_t> message) {
  if (message.size() == message_size<MessageType>::value) {
    message[0] = (message[0] & 0x00FFFFFF) |
                 (static_cast<std::uint32_t>(MessageType) << 24);
    midi2::umpProcessor p;
    std::for_each(std::begin(message), std::end(message),
                  std::bind_front(&decltype(p)::processUMP, &p));
  }
}

void utility(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::utility>(
      {std::begin(message), std::end(message)});
}
void system(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::system>(
      {std::begin(message), std::end(message)});
}
void m1cvm(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::m1cvm>(
      {std::begin(message), std::end(message)});
}
void sysex7(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::sysex7>(
      {std::begin(message), std::end(message)});
}
void m2cvm(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::m2cvm>(
      {std::begin(message), std::end(message)});
}
void data(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::data>(
      {std::begin(message), std::end(message)});
}
void flex_data(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::flex_data>(
      {std::begin(message), std::end(message)});
}
void midi_endpoint(std::vector<std::uint32_t> message) {
  process_message<midi2::ump_message_type::midi_endpoint>(
      {std::begin(message), std::end(message)});
}
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, utility);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, system);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, m1cvm);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, sysex7);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, m2cvm);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, data);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, flex_data);
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, midi_endpoint);
#endif

// NOLINTNEXTLINE
TEST(UMPProcessor, UtilityMessage) {
  utility({});
}
// NOLINTNEXTLINE
TEST(UMPProcessor, SystemMessage) {
  system({});
}
// NOLINTNEXTLINE
TEST(UMPProcessor, M1CVMMessage) {
  m1cvm({});
}
// NOLINTNEXTLINE
TEST(UMPProcessor, Sysex7Message) {
  sysex7({});
}
// NOLINTNEXTLINE
TEST(UMPProcessor, M2CVMMessage) {
  m2cvm({});
}
// NOLINTNEXTLINE
TEST(UMPProcessor, DataaMessage) {
  data({});
}
// NOLINTNEXTLINE
TEST(UMPProcessor, FlexDataMessage) {
  flex_data({});
}
// NOLINTNEXTLINE
TEST(UMPProcessor, MidiEndpointaMessage) {
  midi_endpoint({});
}

}  // end anonymous namespace
