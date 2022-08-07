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
#include <libabaco.h>
#include <libabaco_mp.h>
#include <bytecode.h>
#include <glib.h>

static int
testfunc2 (AbacoVM* vm)
{
  g_print ("(%s): top %i\r\n", G_STRLOC, abaco_vm_gettop (vm));
  g_print ("(%s): type %s\r\n", G_STRLOC, abaco_mp_typename (ABACO_MP (vm), -1));
  g_print ("(%s): value %s\r\n", G_STRLOC, abaco_mp_tostring (ABACO_MP (vm), -1, 10));
  g_print ("\r\n");

  abaco_mp_pushdouble (ABACO_MP (vm), 52);
return 1;
}

static int
testfunc1 (AbacoVM* vm)
{
  g_print ("(%s): top %i\r\n", G_STRLOC, abaco_vm_gettop (vm));
  g_print ("(%s): type %s\r\n", G_STRLOC, abaco_mp_typename (ABACO_MP (vm), -1));
  g_print ("(%s): value %s\r\n", G_STRLOC, abaco_mp_tostring (ABACO_MP (vm), -1, 10));
  g_print ("\r\n");

  abaco_vm_pushupvalue (vm, 0);
  g_print ("(%s): top %i\r\n", G_STRLOC, abaco_vm_gettop (vm));
  g_print ("(%s): type %s\r\n", G_STRLOC, abaco_mp_typename (ABACO_MP (vm), -1));
  g_print ("(%s): value %s\r\n", G_STRLOC, abaco_mp_tostring (ABACO_MP (vm), -1, 10));
  g_print ("\r\n");

  abaco_vm_pushcclosure (vm, testfunc2, 0);
  abaco_mp_pushdouble (ABACO_MP (vm), 76);
  abaco_vm_call (vm, 1);

  g_print ("(%s): top %i\r\n", G_STRLOC, abaco_vm_gettop (vm));
  g_print ("(%s): type %s\r\n", G_STRLOC, abaco_mp_typename (ABACO_MP (vm), -1));
  g_print ("(%s): value %s\r\n", G_STRLOC, abaco_mp_tostring (ABACO_MP (vm), -1, 10));
  g_print ("\r\n");
return 0;
}

static void
dotest (AbacoVM* vm, AbacoMP* mp)
{
  abaco_mp_pushdouble (mp, 17);
  abaco_vm_pushcclosure (vm, testfunc1, 1);
  abaco_mp_pushdouble (mp, 91);
  abaco_vm_call (vm, 1);
  abaco_vm_settop (vm, 0);

  GError* tmp_err = NULL;
  abaco_vm_loadstring (vm, "(8+3)*2", &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
  {
    g_critical
    ("(%s): %s: %i: %s",
     G_STRLOC,
     g_quark_to_string
     (tmp_err->domain),
     tmp_err->code,
     tmp_err->message);
    g_error_free (tmp_err);
    g_assert_not_reached ();
  }

  abaco_vm_call (vm, 0);
}

int
main (int argc, char* argv [])
{
  AbacoVM* vm = abaco_mp_new ();;
  AbacoMP* mp = ABACO_MP (vm);

  dotest (vm, mp);
  g_object_unref (mp);
return 0;
}
