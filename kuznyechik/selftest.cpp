#include "kuznyechik.hpp"
#include <cstdlib>
#include <iostream>
#include <iomanip>

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
    for (auto it = a.begin(); it != a.end(); ++it) 
    {
        s << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)*it << " ";
    }
    endl ? s << std::endl : s << " -  ";
}

bool s_selftest (void) 
{
    std::vector<uint8_t> gost_s[4] = 
    {
        hexstr_to_array("b66cd8887d38e8d77765aeea0c9a7efc"),
        hexstr_to_array("559d8dd7bd06cbfe7e7b262523280d39"),
        hexstr_to_array("0c3322fed531e4630d80ef5c5a81c50b"),
        hexstr_to_array("23ae65633f842d29c5df529c13f5acda")
    };

    std::vector<uint8_t> self_s = hexstr_to_array("ffeeddccbbaa99881122334455667700");
    
    for (int iter = 0; iter < 4; iter++) 
    {
        do_s(self_s, self_s);

        for (int byte = 0; byte < 16; byte++) 
        {
            if (self_s.at(byte) != gost_s[iter].at(byte)) 
            {
                std::cerr << "S selftest failed at " << byte << ": (iter " << iter << ")\n";
                std::cerr << "Expected ";
                print_vec(std::cerr, gost_s[iter], true);

                std::cerr << "Got      ";
                print_vec(std::cerr, self_s, true);

                return false;
            }
        }

        print_vec(std::cout, self_s, false);
        print_vec(std::cout, gost_s[iter], true);
    }

    std::cout << "S selftest passed" << std::endl;
    return true;
}

bool s_inv_selftest (void) 
{
    std::vector<uint8_t> gost_s[4] = 
    {
        hexstr_to_array("0c3322fed531e4630d80ef5c5a81c50b"),
        hexstr_to_array("559d8dd7bd06cbfe7e7b262523280d39"),
        hexstr_to_array("b66cd8887d38e8d77765aeea0c9a7efc"),
        hexstr_to_array("ffeeddccbbaa99881122334455667700")      
    };

    std::vector<uint8_t> self_s = hexstr_to_array("23ae65633f842d29c5df529c13f5acda");
    
    for (int iter = 0; iter < 4; iter++) 
    {
        do_inv_s(self_s, self_s);

        for (int byte = 0; byte < 16; byte++) 
        {
            if (self_s.at(byte) != gost_s[iter].at(byte)) 
            {
                std::cerr << "inv-S selftest failed at " << byte << ": (iter " << iter << ")\n";
                std::cerr << "Expected ";
                print_vec(std::cerr, gost_s[iter], true);

                std::cerr << "Got      ";
                print_vec(std::cerr, self_s, true);

                return false;
            }
        }

        print_vec(std::cout, self_s, false);
        print_vec(std::cout, gost_s[iter], true);
    }

    std::cout << "inv-S selftest passed" << std::endl;
    return true;
}

bool r_selftest (void) {
    //std::vector<uint8_t> gost_plain = hexstr_to_array("00000000000000000000000000000100");
    std::vector<uint8_t> gost_r[4] = 
    {
        hexstr_to_array("94000000000000000000000000000001"),
        hexstr_to_array("a5940000000000000000000000000000"),
        hexstr_to_array("64a59400000000000000000000000000"),
        hexstr_to_array("0d64a594000000000000000000000000")
    };

    std::vector<uint8_t> self_r = hexstr_to_array("00000000000000000000000000000100");

    for (int iter = 0; iter < 4; iter++)
    {
        do_r(self_r, self_r);

        for (int byte = 0; byte < 16; byte++)
        {
            if (self_r.at(byte) != gost_r[iter].at(byte))
            {
                std::cerr << "R selftest failed at " << byte << " byte (iter " << iter << ")\n";
                std::cerr << "Expected ";
                print_vec(std::cerr, gost_r[iter], true);

                std::cerr << "Got      ";
                print_vec(std::cerr, self_r, true);

                return false;
            }
        }

        print_vec(std::cout, self_r, false);
        print_vec(std::cout, gost_r[iter], true);
    }

    std::cout << "R selftest passed" << std::endl;
    return true;
}

