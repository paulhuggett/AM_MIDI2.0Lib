add_executable(demo_midi1_to_ump midi1_to_ump.cpp)
target_link_libraries(demo_midi1_to_ump PRIVATE midi2)
setup_target(demo_midi1_to_ump)
add_test(NAME demo-midi1-to-ump COMMAND demo_midi1_to_ump)
