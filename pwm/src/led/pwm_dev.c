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
#include <asm/delay.h>

#define TT_MAJOR_NUMBER 505
#define TT_DEV_NAME   "led_pwm1"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0X04
static void __iomem *gpio_base;
volatile unsigned int *gpsel1;


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

static void __iomem *clk;
volatile unsigned int *clkdiv;
volatile unsigned int *clkctl;
static void __iomem *pwm;
volatile unsigned int *pwmctl;
volatile unsigned int *pwmrng1;
volatile unsigned int *pwmdat1;


#define IOCTL_MAGIC_NUMBER 'p'
#define IOCTL_CMD_SET_DIRECTION_90_INVERSE _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_SET_DIRECTION_90 _IOWR(IOCTL_MAGIC_NUMBER, 1, int)

int led_open(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "LED driver open!!\n");
   
   gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
   gpsel1=(volatile unsigned int *)(gpio_base+GPFSEL1);
   
   clk = ioremap(CLK_BASE_ADDR, 0xFF);   //size
   clkdiv=(volatile unsigned int*)(clk+CLK_PWM_DIV);
   clkctl=(volatile unsigned int*)(clk+CLK_PWM_CTL);
   pwm = ioremap(PWM_BASE_ADDR, 0xFF);
   pwmctl = (volatile unsigned int*)(pwm+PWM_CTL);
   pwmrng1 = (volatile unsigned int*)(pwm+PWM_RNG1);
   pwmdat1 = (volatile unsigned int*)(pwm+PWM_DAT1);

   return 0;
}

int init_pwm(void) {
   int pwm_ctrl = *pwmctl;
   *pwmctl = 0; // store PWM control and stop PWM
   msleep(10);//
   *clkctl = BCM_PASSWORD | (0x01 << 5); // stop PWM Clock
   msleep(10);//
   // idiv = 1920000 / range * frequency
   int idiv = (int)(19200000.0f / 1024000.0f);// Oscilloscope to 1000Hz
   *clkdiv = BCM_PASSWORD | (idiv << 12); // integer part of divisior register
   *clkctl = BCM_PASSWORD | (0x11); //set source to oscilloscope & enable PWM CLK

   *pwmctl = pwm_ctrl; // restore PWM control and enable PWM
}

int led_release(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "LED driver close!!\n");
   iounmap((void*)gpio_base);
   iounmap((void*)pwm);
   return 0;
}
long led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
   unsigned int kbuf = 0;
   int i = 0;
   init_pwm();
   
    
   switch(cmd) {
      
      case IOCTL_CMD_SET_DIRECTION_90:
         
         copy_from_user(&kbuf, (const void*)arg, 4);
         *gpsel1 |= (1<<8);
         printk(KERN_ALERT "LED breath!!\n");
         *pwmctl |= (1);      //PWEN       1
         *pwmctl &= ~(1<<1);    //MODE1      0
         *pwmctl |= (1<<7);    //MSEN1      MS   1
         *pwmrng1 = (1<<10);      //RANGE      1024
         /*
         int a = kbuf; // addition using bitwise operation
         int b = kbuf + 1; 
         int x = a ^ b;
         int an = a & b;
         an = an << 1;
         while ((an & x) != 0)
         {
            int txor = an^x;
            int tand = an&x;
            int uand = tand << 1;
            an = uand;
            x = txor;
         }
         * */
         *pwmdat1|= kbuf;    //DAT      0 ~ 1024
         
         return 1;
         break;
   }

   return 1800;
}

static struct file_operations led_fops = {
   .owner = THIS_MODULE,
   .open = led_open,
   .release = led_release,
   .unlocked_ioctl = led_ioctl,
};
   
int __init led_init(void){
   if(register_chrdev(TT_MAJOR_NUMBER, TT_DEV_NAME, &led_fops) < 0)
      printk(KERN_ALERT "LED driver initialization fail\n");
   else
      printk(KERN_ALERT "LED driver initialization success\n");
   
   return 0;
}

void __exit led_exit(void){
   unregister_chrdev(TT_MAJOR_NUMBER, TT_DEV_NAME);
   printk(KERN_ALERT "LED driver exit done\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yoo seong min");
MODULE_DESCRIPTION("des");

   