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

/*模块选项*/
enum {
	MODULE_TEST_PROTO = 0,
	MODULE_TEST_TLV,
	MODULE_TEST_JSON
};

/*头部*/
typedef struct test_hdr_s {
	unsigned short ver; //头部版本暂定为1  收到后需校验
	unsigned short hdr_len; //头部长度	收到后需校验
	unsigned int dat_len;	//负载数据长度
	unsigned short module;	//当前业务模块
	unsigned short cmd;	//模块下的命令号
}test_hdr_t;

static size_t read_buffer (unsigned max_length, uint8_t *out)
{
	size_t cur_len =0;
	uint8_t nread;
	while((nread=fread(out+ cur_len,1, max_length - cur_len, stdin))!=0)
	{
		cur_len += nread;
		if(cur_len == max_length)
		{
			fprintf(stderr,"max messagelength exceeded\n");
			exit(1);
		}
	}
	return cur_len;
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
	
	int retval;
	while (1) {
		read_fds = fds;
		retval = select(maxfd+1, &read_fds, NULL, NULL, NULL);
		if (0 > retval) {
			perror("select");
			break;
		}
		int i;
		char buf[100] = {0};
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
					int ret = recv(i, (void *)&head, sizeof(test_hdr_t), 0);
					if (ret < 0) {
						/*接收出错*/
						perror("recv");
						continue;
					}
					else if (ret == 0) {
						/*客户端已经关闭*/
						printf("client close!\n");
						FD_CLR(i, &fds);
						close(i);
					}
					else {
						/*将头部的网络字节序转换回主机字节序*/
						head.ver 		= ntohs(head.ver);
						head.hdr_len	= ntohs(head.hdr_len);
						head.dat_len	= ntohl(head.dat_len);
						head.module		= ntohs(head.module);
						head.cmd		= ntohs(head.cmd);		

						/*校验*/
#if 0
						printf("ver: %d\n", head.ver);
						printf("real head len: %d\n", sizeof(head));
						printf("recv head len: %d\n", head.hdr_len);
#endif
						/*校验通过才会向下执行*/
						if (HEAD_VERSION == head.ver && 
							sizeof(head) == head.hdr_len) {
							if(head.module == MODULE_TEST_PROTO) {
								
								if(head.cmd == COMMAND_SET) {
									/*解析为客户端要修改数据*/
									Student *proto_stu;
									char buf[head.dat_len];		//装接收的序列化数据
									int ret = recv(i, (void *)buf, 
													head.dat_len, 0);
									if (ret < 0) {
										/*接收出错*/
										perror("recv");
										continue;
									}
									
									proto_stu = student__unpack(NULL, head.dat_len, buf);

									struct_stu.id    = proto_stu->id;
									strcpy(struct_stu.name,proto_stu->name);
									strcpy(struct_stu.major,proto_stu->major);
#if 1								
									/*显示接收的反序列化后的信息*/
									printf("id:      %d\n", struct_stu.id);
									printf("name:    %s\n", struct_stu.name);
									printf("major:   %s\n", struct_stu.major);
#endif								
								}
								else if (head.cmd == COMMAND_GET) {
									Student proto_stu = STUDENT__INIT;
									void *buf;
			
									proto_stu.id = struct_stu.id;
									proto_stu.name = struct_stu.name;
									proto_stu.major = struct_stu.major;
									//head.dat_len = student__get_packed_size(&proto_stu);	//将序列化后的长度存到头部中
									
									buf = malloc(head.dat_len);
									student__pack(&proto_stu, buf);
									send(i, (void *)buf, head.dat_len, 0);
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