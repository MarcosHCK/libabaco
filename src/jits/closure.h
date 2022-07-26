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
#ifndef __JITS_CLOSURE__
#define __JITS_CLOSURE__ 1
#include <glib-object.h>

#define EXPORT G_GNUC_INTERNAL
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
  int base;
};

typedef void (*BlockFunc) (gpointer stack, guint stacksz, guint n_params);

EXPORT Closure*
abaco_jits_closure_new (gsize blocksz);
EXPORT void
abaco_jits_closure_prepare (Closure* cc);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JITS_CLOSURE__
