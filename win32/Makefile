#!/usr/bin/make -f

# RELEASE: 0=from git/svn, 1=release (tar.gz) for ekiga, ptlib and opal
RELEASE   := 1
# DEBUG: 0=release, 1=debug version
DEBUG     := 0

MAKEOPTS  := $(MAKEFLAGS)
MAKE_LIB_OPTS:=$(MAKEFLAGS)

BUILDROOT := $(CURDIR)
TARGETDIR := $(BUILDROOT)/dist
SRCDIR    := $(BUILDROOT)/src
INCLUDEDIR:= $(BUILDROOT)/include
LIBDIR    := $(BUILDROOT)/lib
BINDIR    := $(BUILDROOT)/bin

export PKG_CONFIG_LIBDIR:=$(LIBDIR)

export DEB_BUILD_GNU_TYPE:=$(shell dpkg-architecture -qDEB_BUILD_GNU_TYPE)
export DEB_HOST_GNU_TYPE:="i586-pc-mingw32"

WGET := wget -nv -T 60 -N

HOST_TOOL_PREFIX:=i586-mingw32msvc

REALCC:=$(HOST_TOOL_PREFIX)-gcc
REALCXX:=$(HOST_TOOL_PREFIX)-g++

MACH_FLAGS:=-march=pentium-mmx
OPTI_FLAGS:=-O3 -fomit-frame-pointer
#	IX86_TYPE according to Intel
#	0 : 8086/88, 2 = 80286, 3 = 80386
#	4 = 80486, 5 = Pentium(R) Processor, 6 = PentiumPro(R) Processor
#       7 or higher = Processor beyond the PentiumPro6(R) Processor
IX86_TYPE:=5


ifeq ($(DEBUG),1)
	export CC=$(REALCC) -mms-bitfields -g
	export CXX=$(REALCXX) -mms-bitfields -g
else
	export CC=$(REALCC) -mms-bitfields
	export CXX=$(REALCXX) -mms-bitfields
endif

export LD:=$(HOST_TOOL_PREFIX)-ld
export NM:=$(HOST_TOOL_PREFIX)-nm
export AR:=$(HOST_TOOL_PREFIX)-ar
export RANLIB:=$(HOST_TOOL_PREFIX)-ranlib
export DLLTOOL:=$(HOST_TOOL_PREFIX)-dlltool
export DLLWRAP:=$(HOST_TOOL_PREFIX)-dllwrap
export OBJDUMP:=$(HOST_TOOL_PREFIX)-objdump
export RC:=$(HOST_TOOL_PREFIX)-windres
export STRIP:=$(HOST_TOOL_PREFIX)-strip
export AS:=$(HOST_TOOL_PREFIX)-as
export CFLAGS:=-I$(INCLUDEDIR) $(MACH_FLAGS)
export CXXFLAGS:=-I$(INCLUDEDIR) $(MACH_FLAGS)
export LDFLAGS:=-L$(LIBDIR) $(MACH_FLAGS)
export PKG_CONFIG_PATH:=$(LIBDIR)/pkgconfig
export PATH:=$(BINDIR):$(PATH)
export LDD:=$(BUILDROOT)/$(HOST_TOOL_PREFIX)-ldd.sh


ifeq ($(RELEASE),1)
	EKIGA_VER := 3.2.7
	EKIGA_URL := http://ftp.gnome.org/pub/gnome/sources/ekiga/3.2
	EKIGA_ARCHIVE := ekiga-$(EKIGA_VER).tar.gz
	EKIGA_DIR := $(BUILDROOT)/ekiga-$(EKIGA_VER)
else
	#GIT_EKIGA_REV = reset --hard HEAD
	GIT_EKIGA_REV := checkout -b gnome-2-26 origin/gnome-2-26
	EKIGA_URL := git://git.gnome.org/ekiga
	EKIGA_ARCHIVE := ekiga
	EKIGA_DIR = $(BUILDROOT)/ekiga
	EKIGA_VER := $(strip $(shell grep AC_INIT $(SRCDIR)/$(EKIGA_ARCHIVE)/configure.ac | awk 'BEGIN { FS = "," } ; { print $$2 }'))
	EKIGA_REV := $(strip $(shell (cd $(SRCDIR)/$(EKIGA_ARCHIVE); git describe | awk 'BEGIN { FS = "-" } ; { print $$2 "_" $$3 }' )))
ifneq ($(EKIGA_REV),)
		EKIGA_VER := $(EKIGA_VER)-git-$(EKIGA_REV)
endif
endif
EKIGA_PREFIX := $(TARGETDIR)/Ekiga
ifeq ($(DEBUG),1)
EKIGA_INSTALLER := ekiga-setup-$(EKIGA_VER)-debug.exe
else
EKIGA_INSTALLER := ekiga-setup-$(EKIGA_VER).exe
endif
#EKIGA_WIN32_DIFF_DIR := $(CURDIR)/diff
EKIGA_INSTALLER_DIR := $(EKIGA_DIR)/win32/nsisinstaller

ifeq ($(RELEASE),1)
	OPAL_VER := 3.6.8
	OPAL_URL := http://ftp.gnome.org/pub/gnome/sources/opal/3.6
	OPAL_ARCHIVE := opal-$(OPAL_VER).tar.gz
	OPAL_DIR := $(BUILDROOT)/opal-$(OPAL_VER)
else
	OPAL_VER := 3.6-beta9
	# OPAL_REV: HEAD or revision number
	OPAL_REV := HEAD
	OPAL_URL := https://opalvoip.svn.sourceforge.net/svnroot/opalvoip/opal/branches/v3_6
	OPAL_ARCHIVE := opal
	OPAL_DIR := $(BUILDROOT)/opal
endif
ifeq ($(DEBUG),1)
	OPAL_LIB := libopal_d.$(OPAL_VER).dll
else
	OPAL_LIB := libopal.$(OPAL_VER).dll
endif

ifeq ($(RELEASE),1)
	PTLIB_VER := 2.6.7
	PTLIB_URL := http://ftp.gnome.org/pub/gnome/sources/ptlib/2.6
	PTLIB_ARCHIVE := ptlib-$(PTLIB_VER).tar.gz
	PTLIB_DIR := $(BUILDROOT)/ptlib-$(PTLIB_VER)
