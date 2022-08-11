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
#include <libabaco_jit.h>
#include <bytecode.h>
#include <glib.h>

const gchar* output = NULL;
const gchar* execute = NULL;
gboolean benchmark = FALSE;

#define _abaco_closure_unref0(var) ((var == NULL) ? NULL : (var = (abaco_closure_unref (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

int
main (int argc, char* argv [])
{
  GError* tmp_err = NULL;
  GOptionContext* ctx = NULL;

  GOptionEntry entries[] =
  {
    { "benchmark", 0, 0, G_OPTION_ARG_NONE, &benchmark, NULL, NULL },
    { "execute", 'e', 0, G_OPTION_ARG_STRING, &execute, NULL, "CODE" },
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
    AbacoJit* jit = abaco_jit_new (ABACO_JIT_TARGET_ARCH_x86_64);
    AbacoClosure* closure = NULL;

    if (execute != NULL)
    {
      closure =
      abaco_jit_compile_string (jit, execute, &tmp_err);
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
        _abaco_closure_unref0 (closure);
        g_assert_not_reached ();
      }

      g_print ("ok!\r\n");
      _abaco_closure_unref0 (closure);
    }
    else
    {
      GBytes* bytes;
      gchar* content;
      gsize length;

      g_file_get_contents (argv [1], &content, &length, &tmp_err);
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

      bytes = g_bytes_new_take (content, length);
      closure = abaco_jit_compile_bytes (jit, bytes, &tmp_err);
      g_bytes_unref (bytes);

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
        _abaco_closure_unref0 (closure);
        g_assert_not_reached ();
      }

      g_print ("ok!\r\n");
      _abaco_closure_unref0 (closure);
    }
  }
return 0;
}
