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

PACKAGE_NAME=@PACKAGE_NAME@
PACKAGE_RELEASE=@PACKAGE_RELEASE@
PACKAGE_VERSION_MAJOR=@PACKAGE_VERSION_MAJOR@
PACKAGE_VERSION_MINOR=@PACKAGE_VERSION_MINOR@
PACKAGE_VERSION_MICRO=@PACKAGE_VERSION_MICRO@
PACKAGE_VERSION_BUILD=@PACKAGE_VERSION_BUILD@

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
	libabaco.c \
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
	-release ${PACKAGE_RELEASE} \
	-version-info ${PACKAGE_VERSION_MAJOR}:${PACKAGE_VERSION_MINOR}:${PACKAGE_VERSION_MICRO} \
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

GIR_VERSION=${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}

GIR_INCLUDES=\
	Gio-2.0 \
	GLib-2.0 \
	GObject-2.0 \
	$(VOID)

SUFFIXES+=.typelib .gir

.gir.typelib:
	${INTROSPECTION_COMPILER} --output $@ $<

Matcal-${GIR_VERSION}.gir: sources.list ${pkglib_LTLIBRARIES}
	${INTROSPECTION_SCANNER} \
		--nsversion "${GIR_VERSION}" \
		--library "${pkglib_LTLIBRARIES}" \
		--warn-all \
		--namespace "Matcal" \
		--cflags-begin \
			-I${top_srcdir}/src/ \
		--cflags-end \
		--libtool "$(LIBTOOL)" \
		--sources-top-dirs=${top_srcdir} \
		--include-first-in-src="config.h" \
		$(addprefix --include , ${GIR_INCLUDES}) \
		--filelist $< --output $@ \
		$(VOID)

gir_DATA=\
	$(VOID)

typelib_DATA=\
	$(gir_DATA:.gir=.typelib) \
	$(VOID)

CLEANFILES+=\
	$(gir_DATA) \
	$(typelib_DATA) \
	$(VOID)
