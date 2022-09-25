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
  public sealed class Compiler : GLib.Object
  {
    public bool naked { get; construct; }

    /* public API */

    public GLib.Bytes compile_bytes (GLib.Bytes code) throws GLib.Error
    {
      return compile (new MemoryInputStream.from_bytes (code));
    }

    public GLib.Bytes compile_string (string code) throws GLib.Error
    {
      return compile (new MemoryInputStream.from_data (code.data));
    }

    public GLib.Bytes compile (GLib.InputStream input) throws GLib.Error
    {
      var stream = new GLib.DataInputStream (input);
      var tokens = (new Lexer (stream, "string", naked)).tokenize ();
      var tree = (new Parser (naked)).parse (tokens);
    return null;
    }

    /* constructor */

    public Compiler (bool naked)
    {
      Object (naked : naked);
    }

    construct
    {
    }
  }
}
