noinst_LTLIBRARIES = libgmechochat.la

echochat_dir = $(top_srcdir)/lib/engine/components/echo

AM_CXXFLAGS = $(SIGC_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/presence \
	-I$(top_srcdir)/lib/engine/chat

libgmechochat_la_SOURCES = \
	$(echochat_dir)/echo-presentity.h	\
	$(echochat_dir)/echo-presentity.cpp	\
	$(echochat_dir)/echo-simple.h		\
	$(echochat_dir)/echo-simple.cpp		\
	$(echochat_dir)/echo-dialect.h		\
	$(echochat_dir)/echo-dialect.cpp	\
	$(echochat_dir)/echo-main.h		\
	$(echochat_dir)/echo-main.cpp

libgmechochat_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS)
