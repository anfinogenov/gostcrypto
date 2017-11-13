//init vector is first block of encrypted file. It's not crypted.

#ifndef CTRMODE_H
#define CTRMODE_H

#include <stdint.h>

//assigning cipher functions
void ctr_set_cipher(
        uint32_t bs,                //cipher block size (bits)
        void (*setkey)(void*),      //key setting function
        void* (*encrypt)(void*),    //encrypt block function
        void* (*decrypt)(void*)     //decrypt block function
);
void ctr_set_key(void* key);        //simply calls cipher setkey
void* ctr_encrypt(void* message, uint32_t size, uint32_t* enc_size);
void* ctr_decrypt(void* message, uint32_t size, uint32_t* dec_size);
void ctr_encrypt_file(FILE* in, FILE* out);     //encrypt file
void ctr_decrypt_file(FILE* in, FILE* out);
//TODO: ctr_encrypt_next - encrypts next block w/o losing counter

#endif

