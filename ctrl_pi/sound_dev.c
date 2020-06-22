#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/delay.h>

#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define SOUND_MAJOR_NUMBER 509
#define SOUND_MINOR_NUMBER 105
#define SOUND_DEV_NAME "sound_dev"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0x04
#define GPSET0 0x1c
#define GPCLR0 0x28
#define GPLEV0 0x34

#define IOCTL_MAGIC_NUMBER_J	'j'
#define IOCTL_CMD_SET_SOUND_ON		_IOWR(IOCTL_MAGIC_NUMBER_J, 1, int)
#define IOCTL_CMD_SET_SOUND_OFF		_IOWR(IOCTL_MAGIC_NUMBER_J, 2, int)

static void __iomem *gpio_base;
volatile unsigned int *gpsel1;
volatile unsigned int *gpset1;
volatile unsigned int *gpclr1;

int sound_open(struct inode *inode, struct file *filp){
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
	gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
	gpset1 = (volatile unsigned int *)(gpio_base + GPSET0);
	gpclr1 = (volatile unsigned int *)(gpio_base + GPCLR0);
	*gpsel1 |= (1<<18); // 16pin output mode
	return 0;
}

int sound_release(struct inode *inode, struct file *filp){
	iounmap((void *)gpio_base);
	return 0;
}

long sound_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	switch(cmd)
    {
		case IOCTL_CMD_SET_SOUND_ON:
			*gpset1 |= (1<<16); // out 1
			break;
		case IOCTL_CMD_SET_SOUND_OFF:
			*gpclr1 |= (1<<16); // out 0
			break;
		}
		return 0;
}

static struct file_operations sound_fops = {
	.owner = THIS_MODULE,
	.open = sound_open,
	.release = sound_release,
	.unlocked_ioctl = sound_ioctl
};

int __init sound_init(void){
	if(register_chrdev(SOUND_MAJOR_NUMBER, SOUND_DEV_NAME, &sound_fops) < 0){
		printk(KERN_ALERT "SOUND driver initalization fail\n");
	}else{
		printk(KERN_ALERT "SOUND driver initalization success\n");
	}
	return 0;
}

void __exit sound_exit(void){
	unregister_chrdev(SOUND_MAJOR_NUMBER, SOUND_DEV_NAME);
	printk(KERN_ALERT "SOUND driver exit done!!\n");
}

module_init(sound_init);
module_exit(sound_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You sung min");
MODULE_DESCRIPTION("sound");
