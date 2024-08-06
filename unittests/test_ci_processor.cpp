#include "midi2/midiCIProcessor.h"

#include <gtest/gtest.h>

TEST(CIProcessor, Empty) {
  midi2::midiCIProcessor ci;
  ci.processMIDICI(0);
}
