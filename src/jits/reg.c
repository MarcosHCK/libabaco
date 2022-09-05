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
#include <reg.h>

/* internal API */

void
_jit_load_dot (mpq_t q, const gchar* expr, const gchar* val)
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

  partial = length - g_utf8_skip [*(guchar*) val];
  next = g_utf8_next_char (val);
  if (partial > bufsz)
    buf = g_malloc (partial + 1);
  else
    buf = & stat [0];

  t = val - expr;
  memcpy (& buf [0], expr, t);
  memcpy (& buf [t], next, partial - t);
  buf [partial] = '\0';

  result =
  mpz_set_str (num, buf, 10);
  if (G_UNLIKELY (result < 0))
  {
    if (buf != & stat [0])
      g_free (buf);

    g_error ("can't parse input %s", expr);
    g_assert_not_reached ();
  }

  memset (& buf [1], '0', partial - 1);
  buf [partial] = '\0';
  buf [0] = '1';

  result =
  mpz_set_str (den, buf, 10);
  if (G_UNLIKELY (result < 0))
  {
    if (buf != & stat [0])
      g_free (buf);

    g_error ("Can't parse '%s'", expr);
    g_assert_not_reached ();
  }

  /* canonicalize */

  if (buf != & stat [0])
    g_free (buf);

  mpq_canonicalize (q);
}

/* stdlib */

void
_jit_clean (Reg* reg)
{
  ucl_reg_clear (reg);
}

void
_jit_move (Reg* dst, const Reg* src)
{
  ucl_reg_copy (dst, src);
}

void
_jit_load (Reg* reg, const gchar* expr)
{
  const gchar* val = expr;
  const long bufsz = 512;
  gsize length = strlen (expr);
  gchar stat [bufsz + sizeof (void*)];

  do
  {
    switch (*val)
    {
    case 0:
      ucl_reg_setup (reg, UCL_REG_TYPE_INTEGER);
      mpz_set_str (reg->integer, expr, 10);
      return;
    case '.':
      ucl_reg_setup (reg, UCL_REG_TYPE_RATIONAL);
      _jit_load_dot (reg->rational, expr, val);
      return;
    case '/':
      ucl_reg_setup (reg, UCL_REG_TYPE_RATIONAL);
      mpq_set_str (reg->rational, expr, 10);
      mpq_canonicalize (reg->rational);
      return;
    }

    val = g_utf8_next_char (val);
  } while (val != NULL);
}

gchar*
_jit_save (Reg* reg)
{
  gchar* result = NULL;
  gsize length = 0;

  switch (reg->type)
  {
  case UCL_REG_TYPE_INTEGER:
    length = 2;
    length += mpz_sizeinbase (reg->integer, 10);
    result = g_malloc (length);
    result = mpz_get_str (result, 10, reg->integer);
    break;
  case UCL_REG_TYPE_RATIONAL:
    length = 3;
    length += mpz_sizeinbase (mpq_numref (reg->rational), 10);
    length += mpz_sizeinbase (mpq_denref (reg->rational), 10);
    result = mpq_get_str (g_malloc (length), 10, reg->rational);
    break;
  case UCL_REG_TYPE_REAL:
    {
      mpfr_exp_t exp;
      mpfr_rnd_t mode;
      gsize partial;
      gchar* tmp;

      mode = mpfr_get_default_rounding_mode ();
      tmp = mpfr_get_str (NULL, &exp, 10, 0, reg->real, mode);
      g_assert (tmp != NULL);

      length = strlen (tmp);

      if (exp >= 0)
      {
        partial = ((length > exp) ? length : exp + 1) + 1;
        if (exp != 0)
          result = g_malloc (partial + 1);
        else
        {
          result = g_malloc (partial + 2);
          result [0] = '0';
          ++result;
        }

        memset (result, '0', partial);
        memcpy (& result [      0], & tmp [  0], (length > exp) ?          exp : length);
        memcpy (& result [exp + 1], & tmp [exp], (length > exp) ? length - exp :      0);
        result [partial] = '\0';
        result [exp] = '.';

        if (exp == 0)
          --result;
      }
      else
      {
        exp = -exp;
        partial = length + exp + 2;
        result = g_malloc (partial + 1);

        memset (result, '0', partial);
        memcpy (& result [exp + 2], tmp, length);
        result [partial] = '\0';
        result [1] = '.';
      }

      mpfr_free_str (tmp);
    }
    break;
  default:
    g_error ("can't serialize non-numerical value");
    g_assert_not_reached ();
    break;
  }
return result;
}
