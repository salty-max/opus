# Custom targets behind the quick / verify / ci gates. Each one is a plain
# build target, so it runs the same way on every OS:
#
#   format        rewrite every first-party file with clang-format
#   format-check  fail if any file is not clang-format clean
#   lint          opus-lint over the whole tree (layout rules included)
#   docs          Doxygen API reference; any undocumented public symbol fails
#   run           build and launch the sandbox

set(OPUS_FORMAT_DIRS engine tests sandbox tools)

find_program(OPUS_CLANG_FORMAT_EXE
  NAMES clang-format clang-format-23
  HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin
)
if(OPUS_CLANG_FORMAT_EXE)
  foreach(mode IN ITEMS fix check)
    set(target_name format)
    if(mode STREQUAL "check")
      set(target_name format-check)
    endif()
    add_custom_target(${target_name}
      COMMAND "${CMAKE_COMMAND}"
        -DCLANG_FORMAT=${OPUS_CLANG_FORMAT_EXE}
        -DMODE=${mode}
        -DROOT=${PROJECT_SOURCE_DIR}
        "-DDIRS=${OPUS_FORMAT_DIRS}"
        -P "${PROJECT_SOURCE_DIR}/cmake/scripts/format.cmake"
      COMMENT "clang-format (${mode})"
      VERBATIM
    )
  endforeach()
else()
  foreach(target_name IN ITEMS format format-check)
    add_custom_target(${target_name}
      COMMAND "${CMAKE_COMMAND}" -E echo "clang-format not found; install it (see docs/development.md)"
      COMMAND "${CMAKE_COMMAND}" -E false
    )
  endforeach()
endif()

add_custom_target(lint
  COMMAND $<TARGET_FILE:opus-lint> --root "${PROJECT_SOURCE_DIR}"
  DEPENDS opus-lint
  COMMENT "opus-lint (whole tree)"
  VERBATIM
)

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
  set(OPUS_DOCS_OUTPUT_DIR "${PROJECT_BINARY_DIR}/docs")
  configure_file("${PROJECT_SOURCE_DIR}/docs/Doxyfile.in" "${PROJECT_BINARY_DIR}/Doxyfile" @ONLY)
  add_custom_target(docs
    COMMAND Doxygen::doxygen "${PROJECT_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Doxygen API reference -> ${OPUS_DOCS_OUTPUT_DIR}/html"
    VERBATIM
  )
else()
  add_custom_target(docs
    COMMAND "${CMAKE_COMMAND}" -E echo "doxygen not found; install it (see docs/development.md)"
    COMMAND "${CMAKE_COMMAND}" -E false
  )
endif()

add_custom_target(run
  COMMAND $<TARGET_FILE:opus-sandbox>
  DEPENDS opus-sandbox
  USES_TERMINAL
)
