#include "Diff.h"
#include "DiffUtils.h"
#include "DiffDSL.h"
#include "TreeTexIO.h"

#include "Settings.h"
#include "Assert.h"
#include "ColorOutput.h"
#include "ResourceBundle.h"
#include "Error.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#pragma GCC diagnostic ignored "-Wtautological-compare"
#pragma GCC diagnostic ignored "-Wcast-qual"

const double ACCURACY = 1. / 10000.;

static inline bool isEqual(double first, double second)
{
  return fabs(first - second) < ACCURACY;
}

static inline double factorial(int n)
{
  double result = 1;

  for (int i = 2; i <= n; ++i)
    result *= i;

  return result;
}

static double getVariableValue(const db::VarTable *table,  db::variable_t variable);

static db::TreeNode *createNumber(db::number_t value);

static db::TreeNode *createVariable(db::variable_t value);

static bool isConst(const db::TreeNode *node);

static db::TreeNode *simplite(db::TreeNode *node, bool *wasChange, FILE *file);

static db::TreeNode *diff(db::TreeNode *node);

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

static bool isConst(const db::TreeNode *node)
{
  assert(node);

  if (IS_VAR(node) && !strcmp(VARIABLE(node), db::DEFAULT_MAIN_NAME))
    return false;

  if (Left  && !isConst(Left )) return false;
  if (Right && !isConst(Right)) return false;

  return true;
}

db::Tree diffExpresion(const db::Tree *tree, FILE *file, int *error)
{
  assert(tree);

  db::Tree diffTree{};

  db::createTree(&diffTree);

  if (tree->root)
    {
      diffTree.root = diff(tree->root);

      if (!diffTree.root)
        ERROR(diffTree);
    }

  bool wasChange = false;

  do
    {
      if (file)
        {
          fprintf(file, "After diff:\n");
          db::saveTexTree(tree, file);
        }

      wasChange = false;

      diffTree.root = simplite(diffTree.root, &wasChange, file);

      if (file)
        {
          fprintf(file, "After simplite:\n");
          db::saveTexTree(tree, file);
        }
    } while (wasChange);

  return diffTree;
}

db::Tree calculateTanget(const db::VarTable *table, const db::Tree *originTree, int *error)
{
  if (!isVarTableValid(table))
    ERROR({});
  if (!originTree || !originTree->root)
    ERROR({});

  db::TreeNode *var = VAR((char *)"x");

  double *value = db::searchMainVariable(table);

  db::Tree diffTree{};

  diffTree = diffExpresion(originTree, nullptr);

  db::Tree tree{};

  tree.root = ADD(
                  MUL(
                      NUM(calculateNode(table,   diffTree.root)),
                      SUB(var, NUM(*value))
                     ),
                  NUM(    calculateNode(
                                        table,
                                        originTree->root
                                       )
                     )
                 );

  db::destroyTree(&diffTree);

  return tree;
}

db::Tree calculateSeries(const db::VarTable *table, const db::Tree *originTree, int power, int *error)
{
  if (!isVarTableValid(table))
    ERROR({});
  if (!originTree || !originTree->root)
    ERROR({});

  db::TreeNode *var = VAR((char *)"x");

  double *value = db::searchMainVariable(table);

  db::Tree diffTree{};

  db::Tree series{};

  diffTree = diffExpresion(originTree, nullptr);

  series.root = NUM(calculateNode(table, diffTree.root));

  for (int k = 1; k <= power; ++k)
    {
      series.root = ADD(
                        series.root,
                        DIV(
                            MUL(
                                NUM(calculateNode(table, diffTree.root)),
                                POW(
                                    SUB(db::createNode(var), NUM(*value)),
                                    NUM(k)
                                   )
                               ),
                            NUM(factorial(k))
                           )
                       );

      db::TreeNode *temp = diffTree.root;
      diffTree = diffExpresion(&diffTree, nullptr);
      db::removeNode(temp);
    }

  db::destroyTree(&diffTree);

  db::removeNode(var);

  return series;
}

