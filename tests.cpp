//
// Created by andrew on 3/05/24.
//

#include <cstdio>

#include "midi2/bytestreamToUMP.h"
#include "midi2/umpMessageCreate.h"
#include "midi2/umpToBytestream.h"
#include "midi2/umpToMIDI1Protocol.h"

namespace {

midi2::bytestreamToUMP BS2UMP;
midi2::umpToBytestream UMP2BS;
midi2::umpToMIDI1Protocol UMP2M1;

auto testPassed = 0U;
auto testFailed = 0U;

void passFail(std::size_t v1, std::size_t v2) {
  if (v1 == v2) {
    printf(".");
    testPassed++;
  } else {
    printf(" fail %#08zx != %#08zx ", v1, v2);
    testFailed++;
  }
}

void testRun_bsToUmp(const char* heading, uint8_t const* bytes,
                     unsigned bytelength, uint32_t const* testCheck,
                     unsigned outlength) {
  std::fputs(heading, stdout);

  auto testCounter = 0U;

  for (auto i = 0U; i < bytelength; i++) {
    BS2UMP.bytestreamParse(bytes[i]);
    while (BS2UMP.availableUMP()) {
      uint32_t const ump = BS2UMP.readUMP();
      // ump contains a ump 32 bit value. UMP messages that have 64bit will
      // produce 2 UMP words
      if (testCounter < outlength) {
        passFail(ump, testCheck[testCounter++]);
      }
    }
  }
  std::printf(" length :");
  passFail(outlength, testCounter);
  std::printf("\n");
}

void testRun_umpToBs(const char* heading, uint8_t const* testBytes,
                     std::size_t const bytelength, uint32_t const* umps,
                     int umplength) {
  std::fputs(heading, stdout);

  auto testCounter = 0U;

  for (int i = 0; i < umplength; i++) {
    UMP2BS.UMPStreamParse(umps[i]);
    while (UMP2BS.availableBS()) {
      uint8_t byte = UMP2BS.readBS();
      // ump contains a ump 32 bit value. UMP messages that have 64bit will
      // produce 2 UMP words
      if (testCounter < bytelength) {
        passFail(byte, testBytes[testCounter++]);
      }
    }
  }
  std::printf(" length :");
  passFail(bytelength, testCounter);
  std::printf("\n");
}

void testRun_umpToM1(const char* heading, uint32_t const* in, unsigned inlength,
                     uint32_t const* out, unsigned outlength) {
  std::fputs(heading, stdout);

  auto testCounter = 0U;

  for (auto i = 0U; i < inlength; i++) {
    UMP2M1.UMPStreamParse(in[i]);
    while (UMP2M1.availableUMP()) {
      uint32_t newUmp = UMP2M1.readUMP();
      // ump contains a ump 32 bit value. UMP messages that have 64bit will
      // produce 2 UMP words
      if (testCounter < outlength) {
        passFail(newUmp, out[testCounter++]);
      }
    }
  }
  std::printf(" length :");
  passFail(outlength, testCounter);
  std::printf("\n");
}

void testRun_umpToump(const char* heading, uint32_t * in, int inlength, uint32_t * out)
{
  std::fputs(heading, stdout);
  for (int i = 0; i < inlength; i++) {
    passFail(in[i], out[i]);
  }
  std::printf("\n");
}

template <typename T, std::size_t Size>
constexpr std::size_t array_elements(T (&)[Size]) noexcept {
  return Size;
}

}  // end anonymous namespace

