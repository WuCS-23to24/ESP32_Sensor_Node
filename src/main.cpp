#include <Arduino.h>
#include <Wire.h>

#include "SparkFunTMP102.h"
#include "SparkFun_u-blox_GNSS_v3.h"
#include "auxiliary.h"
#include "bluetooth.hpp"

uuids UUID_generator;
TMP102 sensor;
Bluetooth<TMP102, uuids> bluetooth;
SFE_UBLOX_GNSS myGNSS;

void NAVSAT_callback(UBX_NAV_SAT_data_t *ubxDataStruct)
{
    Serial.println();

    Serial.print(F("New NAV SAT data received. It contains data for "));
    Serial.print(ubxDataStruct->header.numSvs);
    if (ubxDataStruct->header.numSvs == 1)
        Serial.println(F(" SV."));
    else
        Serial.println(F(" SVs."));

    // Just for giggles, print the signal strength for each SV as a barchart
    for (uint16_t block = 0; block < ubxDataStruct->header.numSvs; block++) // For each SV
    {
        switch (ubxDataStruct->blocks[block].gnssId) // Print the GNSS ID
        {
        case 0:
            Serial.print(F("GPS     "));
            break;
        case 1:
            Serial.print(F("SBAS    "));
            break;
        case 2:
            Serial.print(F("Galileo "));
            break;
        case 3:
            Serial.print(F("BeiDou  "));
            break;
        case 4:
            Serial.print(F("IMES    "));
            break;
        case 5:
            Serial.print(F("QZSS    "));
            break;
        case 6:
            Serial.print(F("GLONASS "));
            break;
        default:
            Serial.print(F("UNKNOWN "));
            break;
        }

        Serial.print(ubxDataStruct->blocks[block].svId); // Print the SV ID

        if (ubxDataStruct->blocks[block].svId < 10)
            Serial.print(F("   "));
        else if (ubxDataStruct->blocks[block].svId < 100)
            Serial.print(F("  "));
        else
            Serial.print(F(" "));

        // Print the signal strength as a bar chart
        for (uint8_t cno = 0; cno < ubxDataStruct->blocks[block].cno; cno++)
            Serial.print(F("="));

        Serial.println();
    }
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

    if (myGNSS.begin() == false) // Connect to the u-blox module using Wire port
    {
        Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    }
    else
    {
        myGNSS.setI2COutput(COM_TYPE_UBX);
        myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT);
        myGNSS.setNavigationFrequency(1);
        myGNSS.setAutoNAVSATcallbackPtr(&NAVSAT_callback);
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
