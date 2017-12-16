#include "./kuznyechik/cipher_3412.hpp"
#include "./hash/headers/hash_3411.h"
#include "./pbkdf2/headers/pbkdf2.h"
#include "./elliptic/headers/elliptic.hpp"
#include "./mode/ofb/ofbmode.hpp"

#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#define KEY_RNG_SIZE 16*1024
#define BLOCKSIZE 1024

struct globalargs_t {
    bool encrypt = true;
    bool archiver_verbose = false; // -v, --verbose
    bool archiver_skip_dot = true;
    bool password_asterisks = false; // -s, --show
    std::string out_filename = std::string("out.eak"); // -o, --output
    std::vector<std::string> input_files; // -i, --input
    unsigned number_of_input_files = 0;
} global_args;

static const struct option long_opts[] =
{
    { "encrypt", no_argument, NULL, 'e' },
    { "decrypt", no_argument, NULL, 'd' },
    { "all", no_argument, NULL, 'a' },
    { "verbose", no_argument, NULL, 'v' },
    { "show", no_argument, NULL, 's' },
    { "output", required_argument, NULL, 'o' },
    { "input", required_argument, NULL, 'i' },
    { "help", no_argument, NULL, 'h'},
    { NULL, no_argument, NULL, 0 }
};

static const char* opt_string = "edavsi:o:h?";

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

void abort (std::string msg)
{
    std::cout << "Abort: " << msg << std::endl;
    exit(EXIT_FAILURE);
}

void write_file_entry (const std::string & filename, std::ofstream & fout, const struct stat filestat, size_t blocksize) 
{
    std::ifstream fin(filename, std::ios_base::binary | std::ios_base::in);

    fout.write(filename.c_str(), filename.length()+1);          // direntry name
    fout.write((char*)&(filestat.st_size), sizeof(off_t));      // direntry size (in bytes)
    fout.write((char*)&(filestat.st_mode), sizeof(mode_t));     // direntry mode (dir, file etc.)

    do 
    {
        char* buf = new char[blocksize];                        // create new buffer for optimized IO
        fin.read(buf, blocksize);
        for (int i = 0; i < blocksize; i++) buf[i] ^= get_gmm();
        fout.write(buf, fin.gcount());                          // write block to archive
        delete[] buf;
    }
    while (fin);

    fin.close();
}

void scan_dir (const std::string & dirname, std::ofstream & fout, bool skip_dot, bool verbose) 
{
    DIR* dir = nullptr;
    struct dirent entry;
    struct dirent *entryptr = nullptr;
    int retval = 0;

    dir = opendir(dirname.c_str());
    if (dir == nullptr) 
    {
        std::cerr << "Error opening directory " << dirname << ": " << strerror(errno) << std::endl;
        return;
    }
    if (verbose) std::cout << "Found directory: " <<  dirname << std::endl;

    // get entries from directory
    while (!(retval = readdir_r(dir, &entry, &entryptr))) 
    {
        if (entryptr == nullptr) break;

        struct stat entryinfo;
        char* full_name = new char[PATH_MAX];
        // concat dirname and entryname for lstat() and recursive call of scan_dir()
        snprintf(full_name, PATH_MAX, "%s/%s", dirname.c_str(), entry.d_name);
        lstat(full_name, &entryinfo);

        // skipping . and .. entries
        if (strncmp(entry.d_name, ".", PATH_MAX) && strncmp(entry.d_name, "..", PATH_MAX)) 
        {
            // if skip_dot flag set, then skipping entries starts with dot
            if (skip_dot ? (strncmp(entry.d_name, ".", 1) != 0) : true) 
            {
                // print info about entry
                if (verbose) printf("%8lld %s\n", entryinfo.st_size, full_name);
                write_file_entry(full_name, fout, entryinfo, BLOCKSIZE);
                
                // if entry is a directory, then recursive call for this directory
                if (S_ISDIR(entryinfo.st_mode)) 
                    scan_dir(full_name, fout, skip_dot, verbose);
            }
        }
        delete[] full_name;
    }
    closedir(dir);
}

void pack (std::ofstream & fout)
{
    for (int i = 0; i < global_args.number_of_input_files; i++)
    {
        DIR* dir;
        dir = opendir(global_args.input_files.at(i).c_str());
        if (dir == nullptr) 
        {
            if (global_args.archiver_verbose)
            {
                std::cerr << "Error opening directory " << global_args.input_files[i] << ": " << strerror(errno) << std::endl;
                std::cerr << "Trying " << global_args.input_files[i] << " as file...";
            }

            std::ifstream fin(global_args.input_files[i], std::ios_base::out | std::ios_base::binary);
            if (!fin)
            {
                if (global_args.archiver_verbose) std::cerr << "failed :(\n";
                continue;
            }
            else
            {
                if (global_args.archiver_verbose) std::cerr << "success!" << std::endl;
                fin.close();
                struct stat entry_stat;
                lstat(global_args.input_files.at(i).c_str(), &entry_stat);
                write_file_entry(global_args.input_files[i], fout, entry_stat, BLOCKSIZE);
            }
        }
        else
        {
            if (global_args.archiver_verbose) printf("Found directory: %s\n", global_args.input_files.at(i).c_str());
            struct stat entry_dir_stat;
            lstat(global_args.input_files.at(i).c_str(), &entry_dir_stat);
            write_file_entry(global_args.input_files[i], fout, entry_dir_stat, BLOCKSIZE);
            scan_dir(global_args.input_files[i], fout, global_args.archiver_skip_dot, global_args.archiver_verbose);
        }
    }
}

