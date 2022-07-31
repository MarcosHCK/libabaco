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
# - src/ast/Makefile.am
girdir=${pkgdatadir}/gir/
typelibdir=${girdir}

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
	libabaco_ast.la \
	$(VOID)

pkginclude_HEADERS=\
	ast.h \
	$(VOID)

#
# Binaries and libraries
# - sources
#

libabaco_ast_la_SOURCES=\
	ast.c \
	$(VOID)
libabaco_ast_la_CFLAGS=\
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	-DG_LOG_DOMAIN=\"AbacoAst\" \
	-D__LIBABACO_AST_INSIDE__=1 \
	$(VOID)
libabaco_ast_la_LIBADD=\
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	$(VOID)
libabaco_ast_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-version-info 0:0:0 \
	$(VOID)

#
# GObject-Introspection
#

include $(INTROSPECTION_MAKEFILE)

GIR_INCLUDES=\
	GLib-2.0 \
	GObject-2.0 \
	$(VOID)

SUFFIXES=.typelib .gir

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
