#!/bin/sh

# sh autogen.sh


if [ -f Makefile ]; then
	echo "Make clean"
	make clean
	echo ""
	make dirs desktop gres
	rm Makefile
fi

#######################################################################################################################

gen_conf_ac ()
{

echo "AC_PREREQ([2.69])
AC_INIT([$1],[$2])

AM_SILENT_RULES([yes])
AM_INIT_AUTOMAKE([1.11 foreign subdir-objects tar-ustar no-dist-gzip dist-xz -Wno-portability])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_SED
PKG_PROG_PKG_CONFIG([0.22])

PKG_CHECK_MODULES(HELIA, [glib-2.0 gdk-pixbuf-2.0 gtk+-3.0 gstreamer-1.0 gstreamer-plugins-base-1.0 gstreamer-plugins-bad-1.0 gstreamer-video-1.0 gstreamer-mpegts-1.0])

AC_CONFIG_FILES([
	Makefile

	src/Makefile
])

AC_OUTPUT
echo \"\"
echo \"  \${PACKAGE} - \${VERSION}\"
echo \"\"
echo \"  Prefix ........ : \${prefix}\"
echo \"\"" > configure.ac

}

gen_make_am ()
{

echo "SUBDIRS = src

desktopdir = \$(datadir)/applications
desktop_DATA = helia.desktop

helia.desktop: build/helia.desktop
	@echo \"Gen desktop	\"  \$@
	@sed 's|Exec=/.*bin|Exec=\$(bindir)|g' $< > \$@" > Makefile.am

}

gen_make_am_src ()
{

cd  `pwd`/$1
echo "bin_PROGRAMS = helia" > Makefile.am

for file in "" "helia_CFLAGS = -I../include \$(HELIA_CFLAGS)" "helia_LDADD  = \$(HELIA_LIBS)" "" "helia_SOURCES  = \\"
do	
	echo "$file" >> Makefile.am
done
	
for file in `ls *.c`
do	
	echo "    $file \\" >> Makefile.am
done

echo "    ../build/data/gresource.c" >> Makefile.am
echo "" >> Makefile.am

cd ../

}

#######################################################################################################################

gen_conf_ac helia 9.9
gen_make_am
gen_make_am_src src

autoreconf --verbose --force --install || exit 1

echo "Now configure process."
