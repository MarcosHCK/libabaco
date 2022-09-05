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

static const UclReg __empty__ = {0};

/* private API */

static void inline
_reg_setup (UclReg* reg, UclRegType type)
{
  switch (reg->type)
  {
  case UCL_REG_TYPE_VOID:
  case UCL_REG_TYPE_POINTER:
    break;
  case UCL_REG_TYPE_INTEGER:
    mpz_init (reg->integer);
    break;
  case UCL_REG_TYPE_RATIONAL:
    mpq_init (reg->rational);
    break;
  case UCL_REG_TYPE_REAL:
    mpfr_init (reg->real);
    break;
  }

  reg->type = type;
}

static void inline
_reg_unset (UclReg* reg)
{
  switch (reg->type)
  {
  cleanup:
    *reg = __empty__;
    G_GNUC_FALLTHROUGH;
  case UCL_REG_TYPE_POINTER:
    reg->type = UCL_REG_TYPE_VOID;
    break;
  case UCL_REG_TYPE_INTEGER:
    mpz_clear (reg->integer);
    goto cleanup;
  case UCL_REG_TYPE_RATIONAL:
    mpq_clear (reg->rational);
    goto cleanup;
  case UCL_REG_TYPE_REAL:
    mpfr_clear (reg->real);
    goto cleanup;
  }
}

#define round (mpfr_get_default_rounding_mode ())

static inline void
_cast (UclReg* reg, const UclReg* from, UclRegType type)
{
  reg->type = type;
  switch (from->type)
  {
  case UCL_REG_TYPE_INTEGER:
    switch (type)
    {
    case UCL_REG_TYPE_RATIONAL:
      mpq_init (reg->rational);
      mpq_set_z (reg->rational, from->integer);
      break;
    case UCL_REG_TYPE_REAL:
      mpfr_init (reg->real);
      mpfr_set_z (reg->real, from->integer, round);
      break;
    }
    break;
  case UCL_REG_TYPE_RATIONAL:
    switch (type)
    {
    case UCL_REG_TYPE_INTEGER:
      mpz_init (reg->integer);
      mpz_set_q (reg->integer, from->rational);
      break;
    case UCL_REG_TYPE_REAL:
      mpfr_init (reg->real);
      mpfr_set_q (reg->real, from->rational, round);
      break;
    }
    break;
  case UCL_REG_TYPE_REAL:
    switch (type)
    {
    case UCL_REG_TYPE_INTEGER:
      mpz_init (reg->integer);
      mpfr_get_z (reg->integer, from->real, round);
      break;
    case UCL_REG_TYPE_RATIONAL:
      mpq_init (reg->rational);
      mpfr_get_q (reg->rational, from->real);
      break;
    }
    break;
  }
}

/* public API */

void
ucl_reg_clear (UclReg* reg)
{
  *reg = __empty__;
}

void
ucl_reg_clears (UclReg* regs, guint n_regs)
{
#if HAVE_MEMSET
  memset (regs, 0, sizeof (UclReg) * n_regs);
#else // !HAVE_MEMSET
  guint i;
  for (i = 0; i < n_regs; i++)
    regs [i] = __empty__;
#endif // HAVE_MEMSET
}

void
ucl_reg_setup (UclReg* reg, UclRegType type)
{
  if (reg->type != type)
  {
    _reg_unset (reg);
    _reg_setup (reg, type);
  }
}

void
ucl_reg_unset (UclReg* reg)
{
  _reg_unset (reg);
}

void
ucl_reg_unsets (UclReg* regs, guint n_regs)
{
  guint i;
  for (i = 0; i < n_regs; i++)
    _reg_unset (& regs [i]);
}

void
ucl_reg_cast (UclReg* reg, const UclReg* from, UclRegType type)
{
  if (reg != from)
    _cast (reg, from, type);
  else
  {
    UclReg tmp;
#if HAVE_MEMCPY
    memcpy (&tmp, from, sizeof (UclReg));
#else // !HAVE_MEMCPY
    tmp = *from;
#endif // HAVE_MEMCPY
#if HAVE_MEMSET
    memset (reg, 0, sizeof (UclReg));
#else // !HAVE_MEMSET
    *reg = __empty__;
#endif // HAVE_MEMSET
    _cast (reg, &tmp, type);
    _reg_unset (&tmp);
  }
}

void
ucl_reg_copy (UclReg* reg, const UclReg* from)
{
  ucl_reg_setup (reg, from->type);

  switch (reg->type)
  {
  case UCL_REG_TYPE_VOID:
    break;
  case UCL_REG_TYPE_POINTER:
    reg->pointer = from->pointer;
    break;
  case UCL_REG_TYPE_INTEGER:
    mpz_set (reg->integer, from->integer);
    break;
  case UCL_REG_TYPE_RATIONAL:
    mpq_set (reg->rational, from->rational);
    break;
  case UCL_REG_TYPE_REAL:
    mpfr_set (reg->real, from->real, round);
    break;
  }
}
