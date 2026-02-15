# MIDI2 Library

## Overview

- Translators

  - To/From Bytestream

    - midi2::bytestream::bytestream_to_ump
    - midi2::bytestream::ump_to_bytestream
    - midi2::bytestream::usbm1_to_bytestream

  - UMP MIDI 1 <-> MIDI 2 conversion
    - midi2::ump::ump_to_midi2
    - midi2::ump::ump_to_midi1

- Dispatchers

  - midi2::ci::ci_dispatcher
  - midi2::ump::ump_dispatcher

- Message Construction

  - midi2::ci::create_message()
  - Types in the midi2::ump namespace (e.g. midi2::ump::m2cvm::note_on)
    in conjunction with midi2::ump::apply()

- MIDI-specific Codecs
  - mcoded7
  - ci7text

## Using Translators

Translators are used to convert between different MIDI message formats. They
share a common, very simple, interface, which makes it easy to switch between
different types of translation.

The library provides five different translators. They are:

- To and from MIDI 1.0 bytestreams

  - MIDI 1.0 bytestream to UMP (\ref midi2::bytestream::bytestream_to_ump)
  - UMP MIDI 1.0 to MIDI 1.0 bytestream (\ref midi2::bytestream::ump_to_bytestream)
  - USB-MIDI Event Packets to MIDI 1.0 bytestream (\ref midi2::bytestream::usbm1_to_bytestream)

- UMP MIDI 1 <-> MIDI 2 conversion
  - MIDI 1.0 UMP packets to MIDI 2.0 UMP (\ref midi2::ump::ump_to_midi2)
  - MIDI 2.0 UMP packets to MIDI 1.0 UMP (\ref midi2::ump::ump_to_midi1)

The translators provide a common interface with `push()`, `empty()`,
`pop()`, and `reset()` methods. Some have additional member functions to
control filtering of input messages. The `input_type` and `output_type`
typedefs can be used to determine the correct types for the input and output
messages. This is enforced by the \ref midi2::translator concept.

```cpp
class translator {
public:
  using input_type = ...;   // Type of input messages
  using output_type = ...;  // Type of output messages

  void push(input_type in); // Push an input message into the translator
  bool empty() const;       // Is output available?
  output_type pop();        // Pop an output message from the translator
  void reset();             // Reset the translator state
};
```

The code snippet below shows the general usage pattern for the translators.
They are designed to handle streaming data: you create a translator instance,
push each input message byte or 32-bit value into it and then immediately pull
any translated output from the translator's internal FIFO. The resulting output
can then be dispatched to its next destination.

```cpp
translator t;
for (translator::input_type in : input) {
  t.push(in);
  while (!t.empty()) {
    Translator::output_type out = translator.pop();
    // Send 'out' to its next destination
  }
}
```

(Replace the “translator” placeholder in the code above with one of the real
translator types.)

## Using Dispatchers

The library provides two different dispatchers. The dispatchers accept input
message bytes or 32-bit values, decode them, and call functions provided by the
application to handle those messages. They are for:

- MIDI Capability Inquiry message (\ref midi2::ci::ci_dispatcher)
- Universal MIDI Packet (UMP) messages (\ref midi2::ump::ump_dispatcher)

When instantiating one of the dispatchers, you must provide a configuration
type. This type defines:

1. The collection of functions that will handle the dispatched messages
2. A “context” object that will be passed to each of the message handling
   functions. See below for more information.
3. (CI dispatcher only) A size for the dispatcher’s internal buffer.

The full requirements for the configuration type are described by the
`midi2::ci::ci_dispatcher_config` or `midi2::ump::ump_dispatcher_config`
concepts.

### Contexts

A “context” object is passed to each of the message handling functions to
enable them to efficiently share state. This can be a pointer to an object
in the larger system or an instance owned by the configuration object itself.
If not needed by the handlers, it can be an empty struct declared with
`[[no_unique_address]]` so that it takes no space.

### Predefined message handlers

1. Null

   A group of message handlers in which every function is a static constexpr
   function that does nothing. The can be very useful to simply drop an entire
   class of input messages. Since they are constexpr, the compiler can often
   remove swathes of code leading up to the call of these functions so they can
   be very efficient.

2. Pure Virtual

   The message handlers are defined as pure virtual. The library’s unit testing
   mocks make extensive use of these classes to track adn verify the delivered
   messages.

