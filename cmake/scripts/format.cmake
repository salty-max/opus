# cmake -DCLANG_FORMAT=<exe> -DMODE=fix|check -DROOT=<repo> -DDIRS=<a;b> -P format.cmake
#
# Globs at run time, not configure time, so new files are always covered.
# Configured templates (*.hpp.in) are skipped: clang-format splits their
# @VARIABLE@ placeholders.

set(files "")
foreach(dir IN LISTS DIRS)
  file(GLOB_RECURSE found "${ROOT}/${dir}/*.cpp" "${ROOT}/${dir}/*.hpp")
  list(APPEND files ${found})
endforeach()

if(MODE STREQUAL "check")
  set(args --dry-run -Werror)
else()
  set(args -i)
endif()

execute_process(
  COMMAND "${CLANG_FORMAT}" ${args} ${files}
  RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "clang-format ${MODE} failed; run `cmake --build --preset debug --target format`")
endif()
