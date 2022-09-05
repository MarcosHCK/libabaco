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
#include <libabaco_jits.h>
#include <x86_64/state.h>

#define accum(suffix,invert) \
  gpointer \
  abaco_jits_##suffix (AbacoJitState* state, const gchar* expr) \
  { \
    gpointer lpc = NULL; \
    if (G_TYPE_CHECK_INSTANCE_TYPE (state, ABACO_JITS_TYPE_X86_64_STATE)) \
      lpc = abaco_jits_x86_64_accum_wrap (state, expr, (invert), ucl_##suffix ); \
    else \
    { \
      /* TODO: add generic interface */ \
      g_error ("Unknown state architecture"); \
      g_assert_not_reached (); \
    } \
  return lpc; \
  }

accum (arithmetic_add, FALSE)
accum (arithmetic_sub, FALSE)
accum (arithmetic_mul, FALSE)
accum (arithmetic_div, FALSE)

void
abaco_jits_arithmetic (AbacoJit* jit)
{
  AbacoJitRelation* relation = NULL;

  relation = abaco_jit_relation_new (abaco_jits_arithmetic_add);
             abaco_jit_relation_set_name (relation, "+");
  abaco_jit_add_operator (jit, relation, FALSE, 2, FALSE);

  relation = abaco_jit_relation_new (abaco_jits_arithmetic_sub);
             abaco_jit_relation_set_name (relation, "-");
  abaco_jit_add_operator (jit, relation, FALSE, 2, FALSE);

  relation = abaco_jit_relation_new (abaco_jits_arithmetic_mul);
             abaco_jit_relation_set_name (relation, "*");
  abaco_jit_add_operator (jit, relation, FALSE, 3, FALSE);

  relation = abaco_jit_relation_new (abaco_jits_arithmetic_div);
             abaco_jit_relation_set_name (relation, "/");
  abaco_jit_add_operator (jit, relation, FALSE, 3, FALSE);
}

accum (power_pow, TRUE)

void
abaco_jits_power (AbacoJit* jit)
{
  AbacoJitRelation* relation = NULL;

  relation = abaco_jit_relation_new (abaco_jits_power_pow);
             abaco_jit_relation_set_name (relation, "^");
  abaco_jit_add_operator (jit, relation, TRUE, 4, FALSE);
}
