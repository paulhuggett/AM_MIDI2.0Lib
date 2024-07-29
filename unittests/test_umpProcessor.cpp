#include <algorithm>
#include <functional>
#include <numeric>

#include "umpProcessor.h"

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

std::ostream& operator<<(std::ostream& os, umpCommon const& common);
std::ostream& operator<<(std::ostream& os, umpCommon const& common) {
  return os << "{ group=" << static_cast<unsigned>(common.group)
            << ", messageType=" << static_cast<unsigned>(common.messageType)
            << ", status=" << static_cast<unsigned>(common.status) << " }";
};

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
  os << ']';
  return os;
}

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
  void utility_message(umpGeneric const& message) {
    original_.utility_message(message);
  }
  void channel_voice_message(umpCVM const& message) {
    original_.channel_voice_message(message);
  }
  void system_message(umpGeneric const& message) {
    original_.system_message(message);
  }
  void send_out_sysex(umpData const& message) {
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
                  chord const& chord) {
    original_.flex_chord(group, addrs, channel, chord);
  }
  void flex_performance(umpData const& mess, std::uint8_t addrs,
                        std::uint8_t channel) {
    original_.flex_performance(mess, addrs, channel);
  }
  void flex_lyric(umpData const& mess, std::uint8_t addrs,
                  std::uint8_t channel) {
    original_.flex_lyric(mess, addrs, channel);
  }

  //---------- UMP Stream
  void midiEndpoint(std::uint8_t majVer, std::uint8_t minVer,
                    std::uint8_t filter) {
    original_.midiEndpoint(majVer, minVer, filter);
  }
  void midiEndpointName(umpData const& mess) {
    original_.midiEndpointName(mess);
  }
  void midiEndpointProdId(umpData const& mess) {
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
  void functionBlockInfo(std::uint8_t fbIdx, bool active,
                         std::uint8_t direction, bool sender, bool recv,
                         std::uint8_t firstGroup, std::uint8_t groupLength,
                         std::uint8_t midiCIVersion, std::uint8_t isMIDI1,
                         std::uint8_t maxS8Streams) {
    original_.functionBlockInfo(fbIdx, active, direction, sender, recv,
                                firstGroup, groupLength, midiCIVersion, isMIDI1,
                                maxS8Streams);
  }
  void functionBlockName(umpData mess, std::uint8_t fbIdx) {
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

class MockCallbacks : public callbacks_base {
public:
  MOCK_METHOD(void, utility_message, (umpGeneric const&), (override));
  MOCK_METHOD(void, channel_voice_message, (umpCVM const&), (override));
  MOCK_METHOD(void, system_message, (umpGeneric const&), (override));
  MOCK_METHOD(void, send_out_sysex, (umpData const&), (override));
};

}  // end anonymous namespace

template class umpProcessor<callbacks_proxy<MockCallbacks>>;

namespace {

constexpr std::uint32_t ump_cvm(status s) {
  static_assert(std::is_same_v<std::underlying_type_t<status>, std::uint8_t>,
                "status type must be a std::uint8_t");
  assert((s & 0x0F) == 0 &&
         "Bottom 4 bits of a channel voice message status enum  must be 0");
  return std::uint32_t{s} >> 4;
}

constexpr auto ump_note_on = ump_cvm(status::note_on);

constexpr std::uint32_t pack(std::uint8_t const b0, std::uint8_t const b1,
                             std::uint8_t const b2, std::uint8_t const b3) {
  return (std::uint32_t{b0} << 24) | (std::uint32_t{b1} << 16) |
         (std::uint32_t{b2} << 8) | std::uint32_t{b3};
}

TEST(UMPProcessor, Midi1NoteOn) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint16_t{0x43}; // 7 bits
  constexpr auto group = std::uint8_t{0};

  umpCVM message;
  message.common.group = group;
  message.common.messageType = ump_message_type::m1cvm;
  message.common.status = status::note_on;
  message.channel = channel;
  message.note = note_number;
  message.value = M2Utils::scaleUp(velocity, 7, 16);
  message.index = 0;
  message.bank = 0;
  message.flag1 = false;
  message.flag2 = false;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, channel_voice_message(message)).Times(1);

  umpProcessor p{callbacks_proxy{callbacks}};
  p.processUMP(
      pack((static_cast<std::uint8_t>(ump_message_type::m1cvm) << 4) | group,
           (ump_note_on << 4) | channel, note_number, velocity));
}

