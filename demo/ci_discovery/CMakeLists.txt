add_executable(demo_ci_discovery ci_discovery.cpp)
target_link_libraries(demo_ci_discovery PRIVATE midi2)
setup_target(demo_ci_discovery)
add_test(NAME demo_ci_discovery COMMAND demo_ci_discovery)

