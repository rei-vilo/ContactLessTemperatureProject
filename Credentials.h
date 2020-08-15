///
/// @file       Credentials.h
/// @brief      Header
/// @details    Credentials for CLTP
/// @n
/// @n @b       Project ContactLessTemperatureProject
/// @n @a       Developed with embedXcode+: https://embedXcode.weebly.com
///
/// @author     Rei Vilo
/// @author     https://embeddedcomputing.weebly.com
///
/// @date       08 Aug 2020 10:57
/// @version    101
///
/// @copyright  (c) Rei Vilo, 2020
/// @copyright  CC = BY SA NC
///
/// @see        ReadMe.txt for references
///


// Core library for code-sense - IDE-based
// !!! Help: http://bit.ly/2AdU7cu
#if defined(ARDUINO) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE


#ifndef CREDENTIALS_RELEASE
///
/// @brief    Release
///
#define CREDENTIALS_RELEASE

///
/// @brief Option for WiFi
/// @details 1=activated, 0=desactivated
///
#define OPTION_WIFI 1

///
/// @brief IP address of the MQTT broker
///
char brokerIP[] = "192.168.1.10";

///
/// @brief Name of the client
///
char clientID[] = "Thermal-1";

///
/// @brief Network name = SSID
///
char ssidWiFi[] = "my network name";

///
/// @brief Network password
///
char passwordWiFi[] = "my network password";

#endif
