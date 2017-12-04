#include <cstdlib>
#include <stdint.h>
#include <cstring>
#include <fstream>
#include <gmp.h>
#include <gmpxx.h>

//#define LENGTH64

#define DEBUG
#ifdef DEBUG
#include <iostream>
#define pr(a) fprintf(stderr, "%s\n", a);
#endif

namespace hybrid
{

struct Point
{
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
    Point operator + (const Point& src);
    Point operator * (const mpz_class& times);
};

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

Point Point::operator + (const Point& src)
{
    Point res(*this);
    res += src;
    return res;
}

Point Point::operator * (const mpz_class& times)
{
    Point res(*this);
    res *= times;
    return res;
}

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
#endif

static mpz_class p;
static mpz_class a;
static mpz_class b;
static mpz_class m;
static mpz_class q;
static mpz_class r;
static Point P;
static Point S;
static uint8_t parameters_set = 0;

static uint8_t key_set = 0;
static uint8_t* dar = NULL;
static size_t darsize = 0;
static mpz_class d;



//__attribute__((constructor))
static void parameters_init(void)
{
    mpz_set_str(p.get_mpz_t(), hex_p, 16);
    mpz_set_str(a.get_mpz_t(), hex_a, 16);
    mpz_set_str(b.get_mpz_t(), hex_b, 16);
    mpz_set_str(m.get_mpz_t(), hex_m, 16);
    mpz_set_str(q.get_mpz_t(), hex_q, 16);

    mpz_set_str(P.x.get_mpz_t(), hex_Px, 16);
    mpz_set_str(P.y.get_mpz_t(), hex_Py, 16);
    //OK!

    Point::mod = p;
    Point::curve_a = a;

    S = P*mpz_class(m*mpz_class_invert(b+967, p)); //some any value.

    size_t mac_mode = 256;

    // mac: Z/q X Z/m -> Z/r => r == 2^256
    mpz_ui_pow_ui(r.get_mpz_t(), 2, mac_mode);

    //key in mpz_t isn't saved between calls, so
    //rop, word count, word order(-1 is less signif), word size,
    //word endian (-1 is le), nails (?? 0), void* op);
    mpz_import(d.get_mpz_t(), darsize, -1, 1, -1, 0, dar);

    parameters_set = 1;
    pr("parameters set");
}

void set_key(uint8_t* key, size_t size)
{
    if (dar != NULL)
        free(dar);
    dar = (uint8_t*)malloc(size);
    memcpy(dar, key, size);
    darsize = size;
    key_set = 1;
    pr("key set");
}

//out format: 4bytes size of t, t, 4bytes size of Wx, Wx, sizeof Wy, Wy, mac(du)
uint8_t* encrypt(uint8_t* data, size_t size, size_t& out_size)
{
    pr("encryption start");
    if (!key_set)
        return NULL;
    //if (!parameters_set) //values are lost after exit from all lib functions
    parameters_init();

    mpz_class s;
    mpz_import(s.get_mpz_t(), size, -1, 1, -1, 0, data);

    //Y ok
    Point Y = P*d;

    //k, random, 0 < k < q
    mpz_class k;
    Point U;

    //to check if U == +-S
    do
    {
        k = 0;
        while (k == 0)
        {
            size_t qsize = mpz_class_size(q); //2 digit per byte => size*2
            uint8_t* kar = (uint8_t*)malloc(qsize);
            std::ifstream ur("/dev/urandom");
            ur.read((char*)kar, qsize);
            mpz_import(k.get_mpz_t(), qsize, -1, 1, -1, 0, kar);
            k %= q;
        }

        //U ok
        U = Y*k; //check if U == +-S, if yes => choose new k
    } while (U.x == S.x);
    //U counted normally

    mpz_class alpha = mpz_class_mod((U.y - S.y) * mpz_class_invert(U.x - S.x, p), p);
    mpz_class beta = mpz_class_mod((S.y*U.x - S.x*U.y) * mpz_class_invert(U.x - S.x, p), p);

    Point W = P*k;

    mpz_class t;
    t = mpz_class_mod(f(alpha, U.y, m)*s + h(alpha, beta, U.y, m), m);

    //TODO: count mac
    //maybe Y.y as salt?
    //uint8_t* du = pbkdf2(hmac_generate_256, 32, U.x,?, salt,?, 10000, outlen);
    //uint8_t* mac = hmac_generate_256(du, dulen, s, slen);

    uint32_t Msize =
        sizeof(uint32_t) + mpz_class_size(t) +
        sizeof(uint32_t) + mpz_class_size(W.x) +
        sizeof(uint32_t) + mpz_class_size(W.y);/* +
        dusize;*/
    uint8_t* M = (uint8_t*) malloc(Msize);

    size_t offset = 0;
    offset += mpz_class_export(M+offset, t);
    offset += mpz_class_export(M+offset, W.x);
    offset += mpz_class_export(M+offset, W.y);

    if (Msize != offset)
    {
        fprintf(stderr, "DEBUG: counted message size != real message size!\n");
        fprintf(stderr, "M: %d; off: %d\n", Msize, offset);
    }

    out_size = offset;
    pr("encryption ok");
    return M;



    //??
    /*
    //cm = floor(log(2, m)) - ceil(log(2, r))
    uint32_t cm = ((uint32_t)floor(log2(size))) - mac_mode;
    mpz_class cm(floor(log2(size)) - mac_mode);
    //cm ok

    //field = Z(2^cm); s \in field
    mpz_class p;
    mpz_ui_pow_ui(p.get_mpz_t(), 2, cm);*/

    //s' = s||mac(du, s)

    //Point W = [k]P
    //t = f(alpha, yu)s' + h(alpha, beta, yu) mod m

    //result: t||(xw, yw)
}

uint8_t* decrypt(uint8_t* data, size_t size, size_t& out_size)
{
    pr("decryption start");
    if (!key_set)
        return NULL;
    //if (!parameters_set)
    parameters_init();

    mpz_class t;
    Point W;

    size_t offset = 0;
    offset += mpz_class_import(t, data+offset);
    offset += mpz_class_import(W.x, data+offset);
    offset += mpz_class_import(W.y, data+offset);
    //TODO: du is here

    //if not the same curve
    if(!check_point_in_curve(W))
        return NULL;

    //U ok
    Point U = W*d;

    mpz_class alpha = mpz_class_mod((U.y - S.y) * mpz_class_invert(U.x - S.x, p), p);
    mpz_class beta = mpz_class_mod((S.y*U.x - S.x*U.y) * mpz_class_invert(U.x - S.x, p), p);

    mpz_class s;
    s = mpz_class_mod((t - h(alpha, beta, U.y, m))*f_inv(alpha, U.y, m), m);

    //TODO: count mac
    //maybe Y.y as salt?
    //uint8_t* du = pbkdf2(hmac_generate_256, 32, U.x,?, salt,?, 10000, outlen);
    //uint8_t* mac = hmac_generate_256(du, dulen, s, slen);

    uint32_t ssize = mpz_class_size(s);

    uint8_t* sar = (uint8_t*)malloc(ssize);
    mpz_export(sar, &out_size, -1, 1, -1, 0, s.get_mpz_t());
    return sar;
}



//read: 4 bytes size, then data. returns total offset
static size_t mpz_class_import(mpz_class& num, const uint8_t* data)
{
    size_t size = *(uint32_t*)data;
    mpz_import(num.get_mpz_t(), size, -1, 1, -1, 0, data+4);
    return size+4;
} //OK

//write: 4 bytes size, then data. returns total offset
static size_t mpz_class_export(uint8_t* data, const mpz_class& num)
{
    size_t size = 0;
    mpz_export(data+4, &size, -1, 1, -1, 0, num.get_mpz_t());
    *(uint32_t*)data = (uint32_t)size;
    return size+4;
} //OK

//size of stored number in bytes
static size_t mpz_class_size(const mpz_class& num)
{
    uint32_t size = mpz_sizeinbase(num.get_mpz_t(), 16);
    size = (size>>1) + (size & 1);
    return size;
} //OK

//check if point besongs to curve
static bool check_point_in_curve(const Point& pt)
{
    if ((pt.y*pt.y) % p == (pt.x*pt.x*pt.x + a*pt.x + b) % p)
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








//a=2, b=3, p=97
//2*(27,7) = (52,68)
//3*(27,7) = (91,58)
//4*(27,7) = (84,37)
//5*(27,7) = (29,43)
//6*(27,7) = (74,20)
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

}

