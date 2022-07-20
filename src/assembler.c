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
#include <assembler.h>
#include <astate.h>
#include <gio/gio.h>

#define ABACO_RULES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), ABACO_TYPE_RULES, AbacoAssemblerClass))
#define ABACO_IS_RULES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ABACO_TYPE_RULES))
#define ABACO_RULES_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), ABACO_TYPE_RULES, AbacoAssemblerClass))
typedef struct _AbacoAssemblerClass AbacoAssemblerClass;
#define _g_object_unref0(var) (var = (g_object_unref (var), NULL))
#define _g_bytes_unref0(var) (var = (g_bytes_unref (var), NULL))
#define _g_error_free0(var) (var = (g_error_free (var), NULL))
#define _g_free0(var) (var = (g_free (var), NULL))

struct _AbacoAssembler
{
  GObject parent_instance;

  /*<private>*/
};

struct _AbacoAssemblerClass
{
  GObjectClass parent_instance;
};

G_DEFINE_FINAL_TYPE (AbacoAssembler, abaco_assembler, G_TYPE_OBJECT);

static void
abaco_assembler_class_init (AbacoAssemblerClass* klass)
{
}

static void
abaco_assembler_init (AbacoAssembler* self)
{
}

VALA_EXTERN AbacoAssembler*
abaco_assembler_new ()
{
  return
  g_object_new
  (ABACO_TYPE_ASSEMBLER,
   NULL);
}

/*
 * Assembler
 *
 */

typedef struct
{
  AState parent;
  guint* vars;
  GQueue callstack;
} ATraverse;

#define s (&((t)->parent))
#define lookup(table,key,loc) \
  (gboolean) \
  (G_GNUC_EXTENSION ({ \
    gboolean has = \
    g_hash_table_lookup_extended \
    ((table), \
     (gchar*) (key), \
     NULL, \
     (gpointer*) (loc)); \
    has; \
  }))

static void
_node_n_args (AbacoAstNode* node, gpointer* args)
{
  AbacoAstSymbolKind kind;
  ATraverse* t = args [0];
  guint* pcount = args [1];
  guint count;

  kind =
  abaco_ast_node_get_kind (node);
  abaco_ast_node_children_foreach
  (node,
   (AbacoAstForeach)
   _node_n_args,
   args);

  if (kind == ABACO_AST_SYMBOL_KIND_VARIABLE)
  {
    const gchar* key;
    gpointer val;
    guint reg;

    key = abaco_ast_node_get_symbol (node);
    ++count;

    if (!lookup (s->inputs, key, &val))
    {
      reg = pcount [0]++;
      val = GUINT_TO_POINTER (reg);
      g_hash_table_insert (s->inputs, (gchar*) key, val);
    }
  }
}

static inline guint
_atraverse_n_args (ATraverse* t, AbacoAstNode* node)
{
  guint count = 0;
  gpointer args [2] = { t, &count };

  abaco_ast_node_children_foreach
  (node,
   (AbacoAstForeach)
   _node_n_args,
   (gpointer)
   args);
return count;
}

static inline guint
_atraverse_intern (ATraverse* t, const gchar* key)
{
  gpointer val;
  guint idx;

  if (!lookup (s->locals, key, &val))
    {
      idx = s->strings->len;
      val = GUINT_TO_POINTER (idx);
      g_hash_table_insert (s->locals, (gchar*) key, val);
      g_ptr_array_add (s->strings, (gchar*) key);
    }
return GPOINTER_TO_UINT (val);
}

static inline guint
_atraverse_arg (ATraverse* t, const gchar* key)
{
  gpointer val;
  guint idx;

  if (!lookup (s->inputs, key, &val))
  {
    g_critical ("Not captured variable");
    g_assert_not_reached ();
  }
  else
  {
    idx = GPOINTER_TO_UINT (val);
    return t->vars [idx];
  }
}

static inline void
_atraverse_push (ATraverse* t, guint reg)
{
  gpointer ptr = GUINT_TO_POINTER (reg);
  g_queue_push_head (&t->callstack, ptr);
}

