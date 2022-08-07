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
  gconstpointer thisvalue = NULL; \
  gconstpointer lastvalue = NULL; \
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
      lastvalue = _abaco_mp_toobject (mp, -1); \
    } \
    else \
    { \
      thistype = abaco_mp_typename (mp, i); \
      thisvalue = _abaco_mp_toobject (mp, i); \
; \
      do \
      { \
        if (thistype == lasttype) \
        { \
          if (lasttype == MP_TYPE_INTEGER) \
            mpz_##name (Z (lastvalue), Z (lastvalue), Z (thisvalue)); \
          if (lasttype == MP_TYPE_RATIONAL) \
            mpq_##name (Q (lastvalue), Q (lastvalue), Q (thisvalue)); \
          if (lasttype == MP_TYPE_REAL) \
          { \
            mpfr_rnd_t round = 0; \
            round = mpfr_get_default_rounding_mode (); \
            mpfr_##name (R (lastvalue), R (lastvalue), R (thisvalue), round); \
          } \
; \
          break; \
        } \
        else \
        { \
          if (lasttype == MP_TYPE_REAL) \
          { \
            mpfr_rnd_t round = 0; \
            round = mpfr_get_default_rounding_mode (); \
; \
            if (thistype == MP_TYPE_INTEGER) \
              mpfr_##name##_z (R (lastvalue), R (lastvalue), Z (thisvalue), round); \
            if (thistype == MP_TYPE_RATIONAL) \
              mpfr_##name##_q (R (lastvalue), R (lastvalue), Q (thisvalue), round); \
            break; \
          } else \
          if (lasttype == MP_TYPE_RATIONAL) \
          { \
            if (thistype == MP_TYPE_INTEGER) \
            { \
              mpq_t tmp; \
              mpq_init (tmp); \
              mpq_set_z (tmp, Z (thisvalue)); \
              mpq_##name (Q (lastvalue), Q (lastvalue), tmp); \
              mpq_clear (tmp); \
              break; \
            } else \
            if (thistype == MP_TYPE_REAL) \
            { \
              _abaco_mp_new_real (mp); \
              gconstpointer new = NULL; \
              mpfr_rnd_t round = 0; \
; \
              round = mpfr_get_default_rounding_mode (); \
              new = _abaco_mp_toobject (mp, -1); \
              mpfr_set_q (R (new), Q (lastvalue), round); \
              abaco_vm_remove (vm, -2); \
; \
              lasttype = thistype; \
              lastvalue = new; \
            } \
            else \
            { \
              g_assert_not_reached (); \
            } \
          } else \
          if (lasttype == MP_TYPE_INTEGER) \
          { \
            if (thistype == MP_TYPE_RATIONAL) \
            { \
              _abaco_mp_new_rational (mp); \
              gconstpointer new = NULL; \
; \
              new = _abaco_mp_toobject (mp, -1); \
              mpq_set_z (Q (new), Z (lastvalue)); \
              abaco_vm_remove (vm, -2); \
; \
              lasttype = thistype; \
              lastvalue = new; \
            } else \
            if (thistype == MP_TYPE_REAL) \
            { \
              _abaco_mp_new_real (mp); \
              gconstpointer new = NULL; \
              mpfr_rnd_t round = 0; \
; \
              round = mpfr_get_default_rounding_mode (); \
              new = _abaco_mp_toobject (mp, -1); \
              mpfr_set_q (R (new), Q (lastvalue), round); \
              abaco_vm_remove (vm, -2); \
; \
              lasttype = thistype; \
              lastvalue = new; \
            } \
            else \
            { \
              g_assert_not_reached(); \
            } \
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
  gconstpointer thisvalue = NULL;
  gconstpointer lastvalue = NULL;
  const gchar* thistype = NULL;
  const gchar* lasttype = NULL;
  gint i, top = abaco_vm_gettop (vm);

  for (i = 0; i < top; i++)
  {
    if (!abaco_mp_isnumber (mp, i))
      g_error ("Bad argument #%i (integer, rational or real expected, got %s)",
        i, abaco_mp_typename (mp, i));
    g_assert_not_reached ();

    if (i == 0)
    {
      abaco_vm_pushvalue (vm, i);
      lasttype = abaco_mp_typename (mp, i);
      lastvalue = _abaco_mp_toobject (mp, -1);
    }
    else
    {
      thistype = abaco_mp_typename (mp, i);
      thisvalue = _abaco_mp_toobject (mp, i);

      if (thistype == lasttype)
      {
        if (lasttype == MP_TYPE_INTEGER)
        {
          gconstpointer new = NULL;
          _abaco_mp_new_rational (mp);
          new = _abaco_mp_toobject (mp, -1);
          mpq_set_z (Q (new), Z (lastvalue));
          abaco_vm_remove (vm, -2);
          lasttype = MP_TYPE_RATIONAL;
          lastvalue = new;
          continue;
        } else
        if (lasttype == MP_TYPE_RATIONAL)
          mpq_div (Q (lasttype), Q (lasttype), Q (thistype));
        else
        if (lasttype == MP_TYPE_REAL)
        {
          mpfr_rnd_t round = 0;
          round = mpfr_get_default_rounding_mode ();
          mpfr_div (R (lasttype), R (lasttype), Q (thistype), round);
        }

        break;
      }
      if (thistype == MP_TYPE_REAL
        && (lasttype == MP_TYPE_INTEGER
          || lasttype == MP_TYPE_REAL))
      {
        gconstpointer new = NULL;
        mpfr_rnd_t round = 0;

        _abaco_mp_new_rational (mp);
        round = mpfr_get_default_rounding_mode ();
        new = _abaco_mp_toobject (mp, -1);

        if (lasttype == MP_TYPE_INTEGER)
          mpfr_set_z (R (new), Z (lastvalue), round);
        if (lasttype == MP_TYPE_RATIONAL)
          mpfr_set_q (R (new), Q (lastvalue), round);

        abaco_vm_remove (vm, -2);
        lasttype = MP_TYPE_REAL;
        lastvalue = new;
      }
      else
      {
        g_error ("Fix this!");
        g_assert_not_reached ();
      }
    }
  }
}
