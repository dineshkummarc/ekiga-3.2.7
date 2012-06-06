noinst_LTLIBRARIES = libgmplatform.la

INCLUDES = -I$(top_srcdir)/lib \
           $(EKIGA_LIB_CFLAGS)

libgmplatform_la_SOURCES = gm-platform.h

if WIN32
libgmplatform_la_SOURCES += platform-win32.c winpaths.h
else
libgmplatform_la_SOURCES += platform-generic.c
endif

AM_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS)
AM_LIBS = $(GTK_LIBS) $(GLIB_LIBS)

libgmplatform_la_LDFLAGS = -export-dynamic -no-undefined $(AM_LIBS)
