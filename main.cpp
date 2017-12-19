#include "./kuznyechik/cipher_3412.hpp"
#include "./hash/headers/hash_3411.h"
#include "./pbkdf2/headers/pbkdf2.h"
#include "./elliptic/headers/elliptic.hpp"
#include "./mode/ofb/ofbmode.hpp"

#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <cstring>
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
    { "decrypt", required_argument, NULL, 'd' },
    { "all", no_argument, NULL, 'a' },
    { "verbose", no_argument, NULL, 'v' },
    { "show", no_argument, NULL, 's' },
    { "output", required_argument, NULL, 'o' },
    { "help", no_argument, NULL, 'h'},
    { NULL, no_argument, NULL, 0 }
};

static const char* opt_string = "ed:avso:h?";

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
    std::cout << "Usage: " << argv[0] << " [-svedh] [-o <file>] source_file ..." << std::endl;
    std::cout <<
    "-s (--show): Show password symbols as * when typing" << std::endl <<
    "-v (--verbose): Enable verbose output for archiver" << std::endl <<
    "-e (--encrypt): Create encrypted archive (enabled by default)" << std::endl <<
    "-d (--decrypt): Unpack and decrypt archive" << std::endl <<
    "-o (--output) <file>: Write data to file <file>" << std::endl <<
    "    When not specified, creates archive 'out.eak'" << std::endl <<
    "-h (--help): Show this help page" << std::endl <<
    "" << std::endl;

    exit(EXIT_FAILURE);
}

void abort (const std::string & msg)
{
    std::cout << "Abort: " << msg << std::endl;
    exit(EXIT_FAILURE);
}

void write_file_entry (const std::string & filename, 
                       std::ofstream & fout, 
                       const struct stat filestat, 
                       size_t blocksize) 
{
    std::ifstream fin(filename, std::ios_base::binary | std::ios_base::in);

    std::string filename_encrypted;
    for (auto it = filename.begin(); it != filename.end(); it++)
        filename_encrypted += *it ^ get_gmm();
    filename_encrypted += get_gmm();

    GOST3411::hmac_256_append((uint8_t*)filename.c_str(), filename.length()+1, 0);
    fout.write(filename_encrypted.c_str(), filename.length()+1);          // direntry name

    GOST3411::hmac_256_append((uint8_t*)&filestat.st_size, sizeof(off_t), 0);
    char st_size_encrypted[sizeof(off_t)];
    for (int i = 0; i < sizeof(off_t); i++)
        st_size_encrypted[i] = ((filestat.st_size >> 8*i) & 0xFF) ^ get_gmm();
    fout.write(st_size_encrypted, sizeof(off_t));      // direntry size (in bytes)
    
    GOST3411::hmac_256_append((uint8_t*)&filestat.st_mode, sizeof(mode_t), 0);
    char st_mode_encrypted[sizeof(mode_t)];
    for (int i = 0; i < sizeof(mode_t); i++)
        st_mode_encrypted[i] = ((filestat.st_mode >> 8*i) & 0xFF) ^ get_gmm();
    fout.write(st_mode_encrypted, sizeof(mode_t));     // direntry mode (dir, file etc.)

    do
    {
        char* buf = new char[blocksize];                        // create new buffer for optimized IO
        fin.read(buf, blocksize);
        GOST3411::hmac_256_append((uint8_t*)buf, fin.gcount(), 0);
        for (uint32_t i = 0; i < fin.gcount(); i++) buf[i] ^= get_gmm();
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

void pack (std::ofstream & fout, uint8_t* hmac_out)
{
    for (uint32_t i = 0; i < global_args.number_of_input_files; i++)
    {
        DIR* dir;
        dir = opendir(global_args.input_files.at(i).c_str());
        if (dir == nullptr) 
        {
            if (global_args.archiver_verbose)
            {
                std::cerr << "Error opening directory " << global_args.input_files.at(i) << ": " << strerror(errno) << std::endl;
                std::cerr << "Trying " << global_args.input_files.at(i) << " as file...";
            }

            std::ifstream fin(global_args.input_files.at(i), std::ios_base::out | std::ios_base::binary);
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
                write_file_entry(global_args.input_files.at(i), fout, entry_stat, BLOCKSIZE);
            }
        }
        else
        {
            if (global_args.archiver_verbose) printf("Found directory: %s\n", global_args.input_files.at(i).c_str());
            struct stat entry_dir_stat;
            lstat(global_args.input_files.at(i).c_str(), &entry_dir_stat);
            write_file_entry(global_args.input_files.at(i), fout, entry_dir_stat, BLOCKSIZE);
            scan_dir(global_args.input_files.at(i), fout, global_args.archiver_skip_dot, global_args.archiver_verbose);
        }
    }
    uint8_t* hmac = GOST3411::hmac_256_append((uint8_t*)"", 0, 1);
    std::copy(hmac, hmac+32, hmac_out);
}

void unpack (std::ifstream & fin, uint8_t* hmac_out) 
{
    while (fin) 
    {
        char* name_buf = new char[PATH_MAX];
        int i = 0;
        char tmp;
        do
        {
            fin.read(&tmp, 1);
            tmp ^= get_gmm();
            name_buf[i++] = tmp;
        } while (tmp != 0 && fin.good());
        if (!fin.good()) break;
        GOST3411::hmac_256_append((uint8_t*)name_buf, i, 0);

        //fin.getline(name_buf, PATH_MAX, '\0');
        off_t size = 0;
        mode_t mode = 0;
        fin.read((char*)&size, sizeof(off_t));
        fin.read((char*)&mode, sizeof(mode_t));
        if (!fin.good()) break;

        for (i = 0; i < sizeof(off_t); i++)
        {
            ((uint8_t*)(&size))[i] ^= get_gmm();
        }

        for (i = 0; i < sizeof(mode_t); i++)
        {
            ((uint8_t*)(&mode))[i] ^= get_gmm();
        }

        if (size == 0 && mode == 0) 
        {
            delete[] name_buf;
            break;
        }

        GOST3411::hmac_256_append((uint8_t*)&size, sizeof(off_t), 0);
        GOST3411::hmac_256_append((uint8_t*)&mode, sizeof(mode_t), 0);

        std::cout << name_buf << ": " << std::dec << size << " bytes, mode: " << std::oct << mode << std::endl;

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
                GOST3411::hmac_256_append((uint8_t*)read_buf, BLOCKSIZE, 0);
                delete[] read_buf;
            }

            char* read_buf = new char [size];
            fin.read(read_buf, size);
            for (int i = 0; i < size; i++) read_buf[i] ^= get_gmm();
            fout.write(read_buf, size);
            GOST3411::hmac_256_append((uint8_t*)read_buf, size, 0);
            delete[] read_buf;

            fout.close();
        }

        delete[] name_buf;
    }
    uint8_t* hmac = GOST3411::hmac_256_append((uint8_t*)"", 0, 1);
    std::copy(hmac, hmac+32, hmac_out);
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
                global_args.number_of_input_files++;
                global_args.input_files.push_back(std::string(argv[optind-1]));
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
    uint8_t* hash = GOST3411::hash_256(temp_data, KEY_RNG_SIZE);
    std::copy(hash, hash+32, out);
    for (int i = 0; i < KEY_RNG_SIZE; i++) temp_data[i] = 0x00;
    for (int i = 0; i < 32; i++) hash[i] = 0x00;
    delete[] temp_data;
}

