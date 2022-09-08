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
#include <libabaco_ucl.h>

#define round (mpfr_get_default_rounding_mode ())

/* hidden API */

UCL_EXPORT gboolean
_ucl_loadq (mpq_t q, const gchar* expr, const gchar* dot, int base);

gboolean
_ucl_loadq (mpq_t q, const gchar* expr, const gchar* dot, int base)
{
  mpz_ptr num = mpq_numref (q);
  mpz_ptr den = mpq_denref (q);
  const long bufsz = 512;
  gchar stat [bufsz + sizeof (void*)];
  gsize length = strlen (expr);
  guint t, partial = 0;
  gchar* next = NULL;
  gchar* buf = NULL;
  int result;

  partial = length - g_utf8_skip [*(guchar*) dot];
  next = g_utf8_next_char (dot);
  if (partial > bufsz)
    buf = g_malloc (partial + 1);
  else
    buf = & stat [0];

  t = dot - expr;
  memcpy (& buf [0], expr, t);
  memcpy (& buf [t], next, partial - t);
  buf [partial] = '\0';

  result =
  mpz_set_str (num, buf, base);
  if (G_UNLIKELY (result < 0))
  {
    if (buf != & stat [0])
      g_free (buf);
    return FALSE;
  }

  memset (& buf [1], '0', partial - 1);
  buf [partial] = '\0';
  buf [0] = '1';

  result =
  mpz_set_str (den, buf, base);
  if (G_UNLIKELY (result < 0))
  {
    if (buf != & stat [0])
      g_free (buf);
    return FALSE;
  }

  /* canonicalize */

  if (buf != & stat [0])
    g_free (buf);

  mpq_canonicalize (q);
return TRUE;
}

/* public API */

void
ucl_reg_load_ldouble (UclReg* reg, long double value)
{
  ucl_reg_setup (reg, UCL_REG_TYPE_REAL);
  mpfr_set_ld (reg->real, value, round);
}

void
ucl_reg_load_double (UclReg* reg, double value)
{
  ucl_reg_setup (reg, UCL_REG_TYPE_RATIONAL);
  mpq_set_d (reg->rational, value);
}

gboolean
ucl_reg_load_string (UclReg* reg, const gchar* expr, int base)
{
  const gchar* val = expr;

  do
  {
    switch (*val)
    {
    case 0:
      ucl_reg_setup (reg, UCL_REG_TYPE_INTEGER);
      return ! (mpz_set_str (reg->integer, expr, base) < 0);
    case '.':
      ucl_reg_setup (reg, UCL_REG_TYPE_RATIONAL);
      return _ucl_loadq (reg->rational, expr, val, base);
    case '/':
      ucl_reg_setup (reg, UCL_REG_TYPE_RATIONAL);
      if (G_UNLIKELY (mpq_set_str (reg->rational, expr, base) < 0))
        return FALSE;
      mpq_canonicalize (reg->rational);
      return TRUE;
    }

    val = g_utf8_next_char (val);
  } while (val != NULL);
return FALSE;
}

gboolean
ucl_reg_load (UclReg* reg, const GValue* value)
{
  if (G_TYPE_CHECK_VALUE_TYPE (value, G_TYPE_STRING))
  {
    const gchar* val = g_value_get_string (value);
    return ucl_reg_load_string (reg, val, 0);
  } else
  if (G_TYPE_CHECK_VALUE_TYPE (value, G_TYPE_DOUBLE))
  {
    double val = (double)
    g_value_get_double (value);
    ucl_reg_load_double (reg, val);
    return TRUE;
  }
  else
  {
    GValue cast = G_VALUE_INIT;
    g_value_init (&cast, G_TYPE_STRING);
    if (g_value_transform (value, &cast))
      return ucl_reg_load (reg, &cast);
    else
      g_value_unset (&cast);
  }
return FALSE;
}
