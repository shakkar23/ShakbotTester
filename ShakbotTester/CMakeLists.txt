
# if linux
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(PNG REQUIRED)
    if(CMAKE_CXX_FLAGS_RELEASE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v3 -mbmi2")
    endif()
endif()

set(sources
    BitBoard.cpp fasthash.cpp search.cpp Source.cpp TBP.cpp TetrisBot.cpp
)

set (CMAKE_CXX_STANDARD 20)


add_executable(ShakBotTester ${sources})

if(UNIX AND NOT APPLE)
    target_include_directories(ShakBotTester PRIVATE ${X11_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS} ${PNG_INCLUDE_DIR})

    target_link_libraries(ShakBotTester ${X11_LIBRARIES} ${OPENGL_LIBRARIES} ${PNG_LIBRARY})
endif()

#link the Util library to ShakBotTester
target_link_libraries(ShakBotTester ShakUtil)
target_include_directories(ShakBotTester PRIVATE ${ShakUtil_INCLUDE_DIR})