void generate_keys (uint8_t* k1, uint8_t* k2, uint8_t* k3)
{
    std::ifstream random_source("/dev/urandom", std::ios::binary | std::ios::in);
    generate_256_random(k1, random_source);
    generate_256_random(k2, random_source);
    generate_256_random(k3, random_source);
    random_source.close();
}

void print_256bit_BE (uint8_t* vec)
{
    for (int i = 31; i >= 0; i--)
    {
        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)vec[i];
    }
    std::cout << std::endl;
}

void measure_speed (void)
{
    GOST3412::lib_init();
    uint8_t key[32] = {0};
    GOST3412::set_key(key);
    uint8_t sample_block[16] = {0};

    auto start = std::chrono::high_resolution_clock::now();
    int iterations = 4000;
    for (int i = 0; i < iterations; i++)
    {
        GOST3412::encrypt_block(sample_block);
    }
    auto finish = std::chrono::high_resolution_clock::now();

    std::cout << "Estimated speed: " <<
    16.0*iterations/std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count()*953.67 << 
    "MB/sec." << std::endl;

    GOST3412::del_key();
    GOST3412::lib_fin();
}

bool check_exists (std::string & name)
{
    std::ifstream fin(name, std::ios_base::in);
    if (fin.good())
    {
        fin.close();
        return true;
    }
    return false;
}

int write_header(const size_t & data_size, uint8_t* data, std::ofstream & fout, const size_t & reserved)
{
    fout.write((char*)&data_size, sizeof(size_t));
    fout.write((char*)data, data_size);
    int reserved_pos = fout.tellp();
    fout.seekp(reserved_pos + reserved, fout.beg);
    return reserved_pos;
}

