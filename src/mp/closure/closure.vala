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
  public abstract class Closure
  {
    private Mp.Stack stack;
    public uint upvalues { get { return stack.get_length (); } }

    /* abstract API */

    public abstract int invoke (Abaco.MP vm);

    /* public API */

    public virtual void pushupvalue (int index, Stack dst)
    {
      if (index > upvalues || index < 0)
        error ("Invalid index");
      stack.push_index ((int) upvalues - index - 1);
      dst.transfer (stack);
    }

    /* constructors */

    protected Closure (Stack? src, int upvalues)
    {
      if (unlikely (upvalues < 0))
        error ("Upvalue number most be no negative");
      if (unlikely (upvalues > 0))
      {
        if (unlikely (src == null))
          error ("Upvalues needs a source stack");
        else
        if (unlikely (upvalues > src.get_length ()))
        {
          error ("Can't push more upvalues than values");
        }
      }

      stack = new Mp.Stack ();
      for (int i = 0; i < upvalues; i++)
        stack.transfer (src);
    }
  }
}
