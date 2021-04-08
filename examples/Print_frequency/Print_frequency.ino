#include "FT857D.h"
FT857D radio;

void setup() {
    Serial.println(115200);
    radio.begin();
}

void loop() {
    Serial.println(radio.getFreqMode());

    delay(100);
}