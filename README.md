# AM MIDI 2.0 Lib

A MIDI 2.0 Library

This is a general purpose Library which provides the building blocks, processing and translations needed for most MIDI 2.0 devices and applications. This library aims to work on everything from embedded devices through to large scale applications with minimal code and data footprint.

This code is based on Andrew Mee’s library at <https://github.com/midi2-dev/AM_MIDI2.0Lib>. It has been heavily modified with a number of goals:

- Using C++20 features
- The library does not throw exceptions or allocate dynamic memory
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

## Examples

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
  midi2::ump::apply(message, [](std::uint32_t const word) {
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
  // In this scenario, we need to pass a “context” to the dispatcher,
  // which will be forwarded to the callbacks as they are invoked.
  // The context facilitates efficient state sharing among message
  // handlers, but this is not necessary for our simple example.
  // Therefore, a struct with no members will suffice.
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

### Example: Create MIDI-CI Messages

~~~cpp
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <vector>

#include "midi2/ci_create_message.hpp"
#include "midi2/ci_types.hpp"
#include "midi2/utils.hpp"

namespace {

std::vector<std::byte> discovery() {
  constexpr midi2::ci::params params{
      .device_id = 0x7F, .version = 2, .remote_muid = 0, .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery discovery{.manufacturer = {0x12, 0x23, 0x34},
                                           .family = 0x1779,
                                           .model = 0x2B5D,
                                           .version = {0x4E, 0x3C, 0x2A, 0x18},
                                           .capability = 0x7F,
                                           .max_sysex_size = 256,
                                           .output_path_id = 0x71};
  std::vector<std::byte> message;
  auto const out_it = std::back_inserter(message);
  midi2::ci::create_message(out_it, midi2::ci::trivial_sentinel<decltype(out_it)>{}, params, discovery);
  return message;
}

}  // end anonymous namespace

int main() {
  for (auto const b : discovery()) {
    std::cout << std::format("{:02x} ", midi2::to_underlying(b));
  }
  std::cout << '\n';
}
~~~

### Example: Process MIDI-CI Messages

MIDI-CI requires a lot of SysEx messages. This library abstracts the complexity of building and parsing MIDI-CI Messages.

~~~cpp
#include "midi2/ci_dispatcher.hpp"

#include <array>
#include <cstdlib>
#include <format>
#include <iostream>
#include <type_traits>

using namespace midi2::ci::literals;

namespace {
// Display the header fields
void print_header(std::ostream &os, midi2::ci::header const &h) {
  os << std::format("device-id=0x{:X}\nversion=0x{:X}\n", h.device_id, h.version)
     << std::format("remote-MUID=0x{:X}\nlocal-MUID=0x{:X}\n\n", h.remote_muid, h.local_muid);
}
// Display the discovery data fields
void print_discovery(std::ostream &os, midi2::ci::discovery const &d) {
  os << std::format("manufacturer={}\nfamily=0x{:X}\nmodel=0x{:X}\n", d.manufacturer, d.family, d.model)
     << std::format("version={}\ncapability=0x{:X}\n", d.version, d.capability)
     << std::format("max-sysex-size=0x{:X}\noutput-path-id=0x{:X}\n", d.max_sysex_size, d.output_path_id);
}

// We must pass a “context” to the dispatcher, which will be forwarded to 
// each of the dispatcher's callbacks. The context lets message handlers share
// state but we don’t need that here, so a struct with no members will suffice.
struct context {};

dispatcher setup_ci_dispatcher(midi2::ci::muid const my_muid) {
  // Create a CI dispatcher instance using std::function<> for 
  // all of its handler functions.
  auto dispatcher = midi2::ci::make_function_dispatcher<context, buffer_size>();
  auto &config = dispatcher.config();

  // Register a handler for checking whether a message is addressed 
  // to this receiver.
  config.system.on_check_muid(
      [my_muid](context, std::uint8_t /*group*/, midi2::ci::muid const m) { return m == my_muid; });

  // Register a handler for Discovery messages.
  config.management.on_discovery([](context, midi2::ci::header const &h, midi2::ci::discovery const &d) {
    print_header(std::cout, h);
    print_discovery(std::cout, d);
    // Send a reply to this message...
  });
  return dispatcher;
}
}  // end anonymous namespace

int main() {
  // Use a proper random number!
  constexpr auto my_muid = midi2::ci::muid{0x01234567U};
  constexpr auto my_group = std::uint8_t{0};
  constexpr auto device_id = 0_b7;
  auto dispatcher = setup_ci_dispatcher(my_muid);

  // A system exclusive message containing a CI discovery request.
  constexpr std::array message{0x7E, 0x7F, 0x0D, 0x70, 0x02, 0x00, 0x00, 0x00, 0x00, 0x7F,
                               0x7F, 0x7F, 0x7F, 0x12, 0x23, 0x34, 0x79, 0x2E, 0x5D, 0x56,
                               0x01, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x02, 0x00, 0x00, 0x00};
  dispatcher.start(my_group, device_id);
  for (auto const b : message) {
    dispatcher.processMIDICI(static_cast<std::byte>(b));
  }
  dispatcher.finish();
}
~~~
---
