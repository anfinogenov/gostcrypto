#pragma once

#include <cstdint>

void magma_set_key (uint8_t* key);
void magma_del_key (void);

void magma_encrypt_block (uint8_t* data);
void magma_decrypt_block (uint8_t* data);
