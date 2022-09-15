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

namespace Ucl
{
  [CCode (cheader_filename = "libabaco_ucl.h", copy_function = "__should_not_copy", destroy_function = "ucl_reg_unset", type_id = "ucl_reg_get_type ()")]
  public struct Reg
  {
    public RegType type;

    [CCode (cname = "ucl_reg_clear")]
    public Reg ();
    [CCode (cname = "ucl_reg_setup")]
    private static void reg_setup (ref unowned Reg reg, RegType type);
    [CCode (cname = "_ucl_reg_setup")]
    public Reg.setup (RegType type)
    {
      this ();
      reg_setup (ref this, type);
    }

    public static void clears ([CCode (array_length_type = "guint")] Reg[] regs);
    public static void unsets ([CCode (array_length_type = "guint")] Reg[] regs);
    public static void copy (ref Reg dst, ref Reg src);
    public static void cast (ref Reg dst, ref Reg src, RegType type);

    public void load_double (double value);
    public bool load_string (string value, int @base);
    public bool load (GLib.Value value);
    public double save_double ();
    public string save_string (int @base);

    [CCode (cname = "ucl_arithmetic_add")]
    public void add (Reg next);
    [CCode (cname = "ucl_arithmetic_sub")]
    public void sub (Reg next);
    [CCode (cname = "ucl_arithmetic_mul")]
    public void mul (Reg next);
    [CCode (cname = "ucl_arithmetic_div")]
    public void div (Reg next);
    [CCode (cname = "ucl_power_pow")]
    public void pow (Reg next);
  }

  [CCode (cheader_filename = "libabaco_ucl.h")]
  public enum RegType
  {
    VOID,
    POINTER,
    INTEGER,
    RATIONAL,
    REAL,
  }
}
