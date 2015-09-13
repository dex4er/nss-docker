#!/bin/sh

# all paths need to be relative to $srcdir variable
srcdir=${srcdir:-.}

# include common script
. $srcdir/common.inc.sh

# prepare tests. how many tests will be?
prepare 1

# general testing rule:
#     if ! is_root; then
#         skip nnn "not root"
#     else
#         do_something
#         check_if_it_is_ok || not
#         ok "message"
#     fi

echo=${ECHO:-/bin/echo}

if ! is_root; then
    # how many tests we need to skip? usually it is plan/2
    skip $tap_plan "not root"
else

    # echo something
    t=`$srcdir/chroot.sh $testtree $echo something 2>&1`
    # check if it is ok or print "not"
    test "$t" = "something" || not
    # print "ok" message with unquoted test output
    ok "chroot echo:" $t
fi

# clean up temporary directory after tests and end tests
cleanup