void write_hmac(const size_t & data_size, const int whence, uint8_t* data, std::ofstream & fout)
{
    fout.seekp(whence, fout.beg);
    fout.write((char*)data, data_size);
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

    measure_speed();

    std::string pass = "";
    do
    {
        pass = get_password("Enter password (8-64): ", global_args.password_asterisks);
        std::cout << std::endl;
    }
    while (pass.length() < 8 || pass.length() >= 64);

    std::cout << "Calculating elliptic key...\n";
    uint8_t* elliptic_key = pbkdf2(hmac_generate_512, 64, (uint8_t*)pass.c_str(), pass.length(), (uint8_t*)"salt", 4, 10000, 64);
    if (elliptic_key == nullptr)
    {
        abort("Some error get after password-based key derivation");
    }
    elliptic::set_key(elliptic_key, 32);

    /* Encrypt */
    if (global_args.encrypt == true) {

        if (check_exists(global_args.out_filename))
        {
            std::cout << "\x1B[101;97mAttention! File \x1B[44;30m " << global_args.out_filename << 
                         " \x1B[101;97m exists. Continue?\x1B[0m y/n " << std::endl;
            char choice = std::cin.get();
            if (tolower(choice) != 'y')
            {
                abort("File exists!");
            }
        }

        uint8_t IV[32], KEY_KUZ[32], KEY_HMAC[32];
        std::cout << "Generating 3 256-bit keys...\n";
        generate_keys(IV, KEY_KUZ, KEY_HMAC);
        
        std::cout << "Encrypting keys to header using password...\n";
        uint8_t keys_concat[96];
        for (int i = 0; i < 32; i++)
        {
            keys_concat[i] = IV[i];
            keys_concat[32+i] = KEY_KUZ[i];
            keys_concat[64+i] = KEY_HMAC[i];
        }

        size_t header_size = 0;
        uint8_t* header = elliptic::encrypt(keys_concat, 96, header_size);
        if (header == nullptr)
        {
            abort("Elliptic encryption failed");
        }
        std::cout << "Keys encrypted!\n";
        std::cout << "OFB initialization...\n";
        init_gmm_generator(KEY_KUZ, IV);

        std::cout << "Opening file " << global_args.out_filename << "...\n";
        std::ofstream fout(global_args.out_filename, std::ios_base::out | std::ios_base::binary);

        size_t hmac_size = 32;
        int hmac_place = write_header(header_size, header, fout, hmac_size);
        GOST3411::hmac_set_key_256(KEY_HMAC, 32);
        uint8_t hmac[32];

        std::cout << "Encrypting...\n";
        pack(fout, hmac);
        std::cout << "HMAC: ";
        print_256bit_BE(hmac);

        std::cout << "Writing HMAC to archive...\n";
        write_hmac(hmac_size, hmac_place, hmac, fout);
        fout.close();

        std::cout << "Job's done!\n";
    }

    /* Decrypt */
    else 
    {
        std::cout << "Archive name: " << global_args.input_files.at(0) << std::endl;
        std::cout << "Opening " << global_args.input_files.at(0) << "...\n";
        std::ifstream fin(global_args.input_files.at(0), std::ios_base::binary | std::ios_base::in);

        size_t header_size_encrypted = 0, header_size = 0;
        fin.read((char*)&header_size_encrypted, sizeof(size_t));

        uint8_t* header_encrypted = new uint8_t[header_size_encrypted];
        fin.read((char*)header_encrypted, header_size_encrypted);
        
        uint8_t* header = elliptic::decrypt(header_encrypted, header_size_encrypted, header_size);
        if (header == nullptr)
        {
            abort("Elliptic decryption failure. Incorrect password?");
        }
        if (header_size != 96) abort("Header size failure");

        std::cout << "Setting keys...";
        init_gmm_generator(header+32, header);
        GOST3411::hmac_set_key_256(header+64, 32);
        uint8_t hmac[32], hmac_read[32];
        fin.read((char*)&hmac_read, 32);

        std::cout << "Keys set! Decryting...\n";
        unpack(fin, hmac);

        fin.close();

        for (int i = 0; i < 32; i++)
        {
            if (hmac[i] != hmac_read[i]) 
            {
                std::cout << "Expected: ";
                print_256bit_BE(hmac_read);
                std::cout << "     Got: ";
                print_256bit_BE(hmac);
                abort("HMAC validation failed!");
            }
        }
        std::cout << "HMAC validation success!\nJob's done!\n";

    }

    exit(EXIT_SUCCESS);
}
