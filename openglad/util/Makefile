# Makefile.in generated automatically by automake 1.5 from Makefile.am.

# Copyright 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001
# Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.



SHELL = /bin/sh

srcdir = .
top_srcdir = ..

prefix = /usr/local
exec_prefix = /usr/local

bindir = /usr/local/games
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = /usr/local/share/games/openglad
sysconfdir = /usr/local/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include
pkgdatadir = $(datadir)/openglad
pkglibdir = $(libdir)/openglad
pkgincludedir = $(includedir)/openglad
top_builddir = ..

ACLOCAL = ${SHELL} /home/sark/Desktop/android_project/openglad/jni/openglad/missing --run aclocal
AUTOCONF = ${SHELL} /home/sark/Desktop/android_project/openglad/jni/openglad/missing --run autoconf
AUTOMAKE = ${SHELL} /home/sark/Desktop/android_project/openglad/jni/openglad/missing --run automake
AUTOHEADER = ${SHELL} /home/sark/Desktop/android_project/openglad/jni/openglad/missing --run autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}
INSTALL_HEADER = $(INSTALL_DATA)
transform = s,x,x,
NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
AMTAR = ${SHELL} /home/sark/Desktop/android_project/openglad/jni/openglad/missing --run tar
AWK = gawk
CC = gcc
CXX = g++
DEPDIR = .deps
EXEEXT = 
INSTALL_STRIP_PROGRAM = ${SHELL} $(install_sh) -c -s
OBJEXT = o
PACKAGE = openglad
SDL_CFLAGS = -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
SDL_CONFIG = /usr/bin/sdl-config
SDL_LIBS = -lSDL -lpthread
VERSION = 0.98
am__include = include
am__quote = 
install_sh = /home/sark/Desktop/android_project/openglad/jni/openglad/install-sh

EXTRA_PROGRAMS = getcolors pixieread
noinst_PROGRAMS = gladpack

getcolors_SOURCES = getcolors.c
gladpack_SOURCES = gladpack.c
pixieread_SOURCES = pixieread.c

# At least on my system gladpack gets messed up with spurious libraries, wierd
LIBS = 
subdir = util
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = $(top_builddir)/src/config.h
CONFIG_CLEAN_FILES =
EXTRA_PROGRAMS = getcolors$(EXEEXT) pixieread$(EXEEXT)
noinst_PROGRAMS = gladpack$(EXEEXT)
PROGRAMS = $(noinst_PROGRAMS)

am_getcolors_OBJECTS = getcolors.$(OBJEXT)
getcolors_OBJECTS = $(am_getcolors_OBJECTS)
getcolors_LDADD = $(LDADD)
getcolors_DEPENDENCIES =
getcolors_LDFLAGS =
am_gladpack_OBJECTS = gladpack.$(OBJEXT)
gladpack_OBJECTS = $(am_gladpack_OBJECTS)
gladpack_LDADD = $(LDADD)
gladpack_DEPENDENCIES =
gladpack_LDFLAGS =
am_pixieread_OBJECTS = pixieread.$(OBJEXT)
pixieread_OBJECTS = $(am_pixieread_OBJECTS)
pixieread_LDADD = $(LDADD)
pixieread_DEPENDENCIES =
pixieread_LDFLAGS =

DEFS = -DHAVE_CONFIG_H
DEFAULT_INCLUDES =  -I. -I$(srcdir) -I$(top_builddir)/src
CPPFLAGS = 
LDFLAGS = 
depcomp = $(SHELL) $(top_srcdir)/depcomp
DEP_FILES = $(DEPDIR)/getcolors.Po $(DEPDIR)/gladpack.Po \
	$(DEPDIR)/pixieread.Po
COMPILE = $(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@
CFLAGS = -g -O2
DIST_SOURCES = $(getcolors_SOURCES) $(gladpack_SOURCES) \
	$(pixieread_SOURCES)
DIST_COMMON = Makefile.am Makefile.in
SOURCES = $(getcolors_SOURCES) $(gladpack_SOURCES) $(pixieread_SOURCES)

all: all-am

.SUFFIXES:
.SUFFIXES: .c .o .obj
$(srcdir)/Makefile.in:  Makefile.am  $(top_srcdir)/configure.ac $(ACLOCAL_M4)
	cd $(top_srcdir) && \
	  $(AUTOMAKE) --foreign  util/Makefile
Makefile:  $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) && \
	  CONFIG_HEADERS= CONFIG_LINKS= \
	  CONFIG_FILES=$(subdir)/$@ $(SHELL) ./config.status

clean-noinstPROGRAMS:
	-test -z "$(noinst_PROGRAMS)" || rm -f $(noinst_PROGRAMS)
getcolors$(EXEEXT): $(getcolors_OBJECTS) $(getcolors_DEPENDENCIES) 
	@rm -f getcolors$(EXEEXT)
	$(LINK) $(getcolors_LDFLAGS) $(getcolors_OBJECTS) $(getcolors_LDADD) $(LIBS)
gladpack$(EXEEXT): $(gladpack_OBJECTS) $(gladpack_DEPENDENCIES) 
	@rm -f gladpack$(EXEEXT)
	$(LINK) $(gladpack_LDFLAGS) $(gladpack_OBJECTS) $(gladpack_LDADD) $(LIBS)
