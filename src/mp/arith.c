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
#include <config.h>
#include <internal.h>

#define Z(ptr) (*(mpz_t*) ((ptr)))
#define Q(ptr) (*(mpq_t*) ((ptr)))
#define R(ptr) (*(mpfr_t*) ((ptr)))

#define simple(name) \
int \
abaco_mp_arith_##name (AbacoVM* vm) \
{ \
  if (!ABACO_IS_MP (vm)) \
g_error ("Incompatible Virtual Machine"); \
; \
  AbacoMP* mp = ABACO_MP (vm); \
  const gchar* thistype = NULL; \
  const gchar* lasttype = NULL; \
  gint i, top = abaco_vm_gettop (vm); \
; \
  for (i = 0; i < top; i++) \
  { \
    if (!abaco_mp_isnumber (mp, i)) \
      g_error ("Bad argument #%i (integer, rational or real expected, got %s)", \
        i, abaco_mp_typename (mp, i)); \
; \
    if (i == 0) \
    { \
      abaco_vm_pushvalue (vm, i); \
      lasttype = abaco_mp_typename (mp, i); \
    } \
    else \
    { \
      thistype = abaco_mp_typename (mp, i); \
; \
      do \
      { \
        if (thistype == lasttype) \
        { \
          if (lasttype == MP_TYPE_INTEGER) \
          { \
            mpz_ptr dst = abaco_mp_tointeger (mp, -1); \
            mpz_ptr src = abaco_mp_tointeger (mp, i); \
            mpz_##name (dst, dst, src); \
          } \
          if (lasttype == MP_TYPE_RATIONAL) \
          { \
            mpq_ptr dst = abaco_mp_torational (mp, -1); \
            mpq_ptr src = abaco_mp_torational (mp, i); \
            mpq_##name (dst, dst, src); \
          } \
          if (lasttype == MP_TYPE_REAL) \
          { \
            mpfr_ptr dst = abaco_mp_toreal (mp, -1); \
            mpfr_ptr src = abaco_mp_toreal (mp, i); \
            mpfr_rnd_t round = mpfr_get_default_rounding_mode (); \
            mpfr_##name (dst, dst, src, round); \
          } \
; \
          break; \
        } \
        else \
        { \
          if (lasttype == MP_TYPE_REAL) \
          { \
            mpfr_ptr dst = abaco_mp_toreal (mp, -1); \
            mpfr_rnd_t round = mpfr_get_default_rounding_mode (); \
; \
            if (thistype == MP_TYPE_INTEGER) \
            { \
              mpz_ptr src = abaco_mp_tointeger (mp, i); \
              mpfr_##name##_z (dst, dst, src, round); \
            } else \
            if (thistype == MP_TYPE_RATIONAL) \
            { \
              mpq_ptr src = abaco_mp_torational (mp, i); \
              mpfr_##name##_q (dst, dst, src, round); \
            } else g_assert_not_reached (); \
            break; \
          } else \
          if (thistype == MP_TYPE_REAL \
            && (lasttype == MP_TYPE_INTEGER \
             || lasttype == MP_TYPE_RATIONAL)) \
            abaco_mp_cast (mp, -1, (lasttype = MP_TYPE_REAL)); \
          else \
          if (thistype == MP_TYPE_RATIONAL \
            && lasttype == MP_TYPE_INTEGER) \
            abaco_mp_cast (mp, -1, (lasttype = MP_TYPE_RATIONAL)); \
          else \
          if (thistype == MP_TYPE_INTEGER \
            && lasttype == MP_TYPE_RATIONAL) \
          { \
            mpq_t tmp; \
            mpq_ptr dst = abaco_mp_torational (mp, -1); \
            mpz_ptr src = abaco_mp_tointeger (mp, i); \
; \
            mpq_init (tmp); \
            mpq_set_z (tmp, src); \
            mpq_##name (dst, dst, tmp); \
            mpq_clear (tmp); \
            break; \
          } \
          else \
          { \
            g_error ("Fix this!"); \
            g_assert_not_reached (); \
          } \
        } \
      } \
      while (TRUE); \
    } \
  } \
return 1; \
}

simple (add);
simple (sub);
simple (mul);

#undef simple

int
abaco_mp_arith_div (AbacoVM* vm)
{
  if (!ABACO_IS_MP (vm))
g_error ("Incompatible Virtual Machine");

  AbacoMP* mp = ABACO_MP (vm);
  const gchar* thistype = NULL;
  const gchar* lasttype = NULL;
  gint i, top = abaco_vm_gettop (vm);

  for (i = 0; i < top; i++)
  {
    if (!abaco_mp_isnumber (mp, i))
      g_error ("Bad argument #%i (integer, rational or real expected, got %s)",
        i, abaco_mp_typename (mp, i));

    if (i == 0)
    {
      abaco_vm_pushvalue (vm, i);
      lasttype = abaco_mp_typename (mp, i);
    }
    else
    {
      thistype = abaco_mp_typename (mp, i);

      do
      {
        if (lasttype == thistype)
        {
          if (lasttype == MP_TYPE_INTEGER)
          {
            lasttype = MP_TYPE_RATIONAL;
            abaco_mp_cast (mp, -1, lasttype);
            continue;
          } else
          if (lasttype == MP_TYPE_RATIONAL)
          {
            mpq_ptr dst = abaco_mp_torational (mp, -1);
            mpq_ptr src = abaco_mp_torational (mp, i);
            mpq_div (dst, dst, src);
          }
          else
          if (lasttype == MP_TYPE_REAL)
          {
            mpfr_ptr dst = abaco_mp_toreal (mp, -1);
            mpfr_ptr src = abaco_mp_toreal (mp, i);
            mpfr_rnd_t round = mpfr_get_default_rounding_mode ();
            mpfr_div (dst, dst, src, round);
          }

          break;
        } else
        if (lasttype == MP_TYPE_REAL)
        {
          mpfr_ptr dst = abaco_mp_toreal (mp, -1);
          mpfr_rnd_t round = mpfr_get_default_rounding_mode ();

          if (thistype == MP_TYPE_INTEGER)
          {
            mpz_ptr src = abaco_mp_tointeger (mp, i);
            mpfr_div_z (dst, dst, src, round);
          } else
          if (thistype == MP_TYPE_RATIONAL)
          {
            mpq_ptr src = abaco_mp_torational (mp, i);
            mpfr_div_q (dst, dst, src, round);
          } else g_assert_not_reached ();
          break;
        } else
        if (thistype == MP_TYPE_REAL
          && (lasttype == MP_TYPE_INTEGER
           || lasttype == MP_TYPE_RATIONAL))
          abaco_mp_cast (mp, -1, (lasttype = MP_TYPE_REAL));
        else
        if (thistype == MP_TYPE_RATIONAL
          && lasttype == MP_TYPE_INTEGER)
          abaco_mp_cast (mp, -1, (lasttype = MP_TYPE_RATIONAL));
        else
        if (thistype == MP_TYPE_INTEGER
          && lasttype == MP_TYPE_RATIONAL)
        {
          mpq_t tmp;
          mpq_ptr dst = abaco_mp_torational (mp, -1);
          mpz_ptr src = abaco_mp_tointeger (mp, i);

          mpq_init (tmp);
          mpq_set_z (tmp, src);
          mpq_div (dst, dst, tmp);
          mpq_clear (tmp);
          break;
        }
        else
        {
          g_error ("Fix this!");
          g_assert_not_reached ();
        }
      }
      while (TRUE);
    }
  }
return 1;
}
