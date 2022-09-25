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
  public errordomain LexerError
  {
    FAILED,
    UNKNOWN_TOKEN,
  }

  internal sealed class Lexer : GLib.Object
  {
    public bool naked { get; construct; }
    private GenericArray<TokenClass> classes;
    private GLib.StringChunk chunk;

    /* private API */

    private static bool is_empty (string token, size_t length)
    {
      unowned var ptr = token;
      var left = token.char_count ((ssize_t) length);

      while (left-- > 0)
      {
        if (ptr.get_char () != (unichar) ' ')
          return false;
        ptr = ptr.next_char ();
      }
    return true;
    }

    private unowned string prepare (string token, size_t length)
    {
      unowned var ptr = token;
      var left = token.char_count ((ssize_t) length);
      bool tail = false;
      size_t tailing = 0;

      while (left-- > 0)
      {
        switch (ptr.get_char ())
        {
          case 0:
            assert_not_reached ();
          case ' ':
            if (tail)
              ++tailing;
            else
            {
              token = token.next_char ();
              --length;
            }
            break;
          default:
            if (tail)
              tailing = 0;
            else
              tail = true;
            break;
        }

        ptr = ptr.next_char ();
      }
    return chunk.insert_len (token, (ssize_t) (length - tailing));
    }

    private void add_operator (Token token)
    {
      var name = token.token.utf8_offset (/* sizeof ("operator") */ 8);
      classes.add (new TokenClass.escape (TokenType.OPERATOR, name));
    }

    private Token[] tokenizei (string input, size_t length, uint ioff) throws GLib.Error
    {
      var tokens = new Array<Token> ();
      var info = (GLib.MatchInfo) null;
      int i, start, stop;
      int last;

      for (i = 0; i < classes.length; i++)
      {
        unowned var klass = classes [i];
        unowned var rexp = klass.rexp;

        if (!rexp.match_full (input, (ssize_t) length, 0, 0, out info))
          continue;
        else
        {
          last = 0;
          while (info.matches ())
          {
            info.fetch_pos (0, out start, out stop);
            if (start > last)
            {
              var sub = tokenizei (input.offset (last), start - last, ioff + last);
              for (int j = 0; j < sub.length; j++)
                tokens.append_val (sub [j]);
            }

            info.next ();

            {
              var token = Token ();
              unowned var begin = input.offset (start);
              unowned var value = prepare (begin, stop - start);

              token.token = value;
              token.klass = klass;
              token.column = ioff + start;
              tokens.append_val (token);

              if (klass.kind == TokenType.KEYWORD
                && value.has_prefix ("operator"))
                  add_operator (token);
            }

            last = stop;
          }

          if (length > last)
          {
            var sub = tokenizei (input.offset (last), length - last, ioff + last);
            for (int j = 0; j < sub.length; j++)
              tokens.append_val (sub [j]);
          }
        }

        break;
      }

      if (tokens.length == 0)
      {
        if (is_empty (input, length))
          return tokens.steal ();
        throw new LexerError.UNKNOWN_TOKEN ("%lli: unknown token '%s'", ioff, prepare (input, length));
      }
    return tokens.steal ();
    }

    /* public API */

    public Token[] tokenize (GLib.DataInputStream stream, GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var tokens = new Array<Token> ();
      var linesz = (size_t) 0;
      var line = (string) null;
      var count = 1;

      while (true)
      {
        line = stream.read_line_utf8 (out linesz, cancellable);
        if (line == null)
          break;
        else
        {
          try
          {
            var sub = tokenizei (line, linesz, 1);
            for (int j = 0; j < sub.length; j++)
            {
              var token = sub [j];
                  token.line = count;
                  token.owner = this;
                  tokens.append_val (token);
            }
          } catch (GLib.Error e) {
            Patch.prefix_error (ref e, "%i: ", count);
            throw e;
          }
        }

        ++count;
      }
    return tokens.steal ();
    }

    /* constructor */

    public Lexer (bool naked)
    {
      Object (naked : naked);
    }

    construct
    {
      chunk = new GLib.StringChunk (128);
      classes = new GenericArray<TokenClass> ();
      classes.add (new TokenClass (TokenType.COMMENT, "/\\*(.*?)\\*/"));
      classes.add (new TokenClass (TokenType.LITERAL, "\"(.*?)\""));
      classes.add (new TokenClass (TokenType.LITERAL, "\'(.*?)\'"));
      classes.add (new TokenClass (TokenType.SEPARATOR, "[(){};,=]"));
      classes.add (new TokenClass.escape (TokenType.KEYWORD, "if"));
      classes.add (new TokenClass.escape (TokenType.KEYWORD, "else"));
      classes.add (new TokenClass.escape (TokenType.KEYWORD, "while"));
      classes.add (new TokenClass.escape (TokenType.KEYWORD, "extern"));
      classes.add (new TokenClass.escape (TokenType.KEYWORD, "return"));
      classes.add (new TokenClass (TokenType.KEYWORD, "operator([^_\\ ])"));
      classes.add (new TokenClass (TokenType.IDENTIFIER, "[a-zA-Z_][a-zA-Z_0-9]*"));
      classes.add (new TokenClass (TokenType.LITERAL, "[0-9\\.][a-zA-Z_0-9\\.]*"));
    }
  }
}
