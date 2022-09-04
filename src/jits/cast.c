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
#define prepare(reg, tmp, _type) \
  G_STMT_START { \
    tmp = *(reg); \
    (reg)->type = (_type); \
  } G_STMT_END

void
_jit_cast (Reg* reg, gchar type)
{
  Reg tmp;
  switch (reg->type)
  {
  case reg_type_integer:
    switch (type)
    {
    case reg_type_rational:
      prepare (reg, tmp, type);
      mpq_init (reg->rational);
      mpq_set_z (reg->rational, tmp.integer);
      mpz_clear (tmp.integer);
      break;
    case reg_type_real:
      prepare (reg, tmp, type);
      mpfr_init (reg->real);
      mpfr_set_z (reg->real, tmp.integer, round);
      mpz_clear (tmp.integer);
      break;
    }
    break;
  case reg_type_rational:
    switch (type)
    {
    case reg_type_integer:
      prepare (reg, tmp, type);
      mpz_init (reg->integer);
      mpz_set_q (reg->integer, tmp.rational);
      mpq_clear (tmp.rational);
      break;
    case reg_type_real:
      prepare (reg, tmp, type);
      mpfr_init (reg->real);
      mpfr_set_q (reg->real, tmp.rational, round);
      mpq_clear (tmp.rational);
      break;
    }
    break;
  case reg_type_real:
    switch (type)
    {
    case reg_type_integer:
      prepare (reg, tmp, type);
      mpz_init (reg->integer);
      mpfr_get_z (reg->integer, tmp.real, round);
      mpfr_clear (tmp.real);
      break;
    case reg_type_rational:
      prepare (reg, tmp, type);
      mpq_init (reg->rational);
      mpfr_get_q (reg->rational, tmp.real);
      mpfr_clear (tmp.real);
      break;
    }
    break;
  }
}
