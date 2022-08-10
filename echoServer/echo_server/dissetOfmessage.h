#pragma once
#include<stdbool.h>
#include<stdint.h>
//int dissect_fe_msg(char* buf, int offset);//状态维护
//int dissect_be_msg(char* buf, int offset);//状态维护
//int dissect_packet_fe_msg(char *buf,int len,char* retbuf,int typestr,char *puser,char *puser2,char *ppwd1,char *ppwd2,char *salt);//客户端解包
//const char* dissect_packet_be_msg(char* buf, char* retbuf, int typestr, int offset);//客户端解包

bool is_ssl_request(char* buf);
bool is_ssl_enable(char* buf);
bool is_startup(char* buf);

int fe_ssl_request_process(char* buf, char* retbuf);
int fe_startup_request_process(char* buf, char* retbuf, char
	* puser);
int fe_auth_process(char* buf,uint32_t len,char* retbuf,char* puser,char *puser2
,char* ppwd1,char *ppwd2,char* saltbuf);

int be_md5_salt_buf(char* buf, char* retbuf);
