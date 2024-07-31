#include <algorithm>
#include <bit>
#include <functional>
#include <numeric>

#include "midi2/bitfield.h"
#include "midi2/umpProcessor.h"
#include "midi2/ump_types.h"

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

}  // end namespace midi2

namespace {

// We want to use mocked instances of a callback struct, but the umpProcessor
// takes the callbacks by value and mocked structs can't be copied. This "proxy"
// object *is* copyable and seves to simply forward any calls to the original
// mock instance.
template <typename T> class callbacks_proxy {
public:
  explicit callbacks_proxy(T& original) : original_(original) {}
  callbacks_proxy(callbacks_proxy const&) = default;
  callbacks_proxy(callbacks_proxy&&) noexcept = default;

  ~callbacks_proxy() noexcept = default;

  callbacks_proxy& operator=(callbacks_proxy const&) = delete;
  callbacks_proxy& operator=(callbacks_proxy&&) noexcept = delete;

  //-----------------------Handlers ---------------------------
  void utility_message(midi2::umpGeneric const& message) {
    original_.utility_message(message);
  }
  void channel_voice_message(midi2::umpCVM const& message) {
    original_.channel_voice_message(message);
  }
  void system_message(midi2::umpGeneric const& message) {
    original_.system_message(message);
  }
  void send_out_sysex(midi2::umpData const& message) {
    original_.send_out_sysex(message);
  }

  //---------- Flex Data
  void flex_tempo(std::uint8_t group, std::uint32_t num10nsPQN) {
    original_.flex_tempo(group, num10nsPQN);
  }
  void flex_time_sig(std::uint8_t group, std::uint8_t numerator,
                     std::uint8_t denominator, std::uint8_t num32Notes) {
    original_.flex_time_sig(group, numerator, denominator, num32Notes);
  }
  void flex_metronome(std::uint8_t group, std::uint8_t numClkpPriCli,
                      std::uint8_t bAccP1, std::uint8_t bAccP2,
                      std::uint8_t bAccP3, std::uint8_t numSubDivCli1,
                      std::uint8_t numSubDivCli2) {
    original_.flex_metronome(group, numClkpPriCli, bAccP1, bAccP2, bAccP3,
                             numSubDivCli1, numSubDivCli2);
  }
  void flex_key_sig(std::uint8_t group, std::uint8_t addrs,
                    std::uint8_t channel, std::uint8_t sharpFlats,
                    std::uint8_t tonic) {
    original_.flex_key_sig(group, addrs, channel, sharpFlats, tonic);
  }
  void flex_chord(std::uint8_t group, std::uint8_t addrs, std::uint8_t channel,
                  midi2::chord const& chord) {
    original_.flex_chord(group, addrs, channel, chord);
  }
  void flex_performance(midi2::umpData const& mess, std::uint8_t addrs,
                        std::uint8_t channel) {
    original_.flex_performance(mess, addrs, channel);
  }
  void flex_lyric(midi2::umpData const& mess, std::uint8_t addrs,
                  std::uint8_t channel) {
    original_.flex_lyric(mess, addrs, channel);
  }

  //---------- UMP Stream
  void midiEndpoint(std::uint8_t majVer, std::uint8_t minVer,
                    std::uint8_t filter) {
    original_.midiEndpoint(majVer, minVer, filter);
  }
  void midiEndpointName(midi2::umpData const& mess) {
    original_.midiEndpointName(mess);
  }
  void midiEndpointProdId(midi2::umpData const& mess) {
    original_.midiEndpointProdId(mess);
  }
  void midiEndpointJRProtocolReq(std::uint8_t protocol, bool jrrx, bool jrtx) {
    original_.midiEndpointJRProtocolReq(protocol, jrrx, jrtx);
  }
  void midiEndpointInfo(std::uint8_t majVer, std::uint8_t minVer,
                        std::uint8_t numOfFuncBlocks, bool m2, bool m1,
                        bool rxjr, bool txjr) {
    original_.midiEndpointInfo(majVer, minVer, numOfFuncBlocks, m2, m1, rxjr,
                               txjr);
  }
  void midiEndpointDeviceInfo(std::array<std::uint8_t, 3> const& manuId,
                              std::array<std::uint8_t, 2> const& familyId,
                              std::array<std::uint8_t, 2> const& modelId,
                              std::array<std::uint8_t, 4> const& version) {
    original_.midiEndpointDeviceInfo(manuId, familyId, modelId, version);
  }
  void midiEndpointJRProtocolNotify(std::uint8_t protocol, bool jrrx,
                                    bool jrtx) {
    original_.midiEndpointJRProtocolNotify(protocol, jrrx, jrtx);
  }

  void functionBlock(std::uint8_t fbIdx, std::uint8_t filter) {
    original_.functionBlock(fbIdx, filter);
  }
  void functionBlockInfo(midi2::function_block_info const& fbi) {
    original_.functionBlockInfo(fbi);
  }
  void functionBlockName(midi2::umpData const& mess, std::uint8_t fbIdx) {
    original_.functionBlockName(mess, fbIdx);
  }

  void startOfSeq() { original_.startOfSeq(); }
  void endOfFile() { original_.endOfFile(); }

  void unknownUMPMessage(std::span<std::uint32_t> sp) {
    original_.unknownUMPMessage(sp);
  }

private:
  T& original_;
};

template <typename T> callbacks_proxy(T&) -> callbacks_proxy<T>;

class MockCallbacks : public midi2::callbacks_base {
public:
  MOCK_METHOD(void, utility_message, (midi2::umpGeneric const&), (override));
  MOCK_METHOD(void, channel_voice_message, (midi2::umpCVM const&), (override));
  MOCK_METHOD(void, system_message, (midi2::umpGeneric const&), (override));
  MOCK_METHOD(void, send_out_sysex, (midi2::umpData const&), (override));

