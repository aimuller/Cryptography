#include "gmp.h"
#define MILLER_RABIN_TEST_NUM 5
#define PRIME_BIT 512
typedef unsigned char Bit8;
typedef unsigned short Bit16;
typedef unsigned int Bit32;
void getRSAparam(mpz_t n, mpz_t p, mpz_t q, mpz_t e, mpz_t d);
void getPrime(mpz_t n);
bool MillerRabin(const mpz_t n);
void PowerMod(const mpz_t a, const mpz_t b, const mpz_t n, mpz_t s);
bool SetTestNum(const mpz_t n);
bool getModInverse(const mpz_t n, const mpz_t e, mpz_t d);
void ChineseRemainderTheorem(const mpz_t a, const  mpz_t b, const  mpz_t p, const mpz_t q, mpz_t s);
void MontPowerMod(const mpz_t a, const mpz_t b, const mpz_t n, mpz_t s);
void MontMult(mpz_t A, mpz_t B, const mpz_t n, int n_bit, const mpz_t IN);
void RSA_CMP(mpz_t n, mpz_t p, mpz_t q, mpz_t e, mpz_t d);
