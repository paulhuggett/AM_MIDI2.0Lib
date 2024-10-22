# AM MIDI 2.0 Lib 
A MIDI 2.0 Library

This is a general purpose Library for building MIDI 2.0 Devices and Applications. This library aims to work on everything from embedded devices through to large scale applications. It provides the building blocks, processing and 
translations needed for most MIDI 2.0 Devices and Applications

This code is based on Andrew Mee’s library at <https://github.com/midi2-dev/AM_MIDI2.0Lib>. It has been heavily modified with a number of goals:

- Using C++20 features
- Limiting use of magic numbers and bitwise operators to define data layout and instead using the C++ type system wherever possible
- UMP convertion classes are now built on the library's `ump_dispatcher` class rather than duplicating code to extract UMP messages
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

* Convert a MIDI 1.0 byte-stream to UMP and back
* Process and construct UMP streams
* Process and construct MIDI CI messages

This library is designed to use a small footprint. It does this by processing each UMP packet (or MIDI 1.0 Byte stream) one at a time. This way large data is handled in small chunks to keep memory small.

Note it is up to the application to:

 * Store Remote MIDI-CI Device details
 * Upon receiving MIDI-CI Message to interpret the Messages data structure (e.g. Profile Id bytes, Note On Articulation etc.)
 * Handle logic and NAK sending and receiving.

This means the overheads for a simple MIDI 2.0 device is down to a compiled size of around 10k (possibly less?), with a memory footprint of around 1k.


### Example: Translate MIDI 1.0 Byte stream to UMP

Here is a quick example

```C++
#include "bytestreamUMP.h"

bytestreamToUMP BS2UMP;

void setup()
{
  Serial.begin(31250);
  
  //Produce MIDI 2.0 Channel Voice Message (Message Type 0x4)
  //Default (false) will return MIDI 1.0 Channel Voice Messages (Message Type 0x2)
  BS2UMP.outputMIDI2 = true; 
  
  //Set the UMP group of the output UMP message. By default this is set to Group 1
  //defaultGroup value is 0 based
  BS2UMP.defaultGroup = 0; //Group 1
}

void loop()
{
  uint8_t inByte = 0;
  // if there is a serial MIDI byte:
  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
    if(inByte == 0xFE) return; //Skip ActiveSense 
    
    BS2UMP.midi1BytestreamParse(inByte);
    while(BS2UMP.available()){
      uint32_t ump = BS2UMP.read();
      //ump contains a ump 32 bit value. UMP messages that have 64bit will produce 2 UMP words
    }
  }
}
```

### Example: Process UMP Streams
UMP Streams accepts a series of 32 bit values. UMP messages that have 64bit will provide 2 UMP words.

```C++

#include "umpProcessor.h"
umpProcessor UMPProcess; 

void noteOff(uint8_t group,  uint8_t mt, uint8_t channel, uint8_t noteNumber, unsigned int velocity, int attributeType, unsigned int attributeData){
//Process incoming MIDI note Off event.
}

void noteOn(uint8_t group,  uint8_t mt, uint8_t channel, uint8_t noteNumber, unsigned int velocity, int attributeType, unsigned int attributeData){ 
}


void cc(uint8_t group,  uint8_t mt, uint8_t channel, uint8_t index, uint32_t value){  
}

void rpn(uint8_t group, uint8_t channel, uint8_t bank,  uint8_t index, uint32_t value){  
}

void setup()
{
    UMPProcess.setNoteOff(noteOff);
    UMPProcess.setNoteOn(noteOn);
    UMPProcess.setControlChange(cc);
    UMPProcess.setRPN(rpn);
}

void loop()
{
...
  while(uint32_t ump = readSomeUMP()){
      UMPProcess.processUMP(ump);
  }
...  
}

```

### Example: Process MIDI-CI Messages
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



