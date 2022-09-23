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
    public bool freezed { get; set; default = false; }

    const string ASSOC_LEFT = "left";
    const string ASSOC_RIGHT = "right";

    /* type API */

    private enum LiteralType
    {
      STRING,
      NUMBER,
    }

    private struct Literal
    {
      public LiteralType type;
      public string value;
      public uint @base;

      /* public API */

      public uint to_uint () throws GLib.Error
      {
        var result = (uint) 0;
        var unparsed = (string) null;
        if (!uint.try_parse (value, out result, out unparsed, @base))
          throw new ParserError.LITERAL ("Literal doesn't fit in a unsigned int representation");
        if (unparsed != null && unparsed.length > 0)
          throw new ParserError.LITERAL ("'%s' junk after translation", unparsed);
      return result;
      }

      /* constructor */

      public Literal (string expr) throws GLib.Error
      {
        switch (expr.get_char ())
        {
          case (unichar) '\'':
          case (unichar) '\"':
            type = LiteralType.STRING;
            value = expr.substring (0, (long) (((char*) expr.index_of_nth_char (-1)) - ((char*) expr)));
            break;
          default:
            {
              unowned string value = expr;
              unowned string suffix = expr;
              unowned uint @base = 10;

              do
              {
                var c = suffix.get_char ();

                if (c == (unichar) 0)
                {
                  suffix = null;
                  break;
                }

                if (!c.isxdigit ())
                {
                  switch (c)
                  {
                    case 'b':
                      @base = 2;
                      break;
                    case 'o':
                      @base = 8;
                      break;
                    case 'd':
                      @base = 10;
                      break;
                    case 'h':
                      @base = 16;
                      break;
                    default:
                      throw new ParserError.LITERAL ("Unknown literal suffix %s", suffix);
                      break;
                  }

                  if (suffix.char_count () > 1)
                    throw new ParserError.LITERAL ("Unknown literal suffix %s", suffix);

                  break;
                }

                suffix = suffix.next_char ();
              } while (true);
  
              this.value = value.substring (0, (long) (((char*) suffix) - ((char*) value)));
              this.@base = @base;
            }
            break;
        }
      }
    }

    /* private API */

    private void define_operator (GLib.Queue<Token?> tokens, Ast.Scope scope, Token? token) throws GLib.Error
    {
      Token? next;
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
            assoc = ASSOC_LEFT;
          else
          if (assoc == "right")
            assoc = ASSOC_RIGHT;
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
          var decl = new Ast.Operator (token.token, precedence, assoc);
          parse_block (tokens, decl, next);
          scope.append (decl);
        }
      }
    }

    private void parse_rvalue (GLib.Queue<Token?> tokens, Ast.RValue scope) throws GLib.Error
    {
      assert_not_reached ();
    }

    private void parse_call (GLib.Queue<Token?> tokens, Ast.Call scope, Token? token) throws GLib.Error
    {
      assert_not_reached ();
    }

    private void parse_identifier (GLib.Queue<Token?> tokens, Ast.Scope scope, Token? token) throws GLib.Error
    {
      Token? next;

      next = tokens.pop_tail ();
      if (next == null)
        throw new ParserError.EXPECTED_TOKEN ("%s: Unexpected end of input after call statement", token.locate ());
      else
      {
        if (next.klass.kind != TokenType.SEPARATOR)
          throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", next.locate (), next.token);
        else
        {
          if (next.token == "(")
          {
            var call = new Ast.Call (token.token, false);
            parse_call (tokens, call, next);
            scope.append (call);
          }
          else
          if (next.token == "=")
          {
            var assign = new Ast.Assign (token.token);
            parse_rvalue (tokens, assign.rvalue);
            scope.append (assign);
          }
        }
      }
    }

    private void parse_extern (GLib.Queue<Token?> tokens, Ast.Scope scope, Token? token) throws GLib.Error
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
          var call = new Ast.Call (identifier.token, true);
          parse_call (tokens, call, identifier);
          scope.append (call);
        }
      }
    }

    private void parse_block (GLib.Queue<Token?> tokens, Ast.Scope scope, Token? begin) throws GLib.Error
    {
      Token? token;
      while ((token = tokens.pop_tail ()) != null)
      {
        unowned var klass = token.klass;
        unowned var expr = token.token;

        print ("t %u:%u '%s' '%s'\r\n", token.line, token.column, klass.kind.to_string (), expr);

        switch (klass.kind)
        {
          case TokenType.IDENTIFIER:
            parse_identifier (tokens, scope, token);
            break;
          case TokenType.KEYWORD:
            {
              if (expr.has_prefix ("operator"))
                define_operator (tokens, scope, token);
              if (expr == "extern")
                parse_extern (tokens, scope, token);
              else
                throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), token.token);
            }
            break;
          case TokenType.SEPARATOR:
            {
              if (expr == "{")
              {
                var scope2 = new Ast.Scope ();
                parse_block (tokens, scope2, token);
                scope.append (scope2);
              }
              else
                throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), token.token);
            }
            break;
          case TokenType.OPERATOR:
            throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), token.token);
            break;
          case TokenType.LITERAL:
            throw new ParserError.UNEXPECTED_TOKEN ("%s: Unexpected token '%s'", token.locate (), token.token);
            break;
          case TokenType.COMMENT:
            break;
        }

        if (klass.kind == TokenType.SEPARATOR && expr == "}")
          return;
      }

      throw new ParserError.EXPECTED_TOKEN ("%s: Expected end of block statement before end of input", begin.locate ());
    }

    /* public API */

    public Ast.Node parse (Token[] tokens) throws GLib.Error
      requires (tokens.length > 0)
    {
      if (freezed)
        error ("Frozen parser");

      var _tokens = new GLib.Queue<Token?> ();

      foreach (unowned var token in tokens)
        _tokens.push_head (token);

      var global = new Ast.Scope ();
      var klass = new TokenClass (TokenType.SEPARATOR, ".");
      var finish = Token ();
      var begin = Token ();

      {
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
        parse_block (_tokens, global, begin);
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
