TESTS =  test-link-module

check_PROGRAMS = test-link-module

test_link_module_SOURCES=test-link-module.c
test_link_module_LDADD=\
	$(top_builddir)/src/libqof/qof/libgnc-qof.la \
        $(top_builddir)/src/app-utils/libgncmod-app-utils.la \
	${top_builddir}/src/gnome-utils/libgncmod-gnome-utils.la \
	${top_builddir}/src/gnome/libgnc-gnome.la \
    ../libgncmod-ledger-core.la

AM_CPPFLAGS = -I${top_srcdir}/src/test-core -I.. ${GLIB_CFLAGS}
