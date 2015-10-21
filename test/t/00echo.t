#!/bin/sh

# all paths need to be relative to $srcdir variable
srcdir=${srcdir:-.}

# include common script
. $srcdir/common.inc.sh

# do not run tests if is not called as root
is_root || skip_all "not root"

# prepare tests. how many tests will be?
prepare 1

# general testing rule:
#     do_something
#     check_if_it_is_ok || not
#     ok "message"

echo=${ECHO:-/bin/echo}

# echo something
t=`$srcdir/chroot.sh $testtree $echo something 2>&1`
# check if it is ok or print "not"
test "$t" = "something" || not
# print "ok" message with unquoted test output
ok "chroot echo:" $t

# clean up temporary directory after tests and end tests
cleanup
