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
#ifndef __JITS_ARITHMETICS__
#define __JITS_ARITHMETICS__ 1
#include <libabaco_jit.h>

#if !defined(EXPORT)
# if defined(_MSC_VER)
#   define EXPORT __declspec(dllexport) extern
# elif __GNUC__ >= 4
#   define EXPORT __attribute__((visibility("default"))) extern
# else // __GNUC__ < 4
#   define EXPORT extern
# endif // _MSC_VER
#endif // EXPORT

#if __cplusplus
extern "C" {
#endif // __cplusplus

EXPORT void
abaco_jits_arithmetics (AbacoJit* state);
EXPORT gpointer
abaco_jits_arithmetics_add (AbacoJitState* state, const gchar* expr);
EXPORT gpointer
abaco_jits_arithmetics_sub (AbacoJitState* state, const gchar* expr);
EXPORT gpointer
abaco_jits_arithmetics_mul (AbacoJitState* state, const gchar* expr);
EXPORT gpointer
abaco_jits_arithmetics_div (AbacoJitState* state, const gchar* expr);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JITS_ARITHMETICS__
