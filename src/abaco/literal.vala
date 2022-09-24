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
  internal enum LiteralType
  {
    STRING,
    NUMBER,
  }

  internal struct Literal
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
}
