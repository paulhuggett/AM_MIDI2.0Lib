#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

add_executable(demo_send_ump send_ump.cpp)
target_link_libraries(demo_send_ump PRIVATE midi2)
setup_target(demo_send_ump)
add_test(NAME demo_send_ump COMMAND demo_send_ump)
