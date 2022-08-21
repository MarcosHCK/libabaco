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
#include <x86_64/jit.h>
#include <x86_64/state.h>

#define ABACO_JITS_X86_64(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), ABACO_JITS_TYPE_X86_64, AbacoJitsX8664))
#define ABACO_JITS_IS_X86_64(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ABACO_JITS_TYPE_X86_64))
typedef struct _AbacoJitsX8664 AbacoJitsX8664;
#define ABACO_JITS_X86_64_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), ABACO_JITS_TYPE_X86_64, AbacoJitsX8664Class))
#define ABACO_JITS_IS_X86_64_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ABACO_JITS_TYPE_X86_64))
#define ABACO_JITS_X86_64_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), ABACO_JITS_TYPE_X86_64, AbacoJitsX8664Class))
typedef struct _AbacoJitsX8664Class AbacoJitsX8664Class;


struct _AbacoJitsX8664
{
  AbacoJit parent;
};

struct _AbacoJitsX8664Class
{
  AbacoJitClass parent;
};

G_DEFINE_FINAL_TYPE (AbacoJitsX8664, abaco_jits_x86_64, ABACO_TYPE_JIT);

static AbacoJitState*
abaco_jits_x86_64_class_new_state (AbacoJit* self, GBytes* code)
{
  return abaco_jits_x86_64_state_new (self, code);
}

static void
abaco_jits_x86_64_class_init (AbacoJitsX8664Class* klass)
{
  AbacoJitClass* jclass = ABACO_JIT_CLASS (klass);
  jclass->new_state = abaco_jits_x86_64_class_new_state;
}

static void
abaco_jits_x86_64_init (AbacoJitsX8664* self)
{
}
