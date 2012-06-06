noinst_LTLIBRARIES = libgmgtk-core.la

gtk_core_dir = $(top_srcdir)/lib/engine/gui/gtk-core

AM_CPPFLAGS = $(SIGC_CFLAGS) $(GTK_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/toolbox \
	-I$(top_srcdir)/lib/gui \
	-I$(top_srcdir)/lib/engine \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/presence \
	-I$(top_srcdir)/lib/engine/addressbook

libgmgtk_core_la_SOURCES = \
	$(gtk_core_dir)/toolbar-builder-gtk.h \
	$(gtk_core_dir)/toolbar-builder-gtk.cpp \
	$(gtk_core_dir)/menu-builder-gtk.h \
	$(gtk_core_dir)/menu-builder-gtk.cpp \
	$(gtk_core_dir)/form-dialog-gtk.h \
	$(gtk_core_dir)/form-dialog-gtk.cpp \
	$(gtk_core_dir)/gtk-core.h \
	$(gtk_core_dir)/gtk-core.cpp \
	$(gtk_core_dir)/gtk-core-main.h \
	$(gtk_core_dir)/gtk-core-main.cpp

libgmgtk_core_la_LIBADD = \
	$(top_builddir)/lib/gui/libgmwidgets.la \
	$(top_builddir)/lib/engine/presence/libgmpresence.la \
	$(top_builddir)/lib/engine/addressbook/libgmaddressbook.la

libgmgtk_core_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS)
