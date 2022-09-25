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

namespace Abaco
{
  public errordomain ParserError
  {
    FAILED,
    LITERAL,
    UNDEFINED,
    EXPECTED_TOKEN,
    UNEXPECTED_TOKEN,
  }

  internal sealed class Parser : GLib.Object
  {
    public bool naked { get; construct; }
    private bool freezed { get; set; default = false; }

    private HashTable<unowned string, Ast.Variable>? variables;

    /* private API */

    private void annotate (Ast.Node node, Token? token)
    {
      node.set_note ("token-location", token.locate ());
      node.set_note_by_id (Ast.Node.Annotations.line_number, token.line.to_string ());
      node.set_note_by_id (Ast.Node.Annotations.column_number, token.column.to_string ());
    }

    private Ast.Variable create_variable (Token? identifier)
    {
      unowned var name = identifier.token;
      Ast.Variable @var = new Ast.Variable (name);
      variables.insert (@var.name, @var);
      annotate (@var, identifier);
    return @var;
    }

    private unowned Ast.Variable? lookup_variable (string name)
    {
      unowned var @var = (Ast.Variable?) null;
      if (variables.lookup_extended (name, null, out @var))
        return @var;
    return null;
    }

    private Ast.Extern create_extern (GLib.Queue<Token?> tokens, Token? token) throws GLib.Error
    {
      Token? next;
      Token? identifier;

      next = tokens.pop_tail ();
      if (next == null)
        throw new ParserError.EXPECTED_TOKEN ("%s: Expected identifier after extern declaration", token.locate ());
      else
      {
        if (next.klass.kind != TokenType.IDENTIFIER)
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
          identifier = (owned) next;
      }

      next = tokens.pop_tail ();
      if (next == null)
        throw new ParserError.EXPECTED_TOKEN ("%s: Expected token ';' after identifier", identifier.locate ());
      else
      {
        if (next.klass.kind != TokenType.SEPARATOR || next.token != ";")
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          return new Ast.Extern (identifier.token);
        }
      }
    }

    private void define_extern (GLib.Queue<Token?> tokens, Token? token) throws GLib.Error
    {
      var @var = create_extern (tokens, token);
      variables.insert (@var.name, @var);
      annotate (@var, token);
    }

    private bool arglist_dotorpar (Token? token, out unichar char_)
    {
      if (token.klass.kind == TokenType.SEPARATOR)
      {
        unowned var t = token.token;
        unowned var c = t.get_char ();
        unowned var n = t.next_char ();

        if (n.get_char () == (unichar) 0)
        {
          char_ = c;
          return c == (unichar) ','
              || c == (unichar) ')';
        }
      }
    return false;
    }

