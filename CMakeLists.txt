cmake_minimum_required(VERSION 3.10)

project(ft_irc)

include_directories(include)

file(GLOB SOURCES
    src/*.cpp
    src/*.hpp
)

add_executable(ft_irc ${SOURCES})
target_compile_features(ft_irc PRIVATE cxx_std_17)

# enable_testing()
# add_subdirectory(tests)
