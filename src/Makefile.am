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
# - src/Makefile.am
girdir=${pkgdatadir}/gir/
typelibdir=${girdir}

#
# Some variables
#

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

noinst_PROGRAMS=\
	abaco \
	$(VOID)

pkglib_LTLIBRARIES=\
	libabaco.la \
	$(VOID)

pkginclude_HEADERS=\
	ast.h \
	libabaco.h \
	$(VOID)

#
# Binaries and libraries
# - sources
#

abaco_SOURCES=\
	abaco.c \
	$(VOID)
abaco_CFLAGS=\
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(VOID)
abaco_LDADD=\
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	libabaco.la \
	$(VOID)

libabaco_la_SOURCES=\
	ast.c \
	libabaco.c \
	rules.vala \
	symbol.c \
	$(VOID)
libabaco_la_CFLAGS=\
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(LIBGMP_CFLAGS) \
	$(LIBMPFR_CFLAGS) \
	-DG_LOG_DOMAIN=\"Abaco\" \
	-D__LIBABACO_INSIDE__=1 \
	$(VOID)
libabaco_la_LIBADD=\
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	$(LIBGMP_LIBS) \
	$(LIBMPFR_LIBS) \
	$(VOID)
libabaco_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-version-info 0:0:0 \
	$(VOID)

#
# Vala
#

AM_VALAFLAGS=\
	--vapidir=vapi/ \
	--vapi-comments \
	--hide-internal \
	--abi-stability \
	--header libabaco.h \
	--library libabaco \
	--pkg config \
	-D DEBUG=${DEBUG} \
	-D DEVELOPER=${DEVELOPER} \
	$(VOID)

#
# .desc.c files support
#

SUFFIXES=.dasc.c .c

DYNASM_CFLAGS=\
	-D DEBUG=\"${DEBUG}\" \
	-D DEVELOPER=\"${DEVELOPER}\" \
	$(VOID)

.dasc.c.c:
	$(DYNASM) $(DYNASM_CFLAGS) -o $@ $<

#
# GObject-Introspection
#

include $(INTROSPECTION_MAKEFILE)

GIR_INCLUDES=\
	Gio-2.0 \
	GLib-2.0 \
	GObject-2.0 \
	$(VOID)

SUFFIXES+=.typelib .gir

.gir.typelib:
	${INTROSPECTION_COMPILER} --output $@ $<

gir_DATA=\
	$(VOID)

typelib_DATA=\
	$(gir_DATA:.gir=.typelib) \
	$(VOID)

CLEANFILES+=\
	$(gir_DATA) \
	$(typelib_DATA) \
	$(VOID)
