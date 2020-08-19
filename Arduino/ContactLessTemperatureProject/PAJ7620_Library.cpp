//
// PAJ7620_Library.cpp
// Class library C++ code
// ----------------------------------
// Developed with embedXcode+
// https://embedXcode.weebly.com
//
// Project 		CLTP_Gestures
//
// Created by 	Rei Vilo, 14 Aug 2020 09:57
// 				https://embeddedcomputing.weebly.com
//
// Copyright 	(c) Rei Vilo, 2020
// License		CC = BY SA NC
//
// See 			PAJ7620_Library.h and ReadMe.txt for references
//


// Library header
#include "PAJ7620_Library.h"

// Code
PAJ7620::PAJ7620()
{
    // nothing
}

bool PAJ7620::begin()
{
    _gesture = GES_NONE_FLAG;
    return paj7620Init();
}

String PAJ7620::WhoAmI()
{
    return "PAJ7620 Gesture";
}

bool PAJ7620::get()
{
    _gesture = GES_NONE_FLAG;
    return paj7620ReadReg(0x43, 1, &_gesture);
    delay(GES_QUIT_TIME);
}

uint8_t PAJ7620::gesture()
{
    return _gesture;
}
