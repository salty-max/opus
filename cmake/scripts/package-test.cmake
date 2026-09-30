# cmake -DBUILD_DIR=<engine build> -DCONFIG=<config> -DSOURCE=<consumer src>
#       -DWORK=<scratch dir> -DCXX=<compiler> -DGENERATOR=<generator>
#       -DSANITIZE=<sanitizers or empty> -P package-test.cmake
#
# Installs the engine into a scratch prefix, then configures, builds and runs
# a consumer that only knows `find_package(opus)`. This is the path a game
# takes through a release package, so a broken package config fails here
# instead of in someone else's build.

file(REMOVE_RECURSE "${WORK}")

function(run)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    list(JOIN ARGN " " command)
    message(FATAL_ERROR "package-test: `${command}` failed")
  endif()
endfunction()

run("${CMAKE_COMMAND}" --install "${BUILD_DIR}" --config "${CONFIG}" --prefix "${WORK}/prefix")

set(flags "")
if(SANITIZE)
  set(flags "-fsanitize=${SANITIZE}")
endif()
run("${CMAKE_COMMAND}" -S "${SOURCE}" -B "${WORK}/build" -G "${GENERATOR}"
  "-DCMAKE_CXX_COMPILER=${CXX}" "-DCMAKE_BUILD_TYPE=${CONFIG}"
  "-DCMAKE_PREFIX_PATH=${WORK}/prefix" "-DCMAKE_CXX_FLAGS=${flags}" "-DCMAKE_EXE_LINKER_FLAGS=${flags}")
run("${CMAKE_COMMAND}" --build "${WORK}/build" --config "${CONFIG}")

find_program(consumer NAMES consumer PATHS "${WORK}/build" "${WORK}/build/${CONFIG}" NO_DEFAULT_PATH REQUIRED)
run("${consumer}")
