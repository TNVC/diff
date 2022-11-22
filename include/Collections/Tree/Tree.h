#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

#include <math.h>

namespace db {

  enum class type_t {
    OPERATOR,
    VARIABLE,
    NUMBER,
  };

  enum operator_t {
    OPERATOR_ADD,
    OPERATOR_SUB,
    OPERATOR_MUL,
    OPERATOR_DIV,
    OPERATOR_SQRT,
    //OPERATOR_ROOT,
    OPERATOR_SIN,
    OPERATOR_COS,
    OPERATOR_POW,
    OPERATOR_LOG,
    OPERATOR_LN,
    OPERATORS_COUNT,
  };

  const operator_t BINARY_OPERATORS[] =
    {
      OPERATOR_ADD,
      OPERATOR_SUB,
      OPERATOR_MUL,
      OPERATOR_DIV,
      OPERATOR_POW,
      OPERATOR_LOG
      //      OPERATOR_ROOT
    };

  const int BINARY_OPERATORS_COUNT = 6;

  const char *const OPERATOR_NAMES[] =
    {
      "+",
      "-",
      "*",
      "/",
      "sqrt",
      "sin",
      "cos",
      "^",
      "log",
      "ln"
    };

  typedef char  *variable_t;
  typedef double number_t;

  const int MAX_VARIABLE_SIZE = 32;

  union treeValue_t {
    operator_t operat;
    variable_t variable;
    number_t   number;
  };

  struct TreeNode {
    type_t      type;
    treeValue_t value;
    TreeNode   *parent;
    TreeNode   *left;
    TreeNode   *right;
  };

  struct Tree {
    TreeNode *root;
    size_t size;
  };

  enum TreeError {
    TREE_NULLPTR = 0x01 << 0,
  };

  const int TREE_ERRORS_COUNT = 1;

  const double DOUBLE_ERROR = 1e-10;

  inline bool compareNumber(number_t first, number_t second)
  {
    return fabs(first - second) < DOUBLE_ERROR;
  }

  inline bool compareValues(const treeValue_t &first, const treeValue_t &second)
  {
    return (first.operat == second.operat) &&
      (first.variable == second.variable) &&
      (compareNumber(first.number, second.number));
  }

  inline bool validateValue(const treeValue_t &value, type_t type)
  {
    if (type == type_t::VARIABLE)
      return value.variable;
    return true;
  }

  TreeNode *createNode(treeValue_t value, type_t type, int *error = nullptr);

  TreeNode *createNode(treeValue_t value, type_t type, TreeNode *parent, int leftChild = true, int *error = nullptr);

  TreeNode *createNode(treeValue_t value, type_t type, TreeNode *left, TreeNode *right, int *error = nullptr);

  TreeNode *createNode(const TreeNode *original, int *error = nullptr);

  TreeNode *setParent(TreeNode *child, TreeNode *parent, int leftChild = true, int *error = nullptr);

  void removeNode(TreeNode *node, int *error = nullptr);


  unsigned validateTree(const Tree *tree);

  void createTree(Tree *tree, int *error = nullptr);

  void destroyTree(Tree *tree, int *error = nullptr);

  void addElement(TreeNode *node, treeValue_t value, int leftChild, int *error = nullptr);

  const TreeNode *findElement(const Tree *tree, treeValue_t value, type_t type, int isList = false, int *error = nullptr);

#define dumpTree(TREE, ERROR, FILE)                                 \
  do_dumpTree(TREE, ERROR, FILE, __FILE__, __func__, __LINE__, "")

#define dumpTreeWithMessage(TREE, ERROR, FILE, MESSAGE, ...)            \
  do_dumpTree(TREE, ERROR, FILE, __FILE__, __func__, __LINE__, MESSAGE __VA_OPT__(,) __VA_ARGS__)

  void do_dumpTree(
                   const Tree *tree,
                   unsigned error,
                   FILE *file,
                   const char* fileName,
                   const char* functionName,
                   int line,
                   const char *message,
                   ...
                   );

  void saveTree(const Tree *tree, FILE *file, int *error = nullptr);

  void loadTree(Tree *tree, FILE *file, int *error = nullptr);
  void loadTree(
                Tree *tree,
                const char *fileName,
                int *error = nullptr
                );
}
