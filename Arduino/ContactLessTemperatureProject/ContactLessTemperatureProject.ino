///
/// @mainpage	ContactLessTemperatureProject
///
/// @details	Remote Contact Less Temperature Project
/// @n
/// @n
/// @n @a		Developed with [embedXcode+](https://embedXcode.weebly.com)
///
/// @author		Rei Vilo
/// @author		https://embeddedcomputing.weebly.com
/// @date		07 Aug 2020 10:32
/// @version	106
///
/// @copyright	(c) Rei Vilo, 2020
/// @copyright	CC = BY SA NC
///
/// @see		ReadMe.txt for references
///


///
/// @file		ContactLessTemperatureProject.ino
/// @brief		Main sketch
///
/// @details	Main program
/// @n @a		Developed with [embedXcode+](https://embedXcode.weebly.com)
///
/// @author		Rei Vilo
/// @author		https://embeddedcomputing.weebly.com
/// @date		07 Aug 2020 10:32
/// @version    106
///
/// @copyright	(c) Rei Vilo, 2020
/// @copyright	CC = BY SA NC
///
/// @see		ReadMe.txt for references
/// @n
///


// Core library for code-sense - IDE-based
// !!! Help: http://bit.ly/2AdU7cu
#if defined(ARDUINO) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE

// Set parameters


// Include application, user and local libraries
// !!! Help http://bit.ly/2CL22Qp
// Peripherals
#include "Wire.h"
#include "SPI.h"

// Sensors
#include "PAJ7620_Library.h"
#include "Seeed_AMG8833_driver.h"
#include "TFT_eSPI.h"
#include "Free_Fonts.h"

// WiFi and MQTT
#include "Credentials.h"

#if (OPTION_WIFI == 1)
#include "AtWiFi.h"
#include "PubSubClient.h"

WiFiClient myClient;
PubSubClient myMQTT(myClient);
volatile bool commandMQTT = false;
#endif

// Define structures and classes
#define myPIR A0
#define myButtonC 30
///
/// @brief    Interpolation
/// @note     8=default=AMG8833, 16 or 24
///
#define INTERPOLATION 16
AMG8833 myAMG8833;
String thermalCheckString = "";
String thermalLocalString = "";
PAJ7620 myPAJ7620;
uint8_t gesture = 0;

// AMG8833 thermal picture
float thermalPicture[PIXEL_NUM] = {0};

#if (INTERPOLATION > 8)
#include "Interpolation.h"
float interpolatedPicture[INTERPOLATION * INTERPOLATION] = {0};
#endif

// Colours constants
const uint16_t RGB565[] =
{
    0xFFFF, 0xFDDF, 0xFBBF, 0xF81F,
    0xF80E, 0xB000, 0xF980, 0xFBA0,
    0xFDC0, 0xFDC0, 0xB7E0, 0x75C0,
    0x07E0, 0x07F6, 0x07FF, 0x03BF,
    0x001F, 0x0016, 0x0006, 0x0000
};

TFT_eSPI myScreen = TFT_eSPI();
#define LCD_BACKLIGHT 72

// Define variables and constants
enum eState
{
    ST_NONE,
    ST_INITIALISATION,
    ST_WAKEUP,
    ST_PICTURE,
    ST_QUESTION,
    ST_RESULT,
    ST_ASSISTANCE,
    ST_SLEEP,
};

eState state = ST_INITIALISATION;
eState oldState = ST_NONE;

enum eTransition
{
    TR_none,
    TR_initialisation,
    TR_pir,
    TR_picture,
    TR_green,
    TR_orange,
    TR_red,
    TR_left,
    TR_right,
    TR_call,
    TR_wait,
};

eTransition transition = TR_initialisation;
eTransition oldTransition = TR_none;

enum eStatus
{
    statusNone,
    statusNegative,
    statusInconclusive,
    statusPositive,
};

eStatus status = statusNone;

// Other
bool doFlag = true;
uint64_t timeStart;

// Prototypes
// !!! Help: http://bit.ly/2TAbgoI


// Utilities
///
/// @brief    Wait with display
/// @param    second period is seconds
///
void wait(uint8_t second)
{
    myScreen.setFreeFont(FSSB12);
    myScreen.setTextDatum(TR_DATUM);
    myScreen.setTextColor(0xffff, 0x0000);

    for (uint8_t t = second; t > 0; t--)
    {
        myScreen.drawString("   " + String(t) + " ", 310, 0, GFXFF);
        delay(1000);
    }
}

