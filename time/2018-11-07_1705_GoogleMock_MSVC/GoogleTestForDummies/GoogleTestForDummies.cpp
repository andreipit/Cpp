// GoogleTestForDummies.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include  "MockTurtle.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Painter.h"

using ::testing::AtLeast;               
using ::testing::Exactly;               
using ::testing::Return;
using ::testing::_;

TEST(PainterTest, CanDrawSomething) {
    MockTurtle turtle;                         
    EXPECT_CALL(turtle, PenDown())             
        .Times(Exactly(8));

    Painter painter(&turtle);                  

    EXPECT_TRUE(painter.DrawCircle(0, 0, 10));
}                                          

TEST(PainterTest, DISABLED_TestGetX) {
    MockTurtle turtle;                     
    EXPECT_CALL(turtle, GetX())            
        .Times(5)
        .WillOnce(Return(100))
        .WillOnce(Return(150))
        .WillRepeatedly(Return(200));

    Painter painter(&turtle);                  

    EXPECT_TRUE(painter.DrawCircle(0, 0, 10));
}                                             

TEST(PainterTest, TestForward) {
    MockTurtle turtle;                        
    // Expects the turtle to move forward.
    EXPECT_CALL(turtle, Forward(_));

    Painter painter(&turtle);                 

    EXPECT_TRUE(painter.DrawCircle(0, 0, 10));
}                                             

int main(int argc, char** argv) {
    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}