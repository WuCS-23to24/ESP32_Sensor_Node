#ifndef A3A894A8_7557_423C_AF30_DF2AD1257EE6
#define A3A894A8_7557_423C_AF30_DF2AD1257EE6

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

template <typename _Sensor_Type> class CharacteristicCallbacks : public BLECharacteristicCallbacks
{
  private:
    _Sensor_Type _sensor;

  public:
    CharacteristicCallbacks(_Sensor_Type sensor) : _sensor(sensor)
    {
    }

    void onWrite(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param)
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

    void onRead(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param)
    {
        float data_raw = _sensor.readTempF();
        std::string data = std::to_string(data_raw);
        pCharacteristic->setValue(data);
    }

    void onNotify(BLECharacteristic *pCharacteristic)
    {
        printf("NOTIFED\n");
    }

    void onStatus(BLECharacteristic *pCharacteristic, Status s, uint32_t code)
    {
        printf("STATUS\n");
    }
};

template <typename _Sensor_Type, typename _UUID_Generator_Type> class Bluetooth
{
  private:
    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pCharacteristic;
    BLEAdvertising *pAdvertising;
    _UUID_Generator_Type _uuid_gen_struct;

  public:
    Bluetooth()
    {
    }
    Bluetooth(_UUID_Generator_Type uuid_gen_struct, _Sensor_Type sensor) : _uuid_gen_struct(uuid_gen_struct)
    {
        BLEDevice::init("TEMP-0");
        pServer = BLEDevice::createServer();
        pService = pServer->createService(_uuid_gen_struct.get_service_uuid());
        pCharacteristic = pService->createCharacteristic(
            _uuid_gen_struct.get_characteristic_uuid(),
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY |
                BLECharacteristic::PROPERTY_INDICATE);

        pCharacteristic->setCallbacks(new CharacteristicCallbacks<_Sensor_Type>(sensor));
        pCharacteristic->setValue("Temp Sensor Node");
        pService->start();

        pAdvertising = pServer->getAdvertising();
        pAdvertising->start();
    }
};

#endif /* A3A894A8_7557_423C_AF30_DF2AD1257EE6 */
