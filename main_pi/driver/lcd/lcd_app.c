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

#include "lcd.h"

#define TT_DEV_PATH_NAME "/dev/lcd_dev"
#define TT_MAJOR_NUMBER 510
#define TT_MINOR_NUMBER 100
#define TT_DEV_NAME   "lcd_display"

#define IOCTL_MAGIC_NUMBER 'i'
#define IOCTL_CMD_LCD_INIT _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_LCD_TEST _IOWR(IOCTL_MAGIC_NUMBER, 1, int)
#define IOCTL_CMD_LCD_SET_STRING _IOWR(IOCTL_MAGIC_NUMBER, 2, struct str_info)

int main()
{
    dev_t lcd_dev;
    int p, lcd;
    struct str_info s;
  
    lcd_dev =makedev(TT_MAJOR_NUMBER, TT_MINOR_NUMBER);
    mknod(TT_DEV_PATH_NAME, S_IFCHR|0666, lcd_dev);
   
  
    lcd = open(TT_DEV_PATH_NAME, O_RDWR);
  
    if (lcd < 0) {
	   printf("fail to open lcd\n");
	   return -1;
    }
	
	int input = 0;
	
	while(1){
		// down
		usleep(1000);
		printf("1. Init\n");
		printf("etc. Test\n");
		printf("Code Enter >> ");
		scanf("%d",&input);
        if(input == 1)
        {
		    ioctl(lcd, IOCTL_CMD_LCD_INIT, NULL);
        }
        else if(input == 2)
        {
	        ioctl(lcd, IOCTL_CMD_LCD_TEST, NULL);
        }
        else
        {
            scanf("%s",s.string);
            s.line++;
            if(s.line > 1)
                s.line = 0;
	        ioctl(lcd, IOCTL_CMD_LCD_SET_STRING, &s);
        }
	}
   close(lcd);
   return 0;
}

