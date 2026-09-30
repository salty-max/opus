cmake_minimum_required(VERSION 3.28)

# The version is owned by scripts/changeset-version.sh — edit it through a
# changeset, never by hand.
project(opus VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(OPUS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(OPUS_CLANG_TIDY "Run clang-tidy on every compiled translation unit" OFF)
set(OPUS_SANITIZE "" CACHE STRING "Comma-separated sanitizers, e.g. address,undefined")

# Only the top-level project gets tests, tools and gates; a game consuming
# the engine through add_subdirectory/FetchContent gets the library alone.
set(OPUS_IS_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(OpusCompileOptions)
include(OpusClangTidy)

add_subdirectory(engine)

if(OPUS_IS_TOP_LEVEL)
  include(CTest)
  include(OpusDependencies)
  add_subdirectory(tools/lint)
  add_subdirectory(sandbox)
  add_subdirectory(tests)
  include(OpusGates)
  include(OpusPackaging)
endif()
