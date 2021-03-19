#include "rainbow_table.h"
#include <stdint.h>  
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
//3位  RCount:2000  10000              
//4位  RCount:2000  200000     约1min
//5位  RCount:2000  3000000    约1min  10中8
//6位  RCount:3000  10000000   约1min  10中5
int RCountList[4] = { 2000, 5000, 5000, 8000 };
long RNumList[4] = { 10000, 200000, 2000000, 10000000 };
int pwd_len; //口令长度
int RCount;  //链长
long RNum;   //链数
Bit8 PWD_CHAR[] = "0123456789abcdefghijklmnopqrstuvwxyz";

void RainbowTable(int len,Bit8 *targetmd){
	//clock_t start, end;
	//char filename[100];
	//strcpy(filename, "rainbow_table_3.txt");
	//filename[14] = '0' + len;
	//start = clock();
	//getPassword(filename, targetmd, len);
	//end = clock();
	//printf("耗费时间为%.3f秒\n\n\n", (double)(end - start) / CLOCKS_PER_SEC);
	int op = 1; 
	char filename[100];
	clock_t start, end;
	strcpy(filename, "rainbow_table_3.txt");
	printf("\n=======请选择彩虹表========\n");
	printf("  1.手动输入  2.系统自带 \n");
	printf("===========================\n");
	printf("请输入[1-2]:");
	scanf("%d", &op);

	switch (op){
	case 1:
		cout << "\n请输入文件名:";
		cin >> filename;
		cout << "\n请等待......\n\n";
		start = clock();
		getPassword(filename, targetmd, len);
		end = clock();
		printf("耗费时间为%.3f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
		cout << "\n按回车继续......";
		getchar(); getchar();
		break;

	case 2:
		filename[14] = '0' + len;
		cout << "\n请等待......\n\n";
		start = clock();
		getPassword(filename, targetmd, len);
		end = clock();
		printf("耗费时间为%.3f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
		cout << "\n按回车继续......";
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

	vector<Line> buf;  //存储头结点和尾结点
	Line aNode;  
	char pwd[10];
	Bit8 md[MD5_DIGEST_LENGTH];
	srand(time(NULL));
	pwd[pwd_len] = '\0';
	for (unsigned long i = 0; i < RNum; i++){
		//随机生成口令作为链头
		for (int k = 0; k < pwd_len; k++)
			pwd[k] = PWD_CHAR[rand() % 36];
		//保存链头
		memcpy(aNode.begin, pwd, pwd_len + 1);
		//生成HASH链
		for (int j = 0; j < RCount; j++){
			MD5((Bit8 *)pwd, pwd_len, md);
			Reduction(md, pwd, j);
		}
		//将链尾保存
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

//R函数定义, md:摘要值  pwd:获取的明文  k:当前的节点号
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
		cout << "不存在该文件, 请输入正确的文件名\n";
		cout << "\n按回车继续......";
		return 0;
	}

	pwd_len = len;
	RCount = RCountList[pwd_len - 3];
	RNum = RNumList[pwd_len - 3];

	char pwd[10];
	Bit8 md[MD5_DIGEST_LENGTH];
	/*列举出目标口令在HASH链中所有的可能位置，并计算其在不同位置上时
	  会产生的链尾，记录所有链尾，与彩虹表的链尾进行比较*/
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
	sort(Rlist.begin(), Rlist.end()); //对字符串进行排序
	//加载彩虹表
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
						if (!memcmp(md, targetmd, MD5_DIGEST_LENGTH)){ //找到目标HASH值
							tmp[pwd_len] = '\0';
							printf("彩虹表中找到的口令为:%s\n", tmp);
							cout << "其MD5哈希值为:";
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
	printf("破解失败！彩虹表中未找到目标口令HASH值\n\n");
	fclose(fp);
	return 0;
}