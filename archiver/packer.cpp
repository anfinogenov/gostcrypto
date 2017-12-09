#include <iostream>
#include <fstream>
#include <thread>
#include <cstring>
#include <cstdlib>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#define BLOCKSIZE 1024

void usage (char** argv) 
{
    std::cerr << "Usage: " << argv[0] << " [-avh] <directory name>" << std::endl;
}

void write_file_entry (const char* filename, std::ofstream & fout, const struct stat filestat, size_t blocksize) 
{
    std::ifstream fin(filename, std::ios_base::binary | std::ios_base::in);

    fout.write(filename, strlen(filename)+1);                   // direntry name
    fout.write((char*)&(filestat.st_size), sizeof(off_t));      // direntry size (in bytes)
    fout.write((char*)&(filestat.st_mode), sizeof(mode_t));     // direntry mode (dir, file etc.)

    do 
    {
        char* buf = new char[blocksize];                        // create new buffer for optimized IO
        fin.read(buf, blocksize);
        fout.write(buf, fin.gcount());                          // write block to archive
        delete[] buf;
    }
    while (fin);

    fin.close();
}

void scan_dir (const char* dirname, std::ofstream & fout, bool skip_dot, bool verbose) 
{
    DIR* dir = nullptr;
    struct dirent entry;
    struct dirent *entryptr = nullptr;
    int retval = 0;

    dir = opendir(dirname);
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
        snprintf(full_name, PATH_MAX, "%s/%s", dirname, entry.d_name);
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

int main (int argc, char** argv) 
{
    if (argc < 2) 
    {
        usage(argv);
        exit(EXIT_FAILURE);
    }

    int opt = 0;
    char opt_string[] = "avh?";
    bool skip = true, verbose = false;
    opt = getopt(argc, argv, opt_string);

    while (opt != -1) {
        switch (opt) {
            case 'a':
            skip = false;
            break;

            case 'v':
            verbose = true;
            break;

            case 'h':
            case '?':
            usage(argv);
            exit(EXIT_SUCCESS);
        }
        opt = getopt(argc, argv, opt_string);
    }

    std::ofstream fout("out.ak", std::ios_base::out | std::ios_base::binary);
    for (int i = optind; i < argc; i++) 
    {
        DIR* dir;
        dir = opendir(argv[i]);
        if (dir == nullptr) 
        {
            if (verbose)
            {
                std::cerr << "Error opening directory " << argv[i] << ": " << strerror(errno) << std::endl;
                std::cerr << "Trying " << argv[i] << " as file...";
            }

            std::ifstream fin(argv[i], std::ios_base::out | std::ios_base::binary);
            if (!fin) 
            {
                if (verbose) std::cerr << "failed :(\n";
                continue;
            }
            else 
            {
                if (verbose) std::cerr << "success!" << std::endl;
                fin.close();
                struct stat entry_stat;
                lstat(argv[i], &entry_stat);
                write_file_entry(argv[i], fout, entry_stat, BLOCKSIZE);
            }
        } 
        else 
        {
            if (verbose) printf("Found directory: %s\n", argv[i]);
            struct stat entry_dir_stat;
            lstat(argv[i], &entry_dir_stat);
            write_file_entry(argv[i], fout, entry_dir_stat, BLOCKSIZE);
            scan_dir(argv[i], fout, skip, verbose);
        }
    }
    fout.close();

    return EXIT_SUCCESS;
}
