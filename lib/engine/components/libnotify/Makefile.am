noinst_LTLIBRARIES = libgmlibnotify.la

libnotify_dir = $(top_srcdir)/lib/engine/components/libnotify

AM_CXXFLAGS = $(SIGC_CFLAGS) $(NOTIFY_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/notification

libgmlibnotify_la_SOURCES = \
	$(libnotify_dir)/libnotify-main.h \
	$(libnotify_dir)/libnotify-main.cpp

libgmlibnotify_la_LIBADD = \
	$(top_builddir)/lib/engine/framework/libgmframework.la \
	$(top_builddir)/lib/engine/notification/libnotification.la

libgmlibnotify_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(NOTIFY_LIBS)
