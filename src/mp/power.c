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

int
abaco_mp_power_sqrt (AbacoVM* vm)
{
  if (!ABACO_IS_MP (vm))
g_error ("Incompatible Virtual Machine");

  AbacoMP* mp = ABACO_MP (vm);
  if (!abaco_mp_isnumber (mp, 0))
    g_error ("Bad argument #0 (integer, rational or real expected, got %s)",
      abaco_mp_typename (mp, 0));

  abaco_mp_pushdouble (mp, 0.5);
  abaco_vm_exchange (vm, 1);
return abaco_mp_power_pow (vm);
}

int
abaco_mp_power_cbrt (AbacoVM* vm)
{
  if (!ABACO_IS_MP (vm))
g_error ("Incompatible Virtual Machine");

  AbacoMP* mp = ABACO_MP (vm);
  if (!abaco_mp_isnumber (mp, 0))
    g_error ("Bad argument #0 (integer, rational or real expected, got %s)",
      abaco_mp_typename (mp, 0));

  abaco_mp_pushstring (mp, "1/3", 10);
  abaco_vm_exchange (vm, 1);
return abaco_mp_power_pow (vm);
}

static inline void
_mp_pow_z (AbacoVM* vm, mpz_ptr expo)
{
  AbacoMP* mp = (AbacoMP*) (gpointer) vm;
  const gchar* basetype = abaco_mp_typename (mp, 1);

  if (mpz_fits_uint_p (expo))
  {
    unsigned int value = mpz_get_ui (expo);
    if (basetype == MP_TYPE_INTEGER)
    {
      mpz_ptr base = abaco_mp_tointeger (mp, 0);
      mpz_pow_ui (base, base, value);
    } else
    if (basetype == MP_TYPE_RATIONAL)
    {
      mpq_ptr base = abaco_mp_torational (mp, 0);
      mpz_ptr num = mpq_numref (base);
      mpz_ptr den = mpq_denref (base);

      mpz_pow_ui (num, num, value);
      mpz_pow_ui (den, den, value);
    } else
    if (basetype == MP_TYPE_REAL)
    {
      mpfr_ptr base = abaco_mp_toreal (mp, 0);
      mpfr_rnd_t round = mpfr_get_default_rounding_mode ();
      mpfr_pow_ui (base, base, value, round);
    }
    else
    {
      g_error ("Fix this!");
      g_assert_not_reached ();
    }
  }
  else
  {
    mpfr_rnd_t round;
    if (basetype == MP_TYPE_INTEGER)
    {
      mpfr_t b;
      mpz_ptr base;

      base = abaco_mp_tointeger (mp, 0);
      round = mpfr_get_default_rounding_mode ();

      mpfr_init (b);
      mpfr_set_z (b, base, round);
      mpfr_pow_z (b, b, expo, round);
      mpfr_get_z (base, b, round);
      mpfr_clear (b);
    }
    else
    if (basetype == MP_TYPE_RATIONAL)
    {
      mpfr_t b;
      mpq_ptr base;

      base = abaco_mp_torational (mp, 0);
      round = mpfr_get_default_rounding_mode ();

      mpfr_init (b);
      mpfr_set_q (b, base, round);
      mpfr_pow_z (b, b, expo, round);
      mpfr_get_q (base, b);
      mpfr_clear (b);
    }
    else
    if (basetype == MP_TYPE_REAL)
    {
      mpfr_ptr base;
      base = abaco_mp_toreal (mp, 0);
      round = mpfr_get_default_rounding_mode ();
      mpfr_pow_z (base, base, expo, round);
    }
    else
    {
      g_error ("Fix this!");
      g_assert_not_reached ();
    }
  }
}

int
abaco_mp_power_pow (AbacoVM* vm)
{
  if (!ABACO_IS_MP (vm))
g_error ("Incompatible Virtual Machine");

  AbacoMP* mp = ABACO_MP (vm);
  if (!abaco_mp_isnumber (mp, 0))
    g_error ("Bad argument #0 (integer, rational or real expected, got %s)",
      abaco_mp_typename (mp, 0));
  if (!abaco_mp_isnumber (mp, 0))
    g_error ("Bad argument #1 (integer, rational or real expected, got %s)",
      abaco_mp_typename (mp, 1));

  const gchar* basetype = abaco_mp_typename (mp, 0);
  const gchar* expotype = abaco_mp_typename (mp, 1);

  if (expotype == MP_TYPE_INTEGER)
  {
    mpz_ptr expo = abaco_mp_tointeger (mp, 1);
    _mp_pow_z (vm, expo);
    abaco_vm_settop (vm, 1);
  }
  else
  if (expotype == MP_TYPE_RATIONAL)
  {
    mpfr_rnd_t round;
    mpfr_ptr base;
    mpq_ptr expo;
    mpfr_t e;

    if (!abaco_mp_cast (mp, 0, MP_TYPE_REAL))
      g_error ("Can't cast base to real");

    round = mpfr_get_default_rounding_mode ();
    expo = abaco_mp_torational (mp, 1);
    base = abaco_mp_toreal (mp, 0);

    mpfr_init (e);
    mpfr_set_q (e, expo, round);
    mpfr_pow (base, base, e, round);
    mpfr_clear (e);

    abaco_vm_settop (vm, 1);
  }
  else
  if (expotype == MP_TYPE_REAL)
  {
    mpfr_rnd_t round;
    mpfr_ptr base;
    mpfr_ptr expo;

    if (!abaco_mp_cast (mp, 0, MP_TYPE_REAL))
      g_error ("Can't cast base to real");

    round = mpfr_get_default_rounding_mode ();
    expo = abaco_mp_toreal (mp, 1);
    base = abaco_mp_toreal (mp, 0);

    mpfr_pow (base, base, expo, round);
    abaco_vm_settop (vm, 1);
  }
  else
  {
    g_error ("Fix this!");
    g_assert_not_reached ();
  }
return 1;
}
