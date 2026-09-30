# With OPUS_CLANG_TIDY=ON, clang-tidy runs as part of compiling each
# first-party translation unit, so it is incremental: only files that
# rebuild get re-checked. Third-party targets are created with it switched
# off (OpusDependencies.cmake).

if(NOT OPUS_CLANG_TIDY)
  return()
endif()

opus_find_llvm_tool(OPUS_CLANG_TIDY_EXE clang-tidy)
if(NOT OPUS_CLANG_TIDY_EXE)
  message(FATAL_ERROR "OPUS_CLANG_TIDY=ON but clang-tidy ${OPUS_LLVM_VERSION} was not found (see docs/development.md)")
endif()
set(CMAKE_CXX_CLANG_TIDY "${OPUS_CLANG_TIDY_EXE};--quiet;--warnings-as-errors=*")
message(STATUS "clang-tidy: ${OPUS_CLANG_TIDY_EXE}")
