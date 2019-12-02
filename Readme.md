## [Helia](https://github.com/vl-nix/Helia)

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


#### Build ( variants )

##### 1. Meson & Ninja

* make clean
* make meson prefix=/usr
* ninja -C build install ( or sudo ninja -C build install )

##### 2. Makefile

* make help
* make clean
* make
* make install ( or sudo make install )

##### 3. Autogen

* sh autogen.sh
* sh configure ( or sh configure --prefix=PREFIX  )
* make
* make install ( or sudo make install )

##### Gettext

* Note:
  * By default, Gettext is not used. Translation into other languages is included in the program.
* If you want to use Gettext:
  * git clone https://github.com/vl-nix/Helia.git
  * cd Helia
  * Build: make GETTEXT=ENABLE
  * Install: sudo make GETTEXT=ENABLE install
  * Uninstall: sudo make GETTEXT=ENABLE uninstall


#### [Preview](https://www.pling.com/p/1312498/)
