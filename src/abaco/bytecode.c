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
#include <bytecode.h>
#include <glib.h>

G_STATIC_ASSERT (sizeof (BHeader) % B_SECTION_ALIGN == 0);
G_STATIC_ASSERT (sizeof (BSection) % B_SECTION_ALIGN == 0);
G_STATIC_ASSERT (sizeof (BHeader) == sizeof (guint64)*2);
G_STATIC_ASSERT (sizeof (BSection) == sizeof (guint64));
G_STATIC_ASSERT (sizeof (BOpcode) == sizeof (guint32));
G_STATIC_ASSERT (((1 << 6) - 1) >= B_OPCODE_MAXOPCODE);
G_STATIC_ASSERT (sizeof (B_HEADER_MAGIC) == 4);

uint32_t
_bytecode_checksum (const uint8_t* code, uint32_t size)
{
  uint32_t i, ints = size / sizeof (gint64);
  gconstpointer ptr = (gconstpointer) code;
  gconstpointer top = ptr + size;
  uint32_t hash = 0;

  for (i = 0; i < ints; i++)
  {
    gint64* chunk = (gint64*) ptr;
            ptr += sizeof (gint64);

    hash ^= g_int64_hash (chunk);
  }

  if (ptr < top)
  {
    union
    {
      gint64 chunk;
      guchar buf [sizeof (gint64)];
    } leftover = {0};

    g_assert (ptr - top < sizeof (gint64));
    for (i = 0; ptr < top; i++, ptr++)
      leftover.buf [i] = *(guchar*) ptr;
    hash ^= g_int64_hash (&leftover.chunk);
  }
return hash;
}
