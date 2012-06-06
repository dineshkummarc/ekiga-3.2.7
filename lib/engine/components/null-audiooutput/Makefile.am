noinst_LTLIBRARIES = libgmaudiooutput-null.la

audiooutput_dir = $(top_srcdir)/lib/engine/components/null-audiooutput/

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/audiooutput \
	-I$(top_srcdir)/lib/engine/audiooutput/null \
	-I$(top_srcdir)/lib/engine/hal

libgmaudiooutput_null_la_SOURCES = \
	$(audiooutput_dir)/audiooutput-manager-null.h \
	$(audiooutput_dir)/audiooutput-manager-null.cpp \
	$(audiooutput_dir)/audiooutput-main-null.h \
	$(audiooutput_dir)/audiooutput-main-null.cpp 

libgmaudiooutput_null_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(PTLIB_LIBS)
