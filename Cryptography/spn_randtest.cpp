#include "spn.h"

void rand_test_spn(void){
	char sourceFile[50] = "AllZero.txt";  //����ȫ0��Ŀ���ļ�
	char targetFile1[50] = "AllZero_after_spn.txt"; //ԭʼSPN���ܺ���ļ�
	char targetFile2[50] = "AllZero_after_spn_plus.txt"; //��ǿSPN���ܺ���ļ�
	Bit32 MyKey32 = 0x0;
	Bit8 MyKey128[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	cout << "��ȴ�......\n";
	CBC_Encrypt_spn(sourceFile, targetFile1, MyKey32); //����CBCģʽ����ԭʼSPN����
	cout << "������ԭʼSPN�����ļ� AllZero_after_spn.txt\n\n";
	cout << "��ȴ�......\n";
	CBC_Encrypt_spn_plus(sourceFile, targetFile2, MyKey128); //����CBCģʽ������ǿSPN����
	cout << "��������ǿSPN�����ļ� AllZero_after_spn_plus.txt\n";
}

void CBC_Encrypt_spn(char *sourceFile, char *targetFile, Bit32 MyKey32){
	FILE* fp = fopen(sourceFile, "rb");//Ҫ���ܵ��ļ�
	FILE* fc = fopen(targetFile, "wb");//���ܺ���ļ�
	Bit16 *testFile;
	long FSize, mySize, SaveSize;//�ļ���С
	int pad;  //��Ҫ�����ֽ���

	Bit16 tmp;//�м���
	Bit16 IV = 0x0;//��ʼ����

	//����PKCS#5��ʽ���ж̿鴦��
	FSize = file_size(fp);  //��ȡ�ļ���С�����ֽ�Ϊ��λ
	SaveSize = FSize;

	pad = FSize % 2;   //ԭʼSPN���鳤��Ϊ16bit���������ֽ�,ģ2��ȡҪ���ĳ���
	if (pad == 0)     //���ļ���С�պ�16bit��������������PKCS#5�����ٲ�2���ֽ�
	{
		FSize += 2;
		mySize = FSize / 2;
		testFile = (Bit16 *)malloc(mySize*sizeof(Bit16));
		testFile[mySize - 1] = 0;  //���ӵ�2���ֽڶ����0
	}
	else
	{
		FSize += 2 - pad;
		mySize = FSize / 2;
		testFile = (Bit16 *)malloc(mySize*sizeof(Bit16));
		testFile[mySize - 1] = (testFile[mySize - 1] & 0xFF00) | Bit8(2 - pad);  // /���ӵ�2-pad���ֽڶ����2-pad
	}
	fread(testFile, sizeof(Bit8), SaveSize, fp);//��Ҫ�����ļ������ݰ��ֽڿ�����testFile��
	//����CBCģʽ����testFile
	tmp = IV ^ testFile[0];  //��һ��ʹ�ó�ʼ����
	spn(0, tmp, MyKey32);//ʹ��ԭʼSPN�㷨����
	testFile[0] = tmp;
	for (int i = 1; i < mySize; i++)
	{
		tmp ^= testFile[i];
		spn(0, tmp, MyKey32);//����
		testFile[i] = tmp;
	}

	//�����ܺ�õ���testFileд���ļ���
	fwrite(testFile, mySize*sizeof(Bit16), 1, fc);

	fclose(fc);
	fclose(fp);
}

void CBC_Encrypt_spn_plus(char *sourceFile, char *targetFile, Bit8 MyKey128[16]){
	//Bit8 MyKey128[16] = { 0x3a, 0x94, 0xd6, 0x3f, 0xf5, 0x96, 0xae, 0xf0, 0xe2, 0xb4, 0x16, 0x28, 0x9d, 0xbc, 0x2e, 0xff };
	FILE* fp = fopen(sourceFile, "rb");//Ҫ���ܵ��ļ�
	FILE* fc = fopen(targetFile, "wb");//���ܺ���ļ�
	Bit8 *testFile;
	long FSize;//�ļ���С
	int pad;  //��������ݿ�

	Bit8 tmp[8];//�м���
	Bit8 IV[8] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };//��ʼ����

	//����PKCS#5��ʽ���ж̿鴦��
	FSize = file_size(fp);  //��ȡ�ļ���С�����ֽ�Ϊ��λ

	pad = FSize % 8;   //��ǿSPN���鳤��Ϊ64bit����8���ֽ�,ģ8��ȡҪ���ĳ���
	if (pad == 0){     //���ļ���С�պ�64bit��������������PKCS#5�����ٲ�8���ֽ�
		FSize = FSize + 8;
		testFile = (Bit8 *)malloc(FSize*sizeof(Bit8));
		for (int i = FSize - 8; i < FSize; i++)
			testFile[i] = 0;  //���ӵ�8���ֽڶ����0
	}
	else{
		FSize = FSize + 8 - pad;
		testFile = (Bit8 *)malloc(FSize*sizeof(Bit8));
		for (int i = FSize - 8 + pad; i < FSize; i++)
			testFile[i] = 8 - pad;
	}
	fread(testFile, sizeof(Bit8), FSize, fp);//��Ҫ�����ļ������ݰ��ֽڿ�����testFile��
	//����CBCģʽ����testFile
	for (int i = 0; i < 8; i++)
		tmp[i] = IV[i] ^ testFile[i];  //��һ��ʹ�ó�ʼ����

	spn_plus(1, tmp, MyKey128);//ʹ�ü�ǿSPN�㷨����

	for (int i = 0; i < 8; i++)
		testFile[i] = tmp[i];

	for (int i = 8; i<FSize; i+=8){
		for (int j = 0; j < 8; j++)
			tmp[j] = tmp[j] ^ testFile[i + j];
		spn_plus(1, tmp, MyKey128);//����
		for (int j = 0; j < 8; j++)
			testFile[i + j] = tmp[j];
	}

	//�����ܺ�õ���testFileд���ļ���
	fwrite(testFile, sizeof(Bit8), FSize, fc);
	fclose(fc);
	fclose(fp);
}

void CBC_Decrypt_spn_plus(char *sourceFile, char *targetFile, Bit8 MyKey128[16]){
	//Bit8 MyKey128[16] = { 0x3a, 0x94, 0xd6, 0x3f, 0xf5, 0x96, 0xae, 0xf0, 0xe2, 0xb4, 0x16, 0x28, 0x9d, 0xbc, 0x2e, 0xff };

	FILE* fp = fopen(sourceFile, "rb");//Ҫ���ܵ��ļ�
	FILE* fc = fopen(targetFile, "wb");//���ܺ���ļ�
	Bit8 *testFile;
	long FSize;//�ļ���С
	int pad;  //��������ݿ�

	Bit8 tmp[8],before[8];//�м���
	Bit8 IV[8] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };//��ʼ����

	FSize = file_size(fp);  //��ȡ�ļ���С�����ֽ�Ϊ��λ
	testFile = (Bit8 *)malloc(FSize*sizeof(Bit8));

	fread(testFile, sizeof(char), FSize, fp);//��Ҫ�����ļ������ݰ��ֽڿ�����testFile��
	//����CBCģʽ����testFile
	
	
	for (int j = 0; j < 8; j++)
		tmp[j] = testFile[j];
	spn_plus(2, tmp, MyKey128);//����
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
		spn_plus(2, tmp, MyKey128);//����
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

	//�����ܺ�õ���testFileд���ļ���
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