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
#ifndef __LIBABACO_JITS__
#define __LIBABACO_JITS__ 1
#include <libabaco_jit.h>

#if !defined(JITS_EXPORT)
# if defined(_MSC_VER)
#   define JITS_EXPORT __declspec(dllexport) extern
# elif __GNUC__ >= 4
#   define JITS_EXPORT __attribute__((visibility("default"))) extern
# else // __GNUC__ < 4
#   define JITS_EXPORT extern
# endif // _MSC_VER
#endif // JITS_EXPORT

/* Archs */
#include "x86_64/jit.h"

#if __cplusplus
extern "C" {
#endif // 

JITS_EXPORT void
abaco_jits_arithmetic (AbacoJit* state);
JITS_EXPORT gpointer
abaco_jits_arithmetic_add (AbacoJitState* state, const gchar* expr);
JITS_EXPORT gpointer
abaco_jits_arithmetic_sub (AbacoJitState* state, const gchar* expr);
JITS_EXPORT gpointer
abaco_jits_arithmetic_mul (AbacoJitState* state, const gchar* expr);
JITS_EXPORT gpointer
abaco_jits_arithmetic_div (AbacoJitState* state, const gchar* expr);
JITS_EXPORT void
abaco_jits_power (AbacoJit* jit);
JITS_EXPORT gpointer
abaco_jits_arithmetic_sqrt (AbacoJitState* state, const gchar* expr);
JITS_EXPORT gpointer
abaco_jits_arithmetic_cbrt (AbacoJitState* state, const gchar* expr);
JITS_EXPORT gpointer
abaco_jits_arithmetic_pow (AbacoJitState* state, const gchar* expr);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBABACO_JITS__
