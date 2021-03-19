#include "rsa.h"

//��ȡRSA�㷨��5������
void getRSAparam(mpz_t n, mpz_t p, mpz_t q, mpz_t e, mpz_t d){
	clock_t start, end;
	//��ȡǰ��������n,p,q
	do{
		getPrime(p);
		getPrime(q);
	} while (!mpz_cmp(p, q));  //p!=q

	mpz_mul(n, p, q);

	//��ȡd,e
	mpz_t p_1, q_1, n_1;   //�ֱ�����(p), ��(q), ��(n)
	mpz_inits(p_1, q_1, n_1,NULL);  //��ʼ��
	mpz_sub_ui(p_1, p, 1);  //��(p)=p-1
	mpz_sub_ui(q_1, q, 1);  //��(q)=q-1
	mpz_mul(n_1, p_1, q_1);  //��(n)=(p-1)(q-1)

	gmp_randstate_t state;
	gmp_randinit_default(state);//��state���г�ʼ��
	gmp_randseed_ui(state, time(NULL));//��state�ó�ʼ����

	while (1){
		mpz_urandomm(e, state, n_1);   //���������0=<b<=��(n), ���ɹ�Կ
		if (getModInverse(e, n_1, d))  //ʹ����eģ��(n)����d, ��Ϊ˽Կ
			break;
	}

	mpz_clears(n_1, p_1, q_1, NULL);
	gmp_randclear(state);
}

//��ȡһ������������СĬ��Ϊ512bit
void getPrime(mpz_t n){
	int i, random = 0;
	srand(time(NULL));
	char rand_num[PRIME_BIT + 1]; //���ڴ��512bit�������
	rand_num[PRIME_BIT] = '\0';   
	rand_num[0] = '1';  //��һ��bit����Ϊ1����֤���ɵ���512λ������
	rand_num[1] = '1';  //�ڶ���bit����Ϊ1����֤��˺���1024λ������
	rand_num[PRIME_BIT - 1] = '1';  //���һbit����Ϊ1����֤������
	while(1){	
		for (i = 2; i<PRIME_BIT -1; i++){	
			random = rand();
			rand_num[i] = '0' + (0x1 & random);
		}
		mpz_set_str(n, rand_num, 2);
		if (SetTestNum(n))
			break;
	} 
}

//�������Լ��Ĵ�����������MillerRabin�㷨���м��
bool SetTestNum(const mpz_t n){
	for (int i = 0; i<MILLER_RABIN_TEST_NUM; i++)
		if (!MillerRabin(n))
			return false;
	return true;
}

//MillerRabin�㷨�������Լ��
bool MillerRabin(const mpz_t n){
	gmp_randstate_t state;
	gmp_randinit_default(state);         //��state���г�ʼ��
	gmp_randseed_ui(state, time(NULL));  //��state�ó�ʼ����

	mpz_t m, a, b, flag, n_1;
	mpz_inits(m, a, b, flag, n_1, NULL);  //��ʼ��
	
	mpz_sub_ui(m, n, 1);      //m=n-1
	mpz_mod_ui(flag, m, 2);  //flag=m%2
	mpz_sub_ui(n_1, n, 1);    //n_1=n-1=-1mod n

	//����������̽���������, ��(n)=n-1=2^r*m, mΪ����
	int r = 0;
	//����r, ��n-1��ʾ��m*2^r
	while (!mpz_tstbit(m,1)){  //�������һλ��Ϊ1˵������������ֹѭ����Ϊ0��˵����ż��������ѭ��
		mpz_tdiv_q_2exp(m, m, 1);    //m����һλ, ��m=m/2
		r++;
	}
	//�������һ��[1,n-1]֮��������
	mpz_urandomm(a, state, n_1);   //���������a, 0<=a<=n-2
	mpz_add_ui(a, a, 1);           //a=a+1,��ʱ1<=a<=n-1
	PowerMod(a, m, n, b);          //�����b=a^m mod n
	if (!mpz_cmp_ui(b, 1)){    //��a^m=1, ��˵��ͨ������̽��, ֱ�ӷ���
		mpz_clears(m, flag, n_1, a, b,NULL);  //��������Ĵ����ռ�
		gmp_randclear(state);
		return  true;
	}
	//n-1��ʾ��m*2^r�����n��һ����������ô����a^m mod n=1��
	//���ߴ���ĳ��iʹ��a^(m*2^i) mod n=n-1 ( 0<=i<r ) 
	for (int i = 0; i < r; i++){   
		if (!mpz_cmp(b, n_1)){   //��b=n-1,˵�����϶���̽�⣬����true
			mpz_clears(m, flag, n_1, a, b, NULL);
			gmp_randclear(state);
			return  true;
		}
		else{
			mpz_mul(b, b, b);//b=b^2;
			mpz_mod(b, b, n);//b=b mod n;
		}
	}

	mpz_clears(m, flag, n_1, a, b, NULL);
	gmp_randclear(state);
	return false;
}

