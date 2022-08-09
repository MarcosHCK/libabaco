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
#include <closure.h>
#include <internal.h>

typedef struct _MpState MpState;

struct _MpState
{
  GBytes* code;
  MpStack* stack;
  GPtrArray* strtab;
  const BSection* stacksect;
  const BSection* strtabsect;
};

static inline const BSection*
_mp_locate_section (GBytes* code, BSectionType type)
{
  gconstpointer ptr = NULL;
  gconstpointer top = NULL;
  const BSection* section = NULL;
  gsize length = 0;

  ptr = g_bytes_get_data (code, &length);
  top = ptr + length;

  while (ptr < top)
  {
    section = (BSection*) ptr;
    if (section->type == type)
      return section;
    if (section->flags & B_SECTION_VIRTUAL)
      ptr += sizeof (BSection);
    else
    {
      gsize size = section->size;
      gsize miss = size % B_SECTION_ALIGN;
      if (miss > 0)
        ptr += size + (B_SECTION_ALIGN - miss);
      else
        ptr += size;
    }
  }
return NULL;
}

static inline GPtrArray*
_mp_load_strtab (GBytes* code, const BSection* section)
{
  GPtrArray* strtab = NULL;
  gconstpointer ptr = NULL;
  gconstpointer top = NULL;
  gsize length = 0;

  length = g_bytes_get_size (code);

  if (top >= section->size + (gpointer) section)
    g_error ("Invalid binary: invalid string table");

  strtab = g_ptr_array_new ();
  ptr = sizeof (BSection) + (gpointer) section;
  top = ptr + section->size - sizeof (BSection);

  while (ptr < top)
  {
    g_ptr_array_add (strtab, (gchar*) ptr);
    ptr += strlen ((gchar*) ptr) + 1;
  }
return strtab;
}

static inline goffset
_mp_locate_entry_offset (GBytes* code)
{
  gconstpointer ptr = NULL;
  gconstpointer top = NULL;
  const BSection* section = NULL;
  goffset offset = 0;
  gsize length = 0;

  ptr = g_bytes_get_data (code, &length);
  top = ptr + length;

  while (ptr < top)
  {
    section = (BSection*) ptr;
    if (section->type == B_SECTION_TYPE_BITS
      && section->flags & B_SECTION_CODE)
    {
      if (G_UNLIKELY (offset != 0))
        return 0;
      else
      {
        goffset start = (goffset) g_bytes_get_data (code, NULL);
        offset = sizeof (BSection) + (goffset) ptr;
        offset -= start;

        if (offset > length)
          return 0;
      }
    }

    if (section->flags & B_SECTION_VIRTUAL)
      ptr += sizeof (BSection);
    else
    {
      gsize size = section->size;
      gsize miss = size % B_SECTION_ALIGN;
      if (miss > 0)
        ptr += size + (B_SECTION_ALIGN - miss);
      else
        ptr += size;
    }
  }
return offset;
}

