#include <Arduino.h>
#include <cstdint>

const int F_CARRIER = 10000;
const int SYMBOL_HALF_MS = 5;

int pin_A0 = digitalPinToBitMask(A0);




void send_baw_data(uint32_t data)
{
    // RZ START sequence
    tone(A0, F_CARRIER);
    delay(7*SYMBOL_HALF_MS);
    noTone(A0);
    dacWrite(pin_A0, 0);
    delay(SYMBOL_HALF_MS);

    // RZ data sequence
    for (int i = 0; i < 32; i++)
    {
        if (bitRead(data, i) == 0)
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

    // RZ STOP sequence
    tone(A0, F_CARRIER);
    delay(3*SYMBOL_HALF_MS);
    noTone(A0);
    dacWrite(pin_A0, 0);
    delay(SYMBOL_HALF_MS);

}

/*void loop()
{
    for (int i = 0; i < 4; i++)
    {
        transmitFrame(pregen_frames[i]);
        delay(1000);
    }
}*/

