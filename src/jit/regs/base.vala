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
  public abstract class Base
  {
    public uint index { get; private set; }
    public abstract void init (Abaco.Compilers.Base compiler);
    public abstract void fini (Abaco.Compilers.Base compiler);
    public abstract void copy (Abaco.Compilers.Base compiler, Base src);

    /* Constructors */

    public static Base create (GLib.Type type, uint index)
      requires (type.is_a (typeof (Base)) && type != typeof (Base))
    {
      var self = (Base) null;
      if (type == typeof (Regs.Mpz))
        self = new Regs.Mpz (index);
      else
      if (type == typeof (Regs.Mpq))
        self = new Regs.Mpq (index);
      else
      if (type == typeof (Regs.Function))
        self = new Regs.Function (index);
      else
      {
        error ("Unknown type %s", type.name ());
      }
    return self;
    }

    protected Base (uint index)
    {
      this.index = index;
    }
  }
}
