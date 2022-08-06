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
#ifndef __MP_VALUE__
#define __MP_VALUE__ 1
#ifndef __LIBABACO_MP_INSIDE__
# error "This is a private header"
#endif // __LIBABACO_MP_INSIDE__
#include <glib.h>
#include <mpfr.h>

#define EXPORT G_GNUC_INTERNAL
typedef struct _MpStack MpStack;

#if __cplusplus
extern "C" {
#endif // __cplusplus

#define _mp_stack_get_length(stack) \
  (G_GNUC_EXTENSION ({ \
    gpointer __stack = (stack); \
    GArray* __array = __stack; \
    __array->len; \
  }))

EXPORT MpStack*
_mp_stack_new ();
EXPORT gpointer
_mp_stack_ref (gpointer stack);
EXPORT void
_mp_stack_unref (gpointer stack);
EXPORT const gchar*
_mp_stack_type (MpStack* stack, int index);
EXPORT void
_mp_stack_transfer (MpStack* dst, MpStack* src);
EXPORT void
_mp_stack_push_nil (MpStack* stack);
EXPORT void
_mp_stack_push_index (MpStack* stack, int index);
EXPORT void
_mp_stack_insert (MpStack* stack, int index);
EXPORT void
_mp_stack_remove (MpStack* stack, int index);
EXPORT void
_mp_stack_push_value (MpStack* stack, const GValue* value);
EXPORT gboolean
_mp_stack_push_string (MpStack* stack, const gchar* value, int base);
EXPORT void
_mp_stack_push_double (MpStack* stack, double value);
EXPORT void
_mp_stack_push_ldouble (MpStack* stack, long double value);
EXPORT void
_mp_stack_peek_value (MpStack* stack, int index, GValue* value);
EXPORT gchar*
_mp_stack_peek_string (MpStack* stack, int index, int base);
EXPORT double
_mp_stack_peek_double (MpStack* stack, int index);
EXPORT long double
_mp_stack_peek_ldouble (MpStack* stack, int index);
EXPORT void
_mp_stack_pop (MpStack* stack, guint count);

#if __cplusplus
}
#endif // __cplusplus

#endif // __MP_VALUE__
