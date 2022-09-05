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
static const UclReg __empty__ = {0};

#define fresh(reg, _type) \
  G_STMT_START { \
    UclReg* __reg = (reg); \
    ucl_reg_clear (__reg); \
    __reg->type = (_type); \
  } G_STMT_END

#if !HAVE_MEMCPY
# define do_save(dst,src,ctype) memcpy ((dst), (src), sizeof (ctype));
#else // !HAVE_MEMCPY
# define do_save(dst,src,ctype) \
  G_STMT_START { \
    guint i; \
    for (i = 0; i < G_N_ELEMENTS (dst); i++) \
      dst [i] = src [i]; \
  } G_STMT_END
#endif // HAVE_MEMCPY

#define save(ctype, member) \
  ctype exp; \
  do_save (exp, accum->member, ctype); \
  *accum = __empty__;

void
ucl_power_pow (UclReg* accum, const UclReg* next)
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
    switch (accum->type)
    {
    case UCL_REG_TYPE_INTEGER:
      {
        save (mpz_t, integer);
        if (mpz_fits_uint_p (exp))
        {
          unsigned int iexp;
          iexp = mpz_get_ui (exp);
          switch (next->type)
          {
          case UCL_REG_TYPE_INTEGER:
            ucl_reg_setup (accum, next->type);
            mpz_pow_ui (accum->integer, next->integer, iexp);
            break;
          case UCL_REG_TYPE_RATIONAL:
            ucl_reg_setup (accum, next->type);
            mpz_pow_ui (mpq_numref (accum->rational), mpq_numref (next->rational), iexp);
            mpz_pow_ui (mpq_denref (accum->rational), mpq_denref (next->rational), iexp);
            mpq_canonicalize (accum->rational);
            break;
          case UCL_REG_TYPE_REAL:
            ucl_reg_setup (accum, next->type);
            mpfr_pow_ui (accum->real, next->real, iexp, round);
            break;
          default:
            mpz_clear (exp);
            g_error ("Should be a numeric value");
            g_assert_not_reached ();
            break;
          }
        }
        else
        {
          ucl_reg_cast (accum, next, UCL_REG_TYPE_REAL);
          mpfr_pow_z (accum->real, next->real, exp, round);
        }

        mpz_clear (exp);
      }
      break;
    case UCL_REG_TYPE_RATIONAL:
      ucl_reg_cast (accum, accum, UCL_REG_TYPE_REAL);
      G_GNUC_FALLTHROUGH;
    case UCL_REG_TYPE_REAL:
      {
        save (mpfr_t, real);
        ucl_reg_cast (accum, next, UCL_REG_TYPE_REAL);
        mpfr_pow (accum->real, accum->real, exp, round);
        mpfr_clear (exp);
      }
      break;
    default:
      g_error ("Should be a numeric value");
      g_assert_not_reached ();
      break;
    }
  }
}
