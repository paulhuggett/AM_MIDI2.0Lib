cmake_minimum_required(VERSION 3.12)

project(libmidi2)

option (MIDI2_WERROR "Enable warnings-as-errors" No)
option (MIDI2_COVERAGE "Enable code coverage" No)
option (MIDI2_SANITIZE "Enable sanitizers" No)
option (MIDI2_FUZZTEST "Enable Fuzz Testing" No)

enable_testing ()

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(setup_target)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIBMIDI2_HEADERS
  "${INCLUDE_DIR}/bytestreamToUMP.h"
  "${INCLUDE_DIR}/fifo.h"
  "${INCLUDE_DIR}/mcoded7.h"
  "${INCLUDE_DIR}/midiCIMessageCreate.h"
  "${INCLUDE_DIR}/midiCIProcessor.h"
  "${INCLUDE_DIR}/umpMessageCreate.h"
  "${INCLUDE_DIR}/umpProcessor.h"
  "${INCLUDE_DIR}/umpToBytestream.h"
  "${INCLUDE_DIR}/umpToMIDI1Protocol.h"
  "${INCLUDE_DIR}/utils.h"
)

add_library(libmidi2 ${LIBMIDI2_HEADERS}
  src/utils.cpp
  src/bytestreamToUMP.cpp
  src/umpToBytestream.cpp
  src/umpToMIDI1Protocol.cpp
  src/umpMessageCreate.cpp
  src/midiCIMessageCreate.cpp
  src/midiCIProcessor.cpp
  src/umpProcessor.cpp
)
setup_target(libmidi2)

target_include_directories(libmidi2 PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

set(LIBMIDI2_EXPORT_NAME libmidi2-config)
install(TARGETS libmidi2
        EXPORT ${LIBMIDI2_EXPORT_NAME}
)
install(EXPORT ${LIBMIDI2_EXPORT_NAME}
        NAMESPACE libmidi2::
        DESTINATION "share/${PROJECT_NAME}"
)

install(FILES ${LIBMIDI2_HEADERS} DESTINATION "include/${PROJECT_NAME}")

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE libmidi2)
setup_target(tests)
set (clang_options -Wno-global-constructors -Wno-used-but-marked-unused)
set (gcc_options)
set (msvc_options -wd4702) # unreachable code.
target_compile_options (
  tests
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${clang_options}>
    $<$<CXX_COMPILER_ID:GNU>:${gcc_options}>
    $<$<CXX_COMPILER_ID:MSVC>:${msvc_options}>
)
add_test(NAME tests COMMAND tests)

if (MIDI2_FUZZTEST)
  set (FUZZTEST_FUZZING_MODE On)
  include (FetchContent)
  set (FUZZTEST_REPO_BRANCH "1635d42" CACHE STRING "FuzzTest repository branch.")
  message ("Building fuzztest at tag " ${FUZZTEST_REPO_BRANCH})
  FetchContent_Declare (
    fuzztest
    GIT_REPOSITORY https://github.com/google/fuzztest.git
    GIT_TAG ${FUZZTEST_REPO_BRANCH}
  )
  FetchContent_MakeAvailable (fuzztest)
  include (GoogleTest)
  fuzztest_setup_fuzzing_flags ()
else ()
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt")
    # Tell gtest to link against the "Multi-threaded Debug DLL runtime library"
    # on Windows.
    set (gtest_force_shared_crt On CACHE BOOL "Always use msvcrt.dll")
    # We don't want to install either gtest or gmock.
    set (INSTALL_GTEST Off CACHE BOOL "Disable gtest install")
    set (INSTALL_GMOCK Off CACHE BOOL "Disable gmock install")
    add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/googletest")
  endif()
endif (MIDI2_FUZZTEST)

add_subdirectory(unittests)
