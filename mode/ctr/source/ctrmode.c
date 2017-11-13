#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "../../headerscommon/cipherstruct.h"
#include "../headers/ctrmode.h"

#define DEBUG

#include "../../headerscommon/functionscommon.h"

//init vector has lenght == block length / 2.
//counter has size equal to size of block: higest bits == init vector,
//lowest bits == 0
//next counter value is counter++ (% (2^block_length))

//inner
static void* ctr_crypt(void* message, uint32_t size, uint8_t iscrypting);
static void ctr_crypt_file(FILE* in, FILE* out, uint8_t iscrypting);
static void IncreaseCounter(uint8_t* counter, uint32_t size);
static uint8_t IncreaseByteCarry(uint8_t* byte);

//from where to read init vector
static char* ctr_vector_source =
#ifdef DEBUG
    "/dev/urandom";
#else
    "/dev/random";
#endif

//call same for all modes function
void ctr_set_cipher(uint32_t bs, void (*setkey)(void*),
            void* (*encrypt)(void*), void* (*decrypt)(void*))
{ mode_set_cipher(bs, setkey, encrypt, decrypt); }

void ctr_set_key(void* key)
{ mode_set_key(key); }

//encrypt array
void* ctr_encrypt(void* message, uint32_t size, uint32_t* enc_size)
{
    void* res = ctr_crypt(message, size, 1);
    *enc_size = size + mode_cipher.bs_in_bytes;
    return res;
}

void* ctr_decrypt(void* message, uint32_t size, uint32_t* dec_size)
{
    void* res = ctr_crypt(message, size, 0);
    *dec_size = size - mode_cipher.bs_in_bytes;
    return res;
}

static void* ctr_crypt(void* message, uint32_t size, uint8_t iscrypting)
{
    uint8_t* vector = (uint8_t*)calloc(1, mode_cipher.bs_in_bytes);
    uint8_t* res;
    uint32_t offset_res = 0;

    if (!iscrypting)
    {
        memcpy(vector, message, mode_cipher.bs_in_bytes);
        message = ((uint8_t*)message) + mode_cipher.bs_in_bytes;
        size -= mode_cipher.bs_in_bytes;
        res = (uint8_t*)malloc(size);
    }
    else
    {
        GenerateInitVector(
                ctr_vector_source,
                vector + mode_cipher.bs_in_bytes / 2,
                mode_cipher.bs_in_bytes / 2);

        offset_res = mode_cipher.bs_in_bytes;
        res = (uint8_t*)malloc(size + offset_res);
        memcpy(res, vector, mode_cipher.bs_in_bytes);
    }

    uint32_t blockscount = size / mode_cipher.bs_in_bytes;
    if (size % mode_cipher.bs_in_bytes != 0) blockscount++;

    for (uint32_t i = 0; i < blockscount; i++)
    {
        void* gamma = mode_cipher.encrypt(vector);
        uint32_t bytestoxor = (size < mode_cipher.bs_in_bytes)
            ? size
            : mode_cipher.bs_in_bytes;
        XorArrays((res+offset_res), message, gamma, bytestoxor);
        IncreaseCounter(vector, mode_cipher.bs_in_bytes);
        offset_res += mode_cipher.bs_in_bytes;
        message = ((uint8_t*)message) + mode_cipher.bs_in_bytes;
        size -= mode_cipher.bs_in_bytes;
    }
    return res;
    // ???
}

void ctr_encrypt_file(FILE* in, FILE* out)
{
    ctr_crypt_file(in, out, 1);
}

void ctr_decrypt_file(FILE* in, FILE* out)
{
    ctr_crypt_file(in, out, 0);
}

static void ctr_crypt_file(FILE* in, FILE* out, uint8_t iscrypting)
{
    uint8_t* vector = (uint8_t*)calloc(1, mode_cipher.bs_in_bytes);

    if (!iscrypting)
    {
        int res = fread(vector, 1, mode_cipher.bs_in_bytes, in);
        if (res < mode_cipher.bs_in_bytes)
        {
            fprintf(
                    stderr,
                    "encrypted file is shorter then init vector size\n");
            return;
        }
   }
    else
    {
        GenerateInitVector(
                ctr_vector_source,
                vector + mode_cipher.bs_in_bytes / 2,
                mode_cipher.bs_in_bytes / 2);

        if (fwrite(vector, 1, mode_cipher.bs_in_bytes, out)
        != mode_cipher.bs_in_bytes)
        {
            fprintf(stderr, "Failed to write init vector\n");
            return;
        }
    }

    uint8_t* block = (uint8_t*)calloc(1, mode_cipher.bs_in_bytes);
    while (!feof(in))
    {
        void* gamma = mode_cipher.encrypt(vector);
        uint32_t bytestoxor = fread(block, 1, mode_cipher.bs_in_bytes, in);

        XorArrays(block, block, gamma, bytestoxor);
        IncreaseCounter(vector, mode_cipher.bs_in_bytes);
        fwrite(block, 1, bytestoxor, out);
    }
    // ???
}

//TODO: prepare for multithreading

static void IncreaseCounter(uint8_t* counter, uint32_t size)
{
    uint8_t carry = IncreaseByteCarry(counter);
    for (int i = 1; i < size; i++)
    {
        if (carry)
            carry = IncreaseByteCarry(counter+i);
    }
}

static uint8_t IncreaseByteCarry(uint8_t* byte)
{
    uint8_t old = *byte;
    uint8_t carry = 0;
    if (++(*byte) < old)
        carry = 1;
    return carry;
}

