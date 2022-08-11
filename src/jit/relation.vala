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
using Abaco.Compilers;
using Abaco.Types;

namespace Abaco
{
  [Compact (opaque = true)]
  public class Relation
  {
    public GLib.Type[] arguments { get; private set; }
    public GLib.Type result { get; private set; }
    public RelationCompiler compiler { get; private owned set; }
    private GLib.Regex? regex;

    public string? name
    {
      get { return regex == null ? null : regex.get_pattern (); }
      set
      {
        GLib.RegexCompileFlags flags;
        string flush;

        try
        {
          flags = GLib.RegexCompileFlags.OPTIMIZE;
          flush = GLib.Regex.escape_string (value);
          regex = new GLib.Regex (flush, flags, 0);
        } catch (GLib.Error e)
        {
          error (@"$(e.domain):$(e.code):$(e.message)");
        }
      }
    }

    public string? expr
    {
      get { return regex == null ? null : regex.get_pattern (); }
      set
      {
        GLib.RegexCompileFlags flags;

        try
        {
          flags = GLib.RegexCompileFlags.OPTIMIZE;
          regex = new GLib.Regex (value, flags, 0);
        } catch (GLib.Error e)
        {
          error (@"$(e.domain):$(e.code):$(e.message)");
        }
      }
    }

    /* public API */

    public bool matches (string expr)
    {
      if (regex.match (expr, 0, null))
        return true;
    return false;
    }

    /* Constructors */

    public Relation.with_name (string name, GLib.Type[] arguments, GLib.Type result, owned RelationCompiler compiler)
    {
      this (arguments, result, (owned) compiler);
      this.name = name;
    }

    public Relation.with_expression (string expr, GLib.Type[] arguments, GLib.Type result, owned RelationCompiler compiler)
    {
      this (arguments, result, (owned) compiler);
      this.expr = expr;
    }

    public Relation (GLib.Type[] arguments, GLib.Type result, owned RelationCompiler compiler)
    {
      this.arguments = arguments;
      this.result = result;
      this.compiler = (owned) compiler;

      foreach (var type in arguments)
      if (!type.is_a (typeof (Abaco.Types.Base)) && type != typeof (Abaco.Types.Base))
        error ("Argument must be derived from '%s'", typeof (Abaco.Types.Base).name ());
      if (!result.is_a (typeof (Abaco.Types.Base)) && result != typeof (Abaco.Types.Base))
        error ("Argument must be derived from '%s'", typeof (Abaco.Types.Base).name ());
    }
  }

  public delegate void RelationCompiler (Abaco.Compilers.Base compiler, string expr, Reg[] arguments, Reg result);

  public static uint relation_hash (Relation self)
  {
    var hash = (uint) 0;
    hash ^= GLib.str_hash ((string) self.name);
    hash ^= GLib.int_hash ((int) self.result);
    foreach (var type in self.arguments)
      hash ^= GLib.int_hash ((int) type);
  return hash;
  }

  private static bool arguments_equal (GLib.Type[] args1, GLib.Type[] args2)
  {
    if (args1.length != args2.length)
      return false;
    else
    {
      int i;
      for (i = 0; i < args1.length; i++)
      {
        if (args1 [i] != args2 [i])
          return false;
      }
    }
  return true;
  }

  public static bool relation_equal (Relation self1, Relation self2)
  {
    return
       self1.name == self2.name
    && self1.result == self2.result
    && arguments_equal
     (self1.arguments,
      self2.arguments);
  }
}
