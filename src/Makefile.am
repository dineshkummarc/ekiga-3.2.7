INCLUDES = \
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib/toolbox			\
	-I$(top_srcdir)/lib/gui				\
	-I$(top_srcdir)/lib/engine/			\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/gui/gtk-frontend	\
	-I$(top_srcdir)/lib/engine/components/call-history \
	-I$(top_srcdir)/lib/engine/account		\
	-I$(top_srcdir)/lib/engine/addressbook	\
	-I$(top_srcdir)/lib/engine/chat		\
	-I$(top_srcdir)/lib/engine/presence	\
	-I$(top_srcdir)/lib/engine/protocol	\
	-I$(top_srcdir)/lib/engine/protocol/sip		\
	-I$(top_srcdir)/lib/engine/videooutput	\
	-I$(top_srcdir)/lib/engine/videoinput	\
	-I$(top_srcdir)/lib/engine/audioinput	\
	-I$(top_srcdir)/lib/engine/audiooutput	\
	-I$(top_srcdir)/lib/engine/hal		\
	-I$(top_srcdir)/lib/engine/framework		\
	-I$(top_srcdir)/lib/engine/gui/gtk-core		\
	-I$(top_srcdir)/lib/engine/components/opal	\
	-I$(top_srcdir)/src				\
	-I$(top_srcdir)/src/dbus-helper/		\
	-I$(top_srcdir)/src/gui/			\
	-I$(top_builddir)/src/dbus-helper/		\
	$(GNOME_INCLUDEDIR)

BUILT_SOURCES = src/revision.h

# XVideo/X
if !WIN32
INCLUDES +=                   				\
	-I$(top_srcdir)/lib/engine/videooutput/common   \
	-I$(top_srcdir)/lib/engine/videooutput/x
endif

# DirectX
if HAVE_DX
INCLUDES +=                   				\
	-I$(top_srcdir)/lib/engine/videooutput/common   \
	-I$(top_srcdir)/lib/engine/videooutput/dx
endif


#-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

bin_SCRIPTS = ekiga-config-tool
bin_PROGRAMS = ekiga

EXTRA_PROGRAMS =

# Common stuff
ekiga_SOURCES =			\
	common.h

nodist_ekiga_SOURCES =

# Graphical User Interface
ekiga_SOURCES +=			\
	gui/accounts.h    		\
	gui/accounts.cpp		\
	gui/callbacks.h			\
	gui/callbacks.cpp		\
	gui/conf.h			\
	gui/conf.cpp			\
	gui/dialpad.h			\
	gui/dialpad.cpp			\
	gui/assistant.h			\
	gui/assistant.cpp		\
	gui/main.h			\
	gui/main.cpp			\
	gui/misc.h			\
	gui/misc.cpp			\
	gui/preferences.h		\
	gui/preferences.cpp		\
	gui/statusicon.h		\
	gui/statusicon.cpp		\
	gui/statusmenu.h		\
	gui/statusmenu.cpp

# Endpoints
ekiga_SOURCES +=				\
	ekiga.h			                \
	ekiga.cpp

# resources
if WIN32
ekiga_SOURCES +=                   		\
	$(top_srcdir)/win32/plugin-gconst.cpp	\
	$(top_srcdir)/win32/ekiga-rc.rc
endif

if HAVE_DBUS
ekiga_SOURCES +=		\
	dbus-helper/dbus.h	\
	dbus-helper/dbus.cpp

nodist_ekiga_SOURCES +=		\
	dbus-helper/dbus-stub.h

bin_PROGRAMS += ekiga-helper

ekiga_helper_SOURCES = 		\
	dbus-helper/dbus-helper.cpp

nodist_ekiga_helper_SOURCES = 	\
	dbus-helper/dbus-helper-stub.h

ekiga_helper_LDADD = $(DBUS_LIBS)

BUILT_SOURCES += dbus-helper/dbus-helper-stub.h dbus-helper/dbus-stub.h

dbus-helper/dbus-helper-stub.h: dbus-helper/dbus-helper-stub.xml build-subdir-stamp
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=helper --mode=glib-server --output=$@ $<

dbus-helper/dbus-stub.h: dbus-helper/dbus-stub.xml build-subdir-stamp
	$(LIBTOOL) --mode=execute dbus-binding-tool --prefix=ekiga_dbus_component --mode=glib-server --output=$@ $<
endif

build-subdir-stamp:
	test -d dbus-helper || mkdir dbus-helper
	touch build-subdir-stamp

if INSTALL_DBUS_SERVICE

%.service: %.service.in
	$(mkdir_p) dbus-helper # this is ugly...
	sed -e "s#\@bindir\@#@bindir@#" $< > $@

servicedir = $(datadir)/dbus-1/services
service_DATA = dbus-helper/org.ekiga.Ekiga.service dbus-helper/org.ekiga.Helper.service
service_in_files = $(service_DATA:.service=.service.in)

endif

.rc.o:
	$(LIBTOOL) --tag=RC --mode=compile $(RC) $< -o $@ -I $(top_srcdir)

GIT_REVISION=\"$$(cd ../.git; git describe )\"
CACHED_REVISION=$$(cat revision.h 2>/dev/null | cut -c24-)

src/revision.h:
	if test -d "../.git" -a -n "$$(which git)"; then \
          if test "x$(GIT_REVISION)" != "x$(CACHED_REVISION)"; then \
            echo "#define EKIGA_REVISION $(GIT_REVISION)" > revision.h; \
          fi \
        else \
          if !(test -e "./revision.h"); then\
            echo "#define EKIGA_REVISION \"unknown\"" > revision.h; \
          fi \
        fi

AM_CXXFLAGS = $(GTK_CFLAGS) $(GLIB_CFLAGS) $(GNOME_CFLAGS) $(DBUS_CFLAGS) $(BONOBO_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS) $(SIGC_CFLAGS) $(XML_CFLAGS) $(NOTIFY_CFLAGS)
AM_LIBS = $(GTK_LIBS) $(GLIB_LIBS) $(GNOME_LIBS) $(DBUS_LIBS) $(BONOBO_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS) $(SIGC_LIBS) $(XML_LIBS) $(NOTIFY_LIBS)

ekiga_LDADD = \
	$(top_builddir)/lib/libekiga.la $(top_builddir)/lib/engine/libekiga_engine.la $(AM_LIBS)

EXTRA_DIST = \
	$(service_in_files)		\
	dbus-helper/dbus-stub.xml	\
	dbus-helper/dbus-helper-stub.xml \
	ekiga-debug-analyser

CLEANFILES = \
	$(service_DATA)		\
	build-subdir-stamp	\
	$(BUILT_SOURCES)

DISTCLEANFILES = revision.h
