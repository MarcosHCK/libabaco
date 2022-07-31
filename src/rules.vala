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
  public errordomain ExpressionError
  {
    FAILED,
    NOT_UTF8,
    UNKNOWN_TOKEN,
    EXPECTED_TOKEN,
    UNEXPECTED_TOKEN,
    EXPECTED_EXPRESSION,
    UNMATCHED_PARENTHESIS,
  }

  public sealed class Rules : GLib.Object
  {
    private GenericArray<GLib.Regex> tokexp;
    private Array<ClassEntry> clsexp;
    private int fn_token = -1;
    private int fn_class = -1;

  /*
   * properties
   *
   */

    public bool code_strict { get; set; }

  /*
   * types
   *
   */

    [Compact]
    class ClassEntry
    {
      public SymbolClass klass;
      public GLib.Regex regex;
    }

    [Compact]
    class Tokens
    {
      public GLib.StringChunk chunk = new GLib.StringChunk (128);
      public GenericArray<unowned string?> array;
    }

  /*
   * private API
   *
   */

    private void add_token (string expr, int pos) throws GLib.Error
    {
      var flags = RegexCompileFlags.OPTIMIZE;
      var regex = new GLib.Regex (expr, flags);
      var at = (pos == -1) ? tokexp.length : pos;
      tokexp.insert (at, regex);
    }

    private void add_class (string expr, int pos, ref SymbolClass klass) throws GLib.Error
    {
      var flags = RegexCompileFlags.OPTIMIZE;
      var regex = new GLib.Regex (expr, flags);
      var at = (pos == -1) ? clsexp.length : pos;

      clsexp.insert_val (at, new ClassEntry ());
      clsexp.index (at).regex = regex;
      clsexp.index (at).klass = klass;
    }

    private bool _validate_len (string? input, ref ssize_t length)
      requires (input != null)
    {
      char* end = null;
      bool valid = false;

      valid = input.validate (length, out end);
      length = (ssize_t) (end - (char*) input);
    return valid;
    }

    private GenericArray<unowned string?>
    tokenizei (string input, ssize_t length, GLib.StringChunk chunk) throws GLib.Error
    {
      var tokens = new GenericArray<unowned string?> ();
      var info = (GLib.MatchInfo) null;
      int i, start, stop;
      int last;

      var tokexps = tokexp.length;
      for (i = 0; i < tokexps; i++)
      {
        unowned var regex = tokexp [i];
        if (!regex.match_full (input, length, 0, 0, out info))
          continue;
        else
        {
          last = 0;
          while (info.matches ())
          {
            info.fetch_pos (0, out start, out stop);
            if (start > last)
              {
                var sub = tokenizei (input.offset (last), start - last, chunk);
                unowned var token = (string) null;
                unowned var j = (int) 0;

                while ((token = sub [j++]) != null)
                  tokens.add (token);
              }

            last = stop;

            unowned var begin = input.offset (start);
            unowned var copy = chunk.insert_len (begin, stop - start);
            tokens.add (copy);
            info.next ();
          }

          if (length > last)
          {
            var sub = tokenizei (input.offset (last), length - last, chunk);
            unowned var token = (string) null;
            unowned var j = (int) 0;

            while ((token = sub [j++]) != null)
              tokens.add (token);
          }
        }

        break;
      }

      if (tokens.length == 0)
        {
          var copy =
          chunk.insert_len (input, length);
          tokens.add (copy);
        }

      tokens.add (null);
    return tokens;
    }

    private Tokens tokenize (string? input, ssize_t length) throws GLib.Error
      requires (_validate_len (input, ref length))
    {
      var tokens = new Tokens ();
      tokens.array = tokenizei (input, length, tokens.chunk);
    return tokens;
    }

    private unowned SymbolClass* classify (string? input, ssize_t length) throws GLib.Error
      requires (_validate_len (input, ref length))
    {
      var clsexps = clsexp.length;
      for (int i = 0; i < clsexps; i++)
      {
        unowned ClassEntry klass = clsexp.index (i);
        unowned GLib.Regex regex = klass.regex;
        if (regex.match_full (input, length, 0, 0, null))
          return & klass.klass;
      }
    return null;
    }

  /*
   * public API
   *
   */

    public void add_constant (string expr) throws GLib.Error
    {
      var klass = SymbolClass ();
      klass.kind = SymbolKind.VARIABLE;
      add_token (expr, -1);
      add_class (expr, -1, ref klass);
    }

    public void add_operator (string expr, bool assoc, uint precedence, bool unary) throws GLib.Error
    {
      var klass = SymbolClass ();
      klass.kind = SymbolKind.OPERATOR;
      klass.opclass.assoc = (OperatorAssoc) (int) assoc;
      klass.opclass.precedence = precedence;
      klass.opclass.unary = unary;

      add_class (expr, fn_class++ - 1, ref klass);
    }

    public void add_function (string expr, int args) throws GLib.Error
    {
      var klass = SymbolClass ();
      klass.kind = SymbolKind.FUNCTION;
      klass.fnclass.args = args;

      add_token (expr, fn_token);
      add_class (expr, fn_class, ref klass);
    }

    private ssize_t calculate_offset (string[] tokens, uint til)
    {
      ssize_t offset = 0;
      int i = 0;

      while (i < til)
      {
    #if DEBUG == 1
        if (tokens [i++] == null)
          error ("PANIC!");
    #endif // DEBUG
        offset += tokens [i].char_count ();
      }
    return offset;
    }

    public Ast.Node parse (string? input, ssize_t length = -1) throws GLib.Error
    {
      if (!_validate_len (input, ref length))
        throw new ExpressionError.NOT_UTF8 ("");
      var tokens_ = tokenize (input, length);
      var parser = new Parser ();
      unowned var tokens = tokens_.array.data;
      unowned var klass = (SymbolClass*) null;
      unowned var token = (string) null;
      unowned var t = (int) (-1);

      parser.code_strict = code_strict;

      while ((token = tokens [++t]) != null)
      {
        klass = classify (token, -1);
        if (klass == null)
        {
          var offset = calculate_offset (tokens, t);
          var msg = ("%lli: unclassed token '%s'").printf (offset, token);
          throw new ExpressionError.FAILED (msg);
        }
        else
        {
          try
          {
            parser.consume (token, *klass);
          } catch (ExpressionError e)
          {
            var emit = (GLib.Error) null;
            Error.propagate_prefixed
            (out emit, e, "%lli: ",
              calculate_offset (tokens, t));
            throw emit;
          }
        }
      }
    return parser.finish ();
    } 

  /*
   * Constructors
   *
   */

    construct
    {
      tokexp = new GenericArray<GLib.Regex> ();
      clsexp = new Array<ClassEntry> ();
      SymbolClass klass;

      try
      {
        /* All things which is not a letter or a number */
        add_token ("[^a-zA-Z0-9\\.]", -1);

        /* Parenthesis */
        klass = SymbolClass ();
        klass.kind = SymbolKind.PARENTHESIS;
        add_class ("[\\(\\)]", -1, ref klass);

        klass = SymbolClass ();
        klass.kind = SymbolKind.COMMA;
        add_class ("[\\,]", -1, ref klass);

        fn_token = (int) tokexp.length;
        fn_class = (int) clsexp.length;

        /* Variables */
        klass = SymbolClass ();
        klass.kind = SymbolKind.VARIABLE;
        add_token ("[\\p{L}]", -1);
        add_class ("[\\p{L}]", -1, ref klass);

        /* Constants */
        klass = SymbolClass ();
        klass.kind = SymbolKind.CONSTANT;
        add_token ("[0-9\\.]+", -1);
        add_class ("[0-9\\.]+", -1, ref klass);

        klass = SymbolClass ();
        klass.kind = SymbolKind.UNKNOWN;
        add_class (".*", -1, ref klass);
      }
      catch (GLib.Error e)
      {
        critical (@"$(e.domain):$(e.code):$(e.message)");
        assert_not_reached ();
      }
    }

    public Rules ()
    {
      Object ();
    }
  }
}
