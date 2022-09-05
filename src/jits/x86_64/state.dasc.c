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
#include <config.h>
#include <dasm_patch.h>
#include <dasm_proto.h>

static int
_jit_extern (dasm_State** ctx, gpointer addr, guint index, int type);
#undef DASM_EXTERN
#define DASM_EXTERN(ctx, addr, idx, type) \
(_jit_extern ((ctx), (addr), (idx), (type)))

#include <dasm_x86.h>
#include <x86_64/jit.h>
#include <x86_64/state.h>
#include <reg.h>

#if __DASC__
|.arch x64
|.section code, symbols, data
|.globals globl_
|.actionlist actions
|.globalnames globl_names
|.externnames extern_names
#else // !__DASC__
# define DASM_MAXSECTION (0)
# define globl_main (0)
# define globl__MAX (0)
# define actions (NULL)
extern const gchar** globl_names;
extern const gchar** extern_names;
# define MAP_ANONYMOUS 0
#endif // __DASC__

typedef struct _Record Record;

#define stacksz_r \
  ( \
    0 \
    + sizeof (gpointer) /* upslot */ \
  )

#if __DASC__
|.type Reg, Reg
|.type Addr, gpointer
|.define upslot, [rsp+(sizeof(gpointer)*0)]
#endif // __DASC__

#if __DASC__
|.macro slot, reg, n
| lea reg, Reg:rbp[n]
|.endmacro
|.macro slot_d, reg, index
| mov reg, index
| imul reg, (regsz)
| lea reg, [rbp+reg]
|.endmacro
|
|.define arg1, rdi
|.define arg2, rsi
|.define arg3, r8
#endif // __DASC__

#define ABACO_JITS_TYPE_X86_64_STATE (abaco_jits_x86_64_state_get_type ())
#define ABACO_JITS_X86_64_STATE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ABACO_JITS_TYPE_X86_64_STATE, AbacoJitsX8664State))
#define ABACO_IS_JITS_X86_64_STATE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ABACO_JITS_TYPE_X86_64_STATE))
typedef struct _AbacoJitsX8664State AbacoJitsX8664State;
#define ABACO_JITS_X86_64_STATE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ABACO_JITS_TYPE_X86_64_STATE, AbacoJitsX8664StateClass))
#define ABACO_IS_JITS_X86_64_STATE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ABACO_JITS_TYPE_X86_64_STATE))
#define ABACO_JITS_X86_64_STATE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ABACO_JITS_TYPE_X86_64_STATE, AbacoJitsX8664StateClass))
typedef struct _AbacoJitsX8664StateClass AbacoJitsX8664StateClass;
#define _g_hash_table_unref0(var) ((var == NULL) ? NULL : (var = (g_hash_table_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

struct _AbacoJitsX8664State
{
  AbacoJitState parent;

  /* private */
  AbacoJit* back;
  dasm_State* state;
  GHashTable* intern;
  GHashTable* pcs;
  gpointer labels [globl__MAX];
  gsize stacksz;
  guint lastpc;
};

struct _AbacoJitsX8664StateClass
{
  AbacoJitStateClass parent;
  GHashTable* externs;
};

G_DEFINE_FINAL_TYPE (AbacoJitsX8664State, abaco_jits_x86_64_state, ABACO_JIT_TYPE_STATE);

static int
_jit_extern (dasm_State** Dst, gpointer addr, guint index, int type)
{
  AbacoJitsX8664StateClass* klass;
  GHashTable* externs;
  const gchar* name;
  gpointer func;
  gintptr base;
  gintptr rel;
  GType gtype;
  int off;

  gtype = ABACO_JITS_TYPE_X86_64_STATE;
  klass = g_type_class_ref (gtype);
  externs = klass->externs;
  name = extern_names [index];
  func = g_hash_table_lookup (externs, name);
         g_assert (func != NULL);
  off = (type) ? (int) (func - addr - 4)
               : (int) ((guintptr) func);
          g_type_class_unref (klass);
return off;
}

G_STATIC_ASSERT (sizeof (gpointer) == 8);
#define Dst (&self->state)

static inline gint
_jit_newpc (AbacoJitsX8664State* self)
{
  guint pc = self->lastpc++;
  dasm_growpc (Dst, self->lastpc);
return pc;
}

