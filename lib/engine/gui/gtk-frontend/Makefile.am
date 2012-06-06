noinst_LTLIBRARIES = libgmgtk-frontend.la

gtk_frontend_dir = $(top_srcdir)/lib/engine/gui/gtk-frontend

AM_CPPFLAGS = $(XML_CFLAGS) $(SIGC_CFLAGS) $(GTK_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/ 					\
	-I$(top_srcdir)/lib/gmconf 				\
	-I$(top_srcdir)/lib/gui 				\
	-I$(top_srcdir)/lib/engine/framework 			\
	-I$(top_srcdir)/lib/engine/chat 			\
	-I$(top_srcdir)/lib/engine/presence 		\
	-I$(top_srcdir)/lib/engine/protocol 		\
	-I$(top_srcdir)/lib/engine/account 		\
	-I$(top_srcdir)/lib/engine/addressbook 		\
	-I$(top_srcdir)/lib/engine/components/call-history 	\
	-I$(top_srcdir)/lib/engine/gui/gtk-core 

libgmgtk_frontend_la_SOURCES = \
	$(gtk_frontend_dir)/addressbook-window.h 	\
	$(gtk_frontend_dir)/addressbook-window.cpp 	\
	$(gtk_frontend_dir)/book-view-gtk.h 		\
	$(gtk_frontend_dir)/book-view-gtk.cpp 		\
	$(gtk_frontend_dir)/roster-view-gtk.h 		\
	$(gtk_frontend_dir)/roster-view-gtk.cpp 	\
	$(gtk_frontend_dir)/call-history-view-gtk.h 	\
	$(gtk_frontend_dir)/call-history-view-gtk.cpp 	\
	$(gtk_frontend_dir)/presentity-view.h		\
	$(gtk_frontend_dir)/presentity-view.cpp		\
	$(gtk_frontend_dir)/chat-area.h			\
	$(gtk_frontend_dir)/chat-area.cpp		\
	$(gtk_frontend_dir)/simple-chat-page.h		\
	$(gtk_frontend_dir)/simple-chat-page.cpp	\
	$(gtk_frontend_dir)/multiple-chat-page.h	\
	$(gtk_frontend_dir)/multiple-chat-page.cpp	\
	$(gtk_frontend_dir)/chat-window.h 		\
	$(gtk_frontend_dir)/chat-window.cpp 		\
	$(gtk_frontend_dir)/gtk-frontend.h 		\
	$(gtk_frontend_dir)/gtk-frontend.cpp

libgmgtk_frontend_la_LIBADD = \
	$(top_builddir)/lib/gui/libgmwidgets.la 			\
	$(top_builddir)/lib/engine/chat/libgmchat.la 		\
	$(top_builddir)/lib/engine/presence/libgmpresence.la 	\
	$(top_builddir)/lib/engine/addressbook/libgmaddressbook.la \
	$(top_builddir)/lib/engine/gui/gtk-core/libgmgtk-core.la

libgmgtk_frontend_la_LDFLAGS = -export-dynamic -no-undefined $(GTK_LIBS)