  MOCK_METHOD(void, functionBlockInfo, (midi2::function_block_info const&),
              (override));
  MOCK_METHOD(void, functionBlockName, (midi2::umpData const&, std::uint8_t),
              (override));

  MOCK_METHOD(void, unknownUMPMessage, (std::span<std::uint32_t>), (override));
};

}  // end anonymous namespace

template class midi2::umpProcessor<callbacks_proxy<MockCallbacks>>;

namespace {

TEST(UMPProcessor, Noop) {
  midi2::umpGeneric message;
  message.common.group = 255;
  message.common.messageType = midi2::ump_message_type::utility;
  message.common.status = 0;
  message.value = 0;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, utility_message(message)).Times(1);

  midi2::umpProcessor p{callbacks_proxy{callbacks}};
  midi2::types::noop w1;
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

constexpr std::uint32_t pack(std::uint8_t const b0, std::uint8_t const b1,
                             std::uint8_t const b2, std::uint8_t const b3) {
  return (std::uint32_t{b0} << 24) | (std::uint32_t{b1} << 16) |
         (std::uint32_t{b2} << 8) | std::uint32_t{b3};
}

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

  midi2::umpProcessor p{callbacks_proxy{callbacks}};
  midi2::types::m1cvm_w1 w1;
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

  midi2::umpProcessor p{callbacks_proxy{callbacks}};
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
  midi2::umpProcessor p{callbacks_proxy{callbacks}};
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

#if 0
  // Send 3 bytes.
  p.processUMP(std::uint32_t{
      (static_cast<std::uint32_t>(midi2::ump_message_type::data) << 28) |
      (std::uint32_t{group} << 24) | (std::uint32_t{0b0011} << 20) |
      (std::uint32_t{3} << 16) | (std::uint32_t{stream_id} << 8)} | payload[13]);
  p.processUMP((payload[14] << 24) | (payload[15] << 16));
  p.processUMP(0);
  p.processUMP(0);
#endif
}

void UMPProcessorNeverCrashes(std::vector<std::uint32_t> const& in) {
  using namespace std::placeholders;
  midi2::umpProcessor p;
  std::for_each(std::begin(in), std::end(in),
                std::bind(&decltype(p)::processUMP, &p, _1));
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, UMPProcessorNeverCrashes);
#endif
TEST(UMPProcessor, Empty) {
  UMPProcessorNeverCrashes({});
}

void FourByteDataMessage(std::array<std::uint32_t, 4> message) {
  // Set the message type to "data".
  message[0] =
      (message[0] & 0x00FFFFFF) |
      (static_cast<std::uint32_t>(midi2::ump_message_type::data) << 28);
  using namespace std::placeholders;
  midi2::umpProcessor p;
  std::for_each(std::begin(message), std::end(message),
                std::bind(&decltype(p)::processUMP, &p, _1));
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(UMPProcessor, FourByteDataMessage);
#endif
TEST(UMPProcessor, FourByteDataMessageAllZero) {
  FourByteDataMessage({});
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

  midi2::umpProcessor p{callbacks_proxy{callbacks}};
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

  midi2::types::function_block_info_w1 word1;
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

  midi2::types::function_block_info_w2 word2;
  word2.first_group = first_group;
  word2.groups_spanned = groups_spanned;
  word2.message_version = version;
  word2.num_sysex8_streams = num_sysex8_streams;

  midi2::umpProcessor p{callbacks_proxy{callbacks}};
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

  midi2::types::function_block_name_w1 word1;
  word1.mt = static_cast<std::uint32_t>(midi2::ump_message_type::midi_endpoint);
  word1.format = 0U;  // "complete UMP"
  word1.status = static_cast<std::uint32_t>(midi2::MIDICI_PROTOCOL_SET);
  word1.block_number = function_block_num;
  word1.name = 'n';

  midi2::umpProcessor p{callbacks_proxy{callbacks}};
  p.processUMP(std::bit_cast<std::uint32_t>(word1));
  p.processUMP(pack('a', 'm', 'e', 0));
  p.processUMP(0);
  p.processUMP(0);
}

}  // end anonymous namespace
