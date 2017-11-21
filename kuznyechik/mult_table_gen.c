#include <stdio.h>
#include <stdlib.h>

void usage (const char* name) 
{
    printf("Usage: %s <out_filename>\n", name);
    exit(EXIT_FAILURE);
}

// https://github.com/mjosaarinen/kuznechik/blob/master/kuznechik_8bit.c
static uint8_t kuz_mul_gf256(uint8_t x, uint8_t y)
{
	uint8_t z;
	
	z = 0;
	while (y) {		
		if (y & 1)
			z ^= x;
		x = (x << 1) ^ (x & 0x80 ? 0xC3 : 0x00);
		y >>= 1;
	}
		
	return z;
}

int main (int argc, const char** argv) 
{
    if (argc == 1) usage(argv[0]);

    FILE* fout = fopen(argv[1], "wbx");
    if (fout == NULL) 
    {
        perror("fout");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 256; i++) 
    {
        for (int j = 0; j < 256; j++) 
        {
            uint8_t result = kuz_mul_gf256(i, j);
            fwrite(&result, 1, 1, fout);
        }
    }

    return 0;
}
