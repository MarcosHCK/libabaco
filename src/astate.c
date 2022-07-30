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
#include <astate.h>

G_GNUC_INTERNAL
void
_astate_begin (AState* s)
{
  AState __empty = {0};
  *s = __empty;

  const gsize tmpor_blocksize = 256;
  const gsize strings_prealloc = 32;

  /* create temporary */
  s->stream = g_memory_output_stream_new_resizable ();
  s->tmpor = g_string_chunk_new (tmpor_blocksize);
  s->inputs = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  s->locals = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  s->strings = g_ptr_array_sized_new (strings_prealloc);

  /* registries */
  g_queue_init (&s->unused);
  s->n_regs = 0;
}

G_GNUC_INTERNAL
void
_astate_finish (AState* s)
{
  GError* tmp_err = NULL;

  if (g_queue_get_length (&s->unused) != s->n_regs)
    g_warning ("Leftover registry un-allocated");

  /* unused queue */
  g_queue_clear (&s->unused);

  /* destroy temporary */
  g_ptr_array_unref (s->strings);
  g_hash_table_remove_all (s->locals);
  g_hash_table_remove_all (s->inputs);
  g_hash_table_unref (s->locals);
  g_hash_table_unref (s->inputs);
  g_string_chunk_free (s->tmpor);

  /* destroy stream */
  g_object_unref (s->stream);
}

G_GNUC_INTERNAL
void
(_astate_throw) (AState* s, GError* error) { _astate_throw (s, error); }
G_GNUC_INTERNAL
void
(_astate_propagate) (AState* s, GError** error) { _astate_propagate (s, error); }

G_GNUC_INTERNAL
gboolean
_astate_write (AState* s, gpointer buf, gsize bufsz)
{
  GError* tmp_err = NULL;
  g_output_stream_write (s->stream, buf, bufsz, NULL, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      _astate_throw (s, tmp_err);
      return FALSE;
    }
return TRUE;
}

G_GNUC_INTERNAL
void
(_astate_put) (AState* s, BOpcode opcode) { _astate_put (s, opcode); }

G_GNUC_INTERNAL
gboolean
_astate_begin_section (AState* s, const gchar* name, BSection* base)
{
  BSection sect = {0};
  GError* tmp_err = NULL;

  sect = *base;
  sect.name = s->strings->len;

  s->lastsect = g_seekable_tell (s->seekable);
  s->lastfill = (sect.flags & B_SECTION_VIRTUAL) ? FALSE : TRUE;

  g_ptr_array_add (s->strings, g_string_chunk_insert_const (s->tmpor, name));

  _astate_write (s, &sect, sizeof (sect));
  _astate_propagate (s, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      _astate_throw (s, tmp_err);
      return FALSE;
    }
return TRUE;
}

G_GNUC_INTERNAL
gboolean
_astate_finish_section (AState* s)
{
  GError* tmp_err = NULL;

  /* flush stream */
  g_output_stream_flush (s->stream, NULL, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      _astate_throw (s, tmp_err);
      return FALSE;
    }

  if (s->lastfill)
  {
    /* calcuate offsets */
    goffset offset = g_seekable_tell (s->seekable);
    goffset land = s->lastsect + G_STRUCT_OFFSET (BSection, size);
    goffset size = offset - s->lastsect;

    /* make aligned */
    if (size % B_SECTION_ALIGN)
    {
      goffset miss = size % B_SECTION_ALIGN;
      goffset add = B_SECTION_ALIGN - miss;
              offset += add;
              size += add;

      guchar trash [B_SECTION_ALIGN] = {0};

      g_output_stream_write_all (s->stream, trash, add, NULL, NULL, &tmp_err);
      if (G_UNLIKELY (tmp_err != NULL))
      {
        _astate_throw (s, tmp_err);
        return FALSE;
      }
    }

    /* histeric checks */
    g_assert (G_MAXUINT32 >= size);
    uint32_t rsize = (uint32_t) size;

    /* seek to header */
    g_seekable_seek (s->seekable, land, G_SEEK_SET, NULL, &tmp_err);
    if (G_UNLIKELY (tmp_err != NULL))
      {
        _astate_throw (s, tmp_err);
        return FALSE;
      }

    /* patch header */
    g_output_stream_write (s->stream, &rsize, sizeof (rsize), NULL, &tmp_err);
    if (G_UNLIKELY (tmp_err != NULL))
      {
        _astate_throw (s, tmp_err);
        return FALSE;
      }

    /* seek to end */
    g_seekable_seek (s->seekable, offset, G_SEEK_SET, NULL, &tmp_err);
    if (G_UNLIKELY (tmp_err != NULL))
      {
        _astate_throw (s, tmp_err);
        return FALSE;
      }
  }
return TRUE;
}

G_GNUC_INTERNAL
gboolean
_astate_switch_section (AState* s, const gchar* name, BSection* base)
{
  GError* tmp_err = NULL;

  _astate_finish_section (s);
  _astate_propagate (s, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      _astate_throw (s, tmp_err);
      return FALSE;
    }

  _astate_begin_section (s, name, base);
  _astate_propagate (s, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      _astate_throw (s, tmp_err);
      return FALSE;
    }
return TRUE;
}

G_GNUC_INTERNAL
guint
_astate_registry_alloc (AState* s)
{
  gpointer pidx;
  guint32 idx;

  if (g_queue_get_length (&s->unused) > 0)
  {
    pidx = g_queue_pop_head (&s->unused);
    idx = GPOINTER_TO_UINT (pidx);
    return idx;
  }
  else
  {
    if (s->n_regs >= G_MAXUINT8)
    {
      g_critical ("All registers are allocated");
      g_assert_not_reached ();
    }

    return s->n_regs++;
  }
}

G_GNUC_INTERNAL
void
_astate_registry_alloca (AState* s, guint* regs, guint n_regs)
{
  gpointer ptr = NULL;
  guint i;

  for (i = 0; i < n_regs; i++)
  {
    if (s->n_regs >= G_MAXUINT - 1)
    {
      g_critical ("All registers are allocated");
      g_assert_not_reached ();
    }

    regs [i] = s->n_regs++;
  }
}

G_GNUC_INTERNAL
void
_astate_registry_free (AState* s, guint idx)
{
  gpointer pidx;
  pidx = GUINT_TO_POINTER (idx);
  g_queue_push_head (&s->unused, pidx);
}