3. Virtual Base

   Each message handler class derives from the matching pure-virtual class. It
   provides concrete implementations that each do nothing. You can derive from
   one of these classes and override one or more of the handlers with very
   little code. The disadvantage is obviously that each mesage incurs the
   overhead of a virtual method call.

4. std::function<>

   The easiest configuration type to use, but may not always be the most time
   or space efficient. Use judiciously! The simplest way to use the
   `std::function<>`-based handlers is to create the dispatcher (possibly
   using one of the helper functions:
   `midi2::ump::make_ump_function_dispatcher()` or
   `midi2::ci::make_function_dispatcher()`).

5. Custom message handlers

## Message Construction

### MIDI CI

midi2::ci::create_message()

### UMP Messages

Creating a UMP message is a simple matter of instantiating the relevant type. The midi2::ump namespace contains a type that corresponds to each of the possible UMP messages. These are further grouped in namespaces according to the message category. These categories are:

| Message Type           | Namespace               |
| ---------------------- | ----------------------- |
| Utility                | `midi2::ump::utility`   |
| System Common          | `midi2::ump::system`    |
| System Real Time       | `midi2::ump::system`    |
| MIDI 1.0 Channel Voice | `midi2::ump::m1cvm`     |
| Data 64-Bit            | `midi2::ump::data64`    |
| MIDI 2.0 Channel Voice | `midi2::ump::m2cvm`     |
| Data 128-Bit           | `midi2::ump::data128`   |
| UMP Stream             | `midi2::ump::stream`    |
| Flex Data              | `midi2::ump::flex_data` |

Each type can be default-constructed. This will set all fields, except those that identify the message type, to 0. Accessor functions, whose names generally match those of field in question, allow the individual fields of the message to be read or written. "Getter" accessors take no arguments and return the associated value; "setter" accessor take the value to be set and return `*this` enabling calls to be chained. Note that there is no "setter" method for the fields that identify the message: these are set implicitly when the message instance is created and are read-only.

For example, to create a MIDI 1.0 Note-on message and set its various fields:

```cpp
auto const non = midi2::ump::m1cvm::note_on{}.group(g).channel(c).note(n).velocity(v);
```

This approach means that calling code does not need to know which of the message's constituent words contains each field: that is the responsibility of the message type itself. Having said that, the UMP message types are all "tuple-like". This means that, when necessary, the individual words of the message can be accessed as members of the tuple:

```cpp
auto noff = midi2::ump::m2cvm::note_off{}
  .group(g)
  .channel(c)
  .note(n)
  .velocity(v)
  .attribute(a);

// You can use std::tuple_size<> to determine the number of words in the message:
constexpr auto words = std::tuple_size_v<midi2::ump::m2cvm::note_off>();

// Extract the first (and only) word of the message.
// We could also use an index (0) as the get() template argument.
auto w0 = get<midi2::ump::m2cvm::note_off::word0>(non);

// We can also use a structured binding declaration.
auto const [x0, x1] = noff;
```

#### Apply

Hello

\example send_ump.cpp

This example demonstrates how to create UMP MIDI 2.0 note-on and note-off messages, to fill in their fields, and how the resulting sequence of 32-bit values can be transmitted. The code uses note-on and note-off, but the same principle applies to all UMP message types. In this example the resulting 32-bit values are simply written to the console, but a real implementation would likely transmit the values using a communications protocol such as USB-MIDI.

Expected output:

```
0x40913C00 0x27100000 - 0x40914000 0x27100000 - 0x40914300 0x27100000 -
0x40813C00 0x27100000 - 0x40814000 0x27100000 - 0x40814300 0x27100000 -
```

\example ump_dispatcher_function.cpp

This demo program shows the UMP dispatcher being used with the provided "function" backend class.

The dispatcher's backend class determines how each UMP message is handled. The "function" backend uses
std::function<> to make it simple to use callback functions (be they lambdas or other types of callable object) as
message handlers.

Be aware that std::function<> is powerful and simple to use but may allocate memory or throw exceptions.
For resource-constrained applications, one of the other backend options may be more suitable.

Expected output:

```
note on: #60, velocity 32528
note off: #60, velocity 32767
```

\example midi1_to_ump.cpp

Expected output:

```
Bytestream input: 0x81 0x60 0x50 0x70 0x70
UMP Packets: 0x20816050 0x20817070
```

\example ci_dispatcher.cpp
\example ci_create_discovery.cpp
