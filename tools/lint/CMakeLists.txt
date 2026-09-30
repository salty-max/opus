file(GLOB_RECURSE OPUS_LINT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE OPUS_LINT_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.test.cpp")
set(OPUS_LINT_MAINS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/doc_snippets_main.cpp")
list(REMOVE_ITEM OPUS_LINT_SOURCES ${OPUS_LINT_TEST_SOURCES} ${OPUS_LINT_MAINS})

add_library(opus-lint-core STATIC ${OPUS_LINT_SOURCES})
target_include_directories(opus-lint-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
opus_apply_compile_options(opus-lint-core)

add_executable(opus-lint main.cpp)
target_link_libraries(opus-lint PRIVATE opus-lint-core)
opus_apply_compile_options(opus-lint)

add_executable(opus-doc-snippets doc_snippets_main.cpp)
target_link_libraries(opus-doc-snippets PRIVATE opus-lint-core)
opus_apply_compile_options(opus-doc-snippets)

# Colocated specs are globbed into the test binary, so none can exist
# without running.
add_executable(opus-lint-tests ${OPUS_LINT_TEST_SOURCES} "${PROJECT_SOURCE_DIR}/tests/main.cpp")
target_link_libraries(opus-lint-tests PRIVATE opus-lint-core doctest::doctest)
opus_apply_compile_options(opus-lint-tests ALLOW_EXCEPTIONS)
doctest_discover_tests(opus-lint-tests TEST_PREFIX "opus-lint: ")
