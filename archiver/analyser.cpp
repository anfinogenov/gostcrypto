#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <cstdint>
#include <unistd.h>

std::mutex print_mutex;

void print_stats (const char* filename, bool verbose) 
{
    uint64_t byte_count[256] = {0};
    uint64_t full_count = 0;

    std::ifstream fin (filename, std::ios_base::binary);
    
    uint8_t byte = 0;
    while (fin.read((char*)&byte, 1)) 
    {
        byte_count[byte]++;
        full_count++;
    }

    fin.close();

    print_mutex.lock();
    if (verbose) 
        std::cout << std::endl << "Checking " << filename << std::endl;
    
    double max = 0;
    int idx_max = 0;
    for (int idx = 0; idx < 256; idx++) 
    {
        double percent = byte_count[idx];
        percent /= full_count * 0.01;
        
        if (percent > max) 
        {
            max = percent;
            idx_max = idx;
        }

        if (verbose) 
        {
            printf("%02X: %2.2lf%%\t", idx, percent);
            if ((idx+1) % 8 == 0) printf("\n");
        }
    }
    printf("Most significant byte in %s: 0x%02X: %2.2lf%%\n", 
            filename, idx_max, max);
    if (verbose) printf("End of stats for %s\n", filename);
    print_mutex.unlock();
}

int main (int argc, char *argv[])
{
    if (argc == 1) 
    {
        std::cout << "Usage: " 
                  << argv[0] 
                  << " [-v] "
                  << " <file_1> <file_2> ... <file_N>" 
                  << std::endl;
        return EXIT_FAILURE;
    }

    int opt = 0;
    char opt_string[] = "v";
    bool verbose = false;
    opt = getopt(argc, argv, opt_string);

    while (opt != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
        }
        opt = getopt(argc, argv, opt_string);
    }

    argc -= optind;
    argv += optind;    

    std::thread* handle = new std::thread[argc];

    for (int fidx = 0; fidx < argc; fidx++) 
    {
        handle[fidx] = std::thread(print_stats, argv[fidx], verbose);
    }
    
    for (int i = 0; i < argc; i++)
    {
        handle[i].join();
    }
    delete[] handle;

    return EXIT_SUCCESS;
}
