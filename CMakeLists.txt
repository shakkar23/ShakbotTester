project("ShakBotTester")

cmake_minimum_required(VERSION 3.22)

if(UNIX AND NOT APPLE)
    if(CMAKE_CXX_FLAGS_RELEASE)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=x86-64-v3 -mbmi2")
    endif()
endif()
# make a utility library that has one cpp file and is only to be used as a static library
add_library(ShakUtil STATIC "Util/util.cpp" "Util/bits.hpp")
target_include_directories(ShakUtil PUBLIC "Util")
# to include the bits header into your project you would do this
set (ShakUtil_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Util")

add_subdirectory(ShakbotTester)
