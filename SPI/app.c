#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

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

int fd_l;
int fd_s;
int fd_g;

int main(void){
	dev_t light_dev;
	dev_t sound_dev;
	dev_t gas_dev;
	
	int d;
	int get;
	int res_l;
	int res_s;
	int res_g;
	
	int val;
	int val2;
	
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
	
	while(1){
		get = ioctl(fd_l,LIGHT_GET,&res_l);
		ioctl(fd_s,SOUND_GET,&res_s);
		ioctl(fd_g,GAS_GET,&res_g);
		printf("LIGHT! : %d\n",res_l);
		printf("SOUND! : %d\n",res_s);
		printf("GAS! : %d\n",res_g);
		
		sleep(0.5);
	}
	
	close(fd_l);
	close(fd_s);
	close(fd_g);
	return 0;
}
