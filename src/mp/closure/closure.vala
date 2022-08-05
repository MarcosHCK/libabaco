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

[CCode (cprefix = "Mp", lower_case_cprefix = "_mp_")]
namespace Mp
{
  public abstract class Closure : Abaco.Closure
  {
    public Mp.Stack stack;

    /* abstract API */

    public abstract int invoke (Abaco.VM vm, int args) throws GLib.Error;

    /* public API */

    public virtual void pushupvalue (int index, Stack dst) throws GLib.Error
    {
    }

    /* constructors */

    protected Closure (Stack src, int upvalues)
      requires (upvalues > 0)
      requires (src.get_length () >= upvalues)
    {
      stack = new Mp.Stack ();
      for (int i = 0; i < upvalues; i++)
        src.transfer (stack);
    }
  }
}
