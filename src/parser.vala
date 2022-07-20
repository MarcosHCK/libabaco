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
using Abaco.Symbols;

namespace Abaco
{
  internal class Parser
  {
    Queue<Ast.Node> output = new Queue<Ast.Node> ();
    Queue<Symbol> operators = new Queue<Symbol> ();
    Queue<uint> args = new Queue<uint> ();
    unowned SymbolClass? pklass = null;
    unowned string ptoken = null;

    const Ast.SymbolKind otypr = Ast.SymbolKind.FUNCTION;
    public bool code_strict { get; set; }

    [Compact]
    class Symbol
    {
      public unowned string token;
      public unowned SymbolClass? klass;
      public SymbolKind kind;

      public Symbol (string token, SymbolClass? klass)
      {
        this.token = token;
        this.klass = klass;
        this.kind = klass.kind;
      }
    }

    void pushsym (string token, SymbolClass? klass) { operators.push_head (new Symbol (token, klass)); }
    unowned Symbol? peeksym () { return operators.peek_head (); }
    void popsym () { operators.pop_head (); }

    void pushvar (string token) throws GLib.Error
    {
      bool valid = true;
      if (pklass != null)
      {
        if (pklass.kind == SymbolKind.PARENTHESIS
          && ptoken[0] == ')')
          valid = false;
        else
        if (pklass.kind == SymbolKind.CONSTANT)
          valid = false;
        else
        if (pklass.kind == SymbolKind.VARIABLE)
          valid = false;
        else
        if (token == ".")
          valid = false;
      }

      if (!valid)
      {
        var msg = ("unexpected token '%s'").printf (token);
        throw new ExpressionError.UNEXPECTED_TOKEN (msg);
      }
      else
      {
        var node = new Ast.Node (token, Ast.SymbolKind.CONSTANT);
        output.push_head (node);
      }
    }

    void pushfunction (string token, uint n_args) throws GLib.Error
    {
      var node = new Ast.Node (token, otypr);
      var child = (Ast.Node) null;

      for (int i = 0; i < n_args; i++)
      {
        child = output.pop_head ();
        if (child == null)
        {
          var msg = "node's stack is empty!";
          throw new ExpressionError.FAILED (msg);
        }

        node.prepend (child);
      }

      output.push_head (node);
    }

    void flushpar (string token) throws GLib.Error
    {
      bool valid = true;
      if (pklass != null)
      {
        if (pklass.kind == SymbolKind.COMMA)
          valid = false;
      }

      if (!valid)
      {
        var msg = ("unexpected token '%s'").printf (token);
        throw new ExpressionError.UNEXPECTED_TOKEN (msg);
      }
      else
      {
        while (true)
        {
          unowned var sym = peeksym ();
          if (sym == null)
          {
            var msg = ("unmatched '(' parenthesis").printf ();
            throw new ExpressionError.UNMATCHED_PARENTHESIS (msg);
          }
          else
          if (sym.kind == SymbolKind.PARENTHESIS)
            break;
          else
          {
            if (sym.kind == SymbolKind.FUNCTION)
              pushfunction (sym.token, 1);
            else
            if (sym.kind == SymbolKind.OPERATOR)
            {
              unowned var oclass = sym.klass.opclass;
              unowned var n_args = (oclass.unary) ? 1 : 2;
              pushfunction (sym.token, n_args);
            }

            popsym ();
          }
        }
      }
    }

    public void consume (string token, SymbolClass? klass) throws GLib.Error
    {
      switch (klass.kind)
      {
      case SymbolKind.UNKNOWN:
        if (!code_strict)
          return;
        else
        {
          var msg = ("unknown token '%s'").printf (token);
          throw new ExpressionError.UNKNOWN_TOKEN (msg);
        }
        break;

      case SymbolKind.CONSTANT:
      case SymbolKind.VARIABLE:
        pushvar (token);
        break;

      case SymbolKind.COMMA:
        if (args.length == 0)
        {
          var msg = ("unexpected token '%s'").printf (token);
          throw new ExpressionError.UNEXPECTED_TOKEN (msg);
        }
        else
        {
          if (pklass.kind == SymbolKind.PARENTHESIS
            && ptoken[0] == '(')
          {
            var msg = ("unexpected token '%s'").printf (token);
            throw new ExpressionError.UNEXPECTED_TOKEN (msg);
          }
          else
          {
            var n_args = args.pop_head ();
            args.push_head (n_args + 1);
            flushpar (token);
          }
        }
        break;

      case SymbolKind.FUNCTION:
        pushsym (token, klass);
        break;

      case SymbolKind.OPERATOR:
        while (true)
        {
          unowned var sym = peeksym ();
          if (sym == null)
            break;
          else
          if (sym.kind == SymbolKind.OPERATOR)
          {
            unowned OperatorClass oclass1 = klass.opclass;
            unowned OperatorClass oclass2 = sym.klass.opclass;
            if (oclass2.precedence > oclass1.precedence
              || (oclass2.precedence == oclass1.precedence
              && (oclass1.assoc == OperatorAssoc.LEFT)))
            {
              var n_args = (oclass2.unary) ? 1 : 2;
              pushfunction (sym.token, n_args);
              popsym ();
              continue;
            }
          }
          else
          if (sym.kind == SymbolKind.FUNCTION)
          {
            pushfunction (sym.token, 1);
            popsym ();
            continue;
          }

          break;
        }

        pushsym (token, klass);
        break;

      case SymbolKind.PARENTHESIS:
        if (token[0] == '(')
        {
          pushsym (token, klass);

          if (pklass != null)
          if (pklass.kind == SymbolKind.FUNCTION)
            args.push_head (1);
        }
        else
        if (token[0] == ')')
        {
          flushpar (token);
          popsym ();

          unowned var sym = peeksym ();
          if (sym != null &&
            sym.kind == SymbolKind.FUNCTION)
          {
            var n_args = args.pop_head ();
            if (pklass.kind == SymbolKind.PARENTHESIS
              && ptoken [0] == '(')
              pushfunction (sym.token, 0);
            else
              pushfunction (sym.token, n_args);
            popsym ();
          }
        }
        break;
      }

      ptoken = token;
      pklass = klass;
    }

    public Ast.Node finish () throws GLib.Error
    {
      unowned Symbol sym;
      while ((sym = peeksym ()) != null)
      {
        if (sym.kind == SymbolKind.PARENTHESIS)
        {
          var msg = ("unmatched '(' parenthesis");
          throw new ExpressionError.UNMATCHED_PARENTHESIS (msg);
        }

        if (sym.kind == SymbolKind.FUNCTION)
          pushfunction (sym.token, 1);
        else
        if (sym.kind == SymbolKind.OPERATOR)
        {
          unowned var oclass = sym.klass.opclass;
          var n_args = (oclass.unary) ? 1 : 2;
          pushfunction (sym.token, n_args);
        }

        popsym ();
      }

      if (output.length > 1)
      {
        var msg = "unfinished tree at finish";
        throw new ExpressionError.FAILED (msg);
      }
    return output.pop_head ();
    }
  }
}
