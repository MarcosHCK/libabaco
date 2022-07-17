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
#include <glib.h>

static void
print_tree (AbacoAstNode* node, int depth)
{
  gchar* pre = g_strnfill (depth * 2, ' ');
  g_print ("%s-%s\r\n", pre, abaco_ast_node_get_symbol (node));

  abaco_ast_node_children_foreach
  (node,
   (AbacoAstForeach) print_tree,
   GINT_TO_POINTER (depth + 1));
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
      AbacoAstNode* ast = NULL;
      int i;

      rules = abaco_rules_new ();

      for (i = 1; i < argc; i++)
      {
        ast =
        abaco_rules_parse(rules, argv [i], -1, &tmp_err);
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

        print_tree (ast, 0);
        abaco_ast_node_unref (ast);
      }
    }
return 0;
}
