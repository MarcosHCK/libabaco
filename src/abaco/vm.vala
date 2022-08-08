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
  [CCode (has_target = false)]
  public delegate int CClosure (Abaco.VM vm);

  public interface VM
  {
    public abstract void settop (int top);
    public abstract int gettop ();
    public abstract void pushvalue (int index);
    public abstract void pushupvalue (int index);
    public abstract void pop ();
    public abstract void exchange (int index);
    public abstract void insert (int index);
    public abstract void remove (int index);
    public abstract void pushcclosure (CClosure closure, int upvalues);

    public abstract bool loadbytes (GLib.Bytes bytes) throws GLib.Error;
    public virtual bool loadstring (string code) throws GLib.Error
    {
      var bytes = new GLib.Bytes.static (code.data);
      var result = loadbytes (bytes);
    return result;
    }

    public abstract int call (int args);
    public abstract void register_operator (string expr, bool assoc, int precedence, bool unary);
    public abstract void register_function (string expr);
  }
}
