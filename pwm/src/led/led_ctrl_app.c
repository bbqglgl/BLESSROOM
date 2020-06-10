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

#define IOCTL_MAGIC_NUMBER	'p'
#define IOCTL_CMD_SET_BRIGHTNESS _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_READ_BRIGHTNESS _IOWR(IOCTL_MAGIC_NUMBER, 1, int)

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
	
		int current_brightness = 0;
		int input = 0;
		int i = 0;
		
		while(1){
			// down
			usleep(100);
			printf("What brightness would you like to set? (0 ~ 1024) >> ");
			scanf("%d",&input);
			ioctl(led, IOCTL_CMD_READ_BRIGHTNESS, &current_brightness);
			printf("\ncurrent brightness is %d\n",current_brightness);
			printf("settring brightness %d...\n",input);

			if(input < current_brightness){
				for(i=current_brightness; i>=input; i--){
					ioctl(led, IOCTL_CMD_SET_BRIGHTNESS, &i);
					usleep(3000); // 1kHz period is 1ms
				}
			}
			// up
			else{
				for(i=current_brightness; i<=input; i++){
					ioctl(led, IOCTL_CMD_SET_BRIGHTNESS, &i);
					usleep(3000); // 1kHz period is 1ms
				}
			}
		}
   close(led);
   return 0;
}

