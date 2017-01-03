#include <stdio.h>
#include <string.h>

const int BLOCK_SIZE = 8192;
bool iscode[256][65536];
unsigned char decode[256][65536];

class binaryInput {
    FILE * fin;
    int cnt;
    unsigned char c;

    bool get()
    {
        if (cnt == 0) {
            fread(&c, 1, 1, fin);
            cnt = 8;
        }
        return c & (1 << (--cnt));
    }

public:
    binaryInput(FILE * _fin):
        fin(_fin),
        cnt(0),
        c(0)
    {}

    unsigned char getch()
    {
        unsigned char res = 0;
        for (int i = 0; i < 8; ++i)
            res = (res << 1) + get();
        return res;
    }

    unsigned short getlen(int len)
    {
        unsigned short res = 0;
        for (int i = 0; i < len; ++i)
            res = (res << 1) + get();
        return res;
    }

    unsigned char getmatch()
    {
        unsigned short res = get();
        unsigned char len = 1;
        while (!iscode[len][res]) {
            ++len;
            res = (res << 1) + get();
        }
        return decode[len][res];
    }
};

int main(int argc, char * argv[])
{
    if (argc != 3) {
        puts("Usage: decompress INPUT OUTPUT");
        return 1;
    }
    printf("Decompressing %s to %s...\n\n", argv[1], argv[2]);
    fflush(stdout);
    FILE * fin = fopen(argv[1], "r");
    FILE * fout = fopen(argv[2], "w");
    binaryInput bin(fin);
    while (!feof(fin)) {
        memset(iscode, 0, sizeof(iscode));
        memset(decode, 0, sizeof(decode));

        unsigned short length = bin.getlen(16);
        if (length == 0)
            break;

        for (int i = 0; i < 256; ++i) {
            unsigned char len = bin.getch();
            unsigned short code = bin.getlen(len);
            if (len > 0) {
                iscode[len][code] = true;
                decode[len][code] = i;
            }
        }

        for (int i = 0; i < length; ++i)
            fputc(bin.getmatch(), fout);
    }
    fclose(fin);
    fclose(fout);

    int size;
    {
        FILE * f = fopen(argv[2], "r");
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fclose(f);
    }
    printf("Created %s (size: %10d)\n", argv[2], size);
    return 0;
}
