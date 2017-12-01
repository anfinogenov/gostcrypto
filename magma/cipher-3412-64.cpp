#include <cstdint>
#include <algorithm>

static const uint8_t magma_key_encrypt_order[32] =
{
    0, 4, 8, 12, 16, 20, 24, 28,
    0, 4, 8, 12, 16, 20, 24, 28,
    0, 4, 8, 12, 16, 20, 24, 28,
    28, 24, 20, 16, 12, 8, 4, 0
};

static const uint8_t magma_key_decrypt_order[32] =
{
    0, 4, 8, 12, 16, 20, 24, 28,
    28, 24, 20, 16, 12, 8, 4, 0,
    28, 24, 20, 16, 12, 8, 4, 0,
    28, 24, 20, 16, 12, 8, 4, 0
};

static const uint8_t substitution[8][16] =
{
    { 12, 4, 6, 2, 10, 5, 11, 9, 14, 8, 13, 7, 0, 3, 15, 1 },
    { 6, 8, 2, 3, 9, 10, 5, 12, 1, 14, 4, 7, 11, 13, 0, 15 },
    { 11, 3, 5, 8, 2, 15, 10, 13, 14, 1, 7, 4, 12, 9, 6, 0 },
    { 12, 8, 2, 1, 13, 4, 15, 6, 7, 0, 10, 5, 3, 14, 9, 11 },
    { 7, 15, 5, 10, 8, 1, 6, 13, 0, 9, 3, 14, 11, 4, 2, 12 },
    { 5, 13, 15, 6, 9, 2, 12, 10, 11, 7, 8, 1, 4, 3, 14, 0 },
    { 8, 14, 2, 5, 6, 9, 1, 12, 15, 4, 11, 0, 13, 10, 3, 7 },
    { 1, 7, 14, 13, 0, 5, 8, 3, 4, 15, 10, 6, 9, 12, 11, 2 }
};

static uint32_t rol32_11 (uint32_t input)
{
    return (input << 11) | (input >> 21);
}

static uint32_t uint32_from_uint8_BE (uint8_t* input)
{
    uint32_t res = 0;
    res |= input[3] << 24;
    res |= input[2] << 16;
    res |= input[1] << 8;
    res |= input[0];
    return res;
}

static void uint8_from_uint32_BE (uint32_t input, uint8_t* output)
{
    for (int i = 0; i < 4; i++) output[i] = ((input >> (24-8*i)) & 0xff);
}

static uint32_t magma_round_func (uint32_t round_key, uint32_t input)
{
    uint32_t temp = round_key + input; //mod 2**32 sum
    uint32_t res = 0;

    res ^= substitution[0][  temp & 0x0000000f];
    res ^= substitution[1][((temp & 0x000000f0) >>  4)] << 4;
    res ^= substitution[2][((temp & 0x00000f00) >>  8)] << 8;
    res ^= substitution[3][((temp & 0x0000f000) >> 12)] << 12;
    res ^= substitution[4][((temp & 0x000f0000) >> 16)] << 16;
    res ^= substitution[5][((temp & 0x00f00000) >> 20)] << 20;
    res ^= substitution[6][((temp & 0x0f000000) >> 24)] << 24;
    res ^= substitution[7][((temp & 0xf0000000) >> 28)] << 28;

    return rol32_11(res);
}

static void magma_round (uint32_t* left, uint32_t* right, uint32_t round_key)
{
    uint32_t saved = *left;
    saved ^= magma_round_func(round_key, *right);
    *left = *right;
    *right = saved;
}

static void magma_block (uint8_t* data, uint8_t* key, const uint8_t* key_order)
{
    uint32_t left = uint32_from_uint8_BE(data);
    uint32_t right = uint32_from_uint8_BE(data+4);
    for (int i = 0; i < 32; i++)
    {
        magma_round(&left, &right, uint32_from_uint8_BE(key + key_order[i]));
    }
    uint8_from_uint32_BE(left, data+4);
    uint8_from_uint32_BE(right, data);
}

static uint8_t k[128] = {0};

void magma_set_key (uint8_t* key)
{
    std::copy(key, key+128, k);
}

void magma_del_key (void)
{
    for (int i = 0; i < 32*4; i++)
    {
        k[i] = 0;
    }
}

void magma_encrypt_block (uint8_t* data)
{
    magma_block(data, k, magma_key_encrypt_order);
}

void magma_decrypt_block (uint8_t* data)
{
    magma_block(data, k, magma_key_decrypt_order);
}
