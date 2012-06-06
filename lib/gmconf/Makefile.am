noinst_LTLIBRARIES = libgmconf.la

libgmconf_la_SOURCES = 	gmconf.h

INCLUDES = -DSYSCONFDIR=\""$(sysconfdir)"\"	\
	   -I$(top_srcdir)/lib


if HAVE_GCONF
libgmconf_la_SOURCES +=	gmconf-gconf.c
else
libgmconf_la_SOURCES += gmconf-glib.c 
endif

AM_CFLAGS = $(GCONF_CFLAGS) $(GLIB_CFLAGS)
AM_LIBS = $(GCONF_LIBS) $(GCONF_LIBS)

libgmconf_la_LDFLAGS = -export-dynamic $(AM_LIBS)

