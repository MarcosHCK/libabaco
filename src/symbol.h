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
#ifndef __SYMBOL__
#define __SYMBOL__ 1
#ifndef __LIBABACO_INSIDE__
# error "Private header"
#endif // __LIBABACO_INSIDE__
#include <glib.h>

typedef struct _OperatorClass OperatorClass;
typedef struct _FunctionClass FunctionClass;
typedef struct _SymbolClass SymbolClass;
typedef gpointer Symbol;

typedef enum
{
  SYMBOL_KIND_UNKNOWN = 0,
  SYMBOL_KIND_PARENTHESIS,
  SYMBOL_KIND_COMMA,
  SYMBOL_KIND_CONSTANT,
  SYMBOL_KIND_VARIABLE,
  SYMBOL_KIND_OPERATOR,
  SYMBOL_KIND_FUNCTION,
} SymbolKind;

typedef enum
{
  OPERATOR_ASSOC_LEFT = 0,
  OPERATOR_ASSOC_RIGHT,
} OperatorAssoc;

struct _OperatorClass
{
  OperatorAssoc assoc : 1;
  guint precedence : 6;
  guint unary : 1;
};

struct _FunctionClass
{
  gint args;
};

struct _SymbolClass
{
  SymbolKind kind;

  union
  {
    OperatorClass opclass;
    FunctionClass fnclass;
  };
};

#if __cplusplus
extern "C" {
#endif // __cplusplus

G_GNUC_INTERNAL
Symbol
_symbol_make (guint index, SymbolKind kind);
G_GNUC_INTERNAL
guint
_symbol_index (Symbol sym);
G_GNUC_INTERNAL
SymbolKind
_symbol_kind (Symbol sym);

#if __cplusplus
}
#endif // __cplusplus

#endif // __SYMBOL__