pixieread$(EXEEXT): $(pixieread_OBJECTS) $(pixieread_DEPENDENCIES) 
	@rm -f pixieread$(EXEEXT)
	$(LINK) $(pixieread_LDFLAGS) $(pixieread_OBJECTS) $(pixieread_LDADD) $(LIBS)

mostlyclean-compile:
	-rm -f *.$(OBJEXT) core *.core

distclean-compile:
	-rm -f *.tab.c

include $(DEPDIR)/getcolors.Po
include $(DEPDIR)/gladpack.Po
include $(DEPDIR)/pixieread.Po

distclean-depend:
	-rm -rf $(DEPDIR)

.c.o:
	source='$<' object='$@' libtool=no \
	depfile='$(DEPDIR)/$*.Po' tmpdepfile='$(DEPDIR)/$*.TPo' \
	$(CCDEPMODE) $(depcomp) \
	$(COMPILE) -c `test -f $< || echo '$(srcdir)/'`$<

.c.obj:
	source='$<' object='$@' libtool=no \
	depfile='$(DEPDIR)/$*.Po' tmpdepfile='$(DEPDIR)/$*.TPo' \
	$(CCDEPMODE) $(depcomp) \
	$(COMPILE) -c `cygpath -w $<`
CCDEPMODE = depmode=gcc3
uninstall-info-am:

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP) $(TAGS_FILES)
	list='$(SOURCES) $(HEADERS) $(TAGS_FILES)'; \
	unique=`for i in $$list; do \
	    if test -f "$$i"; then echo $$i; else echo $(srcdir)/$$i; fi; \
	  done | \
	  $(AWK) '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	mkid -fID $$unique $(LISP)

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) \
		$(TAGS_FILES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS) $(TAGS_FILES)'; \
	unique=`for i in $$list; do \
	    if test -f "$$i"; then echo $$i; else echo $(srcdir)/$$i; fi; \
	  done | \
	  $(AWK) '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || etags $(ETAGS_ARGS) $$tags  $$unique $(LISP)

GTAGS:
	here=`CDPATH=: && cd $(top_builddir) && pwd` \
	  && cd $(top_srcdir) \
	  && gtags -i $(GTAGS_ARGS) $$here

distclean-tags:
	-rm -f TAGS ID GTAGS GRTAGS GSYMS GPATH

DISTFILES = $(DIST_COMMON) $(DIST_SOURCES) $(TEXINFOS) $(EXTRA_DIST)

top_distdir = ..
distdir = $(top_distdir)/$(PACKAGE)-$(VERSION)

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  if test -f $$file; then d=.; else d=$(srcdir); fi; \
	  dir=`echo "$$file" | sed -e 's,/[^/]*$$,,'`; \
	  if test "$$dir" != "$$file" && test "$$dir" != "."; then \
	    $(mkinstalldirs) "$(distdir)/$$dir"; \
	  fi; \
	  if test -d $$d/$$file; then \
	    cp -pR $$d/$$file $(distdir) \
	    || exit 1; \
	  else \
	    test -f $(distdir)/$$file \
	    || cp -p $$d/$$file $(distdir)/$$file \
	    || exit 1; \
	  fi; \
	done
check-am: all-am
check: check-am
all-am: Makefile $(PROGRAMS)

installdirs:

install: install-am
install-exec: install-exec-am
install-data: install-data-am
uninstall: uninstall-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am

installcheck: installcheck-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) INSTALL_PROGRAM="$(INSTALL_STRIP_PROGRAM)" \
	  `test -z '$(STRIP)' || \
	    echo "INSTALL_PROGRAM_ENV=STRIPPROG='$(STRIP)'"` install
mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES) stamp-h stamp-h[0-9]*

maintainer-clean-generic:
	@echo "This command is intended for maintainers to use"
	@echo "it deletes files that may require special tools to rebuild."
clean: clean-am

clean-am: clean-generic clean-noinstPROGRAMS mostlyclean-am

distclean: distclean-am

distclean-am: clean-am distclean-compile distclean-depend \
	distclean-generic distclean-tags

dvi: dvi-am

dvi-am:

info: info-am

info-am:

install-data-am:

install-exec-am:

install-info: install-info-am

install-man:

installcheck-am:

maintainer-clean: maintainer-clean-am

maintainer-clean-am: distclean-am maintainer-clean-generic

mostlyclean: mostlyclean-am

mostlyclean-am: mostlyclean-compile mostlyclean-generic

uninstall-am: uninstall-info-am

.PHONY: GTAGS all all-am check check-am clean clean-generic \
	clean-noinstPROGRAMS distclean distclean-compile \
	distclean-depend distclean-generic distclean-tags distdir dvi \
	dvi-am info info-am install install-am install-data \
	install-data-am install-exec install-exec-am install-info \
	install-info-am install-man install-strip installcheck \
	installcheck-am installdirs maintainer-clean \
	maintainer-clean-generic mostlyclean mostlyclean-compile \
	mostlyclean-generic tags uninstall uninstall-am \
	uninstall-info-am

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
