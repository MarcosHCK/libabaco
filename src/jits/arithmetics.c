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
#include <arithmetics.h>
#include <x86_64/state.h>
#include <reg.h>

void
abaco_jits_arithmetics (AbacoJit* jit)
{
  AbacoJitRelation* relation = NULL;

  relation = abaco_jit_relation_new (abaco_jits_arithmetics_add);
             abaco_jit_relation_set_name (relation, "+");
  abaco_jit_add_operator (jit, relation, FALSE, 2, FALSE);

  relation = abaco_jit_relation_new (abaco_jits_arithmetics_sub);
             abaco_jit_relation_set_name (relation, "-");
  abaco_jit_add_operator (jit, relation, FALSE, 2, FALSE);

  relation = abaco_jit_relation_new (abaco_jits_arithmetics_mul);
             abaco_jit_relation_set_name (relation, "*");
  abaco_jit_add_operator (jit, relation, FALSE, 3, FALSE);

  relation = abaco_jit_relation_new (abaco_jits_arithmetics_div);
             abaco_jit_relation_set_name (relation, "/");
  abaco_jit_add_operator (jit, relation, FALSE, 3, FALSE);
}

#define round (mpfr_get_default_rounding_mode ())
static const Reg __empty__ = {0};
#define fresh(reg, _type) \
  G_STMT_START { \
    *(reg) = __empty__; \
    (reg)->type = (_type); \
  } G_STMT_END

#define simple_wraps(suffix, inner) \
static void \
_jit_##suffix (Reg* accum, const Reg* next) \
{ \
  if (accum->type == reg_type_void) \
  { \
    switch (next->type) \
    { \
    case reg_type_integer: \
      _jit_clean (accum); \
      fresh (accum, next->type); \
      mpz_init (accum->integer); \
      mpz_set (accum->integer, next->integer); \
      break; \
    case reg_type_rational: \
      _jit_clean (accum); \
      fresh (accum, next->type); \
      mpq_init (accum->rational); \
      mpq_set (accum->rational, next->rational); \
      break; \
    case reg_type_real: \
      _jit_clean (accum); \
      fresh (accum, next->type); \
      mpfr_init (accum->real); \
      mpfr_set (accum->real, next->real, round); \
      break; \
    default: \
      g_error ("Should be a numeric value"); \
      g_assert_not_reached (); \
      break; \
    } \
  } \
  else \
  { \
    if (next->type < reg_type_integer \
      || next->type > reg_type_real) \
    { \
      g_error ("Should be a numeric value"); \
      g_assert_not_reached (); \
    } \
 ; \
    if (accum->type >= next->type) \
    { \
      inner; \
    } \
    else \
    { \
      _jit_cast (accum, next->type); \
      _jit_##suffix (accum, next); \
    } \
  } \
}

#define simple_wrap(suffix) \
  simple_wraps (suffix, \
    switch (accum->type) \
      { \
      case reg_type_integer: \
        switch (next->type) \
        { \
        case reg_type_integer: \
          mpz_##suffix (accum->integer, accum->integer, next->integer); \
          break; \
        case reg_type_rational: \
          { \
            mpz_t z; \
            mpz_init (z); \
            mpz_set_q (z, next->rational); \
            mpz_##suffix (accum->integer, accum->integer, z); \
            mpz_clear (z); \
          } \
          break; \
        case reg_type_real: \
          { \
            mpz_t z; \
            mpz_init (z); \
            mpfr_get_z (z, next->real, round); \
            mpz_##suffix (accum->integer, accum->integer, z); \
            mpz_clear (z); \
          } \
          break; \
        } \
        break; \
      case reg_type_rational: \
        switch (next->type) \
        { \
        case reg_type_rational: \
          mpq_##suffix (accum->rational, accum->rational, next->rational); \
          break; \
        case reg_type_integer: \
          { \
            mpq_t q; \
            mpq_init (q); \
            mpq_set_z (q, next->integer); \
            mpq_##suffix (accum->rational, accum->rational, q); \
            mpq_clear (q); \
          } \
          break; \
        case reg_type_real: \
          { \
            mpq_t q; \
            mpq_init (q); \
            mpfr_get_q (q, next->real); \
            mpq_##suffix (accum->rational, accum->rational, q); \
            mpq_clear (q); \
          } \
          break; \
        } \
        break; \
      case reg_type_real: \
        switch (next->type) \
        { \
        case reg_type_integer: \
          mpfr_##suffix##_z (accum->real, accum->real, next->integer, round); \
          break; \
        case reg_type_rational: \
          mpfr_##suffix##_q (accum->real, accum->real, next->rational, round); \
          break; \
        case reg_type_real: \
          mpfr_##suffix (accum->real, accum->real, next->real, round); \
          break; \
        } \
        break; \
      })

#define simple_compiler(suffix) \
gpointer \
abaco_jits_arithmetics_##suffix (AbacoJitState* state, const gchar* expr) \
{ \
  gpointer lpc = NULL; \
  if (G_TYPE_CHECK_INSTANCE_TYPE (state, ABACO_JITS_TYPE_X86_64_STATE)) \
    lpc = abaco_jits_x86_64_accum_wrap (state, expr, _jit_##suffix ); \
  else \
  { \
    /* TODO: add generic interface */ \
    g_error ("Unknown state architecture"); \
    g_assert_not_reached (); \
  } \
return lpc; \
}

#define simple(suffix) \
  simple_wrap (suffix) \
  simple_compiler (suffix)

simple (add)
simple (sub)
simple (mul)

static void
_jit_div (Reg* accum, const Reg* next)
{
  if (accum->type == reg_type_void)
  {
    switch (next->type)
    {
    case reg_type_integer:
      _jit_clean (accum);
      fresh (accum, next->type);
      mpz_init (accum->integer);
      mpz_set (accum->integer, next->integer);
      break;
    case reg_type_rational:
      _jit_clean (accum);
      fresh (accum, next->type);
      mpq_init (accum->rational);
      mpq_set (accum->rational, next->rational);
      break;
    case reg_type_real:
      _jit_clean (accum);
      fresh (accum, next->type);
      mpfr_init (accum->real);
      mpfr_set (accum->real, next->real, round);
      break;
    default:
      g_error ("Should be a numeric value");
      g_assert_not_reached ();
      break;
    }
  }
  else
  {
    if (next->type < reg_type_integer
      || next->type > reg_type_real)
    {
      g_error ("Should be a numeric value");
      g_assert_not_reached ();
    }

    if (accum->type >= next->type)
    {
      switch (accum->type)
      {
      case reg_type_integer:
        _jit_cast (accum, reg_type_rational);
        G_GNUC_FALLTHROUGH;
      case reg_type_rational:
        switch (next->type)
        {
        case reg_type_integer:
          {
            mpz_ptr z = mpq_denref (accum->rational);
            mpz_mul (z, z, next->integer);
            mpq_canonicalize (accum->rational);
          }
          break;
        case reg_type_rational:
          mpq_div (accum->rational, accum->rational, next->rational);
          break;
        }
        break;
      case reg_type_real:
        switch (next->type)
        {
        case reg_type_integer:
          mpfr_div_z (accum->real, accum->real, next->integer, round);
          break;
        case reg_type_rational:
          mpfr_div_q (accum->real, accum->real, next->rational, round);
          break;
        case reg_type_real:
          mpfr_div (accum->real, accum->real, next->real, round);
          break;
        }
        break;
      }
    }
    else
    {
      _jit_cast (accum, next->type);
      _jit_div (accum, next);
    }
  }
}

simple_compiler (div)
