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
  public abstract class Jit : GLib.Object
  {
    private HashTable<Relation, bool> relations;
    private Abaco.Assembler assembler;
    private Abaco.Rules rules;

    /* type API */

    public abstract class State
    {
      protected StackSection stack;
      protected StrtabSection strtab;

      /* type API */

      [SimpleType]
      protected struct Pointer : uint64
      {
      }

      protected class Section
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

      protected class StackSection : Section
      {
        /* public API */

        public uint size
        {
          get
          {
            return head.size;
          }
        }

        /* Constructors */

        public StackSection (GLib.Bytes code)
        {
          base (Bytecode.SectionType.STACK, code);
        }
      }

      protected class StrtabSection : Section
      {
        public string[] strings { get; private set; }
        public uint size { get { return strings.length; } }
    
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

      /* public API */

      public unowned string? peek_string (uint index)
      {
        if (index > strtab.size)
          error ("Invalid string table index");
      return strtab.strings [index];
      }

      /* abstract API */

      public abstract void load_constant (uint index, string expr);
      public abstract void load_function (uint index, string expr);
      public abstract void call (uint index, uint first, uint args);
      public abstract void ret (uint index);
      public abstract Closure? finish ();

      /* Constructors */

      protected State (GLib.Bytes code)
      {
        this.stack = new StackSection (code);
        this.strtab = new StrtabSection (code);
      }
    }

    [Compact (opaque = true)]
    public class Relation
    {
      public Compiler compiler { get; private owned set; }
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

      /* type API */

      [CCode (has_target = false)]
      public delegate void Compiler (State state, uint index, uint first, uint count);

      /* static API */

      public static uint hash (Relation self)
      {
        var hash = (uint) 0;
        hash ^= GLib.str_hash ((string) self.name);
      return hash;
      }

      public static bool equal (Relation self1, Relation self2)
      {
        return
          self1.name == self2.name;
      }

      /* public API */

      public bool matches (string expr)
      {
        return regex.match (expr, 0, null);
      }

      /* Constructors */

      public Relation.with_name (string name, owned Compiler compiler)
      {
        this ((owned) compiler);
        this.name = name;
      }

      public Relation.with_expression (string expr, owned Compiler compiler)
      {
        this ((owned) compiler);
        this.expr = expr;
      }

      public Relation (owned Compiler compiler)
      {
        this.compiler = (owned) compiler;
      }
    }

    /* private API */

    private Closure? compile (GLib.Bytes bytecode)
    {
      var state = new_state (bytecode);
      unowned Bytecode.Section* section;
      unowned var data = bytecode.get_data ();
      unowned var ptr = (uint8*) data;
      unowned var top = ptr + data.length;

      while (ptr < top)
      {
        section = (Bytecode.Section*) ptr;
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
            case Bytecode.Code.LOADK:
              {
                unowned var dst = opcode.a;
                unowned var srci = opcode.bx;
                unowned var src = state.peek_string (srci);
                state.load_constant (dst, src);
              }
              break;
            case Bytecode.Code.LOADF:
              {
                unowned var dst = opcode.a;
                unowned var srci = opcode.bx;
                unowned var src = state.peek_string (srci);
                state.load_function (dst, src);
              }
              break;
            case Bytecode.Code.CALL:
              {
                unowned var dst = opcode.a;
                unowned var src = opcode.b;
                unowned var cnt = opcode.c;
                state.call (dst, src, cnt);
              }
              break;
            case Bytecode.Code.RETURN:
              {
                unowned var src = opcode.a;
                state.ret (src);
              }
              break;
            default:
              error ("Invalid binary: invalid opcode %s", opcode.code.to_string ());
            }
          }
        }
      }
    return state.finish ();
    }

    /* abstract API */

    protected abstract State new_state (GLib.Bytes code);

    /* protected API */

    protected unowned Relation? get_relation (string expr)
    {
      var iter = HashTableIter<Relation, bool> (relations);
      unowned Relation relation;

      while (iter.next (out relation, null))
      if (relation.matches (expr))
        return relation;
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
        relations.insert ((owned) relation, true);
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
        relations.insert ((owned) relation, true);
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
      unowned HashFunc<Relation> hash = Relation.hash;
      unowned EqualFunc<Relation> equal = Relation.equal;
      this.relations = new HashTable<Relation, bool> (hash, equal);
      this.assembler = new Abaco.Assembler ();
      this.rules = new Abaco.Rules ();
    }

    public new static Jit @new (GLib.Type type)
    {
      if (!type.is_a (typeof (Jit)))
        error ("Invalid JIT");
    return (Jit) GLib.Object.@new (type);
    }

    protected Jit ()
    {
      base ();
    }
  }
}
