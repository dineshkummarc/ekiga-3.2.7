noinst_LTLIBRARIES = libnotification.la

notification_dir = $(top_srcdir)/lib/engine/notification

AM_CPPFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf \
	-I$(top_srcdir)/lib/engine/framework

libnotification_la_SOURCES = \
	$(notification_dir)/notification-core.h \
	$(notification_dir)/notification-dummy.cpp

libnotification_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
