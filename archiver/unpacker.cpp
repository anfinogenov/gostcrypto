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

        std::cout << name_buf << ": " << size << "B, mode: " << mode << std::endl;

        if (S_ISDIR(mode)) mkdir(name_buf, mode); 
        else {
            std::ofstream fout (name_buf, std::ios_base::binary | std::ios_base::out);
            chmod(name_buf, mode);

            while (size > BLOCKSIZE) 
            {
                char* read_buf = new char [BLOCKSIZE];
                fin.read(read_buf, BLOCKSIZE);
                fout.write(read_buf, BLOCKSIZE);
                size -= BLOCKSIZE;
                delete[] read_buf;
            }

            char* read_buf = new char [size];
            fin.read(read_buf, size);
            fout.write(read_buf, size);
            delete[] read_buf;

            fout.close();
        }

        delete[] name_buf;
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