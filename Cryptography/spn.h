#include <iostream>
#include <stdlib.h>
#include <time.h>
#define MyKey 0x3a94d63f
typedef unsigned char Bit8;
typedef unsigned short Bit16;
typedef unsigned int Bit32;

using namespace std;

//ԭʼSPN�㷨ʵ��
void spn(int mode, Bit16 &x, Bit32 Key);
void one_round_spn(int mode, int r, Bit16 &x, Bit32 Key);  //һ��SPN�ӽ���
Bit16 round_key(int r, Bit32 Key);
void s_box(int mode, Bit16 &x); //S�У�����ѡ����ܻ����ģʽѡ��S�л�S�е���
void p_box(Bit16 &x); //P�У�����ѡ����ܻ����ģʽѡ��P�л�P�е���
void printBinary(Bit16 x);

//������������Ͳ���������
void get_linear_file(int T);
void linear_analysis(void);
void get_differential_file(int T);
void differential_analysis(void);

//SPN��ǿ
void spn_plus(int mode, Bit8 x[8], Bit8 Key[16]);
void s_box_plus(int mode, int r, Bit8 x[8]);
void p_box_plus(int mode, int r, Bit8 x[8]);
void printHex(Bit8 x[8]);

//����Լ��
void rand_test_spn(void);
long file_size(FILE *fp);
void CBC_Encrypt_spn(char *sourceFile, char *targetFile, Bit32 MyKey32);
void CBC_Encrypt_spn_plus(char *sourceFile, char *targetFile, Bit8 MyKey128[16]);
void CBC_Decrypt_spn_plus(char *sourceFile, char *targetFile, Bit8 MyKey128[16]);