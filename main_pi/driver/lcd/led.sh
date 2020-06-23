sudo rmmod lcd_dev
make clean
make
sudo insmod lcd_dev.ko
gcc -o lcd_app lcd_app.c
sudo mknod -m 666 /dev/lcd_dev c 510 100
