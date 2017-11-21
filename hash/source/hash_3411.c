#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../headers/hash_3411.h"
#include "../headers/const.h"



//inner variables
static uint8_t init_vector_256[64] = {0x01};
static uint8_t init_vector_512[64] = {0x00};
static uint8_t ipad[64] = {0x36};
static uint8_t opad[64] = {0x5c};

//inner function prototypes
static uint8_t* _hash_generate(const int mode, const uint8_t* data, size_t len);
static uint8_t* _hmac_generate(
        const int mode, const uint8_t* inkey,
        size_t keylen, const uint8_t* data, size_t len);
static uint8_t* _xor_block(uint8_t* res, const uint8_t* a, const uint8_t* b);
static uint8_t* _s_block(uint8_t* res, const uint8_t* a);
static uint8_t* _p_block(uint8_t* res, const uint8_t* a);
static uint8_t* _l_block(uint8_t* res, const uint8_t* a);
static uint8_t* _lps_block(uint8_t* res, const uint8_t* a);
static uint8_t* _e_block(uint8_t* res, const uint8_t* a, const uint8_t* m);
static uint8_t* _g_message(
        uint8_t* res, const uint8_t* n,
        const uint8_t* h, const uint8_t* m);
static uint8_t* _sum_block(uint8_t* res, const uint8_t* a, const uint8_t* b);



//initialization
__attribute__((constructor))
static void _fill_init_vectors(void)
{
    for (int i = 1; i < 64; i++)
    {
        init_vector_256[i] = init_vector_256[0]; //0x01;
        init_vector_512[i] = init_vector_512[0]; //0x00;
        ipad[i] = ipad[0]; //0x36;
        opad[i] = opad[0]; //0x5c;
    }
}



//outer functions
uint8_t* hash_generate_256(const uint8_t* data, size_t len)
{ return _hash_generate(256, data, len); }

uint8_t* hash_generate_512(const uint8_t* data, size_t len)
{ return _hash_generate(512, data, len); }

uint8_t* hmac_generate_256(
        const uint8_t* key, size_t keylen, const uint8_t* data, size_t len)
{ return _hmac_generate(256, key, keylen, data, len); }

uint8_t* hmac_generate_512(
        const uint8_t* key, size_t keylen, const uint8_t* data, size_t len)
{ return _hmac_generate(512, key, keylen, data, len); }


//inner functions
static uint8_t* _hash_generate(const int mode, const uint8_t* data, size_t len)
{
    //1
    uint8_t* h = (uint8_t*)malloc(64);
    memcpy(
            h,
            (mode == 256) ? init_vector_256 : init_vector_512,
            64
    );

    uint8_t* n = (uint8_t*)calloc(1, 64);
    uint8_t* s = (uint8_t*)calloc(1, 64);
    uint8_t* m = (uint8_t*)calloc(1, 64);

    uint8_t temp[64];

    //2
    while (len >= 64)
    {
        memcpy(m, data, 64);

        _g_message(temp, n, h, m);
        memcpy(h, temp, 64);

        int64_t num[8] = {0};
        num[0] = 512;
        _sum_block(n, n, (uint8_t*)num);

        _sum_block(s, s, m);

        data += 64;
        len -= 64;
    }

    //3, len < 64
    for (int i = len; i < 64; i++)
        m[i] = 0;
    m[len] = 1;
    memcpy(m, data, len);

    _g_message(temp, n, h, m);
    memcpy(h, temp, 64);

    int64_t num[8] = {0};
    num[0] = len*8;
    _sum_block(n, n, (uint8_t*)num);

    _sum_block(s, s, m);

    num[0] = 0;
    _g_message(temp, (uint8_t*)num, h, n);
    memcpy(h, temp, 64);

    _g_message(temp, (uint8_t*)num, h, s);
    memcpy(h, temp, 64);

    free(m);
    free(s);
    free(n);

    if (mode != 256)
        return h;

    uint8_t* msb = (uint8_t*)malloc(32);
    memcpy(msb, h+32, 32);
    free(h);
    return msb;
}

