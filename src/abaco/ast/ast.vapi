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

  [CCode (cheader_filename = "ast.h", ref_function = "abaco_ast_node_ref", unref_function = "abaco_ast_node_unref")]
  public class Node
  {
    public uint ref_count;

    public Node (string symbol, SymbolKind kind);
    public unowned string get_symbol ();
    public SymbolKind get_kind ();

    public unowned Node @ref ();
    public void unref ();

    public void append (Node child);
    public void prepend (Node child);
    public uint get_n_children ();
    public void children_foreach ();

    [CCode (simple_generics = true)]
    public void set_note<T> (string key, T data);
    public void set_note_full (string key, void* data, GLib.DestroyNotify notify);
    [CCode (simple_generics = true)]
    public unowned T get_note<T> (string key);
    [CCode (simple_generics = true)]
    public owned T steal_note<T> (string key);

    public string symbol { get { return get_symbol (); } }
    public SymbolKind kind { get { return get_kind (); } }
  }
}
