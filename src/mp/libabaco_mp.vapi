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
  [CCode (cheader_filename = "libabaco_mp.h")]
  public class MP : GLib.Object, Abaco.VM
  {
    [CCode (cname = "ABACO_ASSOC_LEFT")]
    public const bool ASSOC_LEFT;
    [CCode (cname = "ABACO_ASSOC_RIGHT")]
    public const bool ASSOC_RIGHT;

    [CCode (cname = "MP_TYPE_NIL")]
    public const string TYPE_NIL;
    [CCode (cname = "MP_TYPE_VALUE")]
    public const string TYPE_VALUE;
    [CCode (cname = "MP_TYPE_INTEGER")]
    public const string TYPE_INTEGER;
    [CCode (cname = "MP_TYPE_RATIONAL")]
    public const string TYPE_RATIONAL;
    [CCode (cname = "MP_TYPE_REAL")]
    public const string TYPE_REAL;

    public static void load_stdlib (MP vm);

    [CCode (type = "AbacoVM*")]
    public MP ();
    [CCode (type = "AbacoVM*")]
    public MP.naked ();
    public unowned string typename (int index);
    public bool cast (int index, string type);
    public void pushdouble (double value);
    public void pushstring (string value, int @base);
    public bool isinteger (int index);
    public bool isrational (int index);
    public bool isreal (int index);
    public bool isnumber (int index);
    public double todouble (int index);
    public string tostring (int index, int @base);
  }
}
