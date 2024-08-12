add_executable (m2unittests
  test_bitfield.cpp
  test_ci_processor.cpp
  test_ci_types.cpp
  test_fifo.cpp
  test_bytestreamToUMP.cpp
  test_mcoded7.cpp
  test_umpProcessor.cpp
  test_umpToBytestream.cpp
  test_umpToMIDI1Protocol.cpp
  test_ump_bytestream_round_trip.cpp
)
target_compile_definitions (m2unittests PRIVATE MIDI2_FUZZTEST=$<BOOL:${MIDI2_FUZZTEST}>)
target_link_libraries(m2unittests PRIVATE libmidi2)
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
add_test(NAME m2unittests COMMAND m2unittests)


if (MIDI2_FUZZTEST)
  target_link_libraries (m2unittests PRIVATE gtest gmock)
  link_fuzztest (m2unittests)
  gtest_discover_tests (m2unittests)
else ()
  target_link_libraries (m2unittests PRIVATE gmock_main)
endif (MIDI2_FUZZTEST)
