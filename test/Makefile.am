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

noinst_PROGRAMS=\
	abaco \
	$(VOID)

#
# Binaries and libraries
# - sources
#

abaco_SOURCES=\
	abaco.c \
	$(VOID)
abaco_CFLAGS=\
	$(ABACO_CFLAGS) \
	$(ABACO_MP_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(VOID)
abaco_LDADD=\
	$(ABACO_LIBS) \
	$(ABACO_MP_LIBS) \
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	$(VOID)
