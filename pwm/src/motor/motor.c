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

#define ACT_DEV_PATH_NAME "/dev/act_dev"
#define ACT_MAJOR_NUMBER 505
#define ACT_MINOR_NUMBER 101
#define ACT_DEV_NAME   "act_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0X04 //000 input 001 output
#define GPSET0 0x1c
#define GPLEV0 0x34
#define GPCLR0 0x28

// clock control
#define CLK_BASE_ADDR 0x3F101000
//offset
#define CLK_PWM_CTL 0xa0
#define CLK_PWM_DIV 0xa4
//
#define BCM_PASSWORD 0x5A000000


//PWM address set.
#define PWM_BASE_ADDR 0x3F20C000
#define PWM_CTL 0x00
#define PWM_RNG1 0x10
#define PWM_DAT1 0x14

#define IOCTL_MAGIC_NUMBER_P 's'
#define IOCTL_CMD_SET_ANGLE _IOWR(IOCTL_MAGIC_NUMBER_P, 0, int)
#define IOCTL_CMD_READ_ANGLE _IOWR(IOCTL_MAGIC_NUMBER_P,1,int)



int main(void)
{
   dev_t act_dev;
   int motor;
   
   act_dev=makedev(ACT_MAJOR_NUMBER,ACT_MINOR_NUMBER);
   mknod(ACT_DEV_PATH_NAME, S_IFCHR|0666, act_dev);
  
   motor=open(ACT_DEV_PATH_NAME, O_RDWR);
   
   if(motor<0){
      printf("fail to open motor\n");
      return -1;
   }
   
   int input = 0;
   int angle = 0;
   int dev_value = 0;
   printf("start program\n");
   while(1)
   {
      
      angle=ioctl(motor,IOCTL_CMD_READ_ANGLE,&dev_value);
      usleep(1000);
      printf("current angle setting value : %d\n",angle);
      printf("if input = 77 then set -90 degree\nif input = 230 then set 0 degree\n \
      if input = 384 then set 90 degree");
      printf("please type angle (77 ~ 384) >>> ");
      scanf("%d", &input);
      ioctl(motor, IOCTL_CMD_SET_ANGLE, &input);
      usleep(1000);
   }
   

   close(motor);

   
   return 0;
}
