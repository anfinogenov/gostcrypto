#ifndef ELLIPTIC_HPP
#define ELLIPTIC_HPP

#include <stdint.h>

namespace elliptic
{

void set_key(uint8_t* key, size_t size);
uint8_t* encrypt(uint8_t* data, size_t size, size_t& out_size);
uint8_t* decrypt(uint8_t* data, size_t size, size_t& out_size);

}

#endif

