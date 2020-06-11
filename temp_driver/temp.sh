sudo rmmod temp_dev
make clean
make
sudo insmod temp_dev.ko
gcc -o temp_app temp_app.c
