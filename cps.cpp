#include <stdio.h>
#include <string.h>
#include <set>
#include <vector>

struct node {
    int left, right, cnt;
    unsigned char d;
};

bool operator< (const node &a, const node &b)
{
    return a.cnt < b.cnt || (a.cnt == b.cnt && a.d < b.d);
}

const int BLOCK_SIZE = 1024;
char block[BLOCK_SIZE];
unsigned char data[BLOCK_SIZE];
int count[256];
unsigned char code[256];

void printfb(unsigned char c, FILE * fout)
{
    int d = 7;
    for ( ; d >= 0 && !(c & (1 << d)); --d);
    for ( ; d >= 0; --d)
        fputc((c & (1 << d)) ? '1' : '0', fout);
}

std::set<std::pair<node, int>> s;
std::vector<node> v;

void traverse(int x, unsigned char c)
{
    if (v[x].left == -1 && v[x].right == -1) {
        code[v[x].d] = c;
        return;
    }
    traverse(v[x].left, c << 1);
    traverse(v[x].right, (c << 1) + 1);
}

void compress()
{
    memset(count, 0, sizeof(count));
    memcpy(data, block, sizeof(char) * BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; ++i)
        ++count[data[i]];
    v.clear();
    for (int i = 0; i < 256; ++i)
        if (count[i] > 0) {
            v.push_back(node{-1, -1, count[i], (unsigned char)i});
            s.insert(std::make_pair(v.back(), v.size() - 1));
        }
    while (s.size() > 1) {
        int l = s.begin()->second;
        s.erase(*s.begin());
        int r = s.begin()->second;
        s.erase(*s.begin());
        v.push_back(node{l, r, v[l].cnt + v[r].cnt, 0});
        s.insert(std::make_pair(v.back(), v.size() - 1));
    }
    int root = s.begin()->second;
    traverse(root, 0);
    for (int i = 0; i < 256; ++i)
        if (count[i] > 0) {
            printf("%3d %2x ", count[i], i);
            printfb(code[i], stdout);
            printf("\n");
        }
}

int main(int argc, char * argv[])
{
    if (argc != 3) {
        puts("Usage: cps INPUT OUTPUT");
//        return 1;
    }
    FILE * fin = fopen("input", "r");
    FILE * fout = fopen("output", "w");
    while (true) {
        if (fgets(block, BLOCK_SIZE, fin) == NULL)
            break;
        compress();
        break;
        for (int i = 0; i < BLOCK_SIZE && data[i]; ++i)
            fputc(code[data[i]], fout);
    }
    fclose(fin);
    fclose(fout);
    return 0;
}
