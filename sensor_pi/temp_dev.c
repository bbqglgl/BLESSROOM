#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define TEMP_MAJOR_NUMBER 504
#define TEMP_DEV_NAME   "temp_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0X04
#define GPSET0 0x1c
#define GPCLR0 0x28
#define GPLEV 0x34

#define IOCTL_MAGIC_NUMBER 't'
#define TEMP_SET _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
int dht11_dat[5] = { 0, 0, 0, 0, 0 };

static void __iomem *gpio_base;
volatile unsigned int *gpsel1;
volatile unsigned int *gpset1;
volatile unsigned int *gpclr1;
volatile unsigned int *gplev0;



int TEMP_open(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "TEMPA driver open!!\n");
   
   gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
   gpsel1=(volatile unsigned int *)(gpio_base+GPFSEL1);
   gpset1=(volatile unsigned int *)(gpio_base+GPSET0);
   gpclr1=(volatile unsigned int *)(gpio_base+GPCLR0);
   gplev0 = (volatile unsigned int *)(gpio_base + GPLEV);
   return 0;
}

int TEMP_release(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "TEMPA driver close!!\n");
   iounmap((void*)gpio_base);
   return 0;
}

long TEMP_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	
    int counter = 0;
    int j = 0;
    int i;
    int temp=0;
	int flag=0;
	int data = 0;
	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
	
	/*
	Counts the number of events and responses sent combined. 
	Counts from 0 to 255 then from 0 again.
	*/
	switch(cmd) {
		case TEMP_SET:
			*gpsel1 |= (1<<21);	
			*gpclr1 |= (1<<17);	// bus down, send start signal using 17 pin
			mdelay(18); 		// delay greater than 18ms, so DHT11 start signal can be detected
			*gpset1 |= (1<<17);	// bus up, sned start signale using 17 pin
			udelay(40); 		// Wait for DHT11 response
			*gpsel1 &= ~(1<<21); // clear for reading data
			while(counter<255){ // 8bit temperature
				/*
				* connection setting
				* 80us low signal
				* 80us high signal
				* and then transfer start
				*
				* according to experience result
				* If normal, the count cannot exceed 255us in any case.
				* it means low signal repeat until 255us
				* if normal, signal must have rising edge
				*/
				if(((*gplev0) &( 1 << 17))) // detect  voltage
					break;
				udelay(1);
				counter++;
			}
			if(counter==255){
				flag=1;
				return;
			}
			counter=0; // next 8 bit check
			udelay(110); // term for next
			for ( i= 0; i < 40; i++ ) // all data construct 40 bit 
			{ // 8bit * 5
				/*
				* communication 
				* if 1 bit transfer to pi, then 50us low signal until ,
				* and then rising edge until 26 ~ 28us then data is 0
				* or until 70us then data is 1
				*/
				counter = 0;
				while ( counter<255 )
				{ // wait rising dege
					if(((*gplev0) &( 1 << 17))) // if rising edge start
						break; // then break loop
					udelay(1);
					counter++;
				}
				if(counter==255){
					flag=1;
					return;
				}
				counter=0;
				
				while(((*gplev0) &( 1 << 17))){
					udelay(1);
					counter++;
					
					if(counter==255){
						flag=1;
						return;
					}
				}
				j=i/8; // set data location in j
				// write '0' 
				dht11_dat[j] <<= 1; // initialize 0 then, dht11_dat[j] <<= 1 then, dht_dat[j] = 0
				/*
				 * in datasheet
				 * 26 ~ 28us means data '0'
				 * counter less than 26 value
				 * 25 is Proper value...
				 * because program delay  
				 * */
				/*
				*
				*
				*
				*/
				if ( counter > 25)
					dht11_dat[j] =dht11_dat[j]+ 1;
					
			}
			/*
			 * data 
			 * 1. humi high data (8 bit)
			 * 2. humi low data(8 bit)
			 * 3. temp high data(8 bit)
			 * 4. temp low data(8 bit)
			 * 
			 * The exact values are introduced below,
			 * In the kernel, the division operation causes problems
             * Use approximate values.
             * 
			 * Humidity Value
			 * ((float)((integral RH data << 8) + 8bit decimal RH data)/(float)10.0)
			 * Temperature Value 
			 * ((float)((8bit integral T data << 8) + 8bit decimal T data)/(float)10.0)
			 */
			 
			data=(dht11_dat[0]<<8)|(dht11_dat[2]);
			copy_to_user((const void*)arg, &data, 4); 
			printk(KERN_ALERT "Humidity = %d.%d %% Temperature = %d.%d \n",
					dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3]);
			// parity bit check
			if ((dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 256) ) )
			{
				printk(KERN_ALERT "Humidity = %d.%d %% Temperature = %d.%d \n",
					dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3]);
			}else  {
				printk(KERN_ALERT "Data not good, skip\n" );
			}
			
			break;
	}
	return 1700; //no error.
}

static struct file_operations TEMP_fops = {
	.owner = THIS_MODULE,
	.open = TEMP_open,
	.release = TEMP_release,
	.unlocked_ioctl = TEMP_ioctl
};
	

int __init TEMP_init(void){
	if(register_chrdev(TEMP_MAJOR_NUMBER, TEMP_DEV_NAME, &TEMP_fops) < 0)
		printk(KERN_ALERT "TEMPA driver initialization fail\n");
	else
		printk(KERN_ALERT "TEMPA driver initialization success\n");
	
	return 0;
}


void __exit TEMP_exit(void){
	unregister_chrdev(TEMP_MAJOR_NUMBER, TEMP_DEV_NAME);
	printk(KERN_ALERT "TEMPA driver exit done\n");
}

module_init(TEMP_init);
module_exit(TEMP_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You seong min");
MODULE_DESCRIPTION("des");

	
