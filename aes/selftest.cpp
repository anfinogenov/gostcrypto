#include "cipher_aes.cpp"
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>

static int tests_done = 0;

std::vector<uint8_t> hexstr_to_array (const char* inp) 
{
    std::vector<uint8_t> out;
    char temp[3];
    for (int i = 0; (inp[i] != 0 && inp[i+1] != 0); i += 2) 
    {
        temp[0] = inp[i];
        temp[1] = inp[i+1];
        temp[2] = 0;
        out.push_back(strtol(temp, NULL, 16));
    }
    return out;
}

void print_vec (std::ostream & s, const std::vector<uint8_t> & a, bool endl) 
{
    for (auto it = a.rbegin(); it != a.rend(); ++it) 
    {
        s << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)*it << " ";
    }
    endl ? s << std::endl : s << "- ";
}

bool vector_cmp (const std::vector<uint8_t> a, const std::vector<uint8_t> b, const char* test_name)
{
    auto ai = a.begin();
    auto bi = b.begin();
    for (; ai != a.end() && bi != b.end(); ++ai, ++bi)
    {
        if (*ai != *bi)
        {
            std::cerr << test_name << " selftest failed" << std::endl;
            std::cerr << "Expected ";
            print_vec(std::cerr, b, true);

            std::cerr << "Got      ";
            print_vec(std::cerr, a, true);

            return false;
        }
    }
    return true;
}

bool do_test (void (f)(uint8_t*),
              const std::vector<const char*> & strings, const char* init_value, const char* test_name)
{
    std::vector<uint8_t> self_vec = hexstr_to_array(init_value);
    std::vector< std::vector<uint8_t> > gost_vec;

    for (auto it = strings.begin(); it != strings.end(); ++it)
    {
        gost_vec.push_back(hexstr_to_array(*it));
    }

    for (auto it = gost_vec.begin(); it != gost_vec.end(); ++it)
    {
        f(self_vec.data());
        if (vector_cmp(*it, self_vec, test_name) == false) return false;
    }

    std::cout << ".";
    tests_done++;
    return true;
}

bool key_selftest (void)
{
    AES256::set_key(hexstr_to_array("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4").data());

    std::vector<uint8_t> nist_round_k = hexstr_to_array("603deb1015ca71be2b73aef0857d7781"
                                                        "1f352c073b6108d72d9810a30914dff4"
                                                        "9ba354118e6925afa51a8b5f2067fcde"
                                                        "a8b09c1a93d194cdbe49846eb75d5b9a"
                                                        "d59aecb85bf3c917fee94248de8ebe96"
                                                        "b5a9328a2678a647983122292f6c79b3"
                                                        "812c81addadf48ba24360af2fab8b464"
                                                        "98c5bfc9bebd198e268c3ba709e04214"
                                                        "68007bacb2df331696e939e46c518d80"
                                                        "c814e20476a9fb8a5025c02d59c58239"
                                                        "de1369676ccc5a71fa2563959674ee15"
                                                        "5886ca5d2e2f31d77e0af1fa27cf73c3"
                                                        "749c47ab18501ddae2757e4f7401905a"
                                                        "cafaaae3e4d59b349adf6acebd10190d"
                                                        "fe4890d1e6188d0b046df344706c631e");

    std::vector<uint8_t> actual_key(240);
    std::copy(AES256::round_k, AES256::round_k+240, actual_key.begin());

    if (vector_cmp(nist_round_k, actual_key, "Key derivation") == false) return false;
    AES256::del_key();
    std::cout << "Key derivation tested successfully!\n";
    return true;
}

bool block_selftest (void)
{
    AES256::set_key(hexstr_to_array("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f").data());
    std::vector<uint8_t> data = hexstr_to_array("00112233445566778899aabbccddeeff");

    std::vector<uint8_t> nist_plain = hexstr_to_array("00112233445566778899aabbccddeeff");
    std::vector<uint8_t> nist_enc = hexstr_to_array("8ea2b7ca516745bfeafc49904b496089");

    AES256::encrypt_block(data.data());
    if (vector_cmp(data, nist_enc, "encrypt block") == false) return false;

    AES256::decrypt_block(data.data());
    if (vector_cmp(data, nist_plain, "decrypt block") == false) return false;

    AES256::del_key();
    std::cout << "Block encryption/decryption tested successfully!\n";

    return true;
}

int main (void)
{
    AES256::lib_init();
    key_selftest();
    block_selftest();
}