void executeExpresion(const db::Tree *tree, int *error)
{
  assert(tree);
  if (!tree->root)
    ERROR();

  Settings settings{};
  getSettings(&settings);

  db::VarTable *table = settings.table;

  double result = calculateNode(table, tree->root);

  printf("%lg\n", result);
}

static double getVariableValue(const db::VarTable *table, db::variable_t variable)
{
  assert(table);
  assert(variable);

  for (size_t i = 0; i < table->size; ++i)
    if (!strcmp(table->table[i].name, variable))
      return table->table[i].value;

  return NAN;
}

double calculateNode(const db::VarTable *table, const db::TreeNode *node)
{
  assert(table);
  //assert(node);

  if (IS_NUM(node)) return NUMBER(node);
  if (IS_VAR(node))
    {
      if (!isnan(*db::searchMainVariable(table)) && !strcmp(
                                               VARIABLE(node),
                                               db::DEFAULT_MAIN_NAME
                                              ))
        return *db::searchMainVariable(table);

      return getVariableValue(table, VARIABLE(node));
    }

  double leftValue  = (Left  ? calculateNode(table, Left ) : NAN);
  double rightValue = (Right ? calculateNode(table, Right) : NAN);

  switch (OPERATOR(node))
    {
    case db::OPERATOR_ADD : return leftValue + rightValue;
    case db::OPERATOR_SUB : return leftValue - rightValue;
    case db::OPERATOR_MUL : return leftValue * rightValue;
    case db::OPERATOR_DIV : return leftValue / rightValue;
    case db::OPERATOR_SQRT: return sqrt(rightValue);
    case db::OPERATOR_SIN : return sin(rightValue);
    case db::OPERATOR_COS : return cos(rightValue);
    case db::OPERATOR_POW : return pow(leftValue, rightValue);
    case db::OPERATOR_LOG : return log(rightValue) / log(leftValue);
    case db::OPERATOR_LN  : return log(rightValue);
    case db::OPERATORS_COUNT:
    default: return NAN;
    }
}

static db::TreeNode *diff(db::TreeNode *node)
{
  assert(node);

  switch (node->type)
    {
    case db::type_t::NUMBER:   return NUM(0);
    case db::type_t::VARIABLE:
      {
        if (!strcmp(VARIABLE(node), db::DEFAULT_MAIN_NAME))
          return NUM(1);
        else
          return NUM(0);
      }
    case db::type_t::OPERATOR:
      switch (OP_VAL(node))
        {
        case db::OPERATOR_ADD:
          return ADD(dLeft, dRight);
        case db::OPERATOR_SUB:
          return SUB(dLeft, dRight);
        case db::OPERATOR_MUL:
          return ADD(MUL(dLeft, CopyRight), MUL(CopyLeft, dRight));
        case db::OPERATOR_DIV:
          return DIV(
                     SUB(MUL(dLeft, CopyRight), MUL(CopyLeft, dRight)),
                     MUL(CopyRight, CopyRight)
                    );
        case db::OPERATOR_SIN:
          return MUL(COS(CopyRight), dRight);
        case db::OPERATOR_COS:
          return MUL(NUM(-1), MUL(SIN(CopyRight), dRight));
        case db::OPERATOR_POW:
          {
            bool isLeftConst  = isConst(Left);
            bool isRightConst = isConst(Right);

            if (isLeftConst && isRightConst)
              return NUM(0);

            if (isLeftConst && !isRightConst)
              return MUL(
                         MUL(POW(CopyLeft, CopyRight), LN(CopyLeft)),
                         dRight
                        );

            if (!isLeftConst && isRightConst)
              return MUL(
                         MUL(
                             CopyRight,
                             POW(CopyLeft, SUB(CopyRight, NUM(1)))
                            ),
                         dLeft
                        );

            if (!isLeftConst && !isRightConst)
              return MUL(
                         POW(CopyLeft, CopyRight),
                         ADD(
                             MUL(dLeft, CopyRight),
                             MUL(CopyLeft, dRight)
                            )
                        );

            return nullptr;
          }
        case db::OPERATOR_SQRT:
          return DIV(dRight, MUL(NUM(2), SQRT(CopyRight)));
        case db::OPERATOR_LOG:
          return DIV(MUL(dRight, LN(CopyLeft)), CopyRight);
        case db::OPERATOR_LN:
          return DIV(dRight, CopyRight);
        case db::OPERATORS_COUNT:
        default: return nullptr;
        }
    default: return nullptr;
    }
}

