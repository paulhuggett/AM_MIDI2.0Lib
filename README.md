# AM MIDI 2.0 Lib

A MIDI 2.0 Library

This is a general purpose Library which provides the building blocks, processing and translations needed for most MIDI 2.0 devices and applications. This library aims to work on everything from embedded devices through to large scale applications with minimal code and data footprint.

This code is based on Andrew Mee’s library at <https://github.com/midi2-dev/AM_MIDI2.0Lib>. It has been heavily modified with a number of goals:

- Using C++20 features
- Limiting use of magic numbers and bitwise operators to define data layout and instead using the C++ type system wherever possible
- UMP conversion classes are now built on the library's `ump_dispatcher` class rather than duplicating code to extract UMP messages
- Using templates to pass callables to enable cross-callback optimization
- Significantly increasing the depth of testing

[![CI Build & Test](https://github.com/paulhuggett/AM_MIDI2.0Lib/actions/workflows/ci.yaml/badge.svg)](https://github.com/paulhuggett/AM_MIDI2.0Lib/actions/workflows/ci.yaml)
[![codecov](https://codecov.io/gh/paulhuggett/AM_MIDI2.0Lib/graph/badge.svg?token=8q2aEvPTyv)](https://codecov.io/gh/paulhuggett/AM_MIDI2.0Lib)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=paulhuggett_AM_MIDI2.0Lib&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=paulhuggett_AM_MIDI2.0Lib)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/425f68679a124a1cbb0efa50342d8e8a)](https://app.codacy.com/gh/paulhuggett/AM_MIDI2.0Lib/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Fuzz Test](https://github.com/paulhuggett/AM_MIDI2.0Lib/actions/workflows/fuzztest.yaml/badge.svg)](https://github.com/paulhuggett/AM_MIDI2.0Lib/actions/workflows/fuzztest.yaml)

## What does this do?

Please read the MIDI 2.0 specification on https://midi.org/specifications to understand the following.

This library can:

- Convert a MIDI 1.0 byte-stream to UMP and back
- Process and construct UMP streams
- Process and construct MIDI CI messages

This library is designed to use a small footprint. It does this by processing each UMP packet (or MIDI 1.0 Byte stream) one at a time. This way large data is handled in small chunks to keep memory small.

Note it is up to the application to:

- Store Remote MIDI-CI Device details
- Upon receiving MIDI-CI Message to interpret the Messages data structure (e.g. Profile Id bytes, Note On Articulation etc.)
- Handle logic and NAK sending and receiving.

This means the overheads for a simple MIDI 2.0 device is down to a compiled size of around 10k (possibly less?), with a memory footprint of around 1k.

### Example: Creating and Sending UMP Messages

```C++
#include <midi2/ump_types.hpp>

void send_note_on(std::uint8_t channel, std::uint8_t note, std::uint32_t velocity) {
  // Create an instance of the type that represents the UMP message to be sent. Set
  // the fields of the message as desired.
  auto const message = midi2::ump::m2cvm::note_on{}
    .group(0)
    .channel(channel)
    .note(note)
    .velocity(velocity);

  // Invoke a function for each of the words that make up the complete message. There's
  // no need for this code to understand the layout or size of the message.
  midi2::ump::apply(message, [](auto const v) {
      auto const word = std::uint32_t{v};
      // ... transmit the 32 bit word ...
      return std::error_code{};
    });
}
```

### Example: Processing a UMP stream

```cpp
#include <iostream>
#include <midi2/ump_dispatcher.hpp>

int main() {
  // We must pass a "context" to the dispatcher which will be forwarded to the callbacks
  // as they are invoked. The context enables message handlers to efficiently share state
  // but we don't need that in this simple example so a struct with no members will
  // suffice.
  struct context {};

  // Create the dispatcher with default-initialized context.
  auto dispatcher = midi2::make_ump_function_dispatcher<context>();

  // Ask the dispatcher for its configuration object and install handlers for MIDI2
  // note on/off channel  voice messages.
  dispatcher.config()
    .m2cvm
      .on_note_off([](context, midi2::ump::m2cvm::note_off const& noff) {
        std::cout << "note off: #" << unsigned{noff.note()}
                  << ", velocity " << noff.velocity() << '\n';
      })
      .on_note_on([](context, midi2::ump::m2cvm::note_on const& non) {
        std::cout << "note on: #" << unsigned{non.note()}
	          << ", velocity " << non.velocity() << '\n';
      });

  // Send note-on/off messages to the dispatcher.
  for (std::uint32_t const v : {0x40913C00, 0x7F100000, 0x40813C00, 0x7FFF0000}) {
    dispatcher.processUMP(v);
  }
}
```

### Example: Translate MIDI 1.0 Byte stream to UMP

Here is a quick example

```C++
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <midi2/bytestream_to_ump.hpp>

namespace {
consteval std::byte operator""_b(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::byte>(arg);
}
}  // end anonymous namespace

int main() {
  // Convert the bytestream to UMP group 0 and write it to stdout.
  std::uint8_t group = 0;
  midi2::bytestream_to_ump bs2ump{group};

  // A bytestream containing MIDI1 note-on events with running status.
  for (auto b : {0x81_b, 0x60_b, 0x50_b, 0x70_b, 0x70_b}) {
    // Push each byte into the bs2ump instance.
    bs2ump.push(b);

    // Pull as many 32-bit UMP values as are available and display them.
    while (!bs2ump.empty()) {
      std::cout << std::hex << bs2ump.pop() << '\n';
    }
  }
}
```

### Example: Process MIDI-CI Messages

> THIS EXAMPLE IS OBSOLETE

MIDI-CI requires a lot of SysEx messages. This library abstracts the complexity of building and parsing most MIDI-CI Messages.

```C++

#include "midiCIProcessor.h"
midi2Processor midiCIProcess;
uint32_t localMUID;
uint8_t sysexId[3] = {0x00 , 0x02, 0x22};
uint8_t famId[2] = {0x7F, 0x00};
uint8_t modelId[2] = {0x7F, 0x00};
uint8_t ver[4];
unint8_t sysexBuffer[512];

bool checkMUID(uint8_t group, uint32_t muid){
	return (localMUID==muid);
}

void recvDiscovery(uint8_t group, struct MIDICI ciDetails, uint8_t* remotemanuId, uint8_t* remotefamId, uint8_t* remotemodelId, uint8_t *remoteverId, uint8_t remoteciSupport, uint16_t remotemaxSysex){
	Serial.print("->Discovery: remoteMuid ");Serial.println(ciDetails.remoteMUID);
    uint16_t sBuffLen = sendDiscoveryReply(sysexBuffer, localMUID, ciDetails.remoteMUID, sysexId, famId, modelId, ver, 0b11100, 512);
    sendSysExOutOfDevice(sysexBuffer, sBuffLen);
}

void setup()
{
  localMUID = random(0xFFFFEFF);

  midiCIProcess.setRecvDiscovery(recvDiscovery);
  midiCIProcess.setCheckMUID(checkMUID);

  uint16_t sBuffLen = sendDiscoveryRequest(sysexBuffer,1, sysexId, famId, modelId, ver,12, 512);
  sendSysExOutOfDevice(sysexBuffer, sBuffLen);
}

void loop()
{
...
  while(uint8_t sysexByte = getNextSysexByte()){
    midiCIProcess.processUMP(sysexByte);
  }
...
}

```

---
