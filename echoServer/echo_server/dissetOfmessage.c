#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include<stdbool.h>
#include<assert.h>
#include <netinet/tcp.h>
#include "Md5.h"
#include "dissetOfmessage.h"
#include<stdint.h>


/*解包部分用宏封装*/
#define read_string(val,buf,lenth,offset)\
do{\
	int l_=strnlen(buf+offset,lenth-offset);\          
	if (offset + l_ >= lenth) {\	
		break; \
	}\
	memcpy(val, buf + offset, l_);\
	val[l_]=0;\
	offset += l_ + 1;\
 } while (0)

#define write_string(out_buf,out_len,val)\
do{\
	int l_=strlen(val);\
	memcpy(out_buf+out_len,val,l_);\
	out_len+=l_;\
	*(out_buf+out_len)=0;\
	out_len+=1;\
}while(0)


typedef enum {
	PGSQL_AUTH_STATE_NONE,               /*  No authentication seen or used */
	PGSQL_AUTH_SASL_REQUESTED,           /* Server sends SASL auth request with supported SASL mechanisms*/
	PGSQL_AUTH_SASL_CONTINUE,            /* Server and/or client send further SASL challange-response messages */
	PGSQL_AUTH_GSSAPI_SSPI_DATA,         /* GSSAPI/SSPI in use */
} pgsql_auth_state_t;

enum
{
	UNKNOWN=0,
	CANCEL_REQUEST=1,
	SSL_REQUEST=2,
	GSS_ENCRYPT_REQUEST=3,
	STARTUP_MESSAGE=4,
	SASLINITIALRESPONSE_MESSAGE=5,
	SASLRESPONSE_MESSAGE=6,
	GSSRESPOONSE_MESSAGE=7,
	PASSWORD_MESSAGE=8,
};

enum 
{
	PGSQL_AUTH_TYPE_SUCCESS=0,
	PGSQL_AUTH_TYPE_KERBEROS4=1,
	PGSQL_AUTH_TYPE_KERBEROS5=2,
	PGSQL_AUTH_TYPE_PLAINTEXT=3,
	PGSQL_AUTH_TYPE_CRYPT=4,
	PGSQL_AUTH_TYPE_MD5=5,
    PGSQL_AUTH_TYPE_SCM=6,
	PGSQL_AUTH_TYPE_GSSAPI=7,
	PGSQL_AUTH_TYPE_GSSAPI_SSPI_CONTINUE=8,
	PGSQL_AUTH_TYPE_SSPI=9,
	PGSQL_AUTH_TYPE_SASL=10,
	PGSQL_AUTH_TYPE_SASL_CONTINUE=11,
	PGSQL_AUTH_TYPE_SASL_COMPLETE=12,
};

typedef struct pgsql_conn_data {
	bool ssl_requested;
	pgsql_auth_state_t auth_state; /* Current authentication state */
} pgsql_conn_data_t;

typedef struct _value_string {
	uint32_t      value;
	const char* strptr;
} value_string;

/*client messages*/
static const value_string fe_messages[] = {
	{ 'p', "Authentication message" },
	{ 'Q', "Simple query" },
	{ 'P', "Parse" },
	{ 'B', "Bind" },
	{ 'E', "Execute" },
	{ 'D', "Describe" },
	{ 'C', "Close" },
	{ 'H', "Flush" },
	{ 'S', "Sync" },
	{ 'F', "Function call" },
	{ 'd', "Copy data" },
	{ 'c', "Copy completion" },
	{ 'f', "Copy failure" },
	{ 'X', "Termination" },
	{ 0, NULL }
};

/*server messages*/
static const value_string be_messages[] = {
	{ 'R', "Authentication request" },
	{ 'K', "Backend key data" },
	{ 'S', "Parameter status" },
	{ '1', "Parse completion" },
	{ '2', "Bind completion" },
	{ '3', "Close completion" },
	{ 'C', "Command completion" },
	{ 't', "Parameter description" },
	{ 'T', "Row description" },
	{ 'D', "Data row" },
	{ 'I', "Empty query" },
	{ 'n', "No data" },
	{ 'E', "Error" },
	{ 'N', "Notice" },
	{ 's', "Portal suspended" },
	{ 'Z', "Ready for query" },
	{ 'A', "Notification" },
	{ 'V', "Function call response" },
	{ 'G', "CopyIn response" },
	{ 'H', "CopyOut response" },
	{ 'd', "Copy data" },
	{ 'c', "Copy completion" },
	{ 'v', "Negotiate protocol version" },
	{ 0, NULL }
};

/*截取四个字节长度*/
static inline uint32_t pntoh32(const void* p)
{
	return (uint32_t) * ((const uint8_t*)(p)+0) << 24 |
		(uint32_t) * ((const uint8_t*)(p)+1) << 16 |
		(uint32_t) * ((const uint8_t*)(p)+2) << 8 |
		(uint32_t) * ((const uint8_t*)(p)+3) << 0;
}

/*截取两个字节长度*/
static inline uint16_t pntoh16(const void* p)
{
	return 
		(uint16_t) * ((const uint8_t*)(p)+0) << 8 |
		(uint16_t) * ((const uint8_t*)(p)+1) << 0;
}