static void
abaco_jits_x86_64_state_class_finalize (AbacoJitState* pself)
{
  AbacoJitsX8664State* self = (gpointer) pself;
  g_hash_table_remove_all (self->intern);
  _g_hash_table_unref0 (self->intern);
  g_hash_table_remove_all (self->pcs);
  _g_hash_table_unref0 (self->pcs);
  dasm_free (Dst);
ABACO_JIT_STATE_CLASS (abaco_jits_x86_64_state_parent_class)->finalize (pself);
}

static void
abaco_jits_x86_64_state_class_move (AbacoJitState* pself, guint index1, guint index2)
{
  AbacoJitsX8664State* self = (gpointer) pself;
  g_assert (self->stacksz > index1);
  g_assert (self->stacksz > index2);
#if __DASC__
  | slot arg1, (index1)
  | slot arg2, (index2)
  | call extern _jit_move
#endif // __DASC__
}

static inline void
dumpbuffer (AbacoJitsX8664State* self, gconstpointer buf, gsize length)
{
  gsize dwords = (length / 4);
  gsize i;

  for (i = 0; i < dwords; i++)
    {
#if __DASC__
      |.dword (*(guint32*) buf)
#endif // __DASC__
      buf += 4;
    }

  for (i = 0; i < (length % 4); i++)
    {
#if __DASC__
      |.byte (*(guint8*) buf)
#endif // __DASC__
      ++buf;
    }
}

static inline guint
_intern_constant_insert (AbacoJitsX8664State* self, const gchar* key, const gchar* value)
{
  gpointer ploc = NULL;
  gsize length = 0;
  guint loc = 0;

  loc = self->lastpc++;
  ploc = GUINT_TO_POINTER (loc);
  length = strlen (value);

  g_hash_table_insert (self->intern, g_strdup (key), ploc);
  dasm_growpc (Dst, self->lastpc);
#if __DASC__
  |.data
  |=>(loc):
#endif // __DASC__
  dumpbuffer (self, value, length + 1);
#if __DASC__
  |.code
#endif // __DASC__
return loc;
}

static inline guint
_intern_constant (AbacoJitsX8664State* self, const gchar* expr)
{
  const long bufsz = 512 - 2;
  gchar stat [bufsz + 2];
  gchar* val = (gchar*) expr;
  gchar* buf = NULL;
  gsize partial;
  guint loc;
  mpq_t q;

  do
  {
    switch (*val)
    {
    case 0:
      loc = _intern_constant_insert (self, expr, expr);
      return loc;
    case '.':
      mpq_init (q);
      _jit_load_dot (q, expr, val);

      partial = 3;
      partial += mpz_sizeinbase (mpq_numref (q), 10);
      partial += mpz_sizeinbase (mpq_denref (q), 10);

      if (partial > bufsz)
        buf = g_malloc (partial);
      else
        buf = & stat [0];

      buf = mpq_get_str (buf, 10, q);
      loc = _intern_constant_insert (self, expr, buf);

      if (buf != & stat [0])
        g_free (buf);
      mpq_clear (q);
      return loc;
    case '/':
      mpq_init (q);
      mpq_set_str (q, expr, 10);
      mpq_canonicalize (q);

      partial = 3;
      partial += mpz_sizeinbase (mpq_numref (q), 10);
      partial += mpz_sizeinbase (mpq_denref (q), 10);

      if (partial > bufsz)
        buf = g_malloc (partial);
      else
        buf = & stat [0];

      buf = mpq_get_str (buf, 10, q);
      loc = _intern_constant_insert (self, expr, buf);

      if (buf != & stat [0])
        g_free (buf);
      mpq_clear (q);
      return loc;
    }

    val = g_utf8_next_char (val);
  } while (val != NULL);
}

static void
abaco_jits_x86_64_state_class_load_constant (AbacoJitState* pself, guint index, const gchar* expr)
{
  AbacoJitsX8664State* self = (gpointer) pself;
  g_assert (self->stacksz > index);
  gpointer ploc = 0;
  guint loc = 0;
  gchar best;

  if (!g_hash_table_lookup_extended (self->intern, expr, NULL, &ploc))
    loc = _intern_constant (self, expr);
  else
    loc = GPOINTER_TO_UINT (ploc);

#if __DASC__
  | slot arg1, (index)
  | lea arg2, [=>(loc)]
  | call extern _jit_load
#endif // __DASC__
}

