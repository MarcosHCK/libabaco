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
#ifndef __LIBABACO_ASSEMBLER__
#define __LIBABACO_ASSEMBLER__ 1
#include <glib-object.h>
#include <ast.h>

#if !defined(VALA_EXTERN)
#if defined(_MSC_VER)
#define VALA_EXTERN __declspec(dllexport) extern
#elif __GNUC__ >= 4
#define VALA_EXTERN __attribute__((visibility("default"))) extern
#else
#define VALA_EXTERN extern
#endif
#endif

#define ABACO_TYPE_ASSEMBLER (abaco_assembler_get_type ())
#define ABACO_ASSEMBLER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), ABACO_TYPE_ASSEMBLER, AbacoAssembler))
#define ABACO_IS_ASSEMBLER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ABACO_TYPE_ASSEMBLER))
typedef struct _AbacoAssembler AbacoAssembler;

#if __cplusplus
extern "C" {
#endif // __cplusplus

VALA_EXTERN GType
abaco_assembler_get_type (void) G_GNUC_CONST;
VALA_EXTERN AbacoAssembler*
abaco_assembler_new ();
VALA_EXTERN GBytes*
abaco_assembler_assemble (AbacoAssembler* assembler, AbacoAstNode* tree, GError** tmp_err);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBABACO_ASSEMBLER__
