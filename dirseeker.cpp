#include <iostream>
#include <thread>
#include <cstring>

#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

void usage (char** argv) {
    std::cerr << "Usage: " << argv[0] << " [-a] <directory name>" << std::endl;
}

void scan_dir (const char* dirname, bool skip_dot, bool verbose) {
    DIR* dir = nullptr;
    struct dirent entry;
    struct dirent *entryptr = nullptr;
    int retval = 0;
    
    dir = opendir(dirname);
    if (dir == nullptr) {
        fprintf(stderr, "Error opening directory %s: %s\n", dirname, strerror(errno));
        return;
    }
    if (verbose) printf("Found directory: %s\n", dirname);

    // get entries from directory
    while (!(retval = readdir_r(dir, &entry, &entryptr))) {
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
                printf("%8lld %s\n", entryinfo.st_size, full_name);
                
                // if entry is a directory, then recursive call for this directory
                if (S_ISDIR(entryinfo.st_mode)) 
                    scan_dir(full_name, skip_dot, verbose);
            }         
        }       
        delete[] full_name;
    }
    closedir(dir);
}

int main (int argc, char** argv) {
    int opt = 0;
    char opt_string[] = "av?";
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

            case '?':
            usage(argv);
            exit(EXIT_SUCCESS);
        }
        opt = getopt(argc, argv, opt_string);
    }

    for (int i = optind; i < argc; i++)
        scan_dir(argv[i], skip, verbose);
    return 0;
}

// читать вместо файла блок с файлом