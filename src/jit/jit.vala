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
  public enum JitTargetArch
  {
    x86_64,
  }

  public class Jit : GLib.Object
  {
    private GenericSet<Relation> relations { get; private set; }
    private Abaco.Assembler assembler { get; set; }
    private Abaco.Rules rules { get; set; }

    public JitTargetArch target { get; private set; }
    public ConstantLoader kloader { get; set; }
    public FunctionLoader floader { get; set; }
    public FunctionInvoker invoker { get; set; }

    /* private API */

    [CCode (has_target = false)]
    public delegate void ConstantLoader (Compilers.Base compiler, string expr, Reg result);
    [CCode (has_target = false)]
    public delegate void FunctionLoader (Compilers.Base compiler, Relation relation, Reg result);
    [CCode (has_target = false)]
    public delegate void FunctionInvoker (Compilers.Base compiler, Reg[] arguments, Reg result);

    private class Section
    {
      private GLib.Bytes code;
      protected Bytecode.Section* head;
      protected uint8* data;

      /* Constructors */

      private static Bytecode.Section* select (Bytecode.SectionType type, GLib.Bytes code)
      {
        unowned Bytecode.Section* section;
        unowned var data = code.get_data ();
        unowned var ptr = (uint8*) data;
        unowned var top = ptr + data.length;

        while (ptr < top)
        {
          section = (Bytecode.Section*) ptr;
          if (section.type == type)
            return section;
          if (Bytecode.SectionFlags.VIRTUAL in section.flags)
            ptr += sizeof (Bytecode.Section);
          else
          {
            var size = section.size;
            var miss = size % Bytecode.SECTION_ALIGN;
            if (miss > 0)
              ptr += size + (Bytecode.SECTION_ALIGN - miss);
            else
              ptr += size;
          }
        }
      return null;
      }

      protected Section (Bytecode.SectionType type, GLib.Bytes code)
      {
        this.head = select (type, code);
        if (unlikely (this.head == null))
          error (@"Can't locate a section type $(type)");

        this.code = code;
        this.data = (uint8*) &(head [1]);
      }
    }

    private class StackSection : Section
    {
      private Reg[] stack;

      /* public API */

      public unowned Reg? take (uint index)
        requires (index < stack.length)
      {
        return stack [index];
      }
  
      /* Constructors */

      public StackSection (GLib.Bytes code)
      {
        base (Bytecode.SectionType.STACK, code);
        stack = new Reg [head.size];
        for (int i = 0; i < stack.length; i++)
          stack [i] = new Reg (i);
      }
    }

    private class StrtabSection : Section
    {
      public string[] strings { get; private set; }
      public uint size { get { return strings.length; } }

      /* public API */

      public unowned string? take (uint index)
        requires (index < strings.length)
      {
        return strings [index];
      }
  
      /* Constructors */

      public StrtabSection (GLib.Bytes code)
      {
        base (Bytecode.SectionType.STRTAB, code);
        var strings = new GenericArray<unowned string> ();

        unowned var ptr = (uint8*) data;
        unowned var top = ptr + head.size;
        unowned string str = null;

        while (ptr < top)
        {
          str = (string) ptr;
          strings.add (str);
          ptr += str.length + 1;
        }

        this.strings = strings.steal ();
      }
    }

    [SimpleType]
    private struct LoadIterator : uintptr
    {
      public void iterate (Relation relation)
      {
        unowned var args = (uintptr*) this;
        unowned var jit = (Jit) args [0];
        unowned var compiler = (Compilers.Base) args [1];
        unowned var expr = (string) args [2];
        unowned var result = (Reg) args [3];

        if (jit == null)
          return;

        if (relation.matches (expr))
        {
          jit.floader (compiler, relation, result);
          args [0] = (uintptr) null;
        }
      }
    }

    private void load_function (Compilers.Base compiler, string expr, Reg result)
    {
      LoadIterator iter;
      uintptr args [4];

      args [0] = (uintptr) this;
      args [1] = (uintptr) compiler;
      args [2] = (uintptr) expr;
      args [3] = (uintptr) result;
      iter = (LoadIterator) args;
      result.clear (compiler);

      relations.@foreach (iter.iterate);
      if (result.init == false)
        error ("Unknown expression '%s'", expr);
    }

    private Closure? compile (GLib.Bytes code) throws GLib.Error
    {
      var state = (Compilers.Base) null;

      switch (target)
      {
      case JitTargetArch.x86_64:
        state = new Compilers.x86_64 ();
        break;
      default:
        error ("Unknown architecture %s", target.to_string ());
      }

      var stack = new StackSection (code);
      var strtab = new StrtabSection (code);

      {
        unowned Bytecode.Section* section;
        unowned var data = code.get_data ();
        unowned var ptr = (uint8*) data;
        unowned var top = ptr + data.length;

        while (ptr < top)
        {
          section = (Bytecode.Section*) ptr;
          if (Bytecode.SectionType.BITS == section.type
            && Bytecode.SectionFlags.CODE in section.flags)
            {
              unowned Bytecode.Opcode* opcode;
              unowned var ptr2 = (uint8*) & section [1];
              unowned var top2 = ptr2 + (section.size - sizeof (Bytecode.Section));

              while (ptr2 < top2)
              {
                opcode = (Bytecode.Opcode*) ptr2;
                ptr2 += sizeof (Bytecode.Opcode);

                switch (opcode.code)
                {
                case Bytecode.Code.NOP:
                  break;
                case Bytecode.Code.MOVE:
                  {
                    unowned var dsti = (uint32) opcode.a;
                    unowned var srci = (uint32) opcode.b;
                    unowned var dst = stack.take (dsti);
                    unowned var src = stack.take (srci);
                    Reg.move (state, dst, src);
                  }
                  break;
                case Bytecode.Code.LOADK:
                  {
                    unowned var dsti = (uint32) opcode.a;
                    unowned var srci = (uint32) opcode.bx;
                    unowned var dst = stack.take (dsti);
                    unowned var src = strtab.take (srci);
                    kloader (state, src, dst);
                  }
                  break;
                case Bytecode.Code.LOADF:
                  {
                    unowned var dsti = (uint32) opcode.a;
                    unowned var srci = (uint32) opcode.bx;
                    unowned var dst = stack.take (dsti);
                    unowned var src = strtab.take (srci);
                    load_function (state, src, dst);
                  }
                  break;
                default:
                  error ("Invalid binary: invalid opcode %s", opcode.code.to_string ());
                  break;
                }
              }
            }

          if (Bytecode.SectionFlags.VIRTUAL in section.flags)
            ptr += sizeof (Bytecode.Section);
          else
          {
            var size = section.size;
            var miss = size % Bytecode.SECTION_ALIGN;
            if (miss > 0)
              ptr += size + (Bytecode.SECTION_ALIGN - miss);
            else
              ptr += size;
          }
        }
      }
    return null;
    }

    /* public API */

    public Closure? compile_bytes (GLib.Bytes code) throws GLib.Error
    {
      unowned var expr = code.get_data ();
      var tree = rules.parse ((string) expr, expr.length);
      var byte = assembler.assemble (tree);
    return compile (byte);
    }

    public Closure? compile_string (string code) throws GLib.Error
    {
      var bytes = new GLib.Bytes.static (code.data);
      var closure = compile_bytes (bytes);
    return closure;
    }

    public void add_operator (owned Relation relation, bool assoc, int precedence, bool unary)
    {
      try
      {
        var expr = relation.expr;
        relations.add ((owned) relation);
        rules.add_operator (expr, assoc, precedence, unary);
      }
      catch (GLib.Error e)
      {
        error (@"$(e.domain):$(e.code):$(e.message)");
      }
    }

    public void add_function (owned Relation relation, int arguments)
    {
      try
      {
        var expr = relation.expr;
        relations.add ((owned) relation);
        rules.add_function (expr, arguments);
      }
      catch (GLib.Error e)
      {
        error (@"$(e.domain):$(e.code):$(e.message)");
      }
    }

    /* Constructors */

    construct
    {
      unowned HashFunc<Relation> hash = Abaco.relation_hash;
      unowned EqualFunc<Relation> equal = Abaco.relation_equal;
      this.relations = new GenericSet<Relation> (hash, equal);
      this.assembler = new Abaco.Assembler ();
      this.rules = new Abaco.Rules ();
      this.target = target;
    }

    public Jit.naked (JitTargetArch target)
    {
      base ();
      kloader = () => { };
      floader = () => { };
      invoker = () => { };
    }

    public Jit (JitTargetArch target)
    {
      base ();
      load_default (this);
    }
  }
}
