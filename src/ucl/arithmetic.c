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

#define fresh(reg, _type) \
  G_STMT_START { \
    UclReg* __reg = (reg); \
    ucl_reg_clear (__reg); \
    __reg->type = (_type); \
  } G_STMT_END

#define round (mpfr_get_default_rounding_mode ())

#define simple(suffix) \
void \
ucl_arithmetic_##suffix (UclReg* accum, const UclReg* next) \
{ \
  if (accum->type == UCL_REG_TYPE_VOID) \
  { \
    switch (next->type) \
    { \
    case UCL_REG_TYPE_INTEGER: \
      fresh (accum, next->type); \
      mpz_init (accum->integer); \
      mpz_set (accum->integer, next->integer); \
      break; \
    case UCL_REG_TYPE_RATIONAL: \
      fresh (accum, next->type); \
      mpq_init (accum->rational); \
      mpq_set (accum->rational, next->rational); \
      break; \
    case UCL_REG_TYPE_REAL: \
      fresh (accum, next->type); \
      mpfr_init (accum->real); \
      mpfr_set (accum->real, next->real, round); \
      break; \
    default: \
      g_error ("Should be a numeric value"); \
      g_assert_not_reached (); \
      break; \
    } \
  } \
  else \
  { \
    if (next->type < UCL_REG_TYPE_INTEGER \
      || next->type > UCL_REG_TYPE_REAL) \
    { \
      g_error ("Should be a numeric value"); \
      g_assert_not_reached (); \
    } \
 ; \
    if (accum->type >= next->type) \
    { \
      switch (accum->type) \
      { \
      case UCL_REG_TYPE_INTEGER: \
        switch (next->type) \
        { \
        case UCL_REG_TYPE_INTEGER: \
          mpz_##suffix (accum->integer, accum->integer, next->integer); \
          break; \
        case UCL_REG_TYPE_RATIONAL: \
          { \
            mpz_t z; \
            mpz_init (z); \
            mpz_set_q (z, next->rational); \
            mpz_##suffix (accum->integer, accum->integer, z); \
            mpz_clear (z); \
          } \
          break; \
        case UCL_REG_TYPE_REAL: \
          { \
            mpz_t z; \
            mpz_init (z); \
            mpfr_get_z (z, next->real, round); \
            mpz_##suffix (accum->integer, accum->integer, z); \
            mpz_clear (z); \
          } \
          break; \
        } \
        break; \
      case UCL_REG_TYPE_RATIONAL: \
        switch (next->type) \
        { \
        case UCL_REG_TYPE_RATIONAL: \
          mpq_##suffix (accum->rational, accum->rational, next->rational); \
          break; \
        case UCL_REG_TYPE_INTEGER: \
          { \
            mpq_t q; \
            mpq_init (q); \
            mpq_set_z (q, next->integer); \
            mpq_##suffix (accum->rational, accum->rational, q); \
            mpq_clear (q); \
          } \
          break; \
        case UCL_REG_TYPE_REAL: \
          { \
            mpq_t q; \
            mpq_init (q); \
            mpfr_get_q (q, next->real); \
            mpq_##suffix (accum->rational, accum->rational, q); \
            mpq_clear (q); \
          } \
          break; \
        } \
        break; \
      case UCL_REG_TYPE_REAL: \
        switch (next->type) \
        { \
        case UCL_REG_TYPE_INTEGER: \
          mpfr_##suffix##_z (accum->real, accum->real, next->integer, round); \
          break; \
        case UCL_REG_TYPE_RATIONAL: \
          mpfr_##suffix##_q (accum->real, accum->real, next->rational, round); \
          break; \
        case UCL_REG_TYPE_REAL: \
          mpfr_##suffix (accum->real, accum->real, next->real, round); \
          break; \
        } \
        break; \
      } \
    } \
    else \
    { \
      ucl_reg_cast (accum, accum, next->type); \
      ucl_arithmetic_##suffix (accum, next); \
    } \
  } \
}

simple (add);
simple (sub);
simple (mul);

void
ucl_arithmetic_div (UclReg* accum, const UclReg* next)
{
  if (accum->type == UCL_REG_TYPE_VOID)
  {
    switch (next->type)
    {
    case UCL_REG_TYPE_INTEGER:
      fresh (accum, next->type);
      mpz_init (accum->integer);
      mpz_set (accum->integer, next->integer);
      break;
    case UCL_REG_TYPE_RATIONAL:
      fresh (accum, next->type);
      mpq_init (accum->rational);
      mpq_set (accum->rational, next->rational);
      break;
    case UCL_REG_TYPE_REAL:
      fresh (accum, next->type);
      mpfr_init (accum->real);
      mpfr_set (accum->real, next->real, round);
      break;
    default:
      g_error ("Should be a numeric value");
      g_assert_not_reached ();
      break;
    }
  }
  else
  {
    if (next->type < UCL_REG_TYPE_INTEGER
      || next->type > UCL_REG_TYPE_REAL)
    {
      g_error ("Should be a numeric value");
      g_assert_not_reached ();
    }

    if (accum->type >= next->type)
    {
      switch (accum->type)
      {
      case UCL_REG_TYPE_INTEGER:
        ucl_reg_cast (accum, accum, UCL_REG_TYPE_RATIONAL);
        G_GNUC_FALLTHROUGH;
      case UCL_REG_TYPE_RATIONAL:
        switch (next->type)
        {
        case UCL_REG_TYPE_INTEGER:
          {
            mpz_ptr z = mpq_denref (accum->rational);
            mpz_mul (z, z, next->integer);
            mpq_canonicalize (accum->rational);
          }
          break;
        case UCL_REG_TYPE_RATIONAL:
          mpq_div (accum->rational, accum->rational, next->rational);
          break;
        }
        break;
      case UCL_REG_TYPE_REAL:
        switch (next->type)
        {
        case UCL_REG_TYPE_INTEGER:
          mpfr_div_z (accum->real, accum->real, next->integer, round);
          break;
        case UCL_REG_TYPE_RATIONAL:
          mpfr_div_q (accum->real, accum->real, next->rational, round);
          break;
        case UCL_REG_TYPE_REAL:
          mpfr_div (accum->real, accum->real, next->real, round);
          break;
        }
        break;
      }
    }
    else
    {
      ucl_reg_cast (accum, accum, next->type);
      ucl_arithmetic_div (accum, next);
    }
  }
}
