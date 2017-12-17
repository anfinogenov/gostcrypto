#include <cstdlib>
#include <stdint.h>
#include <cstring>
#include <fstream>
#include <gmp.h>
#include <gmpxx.h>

#include "../../hash/headers/hash_3411.h"
#include "../../pbkdf2/headers/pbkdf2.h"

#define LENGTH64

#define DEBUG
#ifdef DEBUG
#include <iostream>
#define eprint(a) std::cerr << a << std::endl
#endif

namespace elliptic
{

//to keep one size everywhere, in import and export
typedef uint8_t mpz_size_t;

struct Point
{
    //curve parameters for all points
    static mpz_class mod;
    static mpz_class curve_a;

    mpz_class x;
    mpz_class y;

    Point();
    Point(const mpz_class& nx, const mpz_class& ny);
    Point(const Point& src);
    Point& operator = (const Point& src);
    Point& operator += (const Point& src);
    Point& operator *= (const mpz_class& times);
    Point operator + (const Point& src) const;
    Point operator * (const mpz_class& times) const;
};

static void get_next_k_and_U(mpz_class& k, Point& U, const mpz_class& old_k, const Point& Y);
static void get_next_W_and_U(Point& W, Point& U, const Point& old_W, const mpz_class& d);
static uint8_t* count_mac(const Point& U, const Point& S, const uint8_t* data, size_t size);
static size_t mpz_class_import(mpz_class& num, const uint8_t* data);
static size_t mpz_class_export(uint8_t* data, const mpz_class& num);
static size_t mpz_class_size(const mpz_class& num);
static bool check_point_in_curve(const Point& pt);
static mpz_class mpz_class_mod(const mpz_class& n, const mpz_class& d);
static mpz_class mpz_class_invert(const mpz_class& src, const mpz_class& p);

static mpz_class f(
        const mpz_class& alpha, const mpz_class& coord, const mpz_class& m);
static mpz_class f_inv(
        const mpz_class& alpha, const mpz_class& coord, const mpz_class& m);
static mpz_class h(
        const mpz_class& alpha, const mpz_class& beta,
        const mpz_class& coord, const mpz_class& m);
static mpz_class g(const mpz_class& k); //next k_i for encryption
static Point g(const Point& W); //next W_i for decryption



//Point implementation
mpz_class Point::mod;
mpz_class Point::curve_a;

Point::Point()
{}

//debug:
Point::Point(const mpz_class& nx, const mpz_class& ny)
{ x = nx; y = ny; }

Point::Point(const Point& src)
{
    x = mpz_class(src.x);
    y = mpz_class(src.y);
}

Point& Point::operator = (const Point& src)
{
    x = src.x;
    y = src.y;
    return *this;
}

Point& Point::operator += (const Point& src)
{
    mpz_class newx, newy, lambda;
    if (x != src.x)
    {
        lambda = mpz_class_mod((src.y - y) * mpz_class_invert(src.x - x, mod), mod);
        newx = mpz_class_mod(lambda*lambda - x - src.x, mod);
        newy = mpz_class_mod(lambda*(x - newx) - y, mod);
    }
    //else if ((y % mod) == (src.y % mod))
    else if (y == src.y)
    {
        lambda = mpz_class_mod((x*x * 3 + curve_a) * mpz_class_invert(y * 2, mod), mod);
        newx = mpz_class_mod(lambda*lambda - x*2, mod);
        newy = mpz_class_mod(lambda*(x - newx) - y, mod);
    }
    else //p + (-p) == 0
    {
        newx = 0;
        newy = 0;
    }

    x = newx;
    y = newy;
    return *this;
}

Point& Point::operator *= (const mpz_class& times)
{
    mpz_class left(times);
    Point t(*this);

    left--;
    while (left > 0)
    {
        if (mpz_tstbit(left.get_mpz_t(), 0)) //left % 2 != 0
        {
            if (t.x == this->x || t.y == this->y)
                t += t;
            else
                t += *this;
            left--;
        }
        mpz_tdiv_q_2exp(left.get_mpz_t(), left.get_mpz_t(), 1);
        //x /= 2;
        *this += *this;
    }
    x = t.x;
    y = t.y;
    return *this;
}

Point Point::operator + (const Point& src) const
{
    Point res(*this);
    res += src;
    return res;
}

Point Point::operator * (const mpz_class& times) const
{
    Point res(*this);
    res *= times;
    return res;
}

//TODO: as hex
#ifdef LENGTH64
static const char* hex_p =
    "4531ACD1FE0023C7550D267B6B2FEE80922B14B2FFB90F04D4EB7C09B5D2D15DF1D852741AF4704A0458047E80E4546D35B8336FAC224DD81664BBF528BE6373";
static const char* hex_a =
    "7";
static const char* hex_b =
    "1CFF0806A31116DA29D8CFA54E57EB748BC5F377E49400FDD788B649ECA1AC4361834013B2AD7322480A89CA58E0CF74BC9E540C2ADD6897FAD0A3084F302ADC";
static const char* hex_m =
    "4531ACD1FE0023C7550D267B6B2FEE80922B14B2FFB90F04D4EB7C09B5D2D15DA82F2D7ECB1DBAC719905C5EECC423F1D86E25EDBE23C595D644AAF187E6E6DF";
static const char* hex_q =
    "4531ACD1FE0023C7550D267B6B2FEE80922B14B2FFB90F04D4EB7C09B5D2D15DA82F2D7ECB1DBAC719905C5EECC423F1D86E25EDBE23C595D644AAF187E6E6DF";
static const char* hex_Px =
    "24D19CC64572EE30F396BF6EBBFD7A6C5213B3B3D7057CC825F91093A68CD762FD60611262CD838DC6B60AA7EEE804E28BC849977FAC33B4B530F1B120248A9A";
static const char* hex_Py =
    "2BB312A43BD2CE6E0D020613C857ACDDCFBF061E91E5F2C3F32447C259F39B2C83AB156D77F1496BF7EB3351E1EE4E43DC1A18B91B24640B6DBB92CB1ADD371E";
static size_t ot_block_size = 32;
#else
static const char* hex_p =
    "8000000000000000000000000000000000000000000000000000000000000431";
static const char* hex_a =
    "7";
static const char* hex_b =
    "5FBFF498AA938CE739B8E022FBAFEF40563F6E6A3472FC2A514C0CE9DAE23B7E";
static const char* hex_m =
    "8000000000000000000000000000000150FE8A1892976154C59CFC193ACCF5B3";
static const char* hex_q =
    "8000000000000000000000000000000150FE8A1892976154C59CFC193ACCF5B3";
static const char* hex_Px =
    "2";
static const char* hex_Py =
    "8E2A8A0E65147D4BD6316030E16D19C85C97F0A9CA267122B96ABBCEA7E8FC8";
static size_t ot_block_size = 16;
#endif
static uint32_t mac_size = 64;

//curve parameters
static mpz_class p;
static mpz_class a;
static mpz_class b;
static mpz_class m;
static mpz_class q;
static mpz_class r;
static Point P;
static Point S;

static uint8_t key_set = 0;
static uint8_t* dar = NULL;
static size_t darsize = 0;
static mpz_class d;
static mpz_class g_const = 32;


static void parameters_init(void)
{
    mpz_set_str(p.get_mpz_t(), hex_p, 16);
    mpz_set_str(a.get_mpz_t(), hex_a, 16);
    mpz_set_str(b.get_mpz_t(), hex_b, 16);
    mpz_set_str(m.get_mpz_t(), hex_m, 16);
    mpz_set_str(q.get_mpz_t(), hex_q, 16);

    mpz_set_str(P.x.get_mpz_t(), hex_Px, 16);
    mpz_set_str(P.y.get_mpz_t(), hex_Py, 16);

    Point::mod = p;
    Point::curve_a = a;

    S = P*mpz_class(m*mpz_class_invert(b+967, p)); //some any value.

    size_t mac_mode = 512;

    // mac: Z/q X Z/m -> Z/r => r == 2^256 or 2^512
    mpz_ui_pow_ui(r.get_mpz_t(), 2, mac_mode);

    //key in mpz_t isn't saved between calls, so
    //rop, word count, word order(-1 is less signif), word size,
    //word endian (-1 is le), nails (?? 0), void* op);
    mpz_import(d.get_mpz_t(), darsize, -1, 1, -1, 0, dar);

    //pr("parameters set");
}

void set_key(uint8_t* key, size_t size)
{
    //if key exists
    if (dar != NULL)
        free(dar);

    dar = (uint8_t*)malloc(size);
    memcpy(dar, key, size);
    darsize = size;
    key_set = 1;
}

//out format: look inside
uint8_t* encrypt(uint8_t* data, size_t size, size_t& out_size)
{
    //pr("encryption start");
    if (!key_set)
    {
        eprint("elliptic: key is not set, encrypt failed");
        return NULL;
    }

    parameters_init();

    //public key
    Point Y = P*d;

    //k, random, 0 < k < q
    mpz_class k;
    Point U;

    //get k_0
    {
        size_t qsize = mpz_class_size(q);
        uint8_t* kar = (uint8_t*)malloc(qsize);

        std::ifstream ur("/dev/urandom");
        ur.read((char*)kar, qsize);

        mpz_import(k.get_mpz_t(), qsize, -1, 1, -1, 0, kar);
        k = mpz_class_mod(k, q);

        free(kar);
    }

    //W = k_0*P
    Point W = P*k;

    //get U_1 for mac
    get_next_k_and_U(k, U, k, Y);

    uint8_t* mac = count_mac(U, S, data, size);
    if (mac == NULL)
    {
        eprint("encrypt: mac failed");
        return NULL;
    }

    //prepare message size
    size_t Msize =
        sizeof(mpz_size_t) + mpz_class_size(W.x) +
        sizeof(mpz_size_t) + mpz_class_size(W.y) +
        mac_size; //mac size

    //keep et size and et
    size_t t_size = sizeof(uint32_t);
    uint8_t* tar = (uint8_t*)malloc(t_size);

    uint8_t blocks = 0;
    uint8_t last_size = 0;
    //encrypt message
    // [0] - blocks count, [1] - last block size, [2-5] - et size + 4, other - exported classes Wx, Wy, then mac
    while (size != 0)
    {
        size_t itersize = (size > ot_block_size) ? ot_block_size : size;

        //import data as num
        mpz_class s;
        mpz_import(s.get_mpz_t(), itersize, -1, 1, -1, 0, data);

        mpz_class alpha = mpz_class_mod((U.y - S.y) * mpz_class_invert(U.x - S.x, p), p);
        mpz_class beta = mpz_class_mod((S.y*U.x - S.x*U.y) * mpz_class_invert(U.x - S.x, p), p);

        //encrypt message part
        mpz_class t;
        t = mpz_class_mod(f(alpha, U.y, m)*s + h(alpha, beta, U.y, m), m);

        //export message part
        tar = (uint8_t*)realloc(tar, t_size + sizeof(mpz_size_t) + mpz_class_size(t));
        t_size += mpz_class_export(tar + t_size, t);

        size -= itersize;
        data += itersize;
        if (size == 0)
            last_size = itersize; //remember size of last block to decrypt correctly

        get_next_k_and_U(k, U, k, Y);

        ++blocks;
    }

    //concat encrypted message
    *(uint32_t*)tar = (uint32_t)t_size;

    Msize += t_size + 2;
    uint8_t* M = (uint8_t*)malloc(Msize);
    *M = blocks;
    *(M+1) = last_size;
    memcpy(M+2, tar, t_size);
    free(tar); tar = NULL;

    size_t offset = t_size+2;
    offset += mpz_class_export(M+offset, W.x);
    offset += mpz_class_export(M+offset, W.y);
    memcpy(M+offset, mac, mac_size);

    out_size = offset+mac_size;
    return M;
}

uint8_t* decrypt(uint8_t* data, size_t size, size_t& out_size)
{
    out_size = 0;
    if (!key_set)
    {
        eprint("elliptic: key is not set, decrypt failed");
        return NULL;
    }

    parameters_init();

    //parse enctypted message
    Point W;
    uint8_t* macenc = (uint8_t*)malloc(mac_size);

    size_t offset = *(uint32_t*)(data+2) + 2; //skip text
    offset += mpz_class_import(W.x, data+offset);
    offset += mpz_class_import(W.y, data+offset);
    memcpy(macenc, data+offset, mac_size); offset += mac_size;

    //if not the same curve
    if(!check_point_in_curve(W))
    {
        eprint("elliptic: message was encryped on different curve");
        return NULL;
    }

    Point U;
    get_next_W_and_U(W, U, W, d);
    Point U1 = U; //remember U1 for mac

    //decrypt
    size_t blocks = *data;
    size_t last_size = *(data+1);
    size_t ssize = blocks*ot_block_size;
    uint8_t* sar = (uint8_t*)malloc(ssize);
    size_t sar_offset = 0;

    size_t t_size = *(uint32_t*)(data+2);
    offset = sizeof(uint32_t) + 2;
    while(offset < t_size+2) //while not decrypted whole et
    {
        //U ok
        mpz_class t;
        offset += mpz_class_import(t, data+offset);

        mpz_class alpha = mpz_class_mod((U.y - S.y) * mpz_class_invert(U.x - S.x, p), p);
        mpz_class beta = mpz_class_mod((S.y*U.x - S.x*U.y) * mpz_class_invert(U.x - S.x, p), p);

        //decrypt_message part
        mpz_class s;
        s = mpz_class_mod((t - h(alpha, beta, U.y, m))*f_inv(alpha, U.y, m), m);

        size_t temp = 0;
        mpz_export(sar+sar_offset, &temp, -1, 1, -1, 0, s.get_mpz_t());

        //fill not significant bytes with 0
        for (size_t i = temp; i < ot_block_size; i++)
            sar[sar_offset + i] = 0;

        //if last - add size of last, else full block
        sar_offset += (offset < t_size+2) ? ot_block_size : last_size;

        get_next_W_and_U(W, U, W, d);
    }

    ssize = sar_offset;
    out_size = ssize;

    //count mac
    uint8_t* mac = count_mac(U1, S, sar, ssize);

    if (mac == NULL)
    {
        eprint("elliptic: decrypted mac failed");
        return NULL;
    }

    //check mac
    for (size_t i = 0; i < mac_size; i++)
        if (mac[i] != macenc[i])
        {
            eprint("elliptic: invalid mac");
            return NULL;
        }

    return sar;
}

static void get_next_k_and_U(mpz_class& k, Point& U, const mpz_class& old_k, const Point& Y)
{
    //to check if U == +-S
    do
    {
        k = g(old_k);
        k = mpz_class_mod(k, q);

        U = Y*k; //check if U == +-S, if yes => choose new k
    } while (U.x == S.x);
    //U counted normally
}

static void get_next_W_and_U(Point& W, Point& U, const Point& old_W, const mpz_class& d)
{
    //to check if U == +-S
    do
    {
        W = g(old_W);

        U = W*d; //check if U == +-S, if yes => choose new k
    } while (U.x == S.x);
    //U counted normally
}

static uint8_t* count_mac(const Point& U, const Point& S, const uint8_t* data, size_t size)
{
    //convert U.x to array
    size_t Uxsize = 0;
    uint8_t* Ux = (uint8_t*)malloc(mpz_class_size(U.x));
    mpz_export(Ux, &Uxsize, -1, 1, -1, 0, U.x.get_mpz_t());

    //convert S to array
    size_t Ssize = 0, tempsize = 0;
    uint8_t* Sar = (uint8_t*)malloc(mpz_class_size(S.x) + mpz_class_size(S.y));
    mpz_export(Sar, &tempsize, -1, 1, -1, 0, S.x.get_mpz_t());
    mpz_export(Sar+tempsize, &Ssize, -1, 1, -1, 0, S.y.get_mpz_t());
    Ssize += tempsize;

    uint8_t* du = pbkdf2(
            GOST3411::hmac_512, mac_size,
            Ux, Uxsize,
            Sar, Ssize,
            10000, mac_size);

    if (du == NULL)
    {
        eprint("elliptic: key derivation for mac failed");
        return NULL;
    }
    return GOST3411::hmac_512(du, mac_size, data, size);
}



//read: 1 byte size, then data. returns number of bytes read
static size_t mpz_class_import(mpz_class& num, const uint8_t* data)
{
    size_t size = *(mpz_size_t*)data;
    mpz_import(num.get_mpz_t(), size, -1, 1, -1, 0, data+sizeof(mpz_size_t));
    return size+sizeof(mpz_size_t);
} //OK

//write: 1 byte size, then data. returns written bytes count
static size_t mpz_class_export(uint8_t* data, const mpz_class& num)
{
    size_t size = 0;
    mpz_export(data+sizeof(mpz_size_t), &size, -1, 1, -1, 0, num.get_mpz_t());
    *(mpz_size_t*)data = (mpz_size_t)size;
    return size+sizeof(mpz_size_t);
} //OK

//size of stored number in bytes
static size_t mpz_class_size(const mpz_class& num)
{
    size_t size = mpz_sizeinbase(num.get_mpz_t(), 16);
    size = (size>>1) + (size & 1);
    return size;
} //OK

//check if point besongs to curve
static bool check_point_in_curve(const Point& pt)
{
    if (mpz_class_mod(pt.y*pt.y, p) == mpz_class_mod(pt.x*pt.x*pt.x + a*pt.x + b, p))
        return true;
    return false;
} //OK

// operator % can return negative numbers, so using mod
static mpz_class mpz_class_mod(const mpz_class& n, const mpz_class& d)
{
    mpz_class res;
    mpz_mod(res.get_mpz_t(), n.get_mpz_t(), d.get_mpz_t());
    return res;
} //OK

//a^-1
static mpz_class mpz_class_invert(const mpz_class& src, const mpz_class& p)
{
    mpz_class res;
    mpz_invert(res.get_mpz_t(), src.get_mpz_t(), p.get_mpz_t());
    return res;
} //OK

static mpz_class f(
        const mpz_class& alpha, const mpz_class& coord, const mpz_class& m)
{
    // m is prime or m = p1p2; p1, p2 are primes
    return mpz_class_mod(alpha * coord, m);
}

static mpz_class f_inv(
        const mpz_class& alpha, const mpz_class& coord, const mpz_class& m)
{
    mpz_class fr = f(alpha, coord, m);
    return mpz_class_invert(fr, m);
}

static mpz_class h(
        const mpz_class& alpha, const mpz_class& beta,
        const mpz_class& coord, const mpz_class& m)
{
    return mpz_class_mod(f_inv(alpha, coord, m) * beta, m);
}

//next k_i for encryption
static mpz_class g(const mpz_class& k)
{ return k + g_const; }

//next W_i for decryption
static Point g(const Point& W)
{ return W + (P*g_const); }







//a=2, b=3, p=97
//2*(27,7) = (52,68)
//3*(27,7) = (91,58)
//4*(27,7) = (84,37)
//5*(27,7) = (29,43)
//6*(27,7) = (74,20)
#ifdef DEBUG
void debug()
{
    std::cerr << "mpz mod test:" << std::endl;
    p = 13;
    a = -3;
    b = mpz_class_mod(a, p);
    std::cerr << a << " % " << p << " == " << b << std::endl;

    std::cerr << "mpz invert test:" << std::endl;
    p = 13;
    a = -6;
    b = mpz_class_invert(a, p);
    std::cerr << a << "^-1 % " << p << " == " << b << std::endl;

    std::cerr << "point math test:" << std::endl;
    a = 2;
    b = 3;
    p = 97;
    Point::mod = p;
    Point::curve_a = a;
    Point po;
    po.x = 27;
    po.y = 7;
    Point pe[5] =
        {Point(52,68), Point(91,58), Point(84,37), Point(29,43), Point(74,20)};
    bool multok = true;
    for (int i = 0; i < 5; i++)
    {
        Point l = po*(i+2);
        if (l.x != pe[i].x || l.y != pe[i].y)
            multok = false;
    }
    if (multok)
        std::cerr << "OK" << std::endl;
    else
        std::cerr << "FAIL" << std::endl;
}
#endif

}
#undef eprint

