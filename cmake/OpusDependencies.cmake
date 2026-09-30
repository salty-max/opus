# Third-party dependencies. Every dependency is pinned — a release tarball
# with its SHA-256, or an exact tag — fetched at configure time and built
# with the project, identically on all three desktop platforms.
#
# Third-party code is outside the first-party gates: it never gets clang-tidy
# or opus_apply_compile_options (so no warnings-as-errors), and its headers
# are SYSTEM so their warnings never surface in first-party translation units.
# Every dependency is recorded in THIRD_PARTY.md.

include(FetchContent)

# Runs FetchContent_MakeAvailable with clang-tidy switched off, so no target
# the dependency defines picks up CMAKE_<LANG>_CLANG_TIDY.
macro(opus_make_available_third_party)
  set(_opus_saved_cxx_clang_tidy "${CMAKE_CXX_CLANG_TIDY}")
  set(CMAKE_CXX_CLANG_TIDY "")
  FetchContent_MakeAvailable(${ARGN})
  set(CMAKE_CXX_CLANG_TIDY "${_opus_saved_cxx_clang_tidy}")
endmacro()

# --- Engine dependencies: needed by every consumer of the library ----------

FetchContent_Declare(SDL3
  URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.16/SDL3-3.4.16.tar.gz
  URL_HASH SHA256=7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68
  SYSTEM
)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
# Installed alongside the engine so the exported opus::opus can resolve its
# private link dependency (see cmake/opusConfig.cmake.in).
set(SDL_INSTALL ON CACHE BOOL "" FORCE)
opus_make_available_third_party(SDL3)

# --- Development dependencies: tests and tools of the top-level build only -

if(OPUS_IS_TOP_LEVEL)
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
  opus_make_available_third_party(doctest)
  set(CMAKE_WARN_DEPRECATED ${_opus_warn_deprecated} CACHE BOOL "" FORCE)
  # Without it doctest forward-declares std::ostream on every standard
  # library but libc++, and MSVC then cannot print a failed string_view check.
  target_compile_definitions(doctest INTERFACE DOCTEST_CONFIG_USE_STD_HEADERS)
  include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")
endif()
