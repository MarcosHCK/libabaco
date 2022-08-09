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
  public enum SymbolKind
  {
    CONSTANT,
    VARIABLE,
    FUNCTION,
  }

  public delegate void Foreach (Node node);

  public class Node
  {
    private AstPatch.Chain chain;
    private Datalist<string?> notes;
    public SymbolKind kind { get; private set; }
    public string symbol { get; private set; }

    public void append (Node child) { AstPatch.Chain.append (ref chain, ref child.chain); }
    public void prepend (Node child) { AstPatch.Chain.prepend (ref chain, ref child.chain); }
    public uint n_children () { return AstPatch.Chain.n_children (ref chain); }
    public void children_foreach (Foreach callback) { AstPatch.Chain.foreach_data (ref chain, callback); }

    public void set_note (string index, string content) { notes.set_data (index, content); }
    public void set_note_by_id (GLib.Quark index, string content) { notes.id_set_data (index, content); }
    public unowned string get_note (string index) { return notes.get_data (index); }
    public unowned string get_note_by_id (GLib.Quark index) { return notes.id_get_data (index); }
    public string steal_note (string index) { return notes.remove_no_notify (index); }
    public string steal_note_by_id (GLib.Quark index) { return notes.id_remove_no_notify (index); }

    public Node (string symbol, SymbolKind kind)
    {
      chain.self = this;
      this.symbol = symbol;
      this.kind = kind;
    }
  }
}
