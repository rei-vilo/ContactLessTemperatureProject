///
/// @file       Interpolation.h
/// @brief      Library header
/// @details    Thermal image interpolation
/// @n
/// @n @b       Project ContactLessTemperatureProject
/// @n @a       Developed with embedXcode+: https://embedXcode.weebly.com
///
/// @author     Rei Vilo
/// @author     https://embeddedcomputing.weebly.com
///
/// @date       16 Aug 2020
/// @version    106
///
/// @copyright  (c) Rei Vilo, 2020
/// @note       Wrapper for Interpolation.cpp
///
/// @see        ReadMe.txt for references
/// https://github.com/adafruit/Adafruit_AMG88xx/tree/master/examples/thermal_cam_interpolate
///

// Core library for code-sense - IDE-based
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

#ifndef INTERPLATION_RELEASE
///
/// @brief	Release
///
#define INTERPLATION_RELEASE

// Prototypes moved from Interpolation.cpp

float get_point(float * p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void set_point(float * p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
void get_adjacents_1d(float * src, float * dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void get_adjacents_2d(float * src, float * dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
float cubicInterpolate(float p[], float x);
float bicubicInterpolate(float p[], float x, float y);

///
/// @brief    Interpolate an image
/// @param    src source array, float
/// @param    src_rows source number of rows
/// @param    src_cols source number of columns
/// @param    dest destination array, float
/// @param    dest_rows destination number of rows
/// @param    dest_cols destination number of columns
///
void interpolate_image(float * src, uint8_t src_rows, uint8_t src_cols, float * dest, uint8_t dest_rows, uint8_t dest_cols);

#endif // INTERPLATION_RELEASE
