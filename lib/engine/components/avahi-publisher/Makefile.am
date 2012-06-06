noinst_LTLIBRARIES = libavahipublisher.la

avahi_publisher_dir = $(top_srcdir)/lib/engine/components/avahi-publisher/

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(AVAHI_CFLAGS)

INCLUDES = 						\
	-I$(top_srcdir)/lib/gmconf	 		\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/protocol 	\
	-I$(top_srcdir)/lib/engine/presence 	\
	-I$(top_srcdir)/lib/engine/account 

libavahipublisher_la_SOURCES = 						\
	$(avahi_publisher_dir)/avahi-publisher-main.cpp			\
	$(avahi_publisher_dir)/avahi-publisher-main.h			\
	$(avahi_publisher_dir)/avahi-publisher.cpp			\
	$(avahi_publisher_dir)/avahi-publisher.h

libavahipublisher_la_LIBADD = 						\
	$(top_builddir)/lib/engine/account/libaccount.la		\
	$(top_builddir)/lib/engine/framework/libgmframework.la		\
	$(top_builddir)/lib/engine/presence/libgmpresence.la

libavahipublisher_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(AVAHI_LIBS)
