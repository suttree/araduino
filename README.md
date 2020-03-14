See araduino.tumblr.com

# todo

- crontab info
- git pull daily
- change the waveform to sound more like a whistle and less like a guitar

- to increase volume on latest versions of Ubuntu:
alsamixer
sudo alsactl store


# Make sure headphone output is selected (not HDMI), in /boot/config
# Enable audio (loads snd_bcm2835)
dtparam=audio=on
