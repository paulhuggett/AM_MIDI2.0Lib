add_executable (unittests
    test_fifo.cpp
    test_bytestreamToUMP.cpp
    test_mcoded7.cpp
)
target_compile_definitions (unittests PRIVATE ICUBABY_FUZZTEST=$<BOOL:${ICUBABY_FUZZTEST}>)
target_link_libraries(unittests PRIVATE libmidi2)
add_test(NAME unittests COMMAND unittests)


if (ICUBABY_FUZZTEST)
  target_link_libraries (unittests PRIVATE gtest gmock)
  link_fuzztest (unittests)
  gtest_discover_tests (unittests)
else ()
  target_link_libraries (unittests PRIVATE gmock_main)
endif (ICUBABY_FUZZTEST)
