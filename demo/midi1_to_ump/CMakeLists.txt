add_executable(demo_midi1_to_ump midi1_to_ump.cpp)
target_link_libraries(demo_midi1_to_ump PRIVATE midi2)

      target_compile_features(demo_midi1_to_ump PRIVATE cxx_std_20)
