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
  internal enum TokenType
  {
    IDENTIFIER,
    KEYWORD,
    SEPARATOR,
    OPERATOR,
    LITERAL,
    COMMENT,
  }

  internal enum OperatorAssoc
  {
    LEFT = '<',
    RIGHT = '>',
  }

  internal class TokenClass
  {
    public TokenType kind { get; private set; }
    public GLib.Regex rexp { get; private set; }

    /* constructor */

    public TokenClass (TokenType type, string expr) throws GLib.Error
    {
      this.kind = type;
      this.rexp = new Regex (expr, RegexCompileFlags.OPTIMIZE);
    }

    public TokenClass.escape (TokenType type, string name) throws GLib.Error
    {
      this (type, Regex.escape_string (name));
    }
  }

  internal struct Token
  {
    public unowned string token;
    public unowned TokenClass klass;
    public Lexer owner;

    public uint line;
    public uint column;

    /* public API */

    public string locate ()
    {
      return @"$line: $column: ";
    }
  }
}
