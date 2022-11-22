#include "Variable.h"
#include "Tree.h"

#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "Error.h"
#include "Assert.h"
#include "ResourceBundle.h"
#include "ColorOutput.h"
#include "ErrorHandler.h"
#include "Diff.h"
#include "SystemLike.h"
#include "GarbageCollector.h"

const int DEFAULT_GROWTH_FACTOR = 2;

static void searchAndUpdateVariable(db::VarTable *table, db::TreeNode *expression);

static bool searchVariable(const db::VarTable *table, db::variable_t value);

void db::updateVarTable(db::VarTable *table, db::TreeNode *expression, int *error)
{
  if (!table)
    ERROR();

  if (!isVarTableValid(table))
    ERROR();

  if (!expression)
    ERROR();

  searchAndUpdateVariable(table, expression);
}

static void searchAndUpdateVariable(db::VarTable *table, db::TreeNode *expression)
{
  assert(table);
  assert(isVarTableValid(table));
  assert(expression);

  if (expression->type == db::type_t::NUMBER) return;

  if (expression->left )
    searchAndUpdateVariable(table, expression->left );
  if (expression->right)
    searchAndUpdateVariable(table, expression->right);

  if (expression->type == db::type_t::VARIABLE)
    {
      if (searchVariable(table, expression->value.variable)) return;

      printf("%s" ITALIC "%s" RESET ": ",
             db::getString(getBundle(), "variable.read"),
             expression->value.variable);

      double value = NAN;

      while (scanf(" %lg", &value) != 1 && !isfinite(value))
        {
          while (getchar() != '\n') continue;

          printf("%s", db::getString(getBundle(), "input.incorrect"));
        }

      while (getchar() != '\n') continue;

      if (table->size == table->capacity)
        {
          db::Variable *temp = (db::Variable *)recalloc(table->table, (table->capacity+1)*DEFAULT_GROWTH_FACTOR, sizeof(db::Variable));

          if (!temp)
            {
              free(table->table);

              handleError("Out of memory!!");

              assert(0);///////
            }

          table->table = temp;

          ++table->capacity;
          table->capacity *= DEFAULT_GROWTH_FACTOR;
        }

      table->table[table->size++] = {strdup(expression->value.variable), 0, value};

      addElementForFree(table->table[table->size - 1].name);
    }
}

static bool searchVariable(const db::VarTable *table, db::variable_t value)
{
  assert(table);
  assert(isVarTableValid(table));

  for (size_t i = 0; i < table->size; ++i)
    if (!strcmp(table->table[i].name, value))
      return true;

  return false;
}

double *db::searchMainVariable(const db::VarTable *table, int *error)
{
  if (!table)
    ERROR(nullptr);

  for (size_t i = 0; i < table->size; ++i)
    if (!strcmp(table->table[i].name, db::DEFAULT_MAIN_NAME))
      return &table->table[i].value;

  return nullptr;
}