enum eScreen
{
    screenNone,
    screenWarning,
    screenResult,
    screenQuestion,
    screenAssistance,
    screenPositive,
};

///
/// @brief    Display screen
/// @param    number screen number, as per eScreen
///
// Arduino doesn't accept enum as parameter
//void displayScreen(eScreen number)
void displayScreen(uint8_t number)
{
    myScreen.fillScreen(0x0000);
    uint16_t y = 0;

    switch (number)
    {
        case screenWarning:

            myScreen.setTextDatum(TC_DATUM);
            myScreen.setTextColor(0xffff, 0x0000);

            myScreen.setFreeFont(FSSB12);
            y = myScreen.fontHeight();
            myScreen.drawString("WARNING", 160, y, GFXFF);
            y += myScreen.fontHeight();

            myScreen.setFreeFont(FSS9);
            y += myScreen.fontHeight();
            myScreen.drawString("This project is for EDUCATIONAL", 160, y, GFXFF);
            y += myScreen.fontHeight();
            myScreen.drawString("PURPOSE ONLY and SHALL NOT", 160, y, GFXFF);
            y += myScreen.fontHeight();
            myScreen.drawString("be used for effective fever detection.", 160, y, GFXFF);
            y += myScreen.fontHeight();
            y += myScreen.fontHeight();
            myScreen.drawString("Contact your local health authority ", 160, y, GFXFF);
            y += myScreen.fontHeight();
            myScreen.drawString("for appropriate tools and procedures.", 160, y, GFXFF);
            break;

        case screenResult:

            myScreen.fillCircle(160, 120, 40, 0x07E0);

            myScreen.setTextDatum(MC_DATUM);
            myScreen.setTextColor(0x0000, 0x07E0);
            myScreen.setFreeFont(FSSB24);
            myScreen.drawString("+", 160, 110, GFXFF);
            myScreen.setFreeFont(FSSB12);
            myScreen.setTextColor(0xffff, 0x0000);
            myScreen.drawString("No fever detected", 160, 40, GFXFF);

            myScreen.setFreeFont(FSS9);
            myScreen.drawString("Please proceed to exit.", 160, 200, GFXFF);
            break;

        case screenQuestion:

            myScreen.fillCircle(160, 120, 40, 0xFDC0);

            myScreen.setTextDatum(MC_DATUM);
            myScreen.setFreeFont(FSSB24);
            myScreen.setTextColor(0x0000, 0xFDC0);
            myScreen.drawString("X", 160, 120, GFXFF);
            myScreen.setFreeFont(FSSB12);
            myScreen.setTextColor(0xffff, 0x0000);
            myScreen.drawString("No conclusion", 160, 40, GFXFF);

            myScreen.setFreeFont(FSS9);
            y = 180;
            myScreen.drawString("Move your hand in front of the camera", 160, y, GFXFF);
            y += myScreen.fontHeight();
            myScreen.drawString("<< to the left for a new picture", 160, y, GFXFF);
            y += myScreen.fontHeight();
            myScreen.drawString(">> to the right for assistance", 160, y, GFXFF);
            break;

        case screenAssistance:

            myScreen.fillCircle(160, 120, 40, 0x001F); // blue

            myScreen.setTextDatum(MC_DATUM);
            myScreen.setFreeFont(FSSB24);
            myScreen.setTextColor(0xffff, 0x001F);
            myScreen.drawString("i", 160, 120, GFXFF);
            myScreen.setFreeFont(FSSB12);
            myScreen.setTextColor(0xffff, 0x0000);
            myScreen.drawString("Thank you!", 160, 40, GFXFF);

            myScreen.setFreeFont(FSS9);
            myScreen.drawString("Please wait for assistance.", 160, 200, GFXFF);
            break;

        case screenPositive:

            myScreen.fillCircle(160, 120, 40, 0xF800); // blue

            myScreen.setTextDatum(MC_DATUM);
            myScreen.setFreeFont(FSSB24);
            myScreen.setTextColor(0xffff, 0xF800);
            myScreen.drawString("X", 160, 120, GFXFF);
            myScreen.setFreeFont(FSSB12);
            myScreen.setTextColor(0xffff, 0x0000);
            myScreen.drawString("Some fever detected", 160, 40, GFXFF);

            myScreen.setFreeFont(FSS9);
            y = 180;
            myScreen.drawString("Move your hand in front of the camera", 160, y, GFXFF);
            y += myScreen.fontHeight();
            myScreen.drawString("<< to the left for a new picture", 160, y, GFXFF);
            y += myScreen.fontHeight();
            myScreen.drawString(">> to the right for assistance", 160, y, GFXFF);
            break;

        default:

            break;
    }
}

