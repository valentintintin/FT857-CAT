/*
  FT857D.h	Arduino library for controlling a Yaesu FT857D
			radio via CAT commands.

 Version:  0.1
 Created:  2012.08.16
Released:  2012.08.17
  Author:  James Buck, VE3BUX
     Web:  http://www.ve3bux.com

Version:  1.04
Released:  2020.04.19
  Author:  Philippe Lonc, F6CZV
From previous version 1.03
- Comments on Read Rx status and Tx status were rewritten (only one byte status),
- get SMeter() function is now a String function (was a char*)

 Version:  1.1.0
 Created:  2020.04.08
Released:  2020.04.08
  Author:  Valentin Saugnier, F4HVV

LIMITATION OF LIABILITY :
 This source code is provided "as-is". It may contain bugs. 
Any damages resulting from its use is done under the sole responsibility of the user/developper
 and beyond my responsibility.
*/


#ifndef FT857_CAT_h
#define FT857_CAT_h

#include <Arduino.h>

#if defined(ESP8266)
    #error this library was not designed for ESP8266
#elif defined(ESP32)
    #include <HardwareSerial.h>
#else
    #include <SoftwareSerial.h>
#endif

#include "FT857_defines.h"

class FT857
{
public:
    FT857();
#if defined(ESP32)
    // https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.cpp
    void begin(int uartNr = 2, int8_t rx = -1, int8_t tx = -1, unsigned long baud = 38400);
#else
    void begin(byte rx = 10, byte tx = 11, unsigned long baud = 38400);
#endif
    void flushRX();

    // Setters
    void setFrequency(unsigned long freq);
    void setMode(byte mode);
    void setClarifierState(boolean toggle);
    void setSplitState(boolean toggle);
    void setRepeaterOffset(byte offset);
    void setRepeaterOffsetFrequency(unsigned long freq);
    void setCtcssDcsSquelchMode(byte mode);
    void setCtcssDcsSquelchFrequency(unsigned int, bool isCtcss);

    // Getters
    byte getMode(); // modified by F6CZV
    char getVfo(); // new function F6CZV
    unsigned long getFrequency();
    bool isTx(); // was boolean F6CZV
    byte getSMeter(); // new function F6CZV
    void getCwMeterConf(byte &MTR, bool &KYR, bool &BK); // new function F6CZV
    void getAgcDspConf(bool &AGC, bool &DBF, bool &DNR, bool &DNF); // new function F6CZV
    bool getSplitState(); // new function F6CZV

    // Actions
    void lock(boolean toggle);
    void setPtt(boolean toggle);
    void switchVfo();

private:
#if defined(ESP32)
    HardwareSerial* rigCat;
#else
    SoftwareSerial* rigCat;
#endif

    void sendCmd(byte cmd[], byte len);
    byte singleCmd(int cmd);		// simplifies small cmds
    byte getByte();

    void sendByte(byte cmd);
    unsigned long fromBcdBe(const byte bcd_data[], unsigned bcd_len);
    byte* toBcdBe( byte bcd_data[], unsigned long freq, unsigned bcd_len);
    void comError(char* string);
};

#endif
