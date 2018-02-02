#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "msg.pb-c.h"

#define HEAD_VERSION 1
#define MAX_NAME 20
#define MAX_MAJOR 20

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
	
	int module_value = module_select();
	int cmd_value = command_select();
	

	
	/*数据收发*/
	STU struct_stu;
	memset(&struct_stu, 0, sizeof(struct_stu));
	
	switch (cmd_value) {
		case COMMAND_SET : {
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
						scanf("%d", &struct_stu.id);
						getchar();		//读走回车
						break;
					}
					case SET_NAME : {
						set_value = -1;
						printf("set name: ");
						scanf("%s", struct_stu.name);
						getchar();		//读走回车
						break;
					}
					case SET_MAJOR : {
						set_value = -1;
						printf("set major: ");
						scanf("%s", struct_stu.major);
						getchar();		//读走回车
						break;
					}					
				}
			}
			break;
		}
		case COMMAND_GET : {
			printf("%d\n", COMMAND_GET);
			break;
		}
	}
	
	/*头部数据*/
	test_hdr_t head = {
		.ver = HEAD_VERSION,
		.hdr_len = sizeof(head),
		.cmd = cmd_value
	};
	void *buf;		//用于保存序列化的数据
	
	
	switch (module_value) {
		case MODULE_TEST_PROTO : {
			if(cmd_value == COMMAND_SET) {
				head.module = MODULE_TEST_PROTO;	//将选择的模块信息在头部中标记
				
				Student proto_stu = STUDENT__INIT;
				
				proto_stu.id = struct_stu.id;
				proto_stu.name = struct_stu.name;
				proto_stu.major = struct_stu.major;
				head.dat_len = student__get_packed_size(&proto_stu);	//将序列化后的长度存到头部中
				
				buf = malloc(head.dat_len);
				student__pack(&proto_stu, buf);
			}
			else {
				/*解析为客户端要获取数据*/
				Student *proto_stu;
				head.dat_len = student__get_packed_size(proto_stu);	//将序列化后的长度存到头部中
				
			}
			break;
		}
		case MODULE_TEST_TLV :
			//deal_tlv();
			printf("%d\n", MODULE_TEST_TLV);
			break;
		case MODULE_TEST_JSON :
			//deal_json();
			break;
	}
	
	
	/*头部数据转换成网络序*/
	head.ver = htons(head.ver);
	head.hdr_len = htons(head.hdr_len);
	head.dat_len = htonl(head.dat_len);
	head.module = htons(head.module);
	head.cmd = htons(head.cmd);
	
	send(sockfd, (void *)&head, sizeof(head), 0);
	if (cmd_value == COMMAND_SET) {
		send(sockfd, buf, head.dat_len, 0);
	}
	else if (cmd_value == COMMAND_GET) {
		Student *proto_stu;
		char buf[head.dat_len];		//装接收的序列化数据
		int ret = recv(sockfd, (void *)buf, 
						head.dat_len, 0);
		if (ret < 0) {
			/*接收出错*/
			perror("recv");
		}
		
		proto_stu = student__unpack(NULL, head.dat_len, buf);

		struct_stu.id    = proto_stu->id;
		strcpy(struct_stu.name,proto_stu->name);
		strcpy(struct_stu.major,proto_stu->major);
	}
#if 1								
		/*显示接收的反序列化后的信息*/
		printf("id:      %d\n", struct_stu.id);
		printf("name:    %s\n", struct_stu.name);
		printf("major:   %s\n", struct_stu.major);
#endif	
	
	/*关闭套接字*/
	close(sockfd);
	
	return 0;
}