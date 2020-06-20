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
    int humi;
    int temp;
    int gas;
    int sound;
    int light;
};

struct scontrol_value
{
    int led; //0~1024
    int window; //77~384
    int sound; //77~384
};

extern struct ssensor_value sensor_value;
extern struct scontrol_value control_value;
void *net_process(void* opt);
