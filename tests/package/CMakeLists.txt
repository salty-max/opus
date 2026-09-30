# A downstream game, as small as possible: it finds the installed engine
# package and links opus::opus. Built by the "opusConfig" ctest.
cmake_minimum_required(VERSION 3.28)
project(opus_package_consumer LANGUAGES CXX)
find_package(opus REQUIRED CONFIG)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE opus::opus)
