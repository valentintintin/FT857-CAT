#include "FT857.h"
FT857 radio;

void setup() {
    Serial.begin(115200);
    radio.begin();

    Serial.println(F("Ready !"));
}

void loop() {
    Serial.print(F("VFO: ")); Serial.print(radio.getVfo()); Serial.print(F(" "));

    Serial.print(radio.getFrequency()); Serial.print(F(" "));

    String mode = "UNK";

    switch (radio.getMode()) {
        case 0xFC:
            mode = "PKT";
            break;

        case CAT_MODE_LSB:
            mode = "LSB";
            break;

        case CAT_MODE_USB:
            mode = "USB";
            break;

        case CAT_MODE_CW:
            mode = "CW ";
            break;

        case CAT_MODE_FM:
            mode = "FM ";
            break;

        case CAT_MODE_WFM:
            mode = "WFM";
            break;

        case CAT_MODE_CWR:
            mode = "CWR";
            break;

        case CAT_MODE_AM:
            mode = "AM ";
            break;

        case CAT_MODE_FMN:
            mode = "FMN";
            break;

        case CAT_MODE_DIG:
            mode = "DIG";
            break;
    }

    Serial.print(mode); Serial.print(F(" "));

    byte sMeterValue = radio.getSMeter();
    String sMeter = "S";

    if (sMeterValue < 10) {
        sMeter += String(sMeterValue);
    }
    else {
        switch (sMeterValue) {
            case 10:
                sMeter += "9+10";
                break;

            case 11:
                sMeter += "9+20";
                break;

            case 12:
                sMeter += "9+30";
                break;

            case 13:
                sMeter += "9+40";
                break;

            case 14:
                sMeter += "9+50";
                break;

            case 15:
                sMeter += "9+60";
                break;

            default:
                sMeter += "-UNK";
                break;
        }
    }

    Serial.print(sMeter); Serial.print(F(" "));

    Serial.print(F("PTT: ")); Serial.println(radio.isTx());

    delay(300);
}