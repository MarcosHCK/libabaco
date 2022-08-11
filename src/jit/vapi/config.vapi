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

[CCode (cprefix = "", lower_case_cprefix = "", cheader_filename = "config.h")]
namespace Config
{
  public const bool DEBUG;
  public const bool DEVELOPER;
  public const string GAPPNAME;
  public const string GEXTNAME;
  public const string GRESNAME;

  public const string ABSTOPBUILDDIR;
  public const string ASSETSDIR;
  public const string GFXDIR;
  public const string GIRDIR;
  public const string MODULESDIR;
  public const string PKGDATADIR;
  public const string PKGLIBDIR;
  public const string PKGLIBEXECDIR;
  public const string SCHEMASDIR;

  public const string PACKAGE_NAME;
  public const string PACKAGE_RELEASE;
  public const string PACKAGE_TARNAME;
  public const string PACKAGE_VERSION;
  public const string PACKAGE_STRING;
  public const string PACKAGE_BUGREPORT;
  public const string PACKAGE_URL;
  public const string PACKAGE_VERSION_MAYOR;
  public const string PACKAGE_VERSION_MINOR;
  public const string PACKAGE_VERSION_MICRO;
  public const string PACKAGE_VERSION_BUILD;
}