enum eQuestion
{
    answerQuestionLeft,
    answerQuestionRight,
    answerPositiveLeft,
    answerPositiveRight,
    screenQuestionRepeat,
};

///
/// @brief    Display answer
/// @param    number answer number, as per eScreen
///
// Arduino doesn't accept enum as parameter
//void displayAnswer(eQuestion number)
void displayAnswer(uint8_t number)
{
    switch (number)
    {
        case answerQuestionLeft:

            myScreen.setTextDatum(MC_DATUM);
            myScreen.setFreeFont(FSSB24);
            myScreen.setTextColor(0x0000, 0xFDC0);
            myScreen.drawString("<<", 160, 110, GFXFF);

            break;

        case answerQuestionRight:

            myScreen.setTextDatum(MC_DATUM);
            myScreen.setFreeFont(FSSB24);
            myScreen.setTextColor(0x0000, 0xFDC0);
            myScreen.drawString(">>", 160, 110, GFXFF);

            break;

        case answerPositiveLeft:

            myScreen.setTextDatum(MC_DATUM);
            myScreen.setFreeFont(FSSB24);
            myScreen.setTextColor(0xFFFF, 0xF800);
            myScreen.drawString("<<", 160, 110, GFXFF);

            break;

        case answerPositiveRight:

            myScreen.setTextDatum(MC_DATUM);
            myScreen.setFreeFont(FSSB24);
            myScreen.setTextColor(0xFFFF, 0xF800);
            myScreen.drawString(">>", 160, 110, GFXFF);

            break;

        case  screenQuestionRepeat:

            myScreen.setTextDatum(MC_DATUM);
            myScreen.setFreeFont(FSSB12);
            myScreen.fillRect(0, 0, 320, 80, 0x0000);
            myScreen.setTextColor(0xFFFF, 0x0000);
            myScreen.drawString("Please repeat", 160, 40, GFXFF);

            break;

        default:

            break;
    }
}

#if (OPTION_WIFI == 1)
///
/// @brief    MQTT call-back function
/// @param    topic topic
/// @param    payload payload
/// @param    length length
///
void callbackMQTT(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Received: ");
    Serial.print(topic);
    Serial.print(" / ");
    char bufferMQTT[length];
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
        bufferMQTT[i] = (char)payload[i];
    }
    Serial.println();
    bufferMQTT[length] = '\0';
    String stringMQTT = String(bufferMQTT);
    /*
        if (String(stringMQTT) == "image")
        {
        commandMQTT = true;
        Serial.println("***");
        }
        Serial.print("'");
        Serial.print(stringMQTT);
        Serial.print("' ");
        Serial.println(commandMQTT);
    */
    Serial.print("***");
    Serial.print(" '");
    Serial.print(stringMQTT);
    Serial.print("' ");
    Serial.println(commandMQTT);
}

///
/// @brief    Reconnect to MQTT if disconnected
///
void reconnectMQTT()
{
    while (!myMQTT.connected())
    {
        Serial.print("myMQTT.connect... ");
        // Create a random client ID
        // Attempt to connect
        if (myMQTT.connect(clientID))
        {
            Serial.println("done");
            // Once connected, publish an announcement...
            // myMQTT.publish("ThermalOut", "hello world");
            // ... and resubscribe
            myMQTT.subscribe("Thermal/Command");
        }
        else
        {
            Serial.print(".");
            Serial.print(myMQTT.state());
            delay(1000);
        }
    }
}

///
/// @brief    Publish a message
/// @param    topic topic
/// @param    payload message, < 100 characters
///
void publishMQTT(String topic, String payload)
{
    reconnectMQTT();
    Serial.print("MQTT " + topic + ": ");
    Serial.println(myMQTT.publish(topic.c_str(), payload.c_str()) ? "ok" : "nok");
}
#endif

