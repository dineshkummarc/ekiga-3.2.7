noinst_LTLIBRARIES = libtoolbox.la

libtoolbox_la_SOURCES = \
	toolbox.h		\
	toolbox-common.c	\
	toolbox-internal.h	\
	toolbox-internal.c

if WIN32
libtoolbox_la_SOURCES += toolbox-win32.c
else
if HAVE_GNOME
libtoolbox_la_SOURCES += toolbox-gnome.c
else
libtoolbox_la_SOURCES += toolbox-gtk.c
endif
endif

INCLUDES = $(EKIGA_LIB_CFLAGS)

AM_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(GNOME_CFLAGS)
AM_LIBS = $(GTK_LIBS) $(GLIB_LIBS) $(GNOME_LIBS)

libtoolbox_la_LDFLAGS = -export-dynamic $(AM_LIBS)