static inline gint
_mp_doexecute (AbacoMP* self, MpState* state, goffset entry)
{
  const BSection* stacksect = state->stacksect;
  const BSection* strtabsect = state->strtabsect;
  GBytes* code = state->code;
  MpStack* stack = state->stack;
  GPtrArray* strtab = state->strtab;
  gconstpointer ptr = NULL;
  gconstpointer top = NULL;
  BOpcode* opcode = NULL;
  gsize length = 0;

  ptr = g_bytes_get_data (code, &length);
  top = ptr + length;
  ptr = ptr + entry;

  do
  {
    if (G_UNLIKELY (ptr >= top))
      g_error ("Invalid binary: jump out of code");
    else
    {
      opcode = (BOpcode*) ptr;
      switch (opcode->code)
      {
      case B_OPCODE_NOP:
        break;
      case B_OPCODE_MOVE:
        {
          guint dst = opcode->abc.a;
          guint src = opcode->abc.b;
          if (src >= stacksect->size
            || dst >= stacksect->size)
            g_error ("Invalid binary: invalid opcode");
          else
          { 
            _mp_stack_push_index (stack, src);
            _mp_stack_exchange (stack, dst);
            _mp_stack_pop (stack, 1);
          }
        }
        break;
      case B_OPCODE_LOADK:
        {
          guint dst = opcode->abx.a;
          guint src = opcode->abx.bx;
          const gchar* value = NULL;
          const gchar* key = NULL;
          GHashTable* tbl = NULL;
          gboolean has = FALSE;

          if (src >= strtab->len
            || dst >= stacksect->size)
            g_error ("Invalid binary: invalid opcode");
          else
          {
            key = g_ptr_array_index (strtab, src);
            if ((value = _abaco_mp_lookup_constant (self, key)) == NULL)
              value = key; 

            if (!_mp_stack_push_string (stack, value, 10))
              g_error ("Invalid constant '%s'", value);

            _mp_stack_exchange (stack, dst);
            _mp_stack_pop (stack, 1);
          }
        }
        break;
      case B_OPCODE_LOADF:
        {
          guint dst = opcode->abx.a;
          guint src = opcode->abx.bx;
          MpClosure* closure = NULL;
          const gchar* key = NULL;
          GHashTable* tbl = NULL;
          gboolean has = FALSE;

          if (src >= strtab->len
            || dst >= stacksect->size)
            g_error ("Invalid binary: invalid opcode");
          else
          {
            key = g_ptr_array_index (strtab, src);
            if ((closure = _abaco_mp_lookup_function (self, key)) == NULL)
              g_error ("Invalid function '%s'", key);

            GValue value = G_VALUE_INIT;
            g_value_init (&value, _MP_TYPE_CLOSURE);
            _mp_value_set_closure (&value, closure);
            _mp_stack_push_value (stack, &value);
            g_value_unset (&value);

            _mp_stack_exchange (stack, dst);
            _mp_stack_pop (stack, 1);
          }
        }
        break;
      case B_OPCODE_CALL:
        {
          guint dst = opcode->abc.a;
          guint src = opcode->abc.b;
          guint cnt = opcode->abc.c;
          gint result;
          guint i;

          if (src >= stacksect->size
            || dst >= stacksect->size
            || (src + cnt) > stacksect->size)
            g_error ("Invalid binary: invalid opcode");
          else
          {
            GValue value = G_VALUE_INIT;
            _mp_stack_peek_value (stack, dst, &value);
            if (!G_VALUE_HOLDS (&value, _MP_TYPE_CLOSURE))
              g_error ("Invalid function value");
            g_value_unset (&value);

            _mp_stack_push_index (stack, dst);
            _abaco_mp_transfer_from (self, stack);
            for (i = 0; i < cnt; i++)
            {
              _mp_stack_push_index (stack, src + i);
              _abaco_mp_transfer_from (self, stack);
            }

            result =
            abaco_vm_call (ABACO_VM (self), cnt);
            if (result < 0)
              g_assert_not_reached ();
            else
            if (result > 0)
            {
              _abaco_mp_transfer_to (self, stack);
              _mp_stack_exchange (stack, dst);
              _mp_stack_pop (stack, 1);
            }
          }
        }
        break;
      case B_OPCODE_RETURN:
        {
          guint src = opcode->abc.a;
          if (src >= stacksect->size)
            g_error ("Invalid binary: invalid opcode");
          else
          {
            _mp_stack_push_index (stack, src);
            _abaco_mp_transfer_from (self, stack);
            return 1;
          }
        }
        break;
      default:
        g_error ("Invalid binary: invalid opcode");
        break;
      }

      ptr += sizeof (BOpcode);
    }
  }
  while (TRUE);
return 0;
}

gint
_abaco_mp_execute (AbacoMP* self, GBytes* code)
{
  const BSection* stacksect = NULL;
  const BSection* strtabsect = NULL;
  GPtrArray* strtab = NULL;
  MpStack* stack = NULL;
  MpState state = {0};
  goffset entry = 0;
  guint i, top;
  gint result;

  top = abaco_vm_gettop (ABACO_VM (self));

  if ((stacksect = _mp_locate_section (code, B_SECTION_TYPE_STACK)) == NULL)
    g_error ("Invalid binary: can't locate stack section");
  if ((strtabsect = _mp_locate_section (code, B_SECTION_TYPE_STRTAB)) == NULL)
    g_error ("Invalid binary: can't locate string table");
  if ((strtab = _mp_load_strtab (code, strtabsect)) == NULL)
    g_error ("Invalid binary: can't load string table");
  if ((entry = _mp_locate_entry_offset (code)) == 0)
    g_error ("Invalid binary: can't locate entry");

  stack = _mp_stack_new ();

  for (i = 0; i < stacksect->size; i++)
    _mp_stack_push_nil (stack);
  for (i = 0; i < top; i++)
  {
    _abaco_mp_transfer_from (self, stack);
    _mp_stack_exchange (stack, top - i - 1);
    _mp_stack_pop (stack, 1);
  }

  state.code = code;
  state.stack = stack;
  state.strtab = strtab;
  state.stacksect = stacksect;
  state.strtabsect = strtabsect;
  result = _mp_doexecute (self, &state, entry);

  g_ptr_array_unref (strtab);
  _mp_stack_unref (stack);
return result;
}
