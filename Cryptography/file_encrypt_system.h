#include "spn.h"
#include <openssl/crypto.h>
#include <openssl\ec.h>
#include <openssl\ecdsa.h>
#include <openssl\ecdh.h>
#include <openssl\err.h>
#include <openssl\bio.h>
#include <openssl\bn.h>
#include <openssl\comp.h>

#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

void getEccKey(EC_KEY *aliceKey, EC_KEY *bobKey);
void fileEncrypt(char *filename, EC_KEY *aliceKey);
void fileDecrypt(char *filename, EC_KEY *bobKey);
void getFileDig(char *filename, Bit8 *fileHash);
void Verify(Bit8 *fileDig, int diglen, Bit8 *fileSig, int siglen);
void CatAndZip(Bit8 *fileSig, int siglen, char *filename);
void getEncryptKey(Bit8 Key[16]);
void *KDF_MD5(const void *in, size_t inlen, void *out, size_t *outlen);
void printKey(Bit8 *buf, int len);
int list(void);