#include "file_encrypt_system.h"
#include "spn.h"
#define ALICE_NID 928
#define BOB_NID 928
extern "C"
{
#include <openssl/applink.c>
}

//���ܲ���
//�ж��Ƿ���ڸ��ļ�

void fileEncrypt(char *filename, EC_KEY *aliceKey){
	FILE *fp = fopen(filename, "rb");
	if (!fp){
		cout << "�����ڸ��ļ�, ��������ȷ���ļ���\n";
		cout << "\n���س�����......";
		return;
	}
	fclose(fp);

	//��ȡ�ļ�ժҪֵ
	int diglen = 16;
	Bit8 fileDigest[MD5_DIGEST_LENGTH];
	getFileDig(filename, fileDigest);  

	//ʹ��Alice˽Կ���ļ�ժҪֵǩ��
	Bit8 fileSig[1024];
	unsigned int siglen = 0;
	ECDSA_sign(0, fileDigest, MD5_DIGEST_LENGTH, fileSig, &siglen, aliceKey);

	//��ǩ����Ϣ������ƴ�Ӳ�ѹ������������"encrypt_sys_tmp.txt"��
	CatAndZip(fileSig, siglen, filename);

	//��ȡ�ԳƼ����㷨����Կ
	Bit8 Key[16];
	getEncryptKey(Key);

	//ʹ�öԳƼ����㷨CBCģʽ��ѹ������ļ����ܣ��������ܺ�����ݴ�ŵ��ļ�savename��
	char savename[100];
	cout << "������Ҫ���������ļ���: ";
	cin >> savename;
	CBC_Encrypt_spn_plus("encrypt_sys_tmp.txt", savename, Key);
	cout << "���ܺ���ļ��ѱ��浽" << savename << "��!\n";
	cout << "\n���س�����......";
}

//������֤����
void fileDecrypt(char *filename, EC_KEY *bobKey){
	//�ж��Ƿ���ڸ��ļ�
	FILE *fp = fopen(filename, "rb");
	if (!fp){
		cout << "�����ڸ��ļ�, ��������ȷ���ļ���\n";
		cout << "\n���س�����......";
		return;
	}
	fclose(fp);

	//����newKey��Կ
	Bit8 buf[1024], *p;
	fp = fopen("new_pub_key.txt", "rb");
	long len = file_size(fp);
	fread(buf, sizeof(Bit8), len, fp);
	fclose(fp);
	p = buf;
	EC_KEY *new_pub_key = EC_KEY_new_by_curve_name(BOB_NID);
	new_pub_key = o2i_ECPublicKey(&new_pub_key, (const unsigned char **)&p, len);

	//ʹ��newKey�Ĺ�Կ��Bob��˽Կ��ECDH����ʹ����Կ����������ü�����ԿKey
	Bit8 Key[16];
	ECDH_compute_key(Key, MD5_DIGEST_LENGTH, EC_KEY_get0_public_key(new_pub_key), bobKey, KDF_MD5);

	//ʹ��Key���ܻ��ѹ���ļ�, ���������ļ�encrypt_sys_tmp.txt��
	CBC_Decrypt_spn_plus(filename, "encrypt_sys_tmp.txt", Key);

	//��ѹ�ļ�������ĺ�ǩ����ƴ����Ϣ������ǩ����Ϣ�����ķֿ�����
	int siglen;
	fp = fopen("encrypt_sys_tmp.txt", "rb");
	len = file_size(fp) - sizeof(int);   
	fread(&siglen, sizeof(int), 1, fp); //�ȶ���ǩ������
	Bit8 *zipBuf = (Bit8 *)malloc(sizeof(Bit8)*len);
	Bit8 *sigBuf = (Bit8 *)malloc(sizeof(Bit8)*len);
	
	fread(zipBuf, sizeof(Bit8), len, fp);
	fclose(fp);
	COMP_CTX *ctx;
	COMP_METHOD med;
	ctx = COMP_CTX_new(COMP_rle()); //��ʼ��������
	int total = COMP_expand_block(ctx, sigBuf, len * 2, zipBuf, len);
	COMP_CTX_free(ctx);

	//��������
	char savename[100];
	cout << "������Ҫ����������ļ���: ";
	cin >> savename;
	fp = fopen(savename, "wb");
	fwrite((sigBuf + siglen), sizeof(Bit8), (total - siglen), fp);
	fclose(fp);

	//��ȡBob�յ�������ժҪ
	Bit8 fileDig[MD5_DIGEST_LENGTH];
	getFileDig(savename, fileDig);

	//ǩ����֤
	Verify(fileDig, MD5_DIGEST_LENGTH, sigBuf, siglen);
	cout << "���ܺ���ļ��ѱ��浽" << savename << "��!\n";
	cout << "\n���س�����......\n";
	free(zipBuf);
	free(sigBuf);
}

