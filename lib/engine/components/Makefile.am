if HAVE_DBUS
DBUS_DIR = hal-dbus

if HAVE_GNOME
GNOME_SESSION_DIR = gnome-session
endif

endif

if HAVE_LDAP
LDAP_DIR = ldap
endif

if HAVE_DX
DX_DIR = dx-videooutput
endif

if !WIN32
X_DIR = x-videooutput
endif

if HAVE_KAB
KAB_DIR = kab
endif

if HAVE_EDS
EDS_DIR = evolution
endif

if HAVE_AVAHI
AVAHI_DIR = avahi-publisher
endif

if HAVE_GSTREAMER
GSTREAMER_DIR = gstreamer
endif

if HAVE_KDE
KDE_DIR = kde
endif

if HAVE_NOTIFY
LIBNOTIFY_DIR = libnotify
endif

if HAVE_XCAP
XCAP_DIRS = xcap resource-list
endif

if HAVE_AVAHI
AVAHI_DIR += avahi
endif

SUBDIRS =                       \
	call-history	        \
        echo                    \
	gmconf-personal-details \
        local-roster            \
        mlogo-videoinput        \
        null-audioinput         \
        null-audiooutput        \
        opal                    \
        ptlib                   \
	$(GNOME_SESSION_DIR)	\
	$(AVAHI_DIR)            \
	$(DBUS_DIR)             \
	$(GSTREAMER_DIR)        \
	$(KDE_DIR)              \
	$(LIBNOTIFY_DIR)	\
	$(EDS_DIR) 	        \
	$(LDAP_DIR)	        \
	$(KAB_DIR)              \
	$(DX_DIR)               \
	$(X_DIR)                \
	$(XCAP_DIRS)
