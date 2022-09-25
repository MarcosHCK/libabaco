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
    public GLib.DataInputStream stream { get; construct; }
    public string source { get; construct; }
    public bool naked { get; construct; }

    private GenericArray<TokenClass> classes;
    private GLib.StringChunk chunk;

    /* private API */

    private static bool is_empty (string token, size_t length)
    {
      unowned var ptr = token;
      unowned var left = token.char_count ((ssize_t) length);

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
      unowned var left = token.char_count ((ssize_t) length);
      size_t tailing = 0;
      bool tail = false;

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

    private void add_operator (Token token) throws GLib.Error
    {
      unowned var nth = ("operator").char_count ();
      unowned var index = token.token.index_of_nth_char (nth);
      unowned var name = token.token.offset (index);

      classes.add (new TokenClass.escape (TokenType.OPERATOR, name));
    }

    private uint tokenizei (Array<Token> tokens, string input, size_t length, uint linen, uint coln) throws GLib.Error
    {
      var info = (GLib.MatchInfo) null;
      int i, start, stop;
      uint added = 0;
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
              added += tokenizei (tokens, input.offset (last), start - last, linen, coln + last);
            }

            info.next ();

            {
              var token = Token ();
              unowned var begin = input.offset (start);
              unowned var value = prepare (begin, stop - start);

              token.token = value;
              token.klass = klass;
              token.owner = this;
              token.line = linen;
              token.column = coln + start;
              tokens.append_val (token);
              ++added;

              if (klass.kind == TokenType.KEYWORD
                && value.has_prefix ("operator"))
                  add_operator (token);
            }

            last = stop;
          }

          if (length > last)
          {
            added += tokenizei (tokens, input.offset (last), length - last, linen, coln + last);
          }
        }

        break;
      }

      if (added == 0 && !is_empty (input, length))
        throw new LexerError.UNKNOWN_TOKEN ("%s: %u: %u: unknown token '%s'", source, linen, coln, prepare (input, length));
    return added;
    }

    private void tokenizes (Array<Token> tokens, GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var linesz = (size_t) 0;
      var line = (string) null;
      var count = 1;

      while ((line = stream.read_line_utf8 (out linesz, cancellable)) != null)
        tokenizei (tokens, line, linesz, count++, 1);
    }

    private void include (Array<Token> tokens, GLib.DataInputStream stream, string source, GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var lexer = new Lexer (stream, source, false);
          lexer.classes = classes;
          lexer.tokenizes (tokens, cancellable);
    }

    private void include_stdlib (Array<Token> tokens, string path, GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var stdlib = Patch.Stdlib.load ();
      var display = "resources://" + path;
      var input = stdlib.open_stream (path, 0);
      var stream = new GLib.DataInputStream (input);

      include (tokens, stream, display, cancellable);
    }

    /* public API */

    public Token[] tokenize (GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var tokens = new Array<Token> ();
      include_stdlib (tokens, "/org/hck/libabaco/stdlib/arithmetics.abc", cancellable);
      tokenizes (tokens, cancellable);
    return tokens.steal ();
    }

    /* constructor */

    public Lexer (GLib.DataInputStream stream, string source, bool naked)
    {
      Object (stream : stream, source : source, naked : naked);
    }

    construct
    {
      chunk = new GLib.StringChunk (2048);
      classes = new GenericArray<TokenClass> ();

      try
      {
        classes.add (new TokenClass (TokenType.COMMENT, "/\\*(.*?)\\*/"));
        classes.add (new TokenClass (TokenType.KEYWORD, "operator([^_\\ ])"));
        classes.add (new TokenClass (TokenType.LITERAL, "\"(.*?)\""));
        classes.add (new TokenClass (TokenType.LITERAL, "\'(.*?)\'"));
        classes.add (new TokenClass (TokenType.SEPARATOR, "[(){};,=]"));
        classes.add (new TokenClass.escape (TokenType.KEYWORD, "if"));
        classes.add (new TokenClass.escape (TokenType.KEYWORD, "else"));
        classes.add (new TokenClass.escape (TokenType.KEYWORD, "while"));
        classes.add (new TokenClass.escape (TokenType.KEYWORD, "extern"));
        classes.add (new TokenClass.escape (TokenType.KEYWORD, "return"));
        classes.add (new TokenClass.escape (TokenType.KEYWORD, "function"));
        classes.add (new TokenClass (TokenType.IDENTIFIER, "[a-zA-Z_][a-zA-Z_0-9]*"));
        classes.add (new TokenClass (TokenType.LITERAL, "[0-9\\.][a-zA-Z_0-9\\.]*"));
      } catch (GLib.Error e) {
        error (@"$(e.domain):$(e.code):$(e.message)");
        assert_not_reached ();
      }
    }
  }
}
