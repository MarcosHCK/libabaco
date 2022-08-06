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
#include <closure.h>
#include <libabaco_mp.h>
#include <value.h>

static void
abaco_mp_abaco_vm_iface (AbacoVMIface* iface);

#define ABACO_MP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), ABACO_TYPE_MP, AbacoMPClass))
#define ABACO_IS_MP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ABACO_TYPE_MP))
#define ABACO_MP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), ABACO_TYPE_MP, AbacoMPClass))
typedef struct _AbacoMPClass AbacoMPClass;

struct _AbacoMP
{
  GObject parent;

  /*<private>*/
  MpStack* stack;
  guint top;
};

struct _AbacoMPClass
{
  GObjectClass parent;
};

enum
{
  prop_0,
  prop_top,
  prop_number,
};

static
GParamSpec* properties [prop_number] = {0};

G_DEFINE_FINAL_TYPE_WITH_CODE
(AbacoMP,
 abaco_mp,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (ABACO_TYPE_VM,
  abaco_mp_abaco_vm_iface));

/* private API */

#define gettop() \
  (G_GNUC_EXTENSION ({ \
    AbacoMP* __self = self; \
    (gettop) (__self); \
  }))

static inline gint
(gettop) (AbacoMP* self)
{
#if DEVELOPER == 1
  g_assert (_mp_stack_get_length (self->stack) >= self->top);
#endif // DEVELOPER
  return _mp_stack_get_length (self->stack) - self->top;
}

#define validate_index(index) \
  (G_GNUC_EXTENSION ({ \
    AbacoMP* __self = self; \
    gint __index = (index); \
    (validate_index) (__self, __index); \
  }))

static inline gint
(validate_index) (AbacoMP* self, gint index)
{
  if (index < 0)
  {
    if ((index = gettop () + index) >= 0)
      return validate_index (index);
  }
  else
  {
    index += (gint) self->top;
    if (index < _mp_stack_get_length (self->stack))
      return index;
  }
return -1;
}

/* Abaco.VM */

static void
abaco_mp_abaco_vm_iface_settop (AbacoVM* pself, gint new)
{
  AbacoMP* self = ABACO_MP (pself);
  guint i, top = gettop ();
  if (new < top)
  {
    new = top - new;
    if (new < 0)
      g_error ("Too many values to pop");
    else
      _mp_stack_pop (self->stack, new);
  }
  else
  {
    for (i = 0; i < (new - top); i++)
      _mp_stack_push_nil (self->stack);
  }
}

static gint
abaco_mp_abaco_vm_iface_gettop (AbacoVM* pself)
{
  AbacoMP* self = ABACO_MP (pself);
return gettop ();
}

static void
abaco_mp_abaco_vm_iface_pushvalue (AbacoVM* pself, gint index)
{
  AbacoMP* self = ABACO_MP (pself);
  if ((index = validate_index (index)) < 0)
    g_error ("Invalid index");
  _mp_stack_push_index (self->stack, index);
}

static void
abaco_mp_abaco_vm_iface_pushupvalue (AbacoVM* pself, gint index)
{
  AbacoMP* self = ABACO_MP (pself);
  guint length = _mp_stack_get_length (self->stack);
  if (self->top == 0)
    g_error ("Upvalue queried outside a closure");
  else
  {
    guint loc = validate_index (0) - 1;
    GValue value = G_VALUE_INIT;
    _mp_stack_peek_value (self->stack, loc, &value);
    if (!G_VALUE_HOLDS (&value, _MP_TYPE_CLOSURE))
      g_error ("Fix this!");
    else
    {
      MpClosure* closure =
      _mp_value_get_closure (&value);
      _mp_closure_pushupvalue (closure, index, self->stack);
      g_value_unset (&value);
    }
  }
}

static void
abaco_mp_abaco_vm_iface_pop (AbacoVM* pself)
{
  AbacoMP* self = ABACO_MP (pself);
  if (gettop () == 0)
    g_error ("Can't pop any more");
  else
    _mp_stack_pop (self->stack, 1);
}

static void
abaco_mp_abaco_vm_iface_insert (AbacoVM* pself, gint index)
{
  AbacoMP* self = ABACO_MP (pself);
  if ((index = validate_index (index)) < 0)
    g_error ("Invalid index");
  if ((validate_index (-1)) < 0)
    g_error ("Empty stack");
  _mp_stack_insert (self->stack, index);
}

static void
abaco_mp_abaco_vm_iface_remove (AbacoVM* pself, gint index)
{
  AbacoMP* self = ABACO_MP (pself);
  if ((index = validate_index (index)) < 0)
    g_error ("Invalid index");
  _mp_stack_remove (self->stack, index);
}

static void
abaco_mp_abaco_vm_iface_pushcclosure (AbacoVM* pself, AbacoCClosure callback, gint upvalues)
{
  AbacoMP* self = ABACO_MP (pself);
  guint top = gettop ();

  if (upvalues < 0)
    g_error ("Upvalues count must be non-negative");
  if (upvalues > top)
    g_error ("Too much upvalues for closure");

  MpClosure* closure =
  _mp_cclosure_new (self->stack, upvalues, callback);

  GValue value = G_VALUE_INIT;
  g_value_init (&value, _MP_TYPE_CCLOSURE);
  _mp_value_take_closure (&value, closure);
  _mp_stack_push_value (self->stack, &value);
  g_value_unset (&value);
}

