AC_PREREQ(2.64)
AC_INIT([nss-docker], [0.1], [dexter@debian.org], [nss-docker], [https://github.com/dex4er/nss-docker/])

AC_CONFIG_SRCDIR([src/libnss_docker.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.10 foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE

AC_SUBST([V])

LT_INIT

AC_CANONICAL_HOST
AC_CANONICAL_BUILD
AC_LANG([C])

# --disable-static by default
AC_DISABLE_STATIC

# --with-libpath
AC_ARG_WITH([libpath],
    [AS_HELP_STRING([--with-libpath@<:@=LIBPATH@:>@],
        [path list where library might be located @<:@default=$pkglibdir@:>@])],
    [with_libpath=$withval],
    [with_libpath=$libdir/$PACKAGE_NAME])
AS_IF([test "x$with_libpath" = x],
    [AC_MSG_FAILURE([invalid libpath specified])])
AC_SUBST(libpath, $with_libpath)

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
AC_PATH_PROG([ENV], [env], [/usr/bin/env], [/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/local/sbin:$PATH])
AC_PATH_PROG([ISCHROOT], [ischroot], [/usr/bin/ischroot], [/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/local/sbin:$PATH])
AC_PATH_PROG([LDCONFIG], [ldconfig], [/sbin/chroot], [/usr/sbin:/sbin:/usr/bin:/bin:/usr/local/bin:/usr/local/sbin:$PATH])
AC_PATH_PROG([LDD], [ldd], [/usr/bin/ldd], [/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/local/sbin:$PATH])
AC_PATH_PROG([PERL], [perl], [/usr/bin/perl], [/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/local/sbin:$PATH])
AC_PATH_PROG([SHELL], [bash ksh sh], [/bin/sh], [/usr/sbin:/sbin:/usr/bin:/bin:/usr/local/bin:/usr/local/sbin:$PATH])

# Checks for compilator features
ACX_CHECK_C_ALIGNOF
ACX_CHECK_C_ATTRIBUTE([constructor])
ACX_CHECK_C_ATTRIBUTE_VISIBILITY

# Checks for libraries.
AC_CHECK_LIB([dl], [dlsym])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(m4_normalize([
    alloca.h
    dirent.h
    dlfcn.h
    errno.h
    fcntl.h
    ftw.h
    glob.h
    libintl.h
    pwd.h
    shadow.h
    signal.h
    stdarg.h
    stdint.h
    stdio.h
    stdlib.h
    string.h
    sys/inotify.h
    sys/mount.h
    sys/param.h
    sys/socket.h
    sys/stat.h
    sys/statfs.h
    sys/statvfs.h
    sys/syscall.h
    sys/times.h
    sys/types.h
    sys/un.h
    sys/vfs.h
    sys/wait.h
    sys/xattr.h
    unistd.h
    utime.h
]))

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for additional typedefs.
AC_CHECK_MEMBERS([struct sockaddr_un.sun_len],,, ACX_INCLUDES_HEADERS([sys/un.h]))
AC_CHECK_MEMBERS([struct _ftsent.fts_fts],,, ACX_INCLUDES_HEADERS([sys/types.h sys/stat.h fts.h]))

# Checks for library functions.
AC_CHECK_FUNCS(m4_normalize([
    bind
    connect
    gethostbyname
    gethostbyname2
    gethostbyname3
]))
AC_FUNC_ALLOCA
ASX_NO_CROSS_COMPILING([AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK])

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
