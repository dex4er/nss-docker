AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = man src test

EXTRA_DIST = COPYING LICENSE NEWS.md README.md autogen.sh makedist.sh

prove: all
	cd test && $(MAKE) $(AM_MAKEFLAGS) prove

test: all
	cd test && $(MAKE) $(AM_MAKEFLAGS) test

.PHONY: prove test
