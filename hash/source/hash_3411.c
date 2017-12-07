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
static uint8_t* h = NULL;
static uint8_t* n = NULL;
static uint8_t* s = NULL;
static uint8_t* m = NULL;
static uint8_t* temp = NULL;
static uint8_t* datatmp = NULL; //for streamed hash
static size_t datatmplen = 0;

//inner function prototypes
static uint8_t* _hash_generate_append(
        const int mode, const uint8_t* data, size_t len, uint8_t is_end);
static uint8_t* _hmac_generate(
        const int mode, const uint8_t* inkey,
        size_t keylen, const uint8_t* data, size_t len);
static void _hmac_set_key(const int mode, const uint8_t* inkey, size_t keylen);
static uint8_t* _hmac_generate_append(
        const int mode, const uint8_t* data, size_t len, uint8_t is_end);
static void _hash_vars_init(const int mode);
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
{ return _hash_generate_append(256, data, len, 1); }

uint8_t* hash_generate_256_append(const uint8_t* data, size_t len, uint8_t is_end)
{ return _hash_generate_append(256, data, len, is_end); }

uint8_t* hash_generate_512(const uint8_t* data, size_t len)
{ return _hash_generate_append(512, data, len, 1); }

uint8_t* hash_generate_512_append(const uint8_t* data, size_t len, uint8_t is_end)
{ return _hash_generate_append(512, data, len, is_end); }

uint8_t* hmac_generate_256(
        const uint8_t* key, size_t keylen, const uint8_t* data, size_t len)
{ return _hmac_generate(256, key, keylen, data, len); }

uint8_t* hmac_generate_512(
        const uint8_t* key, size_t keylen, const uint8_t* data, size_t len)
{ return _hmac_generate(512, key, keylen, data, len); }


//inner functions
static uint8_t* _hash_generate_append(
        const int mode, const uint8_t* data, size_t len, uint8_t is_end)
{
    //1
    if (h == NULL)
        _hash_vars_init(mode);

    //2
    //while (len >= 64)
    while (!is_end || datatmplen + len >= 64)
    {
        //prepare datatmp
        if (datatmplen + len >= 64)
        {
            memcpy(datatmp+datatmplen, data, 64-datatmplen);
            len -= 64-datatmplen;
            data += 64-datatmplen;
            datatmplen = 64;
        }
        else //if block is not accumulated - wait for next data
        {
            memcpy(datatmp+datatmplen, data, len);
            datatmplen += len;
            return NULL;
        }

        memcpy(m, datatmp, 64);
        datatmplen = 0;

        _g_message(temp, n, h, m);
        memcpy(h, temp, 64);

        int64_t num[8] = {0};
        num[0] = 512;
        _sum_block(n, n, (uint8_t*)num);

        _sum_block(s, s, m);
    }

    //3, len < 64
    memcpy(datatmp+datatmplen, data, len);
    datatmplen += len;

    for (int i = datatmplen; i < 64; i++)
        m[i] = 0;
    m[datatmplen] = 1;
    memcpy(m, datatmp, datatmplen);

    _g_message(temp, n, h, m);
    memcpy(h, temp, 64);

    int64_t num[8] = {0};
    num[0] = datatmplen*8;
    _sum_block(n, n, (uint8_t*)num);

    _sum_block(s, s, m);

    num[0] = 0;
    _g_message(temp, (uint8_t*)num, h, n);
    memcpy(h, temp, 64);

    _g_message(temp, (uint8_t*)num, h, s);
    memcpy(h, temp, 64);

    free(m); m = NULL;
    free(s); s = NULL;
    free(n); n = NULL;
    free(datatmp); datatmp = NULL;

    uint8_t* res = h; h = NULL;
    if (mode != 256)
        return res;

    uint8_t* msb = (uint8_t*)malloc(32);
    memcpy(msb, res+32, 32);
    free(res);
    return msb;
}

static uint8_t* _hmac_generate(
        const int mode, const uint8_t* inkey,
        size_t inkeylen, const uint8_t* data, size_t len)
{
    _hmac_set_key(mode, inkey, inkeylen);
    return _hmac_generate_append(mode, data, len, 1);
}

static uint8_t* key = NULL;
static size_t keylen = 0;
static void _hmac_set_key(const int mode, const uint8_t* inkey, size_t inkeylen)
{
    if (inkeylen < 8 /*32*/ || inkeylen > 64)
    {
        fprintf(stderr, "HMAC: wrong key size (%lld)\n.", inkeylen);
        return;
    }
    keylen = inkeylen;

    key = (uint8_t*)calloc(1, 64);
    memcpy(key, inkey, keylen);

    uint8_t key_xor_ipad[64];
    _xor_block(key_xor_ipad, key, ipad);

    uint8_t* concat = (uint8_t*)malloc(64);
    memcpy(concat, key_xor_ipad, 64);

    _hash_generate_append(mode, concat, 64, 0);
    free(concat);
}

static uint8_t* _hmac_generate_append(
        const int mode, const uint8_t* data, size_t len, uint8_t is_end)
{
    size_t hashlen = (mode == 256) ? 32 : 64;
    uint8_t* hash_ipad = _hash_generate_append(mode, data, len, is_end);

    if (hash_ipad == NULL)
        return NULL;

    uint8_t key_xor_opad[64];
    _xor_block(key_xor_opad, key, opad);

    uint8_t concat2[64+hashlen];
    memcpy(concat2, key_xor_opad, 64);
    memcpy(concat2+64, hash_ipad, hashlen);

    memset(key, 0, keylen);
    keylen = 0;
    free(key); key = NULL;
    free(hash_ipad);

    return _hash_generate_append(mode, concat2, 64+hashlen, 1);
}



//h - result
static void _hash_vars_init(const int mode)
{
    //1
    if (h != NULL) free(h);
    h = (uint8_t*)malloc(64);
    memcpy(
            h,
            (mode == 256) ? init_vector_256 : init_vector_512,
            64);
    if (n != NULL) free(n); n = (uint8_t*)calloc(1, 64);
    if (s != NULL) free(s); s = (uint8_t*)calloc(1, 64);
    if (m != NULL) free(m); m = (uint8_t*)calloc(1, 64);
    if (temp != NULL) free(temp); temp = (uint8_t*)malloc(64);
    if (datatmp != NULL) free(datatmp); datatmp = (uint8_t*)malloc(64);
    datatmplen = 0;
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

