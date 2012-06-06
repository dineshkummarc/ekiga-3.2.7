noinst_LTLIBRARIES = libplugin.la

plugin_dir = $(top_srcdir)/lib/engine/plugin

AM_CPPFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf	 		\
	-I$(top_srcdir)/lib/engine/framework

libplugin_la_SOURCES = \
	$(plugin_dir)/plugin-core.h			\
	$(plugin_dir)/plugin-core.cpp

libplugin_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
