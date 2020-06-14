sudo rmmod light_dev
sudo rmmod sound_dev
sudo rmmod gas_dev
make clean
make
sudo insmod light_dev.ko
sudo insmod sound_dev.ko
sudo insmod gas_dev.ko
rm app
gcc -o app app.c
sudo ./app