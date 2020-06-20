#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>


#define TEMP_DEV_PATH_NAME "/dev/temp_dev"
#define TEMP_MAJOR_NUMBER 504
#define TEMP_MINOR_NUMBER 103
#define TEMP_DEV_NAME   "temp_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0X04 //000 input 001 output
#define GPSET0 0x1c
#define GPLEV0 0x34
#define GPCLR0 0x28

#define IOCTL_MAGIC_NUMBER_T 't'
#define TEMP_SET _IOWR(IOCTL_MAGIC_NUMBER_T, 0, int)

#define TEMP_THRESHOLD 10
#define MOIST_THRESHOLD 20

int fd_t;
int tmp_i[4];
int tmp1[2];
int q = 0;

int main(void)
{
  
   dev_t temp_dev;
   
   
   temp_dev=makedev(TEMP_MAJOR_NUMBER, TEMP_MINOR_NUMBER);
   mknod(TEMP_DEV_PATH_NAME, S_IFCHR|0666, temp_dev);
   
  
  
   fd_t = open(TEMP_DEV_PATH_NAME, O_RDWR);
   // driver send t=(dht11_dat[0]<<8)|(dht11_dat[2]); forward 8 bit humi, backward 8 bit temp
	while(1){
	usleep(100);
	ioctl(fd_t, TEMP_SET, &q);
    tmp1[0] = ((0xff00&q)>>8); // humi
    tmp1[1] = (0xff&q); // temp
    tmp_i[0] = (tmp1[0] - (tmp1[0]%10))/10; //huminity 10 digit
    tmp_i[1] = tmp1[0]%10;  //huminity 1 digit
    tmp_i[2] = (tmp1[1] - (tmp1[1]%10))/10;    //Temp 10 digit
    tmp_i[3] = tmp1[1]%10; // temp 1 digit
     
	printf("humi : %d, temp : %d \n",tmp1[0],tmp1[1]);
  }
   close(fd_t);
   
   return 0;
}

