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
    public string? symbol { get; private set; }
    public SymbolKind kind { get; private set; }
    private GLib.Node<unowned Node> chain;

    public void append (Node child) { chain.append (child.chain.@copy ()); }
    public void prepend (Node child) { chain.prepend (child.chain.@copy ()); }
    public uint n_children () { return chain.n_children (); }

    public void children_foreach (Foreach func)
    {
      chain.children_foreach
      (TraverseFlags.ALL,
       (node) => {
          func (node.data);
       });
    }

    public Node (string symbol, SymbolKind kind)
    {
      this.symbol = symbol;
      this.kind = kind;
      this.chain.children = null;
      this.chain.parent = null;
      this.chain.prev = null;
      this.chain.next = null;
      this.chain.data = this;
    }
  }
}
