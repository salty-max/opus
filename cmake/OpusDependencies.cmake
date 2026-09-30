# Third-party dependencies, pinned to exact tags. Every dependency is
# fetched at configure time and built with the project, identically on
# all three desktop platforms.

include(FetchContent)

FetchContent_Declare(doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.5.3
  GIT_SHALLOW TRUE
  SYSTEM
)
# doctest's own cmake_minimum_required predates 3.10; its deprecation
# warning is noise we cannot act on.
set(_opus_warn_deprecated ${CMAKE_WARN_DEPRECATED})
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(doctest)
set(CMAKE_WARN_DEPRECATED ${_opus_warn_deprecated} CACHE BOOL "" FORCE)
include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")
