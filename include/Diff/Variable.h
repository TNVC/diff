#pragma once

#include <stddef.h>
#include "Tree.h"

namespace db {

  const char *const DEFAULT_MAIN_NAME = "x";

  struct Variable {
    char *name;
    int number;
    mutable double value;
  };

  struct VarTable {
    Variable *table;
    size_t capacity;
    size_t size;

    VarTable &operator=(const VarTable &original) = delete;
  };


  inline bool isVarTableValid(const VarTable *table)
  {
    return
      (table) &&
      (table->capacity ? (bool)table->table : !table->table) &&
      (table->size <= table->capacity);
  }

  void updateVarTable(db::VarTable *table, db::TreeNode *expression, int *error = nullptr);

  double *searchMainVariable(const VarTable *table, int *error = nullptr);
}
