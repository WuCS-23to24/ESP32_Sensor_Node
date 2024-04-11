#ifndef A3A894A8_7557_423C_AF30_DF2AD1257EE6
#define A3A894A8_7557_423C_AF30_DF2AD1257EE6

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <sstream>

bool clientConnected = false;
typedef struct __attribute__((__packed__)) BluetoothTransmissionData
{
    float temp_data;
    /*float latitude;
    float longitude;
    float altitude;*/
} BluetoothTransmissionData_t;

typedef union BluetoothTransmissionDataConverter_u {

    BluetoothTransmissionData_t message;
    uint8_t bytes[sizeof(BluetoothTransmissionData)];
    uint32_t frame;

} BluetoothTransmissionDataConverter_t;

class CharacteristicCallbacks : public BLECharacteristicCallbacks
{
  private:
    BluetoothTransmissionData _data;

  public:
    CharacteristicCallbacks()
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
        std::stringstream str_data("");
        /*str_data << "TEMP: " << _data.temp_data << " LAT: " << _data.latitude << " LONG: " << _data.longitude
                 << " ALT: " << _data.altitude;*/

        Serial.println(str_data.str().c_str());

        pCharacteristic->setValue(str_data.str());
    }

    void onNotify(BLECharacteristic *pCharacteristic)
    {
        printf("NOTIFED\n");
    }

    void onStatus(BLECharacteristic *pCharacteristic, Status s, uint32_t code)
    {
        printf("STATUS\n");
    }

    BluetoothTransmissionData getData()
    {
        return _data;
    }

    void setData(BluetoothTransmissionData data)
    {
        _data = data;
    }
};

class ServerCallbacks : public BLEServerCallbacks
{
  public:
    void onConnect(BLEServer *pServer)
    {
        clientConnected = true;
        Serial.printf("Connected to aggregator.\n");
        BLEDevice::stopAdvertising();
    }

    void onDisconnect(BLEServer *pServer)
    {
        clientConnected = false;
        Serial.printf("Disconnected from aggregator.\n");
        BLEDevice::startAdvertising();
    }
};

template <typename _UUID_Generator_Type> class Bluetooth
{
  private:
    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pCharacteristic;
    BLEAdvertising *pAdvertising;
    _UUID_Generator_Type _uuid_gen_struct;

  public:
    CharacteristicCallbacks *callback_class;

    Bluetooth()
    {
    }

    Bluetooth(_UUID_Generator_Type uuid_gen_struct) : _uuid_gen_struct(uuid_gen_struct)
    {
        callback_class = new CharacteristicCallbacks();

        BLEDevice::init("S0");
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks);
        pService = pServer->createService(_uuid_gen_struct.get_service_uuid());
        pCharacteristic = pService->createCharacteristic(
            _uuid_gen_struct.get_characteristic_uuid(),
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

        pCharacteristic->setCallbacks(callback_class);
        pCharacteristic->setValue("Sensor Node");
        pService->start();

        pAdvertising = pServer->getAdvertising();
        pAdvertising->addServiceUUID(_uuid_gen_struct.get_service_uuid());
        pAdvertising->setScanResponse(false);
        pAdvertising->setMinPreferred(0x0);
        pAdvertising->start();
    }

    void sendData()
    {
        if (clientConnected)
        {
            BluetoothTransmissionDataConverter_t converter;
            converter.message = callback_class->getData();
            pCharacteristic->setValue(converter.bytes, sizeof(BluetoothTransmissionData));
            pCharacteristic->notify();
        }
    }

    bool clientIsConnected()
    {
        return clientConnected;
    }
};

#endif /* A3A894A8_7557_423C_AF30_DF2AD1257EE6 */
