##
# Global definitions
NULL =
SUBDIRS =
AM_CPPFLAGS =					\
	-I$(top_srcdir)/libeasyfc		\
	-I$(top_srcdir)				\
	$(GLIB_CFLAGS)				\
	$(NULL)
DEPS =						\
	$(top_builddir)/libeasyfc/libeasyfc.la	\
	$(NULL)
LIBS =						\
	@LDFLAGS@				\
	$(top_builddir)/libeasyfc/libeasyfc.la	\
	$(NULL)
EXTRA_DIST =					\
	ezfc-tool.py				\
	$(NULL)
TESTS =						\
	$(NULL)


##
# Local definitions
common_private_headers =			\
	$(NULL)
common_sources =				\
	$(NULL)

##
# Local Rules


##
# Target platform
noinst_HEADERS =				\
	$(common_private_headers)		\
	$(NULL)
noinst_PROGRAMS =				\
	test-dump				\
	test-list				\
	test-load				\
	test-save				\
	$(NULL)
#
test_dump_SOURCES =				\
	dump.c					\
	$(NULL)
#
test_list_SOURCES =				\
	list.c					\
	$(NULL)
#
test_load_SOURCES =				\
	load.c					\
	$(NULL)
#
test_save_SOURCES =				\
	save.c					\
	$(NULL)

-include $(top_srcdir)/git.mk