static gint
abaco_mp_abaco_vm_iface_call (AbacoVM* pself, gint args)
{
  AbacoMP* self = ABACO_MP (pself);
  guint top = gettop ();

  if (args < 0)
    g_error ("Arguments count must be non-negative");
  if ((args + 1) > top)
    g_error ("Too much arguments for call");
  gint result, loc = validate_index (-(args + 1));
  guint oldtop = self->top;
  MpClosure* closure = NULL;

  GValue value = G_VALUE_INIT;
  _mp_stack_peek_value (self->stack, loc, &value);
  if (!G_VALUE_HOLDS (&value, _MP_TYPE_CLOSURE))
    g_error ("Attempt to call a non-function value");
  else
  {
    closure =
    _mp_value_get_closure (&value);
    _mp_closure_ref (closure);
    g_value_unset (&value);

    self->top = _mp_stack_get_length (self->stack) - args;
    result = _mp_closure_invoke (closure, self);

    if (result > 0)
    {
      if (gettop () == 0)
    g_error ("Closure returns value but stack is empty");
      abaco_vm_insert (pself, 0);
      abaco_vm_settop (pself, 1);
    }
    else
    {
      abaco_vm_settop (pself, 0);
    }

    _mp_stack_remove (self->stack, loc);
    _mp_closure_unref (closure);
    self->top = oldtop;
  }
return result;
}

/* class */

static void
abaco_mp_abaco_vm_iface (AbacoVMIface* iface)
{
  iface->settop = abaco_mp_abaco_vm_iface_settop;
  iface->gettop = abaco_mp_abaco_vm_iface_gettop;
  iface->pushvalue = abaco_mp_abaco_vm_iface_pushvalue;
  iface->pushupvalue = abaco_mp_abaco_vm_iface_pushupvalue;
  iface->pop = abaco_mp_abaco_vm_iface_pop;
  iface->insert = abaco_mp_abaco_vm_iface_insert;
  iface->remove = abaco_mp_abaco_vm_iface_remove;
  iface->pushcclosure = abaco_mp_abaco_vm_iface_pushcclosure;
  iface->call = abaco_mp_abaco_vm_iface_call;
}

static void
abaco_mp_class_set_property (GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec)
{
  AbacoMP* self = ABACO_MP (pself);
  switch (prop_id)
  {
  case prop_top:
    abaco_vm_settop (ABACO_VM (self), g_value_get_int (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, prop_id, pspec);
    break;
  }
}

static void
abaco_mp_class_get_property (GObject* pself, guint prop_id, GValue* value, GParamSpec* pspec)
{
  AbacoMP* self = ABACO_MP (pself);
  switch (prop_id)
  {
  case prop_top:
    g_value_set_int (value, abaco_vm_gettop (ABACO_VM (self)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, prop_id, pspec);
    break;
  }
}

static void
abaco_mp_class_finalize (GObject* pself)
{
  AbacoMP* self = ABACO_MP (pself);
  _mp_stack_unref (self->stack);
G_OBJECT_CLASS (abaco_mp_parent_class)->finalize (pself);
}

static void
abaco_mp_class_dispose (GObject* pself)
{
  AbacoMP* self = ABACO_MP (pself);
G_OBJECT_CLASS (abaco_mp_parent_class)->dispose (pself);
}

static void
abaco_mp_class_init (AbacoMPClass* klass)
{
  GObjectClass* oclass = G_OBJECT_CLASS (klass);
  GParamFlags flags1 = G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS;

  oclass->set_property = abaco_mp_class_set_property;
  oclass->get_property = abaco_mp_class_get_property;
  oclass->finalize = abaco_mp_class_finalize;
  oclass->dispose = abaco_mp_class_dispose;

  properties [prop_top] = g_param_spec_int ("top", "top", "top", 0, G_MAXINT, 0, flags1);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
}

static void
abaco_mp_init (AbacoMP* self)
{
  self->stack = _mp_stack_new ();
}

/* API */

AbacoVM*
abaco_mp_new ()
{
  return
  g_object_new (ABACO_TYPE_MP, NULL);
}

const gchar*
abaco_mp_typename (AbacoMP* self, gint index)
{
  g_return_val_if_fail (ABACO_IS_MP (self), 0);
  g_return_val_if_fail ((index = validate_index (index)) >= 0, 0);
return _mp_stack_type (self->stack, index);
}

void
abaco_mp_pushdouble (AbacoMP* self, double value)
{
  g_return_if_fail (ABACO_IS_MP (self));
  _mp_stack_push_double (self->stack, value);
}

void
abaco_mp_pushldouble (AbacoMP* self, long double value)
{
  g_return_if_fail (ABACO_IS_MP (self));
  _mp_stack_push_double (self->stack, value);
}

gboolean
abaco_mp_pushstring (AbacoMP* self, const gchar* value, int base)
{
  g_return_val_if_fail (ABACO_IS_MP (self), FALSE);
  g_return_val_if_fail (value != NULL, FALSE);
  _mp_stack_push_string (self->stack, value, base);
}

double
abaco_mp_todouble (AbacoMP* self, gint index)
{
  g_return_val_if_fail (ABACO_IS_MP (self), 0);
  g_return_val_if_fail ((index = validate_index (index)) >= 0, 0);
return _mp_stack_peek_double (self->stack, index);
}

long double
abaco_mp_toldouble (AbacoMP* self, gint index)
{
  g_return_val_if_fail (ABACO_IS_MP (self), 0);
  g_return_val_if_fail ((index = validate_index (index)) >= 0, 0);
return _mp_stack_peek_ldouble (self->stack, index);
}

gchar*
abaco_mp_tostring (AbacoMP* self, gint index, int base)
{
  g_return_val_if_fail (ABACO_IS_MP (self), NULL);
  g_return_val_if_fail ((index = validate_index (index)) >= 0, NULL);
return _mp_stack_peek_string (self->stack, index, base);
}
