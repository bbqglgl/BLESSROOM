sudo rmmod pwm_dev
make clean
make
sudo insmod pwm_dev.ko
gcc -o pwm pwm.c