    private Ast.List create_function_args (GLib.Queue<Token?> tokens, Token? token) throws GLib.Error
    {
      Ast.List arguments = new Ast.List ();
      Token? next;

      while ((next = tokens.pop_tail ()) != null)
      {
        unowned var klass = next.klass;
        unowned var expr = next.token;
        unowned var c = (unichar) 0;

        switch (klass.kind)
        {
          case TokenType.IDENTIFIER:
            {
              var sep = (Token?) null;
              if ((sep = tokens.pop_tail ()) == null)
                throw new ParserError.EXPECTED_TOKEN ("%s: Expected ')' token before end of input", token.locate ());
              if (!arglist_dotorpar (sep, out c))
                throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", sep.locate (), sep.token);
              else
              {
                var arg = new Ast.Variable (next.token);
                    arguments.append (arg);

                if (c == (unichar) ')')
                  return arguments;
              }
            }
            break;
          case TokenType.SEPARATOR:
            {
              if (arglist_dotorpar (next, out c))
              {
                if (c == (unichar) ')')
                  return arguments;
                else
                  throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), expr);
              }
              else
                throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), expr);
            }
            break;

          default:
            throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), expr);
        }
      }

      throw new ParserError.EXPECTED_TOKEN ("%s: Expected ')' token before end of input", token.locate ());
    }

    private Ast.Scope create_function_scope (GLib.Queue<Token?> tokens, Token? token, Ast.List arguments) throws GLib.Error
    {
      var save = new GLib.Queue<Ast.Variable> ();
      var undefs = new GLib.Queue<string> ();
      var saved = (Ast.Variable?) null;
      var undef = (string?) null;
      var scope = (Ast.Scope) null;

      foreach (unowned var child in arguments.children)
      {
        unowned var @var = (Ast.Variable?) null;
        unowned var arg = (Ast.Variable?) child;
        unowned var name = arg.name;

        if (!variables.lookup_extended (name, null, out @var))
          undefs.push_head (name);
        else
          save.push_head (@var);

        variables.insert (name, arg);
      }

      scope = parse_block (tokens, token);
      while ((saved = save.pop_tail ()) != null)
        variables.insert (saved.name, saved);
      while ((undef = undefs.pop_tail ()) != null)
        variables.remove (undef);
    return scope;
    }

    private Ast.Function create_function (GLib.Queue<Token?> tokens, Token? token) throws GLib.Error
    {
      Token? next;
      Token? identifier;
      Ast.List arguments;
      Ast.Scope scope;

      next = tokens.pop_tail ();
      if (next == null)
        throw new ParserError.EXPECTED_TOKEN ("%s: Expected identifier after '%s' keyword", token.locate (), token.token);
      else
      {
        if (next.klass.kind != TokenType.IDENTIFIER)
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          identifier = (owned) next;
        }
      }

      next = tokens.pop_tail ();
      if (next == null)
        throw new ParserError.EXPECTED_TOKEN ("%s: Expected identifier after '%s' keyword", token.locate (), token.token);
      else
      {
        if (next.klass.kind != TokenType.SEPARATOR || next.token != "(")
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          arguments = create_function_args (tokens, token);
        }
      }

      next = tokens.pop_tail ();
      if (next == null)
        throw new ParserError.EXPECTED_TOKEN ("%s: Expected identifier after '%s' keyword", token.locate (), token.token);
      else
      {
        if (next.klass.kind != TokenType.SEPARATOR || next.token != "{")
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          scope = create_function_scope (tokens, next, arguments);
        }
      }
    return new Ast.Function (identifier.token, arguments, scope);
    }

    private void define_function (GLib.Queue<Token?> tokens, Token? token) throws GLib.Error
    {
      var func = create_function (tokens, token);
      variables.insert (func.name, func);
      annotate (func, token);
    }

    private Ast.Operator create_operator (GLib.Queue<Token?> tokens, Token? token) throws GLib.Error
    {
      Token? next;
      Ast.List arguments;
      Ast.Scope scope;
      unowned uint precedence = 0;
      unowned string assoc = null;

      next = tokens.pop_tail ();
      if (next == null)
        throw new ParserError.EXPECTED_TOKEN ("%s: Expected literal after operator declaration", token.locate ());
      else
      {
        if (next.klass.kind != TokenType.LITERAL)
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
          precedence = (Literal (next.token)).to_uint ();
      }

      next = tokens.pop_tail ();
      if (next == null)
        throw new ParserError.EXPECTED_TOKEN ("%s: Expected identifier after operator declaration", token.locate ());
      else
      {
        if (next.klass.kind != TokenType.IDENTIFIER)
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          assoc = next.token;
          if (assoc == "left")
            assoc = Ast.Operator.ASSOC_LEFT;
          else
          if (assoc == "right")
            assoc = Ast.Operator.ASSOC_RIGHT;
          else
            throw new ParserError.EXPECTED_TOKEN ("%s: Expected 'left' or 'right', got '%s'", next.locate (), assoc);
        }
      }

      next = tokens.pop_tail ();
      if (next == null)
        throw new ParserError.EXPECTED_TOKEN ("%s: Expected block declaration after operator declaration", token.locate ());
      else
      {
        if (next.klass.kind != TokenType.SEPARATOR || next.token != "{")
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          arguments = new Ast.List ();
          arguments.append (new Ast.Variable ("a"));
          arguments.append (new Ast.Variable ("b"));
          scope = create_function_scope (tokens, next, arguments);
        }
      }
    return new Ast.Operator (token.token, precedence, assoc, arguments, scope);
    }

    private void define_operator (GLib.Queue<Token?> tokens, Token? token) throws GLib.Error
    {
      var oper = create_operator (tokens, token);
      variables.insert (oper.name, oper);
      annotate (oper, token);
    }

    private unowned Ast.Operator? lookup_operator (string name)
    {
      var name_ = "operator" + name;
      var @var = lookup_variable (name_);
      if (@var != null && @var is Ast.Operator)
        return (Ast.Operator) @var;
    return null;
    }

    private void varguard (Token? token, Token? last) throws GLib.Error
    {
    }

    private Ast.RValue? pushcall (GLib.Queue<Ast.RValue> members, Ast.Operator klass, Token? token) throws GLib.Error
    {
      var args = new Ast.Scope ();
      var rvalue = (Ast.RValue?) null;
      int i, nargs = 2;

      for (i = 0; i < nargs; i++)
      {
        rvalue = members.pop_head ();
        args.prepend (rvalue);
      }

      var call = new Ast.Call (klass, args);
        annotate (call, token);
    return call;
    }

    private Ast.RValue? parse_rvalue (GLib.Queue<Token?> tokens, Token? begin) throws GLib.Error
    {
      var members = new GLib.Queue<Ast.RValue> ();
      var operators = new GLib.Queue<Token?> ();
      var rvalue = (Ast.RValue?) null;
      var token = (Token?) null;
      var last = (Token?) null;

      while ((token = tokens.pop_tail ()) != null)
      {
        unowned var klass = token.klass;
        unowned var expr = token.token;

        varguard (token, last);

        switch (klass.kind)
        {
          case TokenType.IDENTIFIER:
            {
              unowned var next = (Token?) null;
                next = tokens.peek_tail ();

              if (next != null && next.klass.kind == TokenType.OPERATOR)
              {
                if ((rvalue = lookup_variable (token.token)) == null)
                  rvalue = create_variable (token);
                members.push_head ((owned) rvalue);
              }
              else
              {
                rvalue = parse_identifier (tokens, token);
                members.push_head ((owned) rvalue);
              }
            }
            break;
          case TokenType.SEPARATOR:
            {
              unowned var c = expr.get_char ();
              unowned var n = expr.next_char ();

              if (n.get_char () == 0)
              {
                if (c == (unichar) '(')
                  operators.push_head (token);
                else
                if (c == (unichar) ')')
                {
                  unowned var o2 = (Token?) null;

                  while (true)
                  {
                    o2 = operators.peek_head ();
                    if (o2 == null)
                      throw new ParserError.UNEXPECTED_TOKEN ("%s: Unmatched '%s' token", token.locate (), token.token);
                    else
                    {
                      if (o2.klass.kind == TokenType.SEPARATOR)
                        operators.pop_head ();
                      else
                      {
                        unowned var klass2 = (Ast.Operator) null;
                        if ((klass2 = lookup_operator (o2.token)) == null)
                          throw new ParserError.UNDEFINED ("%s: Undefined operator '%s'", o2.locate (), o2.token);

                        rvalue = pushcall (members, klass2, o2);
                        members.push_head ((owned) rvalue);
                        operators.pop_head ();
                        continue;
                      }
                    }

                    break;
                  }
                }
                else
                  throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), token.token);
              }
              else
                  throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), token.token);
            }
            break;
          case TokenType.OPERATOR:
            {
              unowned var o1 = (Token?) token;
              unowned var o2 = (Token?) null;

              while (true)
              {
                o2 = operators.peek_head ();
                if (o2 != null && o2.klass.kind == TokenType.OPERATOR)
                {
                  unowned var klass1 = (Ast.Operator) null;
                    if ((klass1 = lookup_operator (o1.token)) == null)
                      throw new ParserError.UNDEFINED ("%s: Undefined operator '%s'", o1.locate (), o1.token);
                  unowned var klass2 = (Ast.Operator) null;
                    if ((klass2 = lookup_operator (o2.token)) == null)
                      throw new ParserError.UNDEFINED ("%s: Undefined operator '%s'", o2.locate (), o2.token);

                  if ((klass2.precedence > klass1.precedence)
                    || ((klass1.precedence == klass2.precedence)
                      && (klass1.assoc == Ast.Operator.ASSOC_LEFT)))
                  {
                    rvalue = pushcall (members, klass2, o2);
                    members.push_head ((owned) rvalue);
                    operators.pop_head ();
                    continue;
                  }
                }

                break;
              }

              operators.push_head (token);
            }
            break;
          case TokenType.LITERAL:
            {
              rvalue = new Ast.Constant (expr);
              annotate (rvalue, token);
              members.push_head ((owned) rvalue);
            }
            break;
          default:
            throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), token.token);
        }

        last = (owned) token;
      }

      while ((token = operators.pop_head ()) != null)
      {
        unowned var klass = token.klass;
        unowned var expr = token.token;

        if (klass.kind == TokenType.SEPARATOR)
          throw new ParserError.EXPECTED_TOKEN ("%s: Unmatched '%s' token", token.locate (), token.token);
        else
        {
          unowned var o2 = (Token?) token;
          unowned var klass2 = (Ast.Operator) null;
            if ((klass2 = lookup_operator (o2.token)) == null)
              throw new ParserError.UNDEFINED ("%s: Undefined operator '%s'", o2.locate (), o2.token);
          rvalue = pushcall (members, klass2, o2);
          members.push_head ((owned) rvalue);
        }
      }
