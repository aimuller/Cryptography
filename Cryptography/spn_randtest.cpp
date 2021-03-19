#include "spn.h"

void rand_test_spn(void){
	char sourceFile[50] = "AllZero.txt";  //内容全0的目标文件
	char targetFile1[50] = "AllZero_after_spn.txt"; //原始SPN加密后的文件
	char targetFile2[50] = "AllZero_after_spn_plus.txt"; //增强SPN加密后的文件
	Bit32 MyKey32 = 0x0;
	Bit8 MyKey128[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	cout << "请等待......\n";
	CBC_Encrypt_spn(sourceFile, targetFile1, MyKey32); //采用CBC模式进行原始SPN加密
	cout << "已生成原始SPN加密文件 AllZero_after_spn.txt\n\n";
	cout << "请等待......\n";
	CBC_Encrypt_spn_plus(sourceFile, targetFile2, MyKey128); //采用CBC模式进行增强SPN加密
	cout << "已生成增强SPN加密文件 AllZero_after_spn_plus.txt\n";
}

void CBC_Encrypt_spn(char *sourceFile, char *targetFile, Bit32 MyKey32){
	FILE* fp = fopen(sourceFile, "rb");//要加密的文件
	FILE* fc = fopen(targetFile, "wb");//加密后的文件
	Bit16 *testFile;
	long FSize, mySize, SaveSize;//文件大小
	int pad;  //需要填充的字节数

	Bit16 tmp;//中间量
	Bit16 IV = 0x0;//初始向量

	//采用PKCS#5方式进行短块处理
	FSize = file_size(fp);  //获取文件大小，以字节为单位
	SaveSize = FSize;

	pad = FSize % 2;   //原始SPN分组长度为16bit，即两个字节,模2获取要填充的长度
	if (pad == 0)     //若文件大小刚好16bit的整数倍，根据PKCS#5规则再补2个字节
	{
		FSize += 2;
		mySize = FSize / 2;
		testFile = (Bit16 *)malloc(mySize*sizeof(Bit16));
		testFile[mySize - 1] = 0;  //增加的2个字节都填充0
	}
	else
	{
		FSize += 2 - pad;
		mySize = FSize / 2;
		testFile = (Bit16 *)malloc(mySize*sizeof(Bit16));
		testFile[mySize - 1] = (testFile[mySize - 1] & 0xFF00) | Bit8(2 - pad);  // /增加的2-pad个字节都填充2-pad
	}
	fread(testFile, sizeof(Bit8), SaveSize, fp);//将要加密文件中内容按字节拷贝至testFile中
	//采用CBC模式加密testFile
	tmp = IV ^ testFile[0];  //第一轮使用初始向量
	spn(0, tmp, MyKey32);//使用原始SPN算法加密
	testFile[0] = tmp;
	for (int i = 1; i < mySize; i++)
	{
		tmp ^= testFile[i];
		spn(0, tmp, MyKey32);//加密
		testFile[i] = tmp;
	}

	//将加密后得到的testFile写入文件中
	fwrite(testFile, mySize*sizeof(Bit16), 1, fc);

	fclose(fc);
	fclose(fp);
}

void CBC_Encrypt_spn_plus(char *sourceFile, char *targetFile, Bit8 MyKey128[16]){
	//Bit8 MyKey128[16] = { 0x3a, 0x94, 0xd6, 0x3f, 0xf5, 0x96, 0xae, 0xf0, 0xe2, 0xb4, 0x16, 0x28, 0x9d, 0xbc, 0x2e, 0xff };
	FILE* fp = fopen(sourceFile, "rb");//要加密的文件
	FILE* fc = fopen(targetFile, "wb");//加密后的文件
	Bit8 *testFile;
	long FSize;//文件大小
	int pad;  //多出的数据块

	Bit8 tmp[8];//中间量
	Bit8 IV[8] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };//初始向量

	//采用PKCS#5方式进行短块处理
	FSize = file_size(fp);  //获取文件大小，以字节为单位

	pad = FSize % 8;   //增强SPN分组长度为64bit，即8个字节,模8获取要填充的长度
	if (pad == 0){     //若文件大小刚好64bit的整数倍，根据PKCS#5规则再补8个字节
		FSize = FSize + 8;
		testFile = (Bit8 *)malloc(FSize*sizeof(Bit8));
		for (int i = FSize - 8; i < FSize; i++)
			testFile[i] = 0;  //增加的8个字节都填充0
	}
	else{
		FSize = FSize + 8 - pad;
		testFile = (Bit8 *)malloc(FSize*sizeof(Bit8));
		for (int i = FSize - 8 + pad; i < FSize; i++)
			testFile[i] = 8 - pad;
	}
	fread(testFile, sizeof(Bit8), FSize, fp);//将要加密文件中内容按字节拷贝至testFile中
	//采用CBC模式加密testFile
	for (int i = 0; i < 8; i++)
		tmp[i] = IV[i] ^ testFile[i];  //第一轮使用初始向量

	spn_plus(1, tmp, MyKey128);//使用加强SPN算法加密

	for (int i = 0; i < 8; i++)
		testFile[i] = tmp[i];

	for (int i = 8; i<FSize; i+=8){
		for (int j = 0; j < 8; j++)
			tmp[j] = tmp[j] ^ testFile[i + j];
		spn_plus(1, tmp, MyKey128);//加密
		for (int j = 0; j < 8; j++)
			testFile[i + j] = tmp[j];
	}

	//将加密后得到的testFile写入文件中
	fwrite(testFile, sizeof(Bit8), FSize, fc);
	fclose(fc);
	fclose(fp);
}

