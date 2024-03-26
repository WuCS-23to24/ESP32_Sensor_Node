#include <Arduino.h>
#include <Wire.h>

#include "SparkFunTMP102.h"
#include "SparkFun_u-blox_GNSS_v3.h"
#include "auxiliary.h"
#include "bluetooth.hpp"
#include "hardware_timer.hpp"

uuids UUID_generator;
TMP102 sensor;
Bluetooth<uuids> bluetooth;
SFE_UBLOX_GNSS myGNSS;

volatile bool gps_enabled = false;
volatile bool tmp_enabled = false;

volatile SemaphoreHandle_t gps_semaphore;
volatile SemaphoreHandle_t temp_semaphore;
volatile SemaphoreHandle_t ble_send_semaphore;

volatile int8_t GPS_ISR = 0;
volatile int8_t BLE_SEND_ISR = 0;

portMUX_TYPE isr_mux = portMUX_INITIALIZER_UNLOCKED;

void ARDUINO_ISR_ATTR set_semaphore()
{
    taskENTER_CRITICAL_ISR(&isr_mux);
    if (tmp_enabled)
        xSemaphoreGiveFromISR(temp_semaphore, NULL);

    if (GPS_ISR == 2)
    {
        if (gps_enabled)
            xSemaphoreGiveFromISR(gps_semaphore, NULL);
        GPS_ISR = 0;
    }
    else
    {
        GPS_ISR++;
    }

    if (BLE_SEND_ISR == 4)
    {
        if (deviceConnected)
            xSemaphoreGiveFromISR(ble_send_semaphore, NULL);
        BLE_SEND_ISR = 0;
    }
    else
    {
        BLE_SEND_ISR++;
    }
    taskEXIT_CRITICAL_ISR(&isr_mux);
}

void setup()
{
    Wire.begin();
    Serial.begin(115200);

    if (!sensor.begin())
        Serial.println("Could not connect to TMP102, sensor may not be connected.");
    else
    {
        tmp_enabled = true;
        sensor.setFault(0);
        sensor.setAlertPolarity(1);
        sensor.setAlertMode(0);
        sensor.setConversionRate(3);
        sensor.setExtendedMode(0);
        sensor.setHighTempF(120.0);
        sensor.setLowTempF(50.0);
    }

    if (!myGNSS.begin()) // Connect to the u-blox module using Wire port
    {
        Serial.println("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing.");
    }
    else
    {
        gps_enabled = true;
        Serial.println("u-blox GNSS detected");
        myGNSS.setPacketCfgPayloadSize(UBX_NAV_SAT_MAX_LEN);

        myGNSS.setI2COutput(COM_TYPE_UBX);
        myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT);
    }

    UUID_generator.initialize_random_values();
    UUID_generator.generate_hashes();
    Serial.printf("SERVICE UUID - %s\n", UUID_generator.get_service_uuid());
    Serial.printf("SERVICE UUID - %s\n", UUID_generator.get_characteristic_uuid());

    bluetooth = Bluetooth<uuids>(UUID_generator);

    gps_semaphore = xSemaphoreCreateBinary();
    temp_semaphore = xSemaphoreCreateBinary();
    ble_send_semaphore = xSemaphoreCreateBinary();
    setup_timer(set_semaphore);
}

void loop()
{
    if (xSemaphoreTake(gps_semaphore, 0) == pdTRUE)
    {
        if (myGNSS.getPVT() == true)
        {
            auto data = bluetooth.callback_class->getData();
            data.latitude = (float)(myGNSS.getLatitude()) * 1e-7;
            data.longitude = (float)(myGNSS.getLongitude()) * 1e-7;
            data.altitude = (myGNSS.getAltitudeMSL()) / 1000.0f;
            bluetooth.callback_class->setData(data);

            Serial.printf("Lat: %10g", data.latitude);
            Serial.printf(" Long: %10g degrees", data.longitude);
            Serial.printf(" Alt: %10g m", data.altitude);
            Serial.println();
        }
    }
    if (xSemaphoreTake(temp_semaphore, 0) == pdTRUE)
    {
        auto data = bluetooth.callback_class->getData();
        data.temp_data = sensor.readTempF();
        bluetooth.callback_class->setData(data);

        Serial.printf("TEMP: %10g\n", data.temp_data);
    }
    if (xSemaphoreTake(ble_send_semaphore, 0) == pdTRUE && deviceConnected)
    {
        bluetooth.sendData();
    }
}
