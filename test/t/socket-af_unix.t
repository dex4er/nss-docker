#!/bin/sh

srcdir=${srcdir:-.}
. $srcdir/common.inc.sh

abs_srcdir=${abs_srcdir:-`cd "$pwd" 2>/dev/null && pwd -P`}

prepare 5

test_af_unix () {
    n=$1

    $srcdir/chroot.sh $testtree /bin/test-socket-af_unix-server /chroot-socket$n >$testtree/chroot-socket$n.log 2>&1 &
    server_pid=$!

    sleep 3
    test -S "${FAKECHROOT_AF_UNIX_PATH:-$testtree}/chroot-socket$n" || not
    ok "chroot af_unix server socket created" `cat $testtree/chroot-socket$n.log; ls "${FAKECHROOT_AF_UNIX_PATH:-$testtree}/chroot-socket$n" 2>&1`

    t=`$srcdir/chroot.sh $testtree /bin/test-socket-af_unix-client /chroot-socket$n something 2>&1`
    test "$t" = "something" || not
    ok "chroot af_unix client/server returns" $t

    kill $server_pid 2>/dev/null
}

if ! is_root; then
    skip $tap_plan "not root"
else

    unset FAKECHROOT_AF_UNIX_PATH
    test_af_unix 1

    tmpdir=`src/test-mkdtemp /tmp/chroot-socketXXXXXX 2>&1`
    test -d "$tmpdir" || not
    ok "mkdtemp /tmp/chroot-socketXXXXXX returns $tmpdir"

    export FAKECHROOT_AF_UNIX_PATH="$tmpdir"
    test_af_unix 2

    test -e "$tmpdir/chroot-socket$n" && rm -rf "$tmpdir/chroot-socket$n"
    test -d "$tmpdir" && rmdir "$tmpdir"

fi

cleanup
