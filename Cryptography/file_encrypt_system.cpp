#include "file_encrypt_system.h"
#include "spn.h"
#define ALICE_NID 928
#define BOB_NID 928
extern "C"
{
#include <openssl/applink.c>
}

//加密部分
//判断是否存在该文件

void fileEncrypt(char *filename, EC_KEY *aliceKey){
	FILE *fp = fopen(filename, "rb");
	if (!fp){
		cout << "不存在该文件, 请输入正确的文件名\n";
		cout << "\n按回车继续......";
		return;
	}
	fclose(fp);

	//获取文件摘要值
	int diglen = 16;
	Bit8 fileDigest[MD5_DIGEST_LENGTH];
	getFileDig(filename, fileDigest);  

	//使用Alice私钥对文件摘要值签名
	Bit8 fileSig[1024];
	unsigned int siglen = 0;
	ECDSA_sign(0, fileDigest, MD5_DIGEST_LENGTH, fileSig, &siglen, aliceKey);

	//将签名信息与明文拼接并压缩，并保存在"encrypt_sys_tmp.txt"中
	CatAndZip(fileSig, siglen, filename);

	//获取对称加密算法的密钥
	Bit8 Key[16];
	getEncryptKey(Key);

	//使用对称加密算法CBC模式将压缩后的文件加密，并将加密后的内容存放到文件savename中
	char savename[100];
	cout << "请输入要保存密文文件名: ";
	cin >> savename;
	CBC_Encrypt_spn_plus("encrypt_sys_tmp.txt", savename, Key);
	cout << "加密后的文件已保存到" << savename << "中!\n";
	cout << "\n按回车继续......";
}

//解密验证部分
void fileDecrypt(char *filename, EC_KEY *bobKey){
	//判断是否存在该文件
	FILE *fp = fopen(filename, "rb");
	if (!fp){
		cout << "不存在该文件, 请输入正确的文件名\n";
		cout << "\n按回车继续......";
		return;
	}
	fclose(fp);

	//导入newKey公钥
	Bit8 buf[1024], *p;
	fp = fopen("new_pub_key.txt", "rb");
	long len = file_size(fp);
	fread(buf, sizeof(Bit8), len, fp);
	fclose(fp);
	p = buf;
	EC_KEY *new_pub_key = EC_KEY_new_by_curve_name(BOB_NID);
	new_pub_key = o2i_ECPublicKey(&new_pub_key, (const unsigned char **)&p, len);

	//使用newKey的公钥和Bob的私钥作ECDH，再使用密钥派生函数获得加密密钥Key
	Bit8 Key[16];
	ECDH_compute_key(Key, MD5_DIGEST_LENGTH, EC_KEY_get0_public_key(new_pub_key), bobKey, KDF_MD5);

	//使用Key解密获得压缩文件, 并保存在文件encrypt_sys_tmp.txt中
	CBC_Decrypt_spn_plus(filename, "encrypt_sys_tmp.txt", Key);

	//解压文件获得明文和签名的拼接信息，并将签名信息和明文分开处理
	int siglen;
	fp = fopen("encrypt_sys_tmp.txt", "rb");
	len = file_size(fp) - sizeof(int);   
	fread(&siglen, sizeof(int), 1, fp); //先读出签名长度
	Bit8 *zipBuf = (Bit8 *)malloc(sizeof(Bit8)*len);
	Bit8 *sigBuf = (Bit8 *)malloc(sizeof(Bit8)*len);
	
	fread(zipBuf, sizeof(Bit8), len, fp);
	fclose(fp);
	COMP_CTX *ctx;
	COMP_METHOD med;
	ctx = COMP_CTX_new(COMP_rle()); //初始化上下文
	int total = COMP_expand_block(ctx, sigBuf, len * 2, zipBuf, len);
	COMP_CTX_free(ctx);

	//导出明文
	char savename[100];
	cout << "请输入要保存的明文文件名: ";
	cin >> savename;
	fp = fopen(savename, "wb");
	fwrite((sigBuf + siglen), sizeof(Bit8), (total - siglen), fp);
	fclose(fp);

	//获取Bob收到的明文摘要
	Bit8 fileDig[MD5_DIGEST_LENGTH];
	getFileDig(savename, fileDig);

	//签名验证
	Verify(fileDig, MD5_DIGEST_LENGTH, sigBuf, siglen);
	cout << "解密后的文件已保存到" << savename << "中!\n";
	cout << "\n按回车继续......\n";
	free(zipBuf);
	free(sigBuf);
}

