noinst_LTLIBRARIES = libgmevolution.la

evolution_dir = $(top_srcdir)/lib/engine/components/evolution

AM_CXXFLAGS = $(SIGC_CFLAGS) $(EDS_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/addressbook

libgmevolution_la_SOURCES = \
	$(evolution_dir)/evolution-contact.h \
	$(evolution_dir)/evolution-contact.cpp \
	$(evolution_dir)/evolution-book.h \
	$(evolution_dir)/evolution-book.cpp \
	$(evolution_dir)/evolution-source.h \
	$(evolution_dir)/evolution-source.cpp \
	$(evolution_dir)/evolution-main.h \
	$(evolution_dir)/evolution-main.cpp

libgmevolution_la_LIBADD = \
	$(top_builddir)/lib/engine/addressbook/libgmaddressbook.la

libgmevolution_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(EDS_LIBS) $(GLIB_LIBS)