static void
abaco_jits_x86_64_state_class_load_function (AbacoJitState* pself, guint index, const gchar* expr)
{
  AbacoJitsX8664State* self = (gpointer) pself;
  AbacoJitRelationCompiler compiler = NULL;
  AbacoJitRelation* relation = NULL;
  gpointer lpc = NULL;
  guint pc = 0;

  g_assert (self->stacksz > index);

  relation =
  abaco_jit_get_relation (self->back, expr);
  if (G_UNLIKELY (relation == NULL))
  {
    g_error ("Unknown relation '%s'", expr);
    g_assert_not_reached ();
  }
  else
  {
    compiler = abaco_jit_relation_get_compiler (relation);
    if (G_UNLIKELY (compiler == NULL))
    {
      g_error ("Invalid relation '%s'", expr);
      g_assert_not_reached ();
    }
    else
    {
      lpc = compiler (pself, expr);
      pc = GPOINTER_TO_UINT (lpc);
#if __DASC__
      | slot arg1, (index)
      | call extern _jit_clean
      | lea rax, [=>(pc)]
      | slot rcx, (index)
      | mov byte Reg:rcx->type, (UCL_REG_TYPE_POINTER)
      | mov qword Reg:rcx->pointer, rax
#endif // __DASC__
    }
  }
}

static void
abaco_jits_x86_64_state_class_call (AbacoJitState* pself, guint index, guint first, guint count)
{
  AbacoJitsX8664State* self = (gpointer) pself;
  g_assert (self->stacksz > index);
  g_assert (self->stacksz > first);
  g_assert (self->stacksz > first + count - 1);

#if __DASC__
  | slot rax, (index)
  | cmp byte Reg:rax->type, (UCL_REG_TYPE_POINTER)
  | je >1
  | call extern _jit_error_call
  |1:
  | mov arg1, (index)
  | mov arg2, (first)
  | mov arg3, (count)
  | mov rax, Reg:rax->pointer
  | call rax
#endif // __DASC__
}

static void
abaco_jits_x86_64_state_class_ret (AbacoJitState* pself, guint index)
{
  AbacoJitsX8664State* self = (gpointer) pself;
  g_assert (self->stacksz > index);

#if __DASC__
  G_STATIC_ASSERT ((regsz / 4) > 0);
  G_STATIC_ASSERT ((regsz % 4) == 0);
  gsize dwords = (regsz / 4);
  gsize i;

  | mov rax, upslot
  | cmp rax, (index)
  | je >2
  | slot_d arg1, rax
  | call extern _jit_clean
  | mov rax, upslot
#ifndef G_OS_UNIX
  | push rsi
  | push rdi
#endif // G_OS_UNIX
  | slot_d rdi, rax
  | slot rsi, (index)
  | mov rcx, (dwords)
  |1:
  | movsd
  | mov dword [rsi-4], 0
  | dec rcx
  | jnz <1
  |2:
#ifndef G_OS_UNIX
  | pop rsi
  | pop rdi
#endif // G_OS_UNIX
#endif // __DASC__
}

static void
_closure_data_free (gpointer pdata, GClosure* pself)
{
  Closure* self = (Closure*) pself;
#ifdef G_OS_WINDOWS
  VirtualFree (self->block, self->blocksz, 0);
#else // !G_OS_WINDOWS
  munmap (self->block, self->blocksz);
#endif // G_OS_WINDOWS
}

