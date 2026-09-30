# With OPUS_CLANG_TIDY=ON, clang-tidy runs as part of compiling each
# first-party translation unit, so it is incremental: only files that
# rebuild get re-checked. A compiled third-party target must clear its
# CXX_CLANG_TIDY property where it is fetched.

if(NOT OPUS_CLANG_TIDY)
  return()
endif()

# Homebrew ships LLVM keg-only, so its clang-tidy is not on PATH.
find_program(OPUS_CLANG_TIDY_EXE
  NAMES clang-tidy clang-tidy-23
  HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin
  REQUIRED
)
set(CMAKE_CXX_CLANG_TIDY "${OPUS_CLANG_TIDY_EXE};--quiet;--warnings-as-errors=*")
message(STATUS "clang-tidy: ${OPUS_CLANG_TIDY_EXE}")
