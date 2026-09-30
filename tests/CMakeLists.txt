# Every tests/**/*.test.cpp is compiled into one executable, so a new spec
# file cannot exist without running. opus-lint's mirror rule pairs each
# spec with the engine file it covers.
file(GLOB_RECURSE OPUS_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.test.cpp")

add_executable(opus-tests main.cpp ${OPUS_TEST_SOURCES})
target_include_directories(opus-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(opus-tests PRIVATE opus::opus doctest::doctest)
opus_apply_compile_options(opus-tests ALLOW_EXCEPTIONS)

doctest_discover_tests(opus-tests)

add_test(NAME "opusConfig: a find_package consumer builds and runs"
  COMMAND "${CMAKE_COMMAND}"
    "-DBUILD_DIR=${PROJECT_BINARY_DIR}"
    "-DCONFIG=$<CONFIG>"
    "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/package"
    "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/package"
    "-DCXX=${CMAKE_CXX_COMPILER}"
    "-DGENERATOR=${CMAKE_GENERATOR}"
    "-DSANITIZE=${OPUS_SANITIZE}"
    -P "${PROJECT_SOURCE_DIR}/cmake/scripts/package-test.cmake"
)