// Functions
void doInitialisation()
{
    uint8_t countWhile;
    uint16_t y = 0;

    Serial.begin(115200);
    if (doFlag)
    {
        Serial.println("--- doInitialisation");
    }
    doFlag = false;

    Serial.print("myScreen.begin...");
    myScreen.begin();
    myScreen.setRotation(3);
    myScreen.fillScreen(0x0000);
    Serial.println(" done");

    myScreen.setTextColor(0xffff, 0x0000);
    myScreen.setFreeFont(FSSB12);
    myScreen.drawString("Initialisation...", 0, y, GFXFF);
    myScreen.setFreeFont(FSS9);
    y += myScreen.fontHeight();
    myScreen.drawString("myScreen.begin... done", 0, y, GFXFF);

    y += myScreen.fontHeight();
    myScreen.drawString("PIR.begin... ", 0, y, GFXFF);
    pinMode(myPIR, INPUT);
    pinMode(myButtonC, INPUT_PULLUP);
    myScreen.drawString("PIR.begin... done", 0, y, GFXFF);

    y += myScreen.fontHeight();
    myScreen.drawString("Wire.begin... ", 0, y, GFXFF);
    Serial.print("Wire.begin...");
    Wire.begin();
    myScreen.drawString("PIR.begin... ", 0, y, GFXFF);
    Serial.println(" Wire");

    y += myScreen.fontHeight();
    myScreen.drawString("myAMG8833.init... ", 0, y, GFXFF);
    Serial.print("myAMG8833.init...");
    countWhile = 4;
    while ((myAMG8833.init() != NO_ERROR) and (countWhile > 0))
    {
        Serial.print(countWhile);
        delay(1000);
        countWhile--;
    }
    if (countWhile == 0)
    {
        Serial.println(" ERROR");
        myScreen.drawString("myAMG8833.init... ERROR", 0, y, GFXFF);
        myScreen.setFreeFont(FSSB12);
        myScreen.drawString("Initialisation... ERROR", 0, 0, GFXFF);
        while (1);
    }
    myScreen.drawString("myAMG8833.init... done", 0, y, GFXFF);
    Serial.println(" done");

    y += myScreen.fontHeight();
    myScreen.drawString("myPAJ7620.init... ", 0, y, GFXFF);
    Serial.print("myPAJ7620.init...");
    countWhile = 4;
    while ((myPAJ7620.begin() != false) and (countWhile > 0))
    {
        Serial.print(countWhile);
        delay(1000);
        countWhile--;
    }
    if (countWhile == 0)
    {
        myScreen.drawString("myPAJ7620.init... ERROR", 0, y, GFXFF);
        Serial.println(" ERROR");
        myScreen.setFreeFont(FSSB12);
        myScreen.drawString("Initialisation... ERROR", 0, 0, GFXFF);
        while (1);
    }
    myScreen.drawString("myPAJ7620.init... done", 0, y, GFXFF);
    Serial.println(" done");

#if (OPTION_WIFI == 1)
    y += myScreen.fontHeight();
    String text = "SSID: " + String(ssidWiFi);
    myScreen.drawString(text.c_str(), 0, y, GFXFF);
    Serial.println(text);

    y += myScreen.fontHeight();
    myScreen.drawString("WiFi.begin... ", 0, y, GFXFF);
    Serial.print("WiFi.begin...");
    WiFi.begin(ssidWiFi, passwordWiFi);
    countWhile = 64;
    while ((WiFi.status() != WL_CONNECTED) and (countWhile > 0))
    {
        delay(500);
        Serial.print(".");
        countWhile--;
    }
    if (countWhile == 0)
    {
        myScreen.drawString("WiFi.begin... ERROR", 0, y, GFXFF);
        Serial.println(" ERROR");
        myScreen.setFreeFont(FSSB12);
        myScreen.drawString("Initialisation... ERROR", 0, 0, GFXFF);
        while (1);
    }
    myScreen.drawString("WiFi.begin... connected", 0, y, GFXFF);
    Serial.println(" connected");

    // WiFi.ping() does not seem to be implemented
    // uint8_t result = WiFi.ping(brokerIP);
    
    y += myScreen.fontHeight();
    text = "IP: " + WiFi.localIP().toString();
    myScreen.drawString(text.c_str(), 0, y, GFXFF);
    Serial.println(text);

    y += myScreen.fontHeight();
    myScreen.drawString("myMQTT.setServer... ", 0, y, GFXFF);
    Serial.print("myMQTT.setServer...");
    myMQTT.setServer(brokerIP, 1883); // Connect the MQTT Server
    myMQTT.setCallback(callbackMQTT);
    myScreen.drawString("myMQTT.setServer... done", 0, y, GFXFF);
    Serial.println(" done");
#else
    y += myScreen.fontHeight();
    myScreen.drawString("WiFi and MQTT desactivated", 0, y, GFXFF);
#endif

    myScreen.setFreeFont(FSSB12);
    myScreen.drawString("Initialisation... done", 0, 0, GFXFF);
    wait(3);
    pinMode(LCD_BACKLIGHT, OUTPUT);
    digitalWrite(LCD_BACKLIGHT, LOW);

    oldTransition = transition;
    transition = TR_initialisation;
}

