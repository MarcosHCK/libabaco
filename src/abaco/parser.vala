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
    EXPECTED_TOKEN,
    UNEXPECTED_TOKEN,
  }

  internal sealed class Parser : GLib.Object
  {
    public bool naked { get; construct; }
    private bool freezed { get; set; default = false; }

    /* private API */

    private Ast.Operator define_operator (GLib.Queue<Token?> tokens, Token? token) throws GLib.Error
    {
      Token? next;
      var scope = (Ast.Scope) null;
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
            assoc = TokenClass.ASSOC_LEFT;
          else
          if (assoc == "right")
            assoc = TokenClass.ASSOC_RIGHT;
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
          scope = parse_block (tokens, next);
          return new Ast.Operator (token.token, precedence, assoc, scope);
        }
      }
    }

    private Ast.RValue? parse_rvalue (GLib.Queue<Token?> tokens, Token? begin) throws GLib.Error
    {
      var rvalue = (Ast.RValue?) null;
      var token = (Token?) null;
      
      while ((token = tokens.pop_tail ()) != null)
      {
        unowned var klass = token.klass;
        unowned var expr = token.token;

        print ("r %u:%u '%s' '%s'\r\n", token.line, token.column, klass.kind.to_string (), expr);

        switch (klass.kind)
        {
          default:
            throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), token.token);
        }
      }
    return rvalue;
    }

    private Ast.RValue parse_call (GLib.Queue<Token?> tokens, Token? identifier, Token? next, bool ccall) throws GLib.Error
    {
      var arguments = new Ast.Scope ();
      var call = new Ast.Call (identifier.token, ccall, arguments);
      var extra = new GLib.Queue<Token?> ();
      var rvalue = (Ast.RValue) null;
      var last = (Token?) identifier;
      var token = (Token?) null;
      int balance = 1;

      while ((token = tokens.pop_tail ()) != null)
      {
        unowned var klass = token.klass;
        unowned var expr = token.token;

        switch (klass.kind)
        {
          case TokenType.SEPARATOR:
            {
              if (expr == "(")
                balance += 1;
              else
              if (expr == ")")
              {
                balance -= 1;
                if (balance == 0)
                {
                  rvalue = parse_rvalue (extra, last);
                  if (rvalue != null)
                    arguments.append (rvalue);
                  return call;
                }
              }
              else
              if (expr == ",")
              {
                if (balance == 1)
                {
                  rvalue = parse_rvalue (extra, token);
                  if (rvalue == null)
                    throw new ParserError.EXPECTED_TOKEN ("%s: Expected rvalue as argument", identifier.locate ());

                  arguments.append (rvalue);
                  extra.clear ();
                  last = token;
                }
              }
              else
                throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), token.token);
            }
            break;
          default:
            extra.push_head (token);
            break;
        }
      }

      throw new ParserError.EXPECTED_TOKEN ("%s: Unexpected end of input while parsing call arguments", next.locate ());
    }

    private Ast.RValue parse_assign (GLib.Queue<Token?> tokens, Token? identifier, Token? begin) throws GLib.Error
    {
      var extra = new GLib.Queue<Token?> ();
      var variable = (Ast.Variable) null;
      var rvalue = (Ast.RValue) null;
      var next = (Token?) null;

      while ((next = tokens.pop_tail ()) != null)
      {
        unowned var klass = next.klass;
        unowned var expr = next.token;

        switch (klass.kind)
        {
          case TokenType.SEPARATOR:
            {
              if (expr == ";")
              {
                variable = new Ast.Variable (identifier.token);
                rvalue = parse_rvalue (extra, begin);
                if (rvalue == null)
                  throw new ParserError.EXPECTED_TOKEN ("%s: Expected rvalue for assigment", begin.locate ());
                return new Ast.Assign (variable, rvalue);
              }
            }
            break;
          default:
            extra.push_head (next);
            break;
        }
      }

      throw new ParserError.EXPECTED_TOKEN ("%s: Unexpected end of input while parsing rvalue", begin.locate ());
    }

    private Ast.RValue parse_extern (GLib.Queue<Token?> tokens, Token? token) throws GLib.Error
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
        throw new ParserError.EXPECTED_TOKEN ("%s: Unexpected end of input after call statement", identifier.locate ());
      else
      {
        if (next.klass.kind != TokenType.SEPARATOR || next.token != "(")
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          return parse_call (tokens, identifier, next, true);
        }
      }
    }

    private Ast.RValue parse_identifier (GLib.Queue<Token?> tokens, Token? identifier) throws GLib.Error
    {
      Token? next;

      next = tokens.pop_tail ();
      if (next == null)
        return new Ast.Variable (identifier.token);
      else
      {
        if (next.klass.kind != TokenType.SEPARATOR)
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          if (next.token == "(")
            return parse_call (tokens, identifier, next, false);
          else
          if (next.token == "=")
            return parse_assign (tokens, identifier, next);
          else
            throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        }
      }
    }

    private Ast.Scope parse_block (GLib.Queue<Token?> tokens, Token? begin) throws GLib.Error
    {
      var scope = new Ast.Scope ();
      var node = (Ast.Node) null;
      Token? token;

      while ((token = tokens.pop_tail ()) != null)
      {
        unowned var klass = token.klass;
        unowned var expr = token.token;

        switch (klass.kind)
        {
          case TokenType.IDENTIFIER:
            {
              node = parse_identifier (tokens, token);
              scope.append (node);
            }
            break;
          case TokenType.KEYWORD:
            {
              if (expr.has_prefix ("operator"))
              {
                node = define_operator (tokens, token);
                scope.append (node);
              }
              else
              if (expr == "extern")
              {
                node = parse_extern (tokens, token);
                scope.append (node);
              }
              else
                throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), expr);
            }
            break;
          case TokenType.SEPARATOR:
            {
              if (expr == "{")
              {
                node = parse_block (tokens, token);
                scope.append (node);
              }
              else
              if (expr == "}")
                return scope;
              else
              if (expr == ";") { }
              else
                throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), expr);
            }
            break;
          case TokenType.COMMENT:
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
        _tokens.push_head (token);

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
