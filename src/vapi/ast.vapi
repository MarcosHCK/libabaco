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

namespace Abaco.Ast
{
  [CCode (cheader_filename = "ast.h")]
  public enum SymbolKind
  {
    CONSTANT,
    VARIABLE,
    FUNCTION,
  }

  [CCode (cheader_filename = "ast.h")]
  public class Node
  {
    public uint ref_count;
    public Node (string symbol, SymbolKind kind);
    public unowned string get_symbol ();
    public SymbolKind get_kind ();

    public string symbol { get { return get_symbol (); } }
    public SymbolKind kind { get { return get_kind (); } }

    public void append (Node child);
    public void prepend (Node child);
  }
}