void doSleep()
{
    if (doFlag)
    {
        Serial.println("--- doSleep");
    }
    doFlag = false;

    displayScreen(screenNone);
    digitalWrite(LCD_BACKLIGHT, LOW);

    oldTransition = transition;
    transition = TR_pir;
}

void doWakeup()
{
    if (doFlag)
    {
        Serial.println("--- doWakeup");
    }
    doFlag = false;
    if ((digitalRead(myPIR) == HIGH) || (digitalRead(myButtonC)  == LOW))
    {
        digitalWrite(LCD_BACKLIGHT, HIGH);

        displayScreen(screenWarning);

        wait(5);

        transition = TR_picture;
        myScreen.fillScreen(0x0000);
        timeStart = millis();
        Serial.println();
        Serial.println("=== New case");
    }
}

void doPicture()
{
    if (doFlag)
    {
        Serial.println("--- doPicture");
    }
    doFlag = false;

    status = statusNone;
    myScreen.fillScreen(0x0000);

    // 20 colours numbered 0..19
    uint8_t maxIndex = sizeof(RGB565) / sizeof(RGB565[0]) - 1;

    // Absolute range
    const float minC = 33.0; // 19 or T
    const float maxC = 42.5; // 0 or A
    const float thresholdC = 37.3; // as per WHO

    uint8_t countWhile = 4;
    uint8_t count37 = 0;
    uint8_t count30 = 0;

    // Scale
    for (uint8_t i = 0; i < 18; i++)
    {
        myScreen.fillRect(270, 20 + 10 * i, 20, 10, RGB565[i]);
    }
    myScreen.drawRect(270, 20 + 18 * 10, 20, 10, 0x7BEF);

    // Local range
    float minL = 99.9;
    float maxL = -9.9;

    while ((countWhile > 0) and (count30 < 4))
    {
        Serial.print("countWhile= ");
        Serial.println(countWhile);

        count37 = 0;
        count30 = 0;

        // Thermal picture acquisition
        myAMG8833.read_pixel_temperature(thermalPicture);

        // Local range
        minL = 99.9;
        maxL = -9.9;
        for (int i = 0; i < PIXEL_NUM; i++)
        {
            float v = thermalPicture[i];
            minL = min(minL, v);
            maxL = max(maxL, v);
        }
        Serial.print("minL= ");
        Serial.println(minL);
        Serial.print("maxL= ");
        Serial.println(maxL);

        // Display local range scale
        myScreen.setFreeFont(FSS9);
        myScreen.setTextDatum(TC_DATUM);
        myScreen.drawString(String(maxL), 280, 0, GFXFF);

        myScreen.setFreeFont(FSS9);
        myScreen.setTextDatum(TC_DATUM);
        myScreen.drawString(String(minL), 280, 240, GFXFF);

        thermalCheckString = "";
        thermalLocalString = "";
        for (uint8_t i = 0; i < 8; i++)
        {
            for (uint8_t j = 0; j < 8; j++)
            {
                float valueC = thermalPicture[(i * 8) + j];
                float valueL = valueC;
                count37 += (valueC > thresholdC) ? 1 : 0;
                count30 += (valueC > 30.0) ? 1 : 0;

                // Absolute values
                valueC = max(minC, valueC);
                valueC = min(maxC, valueC);
                uint8_t indexC = maxIndex - (uint8_t)((valueC - minC) / (maxC - minC) * maxIndex);

                // Local display
                valueL = max(minL, valueL);
                valueL = min(maxL, valueL);
                uint8_t indexL = maxIndex - (uint8_t)((valueL - minL) / (maxL - minL) * maxIndex);

                myScreen.fillRect(i * 30, j * 30, 30, 30, RGB565[indexL]);
                thermalCheckString += char('A' + indexC);
                thermalLocalString += char('A' + indexL);
            }

            // for (uint8_t i = 0; i < 8; i++)
            // {
            //   for (uint8_t j = 0; j < 8; j++)
            //   {
            //     myScreen.fillRect(i * 30, j * 30, 30, 30, RGB565[thermalLocalString[(i * 8) + j] - 'A']);
            //   }
            // }
            // Serial.println(" ");
        }
        countWhile--;

#if (INTERPOLATION > 8)
        interpolate_image(thermalPicture, 8, 8, interpolatedPicture, INTERPOLATION, INTERPOLATION);
        uint8_t square = 240 / INTERPOLATION;
        
        Serial.print("square= ");
        Serial.println(square);
        
        for (uint8_t i = 0; i < INTERPOLATION; i++)
        {
            for (uint8_t j = 0; j < INTERPOLATION; j++)
            {
                float valueL = interpolatedPicture[(i * INTERPOLATION) + j];
                // Local display
                valueL = max(minL, valueL);
                valueL = min(maxL, valueL);
                uint8_t indexL = maxIndex - (uint8_t)((valueL - minL) / (maxL - minL) * maxIndex);
                myScreen.fillRect(i * square, j * square, square, square, RGB565[indexL]);
            }
        }
#endif

        
        Serial.print("thermalCheckString= ");
        Serial.println(thermalCheckString);
        Serial.print("Local maximum (oC)= ");
        Serial.println(maxL);
        Serial.print("count30 (>30.0oC)= ");
        Serial.println(count30);
        Serial.print("count37 (>37.3oC)= ");
        Serial.println(count37);

        if (count30 < 4)
        {
            delay(500);
        }
    }

    String result = "Inconclusive"; // default
    oldTransition = transition;
    if (countWhile == 0)
    {
        transition = TR_orange;
        status = statusInconclusive;
    }

    if (count30 > 3)
    {
        if (count37 > 2)
        {
            transition = TR_red;
            result = "Positive";
            status = statusPositive;
        }
        else
        {
            transition = TR_green;
            result = "Negative";
            status = statusNegative;
        }
    }
    /*
        // Forced value for debugging
        transition = TR_red;
        result = "Positive";
        status = statusPositive;
    */
#if (OPTION_WIFI == 1)
    publishMQTT("Thermal/Check", thermalCheckString);
    publishMQTT("Thermal/MinC", String(minC));
    publishMQTT("Thermal/MaxC", String(maxC));
    publishMQTT("Thermal/Local", thermalLocalString);
    publishMQTT("Thermal/MinL", String(minL));
    publishMQTT("Thermal/MaxL", String(maxL));
    publishMQTT("Thermal/Result", result);
#endif
}

