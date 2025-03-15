cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ft_irc)
enable_testing()

# Set C++ standard for all targets
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Werror)

include_directories(include)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_library(ft_irc_lib ${SOURCES})

add_executable(ft_irc src/main.cpp)

target_link_libraries(ft_irc PRIVATE ft_irc_lib)

# Download and build Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
FetchContent_MakeAvailable(googletest)

# Add tests directory
add_subdirectory(tests)
