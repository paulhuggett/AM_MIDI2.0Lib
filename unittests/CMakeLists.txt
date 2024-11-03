#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

add_executable (m2unittests
  test_bitfield.cpp
  test_bytestream_to_ump.cpp
  test_cache.cpp
  test_ci7.cpp
  test_ci_create_message.cpp
  test_ci_dispatcher.cpp
  test_ci_types.cpp
  test_fifo.cpp
  test_iumap.cpp
  test_lru.cpp
  test_mcoded7.cpp
  test_scale.cpp
  test_ump_bytestream_round_trip.cpp
  test_ump_dispatcher.cpp
  test_ump_to_bytestream.cpp
  test_ump_to_midi1.cpp
  test_ump_to_midi2.cpp
  test_usbm1_to_bytestream.cpp
)
target_compile_definitions (m2unittests PRIVATE MIDI2_FUZZTEST=$<BOOL:${MIDI2_FUZZTEST}>)
target_link_libraries(m2unittests PRIVATE midi2)
setup_target(m2unittests PEDANTIC $<NOT:$<BOOL:${MIDI2_FUZZTEST}>>)
set (clang_options -Wno-exit-time-destructors -Wno-global-constructors -Wno-used-but-marked-unused)
set (gcc_options)
set (msvc_options -wd4702) # unreachable code.
target_compile_options (
  m2unittests
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${clang_options}>
    $<$<CXX_COMPILER_ID:GNU>:${gcc_options}>
    $<$<CXX_COMPILER_ID:MSVC>:${msvc_options}>
)

if (MIDI2_FUZZTEST)
  target_link_libraries (m2unittests PRIVATE gtest gmock)
  link_fuzztest (m2unittests)
else ()
  target_link_libraries (m2unittests PRIVATE gmock_main)
endif (MIDI2_FUZZTEST)

include(GoogleTest)
gtest_discover_tests(m2unittests)
       
