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
  internal class Variable : Node
  {
    public string id { get; private set; }

    /* constructor */

    public Variable (string id)
    {
      base ();
      this.id = id;
    }
  }

  internal class Constant : Variable
  {
    public string value { get; private set; }

    /* constructor */

    public Constant (string id, string value)
    {
      base (id);
      this.value = value;
    }
  }

  internal class Scope : Node
  {
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
  }

  internal class RValue : Scope { }

  internal class Declaration : Scope
  {
    public string id { get; private set; }

    /* constructor */

    public Declaration (string id)
    {
      base ();
      this.id = id;
    }
  }

  internal class Operator : Declaration
  {
    public uint precedence { get; private set; }
    public string assoc { get; private set; }

    /* constructor */

    public Operator (string id, uint precedence, string assoc)
    {
      base (id);
      this.precedence = precedence;
      this.assoc = assoc;
    }
  }

  internal class Assign : RValue
  {
    public unowned Variable variable { get; private set; }
    public unowned RValue rvalue {get; private set; }

    /* constructor */

    public Assign (string id)
    {
      base ();
      var variable_ = new Variable (id);
      var rvalue_ = new RValue ();

      variable = variable_;
      rvalue = rvalue_;

      Chain.append (ref chain, ref variable_.chain);
      Chain.append (ref chain, ref rvalue_.chain);
    }
  }

  internal class Call : RValue
  {
    public string id { get; private set; }
    public bool ccall { get; private set; }

    /* constructor */

    public Call (string id, bool ccall)
    {
      base ();
      this.id = id;
      this.ccall = ccall;
    }
  }

  internal abstract class Conditional : Node
  {
    public unowned RValue condition { get; private set; }
    public unowned Scope direct { get; private set; }

    /* constructor */

    protected Conditional ()
    {
      base ();
      var condition_ = new RValue ();
      var direct_ = new Scope ();

      condition = condition_;
      direct = direct_;

      Chain.append (ref chain, ref condition_.chain);
      Chain.append (ref chain, ref direct_.chain);
    }
  }

  internal class While : Conditional { }

  internal class If : Conditional
  {
    public unowned Scope reverse { get; private set; }

    /* constructor */

    public If ()
    {
      base ();
      var reverse_ = new Scope ();
      reverse = reverse_;
      Chain.append (ref chain, ref reverse_.chain);
    }
  }
}
