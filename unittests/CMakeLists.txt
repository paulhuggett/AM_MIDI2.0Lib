add_executable (unittests
    test_fifo.cpp
    test_bytestreamToUMP.cpp
    test_mcoded7.cpp
    test_umpToBytestream.cpp
    test_umpToMIDI1Protocol.cpp
    test_ump_bytestream_round_trip.cpp
)
target_compile_definitions (unittests PRIVATE MIDI2_FUZZTEST=$<BOOL:${MIDI2_FUZZTEST}>)
target_link_libraries(unittests PRIVATE libmidi2)
add_test(NAME unittests COMMAND unittests)


if (MIDI2_FUZZTEST)
  target_link_libraries (unittests PRIVATE gtest gmock)
  link_fuzztest (unittests)
  gtest_discover_tests (unittests)
else ()
  target_link_libraries (unittests PRIVATE gmock_main)
endif (MIDI2_FUZZTEST)
