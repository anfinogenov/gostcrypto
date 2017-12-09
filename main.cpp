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
#include <getopt.h>

struct globalargs_t {
    bool archiver_verbose = false; // -v, --verbose
    bool password_asterisks = false; // -s, --show
    char* out_filename; // -o, --output
    char** input_files; // -i, --input
    unsigned number_of_input_files = 0;
} global_args;

static const struct option long_opts[] =
{
    { "verbose", no_argument, NULL, 'v' },
    { "show", no_argument, NULL, 's' },
    { "output", optional_argument, NULL, 'o' },
    { "input", required_argument, NULL, 'i' },
    { "help", no_argument, NULL, 'h'},
    { NULL, no_argument, NULL, 0 }
};

static const char* opt_string = "vsi:o:h?";

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
                    std::cout << "\b \b";
                password.resize(password.length()-1);
            }
        }
        else
        {
            password += temp;
            if (show_asterisk)
                std::cout << '*';
        }
    }
    
    return password;
}

void print_usage (char** argv)
{
    std::cout << "Usage: " << argv[0] << std::endl;
    std::cout <<
    "-s (--show): Show password symbols as * when typing" << std::endl <<
    "-v (--verbose): Enable verbose output for archiver" << std::endl <<
    "" << std::endl;

    exit(EXIT_FAILURE);
}

int main (int argc, char** argv)
{
    int long_index;
    int opt = getopt_long (argc, argv, opt_string, long_opts, &long_index);
    while (opt != -1)
    {
        switch (opt)
        {
            case 'v':
                global_args.archiver_verbose = true;
                break;
            case 's':
                global_args.password_asterisks = true;
                break;
            case 'h':
            case '?':
                print_usage(argv);
                break;
            default: 
                break;
        }
        opt = getopt_long (argc, argv, opt_string, long_opts, &long_index);
    }

    std::string pass = get_password("Enter password: ", global_args.password_asterisks);
    std::cout << std::endl;
    std::cout << "Password demo. ";
    std::cout << "Your super-secret password: " << pass << std::endl;
    
    exit(EXIT_SUCCESS);
}
