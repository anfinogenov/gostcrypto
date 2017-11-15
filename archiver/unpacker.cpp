#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/stat.h>

void usage (char** argv) 
{
    std::cerr << "Usage: " << argv[0] << " <archive name>" << std::endl;
}

// TODO: optimized IO (while !eof read block, 
//                     parse block and write out parsed data)
void unpack (std::ifstream & fin) 
{
    while (!fin.eof()) 
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
            for (int i = 0; i < size; i++) {
                char tmp;
                fin.read(&tmp, 1);
                fout.write(&tmp, 1);
            }
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