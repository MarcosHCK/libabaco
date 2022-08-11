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
using Abaco.Compilers;

namespace Abaco
{
  internal static void load_default (Jit jit)
  {
    jit.kloader =
    (compiler, expr, result) => {
      var type = GLib.Type.INVALID;
      var ratio = false;

      for (var ptr = expr; ptr[0] != 0; ptr = ptr.next_char ())
      {
        var c = ptr.get_char ();
        if (c == '.' || c == '/')
        {
          ratio = true;
          break;
        }
      }

      if (ratio)
        type = typeof (Abaco.Types.Mpq);
      else
        type = typeof (Abaco.Types.Mpz);

      result.clear (compiler);
      result.load (compiler, expr, type);
    };

    jit.floader =
    (compiler, relation, result) => {
      var type = typeof (Abaco.Types.Function);

      result.clear (compiler);
      result.load (compiler, expr, type);
    };

    jit.add_operator
    (new Relation.with_name
     ("+",
      new GLib.Type []
        {
          typeof (Types.Number),
          typeof (Types.Number),
        },
      typeof (Types.Number),
      (compiler, expr, args, result) =>
        {
          assert_not_reached ();
        }),
     false, 2, false);
  }
}
