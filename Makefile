# Helia
#
# Clone
# git clone git@github.com:vl-nix/helia.git
#
# Showing debug:
#  G_MESSAGES_DEBUG=all build/helia

program		= helia
version		= 12.0

CC		= gcc
COMMON		= -Wall -Wextra -Wpedantic

DEFS		= -DPACKAGE=\"$(program)\" -DVERSION=\"$(version)\" -DGETTEXT_PACKAGE=\"$(program)\"

INCLUDES	= -Iinclude
OPTIMIZE	= -O2

LIBS_PKG	= gtk+-3.0 gstreamer-video-1.0 gstreamer-mpegts-1.0

CFLADD		= $(COMMON) $(DEFS) $(INCLUDES) $(OPTIMIZE)
CFLAGS		= $(CFLADD) $(shell pkg-config --cflags $(LIBS_PKG))

LDADD		= -lm
LDLIBS		= $(shell pkg-config --libs $(LIBS_PKG)) $(LDADD)

prefix		= /usr

bindir		= $(prefix)/bin
datadir		= $(prefix)/share
desktopdir	= $(datadir)/applications

sourcedir	= src
builddir	= build
buildsrcdir	= $(builddir)/src

sources		= $(wildcard $(sourcedir)/*.c)
objects		= $(patsubst $(sourcedir)/%.c,$(buildsrcdir)/%.o,$(sources))

xres		= $(builddir)/gresource.xml
gres		= $(patsubst $(builddir)/%.xml,$(buildsrcdir)/%.c,$(xres))
obj_res		= $(patsubst $(buildsrcdir)/%.c,$(buildsrcdir)/%.o,$(gres))

desktop		= $(builddir)/$(program).desktop
binary		= $(builddir)/$(program)


all: dirs $(desktop) $(binary) lang

dirs:
	@echo
	@mkdir -p $(builddir) $(buildsrcdir) $(buildatadir)

$(desktop):
	@sh data/desktop.sh build/helia.desktop $(bindir)

$(xres):
	@sh data/gen-gres.sh $@

$(gres): $(buildsrcdir)/%.c : $(builddir)/%.xml
	@glib-compile-resources $< --target=$@ --generate-source

$(binary): $(obj_res) $(objects) 
	@echo
	@echo '  CCLD	' $@  '( Ver. $(version) )'
	@$(CC) $^ -o $@ $(LDLIBS)
	@echo

$(objects): $(buildsrcdir)/%.o : $(sourcedir)/%.c
	@echo '  CC	' $<
	@$(CC) $(DEFS) $(CFLAGS) -c $< -o $@

$(obj_res): $(buildsrcdir)/%.o : $(buildsrcdir)/%.c
	@echo '  CC	' $<
	@$(CC) $(CFLAGS) -c $< -o $@

lang:
	@echo '  Gettext'
	@for language in po/*.po; do \
		lang=`basename $$language | cut -f 1 -d '.'`; echo "  Lang: " $$lang; \
		mkdir -p $(builddir)/locale/$$lang/LC_MESSAGES/; \
		msgfmt $$language -o $(builddir)/locale/$$lang/LC_MESSAGES/$(program).mo; \
	done
	@echo

clean:
	rm -fr $(builddir) po/*.po~ po/*.pot

strip: 
	strip $(builddir)/$(program)

install: strip
	mkdir -p $(DESTDIR)$(bindir) $(DESTDIR)$(datadir) $(DESTDIR)$(desktopdir)
	install -Dp -m0755 $(builddir)/$(program) $(DESTDIR)$(bindir)/$(program)
	install -Dp -m0644 $(builddir)/$(program).desktop $(DESTDIR)$(desktopdir)/$(program).desktop
	cp -r $(builddir)/locale $(DESTDIR)$(datadir)

uninstall:
	rm -f $(DESTDIR)$(bindir)/$(program)
	rm -f $(DESTDIR)$(desktopdir)/$(program).desktop
	rm -fr $(DESTDIR)$(datadir)/locale/*/*/$(program).mo
