//init vector is first block of encrypted file. It's not crypted.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "cipherstruct.h"

static struct Cipher mode_cipher;
static uint8_t mode_is_cipher_set = 0;
static uint8_t mode_is_key_set = 0;

static void mode_set_cipher(uint32_t bs, void (*setkey)(void*),
            void* (*encrypt)(void*), void* (*decrypt)(void*))
{
    mode_cipher.block_size = bs;
    mode_cipher.bs_in_bytes = (bs / 8) + ((bs % 8) ? 1 : 0);
    mode_cipher.set_key = setkey;
    mode_cipher.encrypt = encrypt;
    mode_cipher.decrypt = decrypt;

    mode_is_cipher_set = 1;
    mode_is_key_set = 0;
}

static void mode_set_key(void* key)
{
    mode_cipher.set_key(key);
    mode_is_key_set = 1;
}

static void GenerateInitVector(char*sourcefname, uint8_t* vector, uint32_t size)
{
    if (size > 32)
    {
        fprintf(
                stderr,
                "Too long init vector: %u. Reading from %s.\n",
                size,
                "/dev/urandom"
        );
        sourcefname = "/dev/urandom";
    }
    int fd = open(sourcefname, O_RDONLY);
    if (read(fd, vector, size) < size)
        fprintf(
                stderr,
                "Failed to read init vector from %s.\n",
                sourcefname
        );
}

static void XorArrays(void* result, void* op1, void* op2, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
        ((uint8_t*)result)[i] = ((uint8_t*)op1)[i] ^ ((uint8_t*)op2)[i];
}

