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
 * along with libabaco. If not, see <http://www.gnu.org/licenses/>.
 *
 */

namespace Abaco.Ast
{
  internal abstract class Node
  {
    private Chain chain;
    private Datalist<string?> notes;
    public string token { get; private set; }

    /* public API */

    public void append (Node child) { Chain.append (ref chain, ref child.chain); }
    public void prepend (Node child) { Chain.prepend (ref chain, ref child.chain); }
    public uint n_children () { return Chain.n_children (ref chain); }

    public void children_foreach (GLib.Func<unowned Node> callback)
    {
      unowned Chain? child = chain.children;
      while (child != null)
      {
        callback ((Node) child.self);
        child = child.next;
      }
    }

    public void set_note (string index, string content) { notes.set_data (index, content); }
    public void set_note_by_id (GLib.Quark index, string content) { notes.id_set_data (index, content); }
    public unowned string get_note (string index) { return notes.get_data (index); }
    public unowned string get_note_by_id (GLib.Quark index) { return notes.id_get_data (index); }
    public string steal_note (string index) { return notes.remove_no_notify (index); }
    public string steal_note_by_id (GLib.Quark index) { return notes.id_remove_no_notify (index); }

    /* constructors */

    protected Node (string token)
    {
      chain.self = this;
      this.token = token;
    }

    ~Node ()
    {
      chain.destroy ();
    }
  }
}
