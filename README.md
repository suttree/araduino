# Ara

The robot songbird  who generates her own birdsong, and sings for you every day.

Ara is a unique and passive experience. Discover companionship and digital art in an uniquely affecting way by living together with my first soulful object that has an inherent need to sing. As featured in Creative Review (https://www.creativereview.co.uk/know-caged-bird-sings/).

![Image of Ara](https://github.com/suttree/araduino/blob/master/ara.png?raw=true)

See duncangough.com/portfolio for more.


### todo / notes / misc

- crontab info
- git pull daily
- change the waveform to sound more like a whistle and less like a guitar

- to increase volume on latest versions of Ubuntu:
alsamixer
sudo alsactl store


## Make sure headphone output is selected (not HDMI), in /boot/config
## Enable audio (loads snd_bcm2835)
dtparam=audio=on