void CBC_Decrypt_spn_plus(char *sourceFile, char *targetFile, Bit8 MyKey128[16]){
	//Bit8 MyKey128[16] = { 0x3a, 0x94, 0xd6, 0x3f, 0xf5, 0x96, 0xae, 0xf0, 0xe2, 0xb4, 0x16, 0x28, 0x9d, 0xbc, 0x2e, 0xff };

	FILE* fp = fopen(sourceFile, "rb");//要加密的文件
	FILE* fc = fopen(targetFile, "wb");//加密后的文件
	Bit8 *testFile;
	long FSize;//文件大小
	int pad;  //多出的数据块

	Bit8 tmp[8],before[8];//中间量
	Bit8 IV[8] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };//初始向量

	FSize = file_size(fp);  //获取文件大小，以字节为单位
	testFile = (Bit8 *)malloc(FSize*sizeof(Bit8));

	fread(testFile, sizeof(char), FSize, fp);//将要加密文件中内容按字节拷贝至testFile中
	//采用CBC模式解密testFile
	
	
	for (int j = 0; j < 8; j++)
		tmp[j] = testFile[j];
	spn_plus(2, tmp, MyKey128);//解密
	for (int j = 0; j < 8; j++)
		tmp[j] = tmp[j] ^ IV[j];
	for (int j = 0; j < 8; j++)
		before[j] = testFile[j];
	for (int j = 0; j < 8; j++)
		testFile[j] = tmp[j];


	for (int i = 8; i<FSize; i += 8)
	{
		for (int j = 0; j < 8; j++)
			tmp[j] = testFile[i + j];
		spn_plus(2, tmp, MyKey128);//解密
		for (int j = 0; j < 8; j++)
			tmp[j] = tmp[j] ^ before[j];
		for (int j = 0; j < 8; j++)
			before[j] = testFile[i + j];
		for (int j = 0; j < 8; j++)
			testFile[i + j] = tmp[j];
	}


	pad = testFile[FSize-1];  
	if (pad == 0)    
		FSize = FSize - 8;
	else
		FSize = FSize - pad;

	//将解密后得到的testFile写入文件中
	fwrite(testFile, sizeof(Bit8), FSize, fc);

	fclose(fc);
	fclose(fp);
}

long file_size(FILE *stream){
	long curpos, length;
	curpos = ftell(stream);
	fseek(stream, 0L, SEEK_END);
	length = ftell(stream);
	fseek(stream, curpos, SEEK_SET);
	return length;
}