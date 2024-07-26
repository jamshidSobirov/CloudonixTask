cmake_minimum_required(VERSION 3.4.1)

add_library(native-lib SHARED native-lib.cpp)

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that you want CMake to locate.
        log)

# Specifies libraries CMake should link with the target library
target_link_libraries( # Specifies the target library.
        native-lib
        # Links the log library to the target library.
        ${log-lib})
