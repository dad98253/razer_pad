# razer_pad
A fun little program that changes the Razer Goliathus Extended Chroma Gaming Mouse Pad color based on CPU utilization

Compiled using gcc on ubuntu 18.04

It requires the OpenRazer driver
(see https://openrazer.github.io/ )
on ubuntu: 
sudo apt-get install openrazer-meta
sudo reboot

To run the program in the background:

sudo pad &
(should work without sudo, also)

If you get an "Error 1", it means that something went wrong when you installed the
razer driver (did you remember to reboot?)
If the driver install worked, the following command should find something:
find /sys/devices -name "matrix_brightness" -print
(see comments in the code regarding the fopen)

To run the program at boot time, edit the crontab for your user account:
crontab -e 
# add the following line to the file:
@reboot <path to your pad binary>

If you are running polychormatic or some other razer control software, you may need to kill it 
before starting this program...

For a demo, see: https://youtu.be/ERGbdmmCCAA
