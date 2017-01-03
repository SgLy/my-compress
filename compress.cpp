#include <stdio.h>
#include <string.h>
#include <set>
#include <vector>

class binaryOutput {
    FILE * fout;
    int cnt;
    unsigned char c;

    void add(bool b)
    {
        c = (c << 1) + b;
        ++cnt;
        if (cnt == 8) {
            fputc(c, fout);
            cnt = 0;
            c = 0;
        }
    }

public:
    binaryOutput(FILE * _fout):
        fout(_fout),
        cnt(0),
        c(0)
    {}

    void print(unsigned short code, unsigned char len)
    {
        for ( ; len; --len)
            add(code & (1 << (len - 1)));
    }

    void clean()
    {
        for ( ; cnt < 8; ++cnt)
            c <<= 1;
        fputc(c, fout);
        cnt = 0;
    }
};

struct node {
    int left, right, cnt;
    unsigned char d;
    node(int _left, int _right, int _cnt, unsigned char _d):
        left(_left),
        right(_right),
        cnt(_cnt),
        d(_d)
    {}
};

bool operator< (const node &a, const node &b)
{
    return a.cnt < b.cnt || (a.cnt == b.cnt && a.d < b.d);
}

const int BLOCK_SIZE = 8192;
unsigned char data[BLOCK_SIZE];
int count[256];
unsigned short code[256];
unsigned char codelen[256];

void fbprintf(FILE * fout, unsigned short c, unsigned char len)
{
    for (int d = len; d; --d)
        fputc((c & (1 << (d - 1))) ? '1' : '0', fout);
}

void frprintf(FILE * fout, unsigned short t)
{
    unsigned char buf[3];
    memcpy(buf, &t, sizeof(t));
    fputc(buf[1], fout);
    fputc(buf[0], fout);
}

std::set< std::pair<node, int> > s;
std::vector<node> v;

void traverse(int x, unsigned char dep, unsigned short c)
{
    if (v[x].left == -1 && v[x].right == -1) {
        code[v[x].d] = c;
        codelen[v[x].d] = dep;
        return;
    }
    traverse(v[x].left, dep + 1, c << 1);
    traverse(v[x].right, dep + 1, (c << 1) + 1);
}

void compress()
{
    memset(count, 0, sizeof(count));
    memset(code, 0, sizeof(code));
    memset(codelen, 0, sizeof(codelen));
    for (int i = 0; i < BLOCK_SIZE; ++i)
        ++count[data[i]];
    v.clear();
    s.clear();
    for (int i = 0; i < 256; ++i)
        if (count[i] > 0) {
            v.push_back(node(-1, -1, count[i], (unsigned char)i));
            s.insert(std::make_pair(v.back(), v.size() - 1));
        }
    while (s.size() > 1) {
        int l = s.begin()->second;
        s.erase(*s.begin());
        int r = s.begin()->second;
        s.erase(*s.begin());
        v.push_back(node(l, r, v[l].cnt + v[r].cnt, 0));
        s.insert(std::make_pair(v.back(), v.size() - 1));
    }
    int root = s.begin()->second;
    traverse(root, 0, 0);
}

int main(int argc, char * argv[])
{
    if (argc != 3) {
        puts("Usage: compress INPUT OUTPUT");
        return 1;
    }
    printf("Compressing %s to %s...\n\n", argv[1], argv[2]);
    fflush(stdout);
    FILE * fin = fopen(argv[1], "r");
    FILE * fout = fopen(argv[2], "w");
    binaryOutput bout(fout);
    long int last_pos = ftell(fin);
    while (!feof(fin)) {
        fread(data, 1, BLOCK_SIZE, fin);
        compress();

        unsigned short length = ftell(fin) - last_pos;
        last_pos = ftell(fin);

        bout.print(length, 16);
        for (int i = 0; i < 256; ++i) {
            bout.print(codelen[i], 8);
            bout.print(code[i], codelen[i]);
        }

        for (int i = 0; i < length; ++i)
            bout.print(code[data[i]], codelen[data[i]]);
    }
    bout.print(0, 16);
    bout.clean();
    fclose(fin);
    fclose(fout);

    int old_size, new_size;
    {
        FILE * f = fopen(argv[1], "r");
        fseek(f, 0, SEEK_END);
        old_size = ftell(f);
        fclose(f);
    }
    {
        FILE * f = fopen(argv[2], "r");
        fseek(f, 0, SEEK_END);
        new_size = ftell(f);
        fclose(f);
    }

    printf("Original size: %10d\n", old_size);
    printf("New size     : %10d\n", new_size);
    printf("Deflated     : %10.2lf%%\n", 100 * double(old_size - new_size) / old_size);
    return 0;
}
