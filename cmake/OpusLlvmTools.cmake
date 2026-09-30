# clang-format and clang-tidy come from one pinned LLVM major version.
# Formatting and diagnostics change between majors, so a different version
# would make the gates disagree between machines; it is rejected instead.
# CI installs the same major (LLVM_VERSION in .github/workflows/ci.yml).
set(OPUS_LLVM_VERSION 23)

# opus_find_llvm_tool(<var> <tool>)
#
# Sets <var> to <tool> of the pinned LLVM version, preferring the versioned
# name (Debian/Ubuntu install clang-tidy-23 next to an older clang-tidy) and
# Homebrew's keg-only LLVM, which is not on PATH. A tool of another version
# counts as missing: <var> is left unset and <var>_REASON says what was found,
# so each caller decides whether the tool is required.
function(opus_find_llvm_tool var tool)
  set(${var}_REASON "${tool} not found" PARENT_SCOPE)
  find_program(${var}
    NAMES ${tool}-${OPUS_LLVM_VERSION} ${tool}
    HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin
  )
  if(NOT ${var})
    return()
  endif()
  execute_process(COMMAND "${${var}}" --version OUTPUT_VARIABLE version_output ERROR_QUIET)
  string(REGEX MATCH "version ([0-9]+)\\." _ "${version_output}")
  if(NOT CMAKE_MATCH_1 STREQUAL OPUS_LLVM_VERSION)
    set(${var}_REASON "${${var}} is LLVM ${CMAKE_MATCH_1}, not the pinned LLVM ${OPUS_LLVM_VERSION}" PARENT_SCOPE)
    # Forget the rejected path so the next configure searches again.
    set(${var} "${var}-NOTFOUND" CACHE FILEPATH "Path to ${tool} ${OPUS_LLVM_VERSION}" FORCE)
  endif()
endfunction()
