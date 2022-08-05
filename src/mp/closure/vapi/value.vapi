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
  [CCode (cheader_filename = "value.h")]
  public class Stack
  {
    public Stack ();
    public int get_length ();
    public unowned string type (int index);
    public void transfer (Stack dst);
    public void push_index (int index);
    public void insert (int index);
    public void remove (int index);
    public void push_value (ref GLib.Value value);
    public void push_string (string value, int @base);
    public void push_double (double value);
    public void peek_value (int index, out GLib.Value value);
    public string peek_string (int index);
    public double peek_double (int index);
    public void pop ();
  }
}
