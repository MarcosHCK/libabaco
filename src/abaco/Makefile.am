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
	bytecode.h \
	libabaco.h \
	$(VOID)

#
# Binaries and libraries
# - sources
#

EXTRA_DIST+=\
	astpatch.h \
	symbol.h \
	$(VOID)

libabaco_la_SOURCES=\
	assembler.vala \
	ast.vala \
	bytecode.c \
	libabaco.c \
	parser.vala \
	rules.vala \
	vm.vala \
	$(VOID)
libabaco_la_CFLAGS=\
	$(ABACO_AST_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	-DG_LOG_DOMAIN=\"Abaco\" \
	-D__LIBABACO_INSIDE__=1 \
	$(VOID)
libabaco_la_LIBADD=\
	$(ABACO_AST_LIBS) \
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
	--vapidir=. \
	--vapidir=vapi/ \
	--vapi-comments \
	--hide-internal \
	--abi-stability \
	--header libabaco.h \
	--library libabaco \
	--pkg config \
	--pkg bytecode \
	--pkg patch \
	--pkg symbol \
	-D DEBUG=${DEBUG} \
	-D DEVELOPER=${DEVELOPER} \
	$(VOID)
