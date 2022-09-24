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
  internal class Scope : Node
  {
    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      var partial = base.debug (spaces);
      unowned var child = (Chain?) chain.children;
      unowned var node = (Node?) null;

      while (child != null)
      {
        node = (Node) child.self;
        partial += "\r\n" + node.debug (spaces + 1);
        child = child.next;
      }
    return partial;
    }

#endif // DEVELOPER

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

  internal interface RValue : Node { }

  internal class Constant : Node, RValue
  {
    public string value { get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return ("%s, value '%s'").printf (base.debug (spaces), value);
    }

#endif // DEVELOPER

    /* constructor */

    public Constant (string value)
    {
      base ();
      this.value = value;
    }
  }

  internal class Variable : Node, RValue
  {
    public string id { get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return ("%s, id '%s'").printf (base.debug (spaces), id);
    }

#endif // DEVELOPER

    /* constructor */

    public Variable (string id)
    {
      base ();
      this.id = id;
    }
  }

  internal class Function : Variable
  {
    public unowned Scope scope { get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return base.debug (spaces)
            + "\r\n"
            + scope.debug (spaces + 1);
    }

#endif // DEVELOPER

    /* constructor */

    public Function (string id, Scope scope)
    {
      base (id);
      this.scope = scope;
      Chain.append (ref chain, ref scope.chain);
    }
  }

  internal class Operator : Variable
  {
    public unowned Scope scope { get; private set; }
    public uint precedence { get; private set; }
    public string assoc { get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return ("%s, precedence %u, assoc '%s'").printf
              (base.debug (spaces), precedence, assoc)
            + "\r\n"
            + scope.debug (spaces + 1);
    }

#endif // DEVELOPER

    /* constructor */

    public Operator (string id, uint precedence, string assoc, Scope scope)
    {
      base (id);
      this.precedence = precedence;
      this.assoc = assoc;
      this.scope = scope;
      Chain.append (ref chain, ref scope.chain);
    }
  }

  internal class Assign : Node, RValue
  {
    public unowned Variable variable { get; private set; }
    public unowned RValue rvalue {get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return base.debug (spaces)
            + "\r\n"
            + variable.debug (spaces + 1)
            + "\r\n"
            + rvalue.debug (spaces + 1);
    }

#endif // DEVELOPER

    /* constructor */

    public Assign (Variable variable, RValue rvalue)
    {
      base ();
      this.variable = variable;
      this.rvalue = rvalue;
      Chain.append (ref chain, ref variable.chain);
      Chain.append (ref chain, ref rvalue.chain);
    }
  }

  internal class Call : Node, RValue
  {
    public string name { get; private set; }
    public bool ccall { get; private set; }
    public unowned Scope arguments { get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return ("%s, name '%s', ccall '%s'").printf
              (base.debug (spaces), name,
                ccall ? "true" : "false")
              + "\r\n"
              + arguments.debug (spaces + 1);
    }

#endif // DEVELOPER

    /* constructor */

    public Call (string name, bool ccall, Scope arguments)
    {
      base ();
      this.name = name;
      this.ccall = ccall;
      this.arguments = arguments;
      Chain.append (ref chain, ref arguments.chain);
    }
  }

  internal abstract class Conditional : Node
  {
    public unowned RValue condition { get; private set; }
    public unowned Scope direct { get; private set; }

    /* constructor */

    protected Conditional (RValue condition, Scope direct)
    {
      base ();
      this.condition = condition;
      this.direct = direct;
      Chain.append (ref chain, ref condition.chain);
      Chain.append (ref chain, ref direct.chain);
    }
  }

  internal class While : Conditional
  {
    /* constructor */

    public While (RValue condition, Scope direct)
    {
      base (condition, direct);
    }
  }

  internal class If : Conditional
  {
    public unowned Scope reverse { get; private set; }

    /* constructor */

    public If (RValue condition, Scope direct, Scope reverse)
    {
      base (condition, direct);
      this.reverse = reverse;
      Chain.append (ref chain, ref reverse.chain);
    }
  }
}