static void
_closure_marshal (GClosure* closure,
                  GValue* result,
                  guint n_params,
                  const GValue* params,
                  gpointer invocation_hint G_GNUC_UNUSED,
                  gpointer marshal_data)
{
  Closure* cc = (Closure*) closure;
  g_return_if_fail (result != NULL);
  g_return_if_fail (cc->stacksz >= n_params);

  gpointer callback = NULL;
  const gchar* param = NULL;
  Reg stat [1024 / regsz];
  Reg* stack = NULL;
  guint i;

  callback = (marshal_data ? marshal_data : cc->main);
  if (cc->stacksz > G_N_ELEMENTS (stat))
    stack = g_malloc0 (cc->stacksz * regsz);
  else
  {
    stack = & stat[0];
#if HAVE_MEMSET
    memset (stack, 0, cc->stacksz * regsz);
#else // !HAVE_MEMSET
    static Reg __empty = {0};
    for (i = 0; i < cc->stacksz; i++)
      stack [i] = __empty;
#endif // HAVE_MEMSET
  }

  for (i = 0; i < n_params; i++)
  {
    param =
    g_value_get_string (& params[i]);
    _jit_load (& stack[i], param);
  }

#ifdef G_OS_UNIX
# define ARG1 "rdi"
# define ARG2 "rsi"
# define ARG3 "rcx"
#else // !G_OS_UNIX
# define ARG1 "rcx"
# define ARG2 "rdx"
# define ARG3 "r8"
#endif // G_OS_UNIX

  asm
  ("push %%rbp;"
   "movq %1, %%rbp;"
   "movq $0, %%" ARG1 ";"
   "movq $0, %%" ARG2 ";"
   "movq %2, %%" ARG3 ";"
   "call *%%rax;"
   "pop %%rbp;"
   :
   : "a" (callback),
     "g" (stack),
     "g" ((guintptr) n_params)
   : "%" ARG1,
     "%" ARG2,
     "%" ARG3);

#undef ARG1
#undef ARG2
#undef ARG3

  g_value_take_string (result, _jit_save (stack));
  ucl_reg_clears (stack, cc->stacksz);
  if (stack != & stat[0])
    g_free (stack);
}

