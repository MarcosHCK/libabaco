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
#ifndef __LIBABACO_GMP__
#define __LIBABACO_GMP__ 1
#include <libabaco.h>

#if !defined(VALA_EXTERN)
#if defined(_MSC_VER)
#define VALA_EXTERN __declspec(dllexport) extern
#elif __GNUC__ >= 4
#define VALA_EXTERN __attribute__((visibility("default"))) extern
#else
#define VALA_EXTERN extern
#endif
#endif

#define ABACO_TYPE_MP (abaco_mp_get_type ())
#define ABACO_MP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), ABACO_TYPE_MP, AbacoMP))
#define ABACO_IS_MP(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ABACO_TYPE_MP))
typedef struct _AbacoMP AbacoMP;

#define MP_TYPE_NIL (g_intern_static_string ("nil"))
#define MP_TYPE_VALUE (g_intern_static_string ("value"))
#define MP_TYPE_INTEGER (g_intern_static_string ("integer"))
#define MP_TYPE_RATIONAL (g_intern_static_string ("rational"))
#define MP_TYPE_REAL (g_intern_static_string ("real"))

#if __cplusplus
extern "C" {
#endif // __cplusplus

VALA_EXTERN GType
abaco_mp_get_type (void) G_GNUC_CONST;
VALA_EXTERN AbacoVM*
abaco_mp_new (void);
VALA_EXTERN const gchar*
abaco_mp_typename (AbacoMP* self, gint index);
VALA_EXTERN void
abaco_mp_pushdouble (AbacoMP* self, double value);
VALA_EXTERN void
abaco_mp_pushldouble (AbacoMP* self, long double value);
VALA_EXTERN gboolean
abaco_mp_pushstring (AbacoMP* self, const gchar* value, int base);
VALA_EXTERN double
abaco_mp_todouble (AbacoMP* self, gint index);
VALA_EXTERN long double
abaco_mp_toldouble (AbacoMP* self, gint index);
VALA_EXTERN gchar*
abaco_mp_tostring (AbacoMP* self, gint index, int base);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBABACO_GMP__