bool r_inv_selftest (void) {
    std::vector<uint8_t> gost_r[4] = 
    {     
        hexstr_to_array("64a59400000000000000000000000000"),
        hexstr_to_array("a5940000000000000000000000000000"),
        hexstr_to_array("94000000000000000000000000000001"),
        hexstr_to_array("00000000000000000000000000000100")
    };

    std::vector<uint8_t> self_r = hexstr_to_array("0d64a594000000000000000000000000");

    for (int iter = 0; iter < 4; iter++)
    {
        do_inv_r(self_r, self_r);

        for (int byte = 0; byte < 16; byte++)
        {
            if (self_r.at(byte) != gost_r[iter].at(byte))
            {
                std::cerr << "inv-R selftest failed at " << byte << " byte (iter " << iter << ")\n";
                std::cerr << "Expected ";
                print_vec(std::cerr, gost_r[iter], true);

                std::cerr << "Got      ";
                print_vec(std::cerr, self_r, true);

                return false;
            }
        }

        print_vec(std::cout, self_r, false);
        print_vec(std::cout, gost_r[iter], true);
    }

    std::cout << "inv-R selftest passed" << std::endl;
    return true;
}

bool l_selftest (void) {
    std::vector<uint8_t> gost_l[4] = 
    {
        hexstr_to_array("d456584dd0e3e84cc3166e4b7fa2890d"),
        hexstr_to_array("79d26221b87b584cd42fbc4ffea5de9a"),
        hexstr_to_array("0e93691a0cfc60408b7b68f66b513c13"),
        hexstr_to_array("e6a8094fee0aa204fd97bcb0b44b8580")
    };

    std::vector<uint8_t> self_l = hexstr_to_array("64a59400000000000000000000000000");

    for (int iter = 0; iter < 4; iter++)
    {
        do_l(self_l, self_l);

        for (int byte = 0; byte < 16; byte++)
        {
            if (self_l.at(byte) != gost_l[iter].at(byte))
            {
                std::cerr << "L selftest failed at " << byte << " byte (iter " << iter << ")\n";
                std::cerr << "Expected ";
                print_vec(std::cerr, gost_l[iter], true);

                std::cerr << "Got      ";
                print_vec(std::cerr, self_l, true);

                return false;
            }
        }

        print_vec(std::cout, self_l, false);
        print_vec(std::cout, gost_l[iter], true);
    }

    std::cout << "L selftest passed" << std::endl;
    return true;
}

bool l_inv_selftest (void) {
    std::vector<uint8_t> gost_l[4] = 
    {
        hexstr_to_array("0e93691a0cfc60408b7b68f66b513c13"),
        hexstr_to_array("79d26221b87b584cd42fbc4ffea5de9a"),
        hexstr_to_array("d456584dd0e3e84cc3166e4b7fa2890d"),
        hexstr_to_array("64a59400000000000000000000000000")
    };

    std::vector<uint8_t> self_l = hexstr_to_array("e6a8094fee0aa204fd97bcb0b44b8580");

    for (int iter = 0; iter < 4; iter++)
    {
        do_inv_l(self_l, self_l);

        for (int byte = 0; byte < 16; byte++)
        {
            if (self_l.at(byte) != gost_l[iter].at(byte))
            {
                std::cerr << "inv-L selftest failed at " << byte << " byte (iter " << iter << ")\n";
                std::cerr << "Expected ";
                print_vec(std::cerr, gost_l[iter], true);

                std::cerr << "Got      ";
                print_vec(std::cerr, self_l, true);

                return false;
            }
        }

        print_vec(std::cout, self_l, false);
        print_vec(std::cout, gost_l[iter], true);
    }

    std::cout << "inv-L selftest passed" << std::endl;
    return true;
}

int main (void) 
{
    s_selftest();
    r_selftest();
    l_selftest();

    s_inv_selftest();
    r_inv_selftest();
    l_inv_selftest();
    return 0;
}
