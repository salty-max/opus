file(GLOB_RECURSE OPUS_LINT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE OPUS_LINT_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.test.cpp")
list(REMOVE_ITEM OPUS_LINT_SOURCES ${OPUS_LINT_TEST_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_library(opus-lint-core STATIC ${OPUS_LINT_SOURCES})
target_include_directories(opus-lint-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
opus_apply_compile_options(opus-lint-core)

add_executable(opus-lint main.cpp)
target_link_libraries(opus-lint PRIVATE opus-lint-core)
opus_apply_compile_options(opus-lint)

# Colocated specs are globbed into the test binary, so none can exist
# without running.
add_executable(opus-lint-tests ${OPUS_LINT_TEST_SOURCES} "${PROJECT_SOURCE_DIR}/tests/main.cpp")
target_link_libraries(opus-lint-tests PRIVATE opus-lint-core doctest::doctest)
opus_apply_compile_options(opus-lint-tests ALLOW_EXCEPTIONS)
doctest_discover_tests(opus-lint-tests TEST_PREFIX "opus-lint: ")
