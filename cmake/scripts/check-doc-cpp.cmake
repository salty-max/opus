# cmake -DEXTRACT=<opus-doc-snippets> -DCXX=<compiler> -DMSVC_STYLE=<bool>
#       -DSYSROOT=<macOS sysroot or empty> -DROOT=<repo> -DOUT=<dir>
#       -DINCLUDES=<dir|dir> -P check-doc-cpp.cmake
#
# A spec example is what a reader copies; one that no longer compiles
# teaches a wrong API silently. Every ```cpp block in docs/ and README.md is
# compiled against the public headers. A block that cannot stand alone
# starts with `// fragment: <why>` and is skipped.

file(GLOB_RECURSE docs RELATIVE "${ROOT}" "${ROOT}/docs/*.md")
list(APPEND docs README.md)

file(REMOVE_RECURSE "${OUT}")
execute_process(
  COMMAND "${EXTRACT}" --root "${ROOT}" --out "${OUT}" ${docs}
  RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "check-doc-cpp: extracting snippets failed")
endif()

string(REPLACE "|" ";" include_dirs "${INCLUDES}")
set(flags "")
if(MSVC_STYLE)
  list(APPEND flags /nologo /std:c++latest /Zs /W4 /WX /permissive- /EHsc)
  foreach(dir IN LISTS include_dirs)
    list(APPEND flags "/I${dir}")
  endforeach()
else()
  list(APPEND flags -std=c++23 -fsyntax-only -Wall -Wextra -Wpedantic -Werror)
  if(SYSROOT)
    list(APPEND flags -isysroot "${SYSROOT}")
  endif()
  foreach(dir IN LISTS include_dirs)
    list(APPEND flags "-I${dir}")
  endforeach()
endif()

file(GLOB snippets "${OUT}/*.cpp")
list(LENGTH snippets count)
set(failed 0)
foreach(snippet IN LISTS snippets)
  execute_process(COMMAND "${CXX}" ${flags} "${snippet}" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    math(EXPR failed "${failed} + 1")
  endif()
endforeach()

if(failed GREATER 0)
  message(FATAL_ERROR "check-doc-cpp: ${failed} of ${count} doc example(s) do not compile")
endif()
message(STATUS "check-doc-cpp: ${count} doc example(s) compile")
