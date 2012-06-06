noinst_LTLIBRARIES = libgmopal.la

opal_dir = $(top_srcdir)/lib/engine/components/opal

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS) $(OPAL_CFLAGS) $(PTLIB_CFLAGS)

INCLUDES = 					        \
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/lib/gmconf			\
	-I$(top_srcdir)/lib/toolbox/			\
	-I$(top_srcdir)/lib/engine/			\
	-I$(top_srcdir)/lib/engine/account		\
	-I$(top_srcdir)/lib/engine/addressbook	\
	-I$(top_srcdir)/lib/engine/chat		\
	-I$(top_srcdir)/lib/engine/hal		\
	-I$(top_srcdir)/lib/engine/presence	\
	-I$(top_srcdir)/lib/engine/protocol	\
	-I$(top_srcdir)/lib/engine/videooutput	\
	-I$(top_srcdir)/lib/engine/videoinput	\
	-I$(top_srcdir)/lib/engine/audioinput	\
	-I$(top_srcdir)/lib/engine/audiooutput	\
	-I$(top_srcdir)/lib/engine/framework

# Endpoints 
libgmopal_la_SOURCES =                          \
	$(opal_dir)/opal-call-manager.h         \
	$(opal_dir)/opal-call-manager.cpp       \
	$(opal_dir)/pcss-endpoint.h             \
	$(opal_dir)/pcss-endpoint.cpp           \
	$(opal_dir)/opal-account.h              \
	$(opal_dir)/opal-account.cpp            \
	$(opal_dir)/opal-bank.h                 \
	$(opal_dir)/opal-bank.cpp               \
	$(opal_dir)/opal-call.h                 \
	$(opal_dir)/opal-call.cpp               \
	$(opal_dir)/opal-codec-description.h    \
	$(opal_dir)/opal-codec-description.cpp  \
	$(opal_dir)/opal-gmconf-bridge.h        \
	$(opal_dir)/opal-gmconf-bridge.cpp      \
	$(opal_dir)/opal-main.h                 \
	$(opal_dir)/opal-main.cpp               \
	$(opal_dir)/opal-audio.h                \
	$(opal_dir)/opal-audio.cpp              \
	$(opal_dir)/opal-videoinput.h           \
	$(opal_dir)/opal-videoinput.cpp         \
	$(opal_dir)/opal-videooutput.h          \
	$(opal_dir)/opal-videooutput.cpp

if HAVE_H323
libgmopal_la_SOURCES += \
	$(opal_dir)/h323-endpoint.h \
	$(opal_dir)/h323-endpoint.cpp
endif

libgmopal_la_SOURCES += \
	$(opal_dir)/sip-chat-simple.h \
	$(opal_dir)/sip-chat-simple.cpp \
	$(opal_dir)/sip-dialect.h \
	$(opal_dir)/sip-dialect.cpp \
	$(opal_dir)/sip-endpoint.h \
	$(opal_dir)/sip-endpoint.cpp

libgmopal_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS) $(OPAL_LIBS) $(PTLIB_LIBS)