//获取Alice和Bob的EC密钥，并导出二者公钥
void getEccKey(EC_KEY *aliceKey, EC_KEY *bobKey){
	FILE *fp;
	EC_GROUP *aliceGroup, *bobGroup;
	//根据选择的椭圆曲线生成密钥参数group
	aliceGroup = EC_GROUP_new_by_curve_name(ALICE_NID);
	bobGroup = EC_GROUP_new_by_curve_name(BOB_NID);
	//设置密钥参数
	EC_KEY_set_group(aliceKey, aliceGroup);
	EC_KEY_set_group(bobKey, bobGroup);
	//分别生成Alice和Bob的密钥
	EC_KEY_generate_key(aliceKey);
	EC_KEY_generate_key(bobKey);

	//导出Alice和Bob的公钥
	Bit8 buf[1024], *p;
	p = buf;
	int len = i2o_ECPublicKey(aliceKey, &p);
	fp = fopen("alice_pub_key.txt", "wb");
	fwrite(buf, sizeof(Bit8), len, fp);//将Alice公钥保存到文件中
	fclose(fp);

	p = buf;
	len = i2o_ECPublicKey(bobKey, &p);
	fp = fopen("bob_pub_key.txt", "wb");
	fwrite(buf, sizeof(Bit8), len, fp);//将Bob公钥保存到文件中
	fclose(fp);
}

//获取文件filename的MD5摘要值
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

//将签名信息和明文拼接并压缩
void CatAndZip(Bit8 *fileSig,int siglen, char *filename){
	FILE *fp = fopen(filename, "rb");
	long filelen = file_size(fp);
	Bit8 *buf = (Bit8 *)malloc(sizeof(Bit8)*(siglen + filelen));
	Bit8 *outbuf = (Bit8 *)malloc(sizeof(Bit8)*(siglen + filelen) * 2);
	memcpy(buf, fileSig, siglen);
	fread((buf + siglen), sizeof(Bit8), filelen, fp); //将文件内容拼接在签名后面
	fclose(fp);

	COMP_CTX *ctx;
	COMP_METHOD med;
	ctx = COMP_CTX_new(COMP_rle()); //初始化上下文

	int total = COMP_compress_block(ctx, outbuf, (siglen + filelen)*2, buf, siglen + filelen);
	COMP_CTX_free(ctx);
	
	fp = fopen("encrypt_sys_tmp.txt", "wb");
	int len = siglen;
	fwrite(&len, sizeof(int), 1, fp); //先写入签名长度
	fwrite(outbuf, sizeof(Bit8), total, fp);  //写入压缩后的文件
	fclose(fp);
	free(buf);
	free(outbuf);
}

//获取对称加密算法密钥
void getEncryptKey(Bit8 Key[MD5_DIGEST_LENGTH]){
	//将Bob公钥读取到buf中
	Bit8 buf[1024], *p;
	FILE *fp = fopen("bob_pub_key.txt", "rb");
	int len = file_size(fp);
	fread(buf, sizeof(Bit8), len, fp);
	p = (unsigned char *)buf;
	fclose(fp);

	//公钥是一个椭圆曲线上的点，导入公钥需要构造椭圆曲线
	EC_KEY *bob_pub_key = EC_KEY_new_by_curve_name(BOB_NID);
	//导入Bob公钥
	bob_pub_key = o2i_ECPublicKey(&bob_pub_key, (const unsigned char **)&p, len);

	//临时生成一个密钥newKey，与Bob公钥作ECDH，再使用密钥派生函数获得加密密钥
	EC_KEY *newKey = EC_KEY_new_by_curve_name(BOB_NID);
	EC_KEY_generate_key(newKey);
	ECDH_compute_key(Key, MD5_DIGEST_LENGTH, EC_KEY_get0_public_key(bob_pub_key), newKey, KDF_MD5);
	
	//将newKey的公钥导出，存放在文件"new_pub_key.txt"中
	p = buf;
	len = i2o_ECPublicKey(newKey, &p);
	fp = fopen("new_pub_key.txt", "wb");
	fwrite(buf, sizeof(Bit8), len, fp);//将newKey公钥保存到文件中
	fclose(fp);
}

//MD5密钥派生函数, 配合ECDH导出对称加密算法密钥
void *KDF_MD5(const void *in, size_t inlen, void *out, size_t *outlen)
{
	if (*outlen < MD5_DIGEST_LENGTH)
		return NULL;
	else
		*outlen = MD5_DIGEST_LENGTH;
	return MD5((const unsigned char *)in, inlen, (unsigned char *)out);
}

//签名验证
void Verify(Bit8 *fileDig, int diglen, Bit8 *fileSig, int siglen){
	//将Alice公钥读取到buf中
	Bit8 buf[1024], *p;
	FILE *fp = fopen("alice_pub_key.txt", "rb");
	int len = file_size(fp);
	fread(buf, sizeof(Bit8), len, fp);
	p = (unsigned char *)buf;

	//公钥是一个椭圆曲线上的点，导入公钥需要构造椭圆曲线
	EC_KEY *alice_pub_key = EC_KEY_new();
	EC_GROUP *group = EC_GROUP_new_by_curve_name(ALICE_NID);
	EC_KEY_set_group(alice_pub_key, group);

	//导入Alice公钥
	alice_pub_key = o2i_ECPublicKey(&alice_pub_key, (const unsigned char **)&p, len);

	int ret = ECDSA_verify(0, (const unsigned char *)fileDig, diglen, fileSig, siglen, alice_pub_key);
	if (ret == 1)
		cout << "\n签名验证通过！\n";
	else
		cout << "\n签名验证不通过！\n";
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

