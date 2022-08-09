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
execute (GBytes* bytes)
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

int
main (int argc, char* argv [])
{
  GError* tmp_err = NULL;
  GOptionContext* ctx = NULL;
  GOptionEntry entries[] =
  {
    {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL},
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
      AbacoAstNode* ast = NULL;
      GBytes* code = NULL;
      int i, j, k;

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

      for (i = 1; i < argc; i++)
      {
        g_print ("\r\n");
        g_print ("> %s\r\n", argv [i]);

        ast =
        abaco_rules_parse (rules, argv [i], -1, &tmp_err);
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
            g_object_unref (rules);
            g_assert_not_reached ();
          }

        g_print ("\r\n");
        print_tree (ast, 0);

        code =
        abaco_assembler_assemble (assembler, ast, &tmp_err);
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
            g_object_unref (rules);
            g_assert_not_reached ();
          }

        g_print ("\r\n");
        print_code (code);
        g_print ("\r\n");
        disassemble (code);
        g_print ("\r\n");

        const gdouble spt = (gdouble) CLOCKS_PER_SEC;
        const gdouble mpt = spt / (gdouble) 1000;
        const int tries = 100000;
        const int reps = 10;
        gdouble result;
        gdouble last;
        int j;

        for (j = 0; j < reps; j++)
        {
          clock_t start = clock ();
          for (k = 0; k < tries; k++)
            result = execute (code);

          if (j > 0 && result != last)
            g_error ("Mismatching results for same input");
          last = result;

          clock_t stop = clock ();
          gdouble took = (gdouble) (stop - start);
                  took /= (gdouble) tries;
          g_print ("took %lf ticks, %lf micros, %lf secs\r\n",
                    took,
                    took / mpt,
                    took / spt);
        }

        g_print ("result %lf\r\n", result);
        abaco_ast_node_unref (ast);
      }

      g_object_unref (assembler);
      g_object_unref (rules);
    }
return 0;
}
