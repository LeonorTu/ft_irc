file(GLOB_RECURSE TEST_SOURCES
    *.cpp
)

add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE gtest_main ft_irc_lib)

include(GoogleTest)
gtest_discover_tests(unit_tests)

# add_executable(channel_tests channelTests.cpp)
# target_link_libraries(channel_tests PRIVATE gtest_main ft_irc_lib)
# gtest_discover_tests(channel_tests)
