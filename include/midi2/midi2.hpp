/// \file midi2.hpp
/// \brief An umbrella include file for the MIDI 2.0 library.

#ifndef MIDI2_HPP
#define MIDI2_HPP

/// \mainpage
/// \include{doc} mainpage.md

#include "midi2/bytestream/bytestream_to_ump.hpp"
#include "midi2/bytestream/bytestream_types.hpp"
#include "midi2/bytestream/ump_to_bytestream.hpp"
#include "midi2/bytestream/usbm1_to_bytestream.hpp"
#include "midi2/ci/ci7text.hpp"
#include "midi2/ci/ci_create_message.hpp"
#include "midi2/ump/ump_dispatcher.hpp"
#include "midi2/ump/ump_to_midi1.hpp"
#include "midi2/ump/ump_to_midi2.hpp"
#include "midi2/ump/ump_types.hpp"

#endif  // MIDI2_HPP
