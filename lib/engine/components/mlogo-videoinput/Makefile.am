noinst_LTLIBRARIES = libgmvideoinput-mlogo.la

videoinput_dir = $(top_srcdir)/lib/engine/components/mlogo-videoinput

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/pixmaps \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/videooutput \
	-I$(top_srcdir)/lib/engine/videoinput \
	-I$(top_srcdir)/lib/engine/videoinput/mlogo \
	-I$(top_srcdir)/lib/engine/hal

libgmvideoinput_mlogo_la_SOURCES = \
	$(videoinput_dir)/videoinput-manager-mlogo.h \
	$(videoinput_dir)/videoinput-manager-mlogo.cpp \
	$(videoinput_dir)/videoinput-main-mlogo.h \
	$(videoinput_dir)/videoinput-main-mlogo.cpp 

libgmvideoinput_mlogo_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(PTLIB_LIBS)
