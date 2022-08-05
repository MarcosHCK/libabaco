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
using Abaco.Bytecode;

namespace Abaco
{
  public class Assembler : GLib.Object
  {
    const int STRTAB_BLOCKSZ = 256;
    const int STRIDX_PREALLOC = 32;
    static GLib.Bytes trash;

    private class Blob : GLib.MemoryOutputStream
    {
      /* public API */

      public virtual GLib.Bytes finish () throws GLib.Error
      {
        var cancel = GLib.Cancellable.get_current ();
        this.close (cancel);
      return this.steal_as_bytes ();
      }

      /* constructor */

      public Blob ()
      {
        Object (
          data : null,
          size : 0,
          realloc_function : GLib.realloc,
          destroy_function : GLib.free
        );
      }
    }

    private class Section : Blob
    {
      public string name { get; private set; }
      public SectionType types { get; set; }
      public SectionFlags flags { get; set; }
      public new size_t size { get; set; }

      public Section (string name)
      {
        this.name = name;
      }
    }

    private class Binary : Blob
    {
      public StrtabSection strtab { get; private set; }

      /* public API */

      public void put (Section section) throws GLib.Error
      {
        var sectsz = sizeof (Abaco.Bytecode.Section);
        var align = Abaco.Bytecode.SECTION_ALIGN;
        var header = Abaco.Bytecode.Section ();
        var bytes = section.finish ();
        var padding = (int) 0;
        var name = (uint) 0;

        if (SectionFlags.VIRTUAL in section.flags)
          header.size = (uint32) section.size;
        else
        {
          var size = bytes.get_size () + sectsz;
          var miss = size % align;
          if (miss > 0)
          {
            padding = align - (int) miss;
            header.size = (uint32) (size + padding);
          }
          else
          {
            header.size = (uint32) (size);
          }
        }

        name = strtab.intern (section.name);
        header.name = (uint16) name;
        header.type = section.types;
        header.flags = section.flags;

        this.write_all ((uint8[]) &header, null);
        this.write_all (bytes.get_data (), null);

        if (padding > 0)
        {
          unowned var buffer = trash.get_data ();
                      buffer.length = padding;
          this.write_all (buffer, null);
        }
      }

      public override GLib.Bytes finish () throws GLib.Error
      {
        this.put (strtab);
      return base.finish ();
      }

      /* constructors */

      public Binary ()
      {
        this.strtab = new StrtabSection (".strtab");
      }
    }

    private class CodeSection : Section
    {
      private GLib.Queue<uint> stack;

      /* public API */

      public void put (Opcode opcode) throws GLib.Error
      {
        unowned var buffer = (uint8[]) & opcode;
        this.write_all (buffer, null);
      }

      public uint[] prepcall (StackSection stack, int nth) throws GLib.Error
        requires (nth > 0)
      {
        var regs = new uint [nth];
        var aligned = true;
        int i;

        for (i = nth - 1; i >= 0; i--)
        {
          regs [i] = pop ();
          if (i != (nth - 1) && ((regs [i] + 1) != regs [i + 1]))
            aligned = false;
        }

        if (!aligned)
        {
          var opcode = Opcode ();
          var dump = regs;
              regs = new uint [nth];

          stack.alloca (regs);
          for (i = 0; i < nth; i++)
          {
            opcode.code = Code.MOVE;
            opcode.a = (uint8) regs [i];
            opcode.b = (uint8) dump [i];

            this.put (opcode);
            stack.unalloc (dump [i]);
          }
        }
      return regs;
      }

      public void push (uint registry)
      {
        stack.push_head (registry);
      }

      public uint pop ()
      {
        if (stack.length == 0)
          error ("Empty stack");
        else
          return stack.pop_head ();
      }

      public void check ()
      {
        if (stack.length > 0)
          error ("Leftover registry on stack");
      }

      /* Constructors */

      public CodeSection (string name)
      {
        base (name);
        this.types = SectionType.BITS;
        this.flags = SectionFlags.CODE;
        this.stack = new GLib.Queue<uint> ();
      }
    }

    private class StackSection : Section
    {
      private GLib.Queue<uint> unused;
      private uint total;

      /* public API */

      public uint alloc ()
      {
        if (total == 255)
          error ("Maximun register number reached");
        if (unused.length > 0)
          return unused.pop_head ();
        else
          return total++;
      }

      public void unalloc (uint reg)
      {
        unused.push_head (reg);
      }

      public void alloca (uint[] regs)
      {
        uint i, length = regs.length;
        for (i = 0; i < length; i++)
          regs [i] = total++;
      }

      public void allocs (uint[] regs)
      {
        uint i, length = regs.length;
        for (i = 0; i < length; i++)
          regs [i] = alloc ();
      }

      public void unallocs (uint[] regs)
      {
        uint i, length = regs.length;
        for (i = 0; i < length; i++)
          unalloc (regs [i]);
      }

      public override GLib.Bytes finish () throws GLib.Error
      {
        size = total;
        return new GLib.Bytes (null);
      }

      public void check ()
      {
        if (unused.length != total)
          error ("Leftover registry not freed");
      }

      /* Constructors */

      public StackSection (string name)
      {
        base (name);
        this.types = SectionType.STACK;
        this.flags = SectionFlags.BSS;
        this.unused = new GLib.Queue<uint> ();
        this.total = 0;
      }
    }

    private class StrtabSection : Section
    {
      private GLib.HashTable<unowned string?, uint> strtab;
      private GLib.GenericArray<unowned string?> stridx;
      private GLib.StringChunk store;

      /* public API */

      public uint intern (string value)
      {
        uint idx = 0;
        if (!strtab.lookup_extended (value, null, out idx))
        {
          idx = stridx.length;
          value = store.insert (value);
          strtab.insert (value, idx);
          stridx.add (value);
        }
      return idx;
      }

