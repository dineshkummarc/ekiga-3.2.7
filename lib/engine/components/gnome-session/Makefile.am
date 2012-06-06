noinst_LTLIBRARIES = libgmgnomesession.la

gnomesession_dir = $(top_srcdir)/lib/engine/components/gnome-session

AM_CPPFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(DBUS_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/protocol

libgmgnomesession_la_SOURCES = \
	$(gnomesession_dir)/gnome-session-main.h 	\
	$(gnomesession_dir)/gnome-session-main.cpp

libgmgnomesession_la_LIBADD = \
	$(top_builddir)/lib/engine/protocol/libgmprotocol.la

libgmgnomesession_la_LDFLAGS = -export-dynamic -no-undefined $(LDAP_LIBS) $(SIGC_LIBS) $(GLIB_LIBS) $(DBUS_LIBS)
