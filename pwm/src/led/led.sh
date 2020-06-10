sudo rmmod led_ctrl_dev
make clean
make
sudo insmod led_ctrl_dev.ko
gcc -o led_ctrl_app led_ctrl_app.c
