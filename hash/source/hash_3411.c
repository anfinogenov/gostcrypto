#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../headers/hash_3411.h"

#define init_vector init_vector_512

#define DEBUG
//inner functions
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



//inner variables


__attribute__((constructor))
static void _set_init_vectors(void)
{
    for (int i = 0; i < 64; i++)
    {
        init_vector_256[i] = 0x01;
        init_vector_512[i] = 0x00;
    }
}

#ifdef DEBUG
#define p(t) printf("%s ", t);
void printle(uint8_t* a, int len)
{
    printf("0x");
    for (int i = len-1; i >= 0; i--)
        printf("%s%hhx", (a[i]<0x10) ? "0" : "", a[i]);
    printf("\n");
}

int main()
{
    uint8_t msg1[63] = {
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x30, 0x31, 0x32
    };
    uint8_t msg2[72] = {
        0xd1, 0xe5, 0x20, 0xe2, 0xe5, 0xf2, 0xf0, 0xe8, 0x2c, 0x20,
        0xd1, 0xf2, 0xf0, 0xe8, 0xe1, 0xee, 0xe6, 0xe8, 0x20, 0xe2,
        0xed, 0xf3, 0xf6, 0xe8, 0x2c, 0x20, 0xe2, 0xe5, 0xfe, 0xf2,
        0xfa, 0x20, 0xf1, 0x20, 0xec, 0xee, 0xf0, 0xff, 0x20, 0xf1,
        0xf2, 0xf0, 0xe5, 0xeb, 0xe0, 0xec, 0xe8, 0x20, 0xed, 0xe0,
        0x20, 0xf5, 0xf0, 0xe0, 0xe1, 0xf0, 0xfb, 0xff, 0x20, 0xef,
        0xeb, 0xfa, 0xea, 0xfb, 0x20, 0xc8, 0xe3, 0xee, 0xf0, 0xe5,
        0xe2, 0xfb
    };
    uint8_t r1[64] = {
        0x1b, 0x54, 0xd0, 0x1a, 0x4a, 0xf5, 0xb9, 0xd5, 0xcc, 0x3d,
        0x86, 0xd6, 0x8d, 0x28, 0x54, 0x62, 0xb1, 0x9a, 0xbc, 0x24,
        0x75, 0x22, 0x2f, 0x35, 0xc0, 0x85, 0x12, 0x2b, 0xe4, 0xba,
        0x1f, 0xfa, 0x00, 0xad, 0x30, 0xf8, 0x76, 0x7b, 0x3a, 0x82,
        0x38, 0x4c, 0x65, 0x74, 0xf0, 0x24, 0xc3, 0x11, 0xe2, 0xa4,
        0x81, 0x33, 0x2b, 0x08, 0xef, 0x7f, 0x41, 0x79, 0x78, 0x91,
        0xc1, 0x64, 0x6f, 0x48
    };
    uint8_t r2[64] = {
        0x1e, 0x88, 0xe6, 0x22, 0x26, 0xbf, 0xca, 0x6f, 0x99, 0x94,
        0xf1, 0xf2, 0xd5, 0x15, 0x69, 0xe0, 0xda, 0xf8, 0x47, 0x5a,
        0x3b, 0x0f, 0xe6, 0x1a, 0x53, 0x00, 0xee, 0xe4, 0x6d, 0x96,
        0x13, 0x76, 0x03, 0x5f, 0xe8, 0x35, 0x49, 0xad, 0xa2, 0xb8,
        0x62, 0x0f, 0xcd, 0x7c, 0x49, 0x6c, 0xe5, 0xb3, 0x3f, 0x0c,
        0xb9, 0xdd, 0xdc, 0x2b, 0x64, 0x60, 0x14, 0x3b, 0x03, 0xda,
        0xba, 0xc9, 0xfb, 0x28
    };

    p("plain:") printle(msg1, 63);
    uint8_t* h1 = hash_generate(msg1, 63);
    p(" hash:") printle(h1, 64);
    p(" real:") printle(r1, 64);

    p("\nplain:") printle(msg2, 72);
    uint8_t* h2 = hash_generate(msg2, 72);
    p(" hash:") printle(h2, 64);
    p(" real:") printle(r2, 64);
}
#endif

uint8_t* hash_generate(const uint8_t* data, size_t len)
{
    //1
    uint8_t* h = (uint8_t*)malloc(64);
    memcpy(h, init_vector, 64);

    uint8_t* n = (uint8_t*)calloc(1, 64);
    uint8_t* s = (uint8_t*)calloc(1, 64);
    uint8_t* m = (uint8_t*)calloc(1, 64);

    uint8_t temp[64];

    //printf("init\n");
    //p("h") printle(h, 64);
    //p("n") printle(n, 64);
    //p("s") printle(s, 64);

    //2
    while (len >= 64)
    {
        //printf("shortern iter (2)\n");
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

    //3
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
    _g_message(temp, num, h, n);
    memcpy(h, temp, 64);

    //for 512
    _g_message(temp, num, h, s);
    memcpy(h, temp, 64);

    free(m);
    free(s);
    free(n);

    return h;
}

//TODO: return new or modify?
//result to last arg
static uint8_t* _xor_block(uint8_t* res, const uint8_t* a, const uint8_t* b)
{
    uint8_t cpy[64];
    memcpy(cpy, a, 64);
    for (int i = 0; i < 64; i++)
        res[i] = a[i] ^ b[i];

    //XorArrays(res, a, b, 64);
    return res;
}

//result to last arg, no copy needed
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

//TODO: consts as 2d-array
static uint8_t* _e_block(uint8_t* res, const uint8_t* a, const uint8_t* m)
{
    //init
    uint8_t klast[64], knext[64];
    memcpy(knext, a, 64);
    memcpy(res, m, 64);

    //iters
    for (int i = 1; i <= 13; ++i)
    {
        _xor_block(res, knext, res); //TODO: always with m or with old result?
        //printf("\niter: %d\n", i);
        //p("e aft xor") printle(res, 64);
        if (i > 12)
            break;

        _lps_block(res, res);
        //p("e after l") printle(res, 64);

        memcpy(klast, knext, 64);
        _xor_block(knext, knext, kC[i-1]);
        _lps_block(knext, knext);
        //p("k after l") printle(knext, 64);
    }
    return res;
}

static uint8_t* _g_message(
        uint8_t* res, const uint8_t* n,
        const uint8_t* h, const uint8_t* m)
{
    //uint8_t t[64];
    //_xor_block(t, h, n);
    //_p_block(t, _s_block(t, t));
    //p("\ng after p") printle(t, 64);
    //_l_block(t, t);
    //p("g after l") printle(t, 64);

    _e_block(res, _lps_block(res, _xor_block(res, h, n)), m);
    _xor_block(res, h, _xor_block(res, res, m));
    //p("\ng after x") printle(res, 64);
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

/* matrix:
| 0011 |   | 1 |
| 1101 |   | 0 |
| 0001 | * | 0 | == | 0+0+0+1 1+0+0+1 0+0+0+1 1+0+0+0 | == 1011
| 1010 |   | 1 |

l[i] & a[j]????
*/

