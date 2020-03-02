library		= libdvbcsasys

CC		= gcc
COMMON		= -Wall -Wextra -Wpedantic

LIBS_PKG	= glib-2.0

CFLADD		= $(COMMON)
CFLAGS		= $(CFLADD) $(shell pkg-config --cflags $(LIBS_PKG))

LDADD		= -lm
LDLIBS		= $(shell pkg-config --libs $(LIBS_PKG)) $(LDADD)

sourcedir	= src
builddir	= build
buildsrcdir	= $(builddir)/src

sources		= $(wildcard $(sourcedir)/*.c)
objects		= $(patsubst $(sourcedir)/%.c,$(buildsrcdir)/%.o,$(sources))

libshared	= $(builddir)/$(library).so
libstatic	= $(builddir)/$(library).a


all: dirs $(libstatic)

dirs:
	@mkdir -p $(builddir) $(buildsrcdir)

$(libstatic): $(objects)
	@echo '  CCLD  static  ' $@
	@ar rsc $@ $^

$(libshared): $(objects)
	@echo '  CCLD  shared  ' $@
	@$(CC) -shared $^ -o $@ $(LDLIBS)

$(objects): $(buildsrcdir)/%.o : $(sourcedir)/%.c
	@echo '  CC	 ' $<
	@$(CC) -fPIC $(DEFS) $(CFLAGS) -c $< -o $@ 

clean:
	rm -fr $(builddir)

inspect:
	gst-inspect-1.0 $(libshared)
