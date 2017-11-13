#ifndef CIPHERSTRUCT_H
#define CIPHERSTRUCT_H

#include <stdint.h>

struct Cipher
{
    uint32_t block_size;  //bits
    uint32_t bs_in_bytes;
    void (*set_key)(void*);
    void* (*encrypt)(void*);
    void* (*decrypt)(void*);
};

#endif

