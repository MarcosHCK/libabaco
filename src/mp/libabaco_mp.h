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
#ifndef __LIBABACO_MP__
#define __LIBABACO_MP__ 1
#include <libabaco.h>
#include <mpfr.h>

#if !defined(MP_EXPORT)
#if defined(_MSC_VER)
#define MP_EXPORT __declspec(dllexport) extern
#elif __GNUC__ >= 4
#define MP_EXPORT __attribute__((visibility("default"))) extern
#else
#define MP_EXPORT extern
#endif
#endif

#define ABACO_TYPE_MP (abaco_mp_get_type ())
#define ABACO_MP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), ABACO_TYPE_MP, AbacoMP))
#define ABACO_IS_MP(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ABACO_TYPE_MP))
typedef struct _AbacoMP AbacoMP;

#define MP_TYPE_NIL (__type_nil__ ())
#define MP_TYPE_VALUE (__type_value__ ())
#define MP_TYPE_INTEGER (__type_integer__ ())
#define MP_TYPE_RATIONAL (__type_rational__ ())
#define MP_TYPE_REAL (__type_real__ ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

#define abaco_mp_isinteger(self,index) \
  (G_GNUC_EXTENSION ({ \
    abaco_mp_typename ((self), (index)) == MP_TYPE_INTEGER; \
  }))
#define abaco_mp_isrational(self,index) \
  (G_GNUC_EXTENSION ({ \
    abaco_mp_typename ((self), (index)) == MP_TYPE_RATIONAL; \
  }))
#define abaco_mp_isreal(self,index) \
  (G_GNUC_EXTENSION ({ \
    abaco_mp_typename ((self), (index)) == MP_TYPE_REAL; \
  }))
#define abaco_mp_isnumber(self,index) \
  (G_GNUC_EXTENSION ({ \
    AbacoMP* __self = (self); \
    gint __index = (index); \
    abaco_mp_isinteger (__self, __index) || \
    abaco_mp_isrational (__self, __index) || \
    abaco_mp_isreal (__self, __index); \
  }))

MP_EXPORT GType
abaco_mp_get_type (void) G_GNUC_CONST;
MP_EXPORT AbacoVM*
abaco_mp_new (void);
MP_EXPORT AbacoVM*
abaco_mp_new_naked (void);
MP_EXPORT void
abaco_mp_load_stdlib (AbacoMP* self);
MP_EXPORT const gchar*
abaco_mp_typename (AbacoMP* self, gint index);
MP_EXPORT gboolean
abaco_mp_cast (AbacoMP* self, gint index, const gchar* type);
MP_EXPORT void
abaco_mp_pushdouble (AbacoMP* self, double value);
MP_EXPORT void
abaco_mp_pushldouble (AbacoMP* self, long double value);
MP_EXPORT gboolean
abaco_mp_pushstring (AbacoMP* self, const gchar* value, int base);
MP_EXPORT double
abaco_mp_todouble (AbacoMP* self, gint index);
MP_EXPORT long double
abaco_mp_toldouble (AbacoMP* self, gint index);
MP_EXPORT gchar*
abaco_mp_tostring (AbacoMP* self, gint index, int base);
MP_EXPORT mpz_ptr
abaco_mp_tointeger (AbacoMP* self, gint index);
MP_EXPORT mpq_ptr
abaco_mp_torational (AbacoMP* self, gint index);
MP_EXPORT mpfr_ptr
abaco_mp_toreal (AbacoMP* self, gint index);

/*
 * Types
 *
 */

MP_EXPORT const gchar* __type_nil__ (void) G_GNUC_CONST;
MP_EXPORT const gchar* __type_value__ (void) G_GNUC_CONST;
MP_EXPORT const gchar* __type_integer__ (void) G_GNUC_CONST;
MP_EXPORT const gchar* __type_rational__ (void) G_GNUC_CONST;
MP_EXPORT const gchar* __type_real__ (void) G_GNUC_CONST;

/*
 * Library
 *
 */

MP_EXPORT int
abaco_mp_arith_add (AbacoVM* vm);
MP_EXPORT int
abaco_mp_arith_sub (AbacoVM* vm);
MP_EXPORT int
abaco_mp_arith_mul (AbacoVM* vm);
MP_EXPORT int
abaco_mp_arith_div (AbacoVM* vm);

#if __cplusplus
}
#endif // __cplusplus

#undef MP_EXPORT
#endif // __LIBABACO_MP__
