cmake_minimum_required(VERSION 3.30...4.1)
project(interpreter)

add_subdirectory(src)
add_subdirectory(samples)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

