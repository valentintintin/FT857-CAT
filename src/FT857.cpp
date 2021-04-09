/*
  FT857.cpp	Arduino library for controlling a Yaesu FT857
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
- getSMeter is now a String function (was a char*). Code was modified accordingly
- unused function code in comments was removed

 Version:  1.1.0
 Created:  2020.04.08
Released:  2020.04.08
  Author:  Valentin Saugnier, F4HVV

LIMITATION OF LIABILITY :
 This source code is provided "as-is". It may contain bugs.
Any damages resulting from its use is done under the sole responsibility of the user/developper
 and beyond my responsibility.

*/

#include <Arduino.h>
#include "FT857.h"

FT857::FT857() {
}

#if defined(ESP32)
    void FT857::begin(int uartNr, int8_t rx, int8_t tx, unsigned long baud) {
        rigCat = new HardwareSerial(uartNr);
        rigCat->begin(baud, SERIAL_8N1, rx, tx);
    }
#else
    void FT857::begin(byte rx, byte tx, unsigned long baud) {
        rigCat = new SoftwareSerial(rx, tx);
        rigCat->begin(baud);
    }
#endif

// lock or unlock the radio
void FT857::lock(boolean toggle) {
    singleCmd(toggle ? CAT_LOCK_ON : CAT_LOCK_OFF);
}

// set or release the virtual PTT button
void FT857::setPtt(boolean toggle) {
    singleCmd(toggle ? CAT_PTT_ON : CAT_PTT_OFF);
}

// set radio frequency directly (as a long integer)
void FT857::setFrequency(unsigned long freq) {
    byte rigFreq[5] = {0, 0, 0, 0, 0};
    rigFreq[4] = CAT_FREQ_SET; // command byte

    byte tempWord[4];
    byte *converted = toBcdBe(tempWord, freq, 8);

    for (byte i = 0; i < 4; i++){
        rigFreq[i] = converted[i];
    }

    sendCmd(rigFreq,5);
    getByte();
}

// set radio mode
void FT857::setMode(byte mode) {
    byte rigMode[5] = {0, 0, 0, 0, 0};
    rigMode[4] = CAT_MODE_SET; // command byte
    rigMode[0] = mode;

    sendCmd(rigMode,5);
    getByte();
}

// turn the clarifier on or off
void FT857::setClarifierState(boolean toggle) {
    singleCmd(toggle ? CAT_CLAR_ON : CAT_CLAR_OFF);
}

// switch between VFO A and VFO B
void FT857::switchVfo() {
    singleCmd(CAT_VFO_AB);
}

// turn split operation on or off
void FT857::setSplitState(bool toggle) {
    singleCmd(toggle ? CAT_SPLIT_ON : CAT_SPLIT_OFF);
}

// control repeater offset direction
void FT857::setRepeaterOffset(byte offset) {
    byte rigOffset[5] = {0, 0, 0, 0, 0};
    rigOffset[4] = CAT_RPTR_OFFSET_CMD; // command byte
    rigOffset[0] = offset;

    sendCmd(rigOffset,5);
    getByte();
}

// set offset for repeater
void FT857::setRepeaterOffsetFrequency(unsigned long freq) {
    byte offsetFreq[5] = {0, 0, 0, 0, 0};
    offsetFreq[4] = CAT_RPTR_FREQ_SET; // command byte

    freq = (freq * 100); // convert the incoming value to kHz

    byte tempWord[4];
    byte *converted = toBcdBe(tempWord, freq, 8);

    for (byte i = 0; i < 4; i++){
        offsetFreq[i] = converted[i];
    }

    sendCmd(offsetFreq,5);
    getByte();
}

// enable or disable various CTCSS and DCS squelch options
void FT857::setCtcssDcsSquelchMode(byte mode) {
    byte rigSql[5] = {0, 0, 0, 0, 0};
    rigSql[4] = CAT_SQL_CMD; // command byte
    rigSql[0] = mode;

    sendCmd(rigSql,5);

    getByte();
}

// set tone (hertz) for CTCSS or DCS
void FT857::setCtcssDcsSquelchFrequency(unsigned int freq, bool isCtcss) {
    byte rigSqlFreq[5] = {0x00,0x00,0x00,0x00,0x00};
    rigSqlFreq[4] = isCtcss ? CAT_SQL_CTCSS_SET : CAT_SQL_DCS_SET;

    byte freq_bcd[5];
    toBcdBe(freq_bcd, (unsigned long) freq, 4);

    for (byte i = 0; i < 4; i++){
        rigSqlFreq[i] = freq_bcd[i];
    }

    sendCmd(rigSqlFreq,5);
    getByte();
}


// get the current mode
byte FT857::getMode() {
    singleCmd(CAT_RX_FREQ_CMD, false);

    for (int j = 0; j < 4; j++) {
        getByte();
    }

    byte reply = getByte();

    return reply == 0xFC ? MODE_PKT : reply;
}

// get the frequency
unsigned long FT857::getFrequency() {
    singleCmd(CAT_RX_FREQ_CMD, false);

    byte chars[4];

    for (int j = 0; j < 4; j++) {
        chars[j] = getByte();
    }

    getByte();

    return fromBcdBe(chars, 8);
}

