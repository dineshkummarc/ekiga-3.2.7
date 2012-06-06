noinst_LTLIBRARIES = libgmconfpersonaldetails.la

gmconf_personal_details_dir = $(top_srcdir)/lib/engine/components/gmconf-personal-details

AM_CXXFLAGS = $(SIGC_CFLAGS) $(GLIB_CFLAGS)

INCLUDES = 					\
	-I$(top_srcdir)/lib/gmconf	 	\
	-I$(top_srcdir)/lib/engine/framework 

libgmconfpersonaldetails_la_SOURCES = 						\
	$(gmconf_personal_details_dir)/gmconf-personal-details.cpp		\
	$(gmconf_personal_details_dir)/gmconf-personal-details.h		\
	$(gmconf_personal_details_dir)/gmconf-personal-details-main.cpp		\
	$(gmconf_personal_details_dir)/gmconf-personal-details-main.h



libgmconfpersonaldetails_la_LIBADD = 						\
	$(top_builddir)/lib/engine/framework/libgmframework.la

libgmconfpersonaldetails_la_LDFLAGS = -export-dynamic -no-undefined $(SIGC_LIBS) $(GLIB_LIBS)
