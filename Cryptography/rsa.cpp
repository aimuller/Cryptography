#include "rsa.h"

//获取RSA算法的5个参数
void getRSAparam(mpz_t n, mpz_t p, mpz_t q, mpz_t e, mpz_t d){
	clock_t start, end;
	//获取前三个参数n,p,q
	do{
		getPrime(p);
		getPrime(q);
	} while (!mpz_cmp(p, q));  //p!=q

	mpz_mul(n, p, q);

	//获取d,e
	mpz_t p_1, q_1, n_1;   //分别代表φ(p), φ(q), φ(n)
	mpz_inits(p_1, q_1, n_1,NULL);  //初始化
	mpz_sub_ui(p_1, p, 1);  //φ(p)=p-1
	mpz_sub_ui(q_1, q, 1);  //φ(q)=q-1
	mpz_mul(n_1, p_1, q_1);  //φ(n)=(p-1)(q-1)

	gmp_randstate_t state;
	gmp_randinit_default(state);//对state进行初始化
	gmp_randseed_ui(state, time(NULL));//对state置初始种子

	while (1){
		mpz_urandomm(e, state, n_1);   //产生随机数0=<b<=φ(n), 生成公钥
		if (getModInverse(e, n_1, d))  //使用求e模φ(n)的逆d, 作为私钥
			break;
	}

	mpz_clears(n_1, p_1, q_1, NULL);
	gmp_randclear(state);
}

