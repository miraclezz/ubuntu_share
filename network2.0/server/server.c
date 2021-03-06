#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "msg.pb-c.h"
#include "cJSON.h"
#include "tlv.h"

#define HEAD_VERSION 1
#define MAX_NAME 20
#define MAX_MAJOR 20
#define MAX_TLV_LEN 1000

/*学生信息*/
typedef struct Stu {
	int id;
	char name[MAX_NAME];
	char major[MAX_MAJOR];
}STU;

/*命令标志*/
enum command {
	COMMAND_SET = 0,
	COMMAND_GET
};

/*模块选项*/
enum mod {
	MODULE_TEST_PROTO = 0,
	MODULE_TEST_TLV,
	MODULE_TEST_JSON
};

/*学生信息的TLV描述信息*/
struct tlv_arg_desc tlv_student[] = {
	{TLV_ID_TYPE, DATA_TYPE_U32, member_info(struct Stu, id)},
	{TLV_NAME_TYPE, DATA_TYPE_STRING, member_info(struct Stu, name)},
	{TLV_MAJOR_TYPE, DATA_TYPE_STRING, member_info(struct Stu, major)},
	{TLV_MSG_MAX, 0, 0, 0}
};

/*头部*/
typedef struct test_hdr_s {
	unsigned short ver; //头部版本暂定为1  收到后需校验
	unsigned short hdr_len; //头部长度	收到后需校验
	unsigned int dat_len;	//负载数据长度
	unsigned short module;	//当前业务模块
	unsigned short cmd;	//模块下的命令号
}test_hdr_t;

int recv_msg (int socket, void *out , int size)
{
	int cur_len = 0;
	int ret;
	while ((ret = recv(socket, out, size - cur_len, 0)) != 0) {
		cur_len += ret;
		if(ret < 0) {
			return -1;
		}
		if (size == cur_len) {
			return cur_len;
		}
	}
	return cur_len;
}

/*将头部的网络字节序转换回主机字节序*/
int head_ntoh(test_hdr_t *head)
{
	head->ver 		= ntohs(head->ver);
	head->hdr_len	= ntohs(head->hdr_len);
	head->dat_len	= ntohl(head->dat_len);
	head->module	= ntohs(head->module);
	head->cmd		= ntohs(head->cmd);	
	return 0;
}

/*头部数据转换成网络序*/
int head_hton(test_hdr_t *head)
{
	head->ver 		= htons(head->ver);
	head->hdr_len 	= htons(head->hdr_len);
	head->dat_len 	= htonl(head->dat_len);
	head->module	= htons(head->module);
	head->cmd 		= htons(head->cmd);	
	return 0;
}

/*显示学生信息*/
int show_msg(STU struct_stu)
{
	printf("\n\n-----------------------------\n");
	printf("id:      %d\n", struct_stu.id);
	printf("name:    %s\n", struct_stu.name);
	printf("major:   %s\n", struct_stu.major);
	printf("-----------------------------\n\n\n");
	return 0;
}

int deal_protobuf_set(int connfd,int len, STU *struct_stu)
{
	/*接收客户端发送的protobuf类型的数据反序列化后进行修改*/
	Student *proto_stu;
	char buf[len];		//装接收的序列化数据
	int ret = recv_msg(connfd, (void *)buf, len);
	if (ret < 0) {
		/*接收出错*/
		perror("recv");
		return -1;
	}
	
	proto_stu = student__unpack(NULL, len, buf);
	if (0 != proto_stu->id) {
		struct_stu->id    = proto_stu->id;
	}
	if (0 != (strcmp(proto_stu->name, ""))) {
		strcpy(struct_stu->name,proto_stu->name);
	}
	if (0 != (strcmp(proto_stu->major, ""))) {
		strcpy(struct_stu->major,proto_stu->major);
	}
	return 0;
}

int deal_json_set(int connfd,int len, STU *struct_stu)
{
	/*接收客户端发送的json类型的数据反序列化后进行修改*/
	char out[len];		//装接收的序列化数据
	int ret = recv_msg(connfd, (void *)out, len);
	if (ret < 0) {
		/*接收出错*/
		perror("recv");
		return -1;
	}
	cJSON *student = cJSON_Parse(out);
	cJSON *c_id 	= cJSON_GetObjectItem(student, "id");											
	cJSON *c_name 	= cJSON_GetObjectItem(student, "name");
	cJSON *c_major 	= cJSON_GetObjectItem(student, "major");

	if (0 != c_id->valueint) {
		struct_stu->id    = c_id->valueint;
	}
	if (0 != (strcmp(c_name->valuestring, ""))) {
		strcpy(struct_stu->name,c_name->valuestring);
	}
	if (0 != (strcmp(c_major->valuestring, ""))) {
		strcpy(struct_stu->major,c_major->valuestring);
	}
	return 0;
}