//const char* try_val_to_str_idx(const uint32_t val, const value_string* vs, int* idx)
//{
//	int i = 0;
//	assert(idx != NULL);
//
//	if (vs) {
//		while (vs[i].strptr) {
//			if (vs[i].value == val) {
//				*idx = i;
//				return(vs[i].strptr);
//			}
//			i++;
//		}
//	}
//
//	*idx = -1;
//	return NULL;
//}
//
//const char* try_val_to_str(const uint32_t val, const value_string* vs)
//{
//	int ignore_me;
//	return try_val_to_str_idx(val, vs, &ignore_me);
//}
//
//const char* val_to_str_const(const uint32_t val, const value_string* vs,
//	const char* unknown_str)
//{
//	const char* ret;
//
//	assert(unknown_str != NULL);
//
//	ret = try_val_to_str(val, vs);
//	if (ret != NULL)
//		return ret;
//
//	return unknown_str;
//}

bool is_ssl_request(char *buf)
{			
	int typestr;
	unsigned int tag;
	uint32_t length;
	uint8_t type;
	uint32_t n = 0;
	type = buf[n + 0];

	length = pntoh32(buf + n);

	if (type == '\0')
	{
		tag = pntoh32(buf + n + 4);
		if (length == 8 && tag == 80877103)
			return true; 	
	}
	return false;
}

bool is_ssl_enable(char* buf)
{
	if (buf[0] == 'S')
		return true;
	else
		return false;
}

bool is_startup(char* buf)
{
	int typestr;
	unsigned int tag;
	uint8_t type;
	uint32_t n = 0;
	type = buf[n + 0];

	if (type == '\0')
	{
		tag = pntoh32(buf + n + 4);
		if (tag == 196608)
			return true;
	}
	return false;
}

int fe_ssl_request_process(char* buf,char* retbuf)
{
	int n = 0;
	uint32_t length = pntoh32(buf + n);
	n += 4;
	memcpy(retbuf, buf, 4);
	memcpy(retbuf + n, buf + n, 4);
	return length;
}

int fe_startup_request_process(char* buf, char* retbuf, char* puser)
{	
	char val[30];
	char lenbuf[4];
	memset(lenbuf, 0, sizeof(lenbuf));
	int n = 0;
	int offset = 0;
	/*length and version 32 bits*/
	uint32_t length = pntoh32(buf + n);	//length，最后计算	
	n += 4;
	uint16_t major_version = pntoh16(buf + n);
	n += 2;
	uint16_t minor_version = pntoh16(buf + n);
	n += 2;
	memcpy(retbuf + 4, buf + 4, 4);
	offset = n;
	while (1)//解包，组包
	{
		read_string(val, buf, length, n);

		if (val[0] == '\0')
		{
			write_string(retbuf, offset, val);
			break;
		}
		if (strcmp(val, "user") == 0)
		{
			write_string(retbuf, offset, val);
			//memset(val, 0, sizeof(val));
			read_string(val, buf, length, n);
			write_string(retbuf, offset, puser);
			length = length + strlen(puser) - strlen(val);
		}
		else if (strcmp(val, "database") == 0) {
			write_string(retbuf, offset, val);
			//memset(val, 0, sizeof(val));
			read_string(val, buf, length, n);
			write_string(retbuf, offset, val);
		}
		else
		{
			write_string(retbuf, offset, val);
			//memset(val, 0, sizeof(val));
			read_string(val, buf, length, n);
			write_string(retbuf, offset, val);
		}
	}
	lenbuf[3] = offset;
	memcpy(retbuf, lenbuf, 4);
	return offset;
}

int fe_auth_process(char* buf, uint32_t len, char* retbuf, char* puser, char* puser2
,char* ppwd1, char* ppwd2, char* saltbuf)
{
	int n = 0;
	n += 1;
	uint32_t length = pntoh32(buf + n);
	n += 4;

	char password1[36];//客户端发来的md5加密后的密码
	char password2[36];//代理解析加密后的密码
	char password3[36];//数据库需要的密码
	memset(password1, 0, sizeof(password1));
	memset(password2, 0, sizeof(password2));
	memset(password3, 0, sizeof(password3));

	memcpy(password1, buf + n, 36);

	encryptionMd5(password2, puser2, saltbuf, ppwd1);

	if (strcmp(password1, password2) == 0)
	{
		/*retbuf里的密码给换成数据库正确的密码*/
		encryptionMd5(password3, puser, saltbuf, ppwd2);
		memcpy(retbuf + n, password3, 36);
	}
	return len;
}

int be_md5_salt_buf(char* buf,char* retbuf)
{
	int n = 0;
	n += 1;
	uint32_t length = pntoh32(buf + n);
	n += 8;
	/*salt value*/
	char saltvalue[4];
	memset(saltvalue, 0, sizeof(saltvalue));
	strncpy(saltvalue, buf + n, 4);
	memcpy(retbuf, saltvalue, strlen(saltvalue));
	return retbuf;
}