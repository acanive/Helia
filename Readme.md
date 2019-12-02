## [Helia](https://www.opencode.net/vl-nix/helia)

* Digital TV
  * DVB-T2/S2/C, ATSC, DTMB, ISDB
  * [Multifrontend](gst#multifrontend)
  * [Scrambling](gst#scrambling)
* Media Player
  * Drag and Drop
    * files, folders, playlists - [M3U](https://en.wikipedia.org/wiki/M3U)
  * Record IPTV - M3U8


#### Requirements

* Graphical user interface - [Gtk+3](https://developer.gnome.org/gtk3)
* Audio & Video & Digital TV - [Gstreamer 1.0](https://gstreamer.freedesktop.org)
* [GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html)


#### Depends

* libgtk 3.0 ( & dev )
* gstreamer 1.0 ( & dev )
* all gst-plugins 1.0 ( & dev )
* gst-libav
* gstreamer-tools ( or gstreamer-utils )
* Initial scan
  * [dvbv5-scan](https://linuxtv.org/downloads/v4l-utils/)
  * [dtv-scan-tables](https://git.linuxtv.org/dtv-scan-tables.git/)
  * [dialog](http://invisible-island.net/dialog/dialog.html)


#### Build ( variants )

#### Gettext

* By default, Gettext is not used. Translation into other languages is included in the program.

##### 1. Meson & Ninja

* meson build --prefix /usr --strip
  * Using the Gettext: meson build --prefix /usr --strip -D enable-gettext=true
* ninja -C build
* sudo ninja -C build install

##### 2. Makefile

* make help
* make
  * Using the Gettext: make gettext=true
* sudo make install

##### 3. Autogen

* sh autogen.sh && sh configure --prefix=/usr && make


#### [Preview](https://www.pling.com/p/1312498/)
