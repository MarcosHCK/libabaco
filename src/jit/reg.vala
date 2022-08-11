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

namespace Abaco
{
  [Compact (opaque = true)]
  public class Reg
  {
    public uint index { get; private set; }
    public bool init { get { return klass != null; } }
    public GLib.Type type { get { return GLib.Type.from_instance ((void*) klass); } }
    private Types.Loadable? klass { get; set; }

    /* public API */

    public static void move (Compilers.Base state, Reg dst, Reg src)
    {
      dst.clear (state);
      dst.klass = src.klass;
      dst.klass.init (state, dst);
      dst.klass.copy (state, dst, src);
    }

    public void load (Compilers.Base state, string expr, GLib.Type type)
    {
      if (unlikely (init == true))
        error ("Register already initialized");
      if (unlikely (klass is Types.Loadable == false))
        error ("Can't load register type %s", type.name ());

      klass = Types.Loadable.create (type);
      klass.init (state, this);
      klass.load (state, expr, this);
    }

    public void clear (Compilers.Base state)
    {
      if (unlikely (init == true))
      {
        klass.fini (state, this);
        klass = null;
      }
    }

    /* Constructor */

    public Reg (uint index)
    {
      this.index = index;
      this.klass = null;
    }
  }
}
