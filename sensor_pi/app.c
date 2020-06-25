#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <pthread.h>
#include "../includes/net_packet.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#define LIGHT_DEV_PATH_NAME "/dev/light_dev"
#define LIGHT_MAJOR_NUMBER 501
#define LIGHT_MINOR_NUMBER 100
#define LIGHT_DEV_NAME   "light_dev"

#define SOUND_DEV_PATH_NAME "/dev/sound_dev"
#define SOUND_MAJOR_NUMBER 502
#define SOUND_MINOR_NUMBER 101
#define SOUND_DEV_NAME   "sound_dev"

#define GAS_DEV_PATH_NAME "/dev/gas_dev"
#define GAS_MAJOR_NUMBER 503
#define GAS_MINOR_NUMBER 102
#define GAS_DEV_NAME   "gas_dev"

#define TEMP_DEV_PATH_NAME "/dev/temp_dev"
#define TEMP_MAJOR_NUMBER 504
#define TEMP_MINOR_NUMBER 103
#define TEMP_DEV_NAME   "temp_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0X04 //000 input 001 output
#define GPSET0 0x1c
#define GPLEV0 0x34
#define GPCLR0 0x28

#define IOCTL_MAGIC_NUMBER_G 'x'
#define GAS_SET _IOWR(IOCTL_MAGIC_NUMBER_G, 0, int)
#define GAS_GET _IOWR(IOCTL_MAGIC_NUMBER_G, 1, int)

#define IOCTL_MAGIC_NUMBER_L 'y'
#define LIGHT_SET _IOWR(IOCTL_MAGIC_NUMBER_L, 0, int)
#define LIGHT_GET _IOWR(IOCTL_MAGIC_NUMBER_L, 1, int)

#define IOCTL_MAGIC_NUMBER_S 'z'
#define SOUND_SET _IOWR(IOCTL_MAGIC_NUMBER_S, 0, int)
#define SOUND_GET _IOWR(IOCTL_MAGIC_NUMBER_S, 1, int)


#define IOCTL_MAGIC_NUMBER_T 't'
#define TEMP_SET _IOWR(IOCTL_MAGIC_NUMBER_T, 0, int)

#define RLOAD 10.0
// Calibration resistance at atmospheric CO2 level
#define RZERO 56.81162928
// Parameters for calculating ppm of CO2 from sensor resistance
#define PARA 116.6020682
#define PARB 2.769034857

// Parameters to model temperature and humidity dependence
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018

// Atmospheric CO2 level for calibration purposes
#define ATMOCO2 415.16

#define TEMP_THRESHOLD 10
#define MOIST_THRESHOLD 20

int fd_l;
int fd_s;
int fd_g;
int fd_t;
int tmp_i[4];
int tmp1[2];
int q = 0;

int main(void){
	dev_t light_dev;
	dev_t sound_dev;
	dev_t gas_dev;
	pthread_t pthread;
	dev_t temp_dev;
   
   
	temp_dev=makedev(TEMP_MAJOR_NUMBER, TEMP_MINOR_NUMBER);
	mknod(TEMP_DEV_PATH_NAME, S_IFCHR|0666, temp_dev);
   
  
  
	fd_t = open(TEMP_DEV_PATH_NAME, O_RDWR);
	int d;
	int get;
	int res_l;
	int res_s;
	int res_g;
	float voltage;
	int val;
	int val2;
	float rs;
	
    //loopback
    char* ip = "192.168.100.4";
	float rr = 0;
    //return value
    int rtnval;

    //you have to set values in 'opt' for networking
    struct net_options opt;
    
	
	light_dev = makedev(LIGHT_MAJOR_NUMBER,LIGHT_MINOR_NUMBER);
	sound_dev = makedev(SOUND_MAJOR_NUMBER,SOUND_MINOR_NUMBER);
	gas_dev = makedev(GAS_MAJOR_NUMBER,GAS_MINOR_NUMBER);
	
	mknod(LIGHT_DEV_PATH_NAME,S_IFCHR|0666,light_dev);
	mknod(SOUND_DEV_PATH_NAME,S_IFCHR|0666,sound_dev);
	mknod(GAS_DEV_PATH_NAME,S_IFCHR|0666,gas_dev);
	
	fd_l = open(LIGHT_DEV_PATH_NAME, O_RDWR);
	fd_s = open(SOUND_DEV_PATH_NAME,O_RDWR);
	fd_g = open(GAS_DEV_PATH_NAME,O_RDWR);
	
	ioctl(fd_l,LIGHT_SET,&d);
	ioctl(fd_s,SOUND_SET,&val);
	ioctl(fd_g,GAS_SET,&val2);
	
	sleep(1);
	  

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
	
	while(1){
		get = ioctl(fd_l,LIGHT_GET,&res_l);
		ioctl(fd_s,SOUND_GET,&res_s);
		ioctl(fd_g,GAS_GET,&res_g);
		
		ioctl(fd_t, TEMP_SET, &q);
		printf("%d\n",res_g);
		voltage = ((float)res_g /(float)1023.0)*5.0;
		rs = (5.0-voltage)/voltage * 100;
		rr = PARA * powf((rs/RZERO),-PARB);
		//rr = ((1023./res_g)*5 - 1)*RLOAD;
		//rr = rr * pow((ATMOCO2/PARA), (1./PARB));
		//rr =  PARA * pow((rr/RZERO), -PARB);
		res_g = rr;
		tmp1[0] = ((0xff00&q)>>8); // humi
		tmp1[1] = (0xff&q); // temp
		tmp_i[0] = (tmp1[0] - (tmp1[0]%10))/10; //huminity 10 digit
		tmp_i[1] = tmp1[0]%10;  //huminity 1 digit
		tmp_i[2] = (tmp1[1] - (tmp1[1]%10))/10;    //Temp 10 digit
		tmp_i[3] = tmp1[1]%10; // temp 1 digit
     
		printf("humi : %d, temp : %d \n",tmp1[0],tmp1[1]);
		sensor_value.gas = res_g;
		sensor_value.light = res_l;
		sensor_value.sound = res_s;
		sensor_value.humi = tmp1[0];
		sensor_value.temp = tmp1[1];
		printf("LIGHT! : %d\n",sensor_value.light);
		printf("SOUND! : %d\n",sensor_value.sound);
		printf("GAS! : %d\n",sensor_value.gas);
		printf("HUMID! : %d\n",sensor_value.humi);
		printf("TEMPARATURE! : %d\n",sensor_value.temp);
				
		sleep(0.5);
	}
	
	close(fd_l);
	close(fd_s);
	close(fd_g);
	return 0;
}
