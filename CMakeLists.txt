cmake_minimum_required(VERSION 3.10)

project(ft_irc)
enable_testing()

include_directories(include)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_library(ft_irc_lib ${SOURCES})

add_executable(ft_irc src/main.cpp)

target_link_libraries(ft_irc PRIVATE ft_irc_lib)
target_compile_features(ft_irc PRIVATE cxx_std_20)

# Download and build Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# Add tests directory
add_subdirectory(tests)
