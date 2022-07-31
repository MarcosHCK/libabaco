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

noinst_LTLIBRARIES=\
	libclosure.la \
	$(VOID)

#
# Binaries and libraries
# - sources
#

libclosure_la_SOURCES=\
	closure.vala \
	$(VOID)
libclosure_la_CFLAGS=\
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	-I../ \
	-DG_LOG_DOMAIN=\"AbacoMP\" \
	-D__LIBABACO_MP_INSIDE__=1 \
	$(VOID)
libclosure_la_LIBADD=\
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	$(VOID)
libclosure_la_LDFLAGS=\
	$(VOID)

#
# Vala
#

AM_VALAFLAGS=\
	--vapidir=vapi/ \
	--vapi-comments \
	--hide-internal \
	--abi-stability \
	--header closure.h \
	--library closure \
	--pkg config \
	-D DEBUG=${DEBUG} \
	-D DEVELOPER=${DEVELOPER} \
	$(VOID)
