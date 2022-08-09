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

[CCode (cprefix = "B", lower_case_cprefix = "b_")]
namespace Abaco.Bytecode
{
  public const int SECTION_ALIGN;

  [CCode (cheader_filename = "bytecode.h")]
  public struct Header
  {
    public uint8 magic [4];
    public uint32 sections;
  }

  [CCode (cheader_filename = "bytecode.h")]
  public struct Section
  {
    public uint16 name;
    public uint8 type;
    public uint8 flags;
    public uint32 size;
  }

  [CCode (cheader_filename = "bytecode.h", has_type_id = false)]
  public enum SectionType
  {
    BITS,
    STACK,
    STRTAB,
    NOTES,
  }

  [Flags]
  [CCode (cheader_filename = "bytecode.h", cprefix = "B_SECTION_", has_type_id = false)]
  public enum SectionFlags
  {
    READ,
    WRITE,
    EXECUTE,
    VIRTUAL,
    DATA,
    CODE,
    BSS,
  }

  [CCode (cheader_filename = "bytecode.h")]
  public struct Note
  {
    public uint16 key;
    public uint16 value;
  }

  [CCode (cheader_filename = "bytecode.h")]
  public class NoteNamespace
  {
    public const string SYMBOLS;
    public const string DEBUG;
  }

  [CCode (cheader_filename = "bytecode.h")]
  public struct Opcode
  {
    public Code code;
    [CCode (cname = "abc.a")]
    public uint a;
    [CCode (cname = "abc.b")]
    public uint b;
    [CCode (cname = "abc.c")]
    public uint c;
    [CCode (cname = "abx.bx")]
    public uint bx;
    [CCode (cname = "asbx.sbx")]
    public uint sbx;
  }

  [CCode (cheader_filename = "bytecode.h", cprefix = "B_OPCODE_")]
  public enum Code
  {
    NOP,
    MOVE,
    LOADK,
    LOADF,
    CALL,
    RETURN,
  }
}
