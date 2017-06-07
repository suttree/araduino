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
