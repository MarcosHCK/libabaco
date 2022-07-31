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

static void
abaco_mp_abaco_vm_iface (AbacoVMIface* iface)
{
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