int deal_tlv_set(int connfd,int len, STU *struct_stu)
{
	/*接收客户端发送的tlv类型的数据结构体化后进行修改*/
	char out[len];		//装接收的字符化数据
	int ret = recv_msg(connfd, (void *)out, len);
	if (ret < 0) {
		/*接收出错*/
		perror("recv");
		return -1;
	}
	STU *struct_stu_tmp = malloc(sizeof(STU));
	tlv2struct(struct_stu_tmp, out, len, tlv_student);
	if (0 != struct_stu_tmp->id) {
		struct_stu->id    = struct_stu_tmp->id;
	}
	if (0 != (strcmp(struct_stu_tmp->name, ""))) {
		strcpy(struct_stu->name,struct_stu_tmp->name);
	}
	if (0 != (strcmp(struct_stu_tmp->major, ""))) {
		strcpy(struct_stu->major,struct_stu_tmp->major);
	}
	free (struct_stu_tmp);
	struct_stu_tmp = NULL;
	return 0;
}

int deal_protobuf_get(int connfd, test_hdr_t *head, STU *struct_stu)
{
	/*将信息序列化*/
	Student proto_stu	= STUDENT__INIT;
	proto_stu.id 		= struct_stu->id;
	proto_stu.name 		= struct_stu->name;
	proto_stu.major 	= struct_stu->major;
	
	
	head->dat_len = student__get_packed_size(&proto_stu);	//计算protobuf类型的数据大小

	int msg_len = head->hdr_len + head->dat_len;
	int head_len = head->hdr_len;
	
	/*将头部信息网络字节序化*/
	head_hton(head);

	/*将头部信息和负载信息加载到一起*/
	void *buf;		//用于保存序列化的数据
	buf = malloc(msg_len);
	memcpy(buf, (void *)head, head_len);
	student__pack(&proto_stu, buf + head_len);		//将序列化后的数据保存到buf中
	
	/*整个信息一起发送给客户端*/
	send(connfd, buf, msg_len, 0); 
	free(buf);
	buf = NULL;
	
	return 0;
}

int deal_json_get(int connfd,test_hdr_t *head, STU *struct_stu)
{
	/*将负载信息字符串化*/
	cJSON *student;
	char *out;
	student = cJSON_CreateObject();
	cJSON_AddItemToObject(student, "id",    cJSON_CreateNumber(struct_stu->id));
	cJSON_AddItemToObject(student, "name",  cJSON_CreateString(struct_stu->name));
	cJSON_AddItemToObject(student, "major", cJSON_CreateString(struct_stu->major));
	out =  cJSON_Print(student);		//将序列化的结果存储在out中
	cJSON_Delete(student);
	
	head->dat_len = strlen(out);
	
	int msg_len  = head->hdr_len + head->dat_len;
	int head_len = head->hdr_len;
	int data_len = head->dat_len;
	
	/*将头部信息网络字节序化*/
	head_hton(head);
	
	/*将头部信息和负载信息加载到一起*/
	void *buf;		//用于保存编码后的数据
	buf = malloc(msg_len);
	memset(buf, 0, msg_len);
	memcpy(buf, (void *)head, head_len);
	memcpy(buf+head_len, (void *)out, data_len);											
	
	/*整个信息一起发送给客户端*/
	send(connfd, buf, msg_len, 0); 

	free(out);
	
	return 0;
}

