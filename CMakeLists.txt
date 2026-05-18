cmake_minimum_required(VERSION 3.20...4.1)
project(interpreter)

add_subdirectory(src)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

