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
#include <libabaco_jits.h>
#include <bytecode.h>
#include <glib.h>

const gchar* output = NULL;
const gchar* execute = NULL;
gboolean benchmark = FALSE;

#define _g_closure_unref0(var) ((var == NULL) ? NULL : (var = (g_closure_unref (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

typedef struct
{
  GClosure parent;
  gpointer main;
  gpointer block;
  gsize blocksz;
  gsize stacksz;
} _Closure;

static inline void
do_report (GClosure* closure, const gchar* code)
{
  GValue result = {0};
  GString* buffer = NULL;
  GError* tmp_err = NULL;

  if (output != NULL)
  {
    _Closure* real = NULL;
    GBytes* dump = NULL;
    gchar* ptr = NULL;
    gsize length = 0;

    real = (gpointer) closure;
    length = real->blocksz;
    ptr = real->block;

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
    g_value_init (&result, G_TYPE_STRING);
    g_closure_invoke (closure, &result, 0, NULL, NULL);
    g_print ("> '%s' = '%s'\r\n", code, g_value_get_string (&result));
    g_value_unset (&result);
  }
  else
  {
    const gdouble spt = (gdouble) CLOCKS_PER_SEC;
    const gdouble mpt = spt / (gdouble) 1000;
    const int tries = 100000;
    const int reps = 10;
    const gchar* got = NULL;
    gdouble partial = 0;
    gchar* last = NULL;
    clock_t src, dst;
    int i, j;

    buffer =
    g_string_sized_new (128);
    g_value_init (&result, G_TYPE_STRING);
    g_print ("trying with %i cycles, %i times\r\n", tries, reps);

    for (i = 0; i < reps; i++)
    {
      for (j = 0; j < tries; j++)
      {
        src = clock ();
        g_closure_invoke (closure, &result, 0, NULL, NULL);
        dst = clock ();

        if (j == 0)
        {
          partial = (gdouble) (dst - src);
          last = g_value_dup_string (&result);
        }
        else
        {
          got = g_value_get_string (&result);
          if (g_strcmp0 (got, last))
            g_error ("Results differs");
          partial += (gdouble) (dst - src);
        }

        g_value_reset (&result);
      }

      partial /= (reps * tries);

      g_string_append_printf (buffer, "> '%s'", code);
      g_string_append_printf (buffer, " = '%s'", last);
      g_string_append_printf (buffer, " (took ");
      g_string_append_printf (buffer, "%lf ticks, ", partial);
      g_string_append_printf (buffer, "%lf micros, ", partial / mpt);
      g_string_append_printf (buffer, "%lf seconds)", partial / spt);
      g_print ("%.*s\r\n", buffer->len, buffer->str);
      g_string_truncate (buffer, 0);
      g_free (last);
    }

    g_string_free (buffer, TRUE);
    g_value_unset (&result);
  }
}

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
    AbacoJit* jit = abaco_jit_new (ABACO_JITS_TYPE_X86_64);
    GClosure* closure = NULL;
  
    abaco_jits_arithmetic (jit);
    abaco_jits_power (jit);

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
        _g_closure_unref0 (closure);
        g_assert_not_reached ();
      }

      do_report (closure, execute);
      _g_closure_unref0 (closure);
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
        _g_closure_unref0 (closure);
        g_assert_not_reached ();
      }

      do_report (closure, execute);
      _g_closure_unref0 (closure);
    }
  }
return 0;
}
