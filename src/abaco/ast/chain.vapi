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
 * along with libabaco. If not, see <http://www.gnu.org/licenses/>.
 *
 */

namespace Abaco.Ast
{
  [CCode (cheader_filename = "chain.h", has_type_id = false, has_copy_function = false, cname = "AstChain", lower_case_cprefix = "_ast_chain_")]
  internal struct Chain
  {
    public void* self;
    public unowned Chain? next;
    public unowned Chain? prev;
    public unowned Chain? parent;
    public unowned Chain? children;

    public static void append (ref Chain a, ref Chain b);
    public static void prepend (ref Chain a, ref Chain b);
    [CCode (cname = "g_node_n_children")]
    public static uint n_children (ref Chain a);

    [DestroysInstance]
    public void destroy ();
  }
}
