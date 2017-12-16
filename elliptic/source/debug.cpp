#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>

#include "../../hash/headers/hash_3411.h"
#include "../../pbkdf2/headers/pbkdf2.h"
#include "../headers/elliptic.hpp"
namespace elliptic{void debug();}
int main()
{
    std::cerr << "===Self tests===" << std::endl;
    elliptic::debug();
    std::cerr << "=======OK=======" << std::endl << std::endl;
    //return 0;

    char passwd[37] = "longpasswordlongpasswordlongpassword";
    uint8_t* key = pbkdf2(hmac_generate_256, 32, (uint8_t*)passwd, 36, (uint8_t*)"salt", 4, 1000, 32);

    elliptic::set_key(key, 32);

    //char message[33] = "12345678901234567890123456789012";
    char message[35] = "1234567890123456789012345678901234";
    message[31] = -128;
    message[33] = 0;
    message[32] = 0;
    message[31] = 0;
    message[30] = 0;
    message[29] = 0;
    message[28] = 0;
    message[27] = 0;
    message[0] = 0;

    size_t out = 0;
    uint8_t* enc = elliptic::encrypt((uint8_t*)message, 34, out);
    uint8_t* dec = elliptic::decrypt(enc, out, out);
    //printf("res size: %d\n", out);

    printf("plain: ");
    for (int i = 0; i < 34; i++)
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

