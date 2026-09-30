file(GLOB_RECURSE OPUS_ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE OPUS_ENGINE_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/opus/*.hpp")

add_library(opus STATIC ${OPUS_ENGINE_SOURCES} ${OPUS_ENGINE_HEADERS})
add_library(opus::opus ALIAS opus)

set(OPUS_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/include" CACHE INTERNAL "Generated public headers")
configure_file(include/opus/version.hpp.in "${OPUS_GENERATED_INCLUDE_DIR}/opus/version.hpp" @ONLY)

target_include_directories(opus
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${OPUS_GENERATED_INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:include>"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_compile_features(opus PUBLIC cxx_std_23)
# PRIVATE: SDL is an implementation detail. No SDL header or type may appear
# under include/opus/ (opus-lint's include-path rule enforces it).
target_link_libraries(opus PRIVATE SDL3::SDL3-static)
opus_apply_compile_options(opus)

include(GNUInstallDirs)
install(TARGETS opus EXPORT opusTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/opus DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.hpp")
install(FILES "${OPUS_GENERATED_INCLUDE_DIR}/opus/version.hpp" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/opus)

include(CMakePackageConfigHelpers)
set(OPUS_PACKAGE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/opus)
install(EXPORT opusTargets NAMESPACE opus:: DESTINATION ${OPUS_PACKAGE_DIR} FILE opusTargets.cmake)
configure_package_config_file("${PROJECT_SOURCE_DIR}/cmake/opusConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/opusConfig.cmake" INSTALL_DESTINATION ${OPUS_PACKAGE_DIR})
# Below 1.0 every minor version may break the API (see .changeset/README.md).
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/opusConfigVersion.cmake"
  COMPATIBILITY SameMinorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/opusConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/opusConfigVersion.cmake"
  DESTINATION ${OPUS_PACKAGE_DIR})
install(FILES "${sdl3_SOURCE_DIR}/LICENSE.txt" DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/opus/SDL3)
