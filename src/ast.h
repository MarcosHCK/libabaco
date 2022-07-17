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
#ifndef __LIBABACO_AST__
#define __LIBABACO_AST__ 1
#include <glib-object.h>

typedef struct _AbacoAstNode AbacoAstNode;
typedef struct _AbacoAstData AbacoAstData;

#if !defined(VALA_EXTERN)
#if defined(_MSC_VER)
#define VALA_EXTERN __declspec(dllexport) extern
#elif __GNUC__ >= 4
#define VALA_EXTERN __attribute__((visibility("default"))) extern
#else
#define VALA_EXTERN extern
#endif
#endif

typedef enum
{
	ABACO_AST_SYMBOL_KIND_CONSTANT,
	ABACO_AST_SYMBOL_KIND_VARIABLE,
	ABACO_AST_SYMBOL_KIND_FUNCTION,
} AbacoAstSymbolKind;

#define ABACO_AST_TYPE_NODE (abaco_ast_node_get_type ())
#define ABACO_AST_NODE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), ABACO_AST_TYPE_NODE, AbacoAstNode))
#define ABACO_AST_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), ABACO_AST_TYPE_NODE, AbacoAstNodeClass))
#define ABACO_AST_IS_NODE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ABACO_AST_TYPE_NODE))
#define ABACO_AST_IS_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ABACO_AST_TYPE_NODE))
#define ABACO_AST_NODE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), ABACO_AST_TYPE_NODE, AbacoAstNodeClass))

typedef struct _AbacoAstNode AbacoAstNode;
typedef struct _AbacoAstNodeClass AbacoAstNodeClass;
typedef struct _AbacoAstNodePrivate AbacoAstNodePrivate;
typedef void (*AbacoAstForeach) (AbacoAstNode* node, gpointer data);

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
abaco_ast_node_get_type (void) G_GNUC_CONST;

struct _AbacoAstNode
{
  GTypeInstance parent_instance;
  guint ref_count;
  AbacoAstNodePrivate* priv;
};

struct _AbacoAstNodeClass
{
  GTypeClass parent_class;
  void (*finalize) (AbacoAstNode *self);
};

VALA_EXTERN AbacoAstNode*
abaco_ast_node_new (const gchar* symbol, AbacoAstSymbolKind kind);
VALA_EXTERN AbacoAstNode*
abaco_ast_node_ref (AbacoAstNode* node);
VALA_EXTERN void
abaco_ast_node_unref (AbacoAstNode* node);
VALA_EXTERN const gchar*
abaco_ast_node_get_symbol (AbacoAstNode* self);
VALA_EXTERN AbacoAstSymbolKind
abaco_ast_node_get_kind (AbacoAstNode* self);
VALA_EXTERN void
abaco_ast_node_append (AbacoAstNode* self, AbacoAstNode* child);
VALA_EXTERN void
abaco_ast_node_prepend (AbacoAstNode* self, AbacoAstNode* child);
VALA_EXTERN void
abaco_ast_node_children_foreach (AbacoAstNode* self, AbacoAstForeach callback, gpointer data);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LIBABACO_AST__
