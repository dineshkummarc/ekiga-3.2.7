if WIN32
lib_LTLIBRARIES = libgmkab.la
else
noinst_LTLIBRARIES = libgmkab.la
endif

kab_dir = $(top_srcdir)/lib/engine/components/kab

AM_CPPFLAGS = $(SIGC_CFLAGS) $(KAB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/addressbook

libgmkab_la_SOURCES = \
	$(kab_dir)/kab-contact.h \
	$(kab_dir)/kab-contact.cpp \
	$(kab_dir)/kab-book.h \
	$(kab_dir)/kab-book.cpp \
	$(kab_dir)/kab-source.h \
	$(kab_dir)/kab-source.cpp \
	$(kab_dir)/kab-main.h \
	$(kab_dir)/kab-main.cpp

libgmkab_la_LIBADD = \
	$(top_builddir)/lib/engine/addressbook/libgmaddressbook.la

libgmkab_la_LDFLAGS = -export-dynamic -no-undefined $(KAB_LIBS)
