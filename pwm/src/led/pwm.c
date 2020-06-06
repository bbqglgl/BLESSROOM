#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#define TT_DEV_PATH_NAME "/dev/tt_dev"
#define TT_MAJOR_NUMBER 505
#define TT_MINOR_NUMBER 100
#define TT_DEV_NAME   "led_pwm1"



#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0X04 //000 input 001 output
#define GPSET0 0x1c
#define GPLEV0 0x34
#define GPCLR0 0x28

#define IOCTL_MAGIC_NUMBER_P 'p'
#define IOCTL_CMD_SET_DIRECTION_90_INVERSE _IOWR(IOCTL_MAGIC_NUMBER_P, 0, int)
#define IOCTL_CMD_SET_DIRECTION_90 _IOWR(IOCTL_MAGIC_NUMBER_P, 1, int)

int main()
{

   dev_t pwm_dev;
   int p, led;
  

   pwm_dev =makedev(TT_MAJOR_NUMBER, TT_MINOR_NUMBER);
   mknod(TT_DEV_PATH_NAME, S_IFCHR|0666, pwm_dev);
   
  
  
   led = open(TT_DEV_PATH_NAME, O_RDWR);
  
   if (led < 0) {
	   printf("fail to open led\n");
	   return -1;
   }
	
		int i = 0;
		
		while(1){
			printf("cycle\n");
			for(; i<=10; ++i){
				ioctl(led, IOCTL_CMD_SET_DIRECTION_90, &i);
				sleep(1);
			}
			sleep(3);
		}
		
	
				
			
		
		
		
		
       

   close(led);
   return 0;
}

