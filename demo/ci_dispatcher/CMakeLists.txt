#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
#
# SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

add_executable(demo_ci_dispatcher ci_dispatcher.cpp)
target_link_libraries(demo_ci_dispatcher PRIVATE midi2)
setup_target(demo_ci_dispatcher)
add_test(NAME demo_ci_dispatcher COMMAND demo_ci_dispatcher)
