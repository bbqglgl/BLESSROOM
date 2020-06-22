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

#define MOTOR_MAJOR_NUMBER 505
#define MOTOR_MINOR_NUMBER 101
#define MOTOR_DEV_NAME   "motor_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0X04
#define GPLEV0 0x34
//PWM
#define PWM_BASE_ADDR 0x3F20C000
//
#define PWM_CTL 0x00
#define PWM_RNG1 0x20
#define PWM_DAT1 0x24

// clock control
#define CLK_BASE_ADDR 0x3F101000
//offset
#define CLK_PWM_CTL 0xa0
#define CLK_PWM_DIV 0xa4

//PASSWORD
#define BCM_PASSWORD 0x5A000000

static void __iomem *gpio_base;
volatile unsigned int *gpsel1;
volatile unsigned int *gplev1;

static void __iomem *pwm;
volatile unsigned int *pwmctl;
volatile unsigned int *pwmrng1;
volatile unsigned int *pwmdat1;

static void __iomem *clk;
volatile unsigned int *clkdiv;
volatile unsigned int *clkctl;

#define IOCTL_MAGIC_NUMBER_S 's'
#define IOCTL_CMD_SET_ANGLE _IOWR(IOCTL_MAGIC_NUMBER_S, 0, int)
#define IOCTL_CMD_READ_ANGLE _IOWR(IOCTL_MAGIC_NUMBER_S,1,int)

int init_motor(void) {
   int pwm_ctrl = *pwmctl;
   *pwmctl = 0;                // store PWM control and stop PWM
   msleep(10);                  // sleep
   *clkctl = BCM_PASSWORD | (0x01 << 5); // stop PWM Clock
   msleep(10);                  //sleep
   
   int idiv = (int)(19200000.0f / 153600.0f); // Oscilloscope to 153600Hz
   *clkdiv = BCM_PASSWORD | (idiv << 12); // integer part of divisior register
   *clkctl = BCM_PASSWORD | (0x11); //set source to oscilloscope & enable PWM CLK

   *pwmctl = pwm_ctrl;             // restore PWM control and enable PWM
}

int motor_open(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "motor driver open!!\n");
   
   gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
   gpsel1=(volatile unsigned int *)(gpio_base+GPFSEL1);
   gplev1 = (volatile unsigned int *)(gpio_base+GPLEV0);
   clk = ioremap(CLK_BASE_ADDR, 0xFF);   //size
   clkctl=(volatile unsigned int*)(clk+CLK_PWM_CTL);
   clkdiv=(volatile unsigned int*)(clk+CLK_PWM_DIV);
   
   pwm = ioremap(PWM_BASE_ADDR, 0xFF);
   pwmctl = (volatile unsigned int*)(pwm+PWM_CTL);
   pwmrng1 = (volatile unsigned int*)(pwm+PWM_RNG1);
   pwmdat1 = (volatile unsigned int*)(pwm+PWM_DAT1);
   *gpsel1 |= (0b100 << 9);
   *pwmctl |= (1<<8);            //PWEN       1
   *pwmctl &= ~(1<<9);          //MODE1      0
   *pwmctl |= (1<<15);          //MSEN1      1
   *pwmrng1 = 3072;            //RANGE      3072
   *pwmdat1 = 0;
   
   init_motor();
   return 0;
}

int motor_release(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "motor driver close!!\n");
   iounmap((void*)gpio_base);
   iounmap((void*)clk);
   iounmap((void*)pwm);
   return 0;
}

long motor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
   unsigned int kbuf = 0;
 
   switch(cmd) {
      // motor pulse width 0.5ms(-90) ~ 2.5ms(90)
      // 3072 / 20ms : 153.6 => 1
      // (153.6 / 2 = 77) ~ (153.6 * 2.5 = 384) 
      // 230 : 0degree
      case IOCTL_CMD_SET_ANGLE:
         printk(KERN_ALERT "motor angle set\n");
         copy_from_user(&kbuf, (const void*)arg, 4);
         *pwmdat1 = kbuf;    //DAT  77 ~ 384
         break;
       
      case IOCTL_CMD_READ_ANGLE: 
         printk(KERN_ALERT "motor angle read\n");
         kbuf = 0;
         kbuf = *pwmdat1;
         copy_to_user((const void*)arg,&kbuf,4);
         break;
   }

   return 0;
}

static struct file_operations motor_fops = {
   .owner = THIS_MODULE,
   .open = motor_open,
   .release = motor_release,
   .unlocked_ioctl = motor_ioctl,
};
   
int __init motor_init(void){
   if(register_chrdev(MOTOR_MAJOR_NUMBER, MOTOR_DEV_NAME, &motor_fops) < 0)
      printk(KERN_ALERT "motor driver initialization fail\n");
   else
      printk(KERN_ALERT "motor driver initialization success\n");
   
   return 0;
}

void __exit motor_exit(void){
   unregister_chrdev(MOTOR_MAJOR_NUMBER, MOTOR_DEV_NAME);
   printk(KERN_ALERT "motor driver exit done\n");
}

module_init(motor_init);
module_exit(motor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You seong min");
MODULE_DESCRIPTION("motor");
