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
#include <internal.h>

#define simple(suffix) \
int \
abaco_mp_arith_##suffix (AbacoVM* vm) \
{ \
  if (!ABACO_IS_MP (vm)) \
g_error ("Incompatible Virtual Machine"); \
 ; \
  AbacoMP* mp = ABACO_MP (vm); \
  gint i, top = abaco_vm_gettop (vm); \
  const gchar* type = NULL; \
  UclReg* accum = NULL; \
  UclReg* next = NULL; \
 ; \
  for (i = 0; i < top; i++) \
  { \
    if (!abaco_mp_isnumber (mp, i)) \
      g_error ("Bad argument #%i (integer, rational or real expected, got %s)", \
        i, abaco_mp_typename (mp, i)); \
 ; \
    if (i == 0) \
    { \
                        abaco_vm_pushvalue (vm,  0); \
      accum = (UclReg*) abaco_mp_tointeger (mp, -1); \
    } \
    else \
    { \
      next = (UclReg*) abaco_mp_tointeger (mp, i); \
                       ucl_arithmetic_##suffix (accum, next); \
    } \
  } \
return 1; \
}

simple (add)
simple (sub)
simple (mul)
simple (div)
