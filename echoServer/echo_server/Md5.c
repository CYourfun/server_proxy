#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <openssl/md5.h>
//#include <cstdint>
#include<stdint.h>


void bytesToHex(uint8_t b[16], char* s)
{
	static const char* hex = "0123456789abcdef";
	int q,w;
	for (q = 0, w = 0; q < 16; q++)
	{
		s[w++] = hex[(b[q] >> 4) & 0x0F];
		s[w++] = hex[b[q] & 0x0F];
	}
	s[w] = '\0';
}


const char* encryptionMd5(const char* pwdbuf, const char* username
 ,const char* salt,char *pwd)
{
	size_t passwd_len = strlen(pwd);
	size_t salt_len1 = strlen(username);
	size_t salt_len2 = strlen(salt);

	char sum[16];
	char sum2[32];
	char sum3[16];

	memset(sum,0,sizeof(sum));
	memset(sum2,0,sizeof(sum2));
	memset(sum3, 0, sizeof(sum3));

	strcpy(pwdbuf, "md5");

	char crypt_buf[64];

	memset(crypt_buf,0,sizeof(crypt_buf));
	memcpy(crypt_buf, pwd, passwd_len);
	memcpy(crypt_buf + passwd_len, username, salt_len1);

	MD5_CTX ctx;	
    MD5_Init(&ctx);
	MD5_Update(&ctx,crypt_buf,passwd_len+salt_len1);
	MD5_Final(sum, &ctx);

	bytesToHex(sum, sum2);//++

	memset(crypt_buf, 0, sizeof(crypt_buf));
	//memcpy(crypt_buf,sum,16);
	memcpy(crypt_buf, sum2, 32);//++
	memcpy(crypt_buf+32,salt,4);
	
	MD5_Init(&ctx);
	MD5_Update(&ctx, crypt_buf, strlen(crypt_buf));
	MD5_Final(sum3, &ctx);

	bytesToHex(sum3, pwdbuf + 3);






}