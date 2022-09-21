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
 * along with libabaco. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __AST_CHAIN__
#define __AST_CHAIN__
#ifndef __LIBABACO_INSIDE__
# error "Private header"
#endif // __LIBABACO_INSIDE__
#include <glib.h>

typedef struct _AstChain AstChain;

G_BEGIN_DECLS

struct _AstChain
{
  gpointer self;
  AstChain* next;
  AstChain* prev;
  AstChain* parent;
  AstChain* children;
};

G_GNUC_INTERNAL gpointer
abaco_ast_node_ref (gpointer instance);
G_GNUC_INTERNAL void
abaco_ast_node_unref (gpointer instance);

G_STATIC_ASSERT (G_STRUCT_OFFSET (AstChain, self) == G_STRUCT_OFFSET (GNode, data));
G_STATIC_ASSERT (G_STRUCT_OFFSET (AstChain, next) == G_STRUCT_OFFSET (GNode, next));
G_STATIC_ASSERT (G_STRUCT_OFFSET (AstChain, prev) == G_STRUCT_OFFSET (GNode, prev));
G_STATIC_ASSERT (G_STRUCT_OFFSET (AstChain, parent) == G_STRUCT_OFFSET (GNode, parent));
G_STATIC_ASSERT (G_STRUCT_OFFSET (AstChain, children) == G_STRUCT_OFFSET (GNode, children));

static inline void
_ast_chain_append (AstChain* a, AstChain* b)
{
  g_node_append ((GNode*) a, (GNode*) b);
  abaco_ast_node_ref (b->self);
}

static inline void
_ast_chain_prepend (AstChain* a, AstChain* b)
{
  g_node_prepend ((GNode*) a, (GNode*) b);
  abaco_ast_node_ref (b->self);
}

static void
_ast_chain_kill (AstChain* child, gpointer parent)
{
  g_node_unlink ((GNode*) child);
  abaco_ast_node_unref (child->self);
}

static inline void
_ast_chain_destroy (AstChain* chain)
{
  g_node_children_foreach ((GNode*) chain, G_TRAVERSE_ALL, _ast_chain_kill, chain);
}

G_END_DECLS

#endif // __AST_CHAIN__
