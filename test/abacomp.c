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
const gchar* execute = NULL;
gboolean benchmark = FALSE;

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

static inline void
do_report (AbacoVM* vm, AbacoMP* mp, const gchar* code)
{
  GString* buffer = NULL;
  GError* tmp_err = NULL;
  gchar* result = NULL;

  if (output != NULL)
  {
    GBytes* dump = NULL;
    gchar* ptr = NULL;
    gsize length = 0;

    dump = abaco_vm_dump (vm, -1);
    ptr = g_bytes_get_data (dump, &length);

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
             abaco_vm_call (vm, 0);
    result = abaco_mp_tostring (mp, -1, 10);

    g_print ("> '%s' = '%s'\r\n", code, result);
    _g_free0 (result);
  }
  else
  {
    const gdouble spt = (gdouble) CLOCKS_PER_SEC;
    const gdouble mpt = spt / (gdouble) 1000;
    const int tries = 100000;
    const int reps = 10;
    gdouble partial = 0;
    gchar* result = NULL;
    gchar* last = NULL;
    clock_t src, dst;
    int i, j;

    buffer = g_string_sized_new (128);

    g_print ("trying with %i cycles, %i times\r\n", tries, reps);

    for (i = 0; i < reps; i++)
    {
      for (j = 0; j < tries; j++)
      {
        abaco_vm_pushvalue (vm, 0);
        src = clock ();
        abaco_vm_call (vm, 0);
        dst = clock ();

        if (j == 0)
        {
          partial = (gdouble) (dst - src);
          last = abaco_mp_tostring (mp, -1, 10);
        }
        else
        {
          result = abaco_mp_tostring (mp, -1, 10);
          if (g_strcmp0 (result, last))
            g_error ("Results differs");

          _g_free0 (last);
          last = result;

          partial += (gdouble) (dst - src);
          partial /= 2;
        }
      }

      abaco_vm_settop (vm, 1);
      g_string_append_printf (buffer, "> '%s'", code);
      g_string_append_printf (buffer, " = '%s'", result);
      g_string_append_printf (buffer, " (took ");
      g_string_append_printf (buffer, "%lf ticks, ", partial);
      g_string_append_printf (buffer, "%lf micros, ", partial / mpt);
      g_string_append_printf (buffer, "%lf seconds)", partial / spt);
      g_print ("%.*s\r\n", buffer->len, buffer->str);
      g_string_truncate (buffer, 0);
      _g_free0 (result);
    }

    g_string_free (buffer, TRUE);
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
    AbacoVM* vm = abaco_mp_new ();
    AbacoMP* mp = ABACO_MP (vm);

    if (execute != NULL)
    {
      abaco_vm_loadstring (vm, execute, &tmp_err);
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

      do_report (vm, mp, execute);
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
        g_error_free (tmp_err);
        g_assert_not_reached ();
      }

      bytes =
      g_bytes_new_take (content, length);
      abaco_vm_loadbytes (vm, bytes, &tmp_err);
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

      do_report (vm, mp, content);
      g_bytes_unref (bytes);
    }
  }
return 0;
}