//获取一个大整数，大小默认为512bit
void getPrime(mpz_t n){
	int i, random = 0;
	srand(time(NULL));
	char rand_num[PRIME_BIT + 1]; //用于存放512bit的随机数
	rand_num[PRIME_BIT] = '\0';   
	rand_num[0] = '1';  //第一个bit设置为1，保证生成的是512位的素数
	rand_num[1] = '1';  //第二个bit设置为1，保证相乘后是1024位的素数
	rand_num[PRIME_BIT - 1] = '1';  //最后一bit设置为1，保证是奇数
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

//设置素性检测的次数，并调用MillerRabin算法进行检测
bool SetTestNum(const mpz_t n){
	for (int i = 0; i<MILLER_RABIN_TEST_NUM; i++)
		if (!MillerRabin(n))
			return false;
	return true;
}

//MillerRabin算法进行素性检测
bool MillerRabin(const mpz_t n){
	gmp_randstate_t state;
	gmp_randinit_default(state);         //对state进行初始化
	gmp_randseed_ui(state, time(NULL));  //对state置初始种子

	mpz_t m, a, b, flag, n_1;
	mpz_inits(m, a, b, flag, n_1, NULL);  //初始化
	
	mpz_sub_ui(m, n, 1);      //m=n-1
	mpz_mod_ui(flag, m, 2);  //flag=m%2
	mpz_sub_ui(n_1, n, 1);    //n_1=n-1=-1mod n

	//下面计算二次探测的最大次数, φ(n)=n-1=2^r*m, m为奇数
	int r = 0;
	//计算r, 将n-1表示成m*2^r
	while (!mpz_tstbit(m,1)){  //测试最后一位，为1说明是奇数，终止循环。为0则说明是偶数，继续循环
		mpz_tdiv_q_2exp(m, m, 1);    //m右移一位, 即m=m/2
		r++;
	}
	//随机生成一个[1,n-1]之间的随机数
	mpz_urandomm(a, state, n_1);   //产生随机数a, 0<=a<=n-2
	mpz_add_ui(a, a, 1);           //a=a+1,此时1<=a<=n-1
	PowerMod(a, m, n, b);          //计算出b=a^m mod n
	if (!mpz_cmp_ui(b, 1)){    //若a^m=1, 则说明通过二次探测, 直接返回
		mpz_clears(m, flag, n_1, a, b,NULL);  //清理申请的大数空间
		gmp_randclear(state);
		return  true;
	}
	//n-1表示成m*2^r，如果n是一个素数，那么或者a^m mod n=1，
	//或者存在某个i使得a^(m*2^i) mod n=n-1 ( 0<=i<r ) 
	for (int i = 0; i < r; i++){   
		if (!mpz_cmp(b, n_1)){   //若b=n-1,说明符合二次探测，返回true
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

//模重复平方法，计算a^b(mod n),并将结果赋值给s
void PowerMod(const mpz_t a, const mpz_t b, const mpz_t n, mpz_t s){

	mpz_t t1, t2, t3;
	mpz_inits(t1, t2, t3, NULL);
	mpz_set_ui(t1, 1);  //t1=1;
	mpz_set(t2, a);    //t2=a;
	mpz_set(t3, b);    //t3=b;

	while (mpz_cmp_ui(t3,0)){

		if (mpz_tstbit(t3,0)){  //测试t3二进制的最后一位，若为1则说明 t3 mod 2=1
			//t1 = (t1*t2) mod n
			mpz_mul(t1, t1, t2); 
			mpz_mod(t1, t1, n);  
		}

		//t2 = (t2*t2) mod n
		mpz_mul(t2, t2, t2);
		mpz_mod(t2, t2, n);

		mpz_tdiv_q_2exp(t3, t3, 1); // 指数t3右移1位,即t3=t3/2
	}

	mpz_set(s, t1); //将最后的结果t1赋值给s
	mpz_clears(t1, t2, t3, NULL);

}

//使用拓展欧几里得算法求e的模n的逆元d
bool getModInverse(const mpz_t e, const mpz_t n, mpz_t d){
	mpz_t a, b, c, c1, t, q, r;
	mpz_inits(a, b, c, c1, t, q, r, NULL);
	mpz_set(a, n);//a=n;
	mpz_set(b, e);//b=e;
	mpz_set_ui(c, 0);//c=0
	mpz_set_ui(c1, 1);//c1=1
	mpz_tdiv_qr(q, r, a, b); 
	while (mpz_cmp_ui(r, 0))//r==0终止循环
	{	
		mpz_mul(t, q, c1);//t=q*c1
		mpz_sub(t, c, t);//t=c-q*c1

		mpz_set(c, c1);   //c=c1  向后移动
		mpz_set(c1, t);   //c1=t  向后移动
		
		mpz_set(a, b);//a=b 除数变为被除数
		mpz_set(b, r);//b=r 余数变为除数，开始下一轮
		mpz_tdiv_qr(q, r, a, b);  //取下一个q
	}
	mpz_set(d, t);  //将最后一轮的t赋值给d, d就是e的模n的逆元

	//保证返回正整数
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

//中国剩余定理
void ChineseRemainderTheorem(const mpz_t a, const  mpz_t b, const  mpz_t p, const mpz_t q, mpz_t s){
	mpz_t x, y, p_0, q_0, p_1, q_1, t1, t2, n;
	mpz_inits(x, y, p_0, q_0, p_1, q_1, t1, t2, n, NULL);

	mpz_sub_ui(p_0, p, 1);  //p_0=p-1，p_0即φ(p)
	mpz_sub_ui(q_0, q, 1);  //q_0=q-1，q_0即φ(q)
	mpz_mod(p_0, b, p_0);   //p_0=b mod p_0，  即p_0=b mod φ(p)
	mpz_mod(q_0, b, q_0);   //q_0=b mod q_0，  即q_0=b mod φ(q)

	//PowerMod(a, p_0, p, x);//x=a^b%p
	//PowerMod(a, q_0, q, y);//y=a^b%q

	mpz_powm(x, a, p_0, p);
	mpz_powm(y, a, q_0, q);


	getModInverse(p, q, p_1);//求p模q的逆
	getModInverse(q, p, q_1);//求q模p的逆

	//mpz_invert(q_1, p, q);//求p模q的逆
	//mpz_invert(p_1, q, p);//求p模q的逆
	//getModInverse(q, p, p_1);//求q模p的逆
	//gmp_printf("*********%Zd", q_1);
	//gmp_printf("*********%Zd", q_1);

	//s=(x*q*q_1 + y*p*p_1) mod n

	mpz_mul(t1, x, q);    //t1=x*q
	mpz_mul(t1, t1, q_1); //t1=x*q*q_1
	mpz_mul(t2, y, p);    //t2=y*p
	mpz_mul(t2, t2, p_1); //t2=y*p*p_1

	mpz_add(s, t1, t2);   //中国剩余定理：(t1+t2)mod n
	mpz_mul(n, p, q);
	mpz_mod(s, s, n);

	mpz_clears(x, y, p_0, q_0, p_1, q_1, t1, t2, n, NULL);
}

//Montgomery算法，计算a^b(mod n),并将结果赋值给s
void MontPowerMod(const mpz_t a, const mpz_t b, const mpz_t n, mpz_t s){
	mpz_t R, R1, Prod, A, IN, B, e;
	mpz_inits(R, R1, Prod, A, IN, B, e, NULL);

	mpz_set(e, b);

	//IN= -n^(-1) mod 2^32
	mpz_ui_pow_ui(B, 2, 32); //B=2^32
	getModInverse(n, B, IN); //IN=n^(-1) mod B
	mpz_sub(IN, B, IN);   //-n^(-1)= B-n^(-1) mod B

	//生成比模n略大的参数R=2^n_bit
	int n_bit = mpz_sizeinbase(n, 2);
	mpz_ui_pow_ui(R, 2, n_bit);

	//Prod=Mont(1)，A=Mont(a)
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

//蒙哥马利模乘，A=(A*B)mod n
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

		//T1>>32，T=T1
		mpz_tdiv_q_2exp(T1, T1, 32); 
		mpz_set(T, T1);

	}

	if (mpz_cmp(T1, n) > 0){  //T1>n，A=T1-n
		mpz_sub(T1, T1, n);
		mpz_set(A, T1);
	}
	else
		mpz_set(A, T1);
	mpz_clears(T, T1, t0, b_32, NULL);
}

//比较模重复平方法、中国剩余定理以及Montgomery算法的对RSA的加速
void RSA_CMP(mpz_t n, mpz_t p, mpz_t q, mpz_t e, mpz_t d){
	int op = 1;
	mpz_t x, y, tmp;
	mpz_inits(x, y, tmp, NULL);
	clock_t start, end;
	printf("\n  请输入将要被加密的数据(请输入整数):\n  ");
	gmp_scanf("%Zd", x);

	while(1){
		system("CLS");
		printf("\n===============请选择快速实现RSA的算法================\n");
		printf("  1.模重复平方  2.中国剩余定理  3.Montgomery  0.退出\n");
		printf("=====================================================\n");
		printf("请输入[0-3]:");
		scanf("%d", &op);
		if (!op)
			break;
		switch (op){
			case 1:
				start = clock();
				PowerMod(x, e, n, y);  //使用公钥e加密明文x，得到密文y
				end = clock();
				printf("\n加密时间为%.3f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("密文为:\n  %Zd\n\n", y);

				start = clock();
				PowerMod(y, d, n, tmp);
				end = clock();
				printf("解密密时间为%.3f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("解密后，明文为:\n  %Zd\n", tmp);
				cout << "\n按回车继续......";
				getchar(); getchar(); 
				break;
			case 2:
				start = clock();
				PowerMod(x, e, n, y);
				end = clock();
				printf("\n加密时间为%.3f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("密文为:\n  %Zd\n\n", y);

				start = clock();
				ChineseRemainderTheorem(y, d, p, q,tmp);
				end = clock();
				printf("解密密时间为%.3f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("解密后，明文为:\n  %Zd\n", tmp);
				cout << "\n按回车继续......";
				getchar(); getchar(); 
				break;
				
			case 3:
				start = clock();
				MontPowerMod(x, e, n, y);  //使用公钥e加密明文x，得到密文y
				end = clock();
				printf("\n加密时间为%.3f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("密文为:\n  %Zd\n\n", y);

				start = clock();
				MontPowerMod(y, d, n, tmp);
				end = clock();
				printf("解密密时间为%.3f秒\n", (double)(end - start) / CLOCKS_PER_SEC);
				gmp_printf("解密后，明文为:\n  %Zd\n", tmp);
				cout << "\n按回车继续......";
				getchar(); getchar();
				break;
			default:;
		}
	} 

	mpz_clears(x, y, tmp, NULL);
}

