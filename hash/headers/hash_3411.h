#ifndef HASH_3411_H
#define HASH_3411_H

#include <stdint.h>

#ifdef __cplusplus
#include <cstdlib>

extern "C"
{
#else
#include <stdlib.h>
#endif

uint8_t* hash_generate_256(const uint8_t* data, size_t len);
uint8_t* hash_generate_512(const uint8_t* data, size_t len);
uint8_t* hash_generate_256_append(const uint8_t* data, size_t len, uint8_t is_end);
uint8_t* hash_generate_512_append(const uint8_t* data, size_t len, uint8_t is_end);
uint8_t* hmac_generate_256(
        const uint8_t* key, size_t keylen, const uint8_t* data, size_t len);
uint8_t* hmac_generate_512(
        const uint8_t* key, size_t keylen, const uint8_t* data, size_t len);
void hmac_generate_set_key_256(const uint8_t* key, size_t keylen);
void hmac_generate_set_key_512(const uint8_t* key, size_t keylen);
uint8_t* hmac_generate_256_append(const uint8_t* data, size_t len, uint8_t is_end);
uint8_t* hmac_generate_512_append(const uint8_t* data, size_t len, uint8_t is_end);

#ifdef __cplusplus
}

namespace GOST3411
{
    uint8_t* hash_256(const uint8_t* data, size_t len)
    { return hash_generate_256(data, len); }

    uint8_t* hash_512(const uint8_t* data, size_t len)
    { return hash_generate_512(data, len); }

    uint8_t* hash_256_append(const uint8_t* data, size_t len, uint8_t is_end)
    { return hash_generate_256_append(data, len, is_end); }

    uint8_t* hash_512_append(const uint8_t* data, size_t len, uint8_t is_end)
    { return hash_generate_256_append(data, len, is_end); }

    uint8_t* hmac_256(
            const uint8_t* key, size_t keylen, const uint8_t* data, size_t len)
    { return hmac_generate_256(key, keylen, data, len); }

    uint8_t* hmac_512(
            const uint8_t* key, size_t keylen, const uint8_t* data, size_t len)
    { return hmac_generate_256(key, keylen, data, len); }

    void hmac_set_key_256(const uint8_t* key, size_t keylen)
    { hmac_generate_set_key_256(key, keylen); }

    void hmac_set_key_512(const uint8_t* key, size_t keylen)
    { hmac_generate_set_key_512(key, keylen); }

    uint8_t* hmac_256_append(const uint8_t* data, size_t len, uint8_t is_end)
    { return hmac_generate_256_append(data, len, is_end); }

    uint8_t* hmac_512_append(const uint8_t* data, size_t len, uint8_t is_end)
    { return hmac_generate_512_append(data, len, is_end); }
}
#endif

#endif

