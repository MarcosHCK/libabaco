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

gchar*
ucl_reg_save_string (const UclReg* reg)
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
  }
return result;
}

#define round (mpfr_get_default_rounding_mode ())

long double
ucl_reg_save_ldouble (const UclReg* reg)
{
  long double result = 0;

  switch (reg->type)
  {
  case UCL_REG_TYPE_INTEGER:
  case UCL_REG_TYPE_RATIONAL:
    {
      UclReg cast;
      ucl_reg_cast (&cast, reg, UCL_REG_TYPE_REAL);
      result = mpfr_get_ld (cast.real, round);
      ucl_reg_unset (&cast);
    }
    break;
  case UCL_REG_TYPE_REAL:
    result = mpfr_get_ld (reg->real, round);
    break;
  }
return result;
}

double
ucl_reg_save_double (const UclReg* reg)
{
  switch (reg->type)
  {
  case UCL_REG_TYPE_INTEGER:
    return mpz_get_d (reg->integer);
  case UCL_REG_TYPE_RATIONAL:
    return mpq_get_d (reg->rational);
  case UCL_REG_TYPE_REAL:
    return mpfr_get_d (reg->real, round);
  }
}
