noinst_LTLIBRARIES = libgmptlib.la

ptlib_dir = $(top_srcdir)/lib/engine/components/ptlib

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf                      \
	-I$(top_srcdir)/lib/engine/framework            \
	-I$(top_srcdir)/lib/engine/audiooutput     \
	-I$(top_srcdir)/lib/engine/audioinput      \
	-I$(top_srcdir)/lib/engine/videoinput      \
	-I$(top_srcdir)/lib/engine/videooutput     \
	-I$(top_srcdir)/lib/engine/components/ptlib     \
	-I$(top_srcdir)/lib/engine/hal

libgmptlib_la_SOURCES = \
	$(ptlib_dir)/utils.h				\
	$(ptlib_dir)/utils.cpp				\
	$(ptlib_dir)/audioinput-manager-ptlib.h         \
	$(ptlib_dir)/audioinput-manager-ptlib.cpp       \
	$(ptlib_dir)/audioinput-main-ptlib.h            \
	$(ptlib_dir)/audioinput-main-ptlib.cpp          \
	$(ptlib_dir)/videoinput-manager-ptlib.h         \
	$(ptlib_dir)/videoinput-manager-ptlib.cpp       \
	$(ptlib_dir)/videoinput-main-ptlib.h            \
	$(ptlib_dir)/videoinput-main-ptlib.cpp          \
	$(ptlib_dir)/audiooutput-manager-ptlib.h        \
	$(ptlib_dir)/audiooutput-manager-ptlib.cpp      \
	$(ptlib_dir)/audiooutput-main-ptlib.h           \
	$(ptlib_dir)/audiooutput-main-ptlib.cpp 

libgmptlib_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(PTLIB_LIBS)
