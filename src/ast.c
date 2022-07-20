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
#include <ast.h>

typedef struct _Chain Chain;

static gint AbacoAstNode_private_offset;
static void abaco_ast_node_class_init (AbacoAstNodeClass* klass);
static void abaco_ast_node_init (AbacoAstNode* self);

struct _AbacoAstNodePrivate
{
  gchar* symbol;
  AbacoAstSymbolKind kind;

  struct _Chain
  {
    AbacoAstNode* self;
    Chain* next;
    Chain* prev;
    Chain* parent;
    Chain* children;
  } chain;
};

G_STATIC_ASSERT (sizeof (Chain) == sizeof (GNode));
G_STATIC_ASSERT (G_STRUCT_OFFSET (Chain, next) == G_STRUCT_OFFSET (GNode, next));
G_STATIC_ASSERT (G_STRUCT_OFFSET (Chain, prev) == G_STRUCT_OFFSET (GNode, prev));
G_STATIC_ASSERT (G_STRUCT_OFFSET (Chain, parent) == G_STRUCT_OFFSET (GNode, parent));
G_STATIC_ASSERT (G_STRUCT_OFFSET (Chain, children) == G_STRUCT_OFFSET (GNode, children));
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

GType
abaco_ast_node_get_type(void)
{
  static gsize __typeid__ = 0;
  if (g_once_init_enter (&__typeid__))
  {
    GType gtype;

    static const
    GTypeInfo __type__ =
    {
      sizeof(AbacoAstNodeClass),
      (GBaseInitFunc) NULL,
      (GBaseFinalizeFunc) NULL,
      (GClassInitFunc) abaco_ast_node_class_init,
      (GClassFinalizeFunc) NULL,
      NULL,
      sizeof(AbacoAstNode),
      0,
      (GInstanceInitFunc) abaco_ast_node_init,
      NULL
    };

    static const
    GTypeFundamentalInfo __fundamental__ =
    {
      (G_TYPE_FLAG_CLASSED
       | G_TYPE_FLAG_INSTANTIATABLE
       | G_TYPE_FLAG_DERIVABLE
       | G_TYPE_FLAG_DEEP_DERIVABLE),
    };

    gtype =
    g_type_register_fundamental
    (g_type_fundamental_next (),
     "AbacoAstNode",
     &__type__,
     &__fundamental__,
     0);

    AbacoAstNode_private_offset =
    g_type_add_instance_private (gtype, sizeof (AbacoAstNodePrivate));
    g_once_init_leave (&__typeid__, gtype);
  }
return __typeid__;
}

static void
abaco_ast_node_finalize (AbacoAstNode* self)
{
  abaco_ast_node_children_foreach
  (self,
   abaco_ast_node_unref,
   NULL);

  g_signal_handlers_destroy (self);
  _g_free0 (self->priv->symbol);
}

static void
abaco_ast_node_class_init (AbacoAstNodeClass* klass)
{
	g_type_class_adjust_private_offset (klass, &AbacoAstNode_private_offset);
	klass->finalize = abaco_ast_node_finalize;
}

static void
abaco_ast_node_init (AbacoAstNode* self)
{
	self->priv = G_STRUCT_MEMBER_P (self, AbacoAstNode_private_offset);
  g_atomic_ref_count_init (& self->ref_count);

  self->priv->chain.self = self;
  self->priv->chain.next = NULL;
  self->priv->chain.prev = NULL;
  self->priv->chain.parent = NULL;
  self->priv->chain.children = NULL;
  self->priv->symbol = NULL;
  self->priv->kind = 0;
}

AbacoAstNode*
abaco_ast_node_new (const gchar* symbol, AbacoAstSymbolKind kind)
{
  AbacoAstNode* self = NULL;
  AbacoAstNodePrivate* priv = NULL;

  self = (AbacoAstNode*)
  g_type_create_instance (ABACO_AST_TYPE_NODE);
  priv = self->priv;

  self->priv->symbol = g_strdup (symbol);
  self->priv->kind = kind;
return self;
}

AbacoAstNode*
abaco_ast_node_ref (AbacoAstNode* self)
{
  g_return_val_if_fail (self != NULL, NULL);
  g_atomic_ref_count_inc (&self->ref_count);
return self;
}

void
abaco_ast_node_unref (AbacoAstNode* self)
{
  if (self != NULL)
  {
    if (g_atomic_ref_count_dec (&self->ref_count))
    {
      AbacoAstNodeClass* klass;
      klass = ABACO_AST_NODE_GET_CLASS (self);
      klass->finalize (self);
      g_type_free_instance ((gpointer) self);
    }
  }
}

const gchar*
abaco_ast_node_get_symbol (AbacoAstNode* self)
{
  g_return_val_if_fail (ABACO_AST_IS_NODE (self), NULL);
return self->priv->symbol;
}

AbacoAstSymbolKind
abaco_ast_node_get_kind (AbacoAstNode* self)
{
  g_return_val_if_fail (ABACO_AST_IS_NODE (self), 0);
return self->priv->kind;
}

void
abaco_ast_node_append (AbacoAstNode* self, AbacoAstNode* child)
{
  g_return_if_fail (ABACO_AST_IS_NODE (self));
  g_return_if_fail (ABACO_AST_IS_NODE (child));
  GNode* a = (GNode*) & self->priv->chain;
  GNode* b = (GNode*) & child->priv->chain;
  g_node_append (a, b);
  abaco_ast_node_ref (child);
}

void
abaco_ast_node_prepend (AbacoAstNode* self, AbacoAstNode* child)
{
  g_return_if_fail (ABACO_AST_IS_NODE (self));
  g_return_if_fail (ABACO_AST_IS_NODE (child));
  GNode* a = (GNode*) & self->priv->chain;
  GNode* b = (GNode*) & child->priv->chain;
  g_node_prepend (a, b);
  abaco_ast_node_ref (child);
}

static void
_foreach (GNode* b, gpointer pdata)
{
  AbacoAstNode* node = b->data;
  gpointer* data = pdata;
  ((AbacoAstForeach) data [0]) (node, data [1]);
}

void
abaco_ast_node_children_foreach (AbacoAstNode* self, AbacoAstForeach callback, gpointer data)
{
  g_return_if_fail (ABACO_AST_IS_NODE (self));

  gpointer pdata [2] = { callback, data };
  GNode* a = (GNode*) & self->priv->chain;
  g_node_children_foreach (a, G_TRAVERSE_ALL, _foreach, pdata);
}
