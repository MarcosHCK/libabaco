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

namespace Abaco
{
  [CCode (cheader_filename = "symbol.h", cprefix = "", lower_case_cprefix = "")]
  namespace Symbols
  {
    [CCode (cname = "OperatorAssoc")]
    public enum OperatorAssoc
    {
      LEFT = 0,
      RIGHT,
    }

    [CCode (cname = "OperatorClass")]
    public struct OperatorClass
    {
      public OperatorAssoc assoc;
      public uint precedence;
      public bool unary;
    }

    [CCode (cname = "FunctionClass")]
    public struct FunctionClass
    {
      public int args;
    }

    [CCode (cname = "SymbolKind")]
    public enum SymbolKind
    {
      UNKNOWN = 0,
      PARENTHESIS,
      COMMA,
      CONSTANT,
      VARIABLE,
      OPERATOR,
      FUNCTION,
    }

    [CCode (cname = "SymbolClass")]
    public struct SymbolClass
    {
      public SymbolKind kind;
      public OperatorClass opclass;
      public FunctionClass fnclass;
    }
  }

  [SimpleType]
  [IntegerType (rank = 9)]
  [CCode (cheader_filename = "symbol.h", cname = "Symbol")]
  internal struct Symbol : uintptr
  {
    [CCode (cname = "_symbol_make")]
    public Symbol (uint index, Symbols.SymbolKind kind);
    [CCode (cname = "_symbol_index")]
    public uint get_index ();
    [CCode (cname = "_symbol_kind")]
    public Symbols.SymbolKind get_kind ();

    public uint index { get { return get_index (); } }
    public Symbols.SymbolKind kind { get { return get_kind (); } }
  }
}
