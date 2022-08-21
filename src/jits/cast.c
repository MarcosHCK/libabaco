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

#define round (mpfr_get_default_rounding_mode ())
#define move(type, name, from) \
  type##_t name; \
  G_STMT_START { \
    type##_ptr dst = (name); \
    type##_ptr src = (from); \
      *dst = *src; \
  } G_STMT_END
#define fresh(reg, _type) \
  G_STMT_START { \
    *(reg) = __empty__; \
    (reg)->type = (_type); \
  } G_STMT_END

static const Reg __empty__ = {0};

void
_jit_cast (Reg* reg, gchar type)
{
  switch (reg->type)
  {
  case reg_type_integer:
    switch (type)
    {
    case reg_type_integer:
      break;
    case reg_type_rational:
      {
        move (mpz, z, reg->integer);
        fresh (reg, type);
        mpq_init (reg->rational);
        mpq_set_z (reg->rational, z);
        mpz_clear (z);
      }
      break;
    case reg_type_real:
      {
        move (mpz, z, reg->integer);
        fresh (reg, type);
        mpfr_init (reg->real);
        mpfr_set_z (reg->real, z, round);
        mpz_clear (z);
      }
      break;
    }
    break;
  case reg_type_rational:
    switch (type)
    {
    case reg_type_integer:
      {
        move (mpq, q, reg->rational);
        fresh (reg, type);
        mpz_init (reg->integer);
        mpz_set_q (reg->integer, q);
        mpq_clear (q);
      }
      break;
    case reg_type_rational:
      break;
    case reg_type_real:
      {
        move (mpq, q, reg->rational);
        fresh (reg, type);
        mpfr_init (reg->real);
        mpfr_set_q (reg->real, q, round);
        mpq_clear (q);
      }
      break;
    }
    break;
  case reg_type_real:
    switch (type)
    {
    case reg_type_integer:
      {
        move (mpfr, r, reg->real);
        fresh (reg, type);
        mpz_init (reg->integer);
        mpfr_get_z (reg->integer, r, round);
        mpfr_clear (r);
      }
      break;
    case reg_type_rational:
      {
        move (mpfr, r, reg->real);
        fresh (reg, type);
        mpq_init (reg->rational);
        mpfr_get_q (reg->rational, r);
        mpfr_clear (r);
      }
      break;
    case reg_type_real:
      break;
    }
    break;
  }
}
