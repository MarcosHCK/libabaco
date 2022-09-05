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
#ifndef __LIBABACO_UCL__
#define __LIBABACO_UCL__ 1
#include <glib.h>
#include <mpfr.h>

#if !defined(UCL_EXPORT)
# if defined(_MSC_VER)
#   define UCL_EXPORT __declspec(dllexport) extern
# elif __GNUC__ >= 4
#   define UCL_EXPORT __attribute__((visibility("default"))) extern
# else // __GNUC__ < 4
#   define UCL_EXPORT extern
# endif // _MSC_VER
#endif // !UCL_EXPORT

typedef struct _UclReg UclReg;

typedef enum
{
  UCL_REG_TYPE_VOID = 0,
  UCL_REG_TYPE_POINTER,
  UCL_REG_TYPE_INTEGER,
  UCL_REG_TYPE_RATIONAL,
  UCL_REG_TYPE_REAL,
} UclRegType;

#if __cplusplus
extern "C" {
#endif // __cplusplus

struct _UclReg
{
  union
  {
    gpointer pointer;
    mpz_t integer;
    mpq_t rational;
    mpfr_t real;
  };

  UclRegType type;
  guint shadow : 1;
} __attribute__ ((aligned (sizeof (gpointer))));

/*
 * unified.c
 *
 */

UCL_EXPORT void
ucl_reg_setup (UclReg* reg, UclRegType type);
UCL_EXPORT void
ucl_reg_clear (UclReg* reg);
UCL_EXPORT void
ucl_reg_clears (UclReg* regs, guint n_regs);
UCL_EXPORT void
ucl_reg_unset (UclReg* reg);
UCL_EXPORT void
ucl_reg_unsets (UclReg* regs, guint n_regs);
UCL_EXPORT void
ucl_reg_cast (UclReg* reg, const UclReg* from, UclRegType type);
UCL_EXPORT void
ucl_reg_copy (UclReg* reg, const UclReg* from);

/*
 * arithmetic.c
 *
 */

UCL_EXPORT void
ucl_arithmetic_add (UclReg* accum, const UclReg* next);
UCL_EXPORT void
ucl_arithmetic_sub (UclReg* accum, const UclReg* next);
UCL_EXPORT void
ucl_arithmetic_mul (UclReg* accum, const UclReg* next);
UCL_EXPORT void
ucl_arithmetic_div (UclReg* accum, const UclReg* next);

/*
 * power.c
 * 
 */

UCL_EXPORT void
ucl_power_pow (UclReg* accum, const UclReg* next);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBABACO_UCL__
