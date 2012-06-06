noinst_LTLIBRARIES = libgmhal-dbus.la

hal_dir = $(top_srcdir)/lib/engine/components/hal-dbus
hal_builddir = $(top_builddir)/lib/engine/components/hal-dbus

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(DBUS_CFLAGS) $(PTLIB_CFLAGS)

AM_CFLAGS = $(GLIB_CFLAGS) $(DBUS_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/hal \
	-I$(top_srcdir)/lib/engine/hal/dbus

libgmhal_dbus_la_SOURCES = \
	$(hal_dir)/hal-manager-dbus.h \
	$(hal_dir)/hal-manager-dbus.cpp \
	$(hal_dir)/hal-main-dbus.h \
	$(hal_dir)/hal-main-dbus.cpp

nodist_libgmhal_dbus_la_SOURCES = \
	$(hal_builddir)/hal-marshal.h \
	$(hal_builddir)/hal-marshal.c

if HAVE_V4L
libgmhal_dbus_la_SOURCES += \
	$(hal_dir)/hal-v4l-helper.h \
	$(hal_dir)/hal-v4l-helper.c
endif

EXTRA_DIST = hal-marshal.list

# forces the HAVE :
BUILT_SOURCES = hal-marshal.h hal-marshal.c

CLEANFILES = $(BUILT_SOURCES)

libgmhal_dbus_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(DBUS_LIBS) $(PTLIB_LIBS)

hal-marshal.h: hal-marshal.list
	$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_hal_dbus_marshal $^ --header > $@.tmp && mv $@.tmp $@
    
hal-marshal.c: hal-marshal.list
	$(LIBTOOL) --mode=execute glib-genmarshal --prefix=gm_hal_dbus_marshal $^ --body > $@.tmp && mv $@.tmp $@
