#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint8_t* _xor_block(
        uint8_t* res, const uint8_t* a, const uint8_t* b, size_t len);
uint8_t* _reverse(uint8_t* res, uint8_t* a, size_t len);

//prf: key, keylen, data, datalen
uint8_t* pbkdf2(
        uint8_t* (*prf)(const uint8_t*, size_t, const uint8_t*, size_t),
        size_t hlen,
        uint8_t* passwd, size_t passwdlen,
        uint8_t* salt, size_t saltlen,
        uint32_t iters,
        size_t dklen)
{
    //TODO: hmac-streebog needs key between 32 and 64 bytes.
    //block have length hlen, hash output length
    size_t l = dklen / hlen;
    size_t r = dklen % hlen;
    if (r == 0)
        r = hlen;
    else
        ++l;

    //key
    uint8_t* dk = (uint8_t*)calloc(1, dklen);
    //hash output to this variable
    uint8_t* prfout = NULL;
    //argument for prf, allocate enough for hash out and first iter's arg
    uint8_t* data = malloc((saltlen+4 > hlen) ? saltlen+4 : hlen);
    size_t datalen;

    //for each block
    for (size_t i = 0; i < l; ++i)
    {
        //block number with msb first
        uint32_t reverse_i;
        _reverse((uint8_t*)&reverse_i, (uint8_t*)&i, 4);

        //iterations (F function)
        for (uint32_t j = 0; j < iters; ++j)
        {
            if (!j)
            {
                //first data is salt concatenated with block num
                memcpy(data, salt, saltlen);
                memcpy(data+saltlen, &reverse_i, 4);
                datalen = saltlen+4;
            }
            else
            {
                //other data are previous prf output
                memcpy(data, prfout, hlen);
                datalen = hlen;
                free(prfout);
            }

            prfout = prf(passwd, passwdlen, data, datalen);

            //xor prf outputs
            _xor_block(dk+(hlen*i), dk+(hlen*i), prfout, (i == l-1) ? r : hlen);
        }
        free(prfout);
    }
    free(data);
    return dk;
}


static uint8_t* _xor_block(
        uint8_t* res, const uint8_t* a, const uint8_t* b, size_t len)
{
    uint8_t cpy[len];
    memcpy(cpy, a, len);
    for (int i = 0; i < len; i++)
        res[i] = a[i] ^ b[i];

    //TODO: use another function
    //XorArrays(res, a, b, 64);
    return res;
}

uint8_t* _reverse(uint8_t* res, uint8_t* a, size_t len)
{
    for (size_t i = 0; i < (len >> 1); ++i)
        res[i] = a[len-i-1];
    return res;
}

