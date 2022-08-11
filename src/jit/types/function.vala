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

namespace Abaco.Types
{
  public sealed class Function : Types.Base, Types.Loadable
  {
    public override void init (Abaco.Compilers.Base state, Reg reg)
    {
      assert_not_reached ();
    }

    public override void fini (Abaco.Compilers.Base state, Reg reg)
    {
      assert_not_reached ();
    }

    public void load (Abaco.Compilers.Base state, string symbol, Reg reg)
    {
      assert_not_reached ();
    }
  }
}
