sudo rmmod lcd_dev
make clean
make
sudo insmod lcd_dev.ko
gcc -o lcd_app lcd_app.c
