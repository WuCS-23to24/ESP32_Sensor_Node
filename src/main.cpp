#include <Arduino.h>
#include <Wire.h>

#include "SparkFunTMP102.h"
#include "auxiliary.h"
#include "bluetooth.hpp"

uuids UUID_generator;
TMP102 sensor;
Bluetooth<TMP102, uuids> bluetooth;

void setup()
{
    Wire.begin();
    Serial.begin(115200);

    if (!sensor.begin())
        Serial.println("Could not connect to TMP102, sensor may not be connected.");
    else
    {
        sensor.setFault(0);
        sensor.setAlertPolarity(1);
        sensor.setAlertMode(0);
        sensor.setConversionRate(3);
        sensor.setExtendedMode(0);
        sensor.setHighTempF(120.0);
        sensor.setLowTempF(50.0);
    }

    UUID_generator.initialize_random_values();
    UUID_generator.generate_hashes();
    Serial.printf("SERVICE UUID - %s\n", UUID_generator.get_service_uuid());
    Serial.printf("SERVICE UUID - %s\n", UUID_generator.get_characteristic_uuid());

    bluetooth = Bluetooth<TMP102, uuids>(UUID_generator, sensor);
}

void loop()
{
    // put your main code here, to run repeatedly:
    delay(2000);
}