      public override GLib.Bytes finish () throws GLib.Error
      {
        foreach (var str in stridx)
        {
          unowned var wrote = (size_t) 0;
          unowned var length = str.length + 1;
          unowned var buffer = str.data;
                      buffer.length = length;
          this.write_all (buffer, out wrote);
        }
      return base.finish ();
      }

      /* constructors */

      public StrtabSection (string name)
      {
        base (name);
        this.types = SectionType.STRTAB;
        this.flags = SectionFlags.DATA;
        this.strtab = new GLib.HashTable<unowned string?, uint> (GLib.str_hash, GLib.str_equal);
        this.stridx = new GLib.GenericArray<unowned string?> (STRIDX_PREALLOC);
        this.store = new GLib.StringChunk (STRTAB_BLOCKSZ);
      }
    }

    [Compact (opaque = true)]
    private class Arguments
    {
      private GLib.HashTable<string, uint> names;
      private uint nargs = 0;

      /* public API */

      public uint lookup (string key)
      {
        uint idx = 0;
        if (!names.lookup_extended (key, null, out idx))
          error ("Unknown variable '%s'", key);
      return idx;
      }

      public void count (Ast.Node node)
      {
        node.children_foreach (count);
        if (node.kind == Ast.SymbolKind.VARIABLE)
        {
          unowned var key = node.symbol;
          if (!names.lookup_extended (key, null, null))
          {
            var idx = nargs++;
            names.insert (key, idx);
          }
        }
      }

      public void alloc (StackSection stack)
      {
        uint i, reg, last = -1;
        for (i = 0; i < nargs; i++)
        {
          reg = stack.alloc ();
          if (i == 0)
          { 
            if (reg != 0)
              error ("First argument MUST be register 0");
          }
          else
          {
            if (last != reg - 1)
              error ("Argument registers MUST be aligned");
          }

          last = reg;
        }
      }

      public void unalloc (StackSection stack)
      {
        uint i;
        for (i = 0; i < nargs; i++)
          stack.unalloc (i);
      }

      /* constructors */

      public Arguments ()
      {
        names = new GLib.HashTable<string, uint> (GLib.str_hash, GLib.str_equal);
      }
    }

    [SimpleType]
    private struct Compiler : uintptr
    {
      public void compile (Ast.Node node, Arguments args, Section[] sections) throws GLib.Error
      {
        unowned var code = (CodeSection) sections [0];
        unowned var stack = (StackSection) sections [1];
        unowned var strtab = (StrtabSection) sections [2];
        unowned var symbol = node.symbol;
        unowned var kind = node.kind;
        var opcode = Opcode ();
        var reg = (uint) 0;

        switch (kind)
        {
        case Ast.SymbolKind.CONSTANT:
          reg = stack.alloc ();
          opcode.code = Code.LOADK;
          opcode.a = reg;
          opcode.bx = strtab.intern (symbol);
          code.put (opcode);
          code.push (reg);
          break;
        case Ast.SymbolKind.VARIABLE:
          reg = stack.alloc ();
          opcode.code = Code.MOVE;
          opcode.a = reg;
          opcode.b = args.lookup (symbol);
          code.put (opcode);
          code.push (reg);
          break;
        case Ast.SymbolKind.FUNCTION:
          {
            var nth = (int) node.n_children ();
            uint[] regs = (nth > 0) ? code.prepcall (stack, nth) : null;
            reg = stack.alloc ();

            opcode.code = Code.LOADF;
            opcode.a = reg;
            opcode.bx = strtab.intern (symbol);
            code.put (opcode);

            opcode.code = Code.CALL;
            opcode.a = reg;
            opcode.b = (nth > 0) ? (uint16) regs [0] : 0;
            opcode.c = (uint16) nth;
            code.put (opcode);

            stack.unallocs (regs);
            code.push (reg);
          }
          break;
        }
      }

      public void traverse (Ast.Node node)
      {
        unowned var args = (uintptr*) this;
        node.children_foreach (traverse);
        if (unlikely (args [0] != 0))
          return;

        /* compile */

        try
        {
          unowned var arguments = (Arguments) args [1];
          unowned var sections = (Section[]) & args [2];
                      sections.length = 3;
          compile (node, arguments, sections);
        } catch (GLib.Error e)
        {
          unowned var
          errors = (GLib.Error[]) & args [0];
          errors [0] = e;
          return;
        }
      }
    }

    public GLib.Bytes assemble (Ast.Node tree) throws GLib.Error
    {
      var binary = new Binary ();
      var arguments = new Arguments ();
      var code = new CodeSection (".code");
      var stack = new StackSection (".stack");
      var strtab = binary.strtab;

      arguments.count (tree);
      arguments.alloc (stack);

      /* begin assemble */

      uintptr args [5];
      args [1] = (uintptr) arguments;
      args [2] = (uintptr) code;
      args [3] = (uintptr) stack;
      args [4] = (uintptr) strtab;
      var cp = (Compiler) args;
          cp.traverse (tree);

      if (unlikely (args [0] != 0))
      {
        unowned var errors = (GLib.Error[]) & args [0];
        var error = (owned) errors [0];
        throw error;
      }

      {
        var reg = (uint8) code.pop ();
        var opcode = Opcode ();

        opcode.code = Code.RETURN;
        opcode.a = reg;
        code.put (opcode);
        stack.unalloc (reg);
      }

      arguments.unalloc (stack);

      code.check ();
      stack.check ();

      /* finish assemble */

      binary.put (code);
      binary.put (stack);
    return binary.finish ();
    }

    static construct
    {
      uint8 source [Abaco.Bytecode.SECTION_ALIGN];
      trash = new GLib.Bytes (source);
    }
  }
}
