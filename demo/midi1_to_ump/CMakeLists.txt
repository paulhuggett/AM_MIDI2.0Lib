#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

add_executable(demo_midi1_to_ump midi1_to_ump.cpp)
target_link_libraries(demo_midi1_to_ump PRIVATE midi2)
setup_target(demo_midi1_to_ump)
add_test(NAME demo-midi1-to-ump COMMAND demo_midi1_to_ump)