static inline guint
_atraverse_pop (ATraverse* t)
{
  guint nth = t->callstack.length;
  gpointer ptr = g_queue_pop_head (&t->callstack);
  guint reg = GPOINTER_TO_UINT (ptr);

  if (nth == 0)
  {
    g_critical ("Empty callstack");
    g_assert_not_reached ();
  }
return reg;
}

static inline guint*
_atraverse_prepcall (ATraverse* t, guint nth)
{
  guint* regs = g_new (guint, nth);
  gboolean aligned = TRUE;
  gint i;

  for (i = nth - 1; i >= 0; --i)
  {
    regs [i] = _atraverse_pop (t);
    if (i != (nth - 1) && ((regs [i] + 1) != regs [i + 1]))
      aligned = FALSE;
  }

  if (!aligned)
  {
    BOpcode opcode = {0};
    guint* dump = regs;
    regs = g_new (guint, nth);

    _astate_registry_alloca (s, regs, nth);

    for (i = 0; i < nth; i++)
    {
      opcode.code = B_OPCODE_MOVE;
      opcode.abc.a = regs [i];
      opcode.abc.b = dump [i];

      _astate_put (s, opcode);
      _astate_registry_free (s, dump [i]);
    }

    _g_free0 (dump);
  }
return regs;
}

static void
_node_compiler (AbacoAstNode* node, ATraverse* t)
{
  AbacoAstSymbolKind kind;
  const gchar* symbol;
  GError* tmp_err = NULL;

  /* descend into children */
  abaco_ast_node_children_foreach
  (node,
   (AbacoAstForeach)
   _node_compiler,
   (gpointer) t);
  _astate_propagate (s, &tmp_err);

  /* chain-up errors */
  if (G_UNLIKELY (tmp_err != NULL))
  {
    _astate_throw (s, tmp_err);
    return;
  }

  kind = abaco_ast_node_get_kind (node);
  symbol = abaco_ast_node_get_symbol (node);

  BOpcode opcode = {0};
  gpointer pidx = NULL;
  guint idx;

  switch (kind)
  {
  case ABACO_AST_SYMBOL_KIND_CONSTANT:
    idx = _astate_registry_alloc (s);
    opcode.code = B_OPCODE_LOADK;
    opcode.abx.a = idx;
    opcode.abx.bx = _atraverse_intern (t, symbol);
    _atraverse_push (t, idx);
    _astate_put (s, opcode);
    if (G_UNLIKELY (tmp_err != NULL))
    {
      _astate_throw (s, tmp_err);
      return;
    }
    break;
  case ABACO_AST_SYMBOL_KIND_VARIABLE:
    idx = _astate_registry_alloc (s);
    opcode.code = B_OPCODE_MOVE;
    opcode.abc.a = idx;
    opcode.abc.b = _atraverse_arg (t, symbol);
    _atraverse_push (t, idx);
    _astate_put (s, opcode);
    if (G_UNLIKELY (tmp_err != NULL))
    {
      _astate_throw (s, tmp_err);
      return;
    }
    break;
  case ABACO_AST_SYMBOL_KIND_FUNCTION:
    {
      guint i, nth = abaco_ast_node_n_children (node);
      guint* regs = _atraverse_prepcall (t, nth);
      idx = _astate_registry_alloc (s);

      /* load function */
      opcode.code = B_OPCODE_LOADF;
      opcode.abx.a = idx;
      opcode.abx.bx = _atraverse_intern (t, symbol);
      _astate_put (s, opcode);
      if (G_UNLIKELY (tmp_err != NULL))
      {
        _astate_throw (s, tmp_err);
        return;
      }

      /* make call */
      opcode.code = B_OPCODE_CALL;
      opcode.abc.a = idx;
      opcode.abc.b = (nth > 0) ? regs [0] : 0;
      opcode.abc.c = nth;
      _astate_put (s, opcode);
      if (G_UNLIKELY (tmp_err != NULL))
      {
        _astate_throw (s, tmp_err);
        return;
      }

      /* push result */
      _atraverse_push (t, idx);

      for (i = 0; i < nth; i++)
        _astate_registry_free (s, regs [i]);
      _g_free0 (regs);
    }
    break;
  }
}