void doQuestion()
{
    if (doFlag)
    {
        Serial.println("--- doQuestion");
    }
    doFlag = false;
    Serial.print("status= ");
    Serial.println(status);

    displayScreen((status == statusPositive) ? screenPositive : screenQuestion);
    // Move your hand in front of the camera
    // . to the left to take a new picture
    // . to the right to call for assistance

    uint8_t countWhile = 4;
    bool flag = true;
    gesture = GES_NONE_FLAG;

    while ((countWhile > 0)  and (gesture == GES_NONE_FLAG))
    {
        Serial.print("countWhile= ");
        Serial.print(countWhile);
        Serial.print("\t");

        uint64_t timeWhile = millis() + 15000; // 15s
        while ((millis() < timeWhile) and (gesture == GES_NONE_FLAG))
        {
            Serial.print(".");
            if (myPAJ7620.get() == false) // success
            {
                gesture = myPAJ7620.gesture();

                if ((gesture != GES_RIGHT_FLAG) and (gesture != GES_LEFT_FLAG))
                {
                    gesture = GES_NONE_FLAG;
                }
            }
            delay(100);
        }
        Serial.println(gesture);
        countWhile--;
        Serial.println();

        if ((flag == true) and (gesture == GES_NONE_FLAG))
        {
            flag = false;
            displayAnswer(screenQuestionRepeat);
            delay(500);
        }
    }

    String result = "";
    String action = "";
    if (gesture == GES_LEFT_FLAG)
    {
        action = "New acquisition";
        if (status == statusPositive)
        {
            displayAnswer(answerPositiveLeft);
            result = "Positive";

        }
        else
        {
            displayAnswer(answerQuestionLeft);
            result = "Inconclusive";
        }

        transition = TR_left;
    }
    else // case GES_RIGHT_FLAG:
    {
        action = "Call for assistance";
        if (status == statusPositive)
        {
            displayAnswer(answerPositiveRight);
            result = "Positive";

        }
        else
        {
            displayAnswer(answerQuestionRight);
            result = "Inconclusive";
        }

        transition = TR_right;
    }

#if (OPTION_WIFI == 1)
    publishMQTT("Thermal/Result", result);
    publishMQTT("Thermal/Action", action);
#endif

    wait(2);
}