//ģ�ظ�ƽ����������a^b(mod n),���������ֵ��s
void PowerMod(const mpz_t a, const mpz_t b, const mpz_t n, mpz_t s){

	mpz_t t1, t2, t3;
	mpz_inits(t1, t2, t3, NULL);
	mpz_set_ui(t1, 1);  //t1=1;
	mpz_set(t2, a);    //t2=a;
	mpz_set(t3, b);    //t3=b;

	while (mpz_cmp_ui(t3,0)){

		if (mpz_tstbit(t3,0)){  //����t3�����Ƶ����һλ����Ϊ1��˵�� t3 mod 2=1
			//t1 = (t1*t2) mod n
			mpz_mul(t1, t1, t2); 
			mpz_mod(t1, t1, n);  
		}

		//t2 = (t2*t2) mod n
		mpz_mul(t2, t2, t2);
		mpz_mod(t2, t2, n);

		mpz_tdiv_q_2exp(t3, t3, 1); // ָ��t3����1λ,��t3=t3/2
	}

	mpz_set(s, t1); //�����Ľ��t1��ֵ��s
	mpz_clears(t1, t2, t3, NULL);

}

//ʹ����չŷ������㷨��e��ģn����Ԫd
bool getModInverse(const mpz_t e, const mpz_t n, mpz_t d){
	mpz_t a, b, c, c1, t, q, r;
	mpz_inits(a, b, c, c1, t, q, r, NULL);
	mpz_set(a, n);//a=n;
	mpz_set(b, e);//b=e;
	mpz_set_ui(c, 0);//c=0
	mpz_set_ui(c1, 1);//c1=1
	mpz_tdiv_qr(q, r, a, b); 
	while (mpz_cmp_ui(r, 0))//r==0��ֹѭ��
	{	
		mpz_mul(t, q, c1);//t=q*c1
		mpz_sub(t, c, t);//t=c-q*c1

		mpz_set(c, c1);   //c=c1  ����ƶ�
		mpz_set(c1, t);   //c1=t  ����ƶ�
		
		mpz_set(a, b);//a=b ������Ϊ������
		mpz_set(b, r);//b=r ������Ϊ��������ʼ��һ��
		mpz_tdiv_qr(q, r, a, b);  //ȡ��һ��q
	}
	mpz_set(d, t);  //�����һ�ֵ�t��ֵ��d, d����e��ģn����Ԫ

	//��֤����������
	mpz_add(d, d, n);
	mpz_mod(d, d, n);

	mpz_clears(a, c, t, q, r, NULL);

	if (mpz_cmp_ui(b, 1)){
		mpz_clear(b);
		return false;
	}
	else{
		mpz_clear(b);
		return true;
	}
}

//�й�ʣ�ඨ��
void ChineseRemainderTheorem(const mpz_t a, const  mpz_t b, const  mpz_t p, const mpz_t q, mpz_t s){
	mpz_t x, y, p_0, q_0, p_1, q_1, t1, t2, n;
	mpz_inits(x, y, p_0, q_0, p_1, q_1, t1, t2, n, NULL);

	mpz_sub_ui(p_0, p, 1);  //p_0=p-1��p_0����(p)
	mpz_sub_ui(q_0, q, 1);  //q_0=q-1��q_0����(q)
	mpz_mod(p_0, b, p_0);   //p_0=b mod p_0��  ��p_0=b mod ��(p)
	mpz_mod(q_0, b, q_0);   //q_0=b mod q_0��  ��q_0=b mod ��(q)

	//PowerMod(a, p_0, p, x);//x=a^b%p
	//PowerMod(a, q_0, q, y);//y=a^b%q

	mpz_powm(x, a, p_0, p);
	mpz_powm(y, a, q_0, q);


	getModInverse(p, q, p_1);//��pģq����
	getModInverse(q, p, q_1);//��qģp����

	//mpz_invert(q_1, p, q);//��pģq����
	//mpz_invert(p_1, q, p);//��pģq����
	//getModInverse(q, p, p_1);//��qģp����
	//gmp_printf("*********%Zd", q_1);
	//gmp_printf("*********%Zd", q_1);

	//s=(x*q*q_1 + y*p*p_1) mod n

	mpz_mul(t1, x, q);    //t1=x*q
	mpz_mul(t1, t1, q_1); //t1=x*q*q_1
	mpz_mul(t2, y, p);    //t2=y*p
	mpz_mul(t2, t2, p_1); //t2=y*p*p_1

	mpz_add(s, t1, t2);   //�й�ʣ�ඨ��(t1+t2)mod n
	mpz_mul(n, p, q);
	mpz_mod(s, s, n);

	mpz_clears(x, y, p_0, q_0, p_1, q_1, t1, t2, n, NULL);
}

