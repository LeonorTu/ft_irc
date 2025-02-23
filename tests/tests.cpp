#include <gtest/gtest.h>
#include "../src/server/server.hpp"

TEST(ServerTest, InitializationTest)
{
    Server server;
    EXPECT_EQ(server.getPort(), 6667); // Assuming default IRC port
    EXPECT_EQ(server.getServerFD(), -1);
}

// TEST(ServerTest, StartServerTest)
// {
//     Server server;
//     server.start();
//     EXPECT_GE(server.getServerFD(), 0);
//     server.stop();
// }
