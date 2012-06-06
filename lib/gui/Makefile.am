SUBDIRS = 

noinst_LTLIBRARIES = libgmwidgets.la

gui_dir = $(top_srcdir)/lib/gui

libgmwidgets_la_SOURCES =		\
	$(gui_dir)/gm-smileys.h		\
	$(gui_dir)/gm-smileys.c		\
	$(gui_dir)/gmwindow.c		\
	$(gui_dir)/gmwindow.h		\
	$(gui_dir)/gmconfwidgets.c	\
	$(gui_dir)/gmconfwidgets.h	\
	$(gui_dir)/codecsbox.cpp	\
	$(gui_dir)/codecsbox.h		\
	$(gui_dir)/gmconnectbutton.c	\
	$(gui_dir)/gmconnectbutton.h	\
	$(gui_dir)/gmdialog.c		\
	$(gui_dir)/gmdialog.h		\
	$(gui_dir)/gmentrydialog.c	\
	$(gui_dir)/gmentrydialog.h	\
	$(gui_dir)/gmlevelmeter.c	\
	$(gui_dir)/gmlevelmeter.h	\
	$(gui_dir)/gmmenuaddon.c	\
	$(gui_dir)/gmmenuaddon.h	\
	$(gui_dir)/gmpreferences.c	\
	$(gui_dir)/gmpreferences.h	\
	$(gui_dir)/gmstatusbar.c	\
	$(gui_dir)/gmstatusbar.h	\
	$(gui_dir)/gmstockicons.c	\
	$(gui_dir)/gmstockicons.h	\
	$(gui_dir)/gmpowermeter.c	\
	$(gui_dir)/gmpowermeter.h	\
	$(gui_dir)/gmcellrendererexpander.c  \
	$(gui_dir)/gmcellrendererexpander.h  \
	$(gui_dir)/gm-cell-renderer-bitext.c \
	$(gui_dir)/gm-cell-renderer-bitext.h \
	$(gui_dir)/gm-text-buffer-enhancer-helper-iface.c \
	$(gui_dir)/gm-text-buffer-enhancer-helper-iface.h \
	$(gui_dir)/gm-text-anchored-tag.c \
	$(gui_dir)/gm-text-anchored-tag.h \
	$(gui_dir)/gm-text-smiley.c \
	$(gui_dir)/gm-text-smiley.h \
	$(gui_dir)/gm-text-extlink.c \
	$(gui_dir)/gm-text-extlink.h \
	$(gui_dir)/gm-text-buffer-enhancer.c \
	$(gui_dir)/gm-text-buffer-enhancer.h \
	$(gui_dir)/gm-smiley-chooser-button.c \
	$(gui_dir)/gm-smiley-chooser-button.h


if !WIN32
libgmwidgets_la_SOURCES +=	\
	$(gui_dir)/xwindow.cpp		\
	$(gui_dir)/xwindow.h
endif

if HAVE_XV
libgmwidgets_la_SOURCES +=	\
	$(gui_dir)/xvwindow.cpp		\
	$(gui_dir)/xvwindow.h
endif

if HAVE_DX
libgmwidgets_la_SOURCES +=	\
	$(gui_dir)/dxwindow.cpp		\
	$(gui_dir)/dxwindow.h
endif

INCLUDES =						\
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/protocol	\
	-I$(top_srcdir)					\
	-I$(top_builddir)

if !WIN32
INCLUDES += -I$(top_srcdir)/lib/pixops
endif

AM_CFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(GNOME_CFLAGS) -std=c99
AM_CXXFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS) $(XV_CFLAGS) $(DX_CFLAGS) $(SIGC_CFLAGS)
AM_LIBS = $(GTK_LIBS) $(GLIB_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS) $(XV_LIBS) $(DX_LIBS) $(GNOME_LIBS) $(SIGC_LIBS)

libgmwidgets_la_LDFLAGS = -export-dynamic
if HAVE_SHM
libgmwidgets_la_LDFLAGS += $(SHM_LIBS)
endif
libgmwidgets_la_LIBADD = $(AM_LIBS)
