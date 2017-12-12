#include "../../kuznyechik/cipher_3412.hpp"
#include <queue>

#define POOL_SIZE 1024

std::queue<uint8_t> pool;
static uint8_t iv[16];

void init_gmm_generator (uint8_t* init, uint8_t* key)
{
    GOST3412::lib_init();
    GOST3412::set_key(key);
    std::copy(init, init+16, iv);
}

void fin_gmm_generator (void)
{
    GOST3412::lib_fin();
}

static void fill_queue (void)
{
    for (int j = 0; j < POOL_SIZE; j++) 
    {
        GOST3412::encrypt_block(iv);
        for (int i = 0; i < 16; i++) 
        {
            pool.push(iv[i]);
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
    pool.pop();
    return gmm;
}