#undef lookup

GBytes*
abaco_assembler_assemble (AbacoAssembler* assembler, AbacoAstNode* tree, GError** error)
{
  g_return_val_if_fail (ABACO_IS_ASSEMBLER (assembler), NULL);
  AbacoAssembler* self = (assembler);
  ATraverse traverse = {0};
  ATraverse* t = &traverse;
  GError* tmp_err = NULL;
  GBytes* bytes = NULL;
  goffset offset = 0;

  _astate_begin (s);
  _astate_propagate (s, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _astate_finish (s);
      return NULL;
    }

  g_queue_init (&t->callstack);

  /* begin assemble */

  BSection code1 =
  {
    .name = 0,
    .type = B_SECTION_TYPE_BITS,
    .flags = B_SECTION_CODE,
    .size = 0,
  };

  _astate_begin_section (s, ".code", &code1);
  _astate_propagate (s, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _astate_finish (s);
      return NULL;
    }

  guint i, n_vars = _atraverse_n_args (t, tree);
  guint* vars = g_new (guint, n_vars);
  for (i = 0; i < n_vars; i++)
  {
    guint reg;
    reg = _astate_registry_alloc (s);
    vars [i] = reg;
  }

  t->vars = vars;

  _node_compiler (tree, t);
  _astate_propagate (s, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _astate_finish (s);
      return NULL;
    }

  for (i = 0; i < n_vars; i++)
  {
    guint reg;
    reg = vars [i];
    _astate_registry_free (s, reg);
  }

  t->vars = NULL;
  _g_free0 (vars);

  G_STMT_START
  {
    guint reg;
    reg = _atraverse_pop (t);
    BOpcode opcode = {0};
    opcode.code = B_OPCODE_RETURN;
    opcode.abc.a = reg;

    _astate_put (s, opcode);
    _astate_registry_free (s, reg);
    if (G_UNLIKELY (tmp_err != NULL))
    {
      _astate_throw (s, tmp_err);
      return NULL;
    }
  }
  G_STMT_END;

  /* write registry table */

  BSection stack1 =
  {
    .name = 0,
    .type = B_SECTION_TYPE_STACK,
    .flags = B_SECTION_BSS,
    .size = s->n_regs,
  };

  _astate_switch_section (s, ".stack", &stack1);
  _astate_propagate (s, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _astate_finish (s);
      return NULL;
    }

  /* write string table */

  BSection strtab1 =
  {
    .name = 0,
    .type = B_SECTION_TYPE_STRTAB,
    .flags = B_SECTION_DATA,
    .size = 0,
  };

  _astate_switch_section (s, ".strtab", &strtab1);
  _astate_propagate (s, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _astate_finish (s);
      return NULL;
    }

  G_STMT_START
  {
    guint i, len = (guint) s->strings->len;
    gchar** ptrs = (gchar**) s->strings->pdata;
    for (i = 0; i < len; i++)
    {
      gchar* str = ptrs [i];
      gsize length = strlen (str) + 1;

      _astate_write (s, str, length);
      _astate_propagate (s, &tmp_err);
      if (G_UNLIKELY (tmp_err != NULL))
      {
        g_propagate_error (error, tmp_err);
        _astate_finish (s);
        return NULL;
      }
    }
  }
  G_STMT_END;

  /* finish pending sections */

  if (g_queue_get_length (&t->callstack) > 0)
    g_warning ("Leftover registry on call stack");
  g_queue_clear (&t->callstack);

  _astate_finish_section (s);
  _astate_propagate (s, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _astate_finish (s);
      return NULL;
    }

  /* close stream */

  g_output_stream_close (s->stream, NULL, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _astate_finish (s);
      return NULL;
    }

  bytes =
  g_memory_output_stream_steal_as_bytes ((gpointer) s->stream);
  _astate_finish (s);
return bytes;
}
