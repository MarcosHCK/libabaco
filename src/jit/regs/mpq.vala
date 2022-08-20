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
  public sealed class Mpq : Base, Number
  {
    public override void init (Abaco.Compilers.Base compiler)
    {
      message ("%s::init invoked", (GLib.Type.from_instance ((void*) this)).name ());
    }

    public override void fini (Abaco.Compilers.Base compiler)
    {
      message ("%s::fini invoked", (GLib.Type.from_instance ((void*) this)).name ());
    }

    public override void copy (Abaco.Compilers.Base compiler, Base src)
    {
      message ("%s::copy invoked", (GLib.Type.from_instance ((void*) this)).name ());
    }

    public void load (Abaco.Compilers.Base compiler, string expr)
    {
      message ("%s::load invoked", (GLib.Type.from_instance ((void*) this)).name ());
    }

    /* Constructors */

    public Mpq (uint index)
    {
      base (index);
    }
  }
}
