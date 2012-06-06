noinst_LTLIBRARIES = libgmaudiooutput.la

audiooutput_dir = $(top_srcdir)/lib/engine/audiooutput

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/audiooutput \
	-I$(top_srcdir)/lib/engine/hal


libgmaudiooutput_la_SOURCES = \
	$(audiooutput_dir)/audiooutput-manager.h       \
	$(audiooutput_dir)/audiooutput-info.h	       \
	$(audiooutput_dir)/audiooutput-scheduler.h     \
	$(audiooutput_dir)/audiooutput-scheduler.cpp   \
	$(audiooutput_dir)/audiooutput-core.h	       \
	$(audiooutput_dir)/audiooutput-core.cpp        \
	$(audiooutput_dir)/audiooutput-gmconf-bridge.h \
	$(audiooutput_dir)/audiooutput-gmconf-bridge.cpp

libgmaudiooutput_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(PTLIB_LIBS)
