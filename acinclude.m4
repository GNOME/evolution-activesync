dnl as-compiler-flag.m4 0.1.0

dnl autostars m4 macro for detection of compiler flags

dnl David Schleef <ds@schleef.org>

dnl $Id: as-compiler-flag.m4,v 1.1 2005/12/15 23:35:19 ds Exp $

dnl AS_COMPILER_FLAG(CFLAGS, ACTION-IF-ACCEPTED, [ACTION-IF-NOT-ACCEPTED])
dnl Tries to compile with the given CFLAGS.
dnl Runs ACTION-IF-ACCEPTED if the compiler can compile with the flags,
dnl and ACTION-IF-NOT-ACCEPTED otherwise.

AC_DEFUN([AS_COMPILER_FLAG],
[
  AC_MSG_CHECKING([to see if compiler understands $1])

  save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $1"

  AC_TRY_COMPILE([ ], [], [flag_ok=yes], [flag_ok=no])
  CFLAGS="$save_CFLAGS"

  if test "X$flag_ok" = Xyes ; then
    m4_ifvaln([$2],[$2])
    true
  else
    m4_ifvaln([$3],[$3])
    true
  fi
  AC_MSG_RESULT([$flag_ok])
])

dnl AS_COMPILER_FLAGS(VAR, FLAGS)
dnl Tries to compile with the given CFLAGS.

AC_DEFUN([AS_COMPILER_FLAGS],
[
  list=$2
  flags_supported=""
  flags_unsupported=""
  AC_MSG_CHECKING([for supported compiler flags])
  for each in $list
  do
    save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $each"
    AC_TRY_COMPILE([ ], [], [flag_ok=yes], [flag_ok=no])
    CFLAGS="$save_CFLAGS"

    if test "X$flag_ok" = Xyes ; then
      flags_supported="$flags_supported $each"
    else
      flags_unsupported="$flags_unsupported $each"
    fi
  done
  AC_MSG_RESULT([$flags_supported])
  if test "X$flags_unsupported" != X ; then
    AC_MSG_WARN([unsupported compiler flags: $flags_unsupported])
  fi
  $1="$$1 $flags_supported"
])


dnl as-ac-expand.m4 0.2.0                                   -*- autoconf -*-
dnl autostars m4 macro for expanding directories using configure's prefix

dnl (C) 2003, 2004, 2005 Thomas Vander Stichele <thomas at apestaart dot org>

dnl Copying and distribution of this file, with or without modification,
dnl are permitted in any medium without royalty provided the copyright
dnl notice and this notice are preserved.

dnl AS_AC_EXPAND(VAR, CONFIGURE_VAR)

dnl example:
dnl AS_AC_EXPAND(SYSCONFDIR, $sysconfdir)
dnl will set SYSCONFDIR to /usr/local/etc if prefix=/usr/local

AC_DEFUN([AS_AC_EXPAND],
[
  EXP_VAR=[$1]
  FROM_VAR=[$2]

  dnl first expand prefix and exec_prefix if necessary
  prefix_save=$prefix
  exec_prefix_save=$exec_prefix

  dnl if no prefix given, then use /usr/local, the default prefix
  if test "x$prefix" = "xNONE"; then
    prefix="$ac_default_prefix"
  fi
  dnl if no exec_prefix given, then use prefix
  if test "x$exec_prefix" = "xNONE"; then
    exec_prefix=$prefix
  fi

  full_var="$FROM_VAR"
  dnl loop until it doesn't change anymore
  while true; do
    new_full_var="`eval echo $full_var`"
    if test "x$new_full_var" = "x$full_var"; then break; fi
    full_var=$new_full_var
  done

  dnl clean up
  full_var=$new_full_var
  AC_SUBST([$1], "$full_var")

  dnl restore prefix and exec_prefix
  prefix=$prefix_save
  exec_prefix=$exec_prefix_save
])
