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
#ifndef __X86_64_JIT_STATE__
#define __X86_64_JIT_STATE__ 1
#include <libabaco_jit.h>
#include <reg.h>

#define ABACO_JITS_TYPE_X86_64_STATE (abaco_jits_x86_64_state_get_type ())
typedef struct _Closure Closure;

#if __cplusplus
extern "C" {
#endif // __cplusplus

struct _Closure
{
  GClosure parent;
  gpointer main;
  gpointer block;
  gsize blocksz;
  gsize stacksz;
};

typedef void (*AccumWrap) (Reg* accum, const Reg* next);

/*
 * internal API
 *
 */

G_GNUC_INTERNAL GType
abaco_jits_x86_64_state_get_type (void) G_GNUC_CONST;
G_GNUC_INTERNAL AbacoJitState*
abaco_jits_x86_64_state_new (AbacoJit* back, GBytes* code);
G_GNUC_INTERNAL gboolean
abaco_jits_x86_64_state_getpc (gpointer pself, guint* out_pc, const gchar* key);
G_GNUC_INTERNAL gpointer
abaco_jits_x86_64_accum_wrap (gpointer pself, const gchar* name, gboolean invert, AccumWrap wrap);

#if __cplusplus
}
#endif // __cplusplus

#endif // __X86_64_JIT_STATE__
