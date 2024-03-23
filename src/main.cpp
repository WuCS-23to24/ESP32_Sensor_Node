/*
    Based on Neil Kolban example for IDF:
   https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp Ported to Arduino
   ESP32 by Evandro Copercini
*/
#define CONFIG_DISABLE_HAL_LOCKS true

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "SparkFunTMP102.h"
#include "UUID.h"

#include <bootloader_random.h>
#include <esp_random.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

UUID uuid2;
UUID uuid1;
TMP102 sensor;

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        if (value.length() > 0)
        {
            Serial.println("*********");
            Serial.print("New value: ");
            for (int i = 0; i < value.length(); i++)
                Serial.print(value[i]);

            Serial.println();
            Serial.println("*********");
        }
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        auto data = sensor.readTempF();
        pCharacteristic->setValue(data);
        Serial.printf("TEMP READ: %g\n", data);
    }
};

void setup()
{
    Serial.begin(115200);

    if (!sensor.begin(0x48, Wire))
    {
        Serial.println("Could not connect to TMP102, sensor may not be connected.");
    }
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

    bootloader_random_enable();
    uint32_t rand_val_1 = esp_random();
    uint32_t rand_val_2 = esp_random();

    uint32_t rand_val_3 = esp_random();
    uint32_t rand_val_4 = esp_random();

    uuid1.seed(rand_val_1, rand_val_2);
    uuid1.generate();
    auto *service_uuid = uuid1.toCharArray();

    uuid2.seed(rand_val_3, rand_val_4);
    uuid2.generate();
    auto *characteristic_uuid = uuid2.toCharArray();

    Serial.printf("SERVICE UUID - %s\n", service_uuid);
    Serial.printf("SERVICE UUID - %s\n", characteristic_uuid);

    BLEDevice::init("TEMP-0");

    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(service_uuid);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        characteristic_uuid, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Hello World");
    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}

void loop()
{
    // put your main code here, to run repeatedly:
    delay(2000);
}
