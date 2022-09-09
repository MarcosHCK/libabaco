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
#include <libabaco.h>
#include <libabaco_mp.h>
#include <bytecode.h>
#include <glib.h>

const gchar* output = NULL;
gboolean benchmark = FALSE;
gboolean printtree = FALSE;
gboolean printcode = FALSE;

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_bytes_unref0(var) ((var == NULL) ? NULL : (var = (g_bytes_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

const gchar* symbolkind[] =
{
  "ABACO_AST_SYMBOL_KIND_CONSTANT",
	"ABACO_AST_SYMBOL_KIND_VARIABLE",
	"ABACO_AST_SYMBOL_KIND_FUNCTION",
};

static void
print_tree (AbacoAstNode* node, int depth)
{
  gchar* pre = g_strnfill (depth * 2, ' ');
  const gchar* symbol = abaco_ast_node_get_symbol (node);
  AbacoAstSymbolKind kind = abaco_ast_node_get_kind (node);

  g_print ("%s-%s : %s\r\n", pre, symbol, symbolkind [(gint) kind]);

  abaco_ast_node_children_foreach
  (node,
   (AbacoAstForeach) print_tree,
   GINT_TO_POINTER (depth + 1));
}

static void
print_code (GBytes* bytes)
{
  const gsize linew = 16;
  const gchar set[] = "0123456789abcdef";

  gsize i, length;
  const guint8* data = g_bytes_get_data (bytes, &length);
  GString* buf = g_string_sized_new (linew * 3);

  do
  {
    gsize chunk = (length > linew) ? linew : length;

    for (i = 0; i < chunk; i++)
    {
      guint8 c = data [i];
      if (i > 0)
      g_string_append_c (buf, ' ');
      g_string_append_c (buf, set [c >> 4]);
      g_string_append_c (buf, set [c & 0xf]);
    }

    gsize spaces = linew * 3 + 2;
    for (i = buf->len; i < spaces; i++)
      g_string_append_c (buf, ' ');

    for (i = 0; i < chunk; i++)
    {
      guint8 c = data [i];
      if (g_ascii_iscntrl (c) ||
        !g_utf8_validate_len (&c, 1, NULL))
        g_string_append_c (buf, '.');
      else
        g_string_append_c (buf, c);
    }

    g_print ("%s\r\n", buf->str);
    g_string_truncate (buf, 0);

    data += chunk;
    length -= chunk;
  }
  while (length > 0);
  g_string_free (buf, TRUE);
}

static const BSection*
get_section_by_type (GBytes* bytes, BSectionType type)
{
  gsize i, length;
  const guint8* data = g_bytes_get_data (bytes, &length);
  const guint8* top = data + length;
  const BSection* section = NULL;

  while (data < top)
  {
    section = (BSection*) data;
    if (section->flags & B_SECTION_VIRTUAL)
      data += sizeof (BSection);
    else
    {
      gsize size = section->size;
      gsize miss = size % B_SECTION_ALIGN;
      if (miss > 0)
        data += size + (B_SECTION_ALIGN - miss);
      else
        data += size;
    }

    if (section->type == type)
      return section;
  }
return NULL;
}

static const gchar*
get_strtab (GBytes* bytes)
{
  const BSection* strtab = NULL;
  BSectionType type = B_SECTION_TYPE_STRTAB;

  strtab = get_section_by_type (bytes, type);
return (const gchar*) ( & (strtab [1]));
}

static const gchar*
get_strtab_n (const gchar* strtab, guint idx)
{
  guint i;
  for (i = 0; i < idx; i++)
    strtab += strlen (strtab) + 1;
return strtab;
}

static void
disassemble (GBytes* bytes)
{
  gsize i, length;
  const guint8* data = g_bytes_get_data (bytes, &length);
  const guint8* top = data + length;
  const BSection* section = NULL;
  const BOpcode* opcode = NULL;

  const gchar* codes [] =
  {
    "NOP",
    "MOVE",
    "LOADK",
    "LOADF",
    "CALL",
    "RETURN",
  };

  G_STATIC_ASSERT (G_N_ELEMENTS (codes) == B_OPCODE_MAXOPCODE);
  GString* buf = g_string_sized_new (128);
  const gchar* strtab = get_strtab (bytes);
  if (strtab == NULL)
  {
    g_critical ("Missing string table");
    g_assert_not_reached ();
  }

  while (data < top)
  {
    section = (BSection*) data;
    opcode = (BOpcode*) (data + sizeof (BSection));

    if (section->flags & B_SECTION_VIRTUAL)
      data += sizeof (BSection);
    else
    {
      gsize size = section->size;
      gsize miss = size % B_SECTION_ALIGN;
      if (miss > 0)
        data += size + (B_SECTION_ALIGN - miss);
      else
        data += size;
    }

    g_print
    ("\r\n"
     "section\r\n"
     "name: %s\r\n"
     "size: %i\r\n"
     "type: %i\r\n"
     "flags: %i\r\n",

     get_strtab_n (strtab, section->name),
     (guint) section->size,
     (guint) section->type,
     (guint) section->flags);

    if (section->type == B_SECTION_TYPE_BITS
      && section->flags == B_SECTION_CODE)
    while (opcode < (BOpcode*) data)
    {
      if (opcode->code > G_N_ELEMENTS (codes))
      {
        g_critical ("unknown opcode %i\r\n", opcode->code);
        g_assert_not_reached ();
      }

      g_string_append (buf, "  ");
      g_string_append (buf, codes [opcode->code]);
      switch (opcode->code)
      {
      case B_OPCODE_MOVE:
        g_string_append_printf (buf, " %u %u", (guint) opcode->abc.a, (guint) opcode->abc.b);
        break;
      case B_OPCODE_LOADK:
        g_string_append_printf (buf, " %u %u", (guint) opcode->abx.a, (guint) opcode->abx.bx);
        break;
      case B_OPCODE_LOADF:
        g_string_append_printf (buf, " %u %u", (guint) opcode->abx.a, (guint) opcode->abx.bx);
        break;
      case B_OPCODE_CALL:
        g_string_append_printf (buf, " %u %u %u", (guint) opcode->abc.a, (guint) opcode->abc.b, (guint) opcode->abc.c);
        break;
      case B_OPCODE_RETURN:
        g_string_append_printf (buf, " %u", (guint) opcode->abc.a);
        break;
      }

      g_print ("%s\r\n", buf->str);
      g_string_truncate (buf, 0);
      ++opcode;
    }
    else
    if (section->type == B_SECTION_TYPE_NOTES)
    {
      gconstpointer ptr = NULL;
      gconstpointer top = NULL;
      BNote* note = NULL;
      gsize length = 0;
      guint i = 0;

      ptr = sizeof (BSection) + (gpointer) section;
      top = ptr + section->size - sizeof (BSection);

      while (ptr < top)
      {
        note = (BNote*) ptr;
        g_print
        ("  '%s' = '%s'\r\n",
         get_strtab_n (strtab, note->key),
         get_strtab_n (strtab, note->value));
        ptr += sizeof (BNote);
      }
    }
    else
    if (section->type == B_SECTION_TYPE_STRTAB)
    {
      gconstpointer ptr = NULL;
      gconstpointer top = NULL;
      gsize length = 0;
      guint i = 0;

      ptr = sizeof (BSection) + (gpointer) section;
      top = ptr + section->size - sizeof (BSection);

      while (ptr < top)
      {
        g_print ("  %i: '%s'\r\n", i++, (gchar*) ptr);
        ptr += strlen ((gchar*) ptr) + 1;
      }
    }
  }

  g_string_free (buf, TRUE);
}

static gdouble*
get_stack (GBytes* bytes)
{
  const BSection* stack = NULL;
  BSectionType type = B_SECTION_TYPE_STRTAB;

  stack = get_section_by_type (bytes, type);
return g_new0 (gdouble, stack->size);
}

static gdouble
do_execute (GBytes* bytes)
{
  gsize i, length;
  gdouble result = -1;
  const guint8* data = g_bytes_get_data (bytes, &length);
  const guint8* top = data + length;
  const BSection* section = NULL;
  const BOpcode* opcode = NULL;

  const gchar* strtab = get_strtab (bytes);
  if (strtab == NULL)
  {
    g_critical ("Missing string table section");
    g_assert_not_reached ();
  }

  gdouble* stack = get_stack (bytes);
  if (stack == NULL)
  {
    g_critical ("Missing stack section");
    g_assert_not_reached ();
  }

  while (data < top)
  {
    section = (BSection*) data;
    opcode = (BOpcode*) (data + sizeof (BSection));

    if (section->flags & B_SECTION_VIRTUAL)
      data += sizeof (BSection);
    else
    {
      gsize size = section->size;
      gsize miss = size % B_SECTION_ALIGN;
      if (miss > 0)
        data += size + (B_SECTION_ALIGN - miss);
      else
        data += size;
    }

    if (section->type == B_SECTION_TYPE_BITS
      && section->flags == B_SECTION_CODE)
    while (opcode < (BOpcode*) data)
    {
      switch (opcode->code)
      {
      case B_OPCODE_MOVE:
        stack [opcode->abc.a] = stack [opcode->abc.b];
        break;
      case B_OPCODE_LOADK:
        stack [opcode->abx.a] = g_strtod (get_strtab_n (strtab, opcode->abx.bx), NULL);
        break;
      case B_OPCODE_LOADF:
        *((gpointer*) & stack [opcode->abx.a]) = (gchar*) get_strtab_n (strtab, opcode->abx.bx);
        break;
      case B_OPCODE_CALL:
        {
          guint i, opn = opcode->abc.c;
          gdouble first;
          gchar* op;

          if (opn > 1)
          {
            first = stack [opcode->abc.b];
            op = *((gpointer*) & stack [opcode->abx.a]);

            switch (op [0])
            {
            case '+':
              for (i = 1; i < opn; i++)
                first += stack [opcode->abc.b + i];
              break;
            case '-':
              for (i = 1; i < opn; i++)
                first -= stack [opcode->abc.b + i];
              break;
            case '*':
              for (i = 1; i < opn; i++)
                first *= stack [opcode->abc.b + i];
              break;
            case '/':
              for (i = 1; i < opn; i++)
                first /= stack [opcode->abc.b + i];
              break;

            default:
              g_error ("Unknown function '%s'", op [0]);
              g_assert_not_reached ();
              break;
            }
          }

          stack [opcode->abc.a] = first;
        }
        break;
      case B_OPCODE_RETURN:
        result = stack [opcode->abc.a];
        break;
      }

      ++opcode;
    }
  }

  g_free (stack);
return result;
}

static inline void
do_report (GBytes* code, const gchar* expr)
{
  GString* buffer = NULL;
  GError* tmp_err = NULL;

  if (output != NULL)
  {
    gconstpointer ptr = NULL;
    gsize length = 0;

    ptr = g_bytes_get_data (code, &length);
    g_file_set_contents (output, ptr, length, &tmp_err);
    if (G_UNLIKELY (tmp_err != NULL))
    {
      g_critical
      ("(%s): %s: %i: %s",
       G_STRLOC,
       g_quark_to_string
       (tmp_err->domain),
       tmp_err->code,
       tmp_err->message);
      g_error_free (tmp_err);
      g_assert_not_reached ();
    }
  }

  if (!benchmark)
  {
    g_print ("> '%s' = '%f'\r\n", code, do_execute (code));
  }
  else
  {
    const gdouble spt = (gdouble) CLOCKS_PER_SEC;
    const gdouble mpt = spt / (gdouble) 1000;
    const int tries = 100000;
    const int reps = 10;
    gdouble partial = 0;
    gdouble result = 0;
    gdouble last = 0;
    clock_t src, dst;
    int i, j;

    buffer =
    g_string_sized_new (128);
    g_print ("trying with %i cycles, %i times\r\n", tries, reps);

    for (i = 0; i < reps; i++)
    {
      for (j = 0; j < tries; j++)
      {
        src = clock ();
        result = do_execute (code);
        dst = clock ();

        if (j == 0)
        {
          partial = (gdouble) (dst - src);
          last = result;
        }
        else
        {
          if (result != last)
            g_error ("Results differs");
          partial += (gdouble) (dst - src);
        }
      }

      partial /= (reps * tries);

      g_string_append_printf (buffer, "> '%s'", expr);
      g_string_append_printf (buffer, " = '%f'", last);
      g_string_append_printf (buffer, " (took ");
      g_string_append_printf (buffer, "%lf ticks, ", partial);
      g_string_append_printf (buffer, "%lf micros, ", partial / mpt);
      g_string_append_printf (buffer, "%lf seconds)", partial / spt);
      g_print ("%.*s\r\n", buffer->len, buffer->str);
      g_string_truncate (buffer, 0);
    }

    g_string_free (buffer, TRUE);
  }
}

static inline GBytes*
do_compile (AbacoRules* rules, AbacoAssembler* assembler, GBytes* expr)
{
  AbacoAstNode* tree = NULL;
  const gchar* expr_ = NULL;
  GError* tmp_err = NULL;
  GBytes* code = NULL;
  gsize length = 0;

  expr_ = g_bytes_get_data (expr, &length);
  tree = abaco_rules_parse (rules, expr_, length, &tmp_err);

  if (G_UNLIKELY (tmp_err != NULL))
  {
    g_critical
    ("(%s): %s: %i: %s",
     G_STRLOC,
     g_quark_to_string
     (tmp_err->domain),
     tmp_err->code,
     tmp_err->message);
    _g_error_free0 (tmp_err);
    g_assert_not_reached ();
  }
  else
  {
    if (printtree)
    {
      print_tree (tree, 0);
    }

    code =
    abaco_assembler_assemble (assembler, tree, &tmp_err);
    abaco_ast_node_unref (tree);

    if (G_UNLIKELY (tmp_err != NULL))
    {
      g_critical
      ("(%s): %s: %i: %s",
      G_STRLOC,
      g_quark_to_string
      (tmp_err->domain),
      tmp_err->code,
      tmp_err->message);
      _g_bytes_unref0 (code);
      _g_error_free0 (tmp_err);
      g_assert_not_reached ();
    }
    else
    if (printcode)
    {
      print_code (code);
    }
  }
return code;
}

int
main (int argc, char* argv [])
{
  GError* tmp_err = NULL;
  GOptionContext* ctx = NULL;
  const gchar* expression = NULL;

  GOptionEntry entries[] =
  {
    { "benchmark", 0, 0, G_OPTION_ARG_NONE, &benchmark, NULL, NULL },
    { "print-tree", 0, 0, G_OPTION_ARG_NONE, &printtree, NULL, NULL },
    { "print-code", 0, 0, G_OPTION_ARG_NONE, &printcode, NULL, NULL },
    { "expression", 'e', 0, G_OPTION_ARG_STRING, &expression, NULL, "CODE" },
    { "output", 'o', 0, G_OPTION_ARG_FILENAME, &output, NULL, "FILE" },
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL },
  };

  ctx =
  g_option_context_new (NULL);
  g_option_context_set_help_enabled (ctx, TRUE);
  g_option_context_set_ignore_unknown_options (ctx, FALSE);
  g_option_context_add_main_entries (ctx, entries, "en_US");

  g_option_context_parse (ctx, &argc, &argv, &tmp_err);
  g_option_context_free (ctx);

  if (G_UNLIKELY (tmp_err != NULL))
  {
    g_critical
    ("(%s): %s: %i: %s",
     G_STRLOC,
     g_quark_to_string
     (tmp_err->domain),
     tmp_err->code,
     tmp_err->message);
    g_error_free (tmp_err);
    g_assert_not_reached ();
  }
  else
  {
    AbacoRules* rules = NULL;
    AbacoAssembler* assembler = NULL;
    GBytes* expr = NULL;
    GBytes* code = NULL;

    rules = abaco_rules_new ();
    assembler = abaco_assembler_new ();

    abaco_rules_add_operator (rules, "[\\+]", FALSE, 2, FALSE, &tmp_err);
      g_assert_no_error (tmp_err);
    abaco_rules_add_operator (rules, "[\\-]", FALSE, 2, FALSE, &tmp_err);
      g_assert_no_error (tmp_err);
    abaco_rules_add_operator (rules, "[\\*]", FALSE, 3, FALSE, &tmp_err);
      g_assert_no_error (tmp_err);
    abaco_rules_add_operator (rules, "[\\/]", FALSE, 3, FALSE, &tmp_err);
      g_assert_no_error (tmp_err);
    abaco_rules_add_operator (rules, "[\\^]",  TRUE, 4, FALSE, &tmp_err);
      g_assert_no_error (tmp_err);

    if (expression != NULL)
    {
      expr = g_bytes_new_static (expression, strlen (expression));
      code = do_compile (rules, assembler, expr);

      do_report (code, expression);
      _g_bytes_unref0 (expr);
      _g_bytes_unref0 (code);
    }
    else
    {
      gchar* contents = NULL;
      gchar* quoted = NULL;
      gsize length = 0;
      int i;

      for (i = 1; i < argc; i++)
      {
        g_file_get_contents (argv [i], &contents, &length, &tmp_err);
        if (G_UNLIKELY (tmp_err != NULL))
        {
          g_critical
          ("(%s): %s: %i: %s",
           G_STRLOC,
           g_quark_to_string
           (tmp_err->domain),
           tmp_err->code,
           tmp_err->message);
          g_error_free (tmp_err);
          g_assert_not_reached ();
        }

        expr = g_bytes_new_take (contents, length);
        code = do_compile (rules, assembler, expr);
        quoted = g_strndup (contents, length);

        do_report (code, quoted);
        _g_bytes_unref0 (expr);
        _g_bytes_unref0 (code);
        _g_free0 (quoted);
      }
    }

    _g_object_unref0 (assembler);
    _g_object_unref0 (rules);
  }
return 0;
}
