#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include "../includes/net_packet.h"

#define LED_DEV_PATH_NAME "/dev/led_dev"
#define LED_DEV_NAME   "led_dev"
#define LED_MAJOR_NUMBER 504
#define LED_MINOR_NUMBER 100
#define MOTOR_DEV_PATH_NAME "/dev/motor_dev"
#define MOTOR_DEV_NAME   "motor_dev"
#define MOTOR_MAJOR_NUMBER 505
#define MOTOR_MINOR_NUMBER 101

#define SOUND_DEV_PATH_NAME "/dev/sound_dev"
#define SOUND_DEV_NAME "sound_dev"
#define SOUND_MAJOR_NUMBER 509
#define SOUND_MINOR_NUMBER 105


#define IOCTL_MAGIC_NUMBER_S 's'
#define IOCTL_CMD_SET_ANGLE _IOWR(IOCTL_MAGIC_NUMBER_S, 0, int)
#define IOCTL_CMD_READ_ANGLE _IOWR(IOCTL_MAGIC_NUMBER_S,1,int)

#define IOCTL_MAGIC_NUMBER_P	'p'
#define IOCTL_CMD_SET_BRIGHTNESS _IOWR(IOCTL_MAGIC_NUMBER_P, 0, int)
#define IOCTL_CMD_READ_BRIGHTNESS _IOWR(IOCTL_MAGIC_NUMBER_P, 1, int)

#define IOCTL_MAGIC_NUMBER_J	'j'
#define IOCTL_CMD_SET_SOUND_ON		_IOWR(IOCTL_MAGIC_NUMBER_J, 1, int)
#define IOCTL_CMD_SET_SOUND_OFF		_IOWR(IOCTL_MAGIC_NUMBER_J, 2, int)

float duty_cycle = 0.5; // duty cycle initialize -90 degree : 0.5ms

void *make_pwm(int sound)
{
      while(1){
      ioctl(sound, IOCTL_CMD_SET_SOUND_ON);
      usleep(duty_cycle*1000.0); // duty cycle
      ioctl(sound, IOCTL_CMD_SET_SOUND_OFF);
      usleep((20.0-duty_cycle)*1000.0);// idle time
      }
      return 0;
}


int main(void)
{
   dev_t motor_dev, led_dev, sound_dev;
   int motor,led,sound,rtnval;
   pthread_t pthread;
   int current_angle = 0, current_brightness = 0;
   int i = 0;
   int need_mic_ctrl = 0;
   //you have to set values in 'opt' for networking
    struct net_options opt;
    //when you want to run a server, set serverIP to NULL
    opt.serverIP = NULL;
    //in this example, get sensor value data from client.
    opt.isMain = 0;

   
   motor_dev=makedev(MOTOR_MAJOR_NUMBER,MOTOR_MINOR_NUMBER);
   led_dev =makedev(LED_MAJOR_NUMBER, LED_MINOR_NUMBER);
   sound_dev=makedev(SOUND_MAJOR_NUMBER,SOUND_MINOR_NUMBER);
   mknod(MOTOR_DEV_PATH_NAME, S_IFCHR|0666, motor_dev);
   mknod(LED_DEV_PATH_NAME, S_IFCHR|0666, led_dev);
   mknod(SOUND_DEV_PATH_NAME, S_IFCHR|0666, sound_dev);
   motor=open(MOTOR_DEV_PATH_NAME, O_RDWR);
   led = open(LED_DEV_PATH_NAME, O_RDWR);
   sound = open(SOUND_DEV_PATH_NAME, O_RDWR);

   if(motor<0){
      printf("fail to open motor\n");
      return -1;
   }
   if (led < 0) {
	   printf("fail to open led\n");
	   return -1;
   }
   if(sound < 0){
      printf("fail to open sound\n");
	   return -1;

   }

   rtnval = pthread_create(&pthread, NULL, net_process, (void *)&opt);
   if(rtnval > 0)
   {
      printf("network pthread error!\n");
      return -1;
   }
   // making pwm simulation function => make_pwm
   rtnval = pthread_create(&pthread, NULL, make_pwm, sound);
   if(rtnval > 0)
   {
      printf("sound pthread error!\n");
      return -1;
   }
   printf("sound pthread work start\n");
   printf("start program\n");
   while(1)
   {
      printf("led : %d\n",control_value.led); // 13pin
      printf("windows : %d\n",control_value.window); // 18pin
      printf("mic : %d\n",control_value.sound); // 16pin
      usleep(1000);
      ioctl(motor,IOCTL_CMD_READ_ANGLE,&current_angle);
      usleep(1000);
      printf("current angle setting value : %d\n",current_angle);
      printf("if input = 77 then set -90 degree\nif input = 230 then set 0 degree\n \
      if input = 384 then set 90 degree");
      // windows control motor
      ioctl(motor, IOCTL_CMD_SET_ANGLE, &control_value.window);
	/***********************************************************************************************************************
        * sound control motor period is 20ms
	* original
	* input : 200 = x ms : 20 ms
	* range 0 ~ 200
	* x = input / 10
	**************************************************************************************************************************/ 
      control_value.sound >=5 && control_value.sound <= 25 ? need_mic_ctrl = 1 : 0; 
      if(need_mic_ctrl){
	 duty_cycle = (float)(control_value.sound / 10.0);
	 //make_pwm(sound);
      }
      usleep(1000);
      // led control
      printf(" off brightness : 0,  maximum brightness : 1024) ");
		ioctl(led, IOCTL_CMD_READ_BRIGHTNESS, &current_brightness);
		printf("\ncurrent brightness is %d\n",current_brightness);
		printf("settring brightness %d...\n",control_value.led);
      // brightness down until incoming input from main pi
		if(control_value.led < current_brightness){
			for(i=current_brightness; i>=control_value.led; i--){
				ioctl(led, IOCTL_CMD_SET_BRIGHTNESS, &i);
				usleep(1000); // sleep 1ms
			}
		}
		// brightness up until incoming input from main pi
		else{
			for(i=current_brightness; i<=control_value.led; i++){
				ioctl(led, IOCTL_CMD_SET_BRIGHTNESS, &i);
				usleep(1000);
			}
		}
   }
   
   close(motor);
   close(led);
   close(sound);
   return 0;
}
