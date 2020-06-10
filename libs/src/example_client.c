#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "../../includes/net_packet.h"

int main(int argc, char* argv[])
{
    pthread_t pthread;

    //loopback
    char* ip = "127.0.0.1";

    //return value
    int rtnval;

    //you have to set values in 'opt' for networking
    struct net_options opt;

    //when you want to run a client, set serverIP to IP string. (ex:"192.168.0.1")
    opt.serverIP = ip;
    //in this example, send sensor value data to main server.
    opt.isMain = 1;

    rtnval = pthread_create(&pthread, NULL, net_process, (void *)&opt);
    if(rtnval > 0)
    {
        printf("pthread error!\n");
        return -1;
    }

    sensor_value.t = 0;
    while(1)
    {
        sensor_value.t ++;
        sleep(1);
    }
    return 0;
}