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
#include <glib.h>

const gchar* output = NULL;
gboolean benchmark = FALSE;
gboolean printtree = FALSE;
gboolean printcode = FALSE;

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_bytes_unref0(var) ((var == NULL) ? NULL : (var = (g_bytes_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

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
    g_assert_not_reached ();
    g_print ("> '%s' = '%f'\r\n", code, 0.f);
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
        g_assert_not_reached ();
        result = 0.f;
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
    GBytes* expr = NULL;
    GBytes* code = NULL;

    if (expression != NULL)
    {
      expr = g_bytes_new_static (expression, strlen (expression));
      code = g_bytes_new_static ("ja", 3);

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
        code = g_bytes_new_static ("ja", 3);
        quoted = g_strndup (contents, length);

        do_report (code, quoted);
        _g_bytes_unref0 (expr);
        _g_bytes_unref0 (code);
        _g_free0 (quoted);
      }
    }
  }
return 0;
}