#if DEVELOPER
      assert (members.length <= 1);
#endif // DEVELOPER
    return members.pop_head ();
    }

    private Ast.RValue parse_call (GLib.Queue<Token?> tokens, Token? identifier, Token? next) throws GLib.Error
    {
      var arguments = new Ast.List ();
      var extra = new GLib.Queue<Token?> ();
      var rvalue = (Ast.RValue) null;
      var last = (Token?) identifier;
      var call = (Ast.Call) null;
      var token = (Token?) null;
      int balance = 1;

      {
        var target = (Ast.Unique?) null;
        if (!this.variables.lookup_extended (identifier.token, null, out target))
          throw new ParserError.UNDEFINED ("%s: Undefined variable '%s'", identifier.locate (), identifier.token);

        call = new Ast.Call (target, arguments);
      }

      annotate (call, identifier);

      while ((token = tokens.pop_tail ()) != null)
      {
        unowned var klass = token.klass;
        unowned var expr = token.token;

        switch (klass.kind)
        {
          case TokenType.SEPARATOR:
            {
              unowned var c = expr.get_char ();
              unowned var n = expr.next_char ();

              if (n.get_char () == (unichar) 0)
              {
                switch (c)
                {
                  case '(':
                    balance += 1;
                    if (balance > 1)
                      extra.push_head ((owned) token);
                    break;
                  case ')':
                    balance -= 1;
                    if (balance >= 1)
                      extra.push_head ((owned) token);
                    else
                    if (balance == 0)
                    {
                      rvalue = parse_rvalue (extra, last);
                      if (rvalue != null)
                        arguments.append ((owned) rvalue);
                      return call;
                    }
                    break;
                  case ',':
                    if (balance == 1)
                    {
                      rvalue = parse_rvalue (extra, token);
                      if (rvalue == null)
                        throw new ParserError.EXPECTED_TOKEN ("%s: Expected rvalue as argument", identifier.locate ());

                      last = (owned) token;
                      arguments.append ((owned) rvalue);
                      extra.clear ();
                    }
                    else
                      extra.push_head ((owned) token);
                    break;

                  default:
                    extra.push_head ((owned) token);
                    break;
                }
              }
              else
                extra.push_head ((owned) token);
            }
            break;

          default:
            extra.push_head ((owned) token);
            break;
        }
      }

      throw new ParserError.EXPECTED_TOKEN ("%s: Unexpected end of input while parsing call arguments", next.locate ());
    }

    private bool isdotcom (Token? token)
    {
      if (token != null && token.klass.kind == TokenType.SEPARATOR)
      {
        unowned var t = token.token;
        unowned var c = t.get_char ();
        unowned var n = t.next_char ();
        if (n.get_char () == (unichar) 0)
        {
          return c == (unichar) ';';
        }
      }
    return false;
    }

    private Ast.RValue parse_assign (GLib.Queue<Token?> tokens, Token? identifier, Token? begin) throws GLib.Error
    {
      var extra = new GLib.Queue<Token?> ();
      var variable = (Ast.Variable?) null;
      var assign = (Ast.Assign?) null;
      var rvalue = (Ast.RValue?) null;
      var next = (Token?) null;

      while ((next = tokens.pop_tail ()) != null)
      {
        unowned var klass = next.klass;
        unowned var expr = next.token;

        switch (klass.kind)
        {
          case TokenType.SEPARATOR:
            if (isdotcom (next))
            {
              if ((variable = lookup_variable (identifier.token)) == null)
                variable = create_variable (identifier);

              rvalue = parse_rvalue (extra, begin);
              if (rvalue == null)
                throw new ParserError.EXPECTED_TOKEN ("%s: Expected rvalue for assigment", begin.locate ());

              assign = new Ast.Assign (variable, rvalue);
              annotate (variable, identifier);
              annotate (assign, begin);
              return assign;
            }
            else
              extra.push_head ((owned) next);
            break;

          default:
            extra.push_head ((owned) next);
            break;
        }
      }

      throw new ParserError.EXPECTED_TOKEN ("%s: Unexpected end of input while parsing rvalue", begin.locate ());
    }

    private Ast.RValue parse_identifier (GLib.Queue<Token?> tokens, Token? identifier) throws GLib.Error
    {
      Token? next;

      next = tokens.pop_tail ();
      if (next == null)
      {
        var variable = (Ast.Variable?) null;
        if ((variable = lookup_variable (identifier.token)) == null)
          variable = create_variable (identifier);
        return variable;
      }
      else
      {
        if (next.klass.kind != TokenType.SEPARATOR)
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          if (next.token == "(")
            return parse_call (tokens, identifier, next);
          else
          if (next.token == "=")
            return parse_assign (tokens, identifier, next);
          else
            throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        }
      }
    }

    private void checkdotcom (GLib.Queue<Token?> tokens) throws GLib.Error
    {
      unowned var token = (Token?) null;
      if ((token = tokens.peek_tail ()) != null)
      {
        if (!isdotcom (token))
          throw new ParserError.EXPECTED_TOKEN ("%s: Expected ';' token", token.locate ());
      }
      else
        throw new ParserError.EXPECTED_TOKEN ("1: 1: Expected ';' before end of input");
    }

    private Ast.Scope parse_block (GLib.Queue<Token?> tokens, Token? begin) throws GLib.Error
    {
      var scope = new Ast.Scope ();
      var node = (Ast.Node) null;
      var prevv = variables;
      Token? token;

      {
        var hash = GLib.str_hash;
        var equal = GLib.str_equal;

        annotate (scope, begin);
        variables = new HashTable<unowned string, Ast.Variable> (hash, equal);

        if (prevv != null)
        {
          unowned var key = (string) null;
          unowned var val = (Ast.Variable) null;
          var iter = HashTableIter<unowned string, Ast.Variable> (prevv);

          while (iter.next (out key, out val))
            variables.insert (key, val);
        }
      }

      while ((token = tokens.pop_tail ()) != null)
      {
        unowned var klass = token.klass;
        unowned var expr = token.token;

        switch (klass.kind)
        {
          case TokenType.IDENTIFIER:
            {
              node = parse_identifier (tokens, token);
              if (node is Ast.Variable)
                throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), expr);
              if (node is Ast.Call)
                checkdotcom (tokens);

              scope.append ((owned) node);
            }
            break;
          case TokenType.KEYWORD:
            {
              if (expr.has_prefix ("operator"))
                define_operator (tokens, token);
              else
              {
                switch (expr)
                {
                  case "extern":
                    define_extern (tokens, token);
                    break;
                  case "function":
                    define_function (tokens, token);
                    break;
                  default:
                    throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), expr);
                }
              }
            }
            break;
          case TokenType.SEPARATOR:
            {
              unowned var c = expr.get_char ();
              unowned var n = expr.next_char ();

              if (n.get_char () == (unichar) 0)
              {
                switch (c)
                {
                  case '{':
                    node = parse_block (tokens, token);
                    scope.append (node);
                    break;
                  case '}':
                    variables = prevv;
                    return scope;
                  case ';':
                    break;

                  default:
                    throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), expr);
                }
              }
              else
                throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), expr);
            }
            break;
          default:
            throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), expr);
        }
      }

      throw new ParserError.EXPECTED_TOKEN ("%s: Expected end of block statement before end of input", begin.locate ());
    }

    /* public API */

    public Ast.Scope parse (Token[] tokens) throws GLib.Error
      requires (tokens.length > 0)
    {
      if (freezed)
        error ("Frozen parser");

      var _tokens = new GLib.Queue<Token?> ();
      var global = (Ast.Scope) null;

      foreach (unowned var token in tokens)
      {
        if (token.klass.kind != TokenType.COMMENT)
          _tokens.push_head (token);
      }

      {
        var klass = new TokenClass (TokenType.SEPARATOR, ".");
        var finish = Token ();
        var begin = Token ();

        finish.klass = klass;
        begin.klass = klass;
        finish.token = "}";
        begin.token = "{";
        finish.line = 1;
        begin.line = 1;
        finish.column = 1;
        begin.column = 1;
        finish.owner = tokens [0].owner;
        begin.owner = tokens [0].owner;
        _tokens.push_head (finish);

        global = parse_block (_tokens, begin);

        if (_tokens.length > 0)
          throw new ParserError.FAILED ("Input left after parsing (usually means extra '}' tokens)");

        print ("%s\r\n", global.debug (0));
      }

      freezed = true;
    return global;
    }

    /* constructor */

    public Parser (bool naked)
    {
      Object (naked : naked);
    }
  }
}
