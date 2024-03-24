#include <Arduino.h>
#include <Wire.h>

#include "SparkFunTMP102.h"
#include "SparkFun_u-blox_GNSS_v3.h"
#include "auxiliary.h"
#include "bluetooth.hpp"
#include "hardware_timer.hpp"

uuids UUID_generator;
TMP102 sensor;
Bluetooth<TMP102, uuids> bluetooth;
SFE_UBLOX_GNSS myGNSS;

volatile SemaphoreHandle_t gps_semaphore;
volatile int32_t milliseconds_since_boot = 0;
portMUX_TYPE gps_isr_mux = portMUX_INITIALIZER_UNLOCKED;

// Time-to-Fix times in milliseconds
#define UBLOX_M10_COLD_START 23000
#define UBLOX_M10_AIDED_START 1000
#define UBLOX_M10_HOT_START 1000

void ARDUINO_ISR_ATTR set_gps_semaphore()
{
    taskENTER_CRITICAL_ISR(&gps_isr_mux);
	if (milliseconds_since_boot > 23000)
    milliseconds_since_boot += 5000;
    taskEXIT_CRITICAL_ISR(&gps_isr_mux);

    xSemaphoreGiveFromISR(gps_semaphore, NULL);
}

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

    if (!myGNSS.begin()) // Connect to the u-blox module using Wire port
    {
        Serial.println("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing.");
    }
    else
    {
        Serial.println("u-blox GNSS detected");
        // myGNSS.setPacketCfgPayloadSize(UBX_NAV_SAT_MAX_LEN);

        myGNSS.setI2COutput(COM_TYPE_UBX);
        // myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT);
        // myGNSS.setNavigationFrequency(1);
    }

    UUID_generator.initialize_random_values();
    UUID_generator.generate_hashes();
    Serial.printf("SERVICE UUID - %s\n", UUID_generator.get_service_uuid());
    Serial.printf("SERVICE UUID - %s\n", UUID_generator.get_characteristic_uuid());

    bluetooth = Bluetooth<TMP102, uuids>(UUID_generator, sensor);

    gps_semaphore = xSemaphoreCreateBinary();
    setup_timer(set_gps_semaphore);
}

void loop()
{
    if (xSemaphoreTake(gps_semaphore, 0) == pdTRUE)
    {
        if (myGNSS.getPVT() == true)
        {
            int32_t latitude = myGNSS.getLatitude();
            Serial.printf("Lat: %10g", (float)latitude * 1e-7);

            int32_t longitude = myGNSS.getLongitude();
            Serial.printf(" Long: %10g degrees", (float)longitude * 1e-7);

            int32_t altitude = myGNSS.getAltitudeMSL();
            Serial.printf(" Alt: %10g m", altitude / 1000.0f);

            Serial.println();
        }
    }
}
