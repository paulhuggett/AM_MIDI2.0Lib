//===-- UMP -------------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/ump/ump_types.hpp"

// Google test
#include <gtest/gtest.h>

namespace {

template <typename T> class UMPCheckFixture : public testing::Test {
public:
  T value_;
};
using ump_types = ::testing::Types<
    midi2::ump::utility::noop, midi2::ump::utility::jr_clock, midi2::ump::utility::jr_timestamp,
    midi2::ump::utility::delta_clockstamp_tpqn, midi2::ump::utility::delta_clockstamp,

    midi2::ump::system::midi_time_code, midi2::ump::system::song_position_pointer, midi2::ump::system::song_select,
    midi2::ump::system::tune_request, midi2::ump::system::timing_clock, midi2::ump::system::sequence_start,
    midi2::ump::system::sequence_continue, midi2::ump::system::sequence_stop, midi2::ump::system::active_sensing,
    midi2::ump::system::reset,

    midi2::ump::m1cvm::note_on, midi2::ump::m1cvm::note_off, midi2::ump::m1cvm::poly_pressure,
    midi2::ump::m1cvm::control_change, midi2::ump::m1cvm::program_change, midi2::ump::m1cvm::channel_pressure,
    midi2::ump::m1cvm::pitch_bend,

    midi2::ump::m2cvm::note_off, midi2::ump::m2cvm::note_on, midi2::ump::m2cvm::poly_pressure,
    midi2::ump::m2cvm::rpn_per_note_controller, midi2::ump::m2cvm::nrpn_per_note_controller,
    midi2::ump::m2cvm::rpn_controller, midi2::ump::m2cvm::nrpn_controller, midi2::ump::m2cvm::rpn_relative_controller,
    midi2::ump::m2cvm::nrpn_relative_controller, midi2::ump::m2cvm::per_note_management,
    midi2::ump::m2cvm::control_change, midi2::ump::m2cvm::program_change, midi2::ump::m2cvm::channel_pressure,
    midi2::ump::m2cvm::pitch_bend, midi2::ump::m2cvm::per_note_pitch_bend,

    midi2::ump::stream::endpoint_discovery, midi2::ump::stream::endpoint_info_notification,
    midi2::ump::stream::device_identity_notification, midi2::ump::stream::endpoint_name_notification,
    midi2::ump::stream::product_instance_id_notification, midi2::ump::stream::jr_configuration_request,
    midi2::ump::stream::jr_configuration_notification, midi2::ump::stream::function_block_discovery,
    midi2::ump::stream::function_block_info_notification, midi2::ump::stream::function_block_name_notification,
    midi2::ump::stream::start_of_clip, midi2::ump::stream::end_of_clip,

    midi2::ump::flex_data::set_tempo, midi2::ump::flex_data::set_time_signature, midi2::ump::flex_data::set_metronome,
    midi2::ump::flex_data::set_key_signature, midi2::ump::flex_data::set_chord_name, midi2::ump::flex_data::text_common,

    midi2::ump::data128::sysex8_in_1, midi2::ump::data128::sysex8_start, midi2::ump::data128::sysex8_continue,
    midi2::ump::data128::sysex8_end, midi2::ump::data128::mds_header, midi2::ump::data128::mds_payload>;
TYPED_TEST_SUITE(UMPCheckFixture, ump_types);
TYPED_TEST(UMPCheckFixture, Check) {
  ASSERT_TRUE(midi2::ump::check(this->value_));
}

}  // end anonymous namespace
