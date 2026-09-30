# Every tests/**/*.test.cpp is compiled into one executable, so a new spec
# file cannot exist without running. opus-lint's mirror rule pairs each
# spec with the engine file it covers.
file(GLOB_RECURSE OPUS_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.test.cpp")

add_executable(opus-tests main.cpp ${OPUS_TEST_SOURCES})
target_include_directories(opus-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(opus-tests PRIVATE opus::opus doctest::doctest)
opus_apply_compile_options(opus-tests ALLOW_EXCEPTIONS)

doctest_discover_tests(opus-tests)
