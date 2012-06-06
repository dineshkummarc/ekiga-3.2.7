noinst_LTLIBRARIES = libaccount.la

account_dir = $(top_srcdir)/lib/engine/account

AM_CPPFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = \
	-I$(top_srcdir)/lib/gmconf	 		\
	-I$(top_srcdir)/lib/engine/framework

libaccount_la_SOURCES = \
	$(account_dir)/account.h			\
	$(account_dir)/bank.h				\
	$(account_dir)/bank-impl.h			\
	$(account_dir)/account-core.h			\
	$(account_dir)/account-core.cpp

libaccount_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