static uint8_t* _hmac_generate(
        const int mode, const uint8_t* inkey,
        size_t keylen, const uint8_t* data, size_t len)
{
    /*if (keylen < 32 || keylen > 64)
    {
        fprintf(stderr, "HMAC: wrong key size (%lld)\n.", keylen);
        return NULL;
    }*/

    uint8_t* key = (uint8_t*)calloc(1, 64);
    //memcpy(key+(64-keylen), inkey, keylen);
    memcpy(key, inkey, keylen);

    uint8_t key_xor_ipad[64];
    uint8_t key_xor_opad[64];
    _xor_block(key_xor_ipad, key, ipad);
    _xor_block(key_xor_opad, key, opad);

    uint8_t* concat = (uint8_t*)malloc(len+64);
    //memcpy(concat, data, len);
    //memcpy(concat+len, key_xor_ipad, 64);
    memcpy(concat, key_xor_ipad, 64);
    memcpy(concat+64, data, len);

    size_t hashlen = (mode == 256) ? 32 : 64;
    uint8_t* hash_ipad = _hash_generate(mode, concat, len+64);

    uint8_t concat2[64+hashlen];
    //memcpy(concat2, hash_ipad, hashlen);
    //memcpy(concat2+hashlen, key_xor_opad, 64);
    memcpy(concat2, key_xor_opad, 64);
    memcpy(concat2+64, hash_ipad, hashlen);

    free(key);
    free(concat);
    free(hash_ipad);

    return _hash_generate(mode, concat2, 64+hashlen);
}



static uint8_t* _xor_block(uint8_t* res, const uint8_t* a, const uint8_t* b)
{
    uint8_t cpy[64];
    memcpy(cpy, a, 64);
    for (int i = 0; i < 64; i++)
        res[i] = a[i] ^ b[i];

    //XorArrays(res, a, b, 64);
    return res;
}

static uint8_t* _s_block(uint8_t* res, const uint8_t* a)
{
    uint8_t cpy[64];
    memcpy(cpy, a, 64);
    for (int i = 0; i < 64; ++i)
        res[i] = kPi[ a[i] ];
    return res;
}

static uint8_t* _p_block(uint8_t* res, const uint8_t* a)
{
    uint8_t cpy[64];
    memcpy(cpy, a, 64);
    for (int i = 0; i < 64; ++i)
        res[i] = cpy[ kTau[i] ];
    return res;
}

static uint64_t _l_mult(uint64_t a)
{
    uint64_t res = 0;
    for (int i = 0; i < 64; ++i)
        res ^= ((a >> i) & 1) * kL[63-i];

    return res;
}

static uint8_t* _l_block(uint8_t* res, const uint8_t* a)
{
    uint64_t cpy[8];
    memcpy(cpy, a, 64);

    uint64_t* res64 = (uint64_t*)res;
    for (int i = 0; i < 8; ++i)
        res64[i] = _l_mult(cpy[i]);

    return res;
}

static uint8_t* _lps_block(uint8_t* res, const uint8_t* a)
{
    return _l_block(res, _p_block(res, _s_block(res, a)));
}

static uint8_t* _e_block(uint8_t* res, const uint8_t* a, const uint8_t* m)
{
    //init
    uint8_t klast[64], knext[64];
    memcpy(knext, a, 64);
    memcpy(res, m, 64);

    //iters
    for (int i = 1; i <= 13; ++i)
    {
        _xor_block(res, knext, res);
        if (i > 12)
            break;

        _lps_block(res, res);

        memcpy(klast, knext, 64);
        _xor_block(knext, knext, (uint8_t*)(kC[i-1]));
        _lps_block(knext, knext);
    }
    return res;
}

static uint8_t* _g_message(
        uint8_t* res, const uint8_t* n,
        const uint8_t* h, const uint8_t* m)
{
    _e_block(res, _lps_block(res, _xor_block(res, h, n)), m);
    _xor_block(res, h, _xor_block(res, res, m));
    return res;
}

static uint8_t* _sum_block(uint8_t* res, const uint8_t* a, const uint8_t* b)
{
    uint8_t cpy[64];
    memcpy(cpy, a, 64);
    char carry = 0;
    for (int i = 0; i < 64; i++)
    {
        uint16_t s = a[i] + b[i] + carry;
        carry = (s >> 8) & 1;
        res[i] = (uint8_t)s;
    }
    return res;
}

