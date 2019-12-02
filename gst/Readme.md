#### Requirements

* [Gstreamer 1.0](https://gstreamer.freedesktop.org/src/)
* [Netstat](http://sourceforge.net/projects/net-tools/)
* gstreamer-tools ( or gstreamer-utils )

#### Multifrontend
##### GstDvbSrc & multifrontend

* make dvb
* sudo make dvb-install


#### Scrambling
##### Dvbcsa or Client-oscam

##### Gstreamer version <= 1.14.5

* make demux-patch

##### 1. GstTSDemux & Dvbcsa

* make demuxcsa
* sudo make demux-install
* sudo mkdir /etc/helia/; sudo cp softcam.key /etc/helia/oscam.keys

##### 2. GstTSDemux & Client-oscam ( client development paused )

* make demuxoscam
* sudo make demux-install


##### Oscam conf
