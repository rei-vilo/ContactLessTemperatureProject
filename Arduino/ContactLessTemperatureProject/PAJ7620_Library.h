///
/// @file		PAJ7620_Library.h
/// @brief		Class library header
/// @details	C++ wrapper for PAJ7620 library
/// @n
/// @n @b		Project CLTP_Gestures
/// @n @a		Developed with embedXcode+: https://embedXcode.weebly.com
///
/// @author		Rei Vilo
/// @author		https://embeddedcomputing.weebly.com
///
/// @date		14 Aug 2020
/// @version	101
///
/// @copyright	(c) Rei Vilo, 2020
/// @copyright	CC = BY SA NC
///
/// @see		    paj7620.h library header
/// @n  A library for Grove-Guesture 1.0
/// @n  Copyright (c) 2015 seeed technology inc.
/// @n  Website    : www.seeed.cc
/// @n  Author     : Wuruibin & Xiangnan
/// @n  Modified Time: June 2015
///


// Core library - IDE-based
// !!! Help: http://bit.ly/2AdU7cu
#if defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"
#elif defined(ROBOTIS) // Robotis specific
#include "libpandora_types.h"
#include "pandora.h"
#elif defined(MPIDE) // chipKIT specific
#include "WProgram.h"
#elif defined(DIGISPARK) // Digispark specific
#include "Arduino.h"
#elif defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#elif defined(LITTLEROBOTFRIENDS) // LittleRobotFriends specific
#include "LRF.h"
#elif defined(MICRODUINO) // Microduino specific
#include "Arduino.h"
#elif defined(TEENSYDUINO) // Teensy specific
#include "Arduino.h"
#elif defined(REDBEARLAB) // RedBearLab specific
#include "Arduino.h"
#elif defined(RFDUINO) // RFduino specific
#include "Arduino.h"
#elif defined(SPARK) // Spark specific
#include "application.h"
#elif defined(ARDUINO) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE

#ifndef PAJ7620_LIBRARY__RELEASE
///
/// @brief    	Release
///
#define PAJ7620_LIBRARY__RELEASE 101

#include "paj7620.h"

///
/// @brief    Additional gesture: none
///
#define GES_NONE_FLAG 0xff


//    Notice: When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
//        You also can adjust the reaction time according to the actual circumstance.
#define GES_REACTION_TIME        500                // You can adjust the reaction time according to the actual circumstance.
#define GES_ENTRY_TIME            800                // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
#define GES_QUIT_TIME            1000

///
/// @class		C++ wrapper for PAJ7620 library
///
class PAJ7620
{

  public:
    ///
    /// @brief		Constructor
    ///
    PAJ7620();

    ///
    /// @brief		Initialisation
    /// @return     false=success, true otherwise
    ///
    bool begin();

    ///
    ///	@brief  	Who am I?
    ///	@return 	Who am I? string
    ///
    String WhoAmI();

    ///
    /// @brief    	Acquire the gesture
    /// @return   	false=success, true otherwise
    ///
    bool get();

    ///
    /// @brief    	Get the recognised gesture
    /// @return     gesture
    /// @note       Recognised gestures are GES_RIGHT_FLAG, GES_LEFT_FLAG, GES_UP_FLAG, GES_DOWN_FLAG, GES_FORWARD_FLAG, GES_BACKWARD_FLAG, GES_CLOCKWISE_FLAG, GES_COUNT_CLOCKWISE_FLAG, GES_WAVE_FLAG
    /// @note       Additional gesture is GES_NONE_FLAG
    ///
    uint8_t gesture();

  private:
    uint8_t _gesture;
};

#endif // PAJ7620_LIBRARY__RELEASE

