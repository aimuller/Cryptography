#include <iostream>
#include <stdio.h>
#include "spn.h"
#include "rsa.h"
#include "file_encrypt_system.h"
#include "rainbow_table.h"
using namespace std;
void printMain(void);
void printSPN(void);
void printRSA(void);
void printFileSys(void);
void printRaibow(void);
int main(void){
	int op = 1;
	while (op){
		printMain();
		scanf("%d", &op);

		switch (op){
		case 1:
			printSPN();
			break;
		case 2:
			printRSA();
			break;
		case 3:
			printFileSys();
			break;
		case 4:
			printRaibow();
		default:;
		}
	}
	cout <<"\n";
	return 0;
}

void printMain(void){
	system("CLS");
	printf("\n");
	printf("===============请选择要测试的功能==============\n");
	printf("  1.SPN  2.RSA  3.文件加密  4.彩虹表  0.退出\n");
	printf("===============================================\n");
	printf("请输入[0-4]:");

}
void printSPN(void){
	int op = 1, i;
	Bit16 X16;
	Bit8 X64[8], Key128[16];
	while (op){
		system("CLS");
		printf("\n=================请选择要测试的项目================\n");
		printf("  1.原始SPN  2.原始SPN线性分析  3.原始SPN差分分析\n");
		printf("  4.增强SPN  5.随机性检测       0.退出\n");
		printf("===================================================\n");
		printf("请输入[0-5]:");
		scanf("%d", &op);
		switch (op){
		case 1:
			X16 = 0x26b7;
			cout << "\n使用原始SPN加密明文0x26b7\n";
			cout << "密钥为:0x" << hex << MyKey << "\n\n";
			spn(1, X16, MyKey);
			cout << "\n加密结果为:0x" << hex << X16 << "\n\n";
			cout << "===================================\n\n";
			cout << "解密密文0x" << hex << X16 << "\n";
			spn(2, X16, MyKey);
			cout << "\n解密结果为:0x" << hex << X16 << endl;
			cout << "\n按回车继续......";
			getchar(); getchar();
			break;
		case 2:
			cout << "\n对原始SPN进行线性分析\n";
			linear_analysis();
			cout << "\n按回车继续......";
			getchar(); getchar();
			break;

		case 3:
			cout << "\n对原始SPN进行差分分析\n";
			differential_analysis();
			cout << "\n按回车继续......";
			getchar(); getchar();
			break;

		case 4:
			for (i = 0; i < 8; i++)
				X64[i] = 0xff;
			for (i = 0; i < 16; i++)
				Key128[i] = 0xff;
			cout << "\n使用增强SPN加密明文0xffffffffffffffff (共64bit)\n";
			cout << "密钥为:0xffffffffffffffffffffffffffffffff (共128bit)\n";
			spn_plus(1, X64, Key128);
			cout << "\n加密结果为:0x";
			for (i = 0; i < 8; i++)
				printf("%x", X64[i]);
			cout << "\n\n";
			cout << "===================================\n\n";
			cout << "解密密文0x";
			for (i = 0; i < 8; i++)
				printf("%x", X64[i]);
			cout << "\n";
			spn_plus(2, X64, Key128);
			cout << "解密结果为:0x";
			for (i = 0; i < 8; i++)
				printf("%x", X64[i]);
			cout << "\n\n";
			cout << "\n按回车继续......";
			getchar(); getchar();
			break;
		case 5:
			cout << "\n对原始SPN算法和SPN增强算法进行随机性检测\n";
			cout << "两个算法的加密密钥都为为全0, 加密模式为CBC\n";
			cout << "加密的文件为AllZero.txt, 内容为全0, 大小约11M\n\n\n";
			rand_test_spn();
			cout << "\n按回车继续......";
			getchar(); getchar();
			break;

		default:;
		}
	}
}
void printRSA(void){
	int op = 1;
	int flag = 0;
	clock_t start, end;
	mpz_t n, p, q, e, d;
	mpz_inits(n, p, q, e, d, NULL);
	while (op){
		system("CLS");
		printf("\n===================请选择要测试的项目=================\n");
		printf("  1.生成RSA的5个参数  2.比较不同方法加速RSA  0.退出\n");
		printf("======================================================\n");
		printf("请输入[0-2]:");
		scanf("%d", &op);

		switch (op){
		case 1:
			cout << "\n正在获取5个参数，请等待......\n\n";
			start = clock();
			getRSAparam(n, p, q, e, d); //获得RSA的5个参数
			end = clock();
			flag = 1;
			cout << "参数p(" << mpz_sizeinbase(p, 2) << "bit):\n";
			gmp_printf("Hex: %Zx\n\n", p);

			cout << "参数q(" << mpz_sizeinbase(q, 2) << "bit):\n";
			gmp_printf("Hex: 0x%Zx\n\n", q);

			cout << "参数n(" << mpz_sizeinbase(n, 2) << "bit):\n";
			gmp_printf("Hex: 0x%Zx\n\n", n);

			cout << "参数e(" << mpz_sizeinbase(e, 2) << "bit):\n";
			gmp_printf("Hex: 0x%Zx\n\n", e);

			cout << "参数d(" << mpz_sizeinbase(d, 2) << "bit):\n";
			gmp_printf("Hex: 0x%Zx\n\n", d);
			printf("获取5个RSA参数的时间为%.3f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
			cout << "\n按回车继续......";
			getchar(); getchar();
			break;
		case 2:
			if (flag)
				RSA_CMP(n, p, q, e, d);
			else{
				cout << "\n请先使用功能1生成RSA算法的5个参数\n";
				cout << "\n按回车继续......";
				getchar(); getchar();
			}
			break;
		default:;
		}
	}
	mpz_clears(n, p, q, e, d, NULL);
}
void printFileSys(void){
	int op = 1, flag = 0, siglen;
	char encryptfile[100], decryptfile[100];
	//list();
	EC_KEY *aliceKey = EC_KEY_new();
	EC_KEY *bobKey = EC_KEY_new();	
	getEccKey(aliceKey, bobKey);

	while (op){
		system("CLS");
		printf("\n========请选择要测试的项目==========\n");
		printf("  1.文件加密  2.文件解密  0.退出\n");
		printf("===================================\n");
		printf("请输入[0-2]:");
		scanf("%d", &op);

		switch (op){
		case 1:
			cout << "\n请输入要加密的文件: ";
			cin >> encryptfile;
			fileEncrypt(encryptfile, aliceKey);
			flag = 1;
			getchar(); getchar();
			break;
		case 2:
			if (!flag){
				cout << "\n请先使用功能1加密文件\n";
				cout << "\n按回车继续......";
				getchar(); getchar();
				break;
			}
			cout << "\n请输入要解密的文件: ";
			cin >> decryptfile;
			fileDecrypt(decryptfile, bobKey);
			flag = 0;
			getchar(); getchar();
			break;
		default:;
		}
	}
}
void printRaibow(void){
	Bit8 PWD_CHAR[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	int op = 1, len;
	Bit8 targetmd[MD5_DIGEST_LENGTH];
	char pwd[10], filename[100];
	while (op){
		system("CLS");
		printf("\n===========请选择要测试的项目=========\n");
		printf("  1.生成彩虹表  2.口令破解  0.退出\n");
		printf("======================================\n");
		printf("请输入[0-2]:");
		scanf("%d", &op);
		switch (op){
		case 1:
			cout << "\n请输入口令长度(3-6):";
			cin >> len;
			if (len > 7){
				cout << "请输入3-6" << endl;
				cout << "\n按回车继续......";
				getchar(); getchar();
				break;
			}
			cout << "请输入要保存的彩虹表文件名:";
			cin >> filename;

			cout << "\n请等待......\n";
			getRainbowTable(filename, len);
			cout << "\n成功生成彩虹表文件" << filename << endl;
			cout << "\n按回车继续......";
			getchar(); getchar();
			break;
		case 2:
			cout << "\n请输入要测试的口令(3-6位):";
			cin >> pwd;
			len = strlen(pwd);
			MD5((Bit8 *)pwd, len, targetmd); //获取测试口令的MD5哈希值
			cout << "其MD5哈希值为: ";
			printKey(targetmd, MD5_DIGEST_LENGTH); //打印该口令的HASH值
			RainbowTable(len, targetmd);   //使用彩虹表破解该哈希值
			//for (int k = 7; k < 8; k++){
			//	len = k;
			//	srand(time(NULL));
			//	for (int i = 0; i < 10; i++){
			//		for (int j = 0; j < len; j++)
			//			pwd[j] = PWD_CHAR[rand() % 36];
			//		pwd[len] = '\0';
			//		cout << pwd << endl;
			//		MD5((Bit8 *)pwd, len, targetmd); //获取测试口令的MD5哈希值
			//		cout << "其MD5哈希值为: ";
			//		printKey(targetmd, MD5_DIGEST_LENGTH); //打印该口令的HASH值
			//		RainbowTable(len, targetmd);   //使用彩虹表破解该哈希值
			//	}
			//}
			//cout << "\n按回车继续......";
			//getchar(); getchar();
			//break;

		default:;
		}
	}
}