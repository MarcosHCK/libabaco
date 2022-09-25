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
  internal class List : Node
  {
    public unowned GLib.List<Node> children { get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      var partial = base.debug (spaces);
      unowned var child = children;

      while (child != null)
      {
        var node = (Node) child.data;
        partial += "\r\n" + node.debug (spaces + 1);
        child = child.next;
      }
    return partial;
    }

#endif // DEVELOPER

    /* public API */

    public void append (Node child) { children.append (child); }
    public void prepend (Node child) { children.prepend (child); }
    public uint n_children () { return children.length (); }

    public void children_foreach (GLib.Func<unowned Node> callback)
    {
      unowned var child = children;
      while (child != null)
      {
        callback ((Node) child.data);
        child = child.next;
      }
    }
  }

  internal class Scope : List { }

  internal interface RValue : Node { }

  internal interface Named : Node
  {
    public abstract string name { get; set; }
  }

  internal interface Unique : Node
  {
    public abstract string id { get; set; }

    /* public API */

    public static string next ()
    {
      return Uuid.string_random ();
    }
  }

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

  internal class Variable : Node, Named, RValue, Unique
  {
    public string name { get; private set; }
    public string id { get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return ("%s, name '%s', id '%s'").printf (base.debug (spaces), name, id);
    }

#endif // DEVELOPER

    /* constructor */

    public Variable (string name)
    {
      base ();
      this.id = Unique.next ();
      this.name = name;
    }
  }

  internal class Extern : Variable
  {
    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return ("%s, extern").printf (base.debug (spaces));
    }

#endif // DEVELOPER

    /* constructor */

    public Extern (string name)
    {
      base (name);
    }
  }

  internal class Function : Variable
  {
    public List arguments { get; private set; }
    public Scope scope { get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return
        base.debug (spaces)
      + "\r\n"
      + scope.debug (spaces + 1);
    }

#endif // DEVELOPER

    /* constructor */

    public Function (string name, List arguments, Scope scope)
    {
      base (name);
      this.arguments = arguments;
      this.scope = scope;
    }
  }

  internal class Operator : Function
  {
    public uint precedence { get; private set; }
    public string assoc { get; private set; }

    public const string ASSOC_LEFT = "left";
    public const string ASSOC_RIGHT = "right";

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      var partial = base.debug (spaces);
      var index = partial.index_of_char ('\r');
      var last = partial.index_of_nth_char (index);

      return
        ("%.*s, precedence %u, assoc '%s'%s").printf
          (last, partial, precedence, assoc,
              partial.offset (last));
    }

#endif // DEVELOPER

    /* constructor */

    public Operator (string name, uint precedence, string assoc, List arguments, Scope scope)
    {
      base (name, arguments, scope);
      this.precedence = precedence;
      this.assoc = assoc;
    }
  }

  internal class Assign : Node, RValue
  {
    public Variable variable { get; private set; }
    public RValue rvalue {get; private set; }

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
    }
  }

  internal class Call : Node, RValue
  {
    public Unique target { get; private set; }
    public List arguments { get; private set; }

    /* debug API */

#if DEVELOPER == 1

    public override string debug (size_t spaces)
    {
      return base.debug (spaces)
              + "\r\n"
              + target.debug (spaces + 1)
              + "\r\n"
              + arguments.debug (spaces + 1);
    }

#endif // DEVELOPER

    /* constructor */

    public Call (Unique target, List arguments)
    {
      base ();
      this.target = target;
      this.arguments = arguments;
    }
  }

  internal abstract class Conditional : Node
  {
    public RValue condition { get; private set; }
    public Scope direct { get; private set; }

    /* constructor */

    protected Conditional (RValue condition, Scope direct)
    {
      base ();
      this.condition = condition;
      this.direct = direct;
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
    public Scope reverse { get; private set; }

    /* constructor */

    public If (RValue condition, Scope direct, Scope reverse)
    {
      base (condition, direct);
      this.reverse = reverse;
    }
  }
}
