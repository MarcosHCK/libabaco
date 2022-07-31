# Copyright 2021-2025 MarcosHCK
# This file is part of libabaco.
#
# libabaco is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libabaco is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libabaco. If not, see <http://www.gnu.org/licenses/>.
#

AC_PREREQ([2.69])

#
# Take version information
# Extract mayor, minor and micro version
# numbers from version string
# Also define release type
#

m4_define([v_VERSION], m4_esyscmd_s([git describe]))
m4_define([v_MAJOR], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\1/']))
m4_define([v_MINOR], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\2/']))
m4_define([v_MICRO], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\3/']))
m4_define([v_STAGE], m4_esyscmd_s([git describe | sed -e 's/^v\([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)\-\([a-z]*\).*/\4/']))

#
# Initialize autoconf
#

AC_INIT([libabaco], [v_VERSION], [libabaco.bugs@hcktm.com], [libabaco], [https://github.com/MarcosHCK/libabaco/])
AC_CONFIG_SRCDIR([src/abaco/libabaco.c])
AC_LANG([C])

#
# Configure aclocal path
#

AC_CONFIG_MACRO_DIRS([m4])

#
# Setup default cross-compiling
# environment mechanics
#

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

#
# Initialize automake
#

AM_INIT_AUTOMAKE([subdir-objects])

#
# Custom build options
#

AC_ARG_ENABLE(
  [debug],
  [AS_HELP_STRING(
    [--enable-debug],
    [Enable debug object output @<:@default=no@:>@])],
  [AM_CONDITIONAL([DEBUG], [test "x$enableval" != "xno"])
   if test "x$enableval" != "xno"; then
    AC_DEFINE([DEBUG], [1], [Debugging enabled])
    AC_SUBST([DEBUG], [1])
   else
    AC_DEFINE([DEBUG], [0], [Debugging disabled])
    AC_SUBST([DEBUG], [0])
   fi],
  [ AC_DEFINE([DEBUG], [0], [Debugging disabled])
    AC_SUBST([DEBUG], [0])])

AC_ARG_ENABLE(
  [developer],
  [AS_HELP_STRING(
    [--enable-developer],
    [Enable developer features @<:@default=no@:>@])],
  [AM_CONDITIONAL([DEVELOPER], [test "x$enableval" != "xno"])
   if test "x$enableval" != "xno"; then
    AC_DEFINE([DEVELOPER], [1], [Developer features enabled])
    AC_SUBST([DEVELOPER], [1])
   else
    AC_DEFINE([DEVELOPER], [0], [Developer features disabled])
    AC_SUBST([DEVELOPER], [0])
   fi],
  [ AC_DEFINE([DEVELOPER], [0], [Developer features disabled])
    AC_SUBST([DEVELOPER], [0])])

AC_SUBST([PACKAGE_VERSION_MAJOR], [v_MAJOR])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MAJOR], [v_MAJOR], [Version mayor number])
AC_SUBST([PACKAGE_VERSION_MINOR], [v_MINOR])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MINOR], [v_MINOR], [Version minor number])
AC_SUBST([PACKAGE_VERSION_MICRO], [v_MICRO])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MICRO], [v_MICRO], [Version micro number])
AC_SUBST([PACKAGE_RELEASE], ["v_STAGE"])
AC_DEFINE([PACKAGE_RELEASE], ["v_STAGE"], [Release stage])

#
# Checks for programs we'll
# need to build
#

LT_PREREQ([2.4.6])
LT_INIT

AM_PROG_AS
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_VALAC

PKG_PROG_PKG_CONFIG
GOBJECT_INTROSPECTION_REQUIRE([1.68.0])

AC_PATH_PROGS([LUA], [luajit], [no])
if test "x$LUA" = "xno"; then
  AC_PATH_PROGS([LUA], [lua-interpreter], [no], [/etc/alternatives/])
  if test "x$LUA" = "xno"; then
    AC_PATH_PROGS([LUA], [lua lua-any lua5.4 lua5.3 lua5.2 lua5.1])
    if test "x$LUA" = "xno"; then
      AC_MSG_FAILURE([no lua interpreter found])
    else
      AC_SUBST([DYNASM], [\${LUA}\ \${top_builddir}/build/dynasm/dynasm.lua])
    fi
  else
    AC_SUBST([DYNASM], [\${LUA}\ \${top_builddir}/build/dynasm/dynasm.lua])
  fi

  AC_SUBST([LUA_VERSION])
  LUA_VERSION=$($LUA -v | $AWK 'match($0,/@<:@0-9@:>@+\.@<:@0-9@:>@+/) {print substr($0,RSTART,RLENGTH)}')
  AC_SUBST([LUA_VERSION_MAYOR])
  LUA_VERSION_MAYOR=$(echo "$LUA_VERSION" | sed -e 's/\(@<:@0-9@:>@*\)\.\(@<:@0-9@:>@*\)/\1/')
  AC_SUBST([LUA_VERSION_MINOR])
  LUA_VERSION_MINOR=$(echo "$LUA_VERSION" | sed -e 's/\(@<:@0-9@:>@*\)\.\(@<:@0-9@:>@*\)/\1/')

  HAS_BIT=$($LUA -e "if pcall(require,'bit') then print('yes') end")
  if test "x$HAS_BIT" != "xyes"; then
    AC_MSG_FAILURE([please install bit library for version $LUA_VERSION])
  fi
else
  AC_SUBST([DYNASM], [\${LUA}\ \${top_builddir}/build/dynasm/dynasm.lua])
fi

AC_SUBST([ABACO_CFLAGS], [-I\${top_builddir}/src/abaco/])
AC_SUBST([ABACO_LIBS], [\${top_builddir}/src/abaco/libabaco.la])
AC_SUBST([ABACO_AST_CFLAGS], [-I\${top_builddir}/src/abaco/ast/])
AC_SUBST([ABACO_AST_LIBS], [\${top_builddir}/src/abaco/ast/libabaco_ast.la])
AC_SUBST([ABACO_GMP_CFLAGS], [-I\${top_builddir}/src/gmp/])
AC_SUBST([ABACO_GMP_LIBS], [\${top_builddir}/src/gmp/libabaco_gmp.la])

#
# Check for libraries using pkg-config
# Note: keep sorted alphabetically 
#

PKG_CHECK_MODULES([GIO], [gio-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0])
PKG_CHECK_MODULES([LIBGMP], [gmp])
PKG_CHECK_MODULES([LIBMPFR], [mpfr])

#
# Check for libraries
#

AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([gmp], [__gmpz_init])

#
# Checks for header files.
#

#
# Checks for typedefs, structures, and compiler characteristics.
#

AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

#
# Checks for library functions.
#

AC_FUNC_REALLOC
AC_CHECK_FUNCS([memcpy])
AC_CHECK_FUNCS([memset])

#
# Prepare output
#

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
src/Makefile
src/abaco/Makefile
src/abaco/ast/Makefile
src/gmp/Makefile
test/Makefile
Makefile
])

AC_OUTPUT
