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
#ifndef __JITS_REG__
#define __JITS_REG__ 1
#include <libabaco_jit.h>
#include <mpfr.h>

#define regsz (sizeof (Reg))
typedef struct _Reg Reg;

#if __cplusplus
extern "C" {
#endif // __cplusplus

struct _Reg
{
  union
  {
    gpointer addr;
    mpz_t integer;
    mpq_t rational;
    mpfr_t real;
  };

  union
  {
    struct
    {
      guchar type;
      guchar shadow : 1;
    };

    gpointer padding;
  };
};

enum
{
  reg_type_void = 0,
  reg_type_pointer,
  reg_type_integer,
  reg_type_rational,
  reg_type_real,
};

/* internal API */

G_GNUC_INTERNAL void
_jit_load_dot (mpq_t q, const gchar* expr, const gchar* val);
G_GNUC_INTERNAL void
_jit_cast (Reg* reg, gchar type);

/* stdlib */

G_GNUC_INTERNAL void
_jit_clean (Reg* reg);
G_GNUC_INTERNAL void
_jit_move (Reg* reg1, const Reg* reg2);
G_GNUC_INTERNAL void
_jit_load (Reg* reg, const gchar* expr);
G_GNUC_INTERNAL gchar*
_jit_save (Reg* reg);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JITS_REG__
