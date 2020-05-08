create /boot/ssh file
create wpa_supplicant file

sudo apt-get install git
git clone https://www.github.com/suttree/araduino.git

sudo apt-get install python-pip

pip install ephem

//sudo apt-get install libblas-dev liblapack-dev libatlas-base-dev gfortran

//sudo pip install numpy

sudo apt-get install build-essential python-dev python-distlib python-setuptools python-pip python-wheel libzmq-dev libgdal-dev

sudo apt-get install xsel xclip libxml2-dev libxslt-dev python-lxml python-h5py python-numexpr python-dateutil python-six python-tz python-bs4 python-html5lib python-openpyxl python-tables python-xlrd python-xlwt cython python-sqlalchemy python-xlsxwriter python-jinja2 python-boto python-gflags python-googleapi python-httplib2 python-zmq libspatialindex-dev

sudo pip install bottleneck rtree

sudo apt-get install python-numpy python-matplotlib python-mpltoolkits.basemap python-scipy python-sklearn python-statsmodels python-pandas

//pip install scipy

// Maybe not needed?
sudo apt-get install libasound2-dev
pip install pyalsaaudio

sudo apt-get install python-pygame

sudo raspi-config
Advanced / Audio / Set headphone

cd ~/src/araduino
python araduino.py -d

// Set the volume
alsamixer
sudo alsactl store

// Install Pirate Audio: https://learn.pimoroni.com/tutorial/sandyj/getting-started-with-pirate-audio
git clone https://github.com/pimoroni/pirate-audio
cd pirate-audio/mopidy
sudo ./install.sh

// For Pirate Audio setup, install pyaudio and configure python musical to use that instead of pygame
sudo apt-get install python-pyaudio

And reduce the volume in araduino.py to something like 0.05

And turn off mopidy audio player that comes with the Pirate Audio speaker
sudo systemctl disable mopidy
