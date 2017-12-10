#include "cipher_3412.cpp"
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
        out.insert(out.begin(), strtol(temp, NULL, 16));
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

bool do_test (void (f)(const uint8_t*, uint8_t*), 
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
        f(self_vec.data(), self_vec.data());
        if (vector_cmp(*it, self_vec, test_name) == false) return false;
    }

    std::cout << ".";
    tests_done++;
    return true;
}

bool key_selftest (void)
{
    GOST3412::set_key(hexstr_to_array("8899aabbccddeeff0011223344556677fedcba98765432100123456789abcdef").data());

    std::vector<uint8_t> gost_keys[10] = 
    {
        hexstr_to_array("8899aabbccddeeff0011223344556677"),
        hexstr_to_array("fedcba98765432100123456789abcdef"),
        hexstr_to_array("db31485315694343228d6aef8cc78c44"),
        hexstr_to_array("3d4553d8e9cfec6815ebadc40a9ffd04"),
        hexstr_to_array("57646468c44a5e28d3e59246f429f1ac"),
        hexstr_to_array("bd079435165c6432b532e82834da581b"),
        hexstr_to_array("51e640757e8745de705727265a0098b1"),
        hexstr_to_array("5a7925017b9fdd3ed72a91a22286f984"),
        hexstr_to_array("bb44e25378c73123a5f32f73cdb6e517"),
        hexstr_to_array("72e9dd7416bcf45b755dbaa88e4a4043")
    };

    for (int i = 0; i < 10; i++)
    {
        std::vector<uint8_t> key_to_test(16);
        std::copy(GOST3412::k[i], GOST3412::k[i]+16, key_to_test.begin());
        if (vector_cmp(key_to_test, gost_keys[i], "Key derivation") == false) continue;
    }

    GOST3412::del_key();
    std::cout << "Key derivation tested successfully!\n";
    return true;
}

bool block_selftest (void)
{
    GOST3412::set_key(hexstr_to_array("8899aabbccddeeff0011223344556677fedcba98765432100123456789abcdef").data());

    std::vector<uint8_t> data = hexstr_to_array("1122334455667700ffeeddccbbaa9988");

    std::vector<uint8_t> gost_plain = hexstr_to_array("1122334455667700ffeeddccbbaa9988");
    std::vector<uint8_t> gost_enc = hexstr_to_array("7f679d90bebc24305a468d42b9d4edcd");

    GOST3412::encrypt_block(data.data());
    if (vector_cmp(data, gost_enc, "encrypt block") == false) return false;
    
    GOST3412::decrypt_block(data.data());
    if (vector_cmp(data, gost_plain, "decrypt block") == false) return false;

    GOST3412::del_key();
    std::cout << "Block encryption\\decryption tested successfully!\n";
    return true;
}

void speed_test(void)
{
    GOST3412::set_key(hexstr_to_array("8899aabbccddeeff0011223344556677fedcba98765432100123456789abcdef").data());
    std::vector<uint8_t> sample_block_vec = hexstr_to_array("aabbccddeeff00112233445566778899");
    uint8_t sample_block[16];
    std::copy(sample_block_vec.begin(), sample_block_vec.end(), sample_block);
    sample_block_vec.clear();

    auto start = std::chrono::high_resolution_clock::now();
    int iterations = 400000;
    for (int i = 0; i < iterations; i++)
    {
        GOST3412::encrypt_block(sample_block);
    }
    auto finish = std::chrono::high_resolution_clock::now();

    GOST3412::del_key();
    std::cout << "Speed test: " <<
    16.0*iterations/std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count()*953.67 << 
    "MB/sec." << std::endl;
}

int main (void) 
{
    GOST3412::lib_init();
    
    do_test(GOST3412::do_s, std::vector<const char*> {
        "b66cd8887d38e8d77765aeea0c9a7efc",
        "559d8dd7bd06cbfe7e7b262523280d39",
        "0c3322fed531e4630d80ef5c5a81c50b",
        "23ae65633f842d29c5df529c13f5acda"}, 
        "ffeeddccbbaa99881122334455667700", "S");

    do_test(GOST3412::do_inv_s, std::vector<const char*> {
        "0c3322fed531e4630d80ef5c5a81c50b",
        "559d8dd7bd06cbfe7e7b262523280d39",
        "b66cd8887d38e8d77765aeea0c9a7efc",
        "ffeeddccbbaa99881122334455667700"},
        "23ae65633f842d29c5df529c13f5acda", "S-inv");

    do_test(GOST3412::do_r, std::vector<const char*> {
        "94000000000000000000000000000001",
        "a5940000000000000000000000000000",
        "64a59400000000000000000000000000",
        "0d64a594000000000000000000000000"},
        "00000000000000000000000000000100", "R");

    do_test(GOST3412::do_inv_r, std::vector<const char*> {
        "64a59400000000000000000000000000",
        "a5940000000000000000000000000000",
        "94000000000000000000000000000001",
        "00000000000000000000000000000100"},
        "0d64a594000000000000000000000000", "R-inv");

    do_test(GOST3412::do_l, std::vector<const char*> {
        "d456584dd0e3e84cc3166e4b7fa2890d",
        "79d26221b87b584cd42fbc4ffea5de9a",
        "0e93691a0cfc60408b7b68f66b513c13",
        "e6a8094fee0aa204fd97bcb0b44b8580"},
        "64a59400000000000000000000000000", "L");

    do_test(GOST3412::do_inv_l, std::vector<const char*> {
        "0e93691a0cfc60408b7b68f66b513c13",
        "79d26221b87b584cd42fbc4ffea5de9a",
        "d456584dd0e3e84cc3166e4b7fa2890d",
        "64a59400000000000000000000000000"},
        "e6a8094fee0aa204fd97bcb0b44b8580", "L-inv");

    std::cout << std::endl << tests_done << " primitives tested successfully!\n";

    key_selftest();
    block_selftest();

    speed_test();

    GOST3412::lib_fin();
    return 0;
}
