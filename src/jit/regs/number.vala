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

namespace Abaco.Regs
{
  public interface Number : Base
  {
    public abstract void load (Abaco.Compilers.Base compiler, string expr);

    /* public API */

    public static GLib.Type select (string expr)
    {
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
        return typeof (Regs.Mpq);
    return typeof (Regs.Mpz);
    }
  }
}
