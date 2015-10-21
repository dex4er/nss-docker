#!/bin/sh

srcdir=${srcdir:-.}
. $srcdir/common.inc.sh

abs_srcdir=${abs_srcdir:-`cd "$pwd" 2>/dev/null && pwd -P`}

is_root || skip_all "not root"

prepare 2

$srcdir/chroot.sh $testtree /bin/test-docker-api-server 1 /tmp/sock test 1.2.3.4 >$testtree/mock-server.log 2>&1 &
server_pid=$!

sleep 3
test -S "$testtree/tmp/sock" || not
ok "chroot /tmp/sock socket created" `cat $testtree/mock-server.log 2>&1`

t=`$srcdir/chroot.sh $testtree /bin/test-docker-api-client /tmp/sock 1.19 test 2>&1`
test "$t" != "${t#HTTP/1.0 200 OK}" || not
ok "chroot docker api mock server returns" `echo $t | tr -c '[:print:]' ' '`

killall -q test-docker-api-server || true

test -d "$tmpdir" && rmdir "$tmpdir"

cleanup
