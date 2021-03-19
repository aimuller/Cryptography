#include "spn.h"
//mode=0��ֻ���ܲ���ӡ���ܹ���    mode=1�����ܲ���ӡ���ܹ���    mode=2�����ܲ���ӡ���ܹ���
void spn(int mode, Bit16 &x, Bit32 Key){   
	if (mode == 0){

		//ǰ3�ֱ仯����P���û�
		for (int r = 1; r < 4; r++)
			one_round_spn(mode, r, x, Key); 

		//���һ�ֲ�����P���û�
		Bit16 rkey = round_key(4, Key); //��ȡ����Կ
		x ^= rkey;  
		s_box(1, x);

		rkey = round_key(5, Key);
		x ^= rkey;
	}
	else if (mode == 1){
		cout << "ԭʼ����Ϊ:\n" << "w0:";
		printBinary(x);
		for (int r = 1; r < 4; r++)
			one_round_spn(mode, r, x, Key);
		cout << "\n��4�ּ���\n";
		Bit16 rkey = round_key(4, Key);
		cout << "K4:" ;
		printBinary(rkey);

		x ^= rkey;
		cout << "u4:" ;
		printBinary(x);

		s_box(mode, x);
		cout << "v4:";
		printBinary(x);
		cout << "\n��5�ּ���\n";
		rkey = round_key(5, Key);
		cout << "K5:";
		printBinary(rkey);
		cout << "\n�õ�����Ϊ\n";
		x ^= rkey;
		cout << "y :";
		printBinary(x);
	}
	else if (mode == 2){
		cout << "\n��1�ֽ���\n";
		Bit16  rkey = round_key(5, Key);
		cout << "K5:";
		printBinary(rkey);
		x ^= rkey;
		cout << "v4:";
		printBinary(x);

		cout << "\n��2�ֽ���\n";
		s_box(mode, x);
		cout << "u4:";
		printBinary(x);
		rkey = round_key(4, Key);
		cout << "K4:";
		printBinary(rkey);
		x ^= rkey;
		cout << "w3:";
		printBinary(x);

		for (int r = 3; r > 0; r--)
			one_round_spn(mode, r, x, Key);
	}
}
void one_round_spn(int mode,int r, Bit16 &x,Bit32 Key){
	if (mode == 0){
		Bit16 rkey = round_key(r, Key);   //��ȡ����Կ
		x ^= rkey;                  //������Կ�׻�
		s_box(1, x);             //S�д���
		p_box(x);             //P���û�
	}
	if (mode == 1){
		cout << "\n��" << r << "�ּ���\n";
		Bit16 rkey = round_key(r, Key);
		cout << "K" << r << ":";
		printBinary(rkey);

		x ^= rkey;
		cout << "u" << r << ":";
		printBinary(x);

		s_box(mode, x);
		cout << "v" << r << ":";
		printBinary(x);

		p_box(x);
		cout << "w" << r << ":";
		printBinary(x);
	}
	if (mode == 2){
		cout << "\n��" << 6-r << "�ֽ���\n";

		p_box(x);
		cout << "v"<< r << ":";
		printBinary(x);

		s_box(mode, x);
		cout << "u" << r << ":";
		printBinary(x);

		Bit16 rkey = round_key(r, Key);
		cout << "K" << r << ":";
		printBinary(rkey);

		x ^= rkey;
		cout << "w" << r-1 << ":";
		printBinary(x);	
	}
}
//KeyΪ�����32bit��Կ, rΪ��ǰ����, r_keyΪ��r����Կ
Bit16 round_key(int r, Bit32 Key){ 
	Bit16 r_key = (Key << (4 * r - 4)) >> 16;
	return r_key;
}
//mode=1:ʹ��S�м���   �������:x  
void s_box(int mode, Bit16 &x){ 
	Bit16 xArray[4] = { 0, 0, 0, 0 }; //�洢4*4bit
	xArray[0] = x >> 12;  //ȡ�����4bit
	xArray[1] = (x & 0xF00) >> 8;  //ȡ���θ�4bit
	xArray[2] = (x & 0xF0) >> 4;  //ȡ���ε�4bit
	xArray[3] = x & 0xF;  //ȡ�����4bit
	Bit16 enSbox[16] = { 0xe, 0x4, 0xd, 0x1, 0x2, 0xf, 0xb, 0x8, 0x3, 0xa, 0x6, 0xc, 0x5, 0x9, 0x0, 0x7 };
	Bit16 deSbox[16] = { 0xe, 0x3, 0x4, 0x8, 0x1, 0xc, 0xa, 0xf, 0x7, 0xd, 0x9, 0x6, 0xb, 0x2, 0x0, 0x5 };
	if (mode == 1){  //ʹ��S�м���
		xArray[0] = enSbox[xArray[0]];
		xArray[1] = enSbox[xArray[1]];
		xArray[2] = enSbox[xArray[2]];
		xArray[3] = enSbox[xArray[3]];
		x = (xArray[0] << 12) | (xArray[1] << 8) | (xArray[2] << 4) | (xArray[3]);
	}
	else{    //ʹ��S�е������
		xArray[0] = deSbox[xArray[0]];
		xArray[1] = deSbox[xArray[1]];
		xArray[2] = deSbox[xArray[2]];
		xArray[3] = deSbox[xArray[3]];
		x = (xArray[0] << 12) | (xArray[1] << 8) | (xArray[2] << 4) | (xArray[3]);
	}
}

void p_box(Bit16 &x){
	Bit16 PBox[16] = { 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16 };
	Bit16 xArray[16],yArray[16];
	for (int i = 0; i < 16; i++){  //ȡ�����б���
		xArray[i] = (x << i);
		xArray[i] = xArray[i] >> 15;
	}

	for (int i = 0; i < 16; i++){ //����P���û�
		yArray[i] = xArray[PBox[i] - 1];
	}
	x = 0;
	for (int i = 0; i < 15; i++)  //���û��Ľ��ƴ������
		x = (x | yArray[i]) << 1;
	x = (x | yArray[15]);
}
void printBinary(Bit16 x){
	for (int i = 0; i < 16; i++){
		if (i % 4 == 0)
			cout << " ";
		Bit16 tmp = x << i;
		tmp = tmp >> 15;
		cout << tmp;
	}
	cout << "\n";
}