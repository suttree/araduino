Installing Scipy via pip needs plenty of RAM and Swap. Do this if it times out
and fails with "exit status 4" or c++ gets killed after a long time:

1. Edit the swap conf file to give 1024 of swap (or more):
sudo vi /etc/dphys-swapfile 
CONF_SWAPSIZE=1024
etc

2. Restart the swap process:
sudo /etc/init.d/dphys-swapfile stop
sudo /etc/init.d/dphys-swapfile start

3. Install scipy via pip (with a unicode export fix first too):
sudo LC_ALL=C pip install scipy

--
Better INSTALL notes

http://www.makeuseof.com/tag/getting-started-raspberry-pi-zero/
https://gist.github.com/gbaman/975e2db164b3ca2b51ae11e45e8fd40a
https://stevegrunwell.com/blog/raspberry-pi-zero-share-internet/
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install git
mkdir src; cd src
git clone https://github.com/suttree/araduino.git
sudo apt-get install python-pip
sudo apt-get install python-dev
sudo reboot
https://raspberrypi.stackexchange.com/questions/70/how-to-set-up-swap-space
sudo LC_ALL=C pip install ephem
sudo LC_ALL=C pip install numpy
sudo apt-get install libblas-dev liblapack-dev libatlas-base-dev gfortran
sudo pip install --upgrade pip
sudo LC_ALL=C pip install scipy
<<<<<<UNDO SWAP FILE FIX>>>>>>
sudo reboot

<<<<< make apple-pi baker backup image >>>>>

<<<<<<TEST AUDIO FIRST>>>>>>>
run next line by line...
https://learn.pimoroni.com/tutorial/phat/raspberry-pi-phat-dac-install
(just asound.conf and config.txt changes)

sudo reboot
sudo apt-get install python-pygame
