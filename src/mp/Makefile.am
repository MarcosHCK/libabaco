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

# Keep sync with
# - src/abaco/Makefile.am
# - src/jit/Makefile.am
# - src/jits/Makefile.am
# - src/mp/Makefile.am
# - src/ucl/Makefile.am
valapkgdir=${datarootdir}/vala/vapi

EXTRA_DIST=
CLEANFILES=
VOID=

#
# Subdirectories
#

SUBDIRS=\
	closure \
	$(VOID)

#
# Binaries and libraries
# - declare
#

pkglib_LTLIBRARIES=\
	libabaco_mp.la \
	$(VOID)

pkginclude_HEADERS=\
	libabaco_mp.h \
	$(VOID)

valapkg_DATA=\
	libabaco_mp.deps \
	libabaco_mp.vapi \
	$(VOID)

#
# Binaries and libraries
# - sources
#

EXTRA_DIST+=\
	internal.h \
	value.h \
	$(VOID)

libabaco_mp_la_SOURCES=\
	arith.c \
	execute.c \
	power.c \
	value.c \
	vm.c \
	$(VOID)
libabaco_mp_la_CFLAGS=\
	$(ABACO_CFLAGS) \
	$(ABACO_UCL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GMP_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(MPFR_CFLAGS) \
	-Iclosure/ \
	-DG_LOG_DOMAIN=\"AbacoMP\" \
	-D__LIBABACO_MP_INSIDE__=1 \
	$(VOID)
libabaco_mp_la_LIBADD=\
	$(ABACO_LIBS) \
	$(ABACO_UCL_LIBS) \
	$(GLIB_LIBS) \
	$(GMP_LIBS) \
	$(GOBJECT_LIBS) \
	$(MPFR_LIBS) \
	closure/libclosure.la \
	$(VOID)
libabaco_mp_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-version-info 0:0:0 \
	$(VOID)
