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
	libabaco_jit.la \
	$(VOID)

pkginclude_HEADERS=\
	libabaco_jit.h \
	$(VOID)

#
# Binaries and libraries
# - sources
#

libabaco_jit_la_SOURCES=\
	closure.vala \
	compilers/base.vala \
	compilers/x86_64.vala \
	defaults.vala \
	jit.vala \
	reg.vala \
	relation.vala \
	types/base.vala \
	types/function.vala \
	types/loadable.vala \
	types/mpq.vala \
	types/mpz.vala \
	types/nil.vala \
	types/number.vala \
	$(VOID)
libabaco_jit_la_CFLAGS=\
	$(ABACO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GMP_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(MPFR_CFLAGS) \
	-DG_LOG_DOMAIN=\"AbacoJIT\" \
	-D__LIBABACO_JIT_INSIDE__=1 \
	$(VOID)
libabaco_jit_la_LIBADD=\
	$(ABACO_LIBS) \
	$(GLIB_LIBS) \
	$(GMP_LIBS) \
	$(GOBJECT_LIBS) \
	$(MPFR_LIBS) \
	$(VOID)
libabaco_jit_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-version-info 0:0:0 \
	$(VOID)

#
# Vala
#

AM_VALAFLAGS=\
	--vapidir=../abaco/ \
	--vapidir=vapi/ \
	--vapi-comments \
	--hide-internal \
	--abi-stability \
	--header libabaco_jit.h \
	--library libabaco_jit \
	--pkg config \
	--pkg bytecode \
	--pkg libabaco \
	-D DEBUG=${DEBUG} \
	-D DEVELOPER=${DEVELOPER} \
	$(VOID)
