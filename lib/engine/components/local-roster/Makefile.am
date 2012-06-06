noinst_LTLIBRARIES = liblocal-roster.la

local_roster_dir = $(top_srcdir)/lib/engine/components/local-roster

AM_CPPFLAGS = $(XML_CFLAGS) $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf	 		\
	-I$(top_srcdir)/lib/engine/framework 		\
	-I$(top_srcdir)/lib/engine/addressbook	\
	-I$(top_srcdir)/lib/engine/account		\
	-I$(top_srcdir)/lib/engine/presence

liblocal_roster_la_SOURCES = \
	$(local_roster_dir)/local-presentity.h \
	$(local_roster_dir)/local-presentity.cpp \
	$(local_roster_dir)/local-heap.h \
	$(local_roster_dir)/local-heap.cpp \
	$(local_roster_dir)/local-cluster.h \
	$(local_roster_dir)/local-cluster.cpp \
	$(local_roster_dir)/local-roster-main.h \
	$(local_roster_dir)/local-roster-main.cpp \
	$(local_roster_dir)/local-roster-bridge.h \
	$(local_roster_dir)/local-roster-bridge.cpp

liblocal_roster_la_LIBADD = \
	$(top_builddir)/lib/engine/addressbook/libgmaddressbook.la \
	$(top_builddir)/lib/engine/presence/libgmpresence.la

liblocal_roster_la_LDFLAGS = -export-dynamic -no-undefined $(XML_LIBS) $(SIGC_LIBS) $(GLIB_LIBS)
