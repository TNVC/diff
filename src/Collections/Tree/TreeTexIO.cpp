#include "TreeTexIO.h"
#include "TreeDump.h"
#include "DiffDSL.h"

#include "Assert.h"
#include <string.h>

const char *const TEX_OPERATOR_NAMES[] =
  {
    "+",
    "-",
    "\\cdot",
    "\\over",
    "sqrt",
    "sin",
    "cos",
    "^",
    "log",
    "ln"
  };

const int MAX_LEXEME_SIZE = 48;

static_assert(MAX_LEXEME_SIZE >= db::MAX_VARIABLE_SIZE);

static char *toString(const db::treeValue_t value, db::type_t type);

static void printNode(const db::TreeNode *node, FILE *file);

void db::saveTexNode(const db::TreeNode *node, FILE *file, int *error)
{
  if (!node)
    ERROR();

  if (!file)
    ERROR();

  fprintf(file, " $$ ");

  printNode(node, file);

  fprintf(file, " $$ ");
}

void db::saveTexTree(const db::Tree *tree, FILE *file, int *error)
{
  CHECK_VALID(tree, error);

  if (!file)
    ERROR();

  if (tree->root)
    {
      fprintf(file, " $$ ");

      printNode(tree->root, file);

      fprintf(file, " $$ \n");
    }
}

static void printNode(const db::TreeNode *node, FILE *file)
{
  assert(node);
  assert(file);

  fprintf(file, "{");

  bool needForLeft  = Left &&
    IS_IT_OPERATOR(node, OPERATOR_POW) && IS_OPERATOR(Left);
  bool needForRight = Right &&
    (IS_IT_OPERATOR(node, OPERATOR_SIN ) ||
     IS_IT_OPERATOR(node, OPERATOR_COS ) ||
     IS_IT_OPERATOR(node, OPERATOR_SQRT) ||
     IS_IT_OPERATOR(node, OPERATOR_LOG ) ||
     IS_IT_OPERATOR(node, OPERATOR_LN  )) && IS_OPERATOR(Right);

  if (needForLeft) fprintf(file, "(");
  if (node->left) printNode(node->left, file);
  if (needForLeft) fprintf(file, ")");

  fprintf(file, " %s ", toString(node->value, node->type));

  if (needForRight) fprintf(file, "(");
  if (node->right) printNode(node->right, file);
  if (needForRight) fprintf(file, ")");

  fprintf(file, "}");
}

static char *toString(const db::treeValue_t value, db::type_t type)
{
  static char buffer[MAX_LEXEME_SIZE] = "";

  memset(buffer, 0, MAX_LEXEME_SIZE);

  switch (type)
    {
    case db::type_t::OPERATOR:
      sprintf(
              buffer,
              " %s ",
              TEX_OPERATOR_NAMES[(int)value.operat]); break;
    case db::type_t::VARIABLE:
      sprintf(buffer, "%s", value.variable);                        break;
    case db::type_t::NUMBER:
      sprintf(buffer, "%lg", value.number);                         break;
    default:
      sprintf(buffer, "UNKNOWN TYPE!!");                            break;
    }

  return buffer;
}
