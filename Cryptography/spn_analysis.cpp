#include "spn.h"
void get_linear_file(int T){
	FILE *fp;
	Bit16 X, Y;
	fp = fopen("linear_file.txt", "wb");
	srand(time(NULL));
	for (int i = 0; i<T; i++)
	{
		X = rand();  //随机获取明文
		Y = X;
		spn(0, Y, MyKey);   //获取密文
		fwrite(&X, sizeof(Bit16), 1, fp);  //将明文存入文件中
		fwrite(&Y, sizeof(Bit16), 1, fp);  //将密文存入文件中
	}
	fclose(fp);
}

void linear_analysis(void){
	FILE *fp;
	int T=8000;
	int Count[256];  //计数器 
	Bit16 deSbox[16] = { 0xe, 0x3, 0x4, 0x8, 0x1, 0xc, 0xa, 0xf, 0x7, 0xd, 0x9, 0x6, 0xb, 0x2, 0x0, 0x5 };
	Bit16 *Tfile = (Bit16 *)malloc(sizeof(Bit16) * T * 2);
	get_linear_file(T);  //获取8000对明密文对
	fp = fopen("linear_file.txt", "rb");
	fread(Tfile, sizeof(Bit16) * T * 2, 1, fp); //将所有明密文对读入
	clock_t start1, start2, end1, end2;
	start1 = clock();
	for (int i = 0; i < 256; i++)
		Count[i] = 0;
	for (int i = 0; i < T; i++){
		Bit16 Z, X5, X7, X8, U6, U8, U14, U16;
		Bit16 X = Tfile[2 * i];
		Bit16 Y = Tfile[2 * i + 1];
		X5 = (X & 0x800) >> 11;
		X7 = (X & 0x200) >> 9;
		X8 = (X & 0x100) >> 8;
		for (int K = 0; K < 256; K++){
			Bit16 V2, V4;
			V2 = ((Y & 0xF00) >> 8) ^ ((K & 0xF0) >> 4);
			V4 = ((Y & 0xF) ^ (K & 0xF));
			U6 = (deSbox[V2] & 4) >> 2;
			U8 = (deSbox[V2] & 1);
			U14 = (deSbox[V4] & 4) >> 2;
			U16 = (deSbox[V4] & 1);
			Z = X5^X7^X8^U6^U8^U14^U16;
			if (!Z) Count[K]++;
		}
	}
	int max = -1;
	int tmp_key, key;
	for (int K=0; K < 256; K++){
		Count[K] = abs(Count[K] - T / 2);  //计算计数器与T/2的差值的绝对值
		if (Count[K] > max){
			max = Count[K];       //绝对值最大者说明偏差值最大，记录此时的密钥作为可能的8位密钥
			tmp_key = K;
		}
	}
	end1 = clock();
	printf("线性分析耗费时间为%.3f秒\n", (double)(end1 - start1) / CLOCKS_PER_SEC);
	cout << "线性分析出来的8位密钥为:0x";
	cout << hex << tmp_key << "\n\n";
	start2 = clock();
	cout << "请等待......" << "\n\n";
	for (Bit32 m = 0; m < 0xFFFF; m++){
		for (Bit32 n = 0; n < 0xF; n++){
			for (Bit32 t = 0; t < 0xF; t++){
				key = (m << 16) | (n << 12) | ((tmp_key & 0xF0) << 4) | (t << 4) | (tmp_key & 0xF);
				Bit16 p1 = 0x26b7;
				Bit16 p2 = 0x596b, p3;
				spn(0, p1, key);
				if (p1 == 0xbcd6)
				{
					p3 = p2;
					spn(0, p2, key);
					spn(0, p3, MyKey);  //再使用一对明密文确定最终结果
					if (p2 == p3){
						cout << "穷举后得到最终的32bit密钥为:0x" << hex << key << endl;
						end2 = clock();
						printf("穷举其余24bit密钥耗费时间为%.3f秒\n\n", (double)(end2 - start2) / CLOCKS_PER_SEC);
						printf("总耗费时间为%.3f秒\n", (double)(end1 + end2 - start1 - start2) / CLOCKS_PER_SEC);
						return;
					}
				}
			}
		}
	}
	cout << "线性分析错误！请再试一次\n";
}

