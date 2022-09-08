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
#include "closure.h"

#ifdef G_OS_WINDOWS
# include <windows.h>
#else // !G_OS_WINDOWS
# include <sys/mman.h>
# if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#   define MAP_ANONYMOUS MAP_ANON
# endif // !MAP_ANONYMOUS && MAP_ANON
#endif // G_OS_WINDOWS

#define regsz (sizeof (UclReg))
#include <libabaco_ucl.h>

/* public API */

static void
_closure_notify (gpointer data, GClosure* gc)
{
  Closure* self = (Closure*) gc;
#ifdef G_OS_WINDOWS
  VirtualFree (self->block, self->blocksz, 0);
#else // !G_OS_WINDOWS
  munmap (self->block, self->blocksz);
#endif // G_OS_WINDOWS
}

static void
_closure_marshal (GClosure* closure,
                  GValue* result,
                  guint n_params,
                  const GValue* params,
                  gpointer invocation_hint G_GNUC_UNUSED,
                  gpointer marshal_data)
{
  Closure* cc = (Closure*) closure;
  g_return_if_fail (result != NULL);
  g_return_if_fail (cc->stacksz >= n_params);

  gpointer callback = NULL;
  UclReg stat [1024 / regsz];
  UclReg* stack = NULL;
  guint i;

  callback = (marshal_data ? marshal_data : cc->main);
  if (cc->stacksz > G_N_ELEMENTS (stat))
    stack = g_malloc0 (cc->stacksz * regsz);
  else
  {
    stack = & stat[0];
#if HAVE_MEMSET
    memset (stack, 0, cc->stacksz * regsz);
#else // !HAVE_MEMSET
    static Reg __empty = {0};
    for (i = 0; i < cc->stacksz; i++)
      stack [i] = __empty;
#endif // HAVE_MEMSET
  }

  for (i = 0; i < n_params; i++)
    ucl_reg_load (& stack [i], & params [i]);

#if __x86_64__
# ifdef G_OS_UNIX
#  define arg1 "rdi"
#  define arg2 "rsi"
#  define arg3 "rcx"
# else // !G_OS_UNIX
#  define arg1 "rcx"
#  define arg2 "rdx"
#  define arg3 "r8"
# endif // G_OS_UNIX

  asm
  (
    "push %%rbp \r\n"
    "movq %1, %%rbp \r\n"
    "movq $0, %%" arg1 "\r\n"
    "movq $0, %%" arg2 "\r\n"
    "movsxl %2, %%" arg3 "\r\n"
    "call *%%rax" "\r\n"
    "pop %%rbp" "\r\n"
    :
    : "a" (callback),
      "r" (stack),
      "r" (n_params)
    : "%" arg1,
      "%" arg2,
      "%" arg3);

# undef arg3
# undef arg2
# undef arg1
#else
# error "Unimplemented architecture"
#endif // __x86_64__

  g_value_take_string (result, ucl_reg_save_string (stack));
  ucl_reg_unsets (stack, cc->stacksz);
  if (stack != & stat[0])
    g_free (stack);
}

Closure*
abaco_jits_closure_new (gsize blocksz)
{
  Closure* cc = NULL;
  GClosure* gc = NULL;
  gpointer block = NULL;

  gc = g_closure_new_simple (sizeof (Closure), NULL);
  cc = (Closure*) gc;

  g_closure_add_finalize_notifier (gc, NULL, _closure_notify);
  g_closure_set_marshal (gc, _closure_marshal);

  cc->blocksz = blocksz;
#ifdef G_OS_WINDOWS
  cc->block = VirtualAlloc (0, blocksz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else // !G_OS_WINDOWS
  cc->block = mmap (0, blocksz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif // G_OS_WINDOWS

  if (gc->floating)
  {
    g_closure_ref (gc);
    g_closure_sink (gc);
  }
return cc;
}

void
abaco_jits_closure_prepare (Closure* cc)
{
  g_return_if_fail (cc != NULL);
#ifdef G_OS_WINDOWS
  G_STMT_START {
    DWORD dwOld;
    VirtualProtect (cc->block, cc->blocksz, PAGE_EXECUTE_READ, &dwOld);
  } G_STMT_END;
#else // !G_OS_WINDOWS
  mprotect (cc->block, cc->blocksz, PROT_READ | PROT_EXEC);
#endif // G_OS_WINDOWS
}
