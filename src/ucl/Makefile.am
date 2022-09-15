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
# Binaries and libraries
# - declare
#

pkglib_LTLIBRARIES=\
	libabaco_ucl.la \
	$(VOID)

pkginclude_HEADERS=\
	libabaco_ucl.h \
	$(VOID)

valapkg_DATA=\
	libabaco_ucl.deps \
	libabaco_ucl.vapi \
	$(VOID)

#
# Binaries and libraries
# - sources
#

libabaco_ucl_la_SOURCES=\
	arithmetic.c \
	load.c \
	power.c \
	save.c \
	unified.c \
	$(VOID)
libabaco_ucl_la_CFLAGS=\
	$(GLIB_CFLAGS) \
  $(GMP_CFLAGS) \
  $(MPFR_CFLAGS) \
	-DG_LOG_DOMAIN=\"AbacoUCL\" \
	-D__LIBABACO_UCL_INSIDE__=1 \
	$(VOID)
libabaco_ucl_la_LIBADD=\
	$(GLIB_LIBS) \
  $(GMP_LIBS) \
  $(MPFR_LIBS) \
	$(VOID)
libabaco_ucl_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-version-info 0:0:0 \
	$(VOID)
