工程目录为Cryptography，该目录下包括：

1. 随机性检测:  全0的文件AllZero.txt、
	       原始SPN加密文件AllZero_after_spn.txt、
	       增强SPN加密文件AllZero_after_spn_plus.txt、
	       原始SPN随机性检测结果Anwser_of_spn_randtest.txt、
	       增强SPN随机性检测结果Anwser_of_spn_plus_randtest.txt

2. ECC文件加密系统:  测试文件test.txt
		加密后的文件test_after_encrypt.txt
		解密后的文件test_after_decrypt.txt

3. 系统自带彩虹表：由于生成的彩虹表过大，系统自带彩虹表并未一起打包。
	               若要使用请先使用系统生成彩虹表，再命名为rainbow_table_N，其中N为所生成彩虹表的口令位数