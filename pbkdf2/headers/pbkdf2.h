#ifndef PBKDF2_H
#define PBKDF2_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t* pbkdf2(
        uint8_t* (*prf)(const uint8_t*, size_t, const uint8_t*, size_t),
        size_t hlen,
        uint8_t* passwd, size_t passwdlen,
        uint8_t* salt, size_t saltlen,
        uint32_t iters,
        size_t dklen);

#ifdef __cplusplus
}
#endif

#endif

