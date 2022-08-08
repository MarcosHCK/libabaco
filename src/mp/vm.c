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

static void
abaco_mp_abaco_vm_iface (AbacoVMIface* iface);

#define ABACO_MP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), ABACO_TYPE_MP, AbacoMPClass))
#define ABACO_IS_MP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ABACO_TYPE_MP))
#define ABACO_MP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), ABACO_TYPE_MP, AbacoMPClass))
typedef struct _AbacoMPClass AbacoMPClass;
#define _abaco_ast_node_unref0(var) ((var == NULL) ? NULL : (var = (abaco_ast_node_unref (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_bytes_unref0(var) ((var == NULL) ? NULL : (var = (g_bytes_unref (var), NULL)))

struct _AbacoMP
{
  GObject parent;

  /*<private>*/
  AbacoAssembler* assembler;
  AbacoRules* rules;
  GHashTable* constants;
  GHashTable* functions;
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

/* internal API */

gpointer
_abaco_mp_toobject (AbacoMP* self, gint index)
{
  if ((index = validate_index (index)) < 0)
    g_error ("Invalid index");
return _mp_stack_peek (self->stack, index);
}

void
_abaco_mp_transfer_to (AbacoMP* self, MpStack* dst)
{
  if (gettop () == 0)
    g_error ("Empty stack");
  _mp_stack_transfer (dst, self->stack);
}

void
_abaco_mp_transfer_from (AbacoMP* self, MpStack* src)
{
  _mp_stack_transfer (self->stack, src);
}

void
_abaco_mp_new_integer (AbacoMP* self)
{
  _mp_stack_new_integer (self->stack);
}

void
_abaco_mp_new_rational (AbacoMP* self)
{
  _mp_stack_new_rational (self->stack);
}

void
_abaco_mp_new_real (AbacoMP* self)
{
  _mp_stack_new_real (self->stack);
}

const gchar*
_abaco_mp_lookup_constant (AbacoMP* self, const gchar* key)
{
  const gchar* value = NULL;
  if (g_hash_table_lookup_extended
    (self->constants,
     key, NULL,
     (gpointer*) &value))
    return value;
return NULL;
}

gpointer
_abaco_mp_lookup_function (AbacoMP* self, const gchar* key)
{
  MpClosure* value = NULL;
  if (g_hash_table_lookup_extended
    (self->functions,
     key, NULL,
     (gpointer*) &value))
    return value;
return NULL;
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
abaco_mp_abaco_vm_iface_exchange (AbacoVM* pself, gint index)
{
  AbacoMP* self = ABACO_MP (pself);
  if ((index = validate_index (index)) < 0)
    g_error ("Invalid index");
  if ((validate_index (-1)) < 0)
    g_error ("Empty stack");
  _mp_stack_exchange (self->stack, index);
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

static gboolean
abaco_mp_abaco_vm_iface_loadbytes (AbacoVM* pself, GBytes* bytes, GError** error)
{
  AbacoMP* self = ABACO_MP (pself);
  const gchar* input = NULL;
  GValue value = G_VALUE_INIT;
  MpClosure* closure = NULL;
  BHeader* header = NULL;
  gsize length = 0;

  input = g_bytes_get_data (bytes, &length);
  header = (BHeader*) input;

  if (!b_header_check_magic (header))
  {
    AbacoAstNode* tree = NULL;
    GError* tmp_err = NULL;

    tree =
    abaco_rules_parse (self->rules, input, length, &tmp_err);
    if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _abaco_ast_node_unref0 (tree);
      return FALSE;
    }

    bytes =
    abaco_assembler_assemble (self->assembler, tree, &tmp_err);
    _abaco_ast_node_unref0 (tree);
    if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _g_bytes_unref0 (bytes);
      return FALSE;
    }

    closure =
    _mp_function_new (bytes);
    g_bytes_unref (bytes);

    g_value_init (&value, _MP_TYPE_FUNCTION);
    _mp_value_take_closure (&value, closure);
    _mp_stack_push_value (self->stack, &value);
    g_value_unset (&value);
  }
  else
  {
    input += sizeof (BHeader);
    length -= sizeof (BHeader);

    bytes = g_bytes_new_static (input, length);
    closure = _mp_function_new (bytes);
    g_bytes_unref (bytes);

    g_value_init (&value, _MP_TYPE_FUNCTION);
    _mp_value_take_closure (&value, closure);
    _mp_stack_push_value (self->stack, &value);
    g_value_unset (&value);
  }
return TRUE;
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

static void
abaco_mp_abaco_vm_iface_register_operator (AbacoVM* pself, const gchar* expr, gboolean assoc, gint precedence, gboolean unary)
{
  AbacoMP* self = ABACO_MP (pself);
  GError* tmp_err = NULL;
  gint loc;

  if ((loc = validate_index (-1)) < 0)
    g_error ("Empty stack");

  GValue value = G_VALUE_INIT;
  _mp_stack_peek_value (self->stack, loc, &value);
  if (!G_VALUE_HOLDS (&value, _MP_TYPE_CLOSURE))
    g_error ("Attempt to register a non-function value");
  else
  {
    MpClosure* closure = NULL;
    closure = _mp_value_get_closure (&value);
    closure = _mp_closure_ref (closure);
    g_value_unset (&value);

    abaco_rules_add_operator (self->rules, expr, assoc, precedence, unary, &tmp_err);
    g_hash_table_insert (self->functions, g_strdup (expr), closure);
    if (G_UNLIKELY (tmp_err != NULL))
    {
      g_error
      ("%s: %i: %s",
       g_quark_to_string
       (tmp_err->domain),
       tmp_err->code,
       tmp_err->message);
      g_error_free (tmp_err);
      g_assert_not_reached ();
    }
  }
}

static void
abaco_mp_abaco_vm_iface_register_function (AbacoVM* pself, const gchar* expr)
{
  AbacoMP* self = ABACO_MP (pself);
  GError* tmp_err = NULL;
  gint loc;

  if ((loc = validate_index (-1)) < 0)
    g_error ("Empty stack");

  GValue value = G_VALUE_INIT;
  _mp_stack_peek_value (self->stack, loc, &value);
  if (!G_VALUE_HOLDS (&value, _MP_TYPE_CLOSURE))
    g_error ("Attempt to register a non-function value");
  else
  {
    MpClosure* closure = NULL;
    closure = _mp_value_get_closure (&value);
    closure = _mp_closure_ref (closure);
    abaco_mp_abaco_vm_iface_pop (pself);
    g_value_unset (&value);

    abaco_rules_add_function (self->rules, expr, -1, &tmp_err);
    g_hash_table_insert (self->functions, g_strdup (expr), closure);
    if (G_UNLIKELY (tmp_err != NULL))
    {
      g_error
      ("%s: %i: %s",
       g_quark_to_string
       (tmp_err->domain),
       tmp_err->code,
       tmp_err->message);
      g_error_free (tmp_err);
      g_assert_not_reached ();
    }
  }
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
  iface->exchange = abaco_mp_abaco_vm_iface_exchange;
  iface->insert = abaco_mp_abaco_vm_iface_insert;
  iface->remove = abaco_mp_abaco_vm_iface_remove;
  iface->pushcclosure = abaco_mp_abaco_vm_iface_pushcclosure;
  iface->loadbytes = abaco_mp_abaco_vm_iface_loadbytes;
  iface->call = abaco_mp_abaco_vm_iface_call;
  iface->register_operator = abaco_mp_abaco_vm_iface_register_operator;
  iface->register_function = abaco_mp_abaco_vm_iface_register_function;
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
  g_hash_table_unref (self->constants);
  g_hash_table_unref (self->functions);
G_OBJECT_CLASS (abaco_mp_parent_class)->finalize (pself);
}

static void
abaco_mp_class_dispose (GObject* pself)
{
  AbacoMP* self = ABACO_MP (pself);
  _g_object_unref0 (self->assembler);
  _g_object_unref0 (self->rules);
  g_hash_table_remove_all (self->constants);
  g_hash_table_remove_all (self->functions);
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
  self->assembler = abaco_assembler_new ();
  self->rules = abaco_rules_new ();
  self->constants = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  self->functions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, _mp_closure_unref);
  self->stack = _mp_stack_new ();
}

/* API */

AbacoVM*
abaco_mp_new ()
{
  AbacoMP* self =
  g_object_new (ABACO_TYPE_MP, NULL);
  abaco_mp_load_stdlib (self);
return (AbacoVM*) self;
}

AbacoVM*
abaco_mp_new_naked ()
{
  return
  g_object_new (ABACO_TYPE_MP, NULL);
}

void
abaco_mp_load_stdlib (AbacoMP* self)
{
  g_return_if_fail (ABACO_IS_MP (self));
  AbacoRules* rules = (self->rules);
  GHashTable* reg = (self->functions);
  MpClosure* closure = NULL;
  GError* tmp_err = NULL;

  abaco_rules_add_operator (rules, "[\\+]", FALSE, 2, FALSE, &tmp_err);
    g_assert_no_error (tmp_err);
  closure = _mp_cclosure_new (NULL, 0, abaco_mp_arith_add);
  g_hash_table_insert (reg, g_strdup ("+"), closure);

  abaco_rules_add_operator (rules, "[\\-]", FALSE, 2, FALSE, &tmp_err);
    g_assert_no_error (tmp_err);
  closure = _mp_cclosure_new (NULL, 0, abaco_mp_arith_sub);
  g_hash_table_insert (reg, g_strdup ("-"), closure);

  abaco_rules_add_operator (rules, "[\\*]", FALSE, 3, FALSE, &tmp_err);
    g_assert_no_error (tmp_err);
  closure = _mp_cclosure_new (NULL, 0, abaco_mp_arith_mul);
  g_hash_table_insert (reg, g_strdup ("*"), closure);

  abaco_rules_add_operator (rules, "[\\/]", FALSE, 3, FALSE, &tmp_err);
    g_assert_no_error (tmp_err);
  closure = _mp_cclosure_new (NULL, 0, abaco_mp_arith_div);
  g_hash_table_insert (reg, g_strdup ("/"), closure);
}

#undef catch

const gchar*
abaco_mp_typename (AbacoMP* self, gint index)
{
  g_return_val_if_fail (ABACO_IS_MP (self), NULL);
  g_return_val_if_fail ((index = validate_index (index)) >= 0, NULL);
return _mp_stack_type (self->stack, index);
}

gboolean
abaco_mp_cast (AbacoMP* self, gint index, const gchar* type)
{
  g_return_val_if_fail (ABACO_IS_MP (self), FALSE);
  g_return_val_if_fail ((index = validate_index (index)) >= 0, FALSE);
  g_return_val_if_fail (type == MP_TYPE_NIL
                      || type == MP_TYPE_VALUE
                      || type == MP_TYPE_INTEGER
                      || type == MP_TYPE_RATIONAL
                      || type == MP_TYPE_REAL, FALSE);
return _mp_stack_cast (self->stack, index, type);
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

mpz_ptr
abaco_mp_tointeger (AbacoMP* self, gint index)
{
  g_return_val_if_fail (ABACO_IS_MP (self), NULL);
  g_return_val_if_fail ((index = validate_index (index)) >= 0, NULL);
return (mpz_ptr) _mp_stack_peek (self->stack, index);
}

mpq_ptr
abaco_mp_torational (AbacoMP* self, gint index)
{
  g_return_val_if_fail (ABACO_IS_MP (self), NULL);
  g_return_val_if_fail ((index = validate_index (index)) >= 0, NULL);
return (mpq_ptr) _mp_stack_peek (self->stack, index);
}

mpfr_ptr
abaco_mp_toreal (AbacoMP* self, gint index)
{
  g_return_val_if_fail (ABACO_IS_MP (self), NULL);
  g_return_val_if_fail ((index = validate_index (index)) >= 0, NULL);
return (mpfr_ptr) _mp_stack_peek (self->stack, index);
}