void get_differential_file(int T){
	FILE *fp;
	Bit16 X1, X2, Y1, Y2;
	fp = fopen("differential_file.txt", "wb");
	srand(time(NULL));
	for (int i = 0; i<T; i++)
	{
		X1 = rand();  //随机获取明文X1
		X2 = X1 ^ 0xb00;  //X1与1011异或获取X2
		Y1 = X1;
		Y2 = X2;
		spn(0, Y1, MyKey);   //获取密文Y1
		spn(0, Y2, MyKey);   //获取密文Y2
		fwrite(&X1, sizeof(Bit16), 1, fp);  //将明文存入文件中
		fwrite(&Y1, sizeof(Bit16), 1, fp);  //将密文文存入文件中
		fwrite(&X2, sizeof(Bit16), 1, fp);  //将明文存入文件中
		fwrite(&Y2, sizeof(Bit16), 1, fp);  //将密文存入文件中
	}
	fclose(fp);
}

void differential_analysis(void){
	FILE *fp;
	int T = 8000;
	int Count[256];  //计数器 
	Bit16 deSbox[16] = { 0xe, 0x3, 0x4, 0x8, 0x1, 0xc, 0xa, 0xf, 0x7, 0xd, 0x9, 0x6, 0xb, 0x2, 0x0, 0x5 };
	Bit16 *Tfile = (Bit16 *)malloc(sizeof(Bit16) * T * 4);
	get_differential_file(T);  //获取300对四元组
	fp = fopen("differential_file.txt", "rb");
	fread(Tfile, sizeof(Bit16) * T * 4, 1, fp); //将所有四元组读入
	clock_t start1, start2, end1, end2;
	start1 = clock();
	for (int i = 0; i < 256; i++)  //计数器置零
		Count[i] = 0;
	for (int i = 0; i < T; i++){
		Bit16 X1, X2, Y1, Y2;
		X1 = Tfile[4 * i];      Y1 = Tfile[4 * i + 1];
		X2 = Tfile[4 * i + 2];  Y2 = Tfile[4 * i + 3];
		if (((Y1 & 0xF000) == (Y2 & 0xF000)) && ((Y1 & 0xF0) == (Y2 & 0xF0))) { //过滤错误对
			for (Bit16 K = 0; K < 256; K++){
				Bit16 V1, V2, V3, V4, U1, U2, U3, U4, U5, U6;
				V1 = ((Y1 & 0xF00) >> 8) ^ ((K & 0xF0) >> 4);	
				V2 = (Y1 & 0xF) ^ (K & 0xF);
				U1 = deSbox[V1];
				U2 = deSbox[V2];
				V3 = ((Y2 & 0xF00) >> 8) ^ ((K & 0xF0) >> 4);							
				V4 = (Y2 & 0xF) ^ (K & 0xF);	
				U3 = deSbox[V3];
				U4 = deSbox[V4];

				U5 = U1^U3;
				U6 = U2^U4;
				if ((U5 == 0x6) && (U6 == 0x6))
					Count[K]++;
			}
		}
	}
	int max = -1;
	int tmp_key, key;
	for (int K = 0; K < 256; K++){
		if (Count[K] > max){
			max = Count[K];  tmp_key = K;
		}
	}
	end1 = clock();
	printf("差分分析耗费时间为%.3f秒\n", (double)(end1 - start1) / CLOCKS_PER_SEC);
	cout << "差分分析出来的8位密钥为:0x";
	cout << hex << tmp_key << "\n\n";
	cout << "请等待......" << "\n\n";
	start2 = clock();
	for (Bit32 m = 0; m < 0xFFFF; m++){
		for (Bit32 n = 0; n < 0xF; n++){
			for (Bit32 t = 0; t < 0xF; t++){
				key = (m << 16) | (n << 12) | ((tmp_key & 0xF0) << 4) | (t << 4) | (tmp_key & 0xF);
				Bit16 p1 = 0x26b7;
				Bit16 p2 = 0x596b, p3;
				spn(0, p1, key);
				if (p1 == 0xbcd6)
				{
					p3 = p2;
					spn(0, p2, key);
					spn(0, p3, MyKey);  //再使用一对明密文确定最终结果
					if (p2 == p3){
						cout << "穷举后得到最终的32bit密钥为:0x" << hex << key << endl;
						end2 = clock();
						printf("穷举其余24bit密钥耗费时间为%.3f秒\n\n", (double)(end2 - start2) / CLOCKS_PER_SEC);
						printf("总耗费时间为%.3f秒\n", (double)(end1 + end2 - start1 - start2) / CLOCKS_PER_SEC);
						return;
					}
				}
			}
		}
	}
	cout << "差分分析错误！请再试一次\n";
}