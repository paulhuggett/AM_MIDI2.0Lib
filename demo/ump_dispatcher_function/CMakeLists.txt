#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

add_executable(demo_ump_dispatcher_function ump_dispatcher_function.cpp)
target_link_libraries(demo_ump_dispatcher_function PRIVATE midi2)
setup_target(demo_ump_dispatcher_function)
add_test(NAME demo-ump-dispatcher-function COMMAND demo_ump_dispatcher_function)
