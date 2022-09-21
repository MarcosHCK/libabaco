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
valapkgdir=${datarootdir}/vala/vapi

EXTRA_DIST=
CLEANFILES=
VOID=

#
# Subdirectories
#

SUBDIRS=\
	$(VOID)

#
# Binaries and libraries
# - declare
#

pkglib_LTLIBRARIES=\
	libabaco.la \
	$(VOID)

pkginclude_HEADERS=\
	libabaco.h \
	$(VOID)

valapkg_DATA=\
	libabaco.deps \
	libabaco.vapi \
	$(VOID)

#
# Binaries and libraries
# - sources
#

libabaco_la_SOURCES=\
	ast/node.vala \
	libabaco.c \
	$(VOID)
libabaco_la_CFLAGS=\
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	-Iast/ \
	-DG_LOG_DOMAIN=\"Abaco\" \
	-D__LIBABACO_INSIDE__=1 \
	$(VOID)
libabaco_la_LIBADD=\
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	$(VOID)
libabaco_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-version-info 0:0:0 \
	$(VOID)

#
# Vala
#

AM_VALAFLAGS=\
	--vapidir=ast/ \
	--vapidir=vapi/ \
	--vapi-comments \
	--hide-internal \
	--abi-stability \
	--header libabaco.h \
	--library libabaco \
	--pkg config \
	--pkg chain \
	-D DEBUG=${DEBUG} \
	-D DEVELOPER=${DEVELOPER} \
	$(VOID)
