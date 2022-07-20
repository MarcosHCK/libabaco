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
#ifndef __ASTATE__
#define __ASTATE__ 1
#ifndef __LIBABACO_INSIDE__
# error "Private header"
#endif // __LIBABACO_INSIDE__
#include <bytecode.h>
#include <gio/gio.h>

typedef struct
{
  union
  {
    GOutputStream* stream;
    GSeekable* seekable;
  };

  /* errors */
  GError* throwed;

  /* code generator */
  GStringChunk* tmpor;
  GHashTable* inputs;
  GHashTable* locals;
  GPtrArray* strings;
  goffset lastsect;
  gboolean lastfill;

  /* registries */
  GQueue unused;
  guint n_regs;
} AState;

#if __cplusplus
extern "C" {
#endif // __cplusplus

G_GNUC_INTERNAL
void
_astate_begin (AState* s);
G_GNUC_INTERNAL
void
_astate_finish (AState* s);

#define _astate_throw(s, error) \
  G_STMT_START { \
    if (G_UNLIKELY ((s)->throwed != NULL)) \
      { \
        g_critical \
        ("(%s): %s: %i: %s", \
         G_STRLOC, \
         g_quark_to_string \
         ((s)->throwed->domain), \
         (s)->throwed->code, \
         (s)->throwed->message); \
        g_error_free ((s)->throwed); \
        g_error_free ((error)); \
        g_assert_not_reached (); \
      } \
      else \
      { \
        g_propagate_error (&(s)->throwed, (error)); \
      } \
  } G_STMT_END;

#define _astate_propagate(s, error) \
  (gboolean) \
  (G_GNUC_EXTENSION({ \
    if (G_UNLIKELY ((s)->throwed != NULL)) \
    { \
      GError* throwed = \
      g_steal_pointer (&(s)->throwed); \
      g_propagate_error ((error), throwed); \
      TRUE; \
    } \
    FALSE; \
  }))

G_GNUC_INTERNAL
void
(_astate_throw) (AState* s, GError* error);
G_GNUC_INTERNAL
void
(_astate_propagate) (AState* s, GError** error);

G_GNUC_INTERNAL
gboolean
_astate_write (AState* s, gpointer buf, gsize bufsz);

#define _astate_put(s,opcode) \
  G_STMT_START { \
    BOpcode c = (opcode); \
    _astate_write ((s), &c, sizeof (c)); \
  } G_STMT_END;

G_GNUC_INTERNAL
void
(_astate_put) (AState* s, BOpcode opcode);

G_GNUC_INTERNAL
gboolean
_astate_begin_section (AState* s, const gchar* name, BSection* base);
G_GNUC_INTERNAL
gboolean
_astate_finish_section (AState* s);
G_GNUC_INTERNAL
gboolean
_astate_switch_section (AState* s, const gchar* name, BSection* base);

G_GNUC_INTERNAL
guint
_astate_registry_alloc (AState* s);
G_GNUC_INTERNAL
void
_astate_registry_alloca (AState* s, guint* regs, guint n_regs);
G_GNUC_INTERNAL
void
_astate_registry_free (AState* s, guint idx);

G_GNUC_INTERNAL
guint
_astate_callstack_push (AState* s);
G_GNUC_INTERNAL
guint
_astate_callstack_steal (AState* s);
G_GNUC_INTERNAL
void
_astate_callstack_pop (AState* s);

#if __cplusplus
}
#endif // __cplusplus

#endif // __ASTATE__