//��ȡAlice��Bob��EC��Կ�����������߹�Կ
void getEccKey(EC_KEY *aliceKey, EC_KEY *bobKey){
	FILE *fp;
	EC_GROUP *aliceGroup, *bobGroup;
	//����ѡ�����Բ����������Կ����group
	aliceGroup = EC_GROUP_new_by_curve_name(ALICE_NID);
	bobGroup = EC_GROUP_new_by_curve_name(BOB_NID);
	//������Կ����
	EC_KEY_set_group(aliceKey, aliceGroup);
	EC_KEY_set_group(bobKey, bobGroup);
	//�ֱ�����Alice��Bob����Կ
	EC_KEY_generate_key(aliceKey);
	EC_KEY_generate_key(bobKey);

	//����Alice��Bob�Ĺ�Կ
	Bit8 buf[1024], *p;
	p = buf;
	int len = i2o_ECPublicKey(aliceKey, &p);
	fp = fopen("alice_pub_key.txt", "wb");
	fwrite(buf, sizeof(Bit8), len, fp);//��Alice��Կ���浽�ļ���
	fclose(fp);

	p = buf;
	len = i2o_ECPublicKey(bobKey, &p);
	fp = fopen("bob_pub_key.txt", "wb");
	fwrite(buf, sizeof(Bit8), len, fp);//��Bob��Կ���浽�ļ���
	fclose(fp);
}

//��ȡ�ļ�filename��MD5ժҪֵ
void getFileDig(char *filename, Bit8 *fileHash){
	MD5_CTX ctx;
	Bit8 outmd[MD5_DIGEST_LENGTH];
	char buffer[1024];
	int len = 0;
	FILE * fp = NULL;
	memset(outmd, 0, sizeof(outmd));
	memset(buffer, 0, sizeof(buffer));

	fp = fopen(filename, "rb");
	if (fp == NULL)
		printf("Can't open file\n");

	MD5_Init(&ctx);
	while ((len = fread(buffer, 1, 1024, fp)) > 0){
		MD5_Update(&ctx, buffer, len);
		memset(buffer, 0, sizeof(buffer));
	}
	MD5_Final(outmd, &ctx);
	for (int i = 0; i < MD5_DIGEST_LENGTH; i < i++)
		fileHash[i] = outmd[i];
}

//��ǩ����Ϣ������ƴ�Ӳ�ѹ��
void CatAndZip(Bit8 *fileSig,int siglen, char *filename){
	FILE *fp = fopen(filename, "rb");
	long filelen = file_size(fp);
	Bit8 *buf = (Bit8 *)malloc(sizeof(Bit8)*(siglen + filelen));
	Bit8 *outbuf = (Bit8 *)malloc(sizeof(Bit8)*(siglen + filelen) * 2);
	memcpy(buf, fileSig, siglen);
	fread((buf + siglen), sizeof(Bit8), filelen, fp); //���ļ�����ƴ����ǩ������
	fclose(fp);

	COMP_CTX *ctx;
	COMP_METHOD med;
	ctx = COMP_CTX_new(COMP_rle()); //��ʼ��������

	int total = COMP_compress_block(ctx, outbuf, (siglen + filelen)*2, buf, siglen + filelen);
	COMP_CTX_free(ctx);
	
	fp = fopen("encrypt_sys_tmp.txt", "wb");
	int len = siglen;
	fwrite(&len, sizeof(int), 1, fp); //��д��ǩ������
	fwrite(outbuf, sizeof(Bit8), total, fp);  //д��ѹ������ļ�
	fclose(fp);
	free(buf);
	free(outbuf);
}

