#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>

#include "../../hash/headers/hash_3411.h"
#include "../../pbkdf2/headers/pbkdf2.h"
#include "../headers/hybrid.hpp"
namespace hybrid{void debug();}
int main()
{
    std::cerr << "===Self tests===" << std::endl;
    hybrid::debug();
    std::cerr << "=======OK=======" << std::endl << std::endl;
    //return 0;

    char passwd[37] = "longpasswordlongpasswordlongpassword";
    uint8_t* key = pbkdf2(hmac_generate_256, 32, (uint8_t*)passwd, 36, (uint8_t*)"salt", 4, 1000, 32);
    hybrid::set_key(key, 32);
    char message[33] = "1234567891234567890012345678901";
    size_t out = 0;
    uint8_t* enc = hybrid::encrypt((uint8_t*)message, 31, out);
    uint8_t* dec = hybrid::decrypt(enc, out, out);
    printf("res size: %d\n", out);

    printf("plain: ");
    for (int i = 0; i < 31; i++)
        printf("%s%x ", (message[i] < 0x10) ? "0" : "", message[i]);
    printf("\n");
    printf("  dec: ");
    for (unsigned int i = 0; i < out; i++)
        printf("%s%x ", (dec[i] < 0x10) ? "0" : "", dec[i]);
    printf("\n");
}

