
#===-- CMakeLists ------------------------------------------------------------*- C++ -*-===//
#
# midi2 library under the MIT license.
# See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
# SPDX-License-Identifier: MIT
#
#===------------------------------------------------------------------------------------===//

cmake_minimum_required(VERSION 3.12)

project(midi2)

option(MIDI2_WERROR "Enable warnings-as-errors" No)
option(MIDI2_COVERAGE "Enable code coverage" No)
option(MIDI2_SANITIZE "Enable sanitizers" No)
option(MIDI2_FUZZTEST "Enable Fuzz Testing" No)
option(MIDI2_UNITTEST "Enable Unit Testing" Yes)
option(MIDI2_DEMO "Enable the demo programs" No)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(setup_target)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(MIDI2_HEADERS
  "${INCLUDE_DIR}/midi2/adt/bitfield.hpp"
  "${INCLUDE_DIR}/midi2/adt/cache.hpp"
  "${INCLUDE_DIR}/midi2/adt/fifo.hpp"
  "${INCLUDE_DIR}/midi2/adt/iumap.hpp"
  "${INCLUDE_DIR}/midi2/adt/lru_list.hpp"
  "${INCLUDE_DIR}/midi2/bytestream_to_ump.hpp"
  "${INCLUDE_DIR}/midi2/ci7text.hpp"
  "${INCLUDE_DIR}/midi2/ci_create_message.hpp"
  "${INCLUDE_DIR}/midi2/ci_dispatcher.hpp"
  "${INCLUDE_DIR}/midi2/ci_dispatcher_backend.hpp"
  "${INCLUDE_DIR}/midi2/ci_types.hpp"
  "${INCLUDE_DIR}/midi2/icubaby.hpp"
  "${INCLUDE_DIR}/midi2/mcoded7.hpp"
  "${INCLUDE_DIR}/midi2/ump_dispatcher.hpp"
  "${INCLUDE_DIR}/midi2/ump_dispatcher_backend.hpp"
  "${INCLUDE_DIR}/midi2/ump_to_bytestream.hpp"
  "${INCLUDE_DIR}/midi2/ump_to_midi1.hpp"
  "${INCLUDE_DIR}/midi2/ump_to_midi2.hpp"
  "${INCLUDE_DIR}/midi2/ump_types.hpp"
  "${INCLUDE_DIR}/midi2/usbm1_to_bytestream.hpp"
  "${INCLUDE_DIR}/midi2/utils.hpp"
)

add_library(midi2 ${MIDI2_HEADERS}
  src/bytestream_to_ump.cpp
  src/ump_to_midi1.cpp
  src/ump_to_midi2.cpp
)
setup_target(midi2)

target_include_directories(midi2 PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

set(MIDI2_EXPORT_NAME midi2-config)
install(TARGETS midi2
        EXPORT ${MIDI2_EXPORT_NAME}
)
install(EXPORT ${MIDI2_EXPORT_NAME}
        NAMESPACE midi2::
        DESTINATION "share/${PROJECT_NAME}"
)

install(FILES ${MIDI2_HEADERS} DESTINATION "include/${PROJECT_NAME}")


if(MIDI2_UNITTEST OR MIDI2_FUZZTEST OR MIDI2_DEMO)
  enable_testing()
endif()


if(MIDI2_DEMO)
  add_subdirectory(demo)
endif(MIDI2_DEMO)


if(MIDI2_FUZZTEST)
  set(FUZZTEST_FUZZING_MODE On)
  include(FetchContent)
  set(FUZZTEST_REPO_BRANCH "main" CACHE STRING "FuzzTest repository branch.")
  message("Building fuzztest at tag " ${FUZZTEST_REPO_BRANCH})
  FetchContent_Declare(
    fuzztest
    GIT_REPOSITORY https://github.com/google/fuzztest.git
    GIT_TAG ${FUZZTEST_REPO_BRANCH}
  )
  FetchContent_MakeAvailable(fuzztest)
  include(GoogleTest)
  fuzztest_setup_fuzzing_flags()

  add_subdirectory(unittests)
elseif(MIDI2_UNITTEST AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt")
  # Tell gtest to link against the "Multi-threaded Debug DLL runtime library"
  # on Windows.
  set(gtest_force_shared_crt On CACHE BOOL "Always use msvcrt.dll")
  # We don't want to install either gtest or gmock.
  set(INSTALL_GTEST Off CACHE BOOL "Disable gtest install")
  set(INSTALL_GMOCK Off CACHE BOOL "Disable gmock install")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/googletest")

  add_subdirectory(unittests)
endif(MIDI2_FUZZTEST)


foreach(target gtest gmock gmock_main gtest_main)
  if(TARGET ${target})
    target_compile_options(${target} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-deprecated-declarations -Wno-covered-switch-default>
      )
      target_compile_features(${target} PUBLIC cxx_std_20)
    target_compile_definitions(${target} PUBLIC GTEST_REMOVE_LEGACY_TEST_CASEAPI_=1)
  endif()
endforeach()