TEST(UMPProcessor, Midi2NoteOn) {
  constexpr auto channel = std::uint8_t{3};
  constexpr auto note_number = std::uint8_t{60};
  constexpr auto velocity = std::uint16_t{0x432};
  constexpr auto group = std::uint8_t{0};

  umpCVM message;
  message.common.group = group;
  message.common.messageType = ump_message_type::m2cvm;
  message.common.status = status::note_on;
  message.channel = channel;
  message.note = note_number;
  message.value = velocity;
  message.index = 0;
  message.bank = 0;
  message.flag1 = false;
  message.flag2 = false;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, channel_voice_message(message)).Times(1);

  umpProcessor p{callbacks_proxy{callbacks}};
  p.processUMP(std::uint32_t{
      (static_cast<std::uint32_t>(ump_message_type::m2cvm) << 28) |
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
template <typename Iterator>
auto UMPDataMatches(std::uint8_t group, std::uint8_t stream_id,
                    std::uint8_t form, Iterator first, Iterator last) {
  return AllOf(Field("common", &umpData::common,
                     Eq(umpCommon{group, ump_message_type::data, 0})),
               Field("streamId", &umpData::streamId, Eq(stream_id)),
               Field("form", &umpData::form, Eq(form)),
               Field("data", &umpData::data, ElementsAreArray(first, last)));
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
    EXPECT_CALL(callbacks, send_out_sysex(UMPDataMatches(
                               group, stream_id, start_form,
                               std::begin(payload), split_point)));
    EXPECT_CALL(callbacks,
                send_out_sysex(UMPDataMatches(group, stream_id, end_form,
                                              split_point, std::end(payload))));
  }
  umpProcessor p{callbacks_proxy{callbacks}};
  // Send 13 bytes
  p.processUMP(
      pack((static_cast<std::uint8_t>(ump_message_type::data) << 4) | group,
           (start_form << 4) | 13U, stream_id, payload[0]));
  p.processUMP(pack(payload[1], payload[2], payload[3], payload[4]));
  p.processUMP(pack(payload[5], payload[6], payload[7], payload[8]));
  p.processUMP(pack(payload[9], payload[10], payload[11], payload[12]));
  // Send the final 3 bytes.
  p.processUMP(
      pack((static_cast<std::uint8_t>(ump_message_type::data) << 4) | group,
           (end_form << 4) | 3U, stream_id, payload[13]));
  p.processUMP(pack(payload[14], payload[15], 0, 0));
  p.processUMP(0);
  p.processUMP(0);

#if 0
  // Send 3 bytes.
  p.processUMP(std::uint32_t{
      (static_cast<std::uint32_t>(ump_message_type::data) << 28) |
      (std::uint32_t{group} << 24) | (std::uint32_t{0b0011} << 20) |
      (std::uint32_t{3} << 16) | (std::uint32_t{stream_id} << 8)} | payload[13]);
  p.processUMP((payload[14] << 24) | (payload[15] << 16));
  p.processUMP(0);
  p.processUMP(0);
#endif
}

void UMPProcessorNeverCrashes(std::vector<std::uint32_t> const& in) {
  using namespace std::placeholders;
  umpProcessor p;
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
  message[0] = (message[0] & 0x00FFFFFF) |
               (static_cast<std::uint32_t>(ump_message_type::data) << 28);
  using namespace std::placeholders;
  umpProcessor p;
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

  umpCVM message;
  message.common.group = group;
  message.common.messageType = ump_message_type::m1cvm;
  message.common.status = status::note_on;
  message.channel = channel;
  message.note = note_number;
  message.value = M2Utils::scaleUp(velocity, 7, 16);
  message.index = 0;
  message.bank = 0;
  message.flag1 = false;
  message.flag2 = false;

  MockCallbacks callbacks;
  EXPECT_CALL(callbacks, channel_voice_message(message)).Times(1);

  umpProcessor p{callbacks_proxy{callbacks}};
  // The first half of a 64-bit MIDI 2 note-on message.
  p.processUMP(
      pack((static_cast<std::uint8_t>(ump_message_type::m2cvm) << 4) | group,
           (ump_note_on << 4) | channel, note_number, 0));
  p.clearUMP();

  // An entire 32-bit MIDI 1 note-on message.
  p.processUMP(
      pack((static_cast<std::uint8_t>(ump_message_type::m1cvm) << 4) | group,
           (ump_note_on << 4) | channel, note_number, velocity));
}

}  // end anonymous namespace
