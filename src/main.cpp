#include <Arduino.h>
#include <avr/wdt.h>

#include "table.h"

constexpr uint8_t ledPin = 13;

void setup() {
    wdt_disable();
    wdt_enable(WDTO_8S);

    digitalWrite(ledPin, LOW);
    pinMode(ledPin, OUTPUT);

    Serial.begin(2000000);
    while (!Serial);
}

void loop() {
    wdt_reset();
}
