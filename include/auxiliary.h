#ifndef B6B5A080_A253_4EB5_8937_6294B99B5CC8
#define B6B5A080_A253_4EB5_8937_6294B99B5CC8

#include <bootloader_random.h>
#include <esp_random.h>

#include "UUID.h"

struct uuids
{
  private:
    const char *service_uuid = "d544bcdc-58fc-4f26-a5a4-24e8fd7f12ae";
    const char *characteristic_uuid = "f74fbe6b-bed4-492b-b9ed-928869ee9fd3";
    /*uint32_t rand1 = 0, rand2 = 0, rand3 = 0, rand4 = 0;
    UUID uuid2;
    UUID uuid1;*/

  public:
    /*uuids()
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
    }*/

    const char *get_service_uuid()
    {
        return service_uuid;
    }

    const char *get_characteristic_uuid()
    {
        return characteristic_uuid;
    }
};

#endif /* B6B5A080_A253_4EB5_8937_6294B99B5CC8 */