//Montgomery�㷨������a^b(mod n),���������ֵ��s
void MontPowerMod(const mpz_t a, const mpz_t b, const mpz_t n, mpz_t s){
	mpz_t R, R1, Prod, A, IN, B, e;
	mpz_inits(R, R1, Prod, A, IN, B, e, NULL);

	mpz_set(e, b);

	//IN= -n^(-1) mod 2^32
	mpz_ui_pow_ui(B, 2, 32); //B=2^32
	getModInverse(n, B, IN); //IN=n^(-1) mod B
	mpz_sub(IN, B, IN);   //-n^(-1)= B-n^(-1) mod B

	//���ɱ�ģn�Դ�Ĳ���R=2^n_bit
	int n_bit = mpz_sizeinbase(n, 2);
	mpz_ui_pow_ui(R, 2, n_bit);

	//Prod=Mont(1)��A=Mont(a)
	mpz_mul_ui(Prod, R, 1);
	mpz_mod(Prod, Prod, n);
	mpz_mul(A, R, a);
	mpz_mod(A, A, n);

	while (mpz_cmp_ui(e, 0)){
		if (mpz_tstbit(e,0))
			MontMult(Prod, A, n, n_bit, IN);
		MontMult(A, A, n, n_bit, IN);
		mpz_tdiv_q_2exp(e, e, 1); //e=e>>1
	}

	//s=MontInv(Prod)=Prod*R(-1) mod n
	getModInverse(R, n, R1);
	mpz_mul(Prod, Prod, R1);
	mpz_mod(Prod, Prod, n);
	mpz_set(s, Prod);

	mpz_clears(R, R1, Prod, A, IN, B, e, NULL);
}

//�ɸ�����ģ�ˣ�A=(A*B)mod n
void MontMult(mpz_t A, mpz_t B, const mpz_t n, int n_bit, const mpz_t IN){
	mpz_t T, T1, t0, b_32;
	mpz_inits(T, T1, t0, b_32, NULL);
	
	mpz_mul(T, A, B);  //T=A*B
	mpz_set_ui(b_32, 0xFFFFFFFF);

	int t = n_bit >> 5;
	for(int i = 0; i < t; i++){
		mpz_and(t0, T, b_32);
		mpz_mul(t0, IN, t0); 
		mpz_and(t0, t0, b_32);

		//T1=T+n*t0
		mpz_mul(T1, n, t0);
		mpz_add(T1, T, T1);

		//T1>>32��T=T1
		mpz_tdiv_q_2exp(T1, T1, 32); 
		mpz_set(T, T1);

	}

	if (mpz_cmp(T1, n) > 0){  //T1>n��A=T1-n
		mpz_sub(T1, T1, n);
		mpz_set(A, T1);
	}
	else
		mpz_set(A, T1);
	mpz_clears(T, T1, t0, b_32, NULL);
}

//�Ƚ�ģ�ظ�ƽ�������й�ʣ�ඨ���Լ�Montgomery�㷨�Ķ�RSA�ļ���
void RSA_CMP(mpz_t n, mpz_t p, mpz_t q, mpz_t e, mpz_t d){
	int op = 1;
	mpz_t x, y, tmp;
	mpz_inits(x, y, tmp, NULL);
	clock_t start, end;
	printf("\n  �����뽫Ҫ�����ܵ�����(����������):\n  ");
	gmp_scanf("%Zd", x);

	while(1){
		system("CLS");
		printf("\n===============��ѡ�����ʵ��RSA���㷨================\n");
		printf("  1.ģ�ظ�ƽ��  2.�й�ʣ�ඨ��  3.Montgomery  0.�˳�\n");
		printf("=====================================================\n");
		printf("������[0-3]:");
		scanf("%d", &op);
		if (!op)
			break;
		switch (op){
			case 1:
				start = clock();
				PowerMod(x, e, n, y);  //ʹ�ù�Կe��������x���õ�����y
				end = clock();
				printf("\n����ʱ��Ϊ%.3f��\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("����Ϊ:\n  %Zd\n\n", y);

				start = clock();
				PowerMod(y, d, n, tmp);
				end = clock();
				printf("������ʱ��Ϊ%.3f��\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("���ܺ�����Ϊ:\n  %Zd\n", tmp);
				cout << "\n���س�����......";
				getchar(); getchar(); 
				break;
			case 2:
				start = clock();
				PowerMod(x, e, n, y);
				end = clock();
				printf("\n����ʱ��Ϊ%.3f��\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("����Ϊ:\n  %Zd\n\n", y);

				start = clock();
				ChineseRemainderTheorem(y, d, p, q,tmp);
				end = clock();
				printf("������ʱ��Ϊ%.3f��\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("���ܺ�����Ϊ:\n  %Zd\n", tmp);
				cout << "\n���س�����......";
				getchar(); getchar(); 
				break;
				
			case 3:
				start = clock();
				MontPowerMod(x, e, n, y);  //ʹ�ù�Կe��������x���õ�����y
				end = clock();
				printf("\n����ʱ��Ϊ%.3f��\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("����Ϊ:\n  %Zd\n\n", y);

				start = clock();
				MontPowerMod(y, d, n, tmp);
				end = clock();
				printf("������ʱ��Ϊ%.3f��\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("���ܺ�����Ϊ:\n  %Zd\n", tmp);
				cout << "\n���س�����......";
				getchar(); getchar();
				break;
			default:;
		}
	} 

	mpz_clears(x, y, tmp, NULL);
}

