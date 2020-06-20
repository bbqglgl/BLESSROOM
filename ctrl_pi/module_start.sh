sudo rmmod led_dev
sudo rmmod motor_dev
sudo rmmod sound_dev
sudo make clean
sudo make
sudo insmod led_dev.ko
sudo insmod motor_dev.ko
sudo insmod sound_dev.ko
gcc -o ctrl ctrl.c -L../libs -lmynet -W -Wall -pthread