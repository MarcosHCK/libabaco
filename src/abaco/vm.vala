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
  public interface Closure
  {
    public virtual int invoke (VM vm, int args) throws GLib.Error
    {
      warning ("%s::invoke not implemented for '%s'",
        typeof (Closure).name (), GLib.Type.from_instance (this).name ());
      return -1;
    }
  }

  public interface VM
  {
    public virtual int call (int args) throws GLib.Error
    {
      warning ("%s::call not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
      return -1;
    }

    public virtual void settop (int top)
    {
      warning ("%s::settop not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
    }

    public virtual int gettop ()
    {
      warning ("%s::gettop not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
      return -1;
    }

    public virtual void pushvalue (int index) throws GLib.Error
    {
      warning ("%s::pushvalue not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
    }

    public virtual void pushupvalue (int index) throws GLib.Error
    {
      warning ("%s::pushupvalue not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
    }

    public virtual void pop ()
    {
      warning ("%s::pop not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
    }

    public virtual void insert (int index)
    {
      warning ("%s::insert not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
    }

    public virtual void remove (int index)
    {
      warning ("%s::remove not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
    }

    public virtual void pushclosure (Closure closure, int upvalues) throws GLib.Error
    {
      warning ("%s::pushclosure not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
    }

    public virtual bool loadbytes (GLib.Bytes bytes) throws GLib.Error
    {
      warning ("%s::loadbytes not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
      return false;
    }

    public bool loadstring (string code) throws GLib.Error
    {
      var bytes = new GLib.Bytes (code.data);
      var result = loadbytes (bytes);
    return result;
    }

    public virtual void register_operator (string expr) throws GLib.Error
    {
      warning ("%s::register_operator not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
    }

    public virtual void register_function (string expr) throws GLib.Error
    {
      warning ("%s::register_function not implemented for '%s'",
        typeof (VM).name (), GLib.Type.from_instance (this).name ());
    }
  }
}