static db::TreeNode *simplite(db::TreeNode *node, bool *wasChange, FILE *file)
{
  assert(node);
  assert(wasChange);

  if (IS_NUM(node)) return node;

  if (IS_VAR(node)) return node;

  if (IS_OPERATOR(node))
    {
      if (Left ) Left  = simplite(Left , wasChange, file);
      if (Right) Right = simplite(Right, wasChange, file);

      bool isLeftConst  = (Left  ? isConst(Left ) : true);
      bool isRightConst = (Right ? isConst(Right) : true);

      switch (OPERATOR(node))
        {
        case db::OPERATOR_ADD:
          {
            if (isLeftConst && isRightConst)
              CALC_CONST(NUMBER(Left) + NUMBER(Right));
            else if (IS_EQUAL(Left, 0)|| IS_EQUAL(Right, 0))
              UPPER_NODE(IS_EQUAL(Left, 0) ? Right  : Left);
            else if (IS_NUM(Right) && NUMBER(Right) < 0)
              {
                OPERATOR(node) = db::OPERATOR_SUB;

                Right = MUL(NUM(-1), Right);
              }

            break;
          }
        case db::OPERATOR_SUB:
          {
            if (isLeftConst && isRightConst)
              CALC_CONST(NUMBER(Left) - NUMBER(Right));
            else if (IS_EQUAL(Right, 0))
              UPPER_NODE(Left);
            else if (IS_NUM(Right) && NUMBER(Right) < 0)
              {
                OPERATOR(node) = db::OPERATOR_ADD;

                Right = MUL(NUM(-1), Right);
              }

            break;
          }
        case db::OPERATOR_MUL:
          {
            if (isLeftConst && isRightConst)
              CALC_CONST(NUMBER(Left) * NUMBER(Right));
            else if (IS_EQUAL(Left, 0) || IS_EQUAL(Right, 0))
              TO_NUMBER(0);
            else if (IS_EQUAL(Left, 1) || IS_EQUAL(Right, 1))
              UPPER_NODE(IS_EQUAL(Left, 1) ? Right : Left);

            break;
          }
        case db::OPERATOR_DIV:
          {
              if (isLeftConst && isRightConst)
                CALC_CONST(NUMBER(Left) / NUMBER(Right));
              else if (IS_EQUAL(Left, 0))
                TO_NUMBER(0);
              else if (IS_EQUAL(Right, 1))
                UPPER_NODE(Left);

            break;
          }
        case db::OPERATOR_SQRT:
          {
            if (isRightConst)
                CALC_CONST(sqrt(NUMBER(Right)));

            break;
          }
        case db::OPERATOR_SIN:
          {
            if (isRightConst)
              CALC_CONST(sin(NUMBER(Right)));

            break;
          }
        case db::OPERATOR_COS:
          {
            if (isRightConst)
              CALC_CONST(cos(NUMBER(Right)));

            break;
          }
        case db::OPERATOR_POW:
          {
            if (isLeftConst && isRightConst)
              CALC_CONST(pow(NUMBER(Left), NUMBER(Right)));

            else if (IS_EQUAL(Left, 1) || IS_EQUAL(Right, 0))
              TO_NUMBER(1);
            else if (IS_EQUAL(Right, 1))
              UPPER_NODE(Left);

            break;
          }
        case db::OPERATOR_LOG:
          {
            if (isLeftConst && isRightConst)
              CALC_CONST(log(NUMBER(Right)) / log(NUMBER(Left)));

            break;
          }
        case db::OPERATOR_LN:
          {
            if (isRightConst)
              CALC_CONST(log(NUMBER(Right)));

            break;
          }
        case db::OPERATORS_COUNT:
        default:
            return nullptr;
        }
    }

  return node;
}
