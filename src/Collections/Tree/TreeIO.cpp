#include "Tree.h"
#include "TreeDump.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "StringsUtils.h"
#include "Assert.h"
#include "ErrorHandler.h"

const int MAX_LEXEME_SIZE = 48;

static_assert(MAX_LEXEME_SIZE >= db::MAX_VARIABLE_SIZE);

static char *toString(const db::treeValue_t value, db::type_t type);

static db::treeValue_t toValue(const char *string, int *error);

static db::type_t getType(const char *string, int *error);

static void printNode(const db::TreeNode *node, FILE *file);

static db::TreeNode *scanNode(FILE *file, int *error);

void db::saveTree(const db::Tree *tree, FILE *file, int *error)
{
  CHECK_VALID(tree, error);

  if (!file)
    ERROR();

  if(tree->root)
    {
      printNode(tree->root, file);

      putc('\n', file);
    }

  CHECK_VALID(tree, error);
}

void db::loadTree(db::Tree *tree, FILE *file, int *error)
{
  CHECK_VALID(tree, error);

  if (!file)
    ERROR();

  int errorCode = 0;

  tree->root = scanNode(file, &errorCode);

  if (errorCode)
    ERROR();

  CHECK_VALID(tree, error);
}

static void printNode(const db::TreeNode *node, FILE *file)
{
  assert(node);
  assert(file);

  fprintf(file, "(");

  if (node->left)
    printNode(node->left, file);

  fprintf(file, "%s", toString(node->value, node->type));

  if (node->right)
      printNode(node->right, file);

  fprintf(file, ")");
}

static char *toString(const db::treeValue_t value, db::type_t type)
{
  static char buffer[MAX_LEXEME_SIZE] = "";

  memset(buffer, 0, MAX_LEXEME_SIZE);

  switch (type)
    {
    case db::type_t::OPERATOR:
      sprintf(buffer, "%s", db::OPERATOR_NAMES[(int)value.number]); break;
    case db::type_t::VARIABLE:
      sprintf(buffer, "%s", value.variable);                        break;
    case db::type_t::NUMBER:
      sprintf(buffer, "%lg", value.number);                         break;
    default:
      sprintf(buffer, "UNKNOWN TYPE!!");                            break;
    }

  return buffer;
}

static db::TreeNode *scanNode(FILE *file, int *error)
{
  assert(file);
  assert(error);

  char ch = '\0';

  if (!fscanf(file, " %c", &ch) || ch != '(')
    {
      if (ch != EOF) ungetc(ch, file);

      return nullptr;
    }

    db::TreeNode *leftChild = scanNode(file, error);

    if (*error) return nullptr;

    db::TreeNode *newNode = nullptr;

    char buff[MAX_LEXEME_SIZE] = "";

    if (fscanf(file, " %[^()]", buff))
      {
        trimString(buff);

        db::treeValue_t value = toValue(buff, error);

        if (*error)
          {
            if (leftChild) db::removeNode(leftChild);

            return nullptr;
          }

        db::type_t type = getType(buff, error);

        if (*error)
          {
            if (leftChild) db::removeNode(leftChild);

            return nullptr;
          }

        newNode = createNode(value, type, error);

        if (type == db::type_t::VARIABLE)
          free(value.variable);

        if (*error)
          {
            if (leftChild) db::removeNode(leftChild);

            return nullptr;
          }
      }

    db::TreeNode *rightChild = scanNode(file, error);

    newNode->left  = leftChild;
    newNode->right = rightChild;

    if (*error || !fscanf(file, " %c", &ch) || ch != ')')
    {
      *error = true;

      if (newNode) removeNode(newNode);

      return nullptr;
    }

  return newNode;
}

static db::treeValue_t toValue(const char *string, int *error)
{
  assert(string);
  assert(error);

  db::treeValue_t value = {};

  for (int i = 0; i < db::OPERATORS_COUNT; ++i)
    if (!strcmp(db::OPERATOR_NAMES[i], string))
      {
        value.operat = (db::operator_t)i;

        return value;
      }

  if (isDigitString(string))
    {
      sscanf(string, "%lg", &value.number);

      return value;
    }

  char *buffer = (char *)calloc(db::MAX_VARIABLE_SIZE + 1, sizeof(char));

  if (!buffer)
    {
      handleError("Out of memory!!");

      *error = true;

      return value;
    }

  if (isCorrectName(string))
    {
      strncpy(buffer, string, db::MAX_VARIABLE_SIZE);

      value.variable = buffer;
    }
  else
    {
      handleError("Invalid node value[%s]!!", string);

      free(buffer);

      *error = true;
    }

  return value;
}

static db::type_t getType(const char *string, int *error)
{
  assert(string);
  assert(error);

  for (int i = 0; i < db::OPERATORS_COUNT; ++i)
    if (!strcmp(db::OPERATOR_NAMES[i], string))
        return db::type_t::OPERATOR;

  if (isDigitString(string))
    return db::type_t::NUMBER;

  if (!isCorrectName(string))
    {
      handleError("Invalid node value[%s]!!", string);

      *error = true;
    }

  return db::type_t::VARIABLE;
}
