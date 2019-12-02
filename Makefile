# https://www.opencode.net/vl-nix/helia
# git clone https://www.opencode.net/vl-nix/helia.git or git clone https://github.com/vl-nix/helia.git

program		= helia
version		= 9.9

DEFS		= -DPACKAGE=\"$(program)\" -DVERSION=\"$(version)\"

ifeq "$(gettext)" "true"
	DEFS	+= -DGETTEXT_PACKAGE=\"$(program)\" -DUSE_GETTEXT 
endif

CC		= gcc
COMMON		= -Wall -Wextra -Wpedantic

INCLUDES	= -Iinclude
OPTIMIZE	= -O2
# DEBUG		= -g -ggdb

LIBS_PKG	= gtk+-3.0 gstreamer-video-1.0 gstreamer-mpegts-1.0

CFLADD		= $(COMMON) $(INCLUDES) $(OPTIMIZE) $(DEBUG)
CFLAGS		= $(CFLADD) $(shell pkg-config --cflags $(LIBS_PKG))

LDADD		= -lm
LDLIBS		= $(shell pkg-config --libs $(LIBS_PKG)) $(LDADD)

prefix		= /usr
#prefix		= $(HOME)/.local

bindir		= $(prefix)/bin
datadir		= $(prefix)/share
desktopdir	= $(datadir)/applications

sourcedir	= src
builddir	= build
buildsrcdir	= $(builddir)/src
buildatadir	= $(builddir)/data

sources		= $(wildcard $(sourcedir)/*.c)
objects		= $(patsubst $(sourcedir)/%.c,$(buildsrcdir)/%.o,$(sources))

xres		= $(buildatadir)/gresource.xml
gres		= $(patsubst $(buildatadir)/%.xml,$(buildatadir)/%.c,$(xres))
obj_res		= $(patsubst $(buildatadir)/%.c,$(buildsrcdir)/%.o,$(gres))

desktop		= $(builddir)/$(program).desktop
binary		= $(builddir)/$(program)


all: info dirs desktop $(binary) lang

info:
	@echo
	@echo ' ' $(program) - $(version)
	@echo
	@echo '  Prefix ........ :' $(prefix)
	@echo

dirs:
	@mkdir -p $(builddir) $(buildsrcdir) $(buildatadir)

xres: $(xres)
gres: $(gres)

desktop: $(desktop)

$(desktop):
	@echo "[Desktop Entry]" > $@
	@for info in "Name=Helia" "Comment=Digital TV & Media Player" "Type=Application" "Exec=$(bindir)/$(program) %U" "Icon=display" "Terminal=false" "Categories=GTK;AudioVideo;Audio;Video;Player;TV;"; do echo "$$info" >> $@; done
	@cat data/desktop-mime >> $@

$(xres):
	@echo "<gresources>" > $@; echo "  <gresource prefix=\"/$(program)\">" >> $@
	@for grfile in data/icons/*.png; do echo "    <file preprocess=\"to-pixdata\">$$grfile</file>" >> $@; done
	@echo "  </gresource>" >> $@; echo "</gresources>" >> $@

$(gres): $(buildatadir)/%.c : $(buildatadir)/%.xml
	@glib-compile-resources $< --target=$@ --generate-source

$(binary): $(obj_res) $(objects) 
	@echo
	@echo '  CCLD	' $@
	@$(CC) $^ -o $@ $(LDLIBS)
	@echo

$(objects): $(buildsrcdir)/%.o : $(sourcedir)/%.c
	@echo '  CC	' $<
	@$(CC) $(DEFS) $(CFLAGS) -c $< -o $@

$(obj_res): $(buildsrcdir)/%.o : $(buildatadir)/%.c
	@echo '  CC	' $<
	@$(CC) $(CFLAGS) -c $< -o $@

# Meson & Ninja
meson:
	meson $(builddir) --prefix $(prefix) --strip
	ninja -C $(builddir)

lang:
ifeq "$(gettext)" "true"
	$(MAKE) -s -C po program=$(program) version=$(version) builddir=$(builddir)
endif

clean:
	rm -fr $(builddir) po/*.po~ po/*.pot

strip: 
	strip $(builddir)/$(program)

install: strip
	mkdir -p $(DESTDIR)$(bindir) $(DESTDIR)$(datadir) $(DESTDIR)$(desktopdir)
	install -Dp -m0755 $(builddir)/$(program) $(DESTDIR)$(bindir)/$(program)
	install -Dp -m0644 $(builddir)/$(program).desktop $(DESTDIR)$(desktopdir)/$(program).desktop
	if [ -d $(builddir)/locale ]; then cp -r $(builddir)/locale $(DESTDIR)$(datadir); fi

uninstall:
	rm -f $(DESTDIR)$(bindir)/$(program)
	rm -f $(DESTDIR)$(desktopdir)/$(program).desktop
	rm -fr $(DESTDIR)$(datadir)/locale/*/*/$(program).mo

help:
	@echo
	@echo ' ' $(program) - $(version)
	@echo
	@echo '  Installation directories:'
	@echo '  Open the Makefile and set the prefix value'
	@echo '    prefix = PREFIX 	install files in PREFIX'
	@echo
	@echo '  Prefix ........ :' $(prefix)
	@echo '  bindir ........ :' $(bindir)
	@echo '  desktopdir .... :' $(desktopdir)
	@echo
	@echo 'Usage: make [TARGET]'
	@echo 'TARGETS:'
	@echo '  all ........... : or make'
	@echo '  help .......... : show targets'
	@echo '  install ....... : install'
	@echo '  uninstall ..... : uninstall'
	@echo '  clean ......... : clean all'
	@echo
	@echo 'Showing debug:'
	@echo '  G_MESSAGES_DEBUG=all ./$(binary)'
	@echo