void doResult()
{
    if (doFlag)
    {
        Serial.println("--- doResult");
    }
    doFlag = false;

#if (OPTION_WIFI == 1)
    publishMQTT("Thermal/Action", "Exit");
#endif
    delay(1000); // give time to see the image
    displayScreen(screenResult);

    wait(10);

    oldTransition = transition;
    transition = TR_wait;

    Serial.print("=== Case closed at (ms) ");
    Serial.println((uint32_t)(millis() - timeStart));
}

void doAssistance()
{
    if (doFlag)
    {
        Serial.println("--- doAssistance");
    }
    doFlag = false;

    switch (transition)
    {
        case TR_red:

            displayScreen(screenPositive);
            break;

        default:

            displayScreen(screenAssistance);
            break;
    }
    wait(10);

    oldTransition = transition;
    transition = TR_call;

    Serial.print("=== Case closed at (ms) ");
    Serial.println((uint32_t)(millis() - timeStart));
}


// Add setup code
void setup()
{
    delay(2000);

    state = ST_INITIALISATION;
    oldState = ST_NONE;
    transition = TR_none;
    oldTransition = TR_none;
}

// Add loop code
void loop()
{
    if (state != ST_INITIALISATION)
    {
#if (OPTION_WIFI == 1)
        reconnectMQTT();
        myMQTT.loop();
#endif
    }
    if (oldTransition != transition)
    {
#if (OPTION_WIFI == 1)
        myMQTT.publish("Thermal/State", String(state).c_str());
        myMQTT.publish("Thermal/Transition", String(transition).c_str());
#endif
        Serial.print((uint32_t)(millis() - timeStart));
        Serial.print("\t");
        Serial.print("Transition= ");
        Serial.print(transition);
        Serial.print(" (");
        Serial.print(oldTransition);
        Serial.println(")");
        transition = oldTransition;
    }

    // FSM main loop
    oldState = state;
    switch (state)
    {
        case ST_INITIALISATION:

            doInitialisation();
            if (transition == TR_initialisation)
            {
                oldState = state;
                state = ST_SLEEP;
            }
            break;

        case ST_SLEEP:

            doSleep();
            if (transition == TR_pir)
            {
                oldState = state;
                state = ST_WAKEUP;
            }
            break;

        case ST_WAKEUP:

            doWakeup();
            if (transition == TR_picture)
            {
                oldState = state;
                state = ST_PICTURE;
            }
            break;

        case ST_PICTURE:

            doPicture();
            if (transition == TR_green)
            {
                oldState = state;
                state = ST_RESULT;
            }
            if ((transition == TR_red) or (transition == TR_orange))
            {
                oldState = state;
                state = ST_QUESTION;
            }
            break;

        case ST_RESULT:

            doResult();
            if (transition == TR_wait)
            {
                oldState = state;
                state = ST_SLEEP;
            }
            break;

        case ST_QUESTION:

            doQuestion();
            if (transition == TR_left)
            {
                oldState = state;
                state = ST_PICTURE;
            }
            if (transition == TR_right)
            {
                oldState = state;
                state = ST_ASSISTANCE;
            }
            break;

        case ST_ASSISTANCE:

            doAssistance();
            if (transition == TR_call)
            {
                oldState = state;
                state = ST_SLEEP;
            }
            break;

        default:

            break;
    }
    if (state != oldState)
    {
        doFlag = true;
    }
}
