#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    auto success = RUN_ALL_TESTS();
    return success;
}