else
	PTLIB_VER := 2.6-beta8
	# PTLIB_REV: HEAD or revision number
	PTLIB_REV := HEAD
	PTLIB_URL := https://opalvoip.svn.sourceforge.net/svnroot/opalvoip/ptlib/branches/v2_6
	PTLIB_ARCHIVE := ptlib
	PTLIB_DIR := $(BUILDROOT)/ptlib
endif
ifeq ($(DEBUG),1)
	PTLIB_LIB := libpt_d.$(PTLIB_VER).dll
else
	PTLIB_LIB := libpt.$(PTLIB_VER).dll
endif

OPENLDAP_VER := 2.4.22
OPENLDAP_DIR := $(BUILDROOT)/openldap-$(OPENLDAP_VER)
OPENLDAP_ARCHIVE := openldap-$(OPENLDAP_VER).tgz
OPENLDAP_URL := http://www.openldap.org/software/download/OpenLDAP/openldap-release

LIBSASL2_VER := 2.1.22
LIBSASL2_DIR := $(BUILDROOT)/cyrus-sasl-$(LIBSASL2_VER)
LIBSASL2_ARCHIVE := cyrus-sasl-$(LIBSASL2_VER).zip
LIBSASL2_URL := http://developer.pidgin.im/static/win32

OGG_VER := 1.2.0
OGG_DIR := $(BUILDROOT)/libogg-$(OGG_VER)
OGG_ARCHIVE := libogg-$(OGG_VER).tar.gz
OGG_URL := http://downloads.xiph.org/releases/ogg

SPEEX_VER := 1.2rc1
SPEEX_DIR := $(BUILDROOT)/speex-$(SPEEX_VER)
SPEEX_ARCHIVE := speex-$(SPEEX_VER).tar.gz
SPEEX_URL := http://downloads.xiph.org/releases/speex

GTK_VER := 2.16.6-2
GTK_URL := http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.16
GTK_ARCHIVE := gtk+-dev_$(GTK_VER)_win32.zip

# this is an installer, not a zip of files to be manually installed
# see http://gtk-win.sourceforge.net for more information
GTK_BIN_VER := 2.16.6
GTK_INSTALLER_URL := http://downloads.sourceforge.net/gtk-win
GTK_BIN_EXE := gtk2-runtime-2.16.6-2010-05-12-ash.exe

GLIB_VER := 2.18.4-1
GLIB_URL := http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.18
GLIB_ARCHIVE := glib-dev_$(GLIB_VER)_win32.zip

ATK_VER := 1.24.0-1
ATK_URL := http://ftp.gnome.org/pub/gnome/binaries/win32/atk/1.24
ATK_ARCHIVE := atk-dev_$(ATK_VER)_win32.zip

CAIRO_VER := 1.8.6-1
CAIRO_URL := http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies
CAIRO_ARCHIVE := cairo-dev_$(CAIRO_VER)_win32.zip

PANGO_VER := 1.24.5-2
PANGO_URL := http://ftp.gnome.org/pub/gnome/binaries/win32/pango/1.24
PANGO_ARCHIVE := pango-dev_$(PANGO_VER)_win32.zip

XML2_VER := 2.7.7
XML2_DIR := libxml2-$(XML2_VER)
XML2_URL := ftp://xmlsoft.org/libxml2
XML2_ARCHIVE := libxml2-$(XML2_VER).tar.gz

LIBPNG_VER := 1.2.37
GNUWIN32_URL := http://downloads.sourceforge.net/gnuwin32

EXPAT_VER = 2.0.1
EXPAT_DIR = $(BUILDROOT)/expat-$(EXPAT_VER)
EXPAT_ARCHIVE := expat-$(EXPAT_VER).tar.gz
EXPAT_URL := http://kent.dl.sourceforge.net/sourceforge/expat

# choose commit 203e34 (2007-09-30), since since then a few modifications
# have been done and the compilation needs to be updated
REGEX_REV := 203e34
REGEX_DIR := $(BUILDROOT)/regex
REGEX_ARCHIVE := regex

SIGC_VER := 2.2.7
SIGC_URL := http://ftp.gnome.org/pub/GNOME/sources/libsigc++/2.2
SIGC_ARCHIVE := libsigc++-$(SIGC_VER).tar.gz
SIGC_DIR := libsigc++-$(SIGC_VER)

GETTEXT_VER := 0.17-1
GETTEXT_URL :=http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies
GETTEXT_ARCHIVE := gettext-runtime-dev-$(GETTEXT_VER).zip

ICONV_VER := 1.9.2-1
ICONV_URL := http://downloads.sourceforge.net/project/gnuwin32/libiconv/$(ICONV_VER)
ICONV_ARCHIVE := libiconv-$(ICONV_VER)-bin.zip

# use date for FFMPEG_VER, because it has two "repositories"
FFMPEG_VER := {2009-04-14}
FFMPEG_URL := svn://svn.mplayerhq.hu/ffmpeg/trunk
FFMPEG_DIR := $(BUILDROOT)/ffmpeg
FFMPEG_ARCHIVE := ffmpeg

X264_VER := d2e1e1c35c43ea9c90c9211be
X264_URL := git://git.videolan.org/x264.git
X264_DIR := $(BUILDROOT)/x264
X264_ARCHIVE := x264

# note that every celt release might break API
CELT_VER := 0.7.1
CELT_URL := http://downloads.xiph.org/releases/celt
CELT_DIR := $(BUILDROOT)/celt-$(CELT_VER)
CELT_ARCHIVE := celt-$(CELT_VER).tar.gz

THEORA_VER := 1.1.1
THEORA_URL := http://downloads.xiph.org/releases/theora
THEORA_DIR := $(BUILDROOT)/libtheora-$(THEORA_VER)
THEORA_ARCHIVE := libtheora-$(THEORA_VER).tar.gz

NSIS_VER := 2.46
NSIS_URL := http://downloads.sourceforge.net/nsis
NSISPROCESSES_URL := http://nsis.sourceforge.net/mediawiki/images/f/f3/
NSISSYTEM_DIR := /usr/share/nsis/Contrib/Modern\ UI
NSISPLUGIN_DIR := $(BUILDROOT)/nsisplugins

LN:=ln
RM:=rm
CP:=cp
MV:=mv
SVN:=svn
GIT:=git