void unpack (std::ifstream & fin) 
{
    while (fin) 
    {
        char* name_buf = new char[PATH_MAX];
        fin.getline(name_buf, PATH_MAX, '\0');
        off_t size = 0;
        mode_t mode = 0;
        fin.read((char*)&size, sizeof(off_t));
        fin.read((char*)&mode, sizeof(mode_t));

        if (size == 0 && mode == 0) 
        {
            delete[] name_buf;
            return;
        }

        std::cout << name_buf << ": " << size << "B, mode: " << std::oct << mode << std::endl;

        if (S_ISDIR(mode)) mkdir(name_buf, mode); 
        else {
            std::ofstream fout (name_buf, std::ios_base::binary | std::ios_base::out);
            chmod(name_buf, mode);

            while (size > BLOCKSIZE) 
            {
                char* read_buf = new char [BLOCKSIZE];
                fin.read(read_buf, BLOCKSIZE);
                for (int i = 0; i < BLOCKSIZE; i++) read_buf[i] ^= get_gmm();
                fout.write(read_buf, BLOCKSIZE);
                size -= BLOCKSIZE;
                delete[] read_buf;
            }

            char* read_buf = new char [size];
            fin.read(read_buf, size);
            for (int i = 0; i < size; i++) read_buf[i] ^= get_gmm();
            fout.write(read_buf, size);
            delete[] read_buf;

            fout.close();
        }

        delete[] name_buf;
    }
}

void get_params (int & argc, char** argv)
{
    int long_index;
    int opt = getopt_long (argc, argv, opt_string, long_opts, &long_index);
    while (opt != -1)
    {
        switch (opt)
        {
            case 'e':
                global_args.encrypt = true;
                break;
            case 'd':
                global_args.encrypt = false;
                break;
            case 'v':
                global_args.archiver_verbose = true;
                break;
            case 's':
                global_args.password_asterisks = true;
                break;
            case 'o':
                global_args.out_filename = std::string(argv[optind-1]);
                break;
            case 'a':
                global_args.archiver_skip_dot = false;
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

    for (int i = optind; i < argc; i++)
    {
        global_args.number_of_input_files++;
        global_args.input_files.push_back(std::string(argv[i]));
    }
}

void generate_256_random (uint8_t* out, std::ifstream & random_source)
{
    uint8_t* temp_data = new uint8_t[KEY_RNG_SIZE];
    random_source.read((char*)temp_data, KEY_RNG_SIZE);
    out = GOST3411::hash_256(temp_data, KEY_RNG_SIZE);
    for (int i = 0; i < KEY_RNG_SIZE; i++) temp_data[i] = 0x00;
    delete[] temp_data;
}

void print_256bit_BE (uint8_t* vec)
{
    for (int i = 31; i >= 0; i--)
    {
        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)vec[i];
    }
    std::cout << std::endl;
}

int main (int argc, char** argv)
{
    if (argc == 1) print_usage(argv);

    get_params(argc, argv);

    if (!global_args.encrypt && global_args.number_of_input_files > 1)
    {
        abort("Too many files to decrypt!");
    }

    if (global_args.number_of_input_files < 1)
    {
        abort("No files provided!");
    }

    std::string pass = get_password("Enter password: ", global_args.password_asterisks);
    std::cout << std::endl;

    uint8_t* IV;
    uint8_t* KEY_KUZ;
    //uint8_t* KEY_HMAC;

    /* Temporary solution for encryption (until signing is not ready) */
    KEY_KUZ = pbkdf2(GOST3411::hmac_256, 32, (uint8_t*)pass.c_str(), pass.length(), (uint8_t*)"salt", 4, 1000, 32);
    print_256bit_BE(KEY_KUZ);
    IV = new uint8_t[32];
    for (int i = 0; i < 32; i++) IV[i] = i;

    /*
     * This code generates pseudo-random keys for all parts of scheme,
     * uncomment when asymmetric part will be ready
     */

    /*
    std::ifstream random_source("/dev/urandom", std::ios::binary | std::ios::in);
    generate_256_random(KEY_KUZ, random_source);
    print_256bit_BE(KEY_KUZ);

    generate_256_random(IV, random_source);
    print_256bit_BE(IV);

    generate_256_random(KEY_HMAC, random_source);
    print_256bit_BE(KEY_HMAC);
    random_source.close();
    */

    init_gmm_generator(KEY_KUZ, IV);

    /* Encrypt */
    if (global_args.encrypt) {
        //TODO: ask about file rewrite
        std::ofstream fout(global_args.out_filename, std::ios_base::out | std::ios_base::binary);
        pack(fout);
        fout.close();
    }

    /* Decrypt */
    else 
    {
        std::ifstream fin(global_args.input_files.at(0), std::ios_base::binary | std::ios_base::in);
        unpack(fin);
        fin.close();
    }

    /* Plan:
     * Create random key and IV for OFB \done
     * Call archiver with getopt_long flags \done
     * Archiver should write enrypted data to out-file \done
     * This time some elliptic and wind(streebog) magic
     * (??)(??)encrypted_data(??) is output
     */

    //TODO: encrypt file headers

    exit(EXIT_SUCCESS);
}
