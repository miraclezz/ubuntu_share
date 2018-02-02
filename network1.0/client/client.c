#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "msg.pb-c.h"
#include "cJSON.h"

#define HEAD_VERSION 1
#define MAX_NAME 20
#define MAX_MAJOR 20

/*学生信息*/
typedef struct Stu {
	int id;
	char name[MAX_NAME];
	char major[MAX_MAJOR];
}__attribute__((packed)) STU;

/*命令标志*/
enum command {
	COMMAND_SET = 0,
	COMMAND_GET
};

/*设置选项*/
enum command_set {
	SET_ID = 0,
	SET_NAME,
	SET_MAJOR,
	SET_COMPLETE
};

/*模块选项*/
enum mod{
	MODULE_TEST_PROTO = 0,
	MODULE_TEST_TLV,
	MODULE_TEST_JSON,
};

/*头部*/
typedef struct test_hdr_s {
	unsigned short ver; //头部版本暂定为1  收到后需校验
	unsigned short hdr_len; //头部长度	收到后需校验
	unsigned int dat_len;	//负载数据长度
	unsigned short module;	//当前业务模块
	unsigned short cmd;	//模块下的命令号
}test_hdr_t;

/*模块选择*/
int module_select(void)
{
	int module_value = -1;		//初始化module为一个无效的值
	printf("-------Please select module: （0、1、2）-------\n");
	printf("-------0:  MODULE_TEST_PROTO            -------\n");
	printf("-------1:  MODULE_TEST_TLV              -------\n");
	printf("-------2:  MODULE_TEST_JSON             -------\n");
	while (module_value > 2 || module_value < 0) {
		printf("select module: ");
		scanf("%d", &module_value);
		getchar();		//读走回车
	}
	
	return module_value;
}

/*命令选择*/
int command_select(void)
{
	int cmd_value = -1;
	printf("-------Please select command: （0、1）-------\n");
	printf("-------0:  COMMAND_SET                -------\n");
	printf("-------1:  COMMAND_GET                -------\n");
	while (cmd_value > 1 || cmd_value < 0) {
		printf("select command: ");
		scanf("%d", &cmd_value);
		getchar();		//读走回车
	}
	
	return cmd_value;
}