//��ȡ�ԳƼ����㷨��Կ
void getEncryptKey(Bit8 Key[MD5_DIGEST_LENGTH]){
	//��Bob��Կ��ȡ��buf��
	Bit8 buf[1024], *p;
	FILE *fp = fopen("bob_pub_key.txt", "rb");
	int len = file_size(fp);
	fread(buf, sizeof(Bit8), len, fp);
	p = (unsigned char *)buf;
	fclose(fp);

	//��Կ��һ����Բ�����ϵĵ㣬���빫Կ��Ҫ������Բ����
	EC_KEY *bob_pub_key = EC_KEY_new_by_curve_name(BOB_NID);
	//����Bob��Կ
	bob_pub_key = o2i_ECPublicKey(&bob_pub_key, (const unsigned char **)&p, len);

	//��ʱ����һ����ԿnewKey����Bob��Կ��ECDH����ʹ����Կ����������ü�����Կ
	EC_KEY *newKey = EC_KEY_new_by_curve_name(BOB_NID);
	EC_KEY_generate_key(newKey);
	ECDH_compute_key(Key, MD5_DIGEST_LENGTH, EC_KEY_get0_public_key(bob_pub_key), newKey, KDF_MD5);
	
	//��newKey�Ĺ�Կ������������ļ�"new_pub_key.txt"��
	p = buf;
	len = i2o_ECPublicKey(newKey, &p);
	fp = fopen("new_pub_key.txt", "wb");
	fwrite(buf, sizeof(Bit8), len, fp);//��newKey��Կ���浽�ļ���
	fclose(fp);
}

//MD5��Կ��������, ���ECDH�����ԳƼ����㷨��Կ
void *KDF_MD5(const void *in, size_t inlen, void *out, size_t *outlen)
{
	if (*outlen < MD5_DIGEST_LENGTH)
		return NULL;
	else
		*outlen = MD5_DIGEST_LENGTH;
	return MD5((const unsigned char *)in, inlen, (unsigned char *)out);
}

//ǩ����֤
void Verify(Bit8 *fileDig, int diglen, Bit8 *fileSig, int siglen){
	//��Alice��Կ��ȡ��buf��
	Bit8 buf[1024], *p;
	FILE *fp = fopen("alice_pub_key.txt", "rb");
	int len = file_size(fp);
	fread(buf, sizeof(Bit8), len, fp);
	p = (unsigned char *)buf;

	//��Կ��һ����Բ�����ϵĵ㣬���빫Կ��Ҫ������Բ����
	EC_KEY *alice_pub_key = EC_KEY_new();
	EC_GROUP *group = EC_GROUP_new_by_curve_name(ALICE_NID);
	EC_KEY_set_group(alice_pub_key, group);

	//����Alice��Կ
	alice_pub_key = o2i_ECPublicKey(&alice_pub_key, (const unsigned char **)&p, len);

	int ret = ECDSA_verify(0, (const unsigned char *)fileDig, diglen, fileSig, siglen, alice_pub_key);
	if (ret == 1)
		cout << "\nǩ����֤ͨ����\n";
	else
		cout << "\nǩ����֤��ͨ����\n";
}

void printKey(Bit8 *buf, int len){
	for (int i = 0; i < len; i++){
		if (i == len - 1)
			printf("%02X", buf[i]);
		else
			printf("%02X", buf[i]);
	}
	printf("\n");
}

int list(void){
	EC_builtin_curve *curves = NULL, *p;
	int curves_count, i;

	if (!(curves_count = EC_get_builtin_curves(NULL, 0)))
	{
		printf("Get built-in EC curves count failed!\n");
		return (-1);
	}
	if (!(curves = (EC_builtin_curve *)malloc(sizeof(EC_builtin_curve) * curves_count)))
	{
		printf("Allocate memory failed!\n");
		return (-1);
	}
	if (!(curves_count = EC_get_builtin_curves(curves, curves_count)))
	{
		printf("Get built-in EC curves info failed!\n");
		free(curves);
		return (-1);
	}

	printf("Total built-in EC curves count: %d\n", curves_count);
	printf("Built-in EC curves info:\n");
	p = curves;
	for (i = 0; i < curves_count; i++)
	{
		printf("EC curve item: %d\n", (i + 1));
		printf("NID: %d\n", p->nid);
		printf("Comment: %s\n", p->comment);
		p++;
	}

	free(curves);
	return 0;
}

