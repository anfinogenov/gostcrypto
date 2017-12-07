/*
#include "./headers/cipher_3412.hpp"
#include "./headers/hash_3411.hpp"
#include "./headers/pbkdf2.hpp"
#include "./headers/hybrid.hpp"
#include "./headers/mode/ctr.hpp"
*/

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>

static unsigned char getch (void)
{
    unsigned char ch = 0;
    struct termios term_old, term_new;

    tcgetattr(STDIN_FILENO, &term_old);
    term_new = term_old;
    term_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term_new);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &term_old);
    return ch;
}

static std::string get_password (const char* output, bool show_asterisk=true)
{
    const char backspace = 127;
    const char newline = '\n';

    std::string password = "";
    unsigned char temp = 0;

    std::cout << output;

    while ((temp = getch()) != newline)
    {
        if (temp == backspace)
        {
            if (password.length() != 0)
            {
                if (show_asterisk)
                {
                    std::cout << "\b \b";
                }
                password.resize(password.length()-1);
            }
        }
        else
        {
            password += temp;
            std::cout << '*';
        }
    }
    
    return password;
}
/*
void usage (char** argv)
{
    std::cout << "Usage: " << argv[0] << std::endl;
    std::cout <<
    "" << std::endl <<
    "" << std::endl <<
    "" << std::endl;
}
*/
int main (/*int argc, char** argv*/)
{
    std::string pass = get_password("Enter password: ");
    std::cout << std::endl << "Your super-secret password: " << pass << std::endl;
    return 0;
}