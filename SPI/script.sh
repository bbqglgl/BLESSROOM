sudo rmmod light_dev
sudo rmmod sound_dev
sudo rmmod gas_dev
sudo rmmod temp_dev
sudo make clean
sudo make
sudo insmod light_dev.ko
sudo insmod sound_dev.ko
sudo insmod gas_dev.ko
sudo insmod temp_dev.ko
sudo rm app
sudo make app
sudo ./app