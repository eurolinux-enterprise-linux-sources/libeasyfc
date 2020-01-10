##
# Global definitions
NULL =
AM_CPPFLAGS =					\
	-I$(top_srcdir)/libeasyfc/		\
	-I$(top_srcdir)				\
	-D__EZFC_COMPILATION			\
	$(FONTCONFIG_CFLAGS)			\
	$(FREETYPE_CFLAGS)			\
	$(GLIB_CFLAGS)				\
	$(LIBXML2_CFLAGS)			\
	$(HARFBUZZ_CFLAGS)			\
	$(NULL)
LIBS =						\
	@LDFLAGS@				\
	$(FONTCONFIG_LIBS)			\
	$(FREETYPE_LIBS)			\
	$(GLIB_LIBS)				\
	$(LIBXML2_LIBS)				\
	$(HARFBUZZ_LIBS)			\
	$(NULL)
EXTRA_DIST =					\
	$(NULL)
MAINTAINERCLEANFILES =				\
	$(libeasyfc_built_public_headers)	\
	$(libeasyfc_built_private_headers)	\
	$(libeasyfc_built_sources)		\
	$(stamp_files)				\
	$(NULL)
CLEANFILES =					\
	xgen-lefh				\
	xgen-lefc				\
	$(NULL)
BUILT_FILES =					\
	$(libeasyfc_built_public_headers)	\
	$(libeasyfc_built_private_headers)	\
	$(libeasyfc_built_sources)		\
	$(NULL)

##
# Local definitions
libeasyfc_public_headers =			\
	ezfc-alias.h				\
	ezfc-error.h				\
	ezfc-config.h				\
	ezfc-font.h				\
	ezfc-utils.h				\
	$(NULL)	
libeasyfc_private_headers =			\
	ezfc-font-private.h			\
	ezfc-mem.h				\
	$(NULL)
libeasyfc_built_public_headers =		\
	$(NULL)
libeasyfc_built_private_headers =		\
	$(NULL)
#
libeasyfc_built_sources =			\
	$(NULL)
libeasyfc_sources =				\
	$(libeasyfc_built_sources)		\
	ezfc.c					\
	ezfc-alias.c				\
	ezfc-config.c				\
	ezfc-font.c				\
	ezfc-mem.c				\
	$(NULL)
#
stamp_files =					\
	$(NULL)


##
# Local Rules


##
# Target platform
lib_LTLIBRARIES = 				\
	libeasyfc.la				\
	$(NULL)
#
libeasyfcincdir = $(includedir)/libeasyfc
libeasyfcinc_HEADERS =				\
	ezfc.h					\
	$(libeasyfc_public_headers)		\
	$(libeasyfc_built_public_headers)	\
	$(NULL)
#
noinst_HEADERS =				\
	$(libeasyfc_private_headers)		\
	$(libeasyfc_built_private_headers)	\
	$(NULL)
#
libeasyfc_la_SOURCES =				\
	$(libeasyfc_sources)			\
	$(NULL)
libeasyfc_la_CFLAGS =				\
	-DG_LOG_DOMAIN="\"EasyFC\""		\
	$(NULL)
libeasyfc_la_LDFLAGS =						\
	$(LDFLAGS)						\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)	\
	$(NULL)

-include $(top_srcdir)/git.mk
