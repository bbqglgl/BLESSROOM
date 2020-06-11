sudo rmmod act_dev
make clean
make
sudo insmod act_dev.ko
gcc -o motor motor.c