int deal_set_struct(STU *struct_stu)
{
	int set_value = -1;
	while(set_value != SET_COMPLETE)
	{
		printf("-------Please select set_option: （0、1、2、3）-------\n");
		printf("-------   0:  id                            -------\n");
		printf("-------   1:  name                          -------\n");
		printf("-------   2:  major                         -------\n");
		printf("-------   3:  complete                      -------\n");
		while (set_value > 3 || set_value < 0) {
			printf("select set_option: ");
			scanf("%d", &set_value);
			getchar();		//读走回车
		}
		
		switch (set_value) {
			case SET_ID :{
				set_value = -1;
				printf("set id: ");
				scanf("%d", &struct_stu->id);
				getchar();		//读走回车
				break;
			}
			case SET_NAME : {
				set_value = -1;
				printf("set name: ");
				scanf("%s", struct_stu->name);
				getchar();		//读走回车
				break;
			}
			case SET_MAJOR : {
				set_value = -1;
				printf("set major: ");
				scanf("%s", struct_stu->major);
				getchar();		//读走回车
				break;
			}					
		}
	}
	return 0;
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

int send_msg (int socket, void *out , int size)
{
	int cur_len = 0;
	int ret;
	while ((ret = send(socket, out, size - cur_len, 0)) != 0) {
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

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr,"Usage: %s <server_ip> <server_port>\n", 
				argv[0]);
		exit(EXIT_FAILURE);
	}
	
	/*创建套接字*/
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sockfd) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	printf("socket........................\n");
	
	/*向服务器发起连接请求*/
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_port		= htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	if (0 > connect(sockfd, (struct sockaddr*)&server_addr,
					sizeof(server_addr))) {
		perror("connect");
		exit(EXIT_FAILURE);		
	}
	printf("connect......................\n");
	
	
	/*使用的数据转换方式及命令选择*/
	int module_value = module_select();
	int cmd_value = command_select();
	

	
	/*学生信息数据*/
	STU struct_stu;
	memset(&struct_stu, 0, sizeof(struct_stu));

	
	/*头部数据*/
	test_hdr_t head = {
		.ver = HEAD_VERSION,
		.hdr_len = sizeof(head),
		.module = module_value,
		.cmd = cmd_value
	};
	
	
	
	
	
	
	
	/*判断需要执行什么命令*/
	switch (cmd_value) {
		case COMMAND_SET : {
			deal_set_struct(&struct_stu);			//接收用户想要设置的学生信息数据
			/*判断set命令传输那种数据类型*/
/***********************************************protobuf set********************************************************************/				
			switch (module_value) {			
				case MODULE_TEST_PROTO : {
					/*将负载信息序列化*/
					Student proto_stu	= STUDENT__INIT;
					proto_stu.id 		= struct_stu.id;
					proto_stu.name 		= struct_stu.name;
					proto_stu.major 	= struct_stu.major;
					
					head.dat_len = student__get_packed_size(&proto_stu);	//计算protobuf类型的数据大小
					
					int msg_len = head.hdr_len + head.dat_len;
					int head_len = head.hdr_len;
					
					/*将头部信息网络字节序化*/
					head_hton(&head);

					/*将头部信息和负载信息加载到一起*/
					void *buf;		//用于保存序列化的数据
					buf = malloc(msg_len);
					memcpy(buf, (void *)&head, head_len);
					student__pack(&proto_stu, buf + head_len);		//将序列化后的数据保存到buf中
					
					/*整个信息一起发送给服务器*/
					send(sockfd, buf, msg_len, 0); 
					
					free(buf);
					buf = NULL;
					
					break;
				}
/***********************************************protobuf set********************************************************************/	

/*----------------------------------------------json set    -------------------------------------------------------------------*/				
				case MODULE_TEST_JSON : {
					/*将负载信息序列化*/
					cJSON *student;
					char *out;
					student = cJSON_CreateObject();
					cJSON_AddItemToObject(student, "id", cJSON_CreateNumber(struct_stu.id));
					cJSON_AddItemToObject(student, "name", cJSON_CreateString(struct_stu.name));
					cJSON_AddItemToObject(student, "major", cJSON_CreateString(struct_stu.major));
					out =  cJSON_Print(student);		//将序列化的结果存储在out中
					cJSON_Delete(student);
					
					head.dat_len = strlen(out);
					
					printf("real head len : %d\n", head.hdr_len);
					printf("real data len : %d\n", head.dat_len);
					
					int msg_len = head.hdr_len + head.dat_len;
					int head_len = head.hdr_len;
					int data_len = head.dat_len;
					
					/*将头部信息网络字节序化*/
					head_hton(&head);
					
					send(sockfd, &head, head_len, 0);
					
					/*将头部信息和负载信息加载到一起*/
					void *buf;		//用于保存序列化的数据
					buf = malloc(msg_len);
					
					strncpy((char *)buf, (char *)&head, head_len);
					strncat((char *)buf, (char *)out, data_len);
					
					
					/*整个信息一起发送给服务器*/
					send(sockfd, buf, msg_len, 0); 
					printf("................................\n");
					
					free(out);
					break;
				}
/*----------------------------------------------json set    -------------------------------------------------------------------*/				
				case MODULE_TEST_TLV : {
					break;
				}
			}
			
			break;
		}
		case COMMAND_GET : {
			
			/*判断get命令传输哪种数据类型*/
			switch (module_value) {
/***********************************************protobuf get********************************************************************/				
				case MODULE_TEST_PROTO : {
					head.dat_len = 0;
					
					/*将头部信息网络字节序化*/
					head_hton(&head);
					
					/*信息发送给服务器*/
					send(sockfd, (void *)&head, sizeof(head), 0); 
					
					/*接收服务器发送的数据*/
					memset(&head, 0, sizeof(head));
					int ret = recv(sockfd, (void *)&head, sizeof(test_hdr_t), 0);
					if (ret < 0) {
						/*接收出错*/
						perror("recv");
						return -1;
					}
					else if (ret == 0) {
						/*服务器已经关闭*/
						printf("server close!\n");
						return -1;
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
							/*接收客户端发送的protobuf类型的数据反序列化后进行修改*/
							Student *proto_stu;
							char buf[head.dat_len];		//装接收的序列化数据
							int ret = recv_msg(sockfd, (void *)buf, head.dat_len);
							if (ret < 0) {
								/*接收出错*/
								perror("recv");
								return -1;
							}
							proto_stu = student__unpack(NULL, head.dat_len, buf);
							struct_stu.id    = proto_stu->id;
							strcpy(struct_stu.name,proto_stu->name);
							strcpy(struct_stu.major,proto_stu->major);
							
#if 1
							/*显示修改后的信息*/
							printf("id:      %d\n", struct_stu.id);
							printf("name:    %s\n", struct_stu.name);
							printf("major:   %s\n", struct_stu.major);
#endif
						}
					}
					break;
				}
/***********************************************protobuf get********************************************************************/				
				case MODULE_TEST_JSON : {
					break;
				}
				case MODULE_TEST_TLV : {
					break;
				}
			}

			break;
		}
	}
	
#if 0								
		/*显示接收的反序列化后的信息*/
		printf("id:      %d\n", struct_stu.id);
		printf("name:    %s\n", struct_stu.name);
		printf("major:   %s\n", struct_stu.major);
#endif	
	
	/*关闭套接字*/
	close(sockfd);
	
	return 0;
}