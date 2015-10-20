#!/bin/sh

srcdir=${srcdir:-.}
. $srcdir/common.inc.sh

abs_srcdir=${abs_srcdir:-`cd "$pwd" 2>/dev/null && pwd -P`}

is_root || skip_all "not root"

prepare 2

echo "hosts: files docker [NOTFOUND=return] dns" > $testtree/etc/nsswitch.conf

$srcdir/chroot.sh $testtree /bin/test-docker-api-server 0 /var/run/docker.sock test 1.2.3.4 >$testtree/mock-server.log 2>&1 &
server_pid=$!

sleep 3
test -S "$testtree/var/run/docker.sock" || not
ok "chroot /var/run/docker.sock socket created" `cat $testtree/mock-server.log 2>&1`

t=`$srcdir/chroot.sh $testtree getent hosts test.docker 2>&1`
test "$t" != "${t#1.2.3.4}" || not
ok "chroot getent hosts test.docker returns" `echo $t | tr -c '[:print:]' ' '`

killall test-docker-api-server || true

test -d "$tmpdir" && rmdir "$tmpdir"

cleanup
