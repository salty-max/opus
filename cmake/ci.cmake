# Full local matrix, mirroring GitHub Actions on this host:
#
#   cmake -P cmake/ci.cmake
#
# verify (lint, tidy, docs, Debug tests) followed by every other build mode
# and the sanitizer build. Required before tagging a release.

set(workflows verify test-relwithdebinfo test-release test-minsizerel)
# Apple Clang's ASan runtime hangs at startup on macOS 26, so macOS builds the
# sanitizer lane with Homebrew LLVM (see the asan-macos preset).
if(CMAKE_HOST_APPLE)
  list(APPEND workflows test-asan-macos)
else()
  list(APPEND workflows test-asan)
endif()

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH root)
foreach(workflow IN LISTS workflows)
  message(STATUS "==> ${workflow}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" --workflow --preset ${workflow}
    WORKING_DIRECTORY "${root}"
    RESULT_VARIABLE result
  )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "ci: workflow '${workflow}' failed")
  endif()
endforeach()
message(STATUS "ci: all workflows green")
