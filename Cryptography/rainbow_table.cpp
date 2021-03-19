#include "rainbow_table.h"
#include <stdint.h>  
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
//3λ  RCount:2000  10000              
//4λ  RCount:2000  200000     Լ1min
//5λ  RCount:2000  3000000    Լ1min  10��8
//6λ  RCount:3000  10000000   Լ1min  10��5
int RCountList[4] = { 2000, 5000, 5000, 8000 };
long RNumList[4] = { 10000, 200000, 2000000, 10000000 };
int pwd_len; //�����
int RCount;  //����
long RNum;   //����
Bit8 PWD_CHAR[] = "0123456789abcdefghijklmnopqrstuvwxyz";

void RainbowTable(int len,Bit8 *targetmd){
	//clock_t start, end;
	//char filename[100];
	//strcpy(filename, "rainbow_table_3.txt");
	//filename[14] = '0' + len;
	//start = clock();
	//getPassword(filename, targetmd, len);
	//end = clock();
	//printf("�ķ�ʱ��Ϊ%.3f��\n\n\n", (double)(end - start) / CLOCKS_PER_SEC);
	int op = 1; 
	char filename[100];
	clock_t start, end;
	strcpy(filename, "rainbow_table_3.txt");
	printf("\n=======��ѡ��ʺ��========\n");
	printf("  1.�ֶ�����  2.ϵͳ�Դ� \n");
	printf("===========================\n");
	printf("������[1-2]:");
	scanf("%d", &op);

	switch (op){
	case 1:
		cout << "\n�������ļ���:";
		cin >> filename;
		cout << "\n��ȴ�......\n\n";
		start = clock();
		getPassword(filename, targetmd, len);
		end = clock();
		printf("�ķ�ʱ��Ϊ%.3f��\n", (double)(end - start) / CLOCKS_PER_SEC);
		cout << "\n���س�����......";
		getchar(); getchar();
		break;

	case 2:
		filename[14] = '0' + len;
		cout << "\n��ȴ�......\n\n";
		start = clock();
		getPassword(filename, targetmd, len);
		end = clock();
		printf("�ķ�ʱ��Ϊ%.3f��\n", (double)(end - start) / CLOCKS_PER_SEC);
		cout << "\n���س�����......";
		getchar(); getchar();
		break;

	default:;
	}
	system("CLS");
}

void getRainbowTable(char *filename, int len){
	pwd_len = len;
	RCount = RCountList[pwd_len - 3];
	RNum = RNumList[pwd_len - 3];

	vector<Line> buf;  //�洢ͷ����β���
	Line aNode;  
	char pwd[10];
	Bit8 md[MD5_DIGEST_LENGTH];
	srand(time(NULL));
	pwd[pwd_len] = '\0';
	for (unsigned long i = 0; i < RNum; i++){
		//������ɿ�����Ϊ��ͷ
		for (int k = 0; k < pwd_len; k++)
			pwd[k] = PWD_CHAR[rand() % 36];
		//������ͷ
		memcpy(aNode.begin, pwd, pwd_len + 1);
		//����HASH��
		for (int j = 0; j < RCount; j++){
			MD5((Bit8 *)pwd, pwd_len, md);
			Reduction(md, pwd, j);
		}
		//����β����
		memcpy(aNode.end, pwd, pwd_len + 1);
		buf.push_back(aNode);
	}
	sort(buf.begin(), buf.end(), cmp);

	FILE *fp = fopen(filename, "wb");
	for (unsigned long i = 0; i < RNum; i++){
		fprintf(fp, "%s %s\n", buf[i].begin, buf[i].end);
	}
	fclose(fp);
}

bool cmp(Line x, Line y){
	if (memcmp((x.end), (y.end), pwd_len) < 0)
		return true;
	else
		return false;
}

