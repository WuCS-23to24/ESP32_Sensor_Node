#ifndef BODYASWIRE_HPP
#define BODYASWIRE_HPP

#include "data_packet.h"
#include <Arduino.h>

const int F_CARRIER = 10000;
const int SYMBOL_HALF_MS = 5;

// class for handling body as wire transmission
class BodyAsWire
{
    private:
        int pin_A0 = digitalPinToBitMask(A0);

        TransmissionData _data;

        void transmitStartRZ()
        {
            tone(A0, F_CARRIER);
            delay(7*SYMBOL_HALF_MS);
            noTone(A0);
            dacWrite(pin_A0, 0);
            delay(SYMBOL_HALF_MS);
        }

        void transmitFloatRZ(float float_data)
        {
            for (int i = 0; i < 32; i++)
            {
                if (bitRead(*(uint32_t*)&float_data, i) == 0)
                {
                    delay(2*SYMBOL_HALF_MS);
                }
                else
                {
                    tone(A0, F_CARRIER);
                    delay(SYMBOL_HALF_MS);
                    noTone(A0);
                    dacWrite(pin_A0, 0);
                    delay(SYMBOL_HALF_MS);
                }
            }
        }

        void transmitStopRZ()
        {
            tone(A0, F_CARRIER);
            delay(3*SYMBOL_HALF_MS);
            noTone(A0);
            dacWrite(pin_A0, 0);
            delay(SYMBOL_HALF_MS);
        }

    public:
        TransmissionData getData()
        {
            return _data;
        }

        void setData(TransmissionData data)
        {
            _data = data;
        }

        void transmitFrame()
        {
            transmitStartRZ();
            transmitFloatRZ(_data.temp_data);
            transmitStopRZ();
        }
};

#endif