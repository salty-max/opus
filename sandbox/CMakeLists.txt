add_executable(opus-sandbox main.cpp)
target_link_libraries(opus-sandbox PRIVATE opus::opus)
opus_apply_compile_options(opus-sandbox)
install(TARGETS opus-sandbox RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
