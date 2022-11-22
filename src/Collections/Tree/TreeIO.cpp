#include "Tree.h"
#include "TreeDump.h"
#include "TreeTexIO.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "Settings.h"
#include "SystemLike.h"
#include "Fiofunctions.h"
#include "StringsUtils.h"
#include "Assert.h"
#include "ErrorHandler.h"

const int MAX_NAME_SIZE = 256;
static_assert(MAX_NAME_SIZE > 0);

const int MAX_LEXEME_SIZE = 48;
static_assert(MAX_LEXEME_SIZE >= db::MAX_VARIABLE_SIZE);

static char *toString(const db::treeValue_t value, db::type_t type);

static db::treeValue_t toValue(const char *string, int *error);

static db::type_t getType(const char *string, int *error);

static void printNode(const db::TreeNode *node, FILE *file);

static db::TreeNode *scanNode(FILE *file, int *error);

static bool isBinary(db::treeValue_t value, db::type_t type);

static bool hasRightOperant(const db::TreeNode *node);

void db::saveTree(const db::Tree *tree, FILE *file, int *error)
{
  CHECK_VALID(tree, error);

  if (!file)
    ERROR();

  Settings settings{};

  getSettings(&settings);

  if (settings.saveType == Save::TEX)
    {
      db::saveTexTree(tree, file, error);

      return;
    }

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

static db::Tree getGeneral(const char *source, bool *fail);

void db::loadTree(
                  db::Tree *tree,
                  const char *fileName,
                  int *error
                  )
{
  CHECK_VALID(tree, error);

  if (!fileName) ERROR();

  size_t fileSize = getFileSize(fileName);

  char *buffer = (char *)calloc(fileSize, sizeof(char));

  readFile(&buffer, fileName);

  bool hasError = false;

  *tree = getGeneral(buffer, &hasError);

  free(buffer);

  if (hasError) ERROR();

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
      sprintf(
              buffer,
              " %s ",
              db::OPERATOR_NAMES[(int)value.operat]); break;
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

    if (fscanf(file, " %[^ ()]", buff) == 1)
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

        bool isNodeBinary = isBinary(value, type);

        if (( isNodeBinary && !leftChild) ||
            (!isNodeBinary &&  leftChild))
          {
            db::removeNode(leftChild);

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
    else
      {
        if (leftChild) db::removeNode(leftChild);

        return nullptr;
      }

    db::TreeNode *rightChild = scanNode(file, error);

    newNode->left  = leftChild;
    newNode->right = rightChild;

    bool hasNodeRightOperant = hasRightOperant(newNode);

    if (( hasNodeRightOperant && !rightChild) ||
        (!hasNodeRightOperant && rightChild))
      {
        db::removeNode(newNode);

        return nullptr;
      }

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

static bool isBinary(db::treeValue_t value, db::type_t type)
{
  if (type != db::type_t::OPERATOR) return false;

  for (int i = 0; i < db::BINARY_OPERATORS_COUNT; ++i)
    if (value.operat == db::BINARY_OPERATORS[i])
      return true;

  return false;
}

static bool hasRightOperant(const db::TreeNode *node)
{
  if (node->type != db::type_t::OPERATOR) return false;

  return true;
}


//#######################################################################

#include "ErrorHandler.h"
#include "DiffDSL.h"
#include <ctype.h>

#define FAIL(...)                               \
  do                                            \
    {                                           \
      handleError("Was unknow error in "        \
                  "FILE:%s at LINE:%d",         \
                  __FILE__, __LINE__);          \
                                                \
      if (fail) *fail = true;                   \
                                                \
      return __VA_ARGS__;                       \
    } while (0)

static void skipSpaces(const char **source, bool *fail);

//static db::Tree      getGeneral          (const char *source, bool *fail);
static db::TreeNode *getExpression       (const char **source, bool *fail);
static db::TreeNode *getTermin           (const char **source, bool *fail);
static db::TreeNode *getPrimaryExpression(const char **source, bool *fail);
static db::TreeNode *getFunction         (const char **source, bool *fail);
static db::TreeNode *getNumber           (const char **source, bool *fail);
static db::TreeNode *getName             (const char **source, bool *fail);

static db::TreeNode *createNumber(db::number_t value);
static db::TreeNode *createVariable(db::variable_t value);

static void skipSpaces(const char **source, bool *fail)
{
  if (!source || !*source || !fail) FAIL();

  while (isspace(**source)) (*source)++;
}

static db::TreeNode *createNumber(db::number_t value)
{
  db::TreeNode *node = db::createNode({.number = value}, db::type_t::NUMBER);

  return node;
}

static db::TreeNode *createVariable(db::variable_t value)
{
  db::TreeNode *node = db::createNode({.variable = value}, db::type_t::VARIABLE);

  return node;
}

static db::Tree getGeneral(const char *source, bool *fail)
{
  if (!source || !fail) FAIL({});

  db::Tree value{};

  *fail = false;

  value.root = getExpression(&source, fail);

  skipSpaces(&source, fail);

  char ch = *source;

  if (ch != '\0')
    {
      handleError("Expected terminator, but found '%c'=%d", isprint(ch) ? ch : '~', ch);

      *fail = true;

      db::destroyTree(&value);

      return {};
    }

  return value;
}

static db::TreeNode *getExpression(const char **source, bool *fail)
{
  if (!source || !*source || !fail) FAIL(nullptr);

  db::TreeNode *value = getTermin(source, fail);

  skipSpaces(source, fail);

  while (strchr("+-", **source) && **source)
    {
      char operat = *(*source)++;

      db::TreeNode *tempValue = getTermin(source, fail);

      if (operat == '+')
        value = ADD(value, tempValue);
      else
        value = SUB(value, tempValue);

      skipSpaces(source, fail);
    }

  return value;
}

static db::TreeNode *getTermin(const char **source, bool *fail)
{
  if (!source || !*source || !fail) FAIL(nullptr);

  db::TreeNode *value = getFunction(source, fail);

  skipSpaces(source, fail);

  while (strchr("*/", **source) && **source)
    {
      char operat = *(*source)++;

      db::TreeNode *tempValue = getFunction(source, fail);

      if (operat == '*')
        value = MUL(value, tempValue);
      else
        value = DIV(value, tempValue);

      skipSpaces(source, fail);
    }

  return value;
}

static db::TreeNode *getFunction(const char **source, bool *fail)
{
  if (!source || !*source || !fail) FAIL(nullptr);

  bool isntFunction = false;

  db::TreeNode *value = getName(source, &isntFunction);

  if (!isntFunction)
    {
      SET_TO_OPERATOR(value);

      value->right = getPrimaryExpression(source, &isntFunction);

      if (isntFunction)
        {
          SET_TO_VARIABLE(value);

          return value;
        }

      for (int i = 0; i < db::OPERATORS_COUNT; ++i)
        if (!strcmp(db::OPERATOR_NAMES[i], VARIABLE(value)))
          {
            OPERATOR(value) = (db::operator_t)i;

            break;
          }

      if (OPERATOR(value) >= db::OPERATORS_COUNT || OPERATOR(value) < 0)
        {
          handleError("Unknown function \"%s\"", VARIABLE(value));

          db::removeNode(value);

          return nullptr;
        }
    }
  else
    value = getPrimaryExpression(source, fail);

  return value;
}

static db::TreeNode *getPrimaryExpression(const char **source, bool *fail)
{
  if (!source || !*source || !fail) FAIL(nullptr);

  db::TreeNode *value = nullptr;

  skipSpaces(source, fail);

  if (**source == '(')
    {
      value = getExpression(&++*source, fail);

      skipSpaces(source, fail);

      char ch = **source;
      if (ch != ')')
        {
          handleError("Expected '(', but found '%c'=%d", isprint(ch) ? ch : '~', ch);

          if (value) db::removeNode(value);

          FAIL(nullptr);
        }

      ++*source;
    }
  else
    {
      bool hasntVariable = false;
      value = getName(source, &hasntVariable);

      if (hasntVariable)
        value = getNumber(source, fail);
    }

  return value;
}

static db::TreeNode *getNumber(const char **source, bool *fail)
{
  if (!source || !*source || !fail) FAIL(nullptr);

  skipSpaces(source, fail);

  double value = 0;

  const char *startPosition = *source;

  while (isdigit(**source))
    value = 10*value + *(*source)++-'0';

  if (*source <= startPosition) { *fail = true; return nullptr; }

  return NUM(value);
}

static db::TreeNode *getName(const char **source, bool *fail)
{
  if (!source || !*source || !fail) FAIL(nullptr);

  skipSpaces(source, fail);

  char buffer[MAX_NAME_SIZE] = "";

  int i = 0;

  const char *startPosition = *source;

  while (isalpha(**source))
    buffer[i++] = *(*source)++;

  if (*source <= startPosition) { *fail = true; return nullptr; }

  return VAR(buffer);
}
