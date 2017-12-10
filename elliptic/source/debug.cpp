#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>

#include "../../hash/headers/hash_3411.h"
#include "../../pbkdf2/headers/pbkdf2.h"
#include "../headers/elliptic.hpp"
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

    char message[33] = "12345678901234567890123456789012";
    message[31] = -128;

    size_t out = 0;
    uint8_t* enc = hybrid::encrypt((uint8_t*)message, 32, out);
    uint8_t* dec = hybrid::decrypt(enc, out, out);
    printf("res size: %d\n", out);

    printf("plain: ");
    for (int i = 0; i < 32; i++)
        printf("%02hhx ", message[i]);
    printf("\n");

    printf("  dec: ");
    if (dec == NULL)
    {
        printf("failed!\n");
        return 1;
    }
    for (unsigned int i = 0; i < out; i++)
        printf("%02hhx ", dec[i]);
    printf("\n");
}

