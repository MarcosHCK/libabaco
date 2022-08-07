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
      AbacoVM* vm = abaco_mp_new ();
      AbacoMP* mp = ABACO_MP (vm);
      int i;

      for (i = 1; i < argc; i++)
      {
        g_print ("> %s\r\n", argv [i]);
        abaco_vm_loadstring (vm, argv [i], &tmp_err);
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

        const gdouble spt = (gdouble) CLOCKS_PER_SEC;
        const gdouble mpt = spt / (gdouble) 1000;
        const int tries = 100000;
        const int reps = 10;
        gdouble result;
        gdouble last;
        int j;

        for (i = 0; i < reps; i++)
        {
          clock_t start = clock ();
          for (j = 0; j < tries; j++)
          {
            abaco_vm_pushvalue (vm, 0);
            if (abaco_vm_call (vm, 0) > 0)
            {
              result = abaco_mp_todouble (mp, -1);
                       abaco_vm_settop (vm, 1);
            }
          }

          if (i > 0 && result != last)
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
      }
    }
return 0;
}