int deal_tlv_get(int connfd, test_hdr_t *head, STU *struct_stu)
{
	/*将负载信息tlv化*/
	char out[MAX_TLV_LEN] = {0};

	head->dat_len = struct2tlv(out, struct_stu, tlv_student);				
	
	int msg_len  = head->hdr_len + head->dat_len;
	int head_len = head->hdr_len;
	int data_len = head->dat_len;
	
	/*将头部信息网络字节序化*/
	head_hton(head);
	
	/*将头部信息和负载信息加载到一起*/
	void *buf;		//用于保存编码后的数据
	buf = malloc(msg_len);
	memset(buf, 0, msg_len);
	memcpy(buf, (void *)head, head_len);
	memcpy(buf+head_len, (void *)out, data_len);											
	
	/*整个信息一起发送给客户端*/
	send(connfd, buf, msg_len, 0); 
	
	free(buf);
	buf = NULL;
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <ip> <prot>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	/*准备好结构体数据*/
	STU struct_stu = {
		.id = 2014,
		.name = "Frank",
		.major = "math"
	};
	
	/*创建套接字*/
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sockfd) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	printf("socket...................\n");
	
	/*绑定本机地址和端口*/
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_port		= htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	if (0 > bind(sockfd, (struct sockaddr*)&server_addr, 
				sizeof(server_addr))) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	printf("bind....................\n");
	
	/*设置监听套接字*/
	if(0 > listen(sockfd, 500)) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("listen.................\n");

	fd_set fds, read_fds;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	int maxfd = sockfd;
	
	/*I/O多路复用*/	
	int retval;
	while (1) {
		read_fds = fds;
		retval = select(maxfd+1, &read_fds, NULL, NULL, NULL);
		if (0 > retval) {
			perror("select");
			break;
		}
		int i;
		int ret;
		for (i=0; i<maxfd+1; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == sockfd) {
					/*识别到客户端发来的连接请求，接收连接请求*/
					struct sockaddr_in client_addr;
					memset(&client_addr, 0, sizeof(client_addr));
					int client_addr_len = sizeof(client_addr);
					int connfd = accept(sockfd, 
										(struct sockaddr*)&client_addr, 
										&client_addr_len);
					if (0 > connfd) {
						perror("accept");
						exit(EXIT_FAILURE);
					}
					printf("accept: %s\n", inet_ntoa(client_addr.sin_addr));
					FD_SET(connfd, &fds);
					maxfd = maxfd>connfd?maxfd:connfd;
				}
				else {
					/*收发数据*/
					
					/*接收头部数据*/
					test_hdr_t head;
					int ret = recv_msg(i, (void *)&head, sizeof(test_hdr_t));
					if (ret < 0) {
						/*接收出错*/
						perror("recv");
						continue;
					}
					else if (ret == 0) {
						/*客户端已经关闭*/
						printf("client close!\n\n");
						FD_CLR(i, &fds);
						close(i);
					}
					else {
						/*将头部的网络字节序转换回主机字节序*/
						head_ntoh(&head);
#if 0
						/*校验*/
						printf("ver: %d\n", head.ver);
						printf("real head len: %d\n", sizeof(head));
						printf("recv head len: %d\n", head.hdr_len);
#endif
						/*校验通过才会向下执行*/
						if (HEAD_VERSION == head.ver && 
							sizeof(head) == head.hdr_len) {
							
							/*解析要执行哪种命令*/
							switch (head.cmd) {
								case COMMAND_SET: {
									/*判断set命令客户端想要使用哪种数据类型*/
									switch (head.module) {
/***********************************************protobuf set********************************************************************/
										case MODULE_TEST_PROTO: {
											deal_protobuf_set(i,head.dat_len, &struct_stu);
											show_msg(struct_stu);
											break;
										}											
										
/*----------------------------------------------json set    -------------------------------------------------------------------*/											
										case MODULE_TEST_JSON : {
											deal_json_set(i,head.dat_len, &struct_stu);											
											show_msg(struct_stu);									
											break;
										}

/*==============================================tlv set     ===================================================================*/										
										case MODULE_TEST_TLV : {
											deal_tlv_set(i, head.dat_len, &struct_stu);
											show_msg(struct_stu);
											break;
										}
									}									
										
									break;
								}
								case COMMAND_GET: {
									/*判断get命令客户端想要使用哪种数据类型*/
									switch (head.module) {
/***********************************************protobuf get********************************************************************/										
										case MODULE_TEST_PROTO: {
											deal_protobuf_get(i, &head, &struct_stu);
											break;
										}

/*----------------------------------------------json get    -------------------------------------------------------------------*/										
										case MODULE_TEST_JSON : {
											deal_json_get(i,&head, &struct_stu);
											break;
										}

/*==============================================tlv get     ===================================================================*/	
										case MODULE_TEST_TLV : {
											deal_tlv_get(i,&head, &struct_stu);
											break;
										}								
									}									
										
									break;
								}
								
							}												
							
						}	
					}
				}	
			}
		}	
	}

	
	/*关闭套接字*/
	close(sockfd);
	
	return 0;
}