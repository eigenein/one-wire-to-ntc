#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_MCP4725.h>

#include "table.h"

// DS18B20 pins.
constexpr uint8_t dallasDataPin = 2;
constexpr uint8_t dallasPullUpPin = 3;
constexpr uint8_t dallasVccPin = 4;

// MCP4725 DAC breakout board pins.
constexpr uint8_t dacVccPin = 21;
constexpr uint8_t dacGroundPin = 20;
constexpr uint8_t dacAddress0Pin = 17;
constexpr uint8_t dacOutputPin = 16;
constexpr uint8_t ledPin = 13;

// MCP4725 DAC I2C addressing.
constexpr uint8_t dacAddress0 = LOW;
constexpr uint8_t dacAddress = 0x60 | dacAddress0;

Adafruit_MCP4725 dac;
OneWire oneWire(dallasDataPin);
DallasTemperature sensors(&oneWire);
DeviceAddress dallasAddress;

uint16_t dacValue = 2048; // 0.5 Vcc

void setupLED() {
    digitalWrite(ledPin, LOW);
    pinMode(ledPin, OUTPUT);
}

void setupSerial() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Starting up.");
}

void setupDAC() {
    pinMode(dacGroundPin, OUTPUT);
    digitalWrite(dacGroundPin, LOW);

    pinMode(dacAddress0Pin, OUTPUT);
    digitalWrite(dacAddress0Pin, dacAddress0);

    // The pin is not used, however it is soldered to the DAC output.
    // Ensure it's in the high-impendance state.
    pinMode(dacOutputPin, INPUT);

    // Power up and start.
    pinMode(dacVccPin, OUTPUT);
    digitalWrite(dacVccPin, HIGH);
    dac.begin(dacAddress);
    dac.setVoltage(dacValue, true);
}

void printDallasAddress(DeviceAddress deviceAddress) {
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16) {
            Serial.print("0");
        }
        Serial.print(deviceAddress[i], HEX);
    }
}

void setupDallas() {
    // I'm NOT using an external pull-up resistor.
    // Instead, the data pin is connected to the pull up pin which is using the internal pull-up.
    pinMode(dallasPullUpPin, INPUT_PULLUP);

    // Power up and start.
    pinMode(dallasVccPin, OUTPUT);
    digitalWrite(dallasVccPin, HIGH);
    sensors.begin();

    // Locate the sensor.
    Serial.println("Locating the sensor.");
    while (!sensors.getAddress(dallasAddress, 0)) {
        Serial.println("Could not find a sensor.");
        delay(1000);
    }
    Serial.print("Found sensor: ");
    printDallasAddress(dallasAddress);
    Serial.println();

    sensors.setResolution(dallasAddress, 12, true);
}

void setup() {
    digitalWrite(ledPin, HIGH);

    setupSerial();
    setupDallas();
    setupDAC();
    setupLED();

    digitalWrite(ledPin, LOW);
    Serial.println("Successfully started up.");
}

float rawTemperatureToADC(const int16_t rawTemperature) {
    return pgm_read_float_near(&(TABLE[rawTemperature & 0xFFF]));
}

/*
 * Using my own implementation because I need the real raw temperature,
 * not the one provided by DallasTemperature.
 */
int16_t getRawTemperature() {
    uint8_t scratchPad[9];
    if (!sensors.isConnected(dallasAddress, scratchPad)) {
        // The power-on reset value.
        return 0x0550;
    }
    // FIXME: this doesn't work with DS18S20.
    return (((int16_t)scratchPad[1]) << 8) | ((int16_t)scratchPad[0]);
}

void loop() {
    sensors.requestTemperatures();
    const int16_t rawTemperature = getRawTemperature();
    if (rawTemperature != 0x0550) {
        dacValue = rawTemperatureToADC(rawTemperature);
        dac.setVoltage(dacValue, false);
    }

    Serial.print("raw: ");
    Serial.print(rawTemperature, HEX);
    Serial.print(" | ");
    Serial.print("temp C: ");
    Serial.print(rawTemperature / 16.0);
    Serial.print(" | ");
    Serial.print("DAC: ");
    Serial.print(dacValue);
    Serial.println();
}
