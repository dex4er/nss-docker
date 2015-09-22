# arg_with_string.m4 - syntax sugar for AC_ARG_WITH
#
# Copyright (c) 2015 Piotr Roszatycki <dexter@debian.org>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# ACX_ARG_WITH_STRING([OPTION], [HELP-STRING], [DEFAULT])
# -------------------------------------------------------
AC_DEFUN([ACX_ARG_WITH_STRING], [
    AC_MSG_CHECKING([$2])
    AC_ARG_WITH([$1], [AS_HELP_STRING([--with-$1=$3], [$2])])
    AS_VAR_IF(AS_TR_SH([with_$1]), [], AS_VAR_SET(AS_TR_SH([with_$1]), [$3]))
    AC_MSG_RESULT([$]AS_TR_SH([with_$1]))
    AC_DEFINE_UNQUOTED(AS_TR_CPP([$1]), ["$]AS_TR_SH([with_$1])["], [$2])])
