#ifndef A3A894A8_7557_423C_AF30_DF2AD1257EE6
#define A3A894A8_7557_423C_AF30_DF2AD1257EE6

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <sstream>
#include "data_packet.h"

bool clientConnected = false; // indicates whether aggregator is connected

// Callbacks for server's main characteristic between sensor and aggregator 
class CharacteristicCallbacks : public BLECharacteristicCallbacks
{
  private:
    TransmissionData _data;

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
        str_data << "TEMP: " << _data.temp_data << " LAT: " << _data.latitude << " LONG: " << _data.longitude
                 << " ALT: " << _data.altitude;

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

    TransmissionData getData()
    {
        return _data;
    }

    void setData(TransmissionData data)
    {
        _data = data;
    }
};

// Callbacks for overall server between sensor and aggregator 
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

// main class for handling BLE 
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

        // Initialize BLE server
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

    // Send a message using notify to the aggregator 
    void sendData()
    {
        if (clientConnected)
        {
            TransmissionDataConverter_t converter;
            converter.message = callback_class->getData();
            pCharacteristic->setValue(converter.bytes, sizeof(TransmissionData));
            pCharacteristic->notify();
        }
    }

    bool clientIsConnected()
    {
        return clientConnected;
    }
};

#endif /* A3A894A8_7557_423C_AF30_DF2AD1257EE6 */