// determine if the radio is in TX state
// unless the radio is actively TX, the result is always
// 0x255 so any value other than 0x255 means TX !
bool FT857::isTx() {                         // was boolean F6CZV
    return singleCmd(CAT_TX_DATA_CMD) == 255 ? false : true;
}

// get the S Meter value from the radio F6CZV
byte FT857::getSMeter() {
    return singleCmd(CAT_RX_DATA_CMD) & 0x0f;
}

// get the VFO status from the radio F6CZV
char FT857::getVfo() {
    byte rigTXState[5] = {0, 0, 0, 0, 0};
    rigTXState[4] = CAT_EEPROM_READ_CMD;
    rigTXState[0] = MSB_ADD_VFO_status;
    rigTXState[1] = LSB_ADD_VFO_status;
    char VFO[1];

    sendCmd(rigTXState, 5);

    VFO[0] = getByte() == 0x80 ? 'A' : 'B';

    getByte(); // byte discarded

    return VFO[0];
}

// get the MTR, Keyer and Break-In configurations from the radio F6CZV
void FT857::getCwMeterConf(byte &MTR, bool &KYR, bool &BK) {
    byte rigTXState[5] = {0, 0, 0, 0, 0};
    rigTXState[4] = CAT_EEPROM_READ_CMD;
    rigTXState[0] = MSB_ADD_CW_MTR_CONF;
    rigTXState[1] = LSB_ADD_CW_MTR_CONF;

    sendCmd(rigTXState, 5);

    byte reply = getByte();
    MTR = reply & 0x03;
    KYR = reply & 0x10;
    BK = reply & 0x20;

    getByte(); // byte discarded
}

// get the AGC, DBF, DNF and DNR configurations from the radio F6CZV
void FT857::getAgcDspConf(bool &AGC,bool &DBF,bool &DNR, bool &DNF) {
    byte rigTXState[5] = {0, 0, 0, 0, 0};
    rigTXState[4] = CAT_EEPROM_READ_CMD;
    rigTXState[0] = MSB_ADD_AGC_DSP_CONF;
    rigTXState[1] = LSB_ADD_AGC_DSP_CONF;

    sendCmd(rigTXState, 5);

    byte reply = getByte();
    AGC = reply & 0x20;
    DBF = reply & 0x04; // only one bit is tested
    DNR = reply & 0x02;
    DNF = reply & 0x01;

    getByte(); // byte discarded
}

// get the SPLIT status from the radio F6CZV
bool FT857::getSplitState() {
    byte rigTXState[5] = {0, 0, 0, 0, 0};
    bool status = false;
    rigTXState[4] = CAT_EEPROM_READ_CMD;
    rigTXState[0] = MSB_ADD_SPLIT_STATUS;
    rigTXState[1] = LSB_ADD_SPLIT_STATUS;

    sendCmd(rigTXState, 5);

    byte reply = getByte();
    status = reply & 0x80;

    getByte(); // byte discarded
    return status;
}

// spit out any DEBUG data via this function
void FT857::comError(char* string) {
    Serial.println("Communication Error!");
    Serial.println(string);
}

// gets a byte of input data from the radio
byte FT857::getByte() {
    unsigned long startTime = millis();
    while (rigCat->available() < 1 && millis() < startTime + 2000);
    return rigCat->read();
}

// this is the function which actually does the
// serial transaction to the radio
void FT857::sendCmd(byte cmd[], byte len) {
    for (byte i = 0; i < len; i++) {
        rigCat->write(cmd[i]);
    }

    // return getByte();	// should make this work more quickly
    // in a future update
}

// this function reduces total code-space by allowing for
// single byte commands to be issued (ie. all the toggles)
byte FT857::singleCmd(int cmd, bool getByteReturn) {
    byte outByte[5] = {0, 0, 0, 0, 0};
    outByte[4] = cmd;
    sendCmd(outByte, 5);
    return getByteReturn ? getByte() : 0;
}

// send a single byte of data (will be removed later)
void FT857::sendByte(byte cmd) {
    rigCat->write(cmd);
}

void FT857::flushRX() {
    rigCat->flush();
}

// GPL
// taken from hamlib work
unsigned long FT857::fromBcdBe(const byte bcdData[], unsigned bcdLen) {
    unsigned int i;
    long f = 0;

    for (i = 0; i < bcdLen / 2; i++) {
        f *= 10;
        f += bcdData[i] >> 4;
        f *= 10;
        f += bcdData[i] & 0x0f;
    }

    if (bcdLen & 1) {
        f *= 10;
        f += bcdData[bcdLen / 2] >> 4;
    }
    return f;
}

// GPL
// taken from hamlib work
byte* FT857::toBcdBe(byte bcdData[], unsigned long freq, unsigned bcdLen) {
    int i;
    byte a;

    if (bcdLen & 1) {
        bcdData[bcdLen / 2] &= 0x0f;
        bcdData[bcdLen / 2] |= (freq % 10) << 4;
/* NB: low nibble is left uncleared */
        freq /= 10;
    }

    for (i = (bcdLen / 2) - 1; i >= 0; i--) {
        a = freq % 10;
        freq /= 10;
        a |= (freq%10) << 4;
        freq /= 10;
        bcdData[i] = a;
    }

    return bcdData;
}
