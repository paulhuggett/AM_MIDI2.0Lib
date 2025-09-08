include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.4
  FIND_PACKAGE_ARGS NAMES benchmark
)
FetchContent_MakeAvailable(benchmark)








add_executable(m2benchmark m2benchmark.cpp)
setup_target(m2benchmark)
target_link_libraries(m2benchmark PRIVATE midi2 benchmark::benchmark)

set(clang_options
  -Wno-exit-time-destructors
  -Wno-global-constructors
  -Wno-used-but-marked-unused
)
set(gcc_options)
set(msvc_options -wd4702) # unreachable code.
target_compile_options(m2benchmark PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${clang_options}>
  $<$<CXX_COMPILER_ID:GNU>:${gcc_options}>
  $<$<CXX_COMPILER_ID:MSVC>:${msvc_options}>
)
