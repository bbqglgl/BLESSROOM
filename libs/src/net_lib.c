#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../includes/net_packet.h"

#define NET_BUFSIZE 512

struct sockaddr_in address_client;
struct sockaddr_in address_server;
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


    if(option->serverIP == NULL)
    {
	    memset(&address_server, 0, sizeof(address_server));
    address_server.sin_family = AF_INET;
		address_server.sin_addr.s_addr = htonl(INADDR_ANY);
    address_server.sin_port = htons(tPort);
    }
    else
    {
	    memset(&address_client,0,sizeof(address_client));
	    address_client.sin_family = AF_INET;
	    address_client.sin_addr.s_addr = inet_addr(option->serverIP);
	    address_client.sin_port = htons(tPort);
    }
    printf("%d\n",tPort);

	//서버일때만 bind()
	if(option->serverIP == NULL)
	{
    	if (bind(fd, (struct sockaddr*)&address_server, sizeof(address_server)) < 0)
    	{
        	printf("bind failed.\n");
	        return -2;
    	}
	}
    return fd;
}


void net_server(int sock, char isMain)
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
void net_client(int sock, char isMain)
{
	int retval;
	// 클라이언트와 데이터 통신
	while (1) {
		// 데이터 보내기
		if(isMain == 1)
		{
			retval = sendto(sock, (char*)(&sensor_value), sizeof(struct ssensor_value), 0,
				(struct sockaddr*)&address_client, sizeof(struct sockaddr));
		}
		else
		{
			retval = sendto(sock, (char*)(&control_value), sizeof(struct scontrol_value), 0,
				(struct sockaddr*)&address_client, sizeof(struct sockaddr));
		}
		if (retval < 0) {
			printf("send() error\n");
			sleep(1);
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
		net_client(sock,t->isMain);
	else
		net_server(sock,t->isMain);

	// closesocket()
	close(sock);

	return 0;
}
