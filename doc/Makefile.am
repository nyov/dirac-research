# $Id: Makefile.am,v 1.1 2005/08/15 15:09:14 asuraparaju Exp $
#

# dirac_api.doxygen.in is not yet in use
EXTRA_DIST = dirac_api.doxygen dirac_api_foot.html dirac_api_head.html


CLEANFILES = html/*

htmldir = $(datadir)/doc/$(GENERIC_LIBRARY_NAME)/code/api_html

html_DATA = html/index.html

html/index.html: dirac_api.doxygen
	SOURCEDIR="$(top_srcdir)" \
	HAVEDOT="$(HAVE_DOT)" \
	doxygen $(srcdir)/dirac_api.doxygen

install-data-hook:
	cp -dR html/* $(DESTDIR)$(htmldir)