int main(){
  std::printf("Starting Tests...\n");

  //******** ByteSteam to UMP ***************
  std::printf("ByteSteam to UMP \n");
  uint8_t const bytes1[] = {0x81, 0x60, 0x50, 0x70, 0x70};
  uint32_t const tests1[] = {0x20816050, 0x20817070};
  testRun_bsToUmp(" Test 1 Note On w/running status: ", bytes1, 5, tests1, 2);

  uint8_t const bytes2[] = {0xF8};
  uint32_t const tests2[] = {0x10f80000};
  testRun_bsToUmp(" Test 2 System Message 1 byte: ", bytes2, 1, tests2, 1);
  uint8_t const bytes3[] = {0xC6, 0x40};
  uint32_t tests3[] = {0x20c64000};
  testRun_bsToUmp(" Test 3 PC 2 bytes : ", bytes3, 2, tests3, 1);

  uint8_t bytes4[] = {0xF0, 0x7E, 0x7F, 0x0D, 0x70, 0x02, 0x4B, 0x60,
                      0x7A, 0x73, 0x7F, 0x7F, 0x7F, 0x7F, 0x7D, 0x00,
                      0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03,
                      0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xF7};
  uint32_t tests4[] = {0x30167e7f, 0x0d70024b, 0x3026607a, 0x737f7f7f,
                       0x30267f7d, 0x00000000, 0x30260100, 0x00000300,
                       0x30360000, 0x10000000};
  testRun_bsToUmp(" Test 4 Sysex : ", bytes4, 32, tests4, 10);

  printf(" Switching to Mt4 \n");
  BS2UMP.set_output_midi2(true);
  uint32_t tests1a[] = {0x40816000, 0xA0820000, 0x40817000, 0xe1860000};
  testRun_bsToUmp(" Test 5 MT4 Note On w/running status: ", bytes1, 5, tests1a,
                  4);

  uint32_t const tests3a[] = {0x40c60000, 0x40000000};
  BS2UMP.set_output_midi2(true);
  testRun_bsToUmp(" Test 6 MT 4 PC 2 bytes : ", bytes3, 2, tests3a, 2);

  uint8_t bytes3b[] = {0xB6, 0x00, 0x01, 0x20, 0x0A, 0xC6, 0x41};
  uint32_t tests3b[] = {0x40c60001, 0x4100010A};
  testRun_bsToUmp(" Test 7 MT 4 PC 2 bytes with Bank MSB LSB : ", bytes3b, 7,
                  tests3b, 2);

  uint8_t bytes4b[] = {0xB6, 101, 0x00, 100, 0x06, 0x06, 0x08};
  uint32_t tests4b[] = {0x40260006, 0x10000000};
  testRun_bsToUmp(" Test 7 MT 4 RPN : ", bytes4b, 7, tests4b, 2);

  //******** UMP ByteSteam  ***************
  printf("UMP to ByteSteam \n");
  uint8_t bytes5[] = {0x81, 0x60, 0x50, 0x81, 0x70, 0x70};
  uint32_t tests5[] = {0x20816050, 0x20817070};
  testRun_umpToBs(" Test 5 Note On: ", bytes5, array_elements(bytes5), tests5,
                  array_elements(tests5));
  testRun_umpToBs(" Test 6 System Message 1 byte: ", bytes2,
                  array_elements(bytes2), tests2, array_elements(tests2));
  testRun_umpToBs(" Test 7 PC 2 bytes : ", bytes3, array_elements(bytes3),
                  tests3, array_elements(tests3));
  testRun_umpToBs(" Test 8 Sysex : ", bytes4, array_elements(bytes4), tests4,
                  array_elements(tests4));

  //***** UMP2M1 *************
  printf("UMP to MIDI 1 Protocol \n");
  uint32_t in[] = {0x20816050, 0x20817070};
  testRun_umpToM1(" Test MIDI 1 : ", in, 2, in, 2);

  testRun_umpToM1(" Test SysEx : ", tests4, 10, tests4, 10);
  testRun_umpToM1(" Test System Msg : ", tests2, 1, tests2, 1);

  uint32_t in2[] = {0x40904000, 0xc1040000};
  uint32_t out2[] = {0x20904060};
  testRun_umpToM1(" Test MT4 : ", in2, 2, out2, 1);

  //***** UMP Meesage Create *************
  printf("UMP Message Create \n");
  uint32_t inUmp1[] = {midi2::UMPMessage::mt0NOOP()};
  uint32_t outUmp1[] = {0x00000000};
  testRun_umpToump(" UMP NOOP : ", inUmp1, 1, outUmp1);

  uint32_t inUmp2[] = {midi2::UMPMessage::mt1TimingClock(8)};
  uint32_t outUmp2[] = {0x18f80000};
  testRun_umpToump(" UMP Timing Clock : ", inUmp2, 1, outUmp2);

  ///****************************
  printf("Tests Passed: %d    Failed : %d\n", testPassed, testFailed);
}
