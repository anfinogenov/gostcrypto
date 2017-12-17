#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../hash/headers/hash_3411.h"
#include "../headers/pbkdf2.h"

#define p(t) printf("%s ", t)
void printle(uint8_t* a, int len)
{
    printf("0x");
    for (int i = len-1; i >= 0; i--)
        printf("%s%hhx", (a[i]<0x10) ? "0" : "", a[i]);
    printf("\n");
}

int main()
{
    char passwd[33] = "HelloWorldHelloWorldHelloWorld32";
    int passwdlen = strlen(passwd);
    char salt[11] = "RandomSalt";
    int saltlen = strlen(salt);

    uint8_t* key = pbkdf2(
            hmac_generate_256, 32,
            passwd, passwdlen,
            salt, saltlen,
            100,
            32);

    p("password:"); printf("%s\n", passwd);//printle(passwd, passwdlen);
    //p("password:"); printle(passwd, passwdlen);
    if (key == NULL)
        fprintf(stderr, "failed!\n");
    else
    {
        p("     key:"); printle(key, 32);
        free(key);
    }
}

