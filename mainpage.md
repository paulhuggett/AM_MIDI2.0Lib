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

  - MIDI 1.0 bytestream to UMP (`midi2::bytestream::bytestream_to_ump`)
  - UMP MIDI 1.0 to MIDI 1.0 bytestream (`midi2::bytestream::ump_to_bytestream`)
  - USB-MIDI Event Packets to MIDI 1.0 bytestream (`midi2::bytestream::usbm1_to_bytestream`)

- UMP MIDI 1 <-> MIDI 2 conversion
  - MIDI 1.0 UMP packets to MIDI 2.0 UMP (`midi2::ump::ump_to_midi2`)
  - MIDI 2.0 UMP packets to MIDI 1.0 UMP (`midi2::ump::ump_to_midi1`)

The translators each provide the same interface with `push()`, `empty()`,
`pop()`, and `reset()` methods. Some have additional member functions to
control filtering of input messages. The `input_type` and `output_type`
typedefs can be used to determine the correct types for the input and output
messages.

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

- MIDI Capability Inquiry message (`midi2::ci::ci_dispatcher<>`)
- Universal MIDI Packet (UMP) messages (`midi2::ump::ump_dispatcher<>`)

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

Types in the midi2::ump namespace (e.g. midi2::ump::m2cvm::note_on) in
conjunction with `midi2::ump::apply()`