//R��������, md:ժҪֵ  pwd:��ȡ������  k:��ǰ�Ľڵ��
void Reduction(Bit8 *md, char *pwd, int k){
	Bit8 x, y;
	k = k%RCount;
	for (int i = 0; i<pwd_len; i++) {
		x = md[i] ^ md[(i + 1) & 0xf] ^ md[(i + 2) & 0xf] ^ md[(i + 3) & 0xf];
		y = md[k & 0xf] + md[(k - 1) & 0xf] + md[(k - 2) & 0xf] + md[(k - 3) & 0xf];
		pwd[i] = PWD_CHAR[(x + y) % 36];
		k = k + k + k + 1;
	}
}

int getPassword(char *filename, Bit8 *targetmd, int len){
	FILE *fp = fopen(filename, "rb");
	if (!fp){
		cout << "�����ڸ��ļ�, ��������ȷ���ļ���\n";
		cout << "\n���س�����......";
		return 0;
	}

	pwd_len = len;
	RCount = RCountList[pwd_len - 3];
	RNum = RNumList[pwd_len - 3];

	char pwd[10];
	Bit8 md[MD5_DIGEST_LENGTH];
	/*�оٳ�Ŀ�������HASH�������еĿ���λ�ã����������ڲ�ͬλ����ʱ
	  ���������β����¼������β����ʺ�����β���бȽ�*/
	vector<string> Rlist;
	for (int i = RCount - 1; i >= 0; i--){
		memcpy(md, targetmd, MD5_DIGEST_LENGTH);
		pwd[pwd_len] = '\0';
		for (int j = i; j < RCount - 1; j++){
			Reduction(md, pwd, j);
			MD5((Bit8 *)pwd, pwd_len, md);
		}
		Reduction(md, pwd, RCount - 1);
		Rlist.push_back((char *)pwd);
	}
	sort(Rlist.begin(), Rlist.end()); //���ַ�����������
	//���زʺ��
	vector<Line> buf;  
	Line aNode;
	char begin[10], end[10], tmp[10];
	while (fscanf(fp, "%s %s", begin, end) != EOF){
		memcpy(aNode.begin, begin, pwd_len + 1);
		memcpy(aNode.end, end, pwd_len + 1);
		buf.push_back(aNode);
	}
	fclose(fp);
	for (long i = 0; i < RCount; i++){
		long t = i;
		while (memcmp(Rlist[t].c_str(), Rlist[i].c_str(), pwd_len)==0 && i < RCount - 1)
			i++;
		long low = 0, high = RNum - 1;
		while (low <= high){
			long mid = (low + high) >> 1;
			if (mid >= RNum || mid < 0)
				break;
			if (memcmp(buf[mid].end, Rlist[t].c_str(), pwd_len) > 0)
				high = mid - 1;
			else if (memcmp(buf[mid].end, Rlist[t].c_str(), pwd_len) < 0)
				low = mid + 1;
			else{
				long s = mid;
				while (memcmp(buf[s].end, Rlist[t].c_str(), pwd_len) == 0 && s > 0)
					s--;
				if (mid > s) s++;
				while (memcmp(buf[s++].end, Rlist[t].c_str(), pwd_len) == 0 && s < RNum){
					memcpy(tmp, buf[s].begin, pwd_len);
					for (int k = 0; k < RCount; k++){
						MD5((Bit8 *)tmp, pwd_len, md);
						if (!memcmp(md, targetmd, MD5_DIGEST_LENGTH)){ //�ҵ�Ŀ��HASHֵ
							tmp[pwd_len] = '\0';
							printf("�ʺ�����ҵ��Ŀ���Ϊ:%s\n", tmp);
							cout << "��MD5��ϣֵΪ:";
							printKey(md, MD5_DIGEST_LENGTH);
							cout << endl;
							return 1;
						}
						Reduction(md, tmp, k);
					}
				}
				break;
			}
		}
	}
	printf("�ƽ�ʧ�ܣ��ʺ����δ�ҵ�Ŀ�����HASHֵ\n\n");
	fclose(fp);
	return 0;
}