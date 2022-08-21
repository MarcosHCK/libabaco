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

#
# Some variables
#

EXTRA_DIST=
CLEANFILES=
VOID=

#
# Binaries and libraries
# - declare
#

pkglib_LTLIBRARIES=\
	libabaco_jits.la \
	$(VOID)

pkginclude_HEADERS=\
	libabaco_jits.h \
	x86_64/jit.h \
	$(VOID)

#
# Binaries and libraries
# - sources
#

EXTRA_DIST+=\
	reg.h \
	stdlib.h \
	x86_64/state.h \
	$(VOID)

libabaco_jits_la_SOURCES=\
	arithmetics.c \
	cast.c \
	reg.c \
	x86_64/jit.c \
  x86_64/state.c \
	$(VOID)
libabaco_jits_la_CFLAGS=\
	$(ABACO_CFLAGS) \
	$(ABACO_JIT_CFLAGS) \
  $(DYNASM_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GMP_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(MPFR_CFLAGS) \
	-I./ \
	-D__DASC__=1 \
	-DG_LOG_DOMAIN=\"AbacoJITS\" \
  -D__LIBABACO_JIT_INSIDE__=1 \
	-D__LIBABACO_JITS_INSIDE__=1 \
	$(VOID)
libabaco_jits_la_LIBADD=\
	$(ABACO_LIBS) \
	$(ABACO_JIT_LIBS) \
  $(DYNASM_LIBS) \
	$(GLIB_LIBS) \
	$(GMP_LIBS) \
	$(GOBJECT_LIBS) \
	$(MPFR_LIBS) \
	$(VOID)
libabaco_jits_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-version-info 0:0:0 \
	$(VOID)

#
# .dasc.c files support
#

SUFFIXES=.dasc.c .c

AM_DYNASMFLAGS=\
	-D DEBUG=\"${DEBUG}\" \
	-D DEVELOPER=\"${DEVELOPER}\" \
	$(VOID)

.dasc.c.c:
	$(DYNASM) $(AM_DYNASMFLAGS) -o $@ $<
