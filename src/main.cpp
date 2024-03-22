/*
    Based on Neil Kolban example for IDF:
   https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp Ported to Arduino
   ESP32 by Evandro Copercini
*/
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "UUID.h"

#include <bootloader_random.h>
#include <esp_random.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

UUID uuid2;
UUID uuid1;

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
};

void setup()
{
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

    Serial.begin(115200);

    Serial.printf("SERVICE UUID - %s\n", service_uuid);
    Serial.printf("SERVICE UUID - %s\n", characteristic_uuid);

    Serial.println("1- Download and install an BLE scanner app in your phone");
    Serial.println("2- Scan for BLE devices in the app");
    Serial.println("3- Connect to MyESP32");
    Serial.println("4- Go to CUSTOM CHARACTERISTIC in CUSTOM SERVICE and write something");
    Serial.println("5- See the magic =)");

    BLEDevice::init("MyESP32");
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
