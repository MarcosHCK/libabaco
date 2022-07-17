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
#include <symbol.h>

#define KIND_BITS (4)
#define INDEX_BITS ((GLIB_SIZEOF_VOID_P * 8) - KIND_BITS)

typedef union
{
  gpointer ptr;

  struct
  {
    guintptr index : INDEX_BITS;
    guintptr kind : KIND_BITS;
  };
} RealSymbol;

G_STATIC_ASSERT (sizeof (RealSymbol) == sizeof (Symbol));

G_GNUC_INTERNAL
Symbol
_symbol_make (guint index, SymbolKind kind)
{
  RealSymbol sym;
#if DEVELOPER == 1
#if GLIB_SIZEOF_VOID_P == 8
  guintptr max = G_MAXUINT64;
#elif GLIB_SIZEOF_VOID_P == 4
  guintptr max = G_MAXUINT32;
#else // GLIB_SIZEOF_VOID_P
# error "WTF?"
#endif // GLIB_SIZEOF_VOID_P
  g_assert ((max >> INDEX_BITS) >= kind);
  g_assert ((max >> KIND_BITS) >= index);
#endif // DEVELOPER
  sym.index = index;
  sym.kind = kind;
return sym.ptr;
}

G_GNUC_INTERNAL
guint
_symbol_index (Symbol sym_)
{
  RealSymbol sym;
  sym.ptr = sym_;
return sym.index;
}

G_GNUC_INTERNAL
SymbolKind
_symbol_kind (Symbol sym_)
{
  RealSymbol sym;
  sym.ptr = sym_;
return sym.kind;
}
