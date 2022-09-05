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

int
abaco_mp_power_sqrt (AbacoVM* vm)
{
  if (!ABACO_IS_MP (vm))
g_error ("Incompatible Virtual Machine");

  AbacoMP* mp = ABACO_MP (vm);
  if (!abaco_mp_isnumber (mp, 0))
    g_error ("Bad argument #0 (integer, rational or real expected, got %s)",
      abaco_mp_typename (mp, 0));

  abaco_mp_pushstring (mp, "1/2", 10);
  abaco_vm_insert (vm, 0);

  UclReg* accum = (UclReg*) abaco_mp_tointeger (mp, 0);
  UclReg* next = (UclReg*) abaco_mp_tointeger (mp, 1);
  ucl_power_pow (accum, next);
  abaco_vm_pop (vm);
return 1;
}

int
abaco_mp_power_cbrt (AbacoVM* vm)
{
  if (!ABACO_IS_MP (vm))
g_error ("Incompatible Virtual Machine");

  AbacoMP* mp = ABACO_MP (vm);
  if (!abaco_mp_isnumber (mp, 0))
    g_error ("Bad argument #0 (integer, rational or real expected, got %s)",
      abaco_mp_typename (mp, 0));

  abaco_mp_pushstring (mp, "1/3", 10);
  abaco_vm_insert (vm, 0);

  UclReg* accum = (UclReg*) abaco_mp_tointeger (mp, 0);
  UclReg* next = (UclReg*) abaco_mp_tointeger (mp, 1);
  ucl_power_pow (accum, next);
  abaco_vm_pop (vm);
return 1;
}

int
abaco_mp_power_pow (AbacoVM* vm)
{
  if (!ABACO_IS_MP (vm))
g_error ("Incompatible Virtual Machine");

  AbacoMP* mp = ABACO_MP (vm);
  if (!abaco_mp_isnumber (mp, 0))
    g_error ("Bad argument #0 (integer, rational or real expected, got %s)",
      abaco_mp_typename (mp, 0));
  if (!abaco_mp_isnumber (mp, 0))
    g_error ("Bad argument #1 (integer, rational or real expected, got %s)",
      abaco_mp_typename (mp, 1));

  UclReg* base = (UclReg*) abaco_mp_tointeger (mp, 0);
  UclReg* exp = (UclReg*) abaco_mp_tointeger (mp, 1);
  ucl_power_pow (exp, base);
return 1;
}
