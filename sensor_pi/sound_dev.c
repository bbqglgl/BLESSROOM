#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>


#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define SOUND_MAJOR_NUMBER 502
#define SOUND_DEV_NAME   "sound_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL0 0x00
#define GPFSEL1 0X04
#define GPSET0 0x1c
#define GPCLR0 0x28

#define SPI_BASE_ADDR 0x3F204000
#define SPICS 0x00
#define SPIFIFO 0x04
#define SPICLK 0x08


#define IOCTL_MAGIC_NUMBER 'z'
#define SOUND_SET _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define SOUND_GET _IOWR(IOCTL_MAGIC_NUMBER, 1, int)

static void __iomem *gpio_base;

volatile unsigned int *gpsel0;
volatile unsigned int *gpsel1;
volatile unsigned int *gpset1;
volatile unsigned int *gpclr1;

static void __iomem *spi_base;

volatile unsigned int *cs;
volatile unsigned int *fifo;
volatile unsigned int *clk;


static void __iomem *aux_base;

int sound_open(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "sound driver open!!\n");
   
   gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
   spi_base = ioremap(SPI_BASE_ADDR, 0x0C);
   
   gpsel1=(volatile unsigned int *)(gpio_base+GPFSEL1);
   gpsel0=(volatile unsigned int *)(gpio_base+GPFSEL0);
   gpset1=(volatile unsigned int *)(gpio_base+GPSET0);
   gpclr1=(volatile unsigned int *)(gpio_base+GPCLR0);
   
   cs = (volatile unsigned int*)(spi_base+SPICS);
   fifo = (volatile unsigned int*)(spi_base +SPIFIFO);
   clk = (volatile unsigned int*)(spi_base + SPICLK);
   //rest of register does not use!
   
   return 0;
}

int sound_release(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "sound driver close!!\n");
   iounmap((void*)gpio_base);
   return 0;
}



int get_value(void)
{
    int res = 0;
   char buf1 = 0x01;
   char buf2 = 0x90;    //channel 1
   *cs = 0x000000B0; //Initialization
   *clk = 128;
         
   printk("after setting >> cs register:%d\n", *cs);
   //first cycle
   while(1) {
         if(((*cs) &(1<<18)) != 0) break; //wait for tx buffer has space.(if value == 1, tx buffer at least one bit)
   }
   *fifo = buf1; //0x01 start value send
   while(1){
      if(((*cs) & (1<<16)) != 0) break; //wait for done.
   }
   printk("end first cycle!\n");
         
   //second cycle
   while(1){
      if(((*cs) &(1<<18)) != 0) break;
   }
   *fifo = buf2; // channel 0 start
   while(1){
      if(((*cs) & (1<<16)) != 0) break;
   }


   printk("end second cycle!\n");
   //third cycle
   //send don't care value and check RX buffer and read them all
   while(1){
      if(((*cs) &(1<<18)) != 0) break;
   }
   *fifo = 0x00; // don't care value
   while(1) 
   {
      if(((*cs)&(1<<17)) == 0) break; //check read fifo empty
      buf1 = *fifo; //read bit from mcp(1/2)
   }
      
   
   while(1) if(((*cs) & (1<<16)) != 0) break;
   
   
   printk("end third cycle!\n");
   //fourth cycle
   while(1){
      if(((*cs) &(1<<18)) != 0) break;
   }
   *fifo = 0x00000000; // don't care value
   while(1) 
   {
      if(((*cs)&(1<<17)) == 0) break; //check read fifo empty
      buf2 = *fifo; //read bit from mcp(2/2)
   }
      
   
   while(1){
      if(((*cs) & (1<<16)) != 0) break;
   }

   buf1 = buf1 & 0x03; //mask
         
   printk("buf1:%u\n", buf1); 
   res = (buf1 <<8) | buf2;
   printk("result:%u\n", res); 
      
   *cs = 0x0000; //reset
   return res;
}

long sound_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
   int res = 0;
   int i = 0;
   int flag = 0;
   switch(cmd) {
      case SOUND_SET:
         //gpfsel setting.
         printk("before setting >> cs register:%d\n", *cs);
                  
         *gpsel0 &= ~(1<<24);
         *gpsel0 &= ~(1<<25);
         *gpsel0 |= (1<<26); //GPIO 8 = ALT0
         
         *gpsel0 &= ~(1<<27);
         *gpsel0 &= ~(1<<28);
         *gpsel0 |= (1<<29); //GPIO 9 = ALT0
         
         *gpsel1 &= ~(1<<0);
         *gpsel1 &= ~(1<<1);
         *gpsel1 |= (1<<2); //GPIO 10 = ALT0
         
         *gpsel1 &= ~(1<<3);
         *gpsel1 &= ~(1<<4);
         *gpsel1 |= (1<<5); // GPIO 11 = ALT0
         return 1;

      case SOUND_GET:
         for(i = 0; i < 10; i++)
         {

            res += get_value();
                        
            msleep(10);
         }
         res /= 2;
         //kbuf = get_value();
         msleep(10);
         printk("%d\n",res);
         copy_to_user((const void*)arg, &res, sizeof(int));
         //printk("%ln\n",&arg);
         return 2;
      default:
         return 3;
   }
   return 1500; //no error.
}


static struct file_operations sound_fops = {
   .owner = THIS_MODULE,
   .open = sound_open,
   .release = sound_release,
   .unlocked_ioctl = sound_ioctl
};
   

int __init sound_init(void){
   if(register_chrdev(SOUND_MAJOR_NUMBER, SOUND_DEV_NAME, &sound_fops) < 0)
      printk(KERN_ALERT "sound driver initialization fail\n");
   else
      printk(KERN_ALERT "sound driver initialization success\n");
   
   
   return 0;
}


void __exit sound_exit(void){
   unregister_chrdev(SOUND_MAJOR_NUMBER, SOUND_DEV_NAME);
   printk(KERN_ALERT "sound driver exit done\n");
}

module_init(sound_init);
module_exit(sound_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YoonHaeng");
MODULE_DESCRIPTION("des");

   

