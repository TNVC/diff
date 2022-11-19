#include "Tree.h"

#include <stdlib.h>
#include <string.h>
#include "Assert.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

#define ERROR(...)                              \
  do                                            \
    {                                           \
      if (error)                                \
        *error = -1;                            \
                                                \
      return __VA_ARGS__;                       \
    } while (0)

db::TreeNode *db::createNode(treeValue_t value, type_t type, int *error)
{
  db::TreeNode *node = (db::TreeNode *)calloc(1, sizeof(db::TreeNode));

  if (!node)
    ERROR(nullptr);

  node->type = type;

  if (type == db::type_t::VARIABLE)
    {
      char *buffer = (char *)calloc(MAX_VARIABLE_SIZE, sizeof(char));
      if (!buffer)
        {
          free(node);

          ERROR(nullptr);
        }

      node->value.variable = buffer;

      strncpy(buffer, value.variable, MAX_VARIABLE_SIZE);
    }
  else
    node->value = value;

  return node;
}

db::TreeNode *db::createNode(db::treeValue_t value, db::type_t type, db::TreeNode *parent, int leftChild, int *error)
{
  assert(parent);

  db::TreeNode *node = createNode(value, type, error);

  if (!node)
    ERROR(nullptr);

  return setParent(node, parent, leftChild, error);
}

db::TreeNode *db::createNode(db::treeValue_t value, db::type_t type, db::TreeNode *left, db::TreeNode *right, int *error)
{
  int errorCode = 0;

  db::TreeNode *node = createNode(value, type, &errorCode);

  if (errorCode)
    ERROR(nullptr);

  node->left  = left;
  node->right = right;

  if (left)
    left->parent  = node;
  if (right)
    right->parent = node;

  return node;
}

db::TreeNode *db::createNode(const db::TreeNode *original, int *error)
{
  if (!original)
    ERROR(nullptr);

  int errorCode = 0;

  db::TreeNode *node =
    createNode(
               original->value,
               original->type,
               original->left ? db::createNode(original->left) : nullptr,
               original->right ? db::createNode(original->right) : nullptr,
               &errorCode
               );

  if (errorCode)
    ERROR(nullptr);

  return node;
}

db::TreeNode *db::setParent(db::TreeNode *child, db::TreeNode *parent, int leftChild, int *error)
{
  assert(child);
  assert(parent);

  child->parent = parent;

  if (leftChild)
    parent->left  = child;
  else
    parent->right = child;

  return child;
}

void db::removeNode(db::TreeNode *node, int *error)
{
  assert(node);

  if (node->left)
    removeNode(node->left);

  if (node->right)
    removeNode(node->right);

  if (node->type == db::type_t::VARIABLE)
      free(node->value.variable);

  free(node);
}
