#define MAIN_SERVER_PORT 9000
#define CONTROL_SERVER_PORT 9001

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct net_options
{
    char* serverIP;
    char isMain;
};

struct ssensor_value
{
    int t;
};

struct scontrol_value
{
    int t;
    float tt;
};

extern struct ssensor_value sensor_value;
extern struct scontrol_value control_value;
void *net_process(void* opt);