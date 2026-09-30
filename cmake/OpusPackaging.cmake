# `cpack` archives the install tree (engine library, public headers,
# CMake package config, sandbox) for GitHub releases.

set(CPACK_PACKAGE_NAME opus)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "opus-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
if(WIN32)
  set(CPACK_GENERATOR ZIP)
else()
  set(CPACK_GENERATOR TGZ)
endif()
include(CPack)