# additional help languages to be built, eg to build only C and fr use:
# DOC_LINGUAS := fr
DOC_LINGUAS := $(strip $(shell grep DOC_LINGUAS $(SRCDIR)/$(EKIGA_ARCHIVE)/help/Makefile.am | cut -d= -f2))

confflags := --prefix=$(BUILDROOT) --build=$(DEB_BUILD_GNU_TYPE) --host=$(DEB_HOST_GNU_TYPE)

confptlib := $(confflags) \
	--enable-vfw --enable-vidfile --with-directx-includedir=$(EKIGA_DIR)/win32/directx \
	--disable-qos --disable-vidfile --disable-alsa --disable-esd --disable-oss --disable-sunaudio --disable-shmaudio \
	--disable-v4l --disable-v4l2 --disable-bsdvideo --disable-avc --disable-dc

confopal := $(confflags) --with-ffmpeg-dir=$(FFMPEG_DIR) --disable-t38 --disable-fax

confekiga:= $(confflags) \
	--with-directx-dir=$(EKIGA_DIR)/win32/directx --prefix=$(EKIGA_PREFIX) \
	--bindir=$(EKIGA_PREFIX) --datadir=$(EKIGA_PREFIX) \
	--sysconfdir=$(EKIGA_PREFIX) --libdir=$(EKIGA_PREFIX) --disable-gnome \
	--disable-dbus --disable-avahi --disable-xv \
	--with-ldap-dir=$(OPENLDAP_DIR) --with-libsasl2-dir=$(LIBSASL2_DIR) # --enable-static-libs

confffmpeg := --cross-prefix=i586-mingw32msvc- --enable-shared --enable-memalign-hack \
	--enable-cross-compile --enable-w32threads --arch=i686 --target-os=mingw32 \
	--disable-ffserver --disable-ffmpeg --disable-ffplay \
	--disable-parsers --enable-parser=h264 --enable-parser=mpeg4video --enable-parser=h263 \
	--disable-decoders --enable-decoder=h264 --enable-decoder=mpeg4  --enable-decoder=h263 \
	--disable-muxers --enable-muxer=h264 --enable-muxer=mpeg4 --enable-muxer=h263 \
	--disable-encoders --enable-encoder=mpeg4 --enable-encoder=h263 \
	--disable-protocols --disable-bsfs --disable-ffplay --prefix=$(BUILDROOT) \
	--libdir=$(LIBDIR) --extra-ldflags=-Wl,-static
#	--disable-network

confx264 := $(confflags) --disable-avs-input

confcelt := $(confflags) --disable-oggtest

conftheora := $(confflags) --disable-vorbistest --disable-sdltest --disable-examples --disable-spec

all: binaries $(TARGETDIR)/$(EKIGA_INSTALLER)
	@echo "Built on $(shell date)"

update-sources::
	@echo "Updating on $(shell date)"

update: update-sources

binaries:
	@echo Checking prerequisites...
	hash $(REALCC) $(REALCXX) $(LD) $(NM) $(AR) $(RANLIB) $(DLLTOOL) $(DLLWRAP) $(OBJDUMP) $(RC) $(AS)||(echo You need to install mingw32;exit 1)
	hash tar unzip zip wget aclocal autoheader libtoolize automake autoconf pkg-config gnome-autogen.sh bison gdk-pixbuf-csource || (echo we need more binaries ; exit 1)
	[ -f /usr/share/gnome-doc-utils/gnome-doc-utils.make ] || (echo gnome-doc-tools not installed; exit 1)
	[ -f /usr/lib/pkgconfig/gconf-2.0.pc ] || (echo libgconf2-dev not installed; exit 1)
	[ -f /usr/lib/pkgconfig/avahi-glib.pc ] || (echo libavahi-glib-dev not installed; exit 1)
	[ -f /usr/lib/pkgconfig/avahi-client.pc ] || (echo libavahi-client-dev not installed; exit 1)
	hash makensis || (echo nsis not installed; exit 1)
	hash xmlto || (echo xmlto not installed; exit 1)
	hash yasm || (echo yasm not installed, mandatory for x264; exit 1)
#	[ $(BUILDROOT) = $(CURDIR) ] || (echo You are supposed to run this as Makefile in $(BUILDROOT); exit 1)
	[ `grep define.*MINGW32_MINOR_VERSION /usr/i586-mingw32msvc/include/_mingw.h|tr -s " "|cut -d" " -f3` -ge 15 ] || (echo 'You need mingw32-runtime at version >= 3.15'; exit 1)
	mkdir -p $(INCLUDEDIR) $(LIBDIR)/pkgconfig $(SRCDIR) $(BINDIR)
	$(RM) -f $(BINDIR)/cygpath
	$(LN) -s /bin/true $(BINDIR)/cygpath
	touch $@

### REGEX
update-sources::
	@echo "--- Getting libregex..."
	mkdir -p $(SRCDIR)/$(REGEX_ARCHIVE)
	cd $(SRCDIR)/$(REGEX_ARCHIVE); \
	for i in regex.c regexec.c regex.h regex_internal.c regex_internal.h regcomp.c alloca_.h alloca.c localcharset.c localcharset.h; do \
	  $(WGET) http://git.savannah.gnu.org/cgit/gnulib.git/plain/lib/$$i?id=$(REGEX_REV) -O $$i; \
	done

