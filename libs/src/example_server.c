#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "../../includes/net_packet.h"

int main(int argc, char* argv[])
{
    pthread_t pthread;

    //return value
    int rtnval;

    //you have to set values in 'opt' for networking
    struct net_options opt;

    //when you want to run a server, set serverIP to NULL
    opt.serverIP = NULL;
    //in this example, get sensor value data from client.
    opt.isMain = 0;

    rtnval = pthread_create(&pthread, NULL, net_process, (void *)&opt);
    if(rtnval > 0)
    {
        printf("pthread error!\n");
        return -1;
    }

    while(1)
    {
        printf("led : %d\n",control_value.led);
        printf("pwm1 : %d\n",control_value.pwm1);
        printf("pwm2 : %d\n",control_value.pwm2);
        sleep(1);
    }
    return 0;
}