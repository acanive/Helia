## [Helia](https://github.com/vl-nix/helia)

* Media Player
  * Drag and Drop: files, folders, playlists - [M3U](https://en.wikipedia.org/wiki/M3U)
  * Record IPTV

* Digital TV
  * Record - Ts / Encoding
  * Scan: DVB ( DVB-T/T2, DVB-S/S2, DVB-C )
  * Convert & Playing: DVB, ATSC, DTMB, ISDB
    * Convert - [DVBv5](https://www.linuxtv.org/docs/libdvbv5/index.html) ⇨ [GstDvbSrc](https://gstreamer.freedesktop.org/documentation/dvb/dvbsrc.html#dvbsrc)
  * [Multifrontend](gst#multifrontend)
  * [Scrambling](gst#scrambling)


#### Requirements

* Graphical user interface - [Gtk+3](https://developer.gnome.org/gtk3)
* Audio & Video & Digital TV - [Gstreamer 1.0](https://gstreamer.freedesktop.org)


#### Depends

* libgtk 3.0 ( & dev )
* gstreamer 1.0 ( & dev )
* all gst-plugins 1.0 ( & dev )
* gst-libav


#### Build

* meson build --prefix /usr --strip
* ninja -C build
* sudo ninja -C build install
