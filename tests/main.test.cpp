
#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <stdio.h>

int main(int argc, char **argv)
{
    std::cout << argv[1] << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(log, log)
{
    std::cout << "hello world" << std::endl; 
}

