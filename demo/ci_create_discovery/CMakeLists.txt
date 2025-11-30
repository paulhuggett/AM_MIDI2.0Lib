#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

add_executable(demo_ci_create_discovery ci_create_discovery.cpp)
target_link_libraries(demo_ci_create_discovery PRIVATE midi2)
setup_target(demo_ci_create_discovery)
add_test(NAME demo_ci_create_discovery COMMAND demo_ci_create_discovery)
