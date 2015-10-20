AC_PREREQ(2.64)
AC_INIT([nss-docker], [0.1], [dexter@debian.org], [nss-docker], [https://github.com/dex4er/nss-docker/])

AC_CONFIG_SRCDIR([src/libnss_docker.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.10 foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE

AC_ARG_VAR([V], [Verbose output])
AC_ARG_VAR([EXTRA_CFLAGS], [Extra C compiler flags])
AC_ARG_VAR([EXTRA_LDFLAGS], [Extra linker flags])

LT_INIT([disable-static])
AC_SUBST([LT_VERSION_INFO], [2:0:0])

AC_CANONICAL_HOST
AC_CANONICAL_BUILD
AC_LANG([C])

# --disable-static by default
AC_DISABLE_STATIC

ACX_ARG_WITH_DEFINE_STRING([docker-socket], [Docker socket path], [/var/run/docker.sock])
ACX_ARG_WITH_DEFINE_STRING([docker-api-version], [Docker API version], [1.12])
ACX_ARG_WITH_DEFINE_STRING([docker-domain-suffix], [Docker domain suffix], [.docker])

# Checks for programs.
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_LN_S
AC_CHECK_PROGS([POD2MAN], [pod2man])
AC_CHECK_PROGS([PROVE], [prove])
AC_CHECK_PROGS([SEQ], [seq gseq])
AC_PATH_PROG([CHROOT], [chroot], [/usr/sbin/chroot], [/usr/sbin:/sbin:/usr/bin:/bin:/usr/local/bin:/usr/local/sbin:$PATH])
AC_PATH_PROG([ECHO], [echo], [/bin/echo], [/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/local/sbin:$PATH])
AC_PATH_PROG([SHELL], [bash ksh sh], [/bin/sh], [/usr/sbin:/sbin:/usr/bin:/bin:/usr/local/bin:/usr/local/sbin:$PATH])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(m4_normalize([
    arpa/inet.h
    assert.h
    errno.h
    limits.h
    netdb.h
    netinet/in.h
    nss.h
    stdio.h
    stdlib.h
    string.h
    sys/socket.h
    sys/types.h
    sys/un.h
    unistd.h
]))

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for additional typedefs.
AC_CHECK_MEMBERS([struct sockaddr_un.sun_len],,, ACX_INCLUDES_HEADERS([sys/un.h]))

# Checks for library functions.
AC_CHECK_FUNCS(m4_normalize([
    bind
    connect
    gethostbyname
    gethostbyname2
    gethostbyname3
    inet_aton
]))

# List of Makefiles
AC_CONFIG_FILES(m4_normalize([
    Makefile
    man/Makefile
    src/Makefile
    test/Makefile
    test/src/Makefile
]))

# prove --failures --exec /bin/sh
ACX_PROG_PROVE_OPT([--failures])
ACX_PROG_PROVE_OPT([--exec /bin/sh])

# Generate output
AC_OUTPUT
