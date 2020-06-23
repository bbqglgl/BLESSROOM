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

#include "lcd.h"

#define TT_MAJOR_NUMBER 510
#define TT_MINOR_NUMBER 100
#define TT_DEV_NAME   "lcd_display"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL0 0X00
static void __iomem *gpio_base;
volatile unsigned int *gpsel0;

#define I2C_BASE_ADDR 0x3F804000
#define I2C_C     0x00
#define I2C_S     0x04
#define I2C_DLEN  0x08
#define I2C_A     0x0c
#define I2C_FIFO  0x10
static void __iomem *i2c_base;
volatile unsigned int *i2c_control;
volatile unsigned int *i2c_status;
volatile unsigned int *i2c_dlen;
volatile unsigned int *i2c_a;
volatile unsigned int *i2c_fifo;

unsigned char string[LCD_LINE][STRING_LEN_MAX];

#define IOCTL_MAGIC_NUMBER 'i'
#define IOCTL_CMD_LCD_INIT _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_LCD_SET_STRING _IOWR(IOCTL_MAGIC_NUMBER, 1, struct str_info)

int lcd_open(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "LCD driver open!!\n");
   
   gpio_base   = ioremap(GPIO_BASE_ADDR, 0x60);
   gpsel0      = (volatile unsigned int *)(gpio_base+GPFSEL0);

   *gpsel0      |= (0b100 << (2*3)); //GPIO2 set alt func0(SDA1)
   *gpsel0      |= (0b100 << (3*3)); //GPIO3 set alt func0(SCL1)
   
   i2c_base    = ioremap(I2C_BASE_ADDR, 0xFF);
   i2c_control = (volatile unsigned int *)(i2c_base+I2C_C);
   i2c_status  = (volatile unsigned int *)(i2c_base+I2C_S);
   i2c_dlen    = (volatile unsigned int *)(i2c_base+I2C_DLEN);
   i2c_a       = (volatile unsigned int *)(i2c_base+I2C_A);
   i2c_fifo    = (volatile unsigned int *)(i2c_base+I2C_FIFO);

   *i2c_control |= (1<<15); //i2c controller enable
   *i2c_dlen    = 1;        //set data length to a byte
   *i2c_a       = 0x27;     //set slave address

   return 0;
}

int lcd_release(struct inode *inode, struct file *filp){
   printk(KERN_ALERT "LCD driver close!!\n");
   iounmap((void*)gpio_base);
   iounmap((void*)i2c_base);
   return 0;
}

void write_i2c(unsigned char byte)
{
   *i2c_fifo     = byte;  //set data in FIFO
   *i2c_control  |= (1<<7);   //Tranfer start

   while((((*i2c_status)&0b10)>>1) == 0); // wait until transfer done
   *i2c_status  |= 0b10;                   // clear Tranfer Done field
   *i2c_control |= (1<<4);   //Clear FIFO

   udelay(10);   //Wait for more then 10us   
}


void lcd_to_i2c(unsigned char byte)
{
   byte = byte & 0xf3;  //drop 2(enable signal) and 3(gap) bit

   //write_i2c(byte & 0x0f);
   byte |= 0b1100;      //lcd enable signal
   write_i2c(byte);

   byte &= ~(1<<2);      //lcd disable signal
   write_i2c(byte);

   usleep_range(50,60);   //Wait for more then 50us
}

void set_couser(unsigned char line, unsigned char loc)
{
   unsigned char t = 0b1000;
   loc &= 0x0f;

   if(line >= 1)
      t |= 0b0100;
   
   printk(KERN_ALERT "%x %x\n", t << 4, loc << 4);

   lcd_to_i2c(t << 4);
   lcd_to_i2c(loc << 4);
}

void write_lcd_char(unsigned char byte)
{
   lcd_to_i2c( (byte&0xf0)       | 0x01);
   lcd_to_i2c( ((byte&0x0f)<<4)  | 0x01);
}

void write_lcd_string(unsigned char line, unsigned char offset)
{
   int i;

   set_couser(line - 1, 0);

   for(i=0 ; i < 16 ; i++)
   {
      if(string[line][i + offset] < ' ')
         break;
      write_lcd_char(string[line][i + offset]);
   }

   for(; i < 16 ; i++)  //fill blanks in display
      write_lcd_char(' ');
}

void lcd_clear(void)
{
   lcd_to_i2c(0x00);
   lcd_to_i2c(0x10);          //Clear display
   usleep_range(2500,3000);   //Wait for more then 2.5ms
}

void init_lcd(void)
{

   lcd_to_i2c(0x30);
   usleep_range(4500,5000);   //Wait for more then 4.1ms

   lcd_to_i2c(0x30);
   usleep_range(150,200);     //Wait for more then 100us

   lcd_to_i2c(0x30);

   lcd_to_i2c(0x20);          //Set 4-Bit interface
   usleep_range(4500,5000);   //Wait for more then 4.1ms
   

   lcd_to_i2c(0x20);          //Set 4-Bit interface
   lcd_to_i2c(0x80);          //Set 2 line and 5x8 font


   lcd_to_i2c(0x00);
   lcd_to_i2c(0xC0);          //Display on
   
   lcd_clear();               //Display Clear

   lcd_to_i2c(0x00);
   lcd_to_i2c(0x60);          //Set entry mode
   
}

long lcd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
   unsigned int kbuf = 0;
   struct str_info t;
    
   switch(cmd) {
      
      case IOCTL_CMD_LCD_INIT:
         init_lcd();
         return 1;
         break;
      case IOCTL_CMD_LCD_SET_STRING:
         copy_from_user(&t, (const void*)arg, sizeof(struct str_info));
         memcpy(string[t.line], t.string, sizeof(t.string));
         write_lcd_string(t.line, 0);
         return 2;
         break;
   }

   return 0;
}

static struct file_operations lcd_fops = {
   .owner = THIS_MODULE,
   .open = lcd_open,
   .release = lcd_release,
   .unlocked_ioctl = lcd_ioctl,
};
   
int __init lcd_init(void){
   if(register_chrdev(TT_MAJOR_NUMBER, TT_DEV_NAME, &lcd_fops) < 0)
      printk(KERN_ALERT "LCD driver initialization fail\n");
   else
      printk(KERN_ALERT "LCD driver initialization success\n");
   
   return 0;
}

void __exit lcd_exit(void){
   unregister_chrdev(TT_MAJOR_NUMBER, TT_DEV_NAME);
   printk(KERN_ALERT "LCD driver exit done\n");
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kiseok Jung");
MODULE_DESCRIPTION("LCD Display 1602 driver");