#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long long uint64;

#define ROTLEFT(a, b) ((a << b) | (a >> (32 - b)))
#define ROTRIGHT(a, b) ((a >> b) | (a << (32 - b)))

#define CH(x,y,z) ((x & y) ^ (~x & z))
#define MAJ(x,y,z) ((x & y) ^ (x & z) ^ (y & z))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ (x >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ (x >> 10))

static const uint32 k[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

string sha256(const string &data) {
    uint32 h0 = 0x6a09e667;
    uint32 h1 = 0xbb67ae85;
    uint32 h2 = 0x3c6ef372;
    uint32 h3 = 0xa54ff53a;
    uint32 h4 = 0x510e527f;
    uint32 h5 = 0x9b05688c;
    uint32 h6 = 0x1f83d9ab;
    uint32 h7 = 0x5be0cd19;

    vector<uint8> msg(data.begin(), data.end());
    uint64 bitlen = msg.size() * 8;

    msg.push_back(0x80);
    while ((msg.size() * 8) % 512 != 448)
        msg.push_back(0x00);

    for (int i = 7; i >= 0; i--)
        msg.push_back((bitlen >> (i * 8)) & 0xff);

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32 w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = (msg[chunk + i * 4] << 24) | (msg[chunk + i * 4 + 1] << 16) |
                   (msg[chunk + i * 4 + 2] << 8) | (msg[chunk + i * 4 + 3]);

        for (int i = 16; i < 64; ++i)
            w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];

        uint32 a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6, h = h7;

        for (int i = 0; i < 64; ++i) {
            uint32 t1 = h + EP1(e) + CH(e, f, g) + k[i] + w[i];
            uint32 t2 = EP0(a) + MAJ(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
    }

    stringstream ss;
    ss << hex << setfill('0');
    ss << setw(8) << h0 << setw(8) << h1 << setw(8) << h2 << setw(8) << h3
       << setw(8) << h4 << setw(8) << h5 << setw(8) << h6 << setw(8) << h7;

    return ss.str();
}
