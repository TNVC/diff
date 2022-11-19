#pragma once

#include "Tree.h"
#include <stdio.h>

namespace db {

  void saveTexTree(const Tree *tree, FILE *file, int *error = nullptr);

  void saveTexNode(const TreeNode *node, FILE *file, int *error = nullptr);

};
