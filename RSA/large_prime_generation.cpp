#include <iostream>
#include <fstream>
#include <gmp.h>

int main (int argc, char** argv) {
    int size = 2048;
    if (argc != 1) {
        size = strtol(argv[1], NULL, 10);
    }

    gmp_randstate_t random;
    gmp_randinit_default(random);
    
    std::ifstream urandf("/dev/urandom", std::ios_base::binary);
    unsigned int seed = 0;
    urandf.read((char*)&seed, sizeof(unsigned int));
    urandf.close();

    gmp_randseed_ui(random, seed);
    

    mpz_t large_prime;
    mpz_init(large_prime);
    mpz_set_ui(large_prime, 4);

    mpz_urandomb(large_prime, random, size);
    while (!mpz_probab_prime_p(large_prime, 50)) {
        mpz_nextprime(large_prime, large_prime);
    }

    char* num = nullptr;
    num = mpz_get_str(num, 10, large_prime);

    std::cout << "got prime (" << size << " bits): " << num << std::endl;

    free(num);
    return 0;
}