static GClosure*
abaco_jits_x86_64_state_class_finish (AbacoJitState* pself)
{
  AbacoJitsX8664State* self = (gpointer) pself;
  GClosure* closure = NULL;
  Closure* cclosure = NULL;
  gpointer block = NULL;
  size_t blocksz = 0;

#if __DASC__
  | add rsp, (stacksz_r)
  | ret
#endif // __DASC__

  dasm_link (Dst, &blocksz);
#ifdef G_OS_WINDOWS
  block = VirtualAlloc (0, blocksz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else // !G_OS_WINDOWS
  block = mmap (0, blocksz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif // G_OS_WINDOWS

  dasm_encode (Dst, block);
#ifdef G_OS_WINDOWS
  G_STMT_START {
    DWORD dwOld;
    VirtualProtect (block, blocksz, PAGE_EXECUTE_READ, &dwOld);
  } G_STMT_END;
#else // !G_OS_WINDOWS
  mprotect (block, blocksz, PROT_READ | PROT_EXEC);
#endif // G_OS_WINDOWS

  closure = g_closure_new_simple (sizeof (Closure), NULL);
  cclosure = ((Closure*) closure);
  cclosure->main = self->labels [globl_main];
  cclosure->stacksz = self->stacksz;
  cclosure->block = block;
  cclosure->blocksz = blocksz;

  if (closure->floating)
  {
    g_closure_ref (closure);
    g_closure_sink (closure);
  }

  g_closure_add_finalize_notifier (closure, NULL, _closure_data_free);
  g_closure_set_marshal (closure, _closure_marshal);
return closure;
}

static void
_jit_error_call ()
{
  g_error ("Attempt to call a non-function value");
  g_assert_not_reached ();
}

static void
_jit_error_nargs (guint got, guint expected)
{
  g_error
  ("Too %s arguments, expected %i, got %i",
   (got > expected) ? "many" : "few",
   expected, got);
  g_assert_not_reached ();
}

static void
abaco_jits_x86_64_state_class_init (AbacoJitsX8664StateClass* klass)
{
  AbacoJitStateClass* oclass = ABACO_JIT_STATE_CLASS (klass);

  oclass->finalize = abaco_jits_x86_64_state_class_finalize;
  oclass->move = abaco_jits_x86_64_state_class_move;
  oclass->load_constant = abaco_jits_x86_64_state_class_load_constant;
  oclass->load_function = abaco_jits_x86_64_state_class_load_function;
  oclass->call = abaco_jits_x86_64_state_class_call;
  oclass->ret = abaco_jits_x86_64_state_class_ret;
  oclass->finish = abaco_jits_x86_64_state_class_finish;

  klass->externs = g_hash_table_new (g_str_hash, g_str_equal);
#define extern(var) G_STRINGIFY (var),var
  g_hash_table_insert (klass->externs, extern (_jit_error_call));
  g_hash_table_insert (klass->externs, extern (_jit_error_nargs));
  g_hash_table_insert (klass->externs, extern (_jit_clean));
  g_hash_table_insert (klass->externs, extern (_jit_move));
  g_hash_table_insert (klass->externs, extern (_jit_load));
  g_hash_table_insert (klass->externs, extern (_jit_save));
#undef extern
}

static void
abaco_jits_x86_64_state_init (AbacoJitsX8664State* self)
{
  dasm_init (Dst, DASM_MAXSECTION);
  dasm_setupglobal (Dst, self->labels, globl__MAX);
  dasm_setup (Dst, actions);
  dasm_growpc (Dst, 0);

  self->intern = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  self->pcs = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  self->lastpc = 0;

#if __DASC__
  |.code
  |->main:
  | sub rsp, (stacksz_r)
# ifdef G_OS_UNIX
  | mov upslot, rdi
# else // !G_OS_UNIX
  | mov upslot, rcx
# endif // G_OS_UNIX
#endif // __DASC__
}

/* internal API */

AbacoJitState*
abaco_jits_x86_64_state_new (AbacoJit* back, GBytes* code)
{
  AbacoJitsX8664State* self;
  g_return_val_if_fail (ABACO_IS_JIT (back), NULL);
  g_return_val_if_fail (code != NULL, NULL);
  self = (gpointer) abaco_jit_state_construct (ABACO_JITS_TYPE_X86_64_STATE, code);
  self->stacksz = abaco_jit_state_stack_section_get_size (self->parent.stack);
  self->back = back;
return (gpointer) self;
}

gboolean
abaco_jits_x86_64_state_getpc (gpointer pself, guint* out_pc, const gchar* key)
{
  g_return_val_if_fail (out_pc != NULL, FALSE);
  g_return_val_if_fail (key != NULL, FALSE);
  AbacoJitsX8664State* self = pself;
  gpointer lpc = NULL;
  guint pc = 0;

  if (g_hash_table_lookup_extended (self->pcs, key, NULL, &lpc))
  {
    *out_pc = GPOINTER_TO_UINT (lpc);
    return TRUE;
  }
  else
  {
    pc = _jit_newpc (self);
    lpc = GUINT_TO_POINTER (pc);
    *out_pc = pc;

    g_hash_table_insert (self->pcs, g_strdup (key), lpc);
    return FALSE;
  }
}

gpointer
abaco_jits_x86_64_accum_wrap (gpointer pself, const gchar* name, gboolean invert, AccumWrap wrap)
{
  AbacoJitsX8664State* self = pself;
  guint pc = 0;

  if (!abaco_jits_x86_64_state_getpc (self, &pc, name))
  {
    const gsize space = sizeof (gpointer) * 4;
    const guintptr func = GUINT64_TO_LE ((guintptr) wrap);
#if __DASC__
    |.symbols
    |=>(pc):
    | sub rsp, (space)
    | mov Addr:rsp [0], r12
    | mov Addr:rsp [1], r13
    | mov Addr:rsp [2], r14
    | mov Addr:rsp [3], rbx
    | mov rbx, qword [>1]
    | slot_d arg1, arg1
    | mov r12, arg1
    | slot_d r13, arg2
    | mov r14, arg2
    | add r14, arg3
    | slot_d r14, r14
    | call extern _jit_clean
    |2:
    | mov arg1, r12
    | mov arg2, r13
    | call rbx
    | add r13, (regsz)
    | cmp r13, r14
    | jne <2
    | mov r12, Addr:rsp [0]
    | mov r13, Addr:rsp [1]
    | mov r14, Addr:rsp [2]
    | mov rbx, Addr:rsp [3]
    | add rsp, (space)
    | ret
    |1:
    |.dword (func & G_MAXUINT32)
    |.dword (func >> 32)
    |.code
#endif // __DASC__
  }
return GUINT_TO_POINTER (pc);
}
