SUBDIRS = framework  plugin notification account addressbook presence chat gui protocol videooutput videoinput audioinput audiooutput hal components

noinst_LTLIBRARIES = libekiga_engine.la

INCLUDES = 								\
	-I$(top_srcdir)/lib/gmconf/	 				\
	-I$(top_srcdir)/lib/engine/	 				\
	-I$(top_srcdir)/lib/engine/gui/gtk-core				\
	-I$(top_srcdir)/lib/engine/gui/gtk-frontend			\
	-I$(top_srcdir)/lib/engine/framework/		 		\
	-I$(top_srcdir)/lib/engine/chat					\
	-I$(top_srcdir)/lib/engine/notification				\
	-I$(top_srcdir)/lib/engine/videooutput				\
	-I$(top_srcdir)/lib/engine/videoinput/  			\
	-I$(top_srcdir)/lib/engine/hal					\
	-I$(top_srcdir)/lib/engine/account				\
	-I$(top_srcdir)/lib/engine/audioinput                           \
	-I$(top_srcdir)/lib/engine/audiooutput                          \
	-I$(top_srcdir)/lib/engine/addressbook				\
	-I$(top_srcdir)/lib/engine/plugin				\
	-I$(top_srcdir)/lib/engine/presence				\
	-I$(top_srcdir)/lib/engine/protocol				\
	-I$(top_srcdir)/lib/engine/components/gmconf-personal-details	\
	-I$(top_srcdir)/lib/engine/components/avahi-publisher		\
	-I$(top_srcdir)/lib/engine/components/gstreamer			\
	-I$(top_srcdir)/lib/engine/components/kde			\
	-I$(top_srcdir)/lib/engine/components/opal	                \
	-I$(top_srcdir)/lib/engine/components/ptlib	                \
	-I$(top_srcdir)/lib/engine/components/call-history		\
	-I$(top_srcdir)/lib/engine/components/evolution		        \
	-I$(top_srcdir)/lib/engine/components/kab			\
	-I$(top_srcdir)/lib/engine/components/ldap			\
	-I$(top_srcdir)/lib/engine/components/avahi			\
	-I$(top_srcdir)/lib/engine/components/local-roster		\
	-I$(top_srcdir)/lib/engine/components/resource-list		\
	-I$(top_srcdir)/lib/engine/components/xcap

# XVideo/X
if !WIN32
INCLUDES +=                                                             \
	-I$(top_srcdir)/lib/engine/components/common-videooutput        \
	-I$(top_srcdir)/lib/engine/components/x-videooutput
endif

# DirectX
if HAVE_DX
INCLUDES +=                                                             \
	-I$(top_srcdir)/lib/engine/components/common-videooutput        \
	-I$(top_srcdir)/lib/engine/components/dx-videooutput
endif

# Default Input Devices
INCLUDES +=                                                             \
	-I$(top_srcdir)/lib/engine/components/null-audioinput           \
	-I$(top_srcdir)/lib/engine/components/null-audiooutput          \
	-I$(top_srcdir)/lib/engine/components/mlogo-videoinput

# Default PTLIB Input Devices
INCLUDES +=                                             \
	-I$(top_srcdir)/lib/engine/videoinput/ptlib     \
	-I$(top_srcdir)/lib/engine/audioinput/ptlib     \
	-I$(top_srcdir)/lib/engine/audiooutput/ptlib

# DBus / HALd
if HAVE_DBUS
INCLUDES +=                                             \
	-I$(top_srcdir)/lib/engine/components/hal-dbus
endif

libekiga_engine_la_SOURCES = engine.h engine.cpp

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GTK_CFLAGS)
AM_LIBS = $(SIGC_LIBS) $(GTK_LIBS)