$(REGEX_DIR)/regex.h: $(SRCDIR)/$(REGEX_ARCHIVE)
	$(RM) -rf $(REGEX_DIR)
	mkdir -p $(REGEX_DIR)
	$(CP) $(SRCDIR)/$(REGEX_ARCHIVE)/* $(REGEX_DIR)
	$(MV) $(REGEX_DIR)/alloca_.h $(REGEX_DIR)/alloca.h
	echo '//' >> $(REGEX_DIR)/config.h
	touch $@

$(LIBDIR)/libregex.a: binaries $(REGEX_DIR)/regex.h
	cd $(REGEX_DIR) ;\
	$(CC) -I$(INCLUDEDIR) -I. -c regex.c ;\
	$(CC) -I$(INCLUDEDIR) -I. -c alloca.c ;\
	$(CC) -I$(INCLUDEDIR) -I. -DLIBDIR=$(INCLUDEDIR) -c localcharset.c ;\
	$(AR) r libregex.a regex.o alloca.o localcharset.o
	$(CP) $(REGEX_DIR)/libregex.a $(LIBDIR)
	$(CP) $(REGEX_DIR)/regex.h $(INCLUDEDIR)
	$(CP) $(REGEX_DIR)/alloca.h $(INCLUDEDIR)
	$(CP) $(REGEX_DIR)/localcharset.h $(INCLUDEDIR)
	touch $@

### libsigc++
update-sources::
	@echo --- Getting libsigc++ ...
	$(WGET) -P src $(SIGC_URL)/$(SIGC_ARCHIVE)

$(SIGC_DIR)/config.status: $(SRCDIR)/$(SIGC_ARCHIVE)
	tar xfz $(SRCDIR)/$(SIGC_ARCHIVE)
	cd $(SIGC_DIR); ./configure $(confflags) --enable-shared

$(LIBDIR)/libsigc++.a:  $(SIGC_DIR)/config.status
	$(MAKE) $(MAKEOPTS) -C $(SIGC_DIR)
	$(MAKE) $(MAKEOPTS) -C $(SIGC_DIR) install
	touch $@

### expat
update-sources::
	@echo --- Getting EXPAT ...
	$(WGET) -P src $(EXPAT_URL)/$(EXPAT_ARCHIVE)

$(EXPAT_DIR)/config.status: $(SRCDIR)/$(EXPAT_ARCHIVE)
	tar xfz $(SRCDIR)/$(EXPAT_ARCHIVE)
	cd $(EXPAT_DIR); ./configure $(confflags)

$(INCLUDEDIR)/expat.h:
	install -m 644 $(EXPAT_DIR)/lib/expat.h $(INCLUDEDIR)/
	touch $@

$(LIBDIR)/libexpat.a:  $(EXPAT_DIR)/config.status $(INCLUDEDIR)/expat.h
	$(MAKE) $(MAKEOPTS) -C $(EXPAT_DIR)
	$(MAKE) $(MAKEOPTS) -C $(EXPAT_DIR) install
	touch $@

### OPENLDAP
update-sources::
	@echo --- Getting OpenLDAP ...
	$(WGET) -P $(SRCDIR) $(OPENLDAP_URL)/$(OPENLDAP_ARCHIVE)

$(OPENLDAP_DIR)/configure: $(LIBDIR)/libregex.a $(SRCDIR)/$(OPENLDAP_ARCHIVE)
	rm -f $(LIBDIR)/libldap.dll
	tar xfz $(SRCDIR)/$(OPENLDAP_ARCHIVE) -C $(BUILDROOT)
	sed -i -e 's#windres#$(RC)#g' $(OPENLDAP_DIR)/libraries/liblutil/Makefile.in
	touch $@

$(OPENLDAP_DIR)/config.status: binaries $(OPENLDAP_DIR)/configure
	#-$(MAKE) -C $(OPENLDAP_DIR) clean
	rm -f $(LIBDIR)/libldap.dll
	$(LN) -sf $(INCLUDEDIR)/regex.h $(OPENLDAP_DIR)/include/
	cd $(OPENLDAP_DIR); ./configure --with-cyrus-sasl=no --enable-bdb=no --enable-hdb=no $(confflags)
	$(MAKE) $(MAKEOPTS) -C $(OPENLDAP_DIR) depend
	touch $@

$(LIBDIR)/libldap.dll: binaries $(OPENLDAP_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/liblutil
	$(CP) -f $(OPENLDAP_DIR)/libraries/liblutil/liblutil.a $(LIBDIR)
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/liblber all install
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/include install
	$(MAKE) $(MAKE_LIB_OPTS) -C $(OPENLDAP_DIR)/libraries/libldap MOD_LIBS="-llutil" all install
	touch $@

### libsasl2
update-sources::
	echo --- Getting libsasl2 ...
	$(WGET) -P $(SRCDIR) $(LIBSASL2_URL)/$(LIBSASL2_ARCHIVE)

$(LIBDIR)/libsasl2.a: $(SRCDIR)/$(LIBSASL2_ARCHIVE)
	rm -f $(LIBDIR)/libsasl2.a
	rm -rf $(LIBSASL2_DIR)
	unzip -u $(SRCDIR)/$(LIBSASL2_ARCHIVE) -d $(BUILDROOT)
	cp $(LIBSASL2_DIR)/lib/libsasl.lib $(LIBDIR)/libsasl2.a

### PTLib
update-sources::
	@echo --- Getting PTLib ...
ifeq ($(RELEASE),1)
	$(WGET) -P $(SRCDIR) $(PTLIB_URL)/$(PTLIB_ARCHIVE)
else
	$(SVN) co $(PTLIB_URL) $(SRCDIR)/$(PTLIB_ARCHIVE) -r $(PTLIB_REV)
endif

$(PTLIB_DIR)/configure: binaries $(LIBDIR)/libldap.dll $(LIBDIR)/libogg.a $(LIBDIR)/libspeex.a $(SRCDIR)/$(PTLIB_ARCHIVE) $(LIBDIR)/libexpat.a
	rm -rf $(PTLIB_DIR) $(LIBDIR)/$(PTLIB_LIB) $(INCLUDEDIR)/ptclib $(INCLUDEDIR)/ptlib
ifeq ($(RELEASE),1)
	tar xfz $(SRCDIR)/$(PTLIB_ARCHIVE) -C $(BUILDROOT)
else
	$(CP) -r $(SRCDIR)/$(PTLIB_ARCHIVE) $(PTLIB_DIR)
endif
	# apply all ptlib_* patches
	cd $(PTLIB_DIR); for i in $(EKIGA_DIR)/win32/diff/ptlib_*; do \
		test `basename $$i` != 'ptlib_*' && (patch -p1 <$$i || exit 1) ;\
	done; true
	touch $@

$(PTLIB_DIR)/config.status: binaries $(PTLIB_DIR)/configure
	rm -f $(LIBDIR)/$(PTLIB_LIB)
	cd $(PTLIB_DIR); ./configure $(confptlib)

$(LIBDIR)/$(PTLIB_LIB): binaries $(PTLIB_DIR)/config.status
ifeq ($(DEBUG),1)
	$(MAKE) $(MAKEOPTS) DEBUG=1 debug -C $(PTLIB_DIR)
	$(MAKE) DEBUG=1 -C $(PTLIB_DIR) install
else
	$(MAKE) $(MAKEOPTS) -C $(PTLIB_DIR)
	$(MAKE) -C $(PTLIB_DIR) install
endif

### libogg
update-sources::
	@echo --- Getting Ogg ...
	$(WGET) -P $(SRCDIR) $(OGG_URL)/$(OGG_ARCHIVE)

$(OGG_DIR)/config.status: binaries $(SRCDIR)/$(OGG_ARCHIVE)
	rm -f $(LIBDIR)/libogg.a
	tar xfz $(SRCDIR)/$(OGG_ARCHIVE) -C $(BUILDROOT)
	cd $(OGG_DIR); ./configure $(confflags)

$(LIBDIR)/libogg.a: binaries $(OGG_DIR)/config.status
	rm -f $(LIBDIR)/libogg.a
	$(MAKE) $(MAKEOPTS) -C $(OGG_DIR)
	$(MAKE) -C $(OGG_DIR) install

### libspeex
update-sources::
	@echo --- Getting Vorbis ...
	$(WGET) -P $(SRCDIR) $(SPEEX_URL)/$(SPEEX_ARCHIVE)

$(SPEEX_DIR)/config.status: binaries $(SRCDIR)/$(SPEEX_ARCHIVE)
	rm -f $(LIBDIR)/libspeex.a
	tar xfz $(SRCDIR)/$(SPEEX_ARCHIVE) -C $(BUILDROOT)
	cd $(SPEEX_DIR); LDFLAGS="$(LDFLAGS) -no-undefined" ./configure $(confflags)

$(LIBDIR)/libspeex.a: binaries $(SPEEX_DIR)/config.status
	rm -f $(LIBDIR)/libspeex.a
	$(MAKE) $(MAKE_LIB_OPTS) -C $(SPEEX_DIR)
	$(MAKE) -C $(SPEEX_DIR) install

### ffmpeg
update-sources::
	@echo --- Getting FFMPEG ...
# ffmpeg has two "repositories" (ffmpeg and libswscale) into their
# repository, so process them separately
ifeq ($(wildcard $(SRCDIR)/$(FFMPEG_ARCHIVE)),)
	$(SVN) co $(FFMPEG_URL) $(SRCDIR)/$(FFMPEG_ARCHIVE) -r $(FFMPEG_VER)
else
	cd $(SRCDIR)/$(FFMPEG_ARCHIVE); $(SVN) up --ignore-externals -r $(FFMPEG_VER)
endif
	cd $(SRCDIR)/$(FFMPEG_ARCHIVE); $(SVN) up -r $(FFMPEG_VER) libswscale

$(FFMPEG_DIR)/config.status: $(SRCDIR)/$(FFMPEG_ARCHIVE)
	rm -rf $(FFMPEG_DIR)
	$(CP) -r $(SRCDIR)/$(FFMPEG_ARCHIVE) $(FFMPEG_DIR)
	cd $(FFMPEG_DIR); ./configure $(confffmpeg)
	touch $@

$(BINDIR)/avcodec.dll: $(FFMPEG_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(FFMPEG_DIR)/libavutil
	$(MAKE) $(MAKE_LIB_OPTS) -C $(FFMPEG_DIR)/libavcodec
	$(MAKE) $(MAKE_LIB_OPTS) -C $(FFMPEG_DIR)/libavformat
	$(MAKE) $(MAKE_LIB_OPTS) -C $(FFMPEG_DIR)/libavdevice
	$(MAKE) $(MAKE_LIB_OPTS) -C $(FFMPEG_DIR)
	$(MAKE) -C $(FFMPEG_DIR) install
	touch $@

### x264
update-sources::
	@echo --- Getting x264 ...
ifeq ($(wildcard $(SRCDIR)/$(X264_ARCHIVE)), )
	$(GIT) clone $(X264_URL) $(SRCDIR)/$(X264_ARCHIVE)
endif
	cd $(SRCDIR)/$(X264_ARCHIVE); git reset --hard $(X264_VER)
	#touch $(SRCDIR)/$(X264_ARCHIVE)

$(X264_DIR)/config.status: $(SRCDIR)/$(X264_ARCHIVE)
	rm -rf $(X264_DIR)
	$(CP) -r $(SRCDIR)/$(X264_ARCHIVE) $(X264_DIR)
	cd $(X264_DIR); CC=$(REALCC) ./configure $(confx264); \
	sed -i -e 's#ranlib#$(RANLIB)#g' $(X264_DIR)/Makefile; \
	sed -i -e 's#strip#$(STRIP)#g' $(X264_DIR)/Makefile; \
	sed -i -e 's#ar #$(AR) #g' $(X264_DIR)/Makefile
	touch $@

$(LIBDIR)/libx264.a: $(X264_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(X264_DIR)
	$(MAKE) -C $(X264_DIR) install

### celt
update-sources::
	echo --- Getting CELT ...
	$(WGET) -P $(SRCDIR) $(CELT_URL)/$(CELT_ARCHIVE)

$(CELT_DIR)/config.status: $(SRCDIR)/$(CELT_ARCHIVE) $(LIBDIR)/libogg.a
	rm -rf $(CELT_DIR) $(INCLUDEDIR)/celt
	tar xfz $(SRCDIR)/$(CELT_ARCHIVE) -C $(BUILDROOT)
	cd $(CELT_DIR); ./configure $(confcelt)

$(LIBDIR)/libcelt0.a: $(CELT_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(CELT_DIR)
	$(MAKE) -C $(CELT_DIR) install

### theora
update-sources::
	@echo --- Getting theora ...
	$(WGET) -P $(SRCDIR) $(THEORA_URL)/$(THEORA_ARCHIVE)

$(THEORA_DIR)/config.status: $(SRCDIR)/$(THEORA_ARCHIVE) $(LIBDIR)/libogg.a
	rm -rf $(THEORA_DIR)
	tar xfz $(SRCDIR)/$(THEORA_ARCHIVE) -C $(BUILDROOT)
	cd $(THEORA_DIR); CC=$(REALCC) LD=$(LD) AR=$(AR) RANLIB=$(RANLIB) ./autogen.sh $(conftheora)

$(LIBDIR)/libtheora.a: $(THEORA_DIR)/config.status
	$(MAKE) $(MAKE_LIB_OPTS) -C $(THEORA_DIR)
	$(MAKE) -C $(THEORA_DIR) install
	sed -i -e "s#library_names=''#library_names='libtheora\.a'#g" $(LIBDIR)/libtheora.la

### OPAL
update-sources::
	@echo --- Getting OPAL ...
ifeq ($(RELEASE),1)
	$(WGET) -P $(SRCDIR) $(OPAL_URL)/$(OPAL_ARCHIVE)
else
	$(SVN) co $(OPAL_URL) $(SRCDIR)/$(OPAL_ARCHIVE) -r $(OPAL_REV)
endif

$(OPAL_DIR)/configure: binaries $(LIBDIR)/libogg.a $(LIBDIR)/libspeex.a $(LIBDIR)/$(PTLIB_LIB) $(SRCDIR)/$(OPAL_ARCHIVE) $(BINDIR)/avcodec.dll $(LIBDIR)/libx264.a $(LIBDIR)/libcelt0.a $(LIBDIR)/libtheora.a
	rm -rf $(OPAL_DIR) $(LIBDIR)/$(OPAL_LIB) $(INCLUDEDIR)/opal
ifeq ($(RELEASE),1)
	tar xfz $(SRCDIR)/$(OPAL_ARCHIVE) -C $(BUILDROOT)
else
	$(CP) -r $(SRCDIR)/$(OPAL_ARCHIVE) $(OPAL_DIR)
endif
	# apply all opal_* patches
	cd $(OPAL_DIR); for i in $(EKIGA_DIR)/win32/diff/opal_*; do \
		test `basename $$i` != 'opal_*' && (patch -p1 <$$i || exit 1) ;\
	done; true
	touch $@

$(OPAL_DIR)/config.status: binaries $(OPAL_DIR)/configure
	rm -f $(LIBDIR)/$(OPAL_LIB)
ifeq ($(DEBUG),1)
	cd $(OPAL_DIR); ./configure $(confopal) --enable-debug
else
	cd $(OPAL_DIR); ./configure $(confopal)
endif

$(LIBDIR)/$(OPAL_LIB): binaries $(OPAL_DIR)/config.status
	rm -f $(LIBDIR)/$(OPAL_LIB)
	$(MAKE) $(MAKEOPTS) -C $(OPAL_DIR)
	$(MAKE) -C $(OPAL_DIR) install
#	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/opal*.pc

### libgtk
update-sources::
	@echo --- Getting GTK2+ ...
	$(WGET) -P $(SRCDIR) $(GTK_URL)/$(GTK_ARCHIVE)

$(LIBDIR)/pkgconfig/gtk+-2.0.pc: $(SRCDIR)/$(GTK_ARCHIVE)
	unzip -u $(SRCDIR)/$(GTK_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/g*.pc
	touch $@

### GTK Runtime
update-sources::
	@echo --- Getting GTK2+ Runtime Library ...
	$(WGET) -P $(SRCDIR) $(GTK_INSTALLER_URL)/$(GTK_BIN_EXE)

$(LIBDIR)/gtk+-$(GTK_BIN_VER)-setup.exe: $(SRCDIR)/$(GTK_BIN_EXE)
	$(CP) $(SRCDIR)/$(GTK_BIN_EXE)  $(LIBDIR)/gtk+-$(GTK_BIN_VER)-setup.exe
	touch $@

### libglib
# already found in gtk runtime, but needed for compilation
update-sources::
	@echo --- Getting GLib ...
	$(WGET) -P $(SRCDIR) $(GLIB_URL)/$(GLIB_ARCHIVE)

$(LIBDIR)/pkgconfig/glib-2.0.pc: $(SRCDIR)/$(GLIB_ARCHIVE)
	unzip -u $(SRCDIR)/$(GLIB_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/g*.pc $(BINDIR)/glib-gettextize
	chmod +x $(BINDIR)/glib-gettextize
	touch $@

### atk
# already found in gtk runtime, but needed for compilation
update-sources::
	@echo --- Getting ATK ...
	$(WGET) -P  $(SRCDIR) $(ATK_URL)/$(ATK_ARCHIVE)

$(LIBDIR)/pkgconfig/atk-2.0.pc: $(SRCDIR)/$(ATK_ARCHIVE)
	unzip -u $(SRCDIR)/$(ATK_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/atk*.pc
	touch $@

### libpng
update-sources::
	@echo --- Getting PNG ...
	$(WGET) -P $(SRCDIR) $(GNUWIN32_URL)/libpng-$(LIBPNG_VER)-bin.zip
	$(WGET) -P $(SRCDIR) $(GNUWIN32_URL)/libpng-$(LIBPNG_VER)-dep.zip
	$(WGET) -P $(SRCDIR) $(GNUWIN32_URL)/libpng-$(LIBPNG_VER)-lib.zip

$(LIBDIR)/libpng.a: $(SRCDIR)/libpng-$(LIBPNG_VER)-bin.zip $(SRCDIR)/libpng-$(LIBPNG_VER)-dep.zip $(SRCDIR)/libpng-$(LIBPNG_VER)-lib.zip
	unzip -u $(SRCDIR)/libpng-$(LIBPNG_VER)-bin.zip -d $(BUILDROOT)
	unzip -u $(SRCDIR)/libpng-$(LIBPNG_VER)-dep.zip -d $(BUILDROOT)
	unzip -u $(SRCDIR)/libpng-$(LIBPNG_VER)-lib.zip -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' lib/pkgconfig/libpng*.pc
	$(LN) -s -f libpng.pc lib/pkgconfig/libpng12.pc
	touch $@

### libintl
update-sources::
	@echo --- Getting Gettext ...
	$(WGET) -P $(SRCDIR) $(GETTEXT_URL)/$(GETTEXT_ARCHIVE)

$(LIBDIR)/libintl.a: $(SRCDIR)/$(GETTEXT_ARCHIVE)
	unzip -u $(SRCDIR)/$(GETTEXT_ARCHIVE) -d $(BUILDROOT)
	touch $@

### libiconv
update-sources::
	@echo --- Getting Iconv ...
	$(WGET) -P $(SRCDIR) $(ICONV_URL)/$(ICONV_ARCHIVE)

$(LIBDIR)/libiconv.a: $(SRCDIR)/$(ICONV_ARCHIVE)
	unzip -u $(SRCDIR)/$(ICONV_ARCHIVE) -d $(BUILDROOT)
	$(RM) -f README.libiconv
	touch $@

### pango
# already found in gtk runtime, but needed for compilation
update-sources::
	@echo --- Getting Pango ...
	$(WGET) -P $(SRCDIR) $(PANGO_URL)/$(PANGO_ARCHIVE)

$(LIBDIR)/pkgconfig/pango32.pc: $(SRCDIR)/$(PANGO_ARCHIVE)
	unzip -u $(SRCDIR)/$(PANGO_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/pango*.pc
	touch $@

### cairo
# already found in gtk runtime, but needed for compilation
update-sources::
	@echo --- Getting Cairo ...
	$(WGET) -P $(SRCDIR) $(CAIRO_URL)/$(CAIRO_ARCHIVE)

$(LIBDIR)/pkgconfig/cairo.pc: $(SRCDIR)/$(CAIRO_ARCHIVE)
	unzip -u $(SRCDIR)/$(CAIRO_ARCHIVE) -d $(BUILDROOT)
	sed -i -e 's#^prefix=.*$$#prefix=$(BUILDROOT)#g' $(LIBDIR)/pkgconfig/cairo*.pc
	touch $@

### libxml2
update-sources::
	@echo --- Getting libXML2 ...
	$(WGET) -P $(SRCDIR) $(XML2_URL)/$(XML2_ARCHIVE)

$(XML2_DIR)/config.status: binaries $(SRCDIR)/$(XML2_ARCHIVE)
	rm -f $(LIBDIR)/libxml2.a
	tar xfz $(SRCDIR)/$(XML2_ARCHIVE) -C $(BUILDROOT)
	cd $(XML2_DIR); ./configure $(confflags) --without-python

$(LIBDIR)/libxml2.a: binaries $(XML2_DIR)/config.status
	rm -f $(LIBDIR)/libxml2.a
	$(MAKE) $(MAKEOPTS) -C $(XML2_DIR)
	$(MAKE) $(MAKEOPTS) -C $(XML2_DIR) install

### ekiga
update-sources::
	@echo --- Getting Ekiga ...
ifeq ($(RELEASE),1)
	$(WGET) -P $(SRCDIR) $(EKIGA_URL)/$(EKIGA_ARCHIVE)
else
ifeq ($(wildcard $(SRCDIR)/$(EKIGA_ARCHIVE)), )
	$(GIT) clone $(EKIGA_URL) $(SRCDIR)/$(EKIGA_ARCHIVE)
	# switch to branch
	cd $(SRCDIR)/$(EKIGA_ARCHIVE); git $(GIT_EKIGA_REV)
else
	cd $(SRCDIR)/$(EKIGA_ARCHIVE); $(GIT) pull
endif
endif

$(EKIGA_DIR)/configure: binaries $(SRCDIR)/$(EKIGA_ARCHIVE)
	rm -rf $(EKIGA_DIR)
ifeq ($(RELEASE),1)
	tar xfz $(SRCDIR)/$(EKIGA_ARCHIVE) -C $(BUILDROOT)
else
	$(CP) -r $(SRCDIR)/$(EKIGA_ARCHIVE) $(EKIGA_DIR)
	cd $(EKIGA_DIR); NOCONFIGURE=1 ./autogen.sh  # generate configure
endif
	# apply all ekiga_* patches
	cd $(EKIGA_DIR); for i in $(EKIGA_DIR)/win32/diff/ekiga_*; do \
		test `basename $$i` != 'ekiga_*' && (patch -p1 <$$i || exit 1) ;\
	done; true
	touch $@

$(EKIGA_DIR)/config.status: binaries $(EKIGA_DIR)/configure $(LIBDIR)/$(PTLIB_LIB) $(LIBDIR)/$(OPAL_LIB) \
	$(LIBDIR)/pkgconfig/gtk+-2.0.pc $(LIBDIR)/pkgconfig/atk-2.0.pc $(LIBDIR)/pkgconfig/glib-2.0.pc \
	$(LIBDIR)/libpng.a $(LIBDIR)/pkgconfig/pango32.pc $(LIBDIR)/pkgconfig/cairo.pc $(LIBDIR)/libxml2.a \
	$(LIBDIR)/libintl.a $(LIBDIR)/libiconv.a $(LIBDIR)/libsigc++.a $(LIBDIR)/libsasl2.a
	rm -f $(EKIGA_DIR)/src/ekiga.exe
ifeq ($(DEBUG),1)
	cd $(EKIGA_DIR); ACLOCAL_FLAGS="-I $(BUILDROOT)/share/aclocal" LDFLAGS="$(LDFLAGS) -Wl,-subsystem,windows" ./configure $(confekiga) --enable-opal-debug
else
	cd $(EKIGA_DIR); ACLOCAL_FLAGS="-I $(BUILDROOT)/share/aclocal" LDFLAGS="$(LDFLAGS) -Wl,-subsystem,windows" ./configure $(confekiga)
endif

$(EKIGA_DIR)/src/ekiga.exe: binaries $(EKIGA_DIR)/config.status
	$(MAKE) $(MAKEOPTS) -C $(EKIGA_DIR)
	$(MAKE) $(MAKEOPTS) -C $(EKIGA_DIR)/help

$(TARGETDIR)/zips: binaries $(EKIGA_DIR)/src/ekiga.exe
	$(RM) -rf $(TARGETDIR)
	mkdir $(TARGETDIR)
	make -C $(EKIGA_DIR) install

	mkdir -p $(EKIGA_PREFIX)/plugins
	for plugin in $(LIBDIR)/opal*/codecs/audio/*; do \
		install -m 755 $$plugin $(EKIGA_PREFIX)/plugins ;\
	done
	for plugin in $(LIBDIR)/opal*/codecs/video/*; do \
		install -m 755 $$plugin $(EKIGA_PREFIX)/plugins ;\
	done
	for lib in $(PTLIB_LIB) $(OPAL_LIB); do \
		install -m 755 $(LIBDIR)/$$lib $(EKIGA_PREFIX) ;\
	done
	for lib in liblber.dll libldap.dll libxml2-2.dll libspeex-1.dll libspeexdsp-1.dll libtheora-0.dll avcodec-52.dll avutil-50.dll libogg-0.dll libsigc-2.0-0.dll ; do \
		install -m 755 $(BINDIR)/$$lib $(EKIGA_PREFIX) ;\
	done
ifeq ($(DEBUG),0)
	$(STRIP) $(EKIGA_PREFIX)/*.dll
	$(STRIP) $(EKIGA_PREFIX)/*.exe
	$(STRIP) $(EKIGA_PREFIX)/plugins/*.dll
	$(STRIP) $(EKIGA_PREFIX)/plugins/*.exe
endif
	# we cannot strip MS compiled dlls
	install -m 755 $(LIBSASL2_DIR)/bin/libsasl.dll $(EKIGA_PREFIX)

	# for RELEASE=0 ekiga.xml files do not exist, so create them
	for lc in C $(DOC_LINGUAS); do \
		mkdir -p $(EKIGA_PREFIX)/help/$$lc; \
		test $$lc != "C" -a ! -f $(EKIGA_DIR)/help/$$lc/ekiga.xml && xml2po -m docbook -e -p $(EKIGA_DIR)/help/$$lc/$$lc.po $(EKIGA_DIR)/help/C/ekiga.xml >$(EKIGA_DIR)/help/$$lc/ekiga.xml ; \
		if test -f $(EKIGA_DIR)/help/$$lc/ekiga.xml; then \
			xmlto -o $(EKIGA_PREFIX)/help/$$lc/ --skip-validation html $(EKIGA_DIR)/help/$$lc/ekiga.xml; \
		fi ;\
		install -m 644 $(EKIGA_DIR)/help/$$lc/ekiga.xml $(EKIGA_PREFIX)/help/$$lc;\
		if test -d $(EKIGA_DIR)/help/$$lc/figures; then \
			mkdir -p $(EKIGA_PREFIX)/help/$$lc/figures; \
			install -m 644 $(EKIGA_DIR)/help/$$lc/figures/*.png $(EKIGA_PREFIX)/help/$$lc/figures; \
		fi \
	done

	#cd $(TARGETDIR); zip -9r ekiga-$(EKIGA_VER).zip Ekiga
	#zip -9r $(TARGETDIR)/ekiga_build-$(EKIGA_VER).zip Makefile #$(EKIGA_WIN32_DIFF_DIR)
	#tar cfvz $(TARGETDIR)/ekiga_build-$(EKIGA_VER).tgz Makefile #$(EKIGA_WIN32_DIFF_DIR)
	touch $@

# Two additional plugins are needed from nsis:
# - System.dll, included in nsis distribution
# - Processes.dll, not included in nsis distribution
update-sources::
	@echo --- Getting NSIS Installer ...
	$(WGET) -P $(SRCDIR) $(NSIS_URL)/nsis-$(NSIS_VER).zip

$(NSISPLUGIN_DIR)/SystemLocal.dll: $(SRCDIR)/nsis-$(NSIS_VER).zip
	mkdir -p $(NSISPLUGIN_DIR)
	unzip -u -j $(SRCDIR)/nsis-$(NSIS_VER).zip nsis-$(NSIS_VER)/Plugins/System.dll -d $(NSISPLUGIN_DIR)
	$(MV) $(NSISPLUGIN_DIR)/System.dll $(NSISPLUGIN_DIR)/SystemLocal.dll
	touch $@

update-sources::
	@echo --- Getting NSIS Processes Plugin ...
	$(WGET) -P $(SRCDIR) $(NSISPROCESSES_URL)/Processes.zip

$(NSISPLUGIN_DIR)/Processes.dll: $(SRCDIR)/Processes.zip
	mkdir -p $(NSISPLUGIN_DIR)
	unzip -u -j $(SRCDIR)/Processes.zip setup-processes/bin/Processes.dll -d $(NSISPLUGIN_DIR)
	touch $@

$(TARGETDIR)/$(EKIGA_INSTALLER): $(TARGETDIR)/zips $(LIBDIR)/gtk+-$(GTK_BIN_VER)-setup.exe $(NSISPLUGIN_DIR)/SystemLocal.dll $(NSISPLUGIN_DIR)/Processes.dll
	makensis -DEKIGA_VERSION=$(EKIGA_VER) -DEKIGA_DIR=$(EKIGA_DIR) \
		-DINSTALLER_DIR=$(EKIGA_INSTALLER_DIR) -DLIB_DIR=$(LIBDIR) \
		-DTARGET_DIR=$(TARGETDIR) -DWITH_GTK=true -DGTK_VERSION=$(GTK_BIN_VER) \
		-DNSISSYSTEMDIR=$(NSISSYTEM_DIR) -DNSISPLUGINDIR=$(NSISPLUGIN_DIR) \
		-DDEBUG=$(DEBUG) $(EKIGA_INSTALLER_DIR)/ekiga.nsi

clean:
	$(RM) -rf $(TARGETDIR)

distclean:
	$(RM) -rf $(OPENLDAP_DIR)
	$(RM) -rf $(LIBSASL2_DIR)
	$(RM) -rf $(BUILDROOT)/share/
	$(RM) -rf $(BINDIR)
	$(RM) -rf $(BUILDROOT)/contrib
	$(RM) -rf $(BUILDROOT)/etc
	$(RM) -rf $(INCLUDEDIR)
	$(RM) -rf $(LIBDIR)
	$(RM) -rf $(OGG_DIR)
	$(RM) -rf $(XML2_DIR)
	$(RM) -rf $(BUILDROOT)/man
	$(RM) -rf $(BUILDROOT)/manifest
	$(RM) -rf $(OPAL_DIR)
	$(RM) -rf $(EKIGA_DIR)
	$(RM) -rf $(PTLIB_DIR)
	$(RM) -rf $(REGEX_DIR)
	$(RM) -rf $(SIGC_DIR)
	$(RM) -rf $(SPEEX_DIR)
	$(RM) -rf $(EXPAT_DIR)
	$(RM) -rf $(FFMPEG_DIR)
	$(RM) -rf $(X264_DIR)
	$(RM) -rf $(THEORA_DIR)
	$(RM) -rf $(CELT_DIR)
	$(RM) -rf $(NSISPLUGIN_DIR)
	$(RM) -rf make
	$(RM) -f binaries
#	$(RM) -rf $(SRCDIR)

.PHONY: clean distclean update update-sources
