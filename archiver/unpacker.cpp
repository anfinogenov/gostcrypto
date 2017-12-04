#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/stat.h>

#define BLOCKSIZE 1024

void usage (char** argv) 
{
    std::cerr << "Usage: " << argv[0] << " <archive name>" << std::endl;
}

void unpack (std::ifstream & fin) 
{
    while (fin) 
    {
        char* buf = new char[PATH_MAX];
        fin.getline(buf, PATH_MAX, '\0');
        off_t size = 0;
        mode_t mode = 0;
        fin.read((char*)&size, sizeof(off_t));
        fin.read((char*)&mode, sizeof(mode_t));
        std::cout << "Found entry: " << buf << " with size " << size <<
                     " and mode " << mode << std::endl;
        if (S_ISDIR(mode)) mkdir(buf, 0777);
        else {
            std::ofstream fout (buf, std::ios_base::binary | std::ios_base::out);

            while (size > BLOCKSIZE) 
            {
                char* buf = new char [BLOCKSIZE];
                fin.read(buf, BLOCKSIZE);
                fout.write(buf, BLOCKSIZE);
                size -= BLOCKSIZE;
                delete[] buf;
            }

            char* buf = new char [size];
            fin.read(buf, size);
            fout.write(buf, size);
            delete[] buf;

            fout.close();
        }
    }
}

int main (int argc, char** argv) {
    if (argc < 2) 
    {
        usage(argv);
        exit(EXIT_FAILURE);
    }

    std::ifstream fin(argv[1], std::ios_base::binary | std::ios_base::in);
    unpack(fin);
    
    fin.close();

    return 0;
}