libekiga_engine_la_LDFLAGS = -export-dynamic
libekiga_engine_la_LIBADD = \
	$(top_builddir)/lib/engine/notification/libnotification.la					\
	$(top_builddir)/lib/engine/gui/gtk-core/libgmgtk-core.la 					\
	$(top_builddir)/lib/engine/gui/gtk-frontend/libgmgtk-frontend.la 				\
	$(top_builddir)/lib/engine/chat/libgmchat.la 							\
	$(top_builddir)/lib/engine/protocol/libgmprotocol.la	 				        \
	$(top_builddir)/lib/engine/framework/libgmframework.la 						\
	$(top_builddir)/lib/engine/addressbook/libgmaddressbook.la 				        \
	$(top_builddir)/lib/engine/components/call-history/libcall-history.la 				\
	$(top_builddir)/lib/engine/account/libaccount.la 						\
	$(top_builddir)/lib/engine/presence/libgmpresence.la 					        \
	$(top_builddir)/lib/engine/plugin/libplugin.la							\
	$(top_builddir)/lib/engine/components/local-roster/liblocal-roster.la 				\
	$(top_builddir)/lib/engine/videooutput/libgmvideooutput.la					\
	$(top_builddir)/lib/engine/videoinput/libgmvideoinput.la					\
	$(top_builddir)/lib/engine/audioinput/libgmaudioinput.la					\
	$(top_builddir)/lib/engine/audiooutput/libgmaudiooutput.la					\
	$(top_builddir)/lib/engine/hal/libgmhal.la							\
	$(top_builddir)/lib/engine/components/gmconf-personal-details/libgmconfpersonaldetails.la	\
	$(top_builddir)/lib/engine/components/opal/libgmopal.la                                           \
	$(AM_LIBS)

# Default Input Devices
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/components/mlogo-videoinput/libgmvideoinput-mlogo.la                 \
	$(top_builddir)/lib/engine/components/null-audioinput/libgmaudioinput-null.la                   \
	$(top_builddir)/lib/engine/components/null-audiooutput/libgmaudiooutput-null.la 

# PTLIB Input Devices
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/components/ptlib/libgmptlib.la

# DBus/HALd
if HAVE_DBUS
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/components/hal-dbus/libgmhal-dbus.la
endif

# XVideo/X
if !WIN32
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/components/x-videooutput/libgmvideooutput-x.la \
	$(top_builddir)/lib/pixops/libpixops.la
endif

# DirectX
if HAVE_DX
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/components/dx-videooutput/libgmvideooutput-dx.la
endif


if HAVE_AVAHI
libekiga_engine_la_LIBADD += \
	$(top_builddir)/lib/engine/components/avahi/libgmavahi.la \
	$(top_builddir)/lib/engine/components/avahi-publisher/libavahipublisher.la
endif

if HAVE_EDS
libekiga_engine_la_LIBADD += components/evolution/libgmevolution.la
endif

if HAVE_LDAP
libekiga_engine_la_LIBADD += components/ldap/libgmldap.la
endif

if HAVE_GNOME
if HAVE_DBUS
INCLUDES += -I$(top_srcdir)/lib/engine/components/gnome-session/
libekiga_engine_la_LIBADD += components/gnome-session/libgmgnomesession.la
endif
endif

if HAVE_GSTREAMER
libekiga_engine_la_LIBADD += components/gstreamer/libgmgstreamer.la
endif

if HAVE_KDE
libekiga_engine_la_LIBADD += components/kde/libgmkde.la
endif

if HAVE_KAB
libekiga_engine_la_LIBADD += components/kab/libgmkab.la
endif

if HAVE_XCAP
libekiga_engine_la_LIBADD += \
	components/xcap/libgmxcap.la \
	components/resource-list/libgmresource_list.la
endif

if HAVE_NOTIFY
INCLUDES += -I$(top_srcdir)/lib/engine/components/libnotify/
libekiga_engine_la_LIBADD += components/libnotify/libgmlibnotify.la
endif

#### FIXME should be removed at a later stage
INCLUDES += \
	-I$(top_srcdir)/lib/gui/	                \
	-I$(top_srcdir)/src                             \
	-I$(top_srcdir)/src/clients/                    \
	-I$(top_srcdir)/src/components/                 \
	-I$(top_srcdir)/src/devices/                    \
	-I$(top_srcdir)/src/endpoints/                  \
	-I$(top_srcdir)/src/gui/                        

AM_CXXFLAGS += $(GTK_CFLAGS) $(GLIB_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS)

libekiga_engine_la_LDFLAGS += $(GTK_LIBS) $(GLIB_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS)

#### End
