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
opus_apply_compile_options(opus)

include(GNUInstallDirs)
install(TARGETS opus EXPORT opusTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/opus DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN "*.hpp")
install(FILES "${OPUS_GENERATED_INCLUDE_DIR}/opus/version.hpp" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/opus)
install(EXPORT opusTargets NAMESPACE opus:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/opus FILE opusConfig.cmake)
