#ifndef B6B5A080_A253_4EB5_8937_6294B99B5CC8
#define B6B5A080_A253_4EB5_8937_6294B99B5CC8

#include <bootloader_random.h>
#include <esp_random.h>

#include "UUID.h"

struct uuids
{
  private:
    char *service_uuid = nullptr;
    char *characteristic_uuid = nullptr;
    uint32_t rand1 = 0, rand2 = 0, rand3 = 0, rand4 = 0;
    UUID uuid2;
    UUID uuid1;

  public:
    uuids()
    {
    }
    ~uuids()
    {
        bootloader_random_disable();
    }

    void initialize_random_values()
    {
        bootloader_random_enable();

        rand1 = esp_random();
        rand2 = esp_random();
        rand3 = esp_random();
        rand4 = esp_random();
    }

    void generate_hashes()
    {
        uuid1.seed(rand1, rand2);
        uuid2.seed(rand3, rand4);

        uuid1.generate();
        uuid2.generate();

        service_uuid = uuid1.toCharArray();
        characteristic_uuid = uuid2.toCharArray();
    }

    char *get_service_uuid()
    {
        return service_uuid;
    }

    char *get_characteristic_uuid()
    {
        return characteristic_uuid;
    }
};

template <typename _Sensor_Type> struct Sensor
{
  private:
    _Sensor_Type _sensor;

  public:
    Sensor(_Sensor_Type sensor) : _sensor(sensor)
    {
        if constexpr (std::is_same<_Sensor_Type, TMP102>::value)
        {
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
        }
    }
};

#endif /* B6B5A080_A253_4EB5_8937_6294B99B5CC8 */
