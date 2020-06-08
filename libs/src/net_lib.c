#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../includes/net_packet.h"

#define NET_BUFSIZE 512

struct sockaddr_in address;
char isMain;
struct ssensor_value sensor_value;
struct scontrol_value control_value;
int Socket_Init(struct net_options *option)
{
	int fd;
	int tPort;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd == 0)
    {
        printf("socket failed.\n");
        return -1;
    }
	if(option->isMain)
		tPort = MAIN_SERVER_PORT;
	else
		tPort = CONTROL_SERVER_PORT;
	
	isMain = option->isMain;

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
	if(option->serverIP == NULL)
		address.sin_addr.s_addr = htonl(INADDR_ANY);
	else
    	address.sin_addr.s_addr = inet_addr(option->serverIP);
    address.sin_port = htons(tPort);

	//서버일때만 bind()
	if(option->serverIP == NULL)
	{
    	if (bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    	{
        	printf("bind failed.\n");
	        return -2;
    	}
	}
    return fd;
}


void net_server(int sock)
{
	int retval;
	struct sockaddr_in othersaddr;
	socklen_t addrlen;
	// 클라이언트와 데이터 통신
	while (1) {
		// 데이터 받기
		addrlen = sizeof(othersaddr);
		if(isMain == 1)
		{
			retval = recvfrom(sock, (char*)(&sensor_value), sizeof(struct ssensor_value), 0,
				(struct sockaddr*)&othersaddr, &addrlen);
		}
		else
		{
			retval = recvfrom(sock, (char*)(&control_value), sizeof(struct scontrol_value), 0,
				(struct sockaddr*)&othersaddr, &addrlen);
		}
		if (retval < 0) {
			//error 발생
			continue;
		}
	}
}
void net_client(int sock)
{
	int retval;
	// 클라이언트와 데이터 통신
	while (1) {
		// 데이터 보내기
		if(isMain == 1)
		{
			retval = sendto(sock, (char*)(&sensor_value), sizeof(struct ssensor_value), 0,
				(struct sockaddr*)&address, sizeof(address));
		}
		else
		{
			retval = sendto(sock, (char*)(&control_value), sizeof(struct scontrol_value), 0,
				(struct sockaddr*)&address, sizeof(address));
		}
		if (retval < 0) {
			printf("send() error\n");
			continue;
		}
		sleep(1);
	}
}

void *net_process(void* opt)
{
	struct net_options* t = (struct net_options *)opt;
	int sock = Socket_Init(t);
	if (sock < 0)
	{
		printf("Init error\n");
		return 0;
	}

	if(t->serverIP != NULL)
		net_client(sock);
	else
		net_server(sock);

	// closesocket()
	close(sock);

	return 0;
}
