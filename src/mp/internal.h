/* Copyright 2021-2025 MarcosHCK
 * This file is part of libabaco.
 *
 * libabaco is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libabaco is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libabaco.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __MP_INTERNAL__
#define __MP_INTERNAL__ 1
#ifndef __LIBABACO_MP_INSIDE__
# error "This is a private header"
#endif // __LIBABACO_MP_INSIDE__
#include <libabaco_mp.h>
#include <value.h>

#define EXPORT G_GNUC_INTERNAL

#if __cplusplus
extern "C" {
#endif // __cplusplus

EXPORT void
_abaco_mp_new_integer (AbacoMP* self);
EXPORT void
_abaco_mp_new_rational (AbacoMP* self);
EXPORT void
_abaco_mp_new_real (AbacoMP* self);
EXPORT void
_abaco_mp_transfer_to (AbacoMP* self, MpStack* dst);
EXPORT void
_abaco_mp_transfer_from (AbacoMP* self, MpStack* src);
EXPORT const gchar*
_abaco_mp_lookup_constant (AbacoMP* self, const gchar* key);
EXPORT gpointer
_abaco_mp_lookup_function (AbacoMP* self, const gchar* key);

#if __cplusplus
}
#endif // __cplusplus

#undef EXPORT
#endif // __MP_INTERNAL__
