-include $(INTROSPECTION_MAKEFILE)

##
# Global definitions
NULL =
AM_CPPFLAGS =					\
	-I$(top_builddir)/libeasyfc-gobject/	\
	-I$(top_builddir)			\
	-I$(top_srcdir)/libeasyfc/		\
	-I$(top_srcdir)				\
	-D__EZFC_COMPILATION			\
	$(GOBJECT_CFLAGS)			\
	$(NULL)
DEPS =						\
	$(top_builddir)/libeasyfc/libeasyfc.la	\
	$(NULL)
LIBS =						\
	@LDFLAGS@				\
	$(GOBJECT_LIBS)				\
	$(top_builddir)/libeasyfc/libeasyfc.la	\
	$(NULL)
EXTRA_DIST =					\
	gengir.sh				\
	$(NULL)
MAINTAINERCLEANFILES =					\
	$(stamp_files)					\
	$(NULL)
CLEANFILES =						\
	$(libeasyfc_gobject_built_public_headers)	\
	$(libeasyfc_gobject_built_private_headers)	\
	$(libeasyfc_gobject_built_sources)		\
	xgen-lefh					\
	xgen-lefc					\
	$(NULL)
BUILT_FILES =						\
	$(libeasyfc_gobject_built_public_headers)	\
	$(libeasyfc_gobject_built_private_headers)	\
	$(libeasyfc_gobject_built_sources)		\
	$(NULL)

INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS =			\
	--add-include-path=$(srcdir)		\
	--warn-all				\
	$(NULL)
INTROSPECTION_COMPILER_ARGS =			\
	--includedir=$(srcdir)			\
	$(NULL)


##
# Local definitions
libeasyfc_gobject_public_headers =	\
	ezfc-gobject.h			\
	$(NULL)	
libeasyfc_gobject_private_headers =	\
	$(NULL)
libeasyfc_gobject_built_public_headers =	\
	$(NULL)
libeasyfc_gobject_built_private_headers =	\
	ezfc-alias.gir.h			\
	ezfc-config.gir.h			\
	ezfc-error.gir.h			\
	ezfc-font.gir.h				\
	ezfc-utils.gir.h			\
	$(NULL)
#
libeasyfc_gobject_built_sources =	\
	ezfc.gir.c			\
	ezfc-alias.gir.c		\
	ezfc-config.gir.c		\
	ezfc-font.gir.c			\
	$(NULL)
libeasyfc_gobject_sources =		\
	ezfc-gobject.c			\
	$(NULL)
#
stamp_files =				\
	$(NULL)


##
# Local Rules


##
# Target platform
lib_LTLIBRARIES = 		\
	libeasyfc-gobject.la	\
	$(NULL)
#
libeasyfcincdir = $(includedir)/libeasyfc
libeasyfcinc_HEADERS =					\
	$(libeasyfc_gobject_public_headers)		\
	$(libeasyfc_gobject_built_public_headers)	\
	$(NULL)
#
noinst_HEADERS =					\
	$(libeasyfc_gobject_private_headers)		\
	$(libeasyfc_gobject_built_private_headers)	\
	$(NULL)
#
libeasyfc_gobject_la_SOURCES =				\
	$(libeasyfc_gobject_sources)			\
	$(NULL)
libeasyfc_gobject_la_CFLAGS =				\
	-DG_LOG_DOMAIN="\"EasyFC-GObject\""		\
	$(NULL)
libeasyfc_gobject_la_LDFLAGS =						\
	$(LDFLAGS)							\
	-version-info $(LT_G_CURRENT):$(LT_G_REVISION):$(LT_G_AGE)	\
	$(NULL)
#
if HAVE_INTROSPECTION
ezfc_headers = $(libeasyfc_gobject_private_headers:.gir.h=.h)
ezfc_sources = $(libeasyfc_gobject_built_sources:.gir.c=.c)
introspection_sources =					\
	$(libeasyfc_gobject_built_private_headers)	\
	$(libeasyfc_gobject_built_sources)		\
	$(libeasyfc_gobject_public_headers)		\
	$(libeasyfc_gobject_sources)			\
	$(NULL)

%.gir.h: $(top_builddir)/libeasyfc/libeasyfc.la gengir.sh Makefile
	$(AM_V_GEN) $(srcdir)/gengir.sh $(top_srcdir)/libeasyfc $@
%.gir.c: $(top_builddir)/libeasyfc/libeasyfc.la gengir.sh Makefile
	$(AM_V_GEN) $(srcdir)/gengir.sh $(top_srcdir)/libeasyfc $@

Easyfc-0.12.gir: $(INTROSPECTION_SCANNER) libeasyfc-gobject.la Makefile
Easyfc_0_12_gir_INCLUDES =		\
	fontconfig-2.0			\
	GLib-2.0			\
	GObject-2.0			\
	$(NULL)
Easyfc_0_12_gir_SCANNERFLAGS =		\
	--symbol-prefix=ezfc		\
	--identifier-prefix=Ezfc	\
	-v				\
	$(NULL)
Easyfc_0_12_gir_CFLAGS =		\
	$(AM_CPPFLAGS)			\
	$(NULL)
Easyfc_0_12_gir_LIBS =			\
	libeasyfc-gobject.la		\
	$(NULL)
Easyfc_0_12_gir_FILES =			\
	$(introspection_sources)	\
	$(NULL)

INTROSPECTION_GIRS += Easyfc-0.12.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif

-include $(top_srcdir)/git.mk
