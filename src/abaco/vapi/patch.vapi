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
  [CCode (cprefix = "Ast")]
  namespace AstPatch
  {
    [CCode (cheader_filename = "astpatch.h", cprefix = "_ast_")]
    public struct Chain
    {
      public void* self;
      public Chain? next;
      public Chain? prev;
      public Chain? parent;
      public Chain? children;

      public static void append (ref Chain a, ref Chain b);
      public static void prepend (ref Chain a, ref Chain b);
      [CCode (cname = "g_node_n_children")]
      public static uint n_children (ref Chain a);
      public static void foreach_data (ref Chain a, Foreach callback);
    }

    [CCode (cheader_filename = "astpatch.h", cprefix = "_ast_")]
    public delegate void Foreach (void* data);
  }
}
