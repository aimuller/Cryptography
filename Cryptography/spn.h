#include <iostream>
#include <stdlib.h>
#include <time.h>
#define MyKey 0x3a94d63f
typedef unsigned char Bit8;
typedef unsigned short Bit16;
typedef unsigned int Bit32;

using namespace std;

//原始SPN算法实现
void spn(int mode, Bit16 &x, Bit32 Key);
void one_round_spn(int mode, int r, Bit16 &x, Bit32 Key);  //一轮SPN加解密
Bit16 round_key(int r, Bit32 Key);
void s_box(int mode, Bit16 &x); //S盒，根据选择加密或解密模式选择S盒或S盒的逆
void p_box(Bit16 &x); //P盒，根据选择加密或解密模式选择P盒或P盒的逆
void printBinary(Bit16 x);

//线性密码分析和差分密码分析
void get_linear_file(int T);
void linear_analysis(void);
void get_differential_file(int T);
void differential_analysis(void);

//SPN增强
void spn_plus(int mode, Bit8 x[8], Bit8 Key[16]);
void s_box_plus(int mode, int r, Bit8 x[8]);
void p_box_plus(int mode, int r, Bit8 x[8]);
void printHex(Bit8 x[8]);

//随机性检测
void rand_test_spn(void);
long file_size(FILE *fp);
void CBC_Encrypt_spn(char *sourceFile, char *targetFile, Bit32 MyKey32);
void CBC_Encrypt_spn_plus(char *sourceFile, char *targetFile, Bit8 MyKey128[16]);
void CBC_Decrypt_spn_plus(char *sourceFile, char *targetFile, Bit8 MyKey128[16]);