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
    jit.add_operator
    (new Relation.with_name
     ("+",
      new GLib.Type []
        {
          typeof (Regs.Number),
          typeof (Regs.Number),
        },
      typeof (Regs.Number),
      (compiler, expr, args, result) =>
        {
          assert_not_reached ();
        }),
     false, 2, false);
  }
}
