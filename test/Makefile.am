SUBDIRS = src

TESTS = \
    t/00echo.t \
    t/socket-af_unix.t \
    #

suffix =

CLEANFILES = .proverc

EXTRA_DIST = $(TESTS) \
    chroot.sh \
    common.inc.sh \
    seq.inc.sh \
    tap.inc.sh \
    testtree.sh \
    touch.inc.sh \
    #

check-src:
	cd src && $(MAKE) $(AM_MAKEFLAGS) check

prove: check-src
	srcdir=$(srcdir) SEQ=$(seq) $(PROVE) $(PROVEFLAGS) $(srcdir)/t

test: check-src
	if [ -n "$(PROVE)" ] && [ "$(PROVE_HAVE_OPT___EXEC__BIN_SH)" = true ]; then \
	    $(MAKE) $(AM_MAKEFLAGS) prove; \
	else \
	    $(MAKE) $(AM_MAKEFLAGS) check-TESTS; \
	fi

.PHONY: check-src prove test
