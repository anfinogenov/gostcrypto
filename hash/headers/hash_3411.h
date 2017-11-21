#ifndef HASH_3411_H
#define HASH_3411_H

#include <stdint.h>

uint8_t* hash_generate_256(const uint8_t* data, size_t len);
uint8_t* hash_generate_512(const uint8_t* data, size_t len);
uint8_t* hmac_generate_256(
        const uint8_t* key, size_t keylen, const uint8_t* data, size_t len);
uint8_t* hmac_generate_512(
        const uint8_t* key, size_t keylen, const uint8_t* data, size_t len);

#endif

