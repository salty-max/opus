# Compiler policy shared by every first-party target.
#
#   opus_apply_compile_options(<target> [ALLOW_EXCEPTIONS])
#
# Engine and tools build without exceptions or RTTI: errors travel through
# std::expected, and dynamic_cast/typeid have no place in the engine.
# Test executables pass ALLOW_EXCEPTIONS because doctest reports failed
# REQUIREs by throwing.

set(_opus_gcc_like_warnings
  -Wall -Wextra -Wpedantic
  -Wconversion -Wsign-conversion -Wshadow -Wold-style-cast -Wcast-align
  -Wnon-virtual-dtor -Woverloaded-virtual -Wnull-dereference
  -Wdouble-promotion -Wimplicit-fallthrough -Wformat=2 -Wundef
)
set(_opus_gcc_only_warnings -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast)
set(_opus_msvc_warnings /W4 /permissive- /utf-8 /Zc:__cplusplus /Zc:preprocessor)

function(opus_apply_compile_options target)
  cmake_parse_arguments(ARG "ALLOW_EXCEPTIONS" "" "" ${ARGN})

  if(MSVC)
    target_compile_options(${target} PRIVATE ${_opus_msvc_warnings})
    if(OPUS_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
    if(ARG_ALLOW_EXCEPTIONS)
      target_compile_options(${target} PRIVATE /EHsc)
    else()
      target_compile_options(${target} PRIVATE /EHs-c- /GR-)
      target_compile_definitions(${target} PRIVATE _HAS_EXCEPTIONS=0)
    endif()
  else()
    target_compile_options(${target} PRIVATE ${_opus_gcc_like_warnings})
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(${target} PRIVATE ${_opus_gcc_only_warnings})
    endif()
    if(OPUS_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
    if(NOT ARG_ALLOW_EXCEPTIONS)
      target_compile_options(${target} PRIVATE -fno-exceptions -fno-rtti)
    endif()
  endif()
endfunction()

# Sanitizers apply to the whole build so that instrumented and plain code
# never mix inside one binary.
if(OPUS_SANITIZE)
  string(REPLACE "," ";" _opus_sanitizers "${OPUS_SANITIZE}")
  if(MSVC)
    if(NOT _opus_sanitizers STREQUAL "address")
      message(FATAL_ERROR "MSVC supports only OPUS_SANITIZE=address (got '${OPUS_SANITIZE}')")
    endif()
    add_compile_options(/fsanitize=address)
  else()
    add_compile_options(-fsanitize=${OPUS_SANITIZE} -fno-omit-frame-pointer -fno-sanitize-recover=all)
    add_link_options(-fsanitize=${OPUS_SANITIZE})
  endif()
endif()
