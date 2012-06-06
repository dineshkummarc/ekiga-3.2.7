noinst_LTLIBRARIES = libgmresource_list.la

resource_list_dir = $(top_srcdir)/lib/engine/components/resource-list/

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(XML_CFLAGS)

INCLUDES = 						\
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/presence	\
	-I$(top_srcdir)/lib/engine/account		\
	-I$(top_srcdir)/lib/engine/components/xcap

libgmresource_list_la_SOURCES = \
	$(resource_list_dir)/resource-list-main.h \
	$(resource_list_dir)/resource-list-main.cpp \
	$(resource_list_dir)/rl-presentity.h \
	$(resource_list_dir)/rl-presentity.cpp \
 	$(resource_list_dir)/rl-entry.h \
 	$(resource_list_dir)/rl-entry.cpp \
 	$(resource_list_dir)/rl-entry-ref.h \
 	$(resource_list_dir)/rl-entry-ref.cpp \
 	$(resource_list_dir)/rl-list.h \
 	$(resource_list_dir)/rl-list.cpp \
	$(resource_list_dir)/rl-heap.h \
	$(resource_list_dir)/rl-heap.cpp \
	$(resource_list_dir)/rl-cluster.h \
	$(resource_list_dir)/rl-cluster.cpp

libgmresource_list_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(XML_LIBS)
