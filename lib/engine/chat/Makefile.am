noinst_LTLIBRARIES = libgmchat.la

chat_dir = $(top_srcdir)/lib/engine/chat

AM_CPPFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/engine/framework \
	-I$(top_srcdir)/lib/engine/presence

libgmchat_la_SOURCES = \
	$(chat_dir)/chat-core.h			\
	$(chat_dir)/chat-core.cpp		\
	$(chat_dir)/dialect.h			\
	$(chat_dir)/dialect-impl.h		\
	$(chat_dir)/chat.h			\
	$(chat_dir)/chat-simple.h		\
	$(chat_dir)/chat-multiple.h

libgmchat_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS)
