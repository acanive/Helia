library		= libgstmpegtsdemux

CC		= gcc
COMMON		= -Wall

INCLUDES	= -I../include
OPTIMIZE	= -O2
# DEBUG		= -g -ggdb

DEFS		= -DPACKAGE=\"gst-plugins-bad\" -DVERSION=\"1.16.0\" -DGETTEXT_PACKAGE=\"gst-plugins-bad-1.0\" -DGST_USE_UNSTABLE_API

LIBS_PKG	= gstreamer-1.0

CFLADD		= $(COMMON) $(INCLUDES) $(OPTIMIZE) $(DEBUG)
CFLAGS		= $(CFLADD) $(shell pkg-config --cflags $(LIBS_PKG))

LDADD		= -lgstreamer-1.0 -lgstbase-1.0 -lgstmpegts-1.0 -lgstcodecparsers-1.0 -lgsttag-1.0 -lgstpbutils-1.0 -lgstaudio-1.0 -lm
LDLIBS		= $(shell pkg-config --libs $(LIBS_PKG)) $(LDADD)

sourcedir	= src
builddir	= build
buildsrcdir	= $(builddir)/src

sources		= $(wildcard *.c)
objects		= $(patsubst %.c,$(buildsrcdir)/%.o,$(sources))

libshared	= $(builddir)/$(library).so
pathinstall 	= $(shell locate $(library).so | grep /usr)


ifeq "$(DECRYPT)" "OSCAM"
DEFS += -DUSE_OSCAM
LDLIBS += ../client-oscam/build/client-oscam.a
endif

ifeq "$(DECRYPT)" "CSASYS"
DEFS += -DUSE_CSASYS
LDLIBS += ../csasys/build/libdvbcsasys.a
endif


all: dirs $(libshared)

dirs:
	@mkdir -p $(builddir) $(buildsrcdir)

ver-patch:
	patch -p 1 < demux.diff

$(libshared): $(objects)
	@echo '  CCLD  shared  ' $@
	@$(CC) -shared $^ -o $@ $(LDLIBS)

$(objects): $(buildsrcdir)/%.o : %.c
	@echo '  CC	 ' $<
	@$(CC) -fPIC $(DEFS) $(CFLAGS) -c $< -o $@ 

clean:
	rm -fr $(builddir)

install:
	install -Dp -m0755 $(libshared) $(pathinstall)

inspect:
	gst-inspect-1.0 $(libshared)
