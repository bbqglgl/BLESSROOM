#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define NET_BUFSIZE 512

int Socket_Init()
{
	int fd;
	struct sockaddr_in address;

    char buf[NET_BUFSIZE];

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd == 0)
    {
        printf("socket failed.\n");
        return -1;
    }

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9000);

    if (bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        printf("bind failed.\n");
        return -2;
    }
    return fd;
}

int main(int argc, char* argv[])
{
	int retval;
	// 데이터 통신에 사용할 변수
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[NET_BUFSIZE + 1];
	int t;

	int sock = Socket_Init();
	if (sock < 0)
	{
		printf("Init error\n");
		return 0;
	}

	// 클라이언트와 데이터 통신
	while (1) {
		// 데이터 받기
		addrlen = sizeof(clientaddr);
		retval = recvfrom(sock, buf, NET_BUFSIZE, 0,
			(struct sockaddr*)&clientaddr, &addrlen);
		if (retval < 0) {
			continue;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[UDP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port), buf);

		// 데이터 보내기
		retval = sendto(sock, buf, retval, 0,
			(struct sockaddr*)&clientaddr, sizeof(clientaddr));
		if (retval < 0) {
			printf("send() error\n");
			continue;
		}
	}

	// closesocket()
	close(sock);

	return 0;
}