#include "stdafx.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>

int main(int argc, char** argv) 
{
    ::testing::InitGoogleMock(&argc, argv);
    RUN_ALL_TESTS();
    std::cin.get();
}