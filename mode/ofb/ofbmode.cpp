#include "../../kuznyechik/cipher_3412.hpp"
#include <queue>

#define POOL_SIZE 128

std::queue<uint8_t> pool;
static uint8_t iv[32];

void init_gmm_generator (uint8_t* init, uint8_t* key)
{
    GOST3412::lib_init();
    GOST3412::set_key(key);
    std::copy(init, init+32, iv);
}

void fin_gmm_generator (void)
{
    GOST3412::lib_fin();
}

static void fill_queue (void)
{
    for (int j = 0; j < POOL_SIZE; j++) 
    {
        uint8_t tmp[16] = {0};
        std::copy(iv+16, iv+32, tmp);
        std::copy(iv, iv+16, iv+16);
        GOST3412::encrypt_block(tmp);
        std::copy(tmp, tmp+16, iv);
        for (int i = 0; i < 16; i++) 
        {
            pool.push(tmp[i]);
        }
    }   
}

uint8_t get_gmm (void)
{
    if (pool.empty())
    {
        fill_queue();
    }
    uint8_t gmm = pool.front();
    pool.front() = 0x00;
    pool.pop();
    return gmm;
}

void encrypt_block (uint8_t* data)
{
    for (int i = 0; i < 16; i++)
    {
        data[i] ^= get_gmm();
    }
}
